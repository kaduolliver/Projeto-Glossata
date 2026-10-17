"""Crash/restart tests. All permissions, humans and evidence are synthetic fixtures."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import unittest
from unittest import mock

PROJECT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT / 'scripts'))
import content_governance as governance
import curation_decisions as decisions
import governance_storage as storage
import test_curation_decisions as fixtures

HISTORY = 'resources/compliance/permission_history.jsonl'
PROJECTION = 'resources/data/photo_provenance.json'
STAGES = ('temporary_opened', 'temporary_written', 'temporary_synced', 'replaced', 'directory_synced')


class PermissionRecoveryTests(unittest.TestCase):
    def fixture(self):
        fixture = fixtures.DecisionTests('test_templates_never_create_approval_or_history')
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        root = fixture.root
        live = governance.archive_permission(root, fixture.fixture.observation, fixture.fixture.photo,
                                             'live_api', governance.now())
        draft = decisions.template(root, fixture.config, 'permission', '7',
                                   fixture.rid, 2)
        draft['decision'].update(reviewer='fixture-human', reviewed_at=governance.now(),
            rationale='Synthetic test only; not an actual permission.', license_version='4.0',
            evidence_paths=[live], license_version_evidence=fixture.public_proof())
        envelope = decisions.template(root, fixture.config, 'permission', '7', fixture.rid, 2, draft)
        return fixture, envelope

    def restart_recovery(self, root):
        code = ('import sys,json; from pathlib import Path; '
                'sys.path.insert(0,sys.argv[1]); import content_governance as g; '
                'print(json.dumps(g.permission_recovery(Path(sys.argv[2]),apply=True)))')
        result = subprocess.run([sys.executable, '-c', code, str(PROJECT/'scripts'), str(root)],
                                capture_output=True, text=True, timeout=20, check=True)
        return json.loads(result.stdout)

    def assert_consistent_once(self, fixture, envelope):
        root = fixture.root
        self.assertEqual(self.restart_recovery(root)['state'], 'consistent')
        decisions.record_decision(root, fixture.config, envelope)
        ledger = (root/HISTORY).read_bytes()
        projected = (root/PROJECTION).read_bytes()
        replay = decisions.record_decision(root, fixture.config, envelope)
        self.assertEqual(replay['state'], 'already_recorded')
        self.assertEqual(ledger, (root/HISTORY).read_bytes())
        self.assertEqual(projected, (root/PROJECTION).read_bytes())
        events = governance.read_chain(root/HISTORY)
        self.assertEqual(len(events), 1)
        self.assertEqual(governance.permission_provenance(root)['7'], events[0]['after'])
        self.assertEqual(events[0]['after']['permission_confirmation']['reviewer'], 'fixture-human')
        self.assertEqual(events[0]['after']['permission_confirmation']['reviewed_at'], envelope['decision']['reviewed_at'])

    def test_exceptions_before_and_after_each_write_stage_recover_without_duplicates(self):
        for filename in (HISTORY, PROJECTION):
            for stage in STAGES:
                with self.subTest(file=filename, stage=stage):
                    fixture, envelope = self.fixture()
                    def crash(point, path):
                        if point == stage and path == fixture.root/filename:
                            raise OSError('Synthetic injected interruption')
                    with mock.patch.object(storage, '_checkpoint', side_effect=crash):
                        with self.assertRaises(OSError):
                            decisions.record_decision(fixture.root, fixture.config, envelope)
                    self.assert_consistent_once(fixture, envelope)

    def test_abrupt_process_exit_at_each_stage_releases_lock_and_recovers_on_restart(self):
        code = '''import sys,os,json
from pathlib import Path
sys.path.insert(0,sys.argv[1])
import curation_decisions as d, curation_requests as r, governance_storage as s
root=Path(sys.argv[2])
def crash(stage,path):
    if stage==sys.argv[4] and path==root/sys.argv[3]: os._exit(73)
s._checkpoint=crash
d.record_decision(root,r.settings(),json.loads((root/'envelope.json').read_text(encoding='utf8')))
'''
        for filename in (HISTORY, PROJECTION):
            for stage in STAGES:
                with self.subTest(file=filename, stage=stage):
                    fixture, envelope = self.fixture()
                    governance.save(fixture.root/'envelope.json', envelope)
                    result = subprocess.run([sys.executable, '-c', code, str(PROJECT/'scripts'),
                                             str(fixture.root), filename, stage], capture_output=True, timeout=20)
                    self.assertEqual(result.returncode, 73, result.stderr)
                    self.assert_consistent_once(fixture, envelope)

    def test_fsync_and_replace_errors_do_not_corrupt_live_files(self):
        for operation in ('fsync', 'replace'):
            with self.subTest(operation=operation):
                fixture, envelope = self.fixture()
                before = (fixture.root/PROJECTION).read_bytes()
                with mock.patch.object(storage.os, operation, side_effect=OSError('Synthetic I/O failure')):
                    with self.assertRaises(OSError):
                        decisions.record_decision(fixture.root, fixture.config, envelope)
                self.assertEqual(before, (fixture.root/PROJECTION).read_bytes())
                self.assert_consistent_once(fixture, envelope)

    def test_already_recorded_reconciles_before_returning_and_audit_fails_closed(self):
        fixture, envelope = self.fixture(); root = fixture.root
        before = governance.load(root/PROJECTION)
        decisions.record_decision(root, fixture.config, envelope)
        ledger = (root/HISTORY).read_bytes()
        governance.save(root/PROJECTION, before)
        self.assertEqual(governance.permission_recovery(root)['state'], 'recovery_required')
        self.assertIn('permission_projection_inconsistent', governance.credits(root)['photos']['7']['rights_issues'])
        self.assertIn('permission_projection_inconsistent',
                      {e['error'] for e in governance.audit_governance(root)['technical_errors']})
        self.assertEqual(before, governance.load(root/PROJECTION))  # audits never repair
        self.assertEqual(decisions.record_decision(root, fixture.config, envelope)['state'], 'already_recorded')
        self.assertEqual(ledger, (root/HISTORY).read_bytes())
        self.assertEqual(governance.permission_recovery(root)['state'], 'consistent')

    def test_direct_and_legacy_decisions_are_idempotent_without_rehashing_history(self):
        fixture, envelope = self.fixture(); root = fixture.root
        decision = envelope['decision']
        governance.confirm_permission(root, decision)
        ledger = (root/HISTORY).read_bytes()
        governance.confirm_permission(root, decision)
        self.assertEqual(ledger, (root/HISTORY).read_bytes())
        # Emulate the old event schema in a temporary fixture only.
        event = governance.read_chain(root/HISTORY)[0]
        del event['operation_sha256']; del event['event_sha256']
        event['event_sha256'] = governance.digest(event)
        legacy = (json.dumps(event, ensure_ascii=False)+'\n').encode('utf8')
        (root/HISTORY).write_bytes(legacy)
        provenance = governance.load(root/PROJECTION); provenance['7'] = event['before']
        governance.save(root/PROJECTION, provenance)
        self.assertEqual(self.restart_recovery(root)['state'], 'consistent')
        governance.confirm_permission(root, decision)
        self.assertEqual(legacy, (root/HISTORY).read_bytes())

    def test_multiple_events_replay_to_latest_without_overwriting_later_human_decisions(self):
        fixture, first = self.fixture(); root = fixture.root
        before = governance.load(root/PROJECTION)
        before['unrelated'] = {'photo_id': 'unrelated', 'legacy_note': 'Keep all metadata'}
        governance.save(root/PROJECTION, before)
        decisions.record_decision(root, fixture.config, first)
        first_bytes = (root/HISTORY).read_bytes()
        second = copy.deepcopy(first)
        second['decision']['rationale'] = 'Later explicit synthetic human decision'
        second = decisions.template(root, fixture.config, 'permission', '7', fixture.rid, 2, second)
        decisions.record_decision(root, fixture.config, second)
        ledger = (root/HISTORY).read_bytes()
        self.assertTrue(ledger.startswith(first_bytes))
        governance.save(root/PROJECTION, before)
        self.assertEqual(self.restart_recovery(root)['state'], 'consistent')
        projected = governance.load(root/PROJECTION)
        self.assertEqual(projected['unrelated'], before['unrelated'])
        self.assertEqual(projected['7']['permission_confirmation']['rationale'], second['decision']['rationale'])
        with self.assertRaisesRegex(ValueError, 'superseded'):
            decisions.record_decision(root, fixture.config, first)
        self.assertEqual(projected, governance.load(root/PROJECTION))
        self.assertEqual(ledger, (root/HISTORY).read_bytes())

    def assert_blocked_unchanged(self, fixture, envelope):
        root = fixture.root
        before = {name: (root/name).read_bytes() if (root/name).exists() else None for name in (HISTORY, PROJECTION)}
        report = self.restart_recovery(root)
        self.assertEqual(report['state'], 'blocked'); self.assertTrue(report['errors'])
        with self.assertRaises(ValueError):
            decisions.record_decision(root, fixture.config, envelope)
        for name, data in before.items():
            self.assertEqual(data, (root/name).read_bytes() if (root/name).exists() else None)

    def test_unknown_projection_blocks_instead_of_overwriting_manual_changes(self):
        fixture, envelope = self.fixture(); root = fixture.root
        decisions.record_decision(root, fixture.config, envelope)
        provenance = governance.load(root/PROJECTION); provenance['7']['manual_note'] = 'Unknown human edit'
        governance.save(root/PROJECTION, provenance)
        self.assert_blocked_unchanged(fixture, envelope)

    def test_missing_changed_evidence_or_photo_binding_blocks_recovery(self):
        for change in ('proof', 'evidence', 'photo'):
            with self.subTest(change=change):
                fixture, envelope = self.fixture(); root = fixture.root
                before = governance.load(root/PROJECTION)
                decisions.record_decision(root, fixture.config, envelope)
                governance.save(root/PROJECTION, before)
                if change == 'proof':
                    (root/envelope['decision']['license_version_evidence']['path']).write_text('Changed fixture')
                elif change == 'evidence':
                    (root/envelope['decision']['evidence_paths'][0]).unlink()
                else:
                    species = governance.load(root/'resources/data/species.json')
                    species[0]['photo']['sha256'] = 'b'*64
                    governance.save(root/'resources/data/species.json', species)
                self.assert_blocked_unchanged(fixture, envelope)

    def test_missing_corrupt_projection_or_torn_history_never_erases_records(self):
        for change in ('missing', 'corrupt', 'torn_history', 'changed_hash'):
            with self.subTest(change=change):
                fixture, envelope = self.fixture(); root = fixture.root
                decisions.record_decision(root, fixture.config, envelope)
                if change == 'missing': (root/PROJECTION).unlink()
                elif change == 'corrupt': (root/PROJECTION).write_text('{')
                elif change == 'torn_history':
                    with (root/HISTORY).open('ab') as output: output.write(b'{"incomplete":')
                else:
                    events = governance.read_chain(root/HISTORY); events[0]['after']['license_version'] = '3.0'
                    (root/HISTORY).write_text(json.dumps(events[0])+'\n', encoding='utf8')
                self.assert_blocked_unchanged(fixture, envelope)

    def test_divergent_transitions_block_even_with_valid_chain_hashes(self):
        fixture, envelope = self.fixture(); root = fixture.root
        governance.confirm_permission(root, envelope['decision'])
        other = copy.deepcopy(envelope['decision']); other['rationale'] = 'Second fixture decision'
        governance.confirm_permission(root, other)
        events = governance.read_chain(root/HISTORY)
        events[1]['before']['unknown_edit'] = True
        del events[1]['event_sha256']; events[1]['event_sha256'] = governance.digest(events[1])
        (root/HISTORY).write_text(''.join(json.dumps(e)+'\n' for e in events), encoding='utf8')
        self.assert_blocked_unchanged(fixture, envelope)

    def test_confirmation_without_event_is_not_accepted_as_a_permission(self):
        fixture, envelope = self.fixture(); root = fixture.root
        governance.confirm_permission(root, envelope['decision'])
        (root/HISTORY).write_bytes(b'')
        self.assert_blocked_unchanged(fixture, envelope)

    def test_shared_lock_blocks_other_writers_and_importer_preserves_decisions(self):
        fixture, envelope = self.fixture(); root = fixture.root
        with storage.governance_lock(root):
            with self.assertRaises(FileExistsError): governance.confirm_permission(root, envelope['decision'])
            with self.assertRaises(FileExistsError): decisions.record_decision(root, fixture.config, envelope)
            with self.assertRaises(FileExistsError): governance.register_photo_provenance(root, '8', {})
        governance.confirm_permission(root, envelope['decision'])
        before = (root/PROJECTION).read_bytes()
        with self.assertRaises(ValueError): governance.register_photo_provenance(root, '7', {'photo_id': '7'})
        self.assertEqual(before, (root/PROJECTION).read_bytes())

    def test_recovery_itself_can_be_interrupted_and_repeated_at_every_write_stage(self):
        for stage in STAGES:
            with self.subTest(stage=stage):
                fixture, envelope = self.fixture(); root = fixture.root
                before = governance.load(root/PROJECTION)
                decisions.record_decision(root, fixture.config, envelope)
                ledger = (root/HISTORY).read_bytes()
                governance.save(root/PROJECTION, before)
                def crash(point, path):
                    if point == stage and path == root/PROJECTION:
                        raise OSError('Synthetic recovery interruption')
                with mock.patch.object(storage, '_checkpoint', side_effect=crash):
                    with self.assertRaises(OSError): governance.permission_recovery(root, apply=True)
                self.assert_consistent_once(fixture, envelope)
                self.assertEqual(ledger, (root/HISTORY).read_bytes())

    def test_legacy_migration_never_replaces_existing_provenance_or_history(self):
        fixture, envelope = self.fixture(); root = fixture.root
        decisions.record_decision(root, fixture.config, envelope)
        original = (root/PROJECTION).read_bytes()
        ledger = (root/HISTORY).read_bytes()
        (root/'resources/compliance/review_baseline.json').unlink()
        with self.assertRaises(ValueError): governance.migrate(root)
        self.assertEqual(original, (root/PROJECTION).read_bytes())
        self.assertEqual(ledger, (root/HISTORY).read_bytes())


if __name__ == '__main__': unittest.main()
