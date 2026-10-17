"""Synthetic local Git repositories; no network, real reviews, keys or credentials."""
import copy
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import governance_history as history
import governance_storage as storage


class HistoryTrustTests(unittest.TestCase):
    def setUp(self):
        folder = tempfile.TemporaryDirectory(); self.addCleanup(folder.cleanup)
        self.root = Path(folder.name)
        self.git('init', '-q')
        self.baseline = self.root / history.BASELINE
        self.baseline.parent.mkdir(parents=True)
        self.baseline.write_bytes(b'{"fixture": "synthetic legacy baseline, not an approval"}\n')
        for ledger in sorted(storage.LEDGERS):
            storage.append_event(self.root, ledger, {'fixture': True, 'value': 'original'}, '')
        self.base = self.commit()

    def git(self, *args):
        environment = dict(os.environ, GIT_CONFIG_NOSYSTEM='1', GIT_CONFIG_GLOBAL=os.devnull)
        return subprocess.check_output(['git', '--no-replace-objects', '-C', str(self.root),
            '-c', 'core.hooksPath=', '-c', 'commit.gpgsign=false', '-c', 'core.autocrlf=false',
            '-c', 'user.name=Synthetic test fixture', '-c', 'user.email=fixture@example.invalid',
            *args], env=environment, stderr=subprocess.DEVNULL).decode('utf8').strip()

    def commit(self, message='Synthetic fixture, not a real curation decision'):
        self.git('add', '--', *history.PATHS)
        self.git('commit', '-q', '--allow-empty', '-m', message)
        return self.git('rev-parse', 'HEAD')

    def ledger(self, name='review_history.jsonl'):
        return self.root / history.PREFIX / name

    def rewrite(self, path, events):
        previous = ''; lines = []
        for original in copy.deepcopy(events):
            original.pop('event_sha256', None); original['previous_sha256'] = previous
            original['event_sha256'] = storage.digest(original)
            previous = original['event_sha256']; lines.append(json.dumps(original))
        path.write_text('\n'.join(lines) + '\n', encoding='utf8', newline='\n')

    def test_append_only_changes_preserve_all_three_ledger_prefixes(self):
        for ledger in sorted(storage.LEDGERS):
            events = storage.read_chain(self.ledger(ledger))
            storage.append_event(self.root, ledger, {'fixture': True, 'value': 'next'}, events[-1]['event_sha256'])
        self.commit()
        report = history.audit_history(self.root, self.base)
        self.assertTrue(report['consistent'], report)
        self.assertEqual(report['comparison']['status'], 'prefix_preserved')
        for ledger in storage.LEDGERS:
            item = report['files'][history.PREFIX + ledger]
            self.assertEqual(item['current']['events'], 2)
            self.assertEqual(item['reference']['events'], 1)

    def test_complete_rehash_passes_internal_integrity_but_fails_retained_reference(self):
        path = self.ledger(); events = storage.read_chain(path)
        events[0].update(reviewer='Synthetic declared reviewer', actor_type='human', value='replaced')
        self.rewrite(path, events)
        self.assertEqual(len(storage.read_chain(path)), 1)  # Expected limit, not a SHA-256 failure.
        self.assertTrue(history.audit_history(self.root)['consistent'])
        report = history.audit_history(self.root, self.base)
        self.assertFalse(report['consistent'])
        self.assertEqual(report['errors'][0]['error'], 'historical_prefix_changed_or_removed')
        self.assertFalse(report['assurances']['authorship_authenticated'])
        self.assertFalse(report['assurances']['specialist_qualification_verified'])
        self.assertNotIn('Synthetic declared reviewer', json.dumps(report))

    def test_mutation_without_rehash_is_rejected_without_exporting_event_body(self):
        path = self.ledger()
        path.write_bytes(path.read_bytes().replace(b'original', b'PRIVATE synthetic payload'))
        report = history.audit_history(self.root, self.base)
        self.assertFalse(report['consistent'])
        self.assertNotIn('PRIVATE', json.dumps(report))
        self.assertEqual(report['errors'][0]['error'], 'invalid_or_unreadable_history')

    def test_truncation_and_removal_cannot_discard_retained_events(self):
        for name in storage.LEDGERS:
            path = self.ledger(name); before = path.read_bytes()
            with self.subTest(ledger=name):
                path.write_bytes(b'')
                self.assertFalse(history.audit_history(self.root, self.base)['consistent'])
                path.unlink()
                self.assertFalse(history.audit_history(self.root, self.base)['consistent'])
                path.write_bytes(before)

    def test_empty_ledger_removal_and_legacy_baseline_change_are_detected(self):
        path = self.ledger(); path.write_bytes(b''); base = self.commit(); path.unlink()
        self.assertFalse(history.audit_history(self.root, base)['consistent'])
        path.write_bytes(b''); self.baseline.write_bytes(b'{"changed": true}\n')
        report = history.audit_history(self.root, base)
        self.assertFalse(report['consistent'])
        self.assertFalse(report['files'][history.BASELINE]['reference_preserved'])

    def test_invalid_missing_mutable_and_noncommit_references_fail_closed(self):
        for base in ('HEAD', 'main', '0', 'f' * 40, self.git('rev-parse', 'HEAD^{tree}')):
            with self.subTest(base=base):
                self.assertFalse(history.audit_history(self.root, base)['consistent'])

    def test_unrelated_history_is_not_accepted_as_a_reference(self):
        self.git('checkout', '--orphan', 'unrelated-fixture')
        self.commit('Unrelated synthetic fixture, not a real curation decision')
        self.assertFalse(history.audit_history(self.root, self.base)['consistent'])

    def test_integrity_only_and_initial_push_never_claim_historical_authentication(self):
        for base, expected in [(None, 'not_requested'), ('', 'not_requested'), ('0' * 40, 'unavailable_initial_push')]:
            with self.subTest(base=base):
                report = history.audit_history(self.root, base)
                self.assertTrue(report['consistent'])
                self.assertEqual(report['comparison']['status'], expected)
                for key in ('authorship_authenticated', 'digital_signatures_verified', 'independent_review_verified'):
                    self.assertIs(report['assurances'][key], False)

    def test_snapshot_is_minimal_and_does_not_change_existing_records(self):
        before = {p: (self.root / p).read_bytes() for p in history.PATHS}
        first = history.audit_history(self.root, self.base)
        self.assertEqual(first, history.audit_history(self.root, self.base))
        self.assertEqual(before, {p: (self.root / p).read_bytes() for p in history.PATHS})
        for name, data in before.items():
            self.assertEqual(first['files'][name]['current']['bytes_sha256'], hashlib.sha256(data).hexdigest())
        self.assertNotIn(str(self.root), json.dumps(first))
        self.assertNotIn('original', json.dumps(first))

    def test_source_copy_without_git_or_ledgers_remains_offline_and_explicit(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            report = history.audit_history(root)
            self.assertTrue(report['consistent'])
            self.assertTrue(all(not entry['current']['present'] for entry in report['files'].values()))
            self.assertFalse(history.audit_history(root, self.base)['consistent'])
            self.assertFalse((root / '.git').exists())

    def test_git_replacement_refs_do_not_substitute_the_pinned_commit(self):
        events = storage.read_chain(self.ledger()); events[0]['value'] = 'replacement'
        self.rewrite(self.ledger(), events)
        other = self.commit()
        self.git('replace', self.base, other)
        self.assertFalse(history.audit_history(self.root, self.base)['consistent'])


if __name__ == '__main__':
    unittest.main()
