"""Synthetic fixtures only: proposal metadata never changes decisions or gates."""
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import subprocess
import unittest
from unittest import mock
import urllib.error

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import test_content_governance as fixtures
import content_governance as g
import rights_investigation as rights
import distribution
import licensing_inventory


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

    def test_educational_inventory_deduplicates_text_without_merging_source_bindings(self):
        self.fixture.write('species_profiles.json', {'1': {'summary': {
            'text': 'Shared synthetic fact.', 'source_url': 'https://example.org/a'}},
            '2': {'summary': {'text': 'Shared synthetic fact.', 'source_url': 'https://example.org/b'}}})
        before = self.files()
        result = rights.educational_text_inventory(self.root)
        self.assertEqual(result['unique_passages'], 1)
        self.assertEqual(result['total_occurrences'], 2)
        row = result['passages'][0]
        self.assertEqual({o['source_url'] for o in row['occurrences']}, {'https://example.org/a', 'https://example.org/b'})
        self.assertEqual(row['originality_state'], 'inconclusive')
        self.assertFalse(row['permission_created'])
        self.assertEqual(self.files(), before)
        self.assertEqual(result, rights.educational_text_inventory(self.root))

    def test_inventory_covers_unused_group_definitions_notes_and_assistance_rationales(self):
        self.fixture.write('group_education.json', {'definitions': [
            ['Unused', 'genus', 'https://example.org/group', 'Source', 'Intro.', 'Hint.', 'Fact.']],
            'plain_notes': {'Unused': ['Alternative hint.', 'Alternative fact.']}})
        self.fixture.write('hint_education.json', {'1': {'etymology_hints': {'species': {
            'text': 'Name origin.', 'source_url': 'https://example.org/name',
            'assistance': {'rationale': 'Synthetic pedagogical rationale.'}}}}})
        result = rights.educational_text_inventory(self.root)
        self.assertEqual(result['definition_count'], 1)
        self.assertEqual(result['plain_note_sets'], 1)
        self.assertEqual(result['total_occurrences'], 7)
        self.assertEqual(result['files']['group_education.json']['passage_occurrences'], 5)
        self.assertEqual(result['decisions_created'], 0)

    def test_inventory_checks_generated_projection_without_renewing_or_approving_it(self):
        import review_hints
        self.fixture.write('species_profiles.json', {'1': {'summary': self.fixture.note}})
        self.fixture.write('hint_education.json', {'1': {}})
        generated = review_hints.prepare(self.root)
        generated['evaluation'] = {'state': 'approved_automated', 'reviewed_at': 'synthetic unchanged date'}
        self.fixture.write('progressive_hints.json', generated)
        before = self.files()
        with mock.patch.object(review_hints, 'evaluate', side_effect=AssertionError('Must not evaluate')):
            result = rights.educational_text_inventory(self.root)
            self.assertTrue(result['projection']['matches_generator'])
            self.assertEqual(self.files(), before)
            generated['species']['1']['recipes'].append({'id': 'tampered', 'text': 'Unbound claim.'})
            self.fixture.write('progressive_hints.json', generated)
            result = rights.educational_text_inventory(self.root)
            self.assertFalse(result['projection']['matches_generator'])
        self.assertEqual(result['decisions_created'], 0)

    def test_observation_license_is_not_photo_permission_and_changes_do_not_revoke_old_grant(self):
        observation = self.observation(); observation['license_code'] = 'cc0'; observation['photos'][0]['license_code'] = 'all-rights-reserved'
        result = self.run_report(online=True, client=self.client(observation))
        row = result['photos'][0]
        self.assertEqual(row['declared_license'], 'cc-by')
        self.assertIn('current_license_requires_policy_review', row['rights_issues'])
        self.assertIn('current_license_changed_requires_analysis_not_automatic_revocation', row['rights_issues'])
        self.assertTrue(result['distribution_gate']['source_blocked'])
        self.assertEqual(g.load(self.root / 'resources/data/species.json')[0]['photo']['license'], 'cc-by')

    def editorial_policy_fixture(self):
        actual = g.policy()
        policy = g.policy(self.root)
        policy['editorial_text_policy'] = copy.deepcopy(actual['editorial_text_policy'])
        decision = copy.deepcopy(next(d for d in actual['maintainer_decisions']
            if d['id'] == policy['editorial_text_policy']['decision_id']))
        policy['maintainer_decisions'] = [d for d in policy.get('maintainer_decisions', [])
            if d['id'] != decision['id']] + [decision]
        g.save(self.root / 'resources/data/content_policy.json', policy)
        return policy

    def text_rights_fixture(self, origin='inconclusive', source_license=None):
        self.fixture.write('species_profiles.json', {'1': {'summary': {
            'text': 'Synthetic educational passage.', 'source_url': 'https://example.org/scientific-evidence'}}})
        record = {'file': 'resources/data/species_profiles.json', 'pointer': '/1/summary/text',
            'text_sha256': hashlib.sha256(b'Synthetic educational passage.').hexdigest(),
            'origin_kind': origin, 'declared_origin': 'Synthetic fixture only.',
            'source_text': {'url': 'https://example.org/text', 'license': source_license},
            'declared_license': None, 'transformations': [], 'rights_available': None,
            'evidence': [], 'investigation_state': 'pending_human_review',
            'editorial_review_key': None, 'attribution': {}}
        self.write_text_rights([record])
        return record

    def write_text_rights(self, records):
        assets = g.load(self.root / 'resources/compliance/asset_rights.json', {})
        assets['educational_text_rights'] = records
        g.save(self.root / 'resources/compliance/asset_rights.json', assets)

    def test_editorial_default_never_licenses_inconclusive_or_external_prose(self):
        record = self.text_rights_fixture()
        self.editorial_policy_fixture()
        before = self.files(); gate = distribution.release_gate(self.root)
        with mock.patch.object(g, 'confirm_permission', side_effect=AssertionError('No permission')), \
             mock.patch.object(g, 'append_review', side_effect=AssertionError('No review')):
            result = rights.educational_text_inventory(self.root)
        metadata = result['rights_metadata']
        self.assertEqual(metadata['technical_errors'], [])
        self.assertIn('origin_inconclusive', metadata['records'][0]['rights_issues'])
        self.assertIsNone(metadata['records'][0]['declared_metadata']['declared_license'])
        self.assertFalse(result['passages'][0]['permission_created'])
        self.assertEqual(metadata['grants_created'], 0)
        self.assertEqual(before, self.files())
        self.assertEqual(gate, distribution.release_gate(self.root))
        record['origin_kind'] = 'external_reproduction'; self.write_text_rights([record])
        self.assertIsNone(rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
            ['declared_metadata']['declared_license'])

    def test_editorial_default_cannot_replace_sharealike_translation_license(self):
        record = self.text_rights_fixture('translation', 'CC-BY-SA-4.0')
        record.update(declared_license='CC-BY-4.0', transformations=['translation'])
        self.write_text_rights([record])
        row = rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
        self.assertIn('sharealike_cannot_be_replaced_by_default', row['rights_issues'])
        record['source_text']['license'] = 'cc-by-sa'; self.write_text_rights([record])
        row = rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
        self.assertIn('sharealike_cannot_be_replaced_by_default', row['rights_issues'])
        self.assertEqual(row['declared_metadata']['source_text']['license'], 'cc-by-sa')
        record['declared_license'] = 'CC-BY-SA-4.0'; self.write_text_rights([record])
        row = rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
        self.assertNotIn('sharealike_cannot_be_replaced_by_default', row['rights_issues'])
        self.assertFalse(row['permission_created'])

    def test_noncommercial_and_no_derivatives_need_specific_review(self):
        record = self.text_rights_fixture('adaptation', 'CC-BY-NC-ND-4.0')
        record['transformations'] = ['adaptation']; self.write_text_rights([record])
        row = rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
        self.assertIn('noncommercial_conditions_require_review', row['rights_issues'])
        self.assertIn('no_derivatives_conditions_require_review', row['rights_issues'])
        self.assertFalse(row['permission_created'])
        record.update(origin_kind='external_reproduction', transformations=[])
        self.write_text_rights([record])
        row = rights.educational_text_inventory(self.root)['rights_metadata']['records'][0]
        self.assertNotIn('no_derivatives_conditions_require_review', row['rights_issues'])
        self.assertFalse(row['permission_created'])

    def test_text_rights_bind_location_and_hash_and_preserve_scientific_source(self):
        record = self.text_rights_fixture('original')
        self.fixture.write('species_profiles.json', {'1': {'summary': {
            'text': 'Synthetic educational passage.', 'source_url': 'https://example.org/scientific-evidence'}},
            '2': {'summary': {'text': 'Synthetic educational passage.', 'source_url': 'https://example.org/another'}}})
        result = rights.educational_text_inventory(self.root)
        self.assertEqual(result['unique_passages'], 1)
        self.assertEqual(result['total_occurrences'], 2)
        self.assertEqual(len(result['rights_metadata']['records']), 1)
        self.assertEqual(result['rights_metadata']['records'][0]['binding']['pointer'], '/1/summary/text')
        self.assertNotEqual(record['source_text']['url'], result['passages'][0]['occurrences'][0]['source_url'])
        record['text_sha256'] = '0' * 64; self.write_text_rights([record, record])
        errors = rights.educational_text_inventory(self.root)['rights_metadata']['technical_errors']
        self.assertIn('text_rights:0:stale_or_unknown_occurrence', errors)
        self.assertIn('text_rights:1:duplicate_occurrence', errors)

    def test_evidence_hashes_are_checked_without_making_approval(self):
        record = self.text_rights_fixture('original')
        path = self.root / 'resources/compliance/synthetic-text-evidence.json'
        g.save(path, {'synthetic': True})
        record.update(evidence=[{'path': path.relative_to(self.root).as_posix(),
            'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'locator': 'Synthetic record'}],
            rights_available='Synthetic test statement; not a real grant.', declared_license='CC-BY-4.0')
        self.write_text_rights([record]); before = self.files()
        result = rights.educational_text_inventory(self.root)['rights_metadata']
        self.assertEqual(result['technical_errors'], [])
        self.assertFalse(result['records'][0]['permission_created'])
        self.assertEqual(before, self.files())
        g.save(path, {'synthetic': 'modified'})
        self.assertIn('text_rights:0:invalid_evidence',
            rights.educational_text_inventory(self.root)['rights_metadata']['technical_errors'])

    def test_documentary_metadata_cannot_smuggle_approval_or_malformed_fields(self):
        record = self.text_rights_fixture(); record['distribution_allowed'] = True
        self.write_text_rights([record])
        self.assertIn('text_rights:0:invalid_fields',
            rights.educational_text_inventory(self.root)['rights_metadata']['technical_errors'])
        record.pop('distribution_allowed'); record['origin_kind'] = []
        self.write_text_rights([record])
        self.assertIn('text_rights:0:invalid_origin_kind',
            rights.educational_text_inventory(self.root)['rights_metadata']['technical_errors'])

    def test_editorial_policy_is_checked_by_existing_code_license_command(self):
        self.code_license_fixture(); policy = self.editorial_policy_fixture()
        self.assertEqual(self.code_errors(), [])
        without_default = copy.deepcopy(policy); without_default.pop('editorial_text_policy')
        g.save(self.root / 'resources/data/content_policy.json', without_default)
        self.assertIn('editorial_default_missing', self.code_errors())
        policy['editorial_text_policy']['automatic_grants'] = True
        g.save(self.root / 'resources/data/content_policy.json', policy)
        self.assertIn('editorial_default_invalid_or_overbroad', self.code_errors())
        policy['editorial_text_policy']['automatic_grants'] = False
        policy['own_material_licenses']['texts'] = 'CC-BY-4.0'
        g.save(self.root / 'resources/data/content_policy.json', policy)
        self.assertIn('editorial_default_is_not_a_corpus_grant', self.code_errors())
        policy['own_material_licenses']['texts'] = None; policy['maintainer_decisions'] = []
        g.save(self.root / 'resources/data/content_policy.json', policy)
        self.assertIn('editorial_default_decision_not_linked', self.code_errors())

    def test_grouped_rights_keep_all_bindings_without_merging_distinct_contexts(self):
        record = self.text_rights_fixture()
        self.fixture.write('species_profiles.json', {'1': {'summary': {'text': 'Synthetic educational passage.'}},
            '2': {'summary': {'text': 'Synthetic educational passage.'}}})
        binding = {k: record.pop(k) for k in ('file','pointer','text_sha256')}
        other = dict(binding, pointer='/2/summary/text')
        record['occurrences'] = [binding, other]; self.write_text_rights([record])
        inv = rights.educational_text_inventory(self.root)
        self.assertEqual(inv['rights_metadata']['technical_errors'], [])
        self.assertEqual(len(inv['rights_metadata']['records']), 1)
        self.assertEqual(inv['rights_metadata']['records'][0]['bindings'], [binding, other])
        self.assertNotIn('occurrences', inv['rights_metadata']['records'][0]['declared_metadata'])
        self.assertIn('text_component_rights_pending', rights.text_resource_issues(inv, binding['file']))
        record['occurrences'].append(binding); self.write_text_rights([record])
        self.assertIn('text_rights:0:duplicate_occurrence',
            rights.educational_text_inventory(self.root)['rights_metadata']['technical_errors'])

    def documented_text_fixture(self):
        record = self.text_rights_fixture('original')
        path = self.root/'resources/compliance/synthetic-prose.json'
        g.save(path, {'synthetic': 'Available rights documented for this test only; not a real grant.'})
        proof = {'path': path.relative_to(self.root).as_posix(),
            'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
        record.update(declared_license='CC-BY-4.0', rights_available='Synthetic own expression only.',
            evidence=[proof], investigation_state='documented',
            attribution={'author_credit':'Synthetic contributor A',
                'license_url':'https://creativecommons.org/licenses/by/4.0/'})
        self.write_text_rights([record])
        return record, proof

    def test_partial_coverage_cannot_release_a_whole_file_or_override_source_terms(self):
        record, proof = self.documented_text_fixture()
        self.fixture.write('species_profiles.json', {'1': {'summary': {'text':'Synthetic educational passage.'},
            'hint': {'text':'Another synthetic component.'}}})
        assets = g.load(self.root/'resources/compliance/asset_rights.json')
        assets.setdefault('assets',[]).append({'path':record['file'],'kind':'text_authorship',
            'sha256':hashlib.sha256((self.root/record['file']).read_bytes()).hexdigest(),
            'permission':{'holders':['Synthetic contributor A','Synthetic contributor B'],
                'distribution_allowed':True,'evidence_path':proof['path'],'evidence_sha256':proof['sha256']}})
        g.save(self.root/'resources/compliance/asset_rights.json', assets)
        errors = g.audit_governance(self.root)['asset_licensing']
        self.assertIn({'path':record['file'],'error':'text_component_coverage_incomplete'}, errors)
        second = copy.deepcopy(record); second.update(pointer='/1/hint/text',
            text_sha256=hashlib.sha256(b'Another synthetic component.').hexdigest())
        second['attribution']['author_credit']='Synthetic contributor B'
        self.write_text_rights([record,second])
        self.assertEqual(rights.text_resource_issues(rights.educational_text_inventory(self.root),record['file']),[])
        self.assertFalse(any(e['path']==record['file'] for e in g.audit_governance(self.root)['asset_licensing']))
        # Component documentation alone never creates the separate file grant.
        assets=g.load(self.root/'resources/compliance/asset_rights.json')
        assets['assets'][0]['permission']['distribution_allowed']=False
        g.save(self.root/'resources/compliance/asset_rights.json',assets)
        self.assertIn({'path':record['file'],'error':'asset_permission_missing'},
            g.audit_governance(self.root)['asset_licensing'])
        second.update(origin_kind='translation',transformations=['translation'],source_text={
            'url':'https://example.org/source','license':'CC-BY-SA-4.0'})
        self.write_text_rights([record,second])
        self.assertIn('text_component_rights_pending',rights.text_resource_issues(
            rights.educational_text_inventory(self.root),record['file']))

    def test_full_mixed_coverage_still_requires_current_evidence_origin_and_attribution(self):
        record, proof = self.documented_text_fixture()
        self.assertEqual(rights.text_resource_issues(rights.educational_text_inventory(self.root),record['file']),[])
        for field,value in [('origin_kind','inconclusive'),('evidence',[]),('attribution',{})]:
            changed=copy.deepcopy(record);changed[field]=value;self.write_text_rights([changed])
            self.assertIn('text_component_rights_pending',rights.text_resource_issues(
                rights.educational_text_inventory(self.root),record['file']))
        self.write_text_rights([record]);(self.root/proof['path']).write_text('Modified fixture',encoding='utf8')
        self.assertIn('text_component_metadata_invalid',rights.text_resource_issues(
            rights.educational_text_inventory(self.root),record['file']))

    def test_attribution_does_not_invent_or_require_unprovided_author_names(self):
        record, _ = self.documented_text_fixture()
        record['attribution']['author_credit']=None
        record['attribution']['credit_not_supplied_by_source']=True
        self.write_text_rights([record])
        self.assertNotIn('passage_attribution_incomplete', rights.educational_text_inventory(self.root)
            ['rights_metadata']['records'][0]['rights_issues'])
        record['attribution'].pop('credit_not_supplied_by_source');record['declared_license']='CC0-1.0'
        record['attribution']['license_url']='https://creativecommons.org/publicdomain/zero/1.0/'
        self.write_text_rights([record])
        self.assertNotIn('passage_attribution_incomplete', rights.educational_text_inventory(self.root)
            ['rights_metadata']['records'][0]['rights_issues'])

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

    def test_reserved_branding_policy_and_official_use_intent_do_not_authorize_distribution(self):
        path = 'resources/branding/synthetic.png'
        image = self.root / path
        image.parent.mkdir(parents=True, exist_ok=True)
        image.write_bytes(b'synthetic branding fixture, not a real icon')
        asset = {'path': path, 'kind': 'branding',
                 'sha256': hashlib.sha256(image.read_bytes()).hexdigest(),
                 'state': 'pending_permission',
                 'permission': {'holder': None, 'evidence_path': None, 'distribution_allowed': False},
                 'administrative_policy': {'id': 'reserved-to-project',
                     'official_use_intended': True, 'general_graphic_reuse_granted': False}}
        g.save(self.root / 'resources/compliance/asset_rights.json', {'assets': [asset]})
        before = self.files()
        for _ in range(2):
            result = distribution.release_gate(self.root)
            self.assertIn({'path': path, 'error': 'asset_permission_missing'}, result['asset_licensing'])
            self.assertNotIn({'path': path, 'error': 'asset_changed_or_missing'}, result['asset_licensing'])
            self.assertTrue(result['distribution_blocked'])
            self.assertEqual(self.files(), before)
        self.assertFalse((self.root / 'resources/compliance/permission_history.jsonl').exists())
        row = next(r for r in self.run_report()['materials'] if r['path'] == path)
        self.assertEqual(row['administrative_policy']['id'], 'reserved-to-project')
        self.assertIsNone(row['permission_evidence'])
        self.assertEqual(row['registered_state'], 'pending_permission')

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

    def test_committed_snapshots_are_replayed_without_cache_or_new_consultation(self):
        original = self.run_report(online=True, client=self.client())
        g.save(self.root / 'docs/SANEAMENTO_DIREITOS.json', original)
        for path in (self.root / rights.EVIDENCE).glob('*.json'): path.unlink()
        client = self.client()
        report = self.run_report(online=True, client=client)
        client.get.assert_not_called()
        proposal = report['photos'][0]['public_proposal']
        self.assertEqual(proposal['snapshot'], original['photos'][0]['public_proposal']['snapshot'])
        self.assertEqual(proposal['archive_source'], 'docs/SANEAMENTO_DIREITOS.json')
        self.assertEqual(report['summary']['queries_this_run'], 0)
        self.assertTrue(report['distribution_gate']['source_blocked'])
        self.assertEqual(report['decisions_created'], 0)
        archive = g.load(self.root / 'docs/SANEAMENTO_DIREITOS.json')
        archive['photos'][0]['public_proposal']['snapshot']['record']['photo']['license_code'] = 'cc0'
        g.save(self.root / 'docs/SANEAMENTO_DIREITOS.json', archive)
        result = self.run_report()
        self.assertTrue(result['staged_evidence_errors'])
        self.assertEqual(result['summary']['photos_with_staged_public_metadata'], 0)

    def test_documentary_register_keeps_traceability_without_repeating_snapshots(self):
        original = self.run_report(online=True, client=self.client())
        g.save(self.root / 'docs/SANEAMENTO_DIREITOS.json', original)
        replay = self.run_report(); register = rights.documentary_summary(replay)
        reference = register['photos'][0]['public_proposal']
        self.assertNotIn('snapshot', reference)
        self.assertEqual(reference['snapshot_content_sha256'], g.digest(replay['photos'][0]['public_proposal']['snapshot']))
        self.assertEqual(reference['archive_source'], 'docs/SANEAMENTO_DIREITOS.json')
        self.assertIn('snapshot', replay['photos'][0]['public_proposal'])
        self.assertEqual(register['decisions_created'], 0)

    def test_actual_bytes_and_review_privacy_are_not_inferred_from_declared_metadata(self):
        row = self.run_report()['photos'][0]
        self.assertEqual(row['recorded_sha256'], 'a'*64)
        self.assertIsNone(row['local_sha256']); self.assertFalse(row['integrity_matches'])
        image = self.root / self.fixture.species['photo']['path']; image.parent.mkdir(parents=True, exist_ok=True)
        image.write_bytes(b'SYNTHETIC changed bytes')
        row = self.run_report()['photos'][0]
        self.assertEqual(row['local_sha256'], rights.file_hash(image)); self.assertFalse(row['integrity_matches'])
        public = rights.public_review({'state': 'pending', 'reviewer': 'private@example.invalid',
                                      'rationale': 'Confidential fixture <private@example.invalid>'})
        self.assertIsNone(public['reviewer']); self.assertTrue(public['private_details_withheld'])
        self.assertNotIn('example.invalid', json.dumps(public))

    def test_captive_casual_grade_is_reported_separately_from_matching_identity(self):
        obs = self.observation(); obs['quality_grade'] = 'casual'
        row = self.run_report(online=True, client=self.client(obs))['photos'][0]
        context = row['scientific_context']
        self.assertTrue(context['photo_association_matches']); self.assertTrue(context['taxon_id_matches'])
        self.assertTrue(context['community_taxon_matches']); self.assertTrue(context['captive'])
        self.assertEqual(context['wild_disagreement_count'], 1)
        self.assertFalse(context['research_grade_policy_met'])
        self.assertFalse(context['incorrect_identification_established'])

    def test_readme_capture_carries_its_photo_pending_items_not_a_new_grant(self):
        image = self.root / 'docs/images/quiz.png'; image.parent.mkdir(parents=True); image.write_bytes(b'SYNTHETIC CAPTURE')
        g.save(image.with_suffix('.json'), {'captured_at': '2026-01-01T00:00:00+00:00',
            'image_sha256': rights.file_hash(image), 'photo': {'photo_id': '7'}})
        rows = self.run_report()['readme_screenshots']
        self.assertEqual(len(rows), 1); self.assertTrue(rows[0]['capture_hash_matches'])
        self.assertEqual(rows[0]['priority'], 0); self.assertTrue(rows[0]['linked_rights_issues'])
        self.assertEqual(rows[0]['state'], 'pending')

    def git(self, *args):
        env = dict(os.environ, GIT_CONFIG_NOSYSTEM='1', GIT_CONFIG_GLOBAL=os.devnull)
        return subprocess.check_output(['git', '-C', str(self.root), '-c', 'core.hooksPath=', '-c', 'commit.gpgsign=false',
            '-c', 'user.name=Synthetic fixture', '-c', 'user.email=fixture@example.invalid', *args], env=env,
            stderr=subprocess.DEVNULL).decode('utf8').strip()

    def test_public_history_inventory_excludes_unpublished_local_branch(self):
        self.git('init', '-q')
        image = self.root / 'old.jpg'; image.write_bytes(b'Synthetic public media')
        self.git('add', 'old.jpg'); self.git('commit', '-qm', 'Synthetic historical media')
        image.unlink(); self.git('add', '-u'); self.git('commit', '-qm', 'Synthetic deletion')
        public = self.git('rev-parse', 'HEAD'); self.git('update-ref', 'refs/remotes/origin/main', public)
        self.git('checkout', '-qb', 'private-fixture')
        private = self.root / 'local-only.png'; private.write_bytes(b'Synthetic private media')
        self.git('add', 'local-only.png'); self.git('commit', '-qm', 'Synthetic private branch')
        private.unlink(); self.git('add', '-u'); self.git('commit', '-qm', 'Synthetic local deletion')
        report = self.run_report(); history = report['public_git_history']
        self.assertTrue(history['assessed'])
        self.assertEqual({r['path'] for r in history['resources']}, {'old.jpg'})
        self.assertEqual(report['summary']['history_only_media_paths'], 1)
        self.assertFalse(any('local-only' in r['path'] for r in history['resources']))

    def licensing_report(self, paths, **kwargs):
        with mock.patch.object(licensing_inventory, 'tracked', return_value=paths), mock.patch.object(
                licensing_inventory.subprocess, 'check_output', return_value=b'a'*40):
            return licensing_inventory.collect(self.root, at='2026-10-17T12:00:00+00:00', **kwargs)

    def test_documentary_categories_do_not_establish_ownership_or_choose_a_license(self):
        paths = ['src/example.cpp', 'scripts/example.py', 'resources/qml/Example.qml', 'README.md',
                 'resources/data/species_profiles.json', 'resources/data/species.json',
                 'resources/images/7.jpg', 'resources/branding/icon.png', 'vendor/example.cpp', 'docs/images/quiz.png']
        report = self.licensing_report(paths)
        self.assertEqual(set(r['category'] for r in report['files']), set(licensing_inventory.CATEGORIES))
        self.assertTrue(all(count == 1 for count in report['counts'].values()))
        self.assertFalse(report['license_selected_by_this_report']); self.assertEqual(report['approvals_created'], 0)
        for row in report['files']: self.assertFalse(row['ownership_confirmed_by_this_inventory'])
        for row in report['files']:
            if row['category'] != 'third_party_photographs': self.assertNotIn('declared_license', row)

    def test_photo_license_does_not_license_its_metadata_code_or_derived_capture(self):
        report = self.licensing_report(['resources/images/7.jpg', 'resources/data/species.json', 'docs/images/quiz.png'])
        photo, metadata, capture = report['files']
        self.assertEqual(photo['declared_license'], 'cc-by'); self.assertIsNone(photo['evidenced_license_version'])
        self.assertTrue(photo['documentary_issues']); self.assertNotIn('declared_license', metadata)
        self.assertNotIn('declared_license', capture)

    def test_dependency_declarations_keep_or_with_conditions_and_missing_notices(self):
        package = self.root / 'package'; package.mkdir()
        g.save(package / 'compliance/dependencies.json', {'packages': [
            {'name': 'synthetic-dependency', 'version': ['1.2.3-4'], 'license_files': [],
             'declared_licenses': ['MIT OR GPL-3.0-only WITH synthetic-exception'], 'obligations_review': 'pending'}], 'binaries': []})
        report = self.licensing_report([], package=package)
        row = report['package']['components'][0]
        self.assertEqual(row['declared_licenses'], ['MIT OR GPL-3.0-only WITH synthetic-exception'])
        self.assertEqual(row['version'], ['1.2.3-4']); self.assertEqual(row['obligations_review'], 'pending')
        self.assertEqual(row['notice_integrity'], [])
        self.assertIn('dependency_obligations_unresolved', {e['error'] for e in report['package']['errors']})

    def test_additional_notice_candidate_is_not_copied_or_approved(self):
        package = self.root / 'package'; package.mkdir()
        prefix = self.root / 'installation'; notice = prefix / 'share/icu/1.2/LICENSE'
        notice.parent.mkdir(parents=True); notice.write_bytes(b'Synthetic notice only')
        g.save(package / 'compliance/dependencies.json', {'packages': [
            {'name': 'synthetic-icu', 'version': ['1.2-3'], 'license_files': [], 'declared_licenses': ['ICU'],
             'obligations_review': 'pending'}], 'binaries': []})
        before = {p: p.read_bytes() for p in package.rglob('*') if p.is_file()}
        report = self.licensing_report([], package=package, prefix=prefix)
        candidate = report['package']['additional_notice_candidates'][0]
        self.assertTrue(candidate['exists']); self.assertEqual(candidate['sha256'], rights.file_hash(notice))
        self.assertFalse(candidate['installed_package_version_matches'])
        self.assertEqual(before, {p: p.read_bytes() for p in package.rglob('*') if p.is_file()})
        self.assertEqual(report['package']['components'][0]['obligations_review'], 'pending')
        desc = prefix.parent / 'var/lib/pacman/local/synthetic-icu-1.2-3/desc'
        desc.parent.mkdir(parents=True); desc.write_text('%NAME%\nsynthetic-icu\n\n%VERSION%\n1.2-3\n', encoding='utf8')
        updated = self.licensing_report([], package=package, prefix=prefix)
        self.assertTrue(updated['package']['additional_notice_candidates'][0]['installed_package_version_matches'])

    def test_licensing_inventory_is_read_only_and_cannot_bypass_existing_gate(self):
        before = self.files()
        with (mock.patch.object(distribution, 'dependencies', side_effect=AssertionError('No inventory/export writes')),
                mock.patch.object(g, 'confirm_permission', side_effect=AssertionError('No grants')),
                mock.patch.object(g, 'append_review', side_effect=AssertionError('No reviews'))):
            report = self.licensing_report(['resources/data/species.json'])
        self.assertEqual(before, self.files()); self.assertTrue(report['gate']['distribution_blocked'])
        self.assertFalse(report['package']['assessed'])

    def test_inventory_omits_self_hash_and_flags_missing_bytes_without_invented_licenses(self):
        report = self.licensing_report([licensing_inventory.ARCHIVE, 'src/missing.cpp'])
        self.assertEqual(len(report['files']), 1); self.assertIsNone(report['files'][0]['sha256'])
        self.assertNotIn('declared_license', report['files'][0])

    def code_license_fixture(self):
        """Synthetic source tree; no grant/reviewer event for any real material."""
        source = Path(__file__).resolve().parents[1]
        for name in ('LICENSE', 'NOTICE'):
            (self.root / name).write_bytes((source / name).read_bytes())
        path = self.root / 'src/Example.cpp'; path.parent.mkdir(parents=True)
        path.write_text('// Synthetic test code only\n', encoding='utf8')
        path = self.root / 'scripts/build_education.py'; path.parent.mkdir(parents=True)
        path.write_text('# Synthetic mixed material, rights pending\n', encoding='utf8')
        scope = copy.deepcopy(g.load(source / licensing_inventory.CODE_SCOPE))
        scope['files'] = ['src/Example.cpp']
        scope['pending_files'] = {'scripts/build_education.py': 'Synthetic mixed prose/code awaiting rights review.'}
        g.save(self.root / licensing_inventory.CODE_SCOPE, scope)
        policy = g.policy(self.root)
        policy['own_material_licenses'] = {'code': 'Apache-2.0', 'texts': None, 'collection': None}
        policy.pop('editorial_text_policy', None)  # Legacy code-only fixture.
        policy['maintainer_decisions'] = [scope['maintainer_decision']]
        g.save(self.root / 'resources/data/content_policy.json', policy)
        assets = g.load(self.root / 'resources/compliance/asset_rights.json', {'assets': []})
        assets['own_material_licenses'] = policy['own_material_licenses']
        g.save(self.root / 'resources/compliance/asset_rights.json', assets)
        return scope

    def code_errors(self):
        return licensing_inventory.check_code_license(self.root,
            paths=['src/Example.cpp', 'scripts/build_education.py'])

    def test_actual_apache_scope_and_canonical_license_are_consistent(self):
        self.assertEqual(licensing_inventory.check_code_license(), [])

    def test_canonical_license_is_complete_and_cannot_accept_extra_restrictions(self):
        self.code_license_fixture(); self.assertEqual(self.code_errors(), [])
        license_path = self.root / 'LICENSE'
        official = license_path.read_bytes()
        for changed in (official[:1000], official + b'Noncommercial use only\n', official.replace(b'January 2004', b'January 2026')):
            with self.subTest(sha=hashlib.sha256(changed).hexdigest()):
                license_path.write_bytes(changed)
                self.assertIn('apache_text_missing_or_modified', self.code_errors())

    def test_notice_missing_original_attribution_is_rejected(self):
        self.code_license_fixture(); (self.root / 'NOTICE').write_text('Apache License, Version 2.0\n', encoding='utf8')
        self.assertIn('notice_missing_attribution_or_scope', self.code_errors())

    def test_code_grant_does_not_grant_photo_rights_or_change_reviews(self):
        before = self.files(); gate_before = distribution.release_gate(self.root)
        self.code_license_fixture(); after_policy = self.files()
        with mock.patch.object(g, 'confirm_permission', side_effect=AssertionError('No photo grants')), \
             mock.patch.object(g, 'append_review', side_effect=AssertionError('No reviews')):
            self.assertEqual(self.code_errors(), [])
            self.assertEqual(self.files(), after_policy)
        for name, data in before.items():
            if name not in {'resources/data/content_policy.json', 'resources/compliance/asset_rights.json'}:
                self.assertEqual(self.files()[name], data, name)
        after = distribution.release_gate(self.root)
        self.assertTrue(after['distribution_blocked'])
        for key in ('photo_licensing', 'asset_licensing', 'scientific_release_blocks'):
            self.assertEqual(after[key], gate_before[key])

    def test_media_and_third_party_code_cannot_be_added_to_own_scope(self):
        scope = self.code_license_fixture()
        for name in ('resources/images/7.jpg', 'vendor/example.cpp', 'resources/data/species.json'):
            with self.subTest(name=name):
                scope['files'] = ['src/Example.cpp', name]
                g.save(self.root / licensing_inventory.CODE_SCOPE, scope)
                self.assertTrue(any(e.startswith('noncode_or_missing_scope_path:') for e in self.code_errors()))

    def test_future_code_is_not_licensed_by_folder_inheritance(self):
        self.code_license_fixture()
        self.assertIn('code_scope_inventory_differs', licensing_inventory.check_code_license(
            self.root, paths=['src/Example.cpp', 'src/New.cpp', 'scripts/build_education.py']))

    def test_pending_mixed_code_cannot_silently_be_relicensed(self):
        scope = self.code_license_fixture()
        assets = g.load(self.root / 'resources/compliance/asset_rights.json')
        assets['assets'].append({'path': 'scripts/build_education.py', 'state': 'pending_originality_and_rights_review'})
        g.save(self.root / 'resources/compliance/asset_rights.json', assets)
        scope['files'].append('scripts/build_education.py'); scope['pending_files'] = {}
        g.save(self.root / licensing_inventory.CODE_SCOPE, scope)
        self.assertIn('pending_material_in_code_scope', self.code_errors())

    def test_texts_and_collection_need_their_own_decision(self):
        self.code_license_fixture()
        policy = g.policy(self.root); policy['own_material_licenses']['texts'] = 'CC-BY-4.0'
        g.save(self.root / 'resources/data/content_policy.json', policy)
        self.assertIn('own_license_declarations_differ', self.code_errors())
        self.assertIn('asset_and_policy_declarations_differ', self.code_errors())

    def test_decision_must_link_to_administrative_authorization_not_a_review(self):
        self.code_license_fixture()
        policy = g.policy(self.root); policy['maintainer_decisions'] = []
        g.save(self.root / 'resources/data/content_policy.json', policy)
        self.assertIn('administrative_decision_not_linked', self.code_errors())
        self.assertFalse((self.root / 'resources/compliance/review_history.jsonl').exists())
        self.assertFalse((self.root / 'resources/compliance/permission_history.jsonl').exists())


if __name__ == '__main__':
    unittest.main()
