"""Synthetic organisms/evidence only; never access the active catalog or players."""
import copy
import hashlib
from pathlib import Path
import shutil
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import test_content_governance as fixtures
import content_governance as g
import build_education
import phylogeny
from scientific_sources import admission
from scientific_sources.audit import staging_errors
from scientific_sources.model import Assertion, Identity, digest
from scientific_sources.store import CandidateStore

PROJECT = Path(__file__).resolve().parents[1]


class AdmissionTests(unittest.TestCase):
    def setUp(self):
        f = fixtures.GovernanceTests(); f.setUp(); self.addCleanup(f.doCleanups)
        f.baseline(); self.base = f.root; self.root = self.base / admission.DRAFTS / 'synthetic'
        shutil.copytree(self.base / 'resources', self.root / 'resources'); f.root = self.root; self.f = f
        self.s = copy.deepcopy(f.species)
        self.s.update(id=2, name='Example novum', source='inaturalist', source_taxon_id='23',
                      source_taxonomy=[{'id': 'inat:1', 'rank': 'kingdom', 'name': 'Animalia', 'parent_id': ''},
                                       {'id': 'inat:21', 'rank': 'genus', 'name': 'Example', 'parent_id': 'inat:1'},
                                       {'id': 'inat:23', 'rank': 'species', 'name': 'Example novum', 'parent_id': 'inat:21'}])
        image = self.root / 'resources/images/8.jpg'; image.parent.mkdir(parents=True); image.write_bytes(b'SYNTHETIC IMAGE ONLY')
        self.s['photo'].update(id='8', taxon_id='23', scientific_name=self.s['name'], path='resources/images/8.jpg',
                               sha256=hashlib.sha256(image.read_bytes()).hexdigest(), source='https://www.inaturalist.org/observations/12',
                               url='https://inaturalist-open-data.s3.amazonaws.com/photos/8/square.jpg')
        policy = g.load(PROJECT / 'resources/data/phylogeny.json'); f.write('phylogeny.json', policy)
        organisms, nodes = phylogeny.project([self.s], [], policy); self.s = organisms[0]
        f.write('species.json', organisms); f.write('taxonomy.json', nodes)
        entry = {'scientific_name': self.s['name'], 'common_name': 'Synthetic organism'}; notes = {}
        for topic in ('summary', 'distribution', 'morphology', 'conservation', 'curiosity'):
            ref = 'species:2:' + topic; entry[topic] = ref
            notes[ref] = dict(f.note, text='Synthetic ' + topic, source_locator='Synthetic section ' + topic)
        profile = {key: entry[key] for key in ('scientific_name', 'common_name')}
        profile.update(display_name='Synthetic organism', reviewed_on=None)
        for topic in ('summary', 'distribution', 'morphology', 'conservation', 'curiosity'):
            profile[topic] = notes['species:2:' + topic]
        profile['conservation']['status'] = 'unconfirmed'
        profile['hint'] = dict(f.note, text='Synthetic learning clue')
        f.write('species_profiles.json', {'2': profile})
        f.write('hint_education.json', {'2': {'scientific_name': self.s['name']}})
        f.write('curation_reviews.json', {'2': {'scientific_name': self.s['name'], **{t: 'pending' for t in g.TOPICS}}})
        f.write('education.json', {'species': {'2': entry}, 'notes': notes})
        f.write('photo_provenance.json', {})
        obs = copy.deepcopy(f.observation); obs.update(id=12, community_taxon_id=23)
        obs['taxon'].update(id=23, name=self.s['name'])
        photo = dict(f.photo, id=8, url=self.s['photo']['url']); obs['photos'] = [photo]
        self.photo_evidence = g.archive_permission(self.root, obs, photo, 'live_api', g.now())
        evidence = g.load(self.root / self.photo_evidence)
        f.write('photo_provenance.json', {'8': {'photo_id': '8', 'observation_id': '12',
            'download_url': self.s['photo']['url'], 'sha256': self.s['photo']['sha256'],
            'declared_license_url': self.s['photo']['license_url'], 'evidence_paths': [self.photo_evidence],
            'first_evidenced_at': evidence['collected_at']}})
        proof = self.root / 'resources/compliance/synthetic-license.txt'
        proof.parent.mkdir(parents=True, exist_ok=True); proof.write_text('SYNTHETIC CC BY 4.0 proof. Not a real permission.', encoding='utf8')
        self.proof = {'path': proof.relative_to(self.root).as_posix(), 'sha256': admission.file_hash(proof),
                      'source_url': 'https://example.org/synthetic-license', 'locator': 'Synthetic license only'}
        snap = {'schema_version': 1, 'url': 'https://example.org/taxon/23', 'retrieved_at': g.now(),
                'payload': {'id': '23', 'name': self.s['name']}}
        # Like the real transport: original HTTP bytes differ from the selected
        # payload. Admission verifies the snapshot/provenance, not a false equality.
        snap['response_sha256'] = hashlib.sha256(b'{"full_response":"synthetic"}').hexdigest()
        snap_path = 'resources/compliance/sources/' + digest(snap) + '.json'
        g.save(self.root / snap_path, snap)
        binding = {'snapshot_sha256': digest(snap), 'url': snap['url'], 'retrieved_at': snap['retrieved_at'],
                   'response_sha256': snap['response_sha256']}
        assertion = Assertion(Identity('inaturalist', '23'), self.s['name'], self.s['name'], 'species', 'accepted',
                              lineage=self.s['source_taxonomy'], provenance=[binding])
        store = CandidateStore(self.base / 'build/scientific-sources/candidates.sqlite')
        try: store.put_many([assertion]); candidate = store.latest()[0]
        finally: store.close()
        self.dossier = {'schema_version': 1, 'species_id': 2, 'candidate': candidate, 'candidate_sha256': digest(candidate),
            'base_inputs': admission.base_inputs(self.base),
            'provider_evidence': [{'path': snap_path, 'sha256': admission.file_hash(self.root / snap_path),
                'source_url': snap['url'], 'locator': '/payload', 'identity_locator': '/id', 'name_locator': '/name'}],
            'source_use': {layer: {'license': 'synthetic-test-license', 'citation': 'Synthetic source',
                'scopes': ['offline_catalog', 'public_repository'],
                'compatibility_rationale': 'Fixture only; no real grant or approval.', 'evidence': self.proof}
                for layer in ('taxonomy_data', 'educational_text')},
            'unavailable_content': {key: 'Not supplied in this synthetic fixture.' for key in
                                    ('etymology_hints', 'morphological_hint', 'taxonomic_assistance', 'hints')}}
        self.write_dossier()
        self.regenerate()

    def write_dossier(self): g.save(self.root / 'resources/compliance/admissions/2.json', self.dossier)

    def check(self):
        return admission.assess(self.root, '2', g.load(self.base / 'resources/data/species.json'))

    def codes(self): return {row['error'] for row in self.check()['errors']}

    def permission(self):
        p = self.s['photo']; meta = g.load(self.root / 'resources/data/photo_provenance.json')['8']
        decision = {'photo_id': '8', 'license_version': '4.0', 'evidence_paths': [self.photo_evidence],
            'reviewer': 'synthetic-fixture-only', 'actor_type': 'human', 'reviewed_at': g.now(),
            'rationale': 'Synthetic test input, never a real human decision.', 'license_version_evidence': self.proof}
        decision['content_sha256'] = digest({'photo_id': '8', 'sha256': p['sha256'], 'license': p['license'],
            'license_url': p['license_url'], 'license_version': '4.0', 'evidence_paths': meta['evidence_paths']})
        g.confirm_permission(self.root, decision)

    def reviews(self):
        for key, record in g.records(self.root).items():
            decision = self.f.decision(key); decision['reviewer'] = 'synthetic-fixture-only'
            decision['rationale'] = 'Synthetic test review, never production curation.'
            if record['kind'] == 'assistance': decision['claim_scope'] = 'hint_assistance'
            g.append_review(self.root, key, decision)

    def regenerate(self):
        g.save(self.root / 'resources/data/education.json', build_education.build(self.root, persist=False))

    def complete(self):
        self.permission(); self.reviews(); self.regenerate()

    def test_incomplete_candidate_never_creates_approvals(self):
        before = {p: p.read_bytes() for p in (self.root / 'resources').rglob('*') if p.is_file()}
        self.assertIn('human_review_required', self.codes())
        self.assertFalse(self.check()['ready_for_proposal'])
        self.assertEqual(self.check()['approvals_created'], 0)
        self.assertEqual(before, {p: p.read_bytes() for p in before})
        self.dossier.pop('candidate'); self.write_dossier()
        self.assertIn('admission_incomplete_or_invalid', self.codes())

    def test_duplicate_external_identity_and_name_are_blocked(self):
        for existing in (dict(self.f.species, source='inaturalist', source_taxon_id='23'),
                         dict(self.f.species, name='EXAMPLE NOVUM')):
            result = admission.assess(self.root, '2', [existing])
            self.assertIn('possible_duplicate_requires_resolution', {r['error'] for r in result['errors']})

    def test_taxon_without_evidence_or_with_wrong_identity_is_blocked(self):
        self.dossier['provider_evidence'] = []; self.write_dossier()
        self.assertIn('taxonomy_evidence_missing', self.codes())
        self.dossier['candidate']['assertion']['canonical_name'] = 'Other species'; self.write_dossier()
        self.assertIn('scientific_identity_diverges', self.codes())
        self.assertIn('candidate_integrity_or_state', self.codes())

    def test_photo_license_credit_and_divergent_identity_are_blocked(self):
        for change, expected in (({'license': None}, 'photo_license_missing'),
                                 ({'license': 'cc-by-sa'}, 'license_outside_policy'),
                                 ({'author': ''}, 'photo_credit_missing'),
                                 ({'scientific_name': 'Other species'}, 'photo_scientific_identity_diverges'),
                                 ({'taxon_id': '999'}, 'photo_taxon_diverges')):
            with self.subTest(change=change):
                organism = copy.deepcopy(self.s); organism['photo'].update(change)
                self.f.write('species.json', [organism]); self.assertIn(expected, self.codes())
        self.f.write('species.json', [self.s])

    def test_casual_is_policy_failure_not_automatically_wrong_identity(self):
        snap = g.load(self.root / self.photo_evidence); snap['record']['quality_grade'] = 'casual'
        relative = 'resources/compliance/permissions/' + digest(snap) + '.json'; g.save(self.root / relative, snap)
        meta = g.load(self.root / 'resources/data/photo_provenance.json'); meta['8']['evidence_paths'] = [relative]
        self.f.write('photo_provenance.json', meta)
        self.assertIn('observation_does_not_meet_import_policy', self.codes())
        self.assertNotIn('photo_taxon_diverges', self.codes())

    def test_legitimate_proposal_is_idempotent_and_preserves_active_bytes_and_events(self):
        self.complete(); before = {p: p.read_bytes() for p in (self.base / 'resources').rglob('*') if p.is_file()}
        first = admission.prepare(self.base, self.root, '2'); self.assertTrue(first['ready_for_proposal'], first)
        path = self.base / first['proposal']; data = path.read_bytes()
        self.assertEqual(first, admission.prepare(self.base, self.root, '2'))
        self.assertEqual(path.read_bytes(), data)
        self.assertEqual(before, {p: p.read_bytes() for p in before})
        self.assertEqual(len(g.read_ledger(self.root)), len(g.records(self.root)))
        self.assertFalse(first['active_catalog_modified']); self.assertEqual(first['approvals_created'], 0)
        self.assertNotIn('resources/compliance/.governance.lock', g.load(path)['workspace_files'])

    def test_permission_cannot_be_forged_by_copying_confirmation_without_events(self):
        self.complete(); path = self.root / 'resources/compliance/permission_history.jsonl'
        path.unlink()
        self.assertIn('admission_incomplete_or_invalid', self.codes())

    def test_evidence_and_content_changes_require_new_review(self):
        self.complete(); self.assertTrue(self.check()['ready_for_proposal'], self.check())
        (self.root / self.proof['path']).write_text('Changed synthetic evidence', encoding='utf8')
        self.assertFalse(self.check()['ready_for_proposal'])
        record = g.records(self.root)['admission:2']
        self.assertEqual(g.status('admission:2', record, {}, g.read_ledger(self.root), g.policy(self.root))['state'], 'needs_review')

    def test_changed_base_or_replaced_event_prefix_prevents_preparation(self):
        self.complete(); g.save(self.base / 'resources/data/catalog_manifest.json', {'version': 'synthetic-change'})
        with self.assertRaisesRegex(ValueError, 'base changed'): admission.prepare(self.base, self.root, '2')
        self.dossier['base_inputs'] = admission.base_inputs(self.base); self.write_dossier()
        decision = self.f.decision('species:2:summary')
        decision['content_sha256'] = g.records(self.base)['species:1:summary']['content_sha256']
        g.append_review(self.base, 'species:1:summary', decision)
        self.dossier['base_inputs'] = admission.base_inputs(self.base); self.write_dossier()
        with self.assertRaisesRegex(ValueError, 'prefix'): admission.prepare(self.base, self.root, '2')

    def test_manual_copy_without_dossier_is_blocked_and_legacy_is_preserved(self):
        self.assertEqual(staging_errors(self.base), [])
        path = self.root / 'resources/compliance/admissions/2.json'; path.unlink()
        self.assertIn('admission_incomplete_or_invalid', {e['error'] for e in staging_errors(self.root)})

    def test_workspace_cannot_target_active_or_escape_draft_directory(self):
        for value in (self.base, self.base / admission.DRAFTS, self.base / 'outside'):
            with self.assertRaises(ValueError): admission.workspace(self.base, value)
        self.assertEqual(admission.workspace(self.base, self.root), self.root)

    def test_template_is_idempotent_and_never_overwrites_prepared_work(self):
        path = admission.template(self.base, self.root, self.dossier['candidate'])
        before = path.read_bytes()
        self.assertEqual(admission.template(self.base, self.root, self.dossier['candidate']), path)
        self.assertEqual(before, path.read_bytes())
        g.save(path, {'edited': True})
        with self.assertRaisesRegex(ValueError, 'differs'):
            admission.template(self.base, self.root, self.dossier['candidate'])

    def test_pending_or_rejected_human_decisions_remain_blocking(self):
        self.complete()
        key = 'topic:2:taxonomy'
        for state in ('pending', 'rejected'):
            decision = self.f.decision(key); decision['state'] = state
            g.append_review(self.root, key, decision)
            result = self.check()
            self.assertFalse(result['ready_for_proposal'])
            self.assertEqual(result['reviews'][key]['state'], state)

    def test_public_use_scope_and_derived_file_edits_do_not_suffice(self):
        self.complete()
        self.dossier['source_use']['educational_text']['scopes'] = ['offline_catalog']; self.write_dossier()
        self.assertIn('data_text_redistribution_not_documented', self.codes())
        education = g.load(self.root / 'resources/data/education.json')
        education['notes']['species:2:summary']['text'] = 'Changed derived content only'
        self.f.write('education.json', education)
        self.assertIn('education_not_regenerated_from_editable_sources', self.codes())

    def test_qualified_proposal_satisfies_new_entry_gate_but_never_releases_legacy_material(self):
        self.complete()
        self.assertEqual(staging_errors(self.root), [])
        # Real publication still requires the separate complete distribution gate.
        result = admission.prepare(self.base, self.root, '2')
        plan = g.load(self.base / result['proposal'])
        self.assertEqual(plan['state'], 'candidates_only')
        self.assertEqual(g.load(self.base / 'resources/data/species.json')[0]['id'], 1)

    def test_interrupted_export_can_be_retried_without_partial_proposal(self):
        self.complete()
        with mock.patch.object(g, 'atomic_write', side_effect=OSError('Synthetic write interruption')):
            with self.assertRaises(OSError): admission.prepare(self.base, self.root, '2')
        directory = self.base / 'build/scientific-sources/admissions'
        self.assertFalse(list(directory.glob('*.json')))
        self.assertTrue(admission.prepare(self.base, self.root, '2')['ready_for_proposal'])

    def test_concurrent_content_change_or_private_db_prevents_export(self):
        self.complete(); original = admission.assess
        def changed(*args, **kwargs):
            result = original(*args, **kwargs)
            (self.root / self.proof['path']).write_text('Changed during assessment', encoding='utf8')
            return result
        with mock.patch.object(admission, 'assess', side_effect=changed):
            with self.assertRaisesRegex(ValueError, 'during validation'): admission.prepare(self.base, self.root, '2')
        (self.root / 'resources/player.sqlite').write_bytes(b'Synthetic private DB')
        with self.assertRaisesRegex(ValueError, 'artifact'): admission.prepare(self.base, self.root, '2')


if __name__ == '__main__': unittest.main()
