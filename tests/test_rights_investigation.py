"""Synthetic fixtures only: proposal metadata never changes decisions or gates."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import unittest
from unittest import mock
import urllib.error

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import test_content_governance as fixtures
import content_governance as g
import rights_investigation as rights
import distribution


class InvestigationTests(unittest.TestCase):
    def setUp(self):
        self.fixture = fixtures.GovernanceTests(); self.fixture.setUp(); self.addCleanup(self.fixture.doCleanups)
        self.root = self.fixture.root; self.fixture.baseline(); g.generate_credits(self.root)
        g.save(self.root / 'docs/INVESTIGACAO_EVIDENCIAS.json', {'baseline_governance': {
            'photo_permission_findings': 2, 'other_asset_findings': 0, 'priority_scientific_blocks': []}})

    def observation(self):
        data = copy.deepcopy(self.fixture.observation)
        data.update(captive=True, quality_metrics=[{'metric': 'wild', 'agree': False, 'user_id': 999}],
            actor_type='human', scientific_review='approved_expert')
        return data

    def client(self, observation=None):
        return mock.Mock(get=mock.Mock(return_value={'results': [observation or self.observation()]}))

    def run_report(self, **kwargs):
        return rights.report(self.root, at='2026-10-17T12:00:00+00:00', **kwargs)

    def files(self):
        return {p.relative_to(self.root).as_posix(): p.read_bytes() for p in (self.root / 'resources').rglob('*') if p.is_file()}

    def test_absent_license_and_credit_are_reported_per_photo(self):
        species = g.load(self.root / 'resources/data/species.json'); species[0]['photo']['license'] = None; species[0]['photo']['author'] = ''
        self.fixture.write('species.json', species)
        row = self.run_report()['photos'][0]
        self.assertIn('license_missing', row['rights_issues'])
        self.assertIn('attribution_incomplete', row['rights_issues'])
        self.assertEqual(row['investigation_state'], 'pending_human_review')

    def test_observation_license_is_not_photo_permission_and_changes_do_not_revoke_old_grant(self):
        observation = self.observation(); observation['license_code'] = 'cc0'; observation['photos'][0]['license_code'] = 'all-rights-reserved'
        result = self.run_report(online=True, client=self.client(observation))
        row = result['photos'][0]
        self.assertEqual(row['declared_license'], 'cc-by')
        self.assertIn('current_license_requires_policy_review', row['rights_issues'])
        self.assertIn('current_license_changed_requires_analysis_not_automatic_revocation', row['rights_issues'])
        self.assertTrue(result['distribution_gate']['source_blocked'])
        self.assertEqual(g.load(self.root / 'resources/data/species.json')[0]['photo']['license'], 'cc-by')

    def test_missing_source_photo_license_and_private_credit_are_not_treated_as_permission(self):
        observation = self.observation(); observation['photos'][0].pop('license_code')
        row = self.run_report(online=True, client=self.client(observation))['photos'][0]
        self.assertIn('current_license_missing', row['rights_issues'])
        self.assertEqual(row['investigation_state'], 'pending_human_review')
        observation['photos'][0]['attribution'] = 'Private fixture <fixture@example.invalid>'
        result = self.run_report(online=True, client=self.client(observation), refresh=True)
        self.assertEqual(result['collection_calls'][0]['withheld_photo_ids'], ['7'])
        self.assertNotIn('fixture@example.invalid', json.dumps(result))

    def test_contradictory_provenance_and_source_credit_are_not_silently_fixed(self):
        provenance = g.load(self.root / 'resources/data/photo_provenance.json')
        provenance['7'].update(license='cc0', credit_as_supplied='Different declared credit')
        self.fixture.write('photo_provenance.json', provenance)
        observation = self.observation(); observation['photos'][0]['attribution'] = 'New photo-specific attribution'
        row = self.run_report(online=True, client=self.client(observation))['photos'][0]
        for code in ('provenance_license_disagrees', 'provenance_credit_disagrees', 'current_credit_differs_from_source'):
            self.assertIn(code, row['rights_issues'])
        self.assertEqual(row['author'], self.fixture.photo['attribution'])

    def test_research_grade_and_explicit_license_url_never_create_scientific_or_legal_approval(self):
        observation = self.observation(); observation['photos'][0]['license_url'] = 'https://creativecommons.org/licenses/by/4.0/'
        before = self.files(); result = self.run_report(online=True, client=self.client(observation))
        self.assertEqual(result['decisions_created'], 0); self.assertEqual(result['scientific_approvals_created'], 0)
        self.assertEqual(result['photos'][0]['investigation_state'], 'pending_human_review')
        self.assertEqual(result['photos'][0]['evidenced_license_version'], None)
        self.assertTrue(distribution.release_gate(self.root)['distribution_blocked'])
        self.assertEqual(self.files(), before)
        self.assertFalse((self.root / 'resources/compliance/permission_history.jsonl').exists())
        self.assertFalse((self.root / 'resources/compliance/review_history.jsonl').exists())

    def test_casual_captive_context_is_observation_evidence_not_taxon_reclassification(self):
        observation = self.observation(); observation['quality_grade'] = 'casual'
        before = self.files(); row = self.run_report(online=True, client=self.client(observation))['photos'][0]
        self.assertIn('current_observation_not_research_grade', row['scientific_issues'])
        self.assertEqual(row['priority'], 0)
        context = row['public_proposal']['snapshot']['observation_context']
        self.assertTrue(context['captive']); self.assertNotIn('user_id', json.dumps(context))
        self.assertEqual(self.files(), before)

    def test_missing_photo_or_incompatible_community_identification_stays_pending(self):
        observation = self.observation(); observation['photos'] = []; observation['community_taxon_id'] = 999
        row = self.run_report(online=True, client=self.client(observation))['photos'][0]
        self.assertIn('photo_not_associated_in_current_response', row['scientific_issues'])
        self.assertIn('current_community_taxon_disagrees', row['scientific_issues'])
        self.assertFalse(row['public_proposal']['snapshot']['photo_present'])
        self.assertEqual(row['investigation_state'], 'pending_human_review')

    def test_existing_update_comparator_detects_inactive_or_renamed_taxon_without_reclassification(self):
        observation = self.observation(); observation['taxon'].update(is_active=False, name='Synthetic revised name')
        before = self.files()
        row = self.run_report(online=True, client=self.client(observation))['photos'][0]
        self.assertIn('taxon_not_active', row['public_source_changes'])
        self.assertIn('current_possible_identification_or_taxonomy_change', row['scientific_issues'])
        self.assertEqual(row['organisms'][0]['name'], 'Example species')
        self.assertEqual(self.files(), before)

    def test_collected_evidence_is_minimal_hashed_and_no_raw_coordinates_or_observer_are_saved(self):
        result = self.run_report(online=True, client=self.client())
        proposal = result['photos'][0]['public_proposal']; snapshot = proposal['snapshot']
        path = self.root / proposal['path']
        self.assertEqual(g.digest(snapshot), path.stem)
        self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), proposal['sha256'])
        for private in ('private_location', 'coordinates', 'Different observer', 'user_id', 'approved_expert'):
            self.assertNotIn(private, json.dumps(snapshot))

    def test_offline_report_is_consistent_and_network_is_opt_in(self):
        with mock.patch.object(rights, 'PublicClient') as network:
            first = self.run_report(); second = self.run_report()
        network.assert_not_called(); self.assertEqual(first, second)
        self.assertEqual(first['historical_comparison']['photo_findings_then'], 2)
        self.assertEqual(first['historical_comparison']['photo_findings_now'], 1)
        self.assertFalse(first['dependencies']['assessed'])
        self.assertIsNone(first['photos'][0]['scope']['tracked_in_current_git'])

    def test_batched_collection_is_bounded_and_429_stops_without_evasion(self):
        species = [{'photo': {'id': str(i), 'source': f'https://www.inaturalist.org/observations/{i}'}} for i in range(1, 26)]
        client = mock.Mock(); client.get.return_value = {'results': []}
        calls = rights.collect_public(self.root, species, maximum=1, client=client)
        self.assertEqual(client.get.call_count, 1); self.assertEqual(len(calls[0]['observation_ids']), 10)
        client.get.side_effect = urllib.error.HTTPError('https://api.inaturalist.org/', 429, 'Fixture limit', {}, None)
        client.get.reset_mock(); calls = rights.collect_public(self.root, species, maximum=25, client=client)
        self.assertEqual(client.get.call_count, 1); self.assertEqual(calls[0]['http_status'], 429)

    def test_oversized_response_splits_batches_without_raising_transport_or_request_limits(self):
        species = [{'photo': {'id': str(i), 'source': f'https://www.inaturalist.org/observations/{i}'}} for i in range(1, 21)]
        client = mock.Mock(); client.get.side_effect = [ValueError('Synthetic byte limit'), {'results': []}]
        calls = rights.collect_public(self.root, species, maximum=2, client=client)
        self.assertEqual(client.get.call_count, 2)
        self.assertEqual(calls[0]['state'], 'response_rejected_by_transport')
        self.assertEqual(len(calls[1]['observation_ids']), 5)

    def test_cache_reuse_tampering_and_response_identity_fail_without_changing_governance(self):
        client = self.client(); self.run_report(online=True, client=client)
        self.run_report(online=True, client=client); self.assertEqual(client.get.call_count, 1)
        path = next((self.root / rights.EVIDENCE).glob('*.json')); path.write_text('{}', encoding='utf8')
        result = self.run_report(); self.assertTrue(result['staged_evidence_errors'])
        self.assertEqual(result['summary']['photos_with_staged_public_metadata'], 0)
        observation = self.observation(); observation['id'] = 999
        result = self.run_report(online=True, client=self.client(observation), refresh=True)
        self.assertEqual(result['collection_calls'][0]['state'], 'collection_failed')
        self.assertEqual(result['decisions_created'], 0)

    def test_package_scope_and_dependency_obligations_are_separate_from_source_catalog(self):
        package = self.root / 'package'; package.mkdir()
        (self.root / 'resources.qrc').write_text('<RCC><qresource><file>resources/images/7.jpg</file></qresource></RCC>', encoding='utf8')
        (package / 'unreviewed.dll').write_bytes(b'Synthetic dependency')
        g.save(package / 'compliance/dependencies.json', {'inventoried_at': '2020-01-01', 'packages': [
            {'name': 'synthetic-library', 'declared_licenses': ['LGPL-3.0'], 'obligations_review': 'pending'}], 'binaries': []})
        report = self.run_report(package=package)
        self.assertTrue(report['distribution_gate']['source_blocked']); self.assertTrue(report['distribution_gate']['package_blocked'])
        component = report['dependencies']['components'][0]
        self.assertEqual(component['priority'], 1)
        self.assertEqual(component['curation_request_id'], rights.request_id('licensing', 'dependencies'))
        self.assertIn('source_provision_not_documented', component['issues'])
        self.assertIn('dependency_obligations_unresolved', component['issues'])
        self.assertIn('dependency_not_inventoried', {e['error'] for e in report['dependencies']['errors']})
        self.assertTrue(report['photos'][0]['scope']['qrc_build_input'])
        self.assertEqual(report['photos'][0]['scope']['package_embedded_presence'], 'not_verified')


if __name__ == '__main__':
    unittest.main()
