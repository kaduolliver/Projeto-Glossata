"""Offline adversarial regression tests for evidence, gates and review lifecycle."""
import copy
from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

PROJECT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT / 'scripts'))
import content_governance as governance
import import_inaturalist as importer
import review_updates
import distribution


class GovernanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'VERSION.txt').write_text('1.0.0\n', encoding='utf8')
        (self.root / 'CMakeLists.txt').write_text('# synthetic fixture\n', encoding='utf8')
        self.photo = {'id': '7', 'license_code': 'cc-by', 'attribution': 'Photographer as credited',
                      'url': 'https://inaturalist-open-data.s3.amazonaws.com/photos/7/square.jpg',
                      'original_dimensions': {'width': 800, 'height': 600}}
        self.observation = {'id': 11, 'quality_grade': 'research', 'community_taxon_id': 22,
                            'license_code': 'cc0', 'user': {'login': 'Different observer'},
                            'private_location': 'secret', 'location': 'sensitive', 'geojson': {'coordinates': [1, 2]},
                            'taxon': {'id': 22, 'name': 'Example species', 'rank': 'species', 'is_active': True},
                            'photos': [self.photo]}
        self.species = {'id': 1, 'name': 'Example species', 'educational_kingdom': 'Animalia', 'source_taxon_id': '22', 'taxonomy': [],
                        'photo': {'id': '7', 'author': self.photo['attribution'], 'license': 'cc-by',
                                  'license_url': 'https://creativecommons.org/licenses/by/4.0/', 'scientific_name': 'Example species',
                                  'taxon_id': '22', 'sha256': 'a'*64, 'path': 'resources/images/7.jpg',
                                  'source': 'https://www.inaturalist.org/observations/11', 'url': self.photo['url'],
                                  'modifications': 'Variante da fonte; sem recorte local.'}}
        self.note = {'text': 'Original educational description.', 'source_url': 'https://example.org/species',
                     'source_title': 'Reference', 'verified': True, 'reviewed_on': '2026-10-14'}
        self.write('species.json', [self.species]); self.write('taxonomy.json', [])
        self.write('curation_reviews.json', {'1': dict(scientific_name='Example species', **{t: 'approved' for t in governance.TOPICS})})
        self.write('education.json', {'notes': {'species:1:summary': self.note},
                                      'species': {'1': {'scientific_name': 'Example species', 'summary': 'species:1:summary'}}})
        self.write('content_policy.json', governance.policy(PROJECT))

    def write(self, filename, data): governance.save(self.root / 'resources/data' / filename, data)

    def baseline(self):
        with mock.patch.object(governance, 'now', return_value='2026-10-15T12:00:00+00:00'):
            return governance.migrate(self.root)

    def decision(self, key='species:1:summary'):
        return {'state': 'approved_editorial', 'reviewer': 'curator-one', 'actor_type': 'human', 'review_kind': 'editorial',
                'reviewed_at': (datetime.now(timezone.utc)-timedelta(seconds=1)).isoformat(),
                'rationale': 'Compared the stated claim against the indicated paragraph.',
                'content_sha256': governance.records(self.root)[key]['content_sha256'], 'claim_scope': 'species',
                'evidence': [{'url': 'https://example.org/species', 'locator': 'Description, paragraph 2', 'supports': 'Description in the summary'}]}

    def test_photo_license_is_not_inherited_and_direct_download_is_guarded(self):
        for code in ('all-rights-reserved', None, '', 'cc-by-sa', 'cc-by-nd', 'cc-by-nc', 'unrecognized'):
            photo = dict(self.photo, license_code=code)
            self.assertFalse(importer.permitted_photo(photo))
            with mock.patch.object(importer.urllib.request, 'urlopen') as network, self.assertRaises(ValueError):
                importer.download_photo(photo)
            network.assert_not_called()
        for code in ('cc0', 'cc-by'): self.assertTrue(importer.permitted_photo(dict(self.photo, license_code=code)))

    def test_research_grade_requires_community_taxon_and_the_specific_associated_photo(self):
        self.assertTrue(governance.observation_eligible(self.observation, self.photo, 22))
        for changed in (dict(self.observation, quality_grade='casual'), dict(self.observation, community_taxon_id=33),
                        dict(self.observation, photos=[]), dict(self.observation, taxon=dict(self.observation['taxon'], is_active=False))):
            self.assertFalse(governance.observation_eligible(changed, self.photo, 22))
        self.assertFalse(governance.observation_eligible(self.observation, dict(self.photo, license_code='cc0'), 22))

    def test_permission_snapshot_is_minimal_content_addressed_and_has_no_observer_or_location(self):
        photo = dict(self.photo, flags=[{'user': {'email': 'private'}, 'flag': 'copyright'}])
        observation = dict(self.observation, photos=[photo])
        relative = governance.archive_permission(self.root, observation, photo, 'live_api', governance.now())
        snapshot = governance.load(self.root / relative)
        self.assertEqual(governance.digest(snapshot), Path(relative).stem)
        raw = json.dumps(snapshot)
        for private in ('Different observer', 'private_location', 'coordinates', 'email', 'secret'):
            self.assertNotIn(private, raw)
        self.assertEqual(snapshot['record']['photo']['attribution'], self.photo['attribution'])
        self.assertIs(snapshot['record']['photo']['flags'], True)

    def test_legacy_migration_is_idempotent_and_never_creates_a_human_review(self):
        first = self.baseline(); bytes_before = (self.root / 'resources/compliance/review_baseline.json').read_bytes()
        self.assertEqual(first, governance.migrate(self.root))
        self.assertEqual(bytes_before, (self.root / 'resources/compliance/review_baseline.json').read_bytes())
        rec = governance.records(self.root)['species:1:summary']
        status = governance.status('species:1:summary', rec, first, [], governance.policy(self.root))
        self.assertEqual(status['state'], 'legacy'); self.assertIsNone(status['reviewer']); self.assertIsNone(status['reviewed_at'])
        self.assertNotIn('approved', status['label'])

    def test_new_ai_text_is_draft_even_with_a_url_verified_flag_and_historical_date(self):
        record = governance.records(self.root)['species:1:summary']
        result = governance.status('species:1:summary', record, {}, [], governance.policy(self.root))
        self.assertEqual(result['state'], 'draft'); self.assertFalse(result['usable'])

    def test_review_requires_responsibility_real_date_and_localized_evidence(self):
        for field, value in [('reviewer', ''), ('reviewed_at', '2999-01-01'), ('evidence', []), ('claim_scope', '')]:
            decision = self.decision(); decision[field] = value
            with self.assertRaises(ValueError): governance.append_review(self.root, 'species:1:summary', decision)
        decision = self.decision(); decision['evidence'][0]['locator'] = ''
        with self.assertRaises(ValueError): governance.append_review(self.root, 'species:1:summary', decision)
        decision = self.decision(); decision['state'] = 'approved_expert'
        with self.assertRaises(ValueError): governance.append_review(self.root, 'species:1:summary', decision)

    def test_one_named_curator_can_record_editorial_review_without_simulating_independence(self):
        event = governance.append_review(self.root, 'species:1:summary', self.decision())
        events = governance.read_ledger(self.root)
        self.assertEqual(len(events), 1); self.assertEqual(events[0]['review_kind'], 'editorial')
        result = governance.status('species:1:summary', governance.records(self.root)['species:1:summary'], {}, events, governance.policy(self.root))
        self.assertEqual(result['state'], 'approved_editorial'); self.assertEqual(result['reviewer'], 'curator-one')
        self.assertEqual(event['event_sha256'], events[0]['event_sha256'])

    def test_changed_text_source_identity_and_photo_invalidate_relevant_review(self):
        baseline = self.baseline()
        governance.append_review(self.root, 'species:1:summary', self.decision())
        original = governance.records(self.root)['topic:1:taxonomy']['content_sha256']
        data = governance.load(self.root / 'resources/data/education.json'); data['notes']['species:1:summary']['source_url'] = 'https://example.org/changed'
        self.write('education.json', data)
        record = governance.records(self.root)['species:1:summary']
        self.assertEqual(governance.status('species:1:summary', record, baseline, governance.read_ledger(self.root), governance.policy(self.root))['state'], 'needs_review')
        self.assertEqual(original, governance.records(self.root)['topic:1:taxonomy']['content_sha256'])
        self.species['name'] = 'Different species'; self.write('species.json', [self.species])
        rec = governance.records(self.root)['topic:1:taxonomy']
        self.assertEqual(governance.status('topic:1:taxonomy', rec, baseline, [], governance.policy(self.root))['state'], 'needs_review')
        self.species['photo']['sha256'] = 'b'*64; self.write('species.json', [self.species])
        rec = governance.records(self.root)['topic:1:photo']
        self.assertEqual(governance.status('topic:1:photo', rec, baseline, [], governance.policy(self.root))['state'], 'needs_review')

    def test_review_expires_without_renewing_date_and_ledger_tampering_is_detected(self):
        event = governance.append_review(self.root, 'species:1:summary', self.decision())
        future = governance.parse_time(event['reviewed_at']) + timedelta(days=366)
        result = governance.status('species:1:summary', governance.records(self.root)['species:1:summary'], {}, [event], governance.policy(self.root), future)
        self.assertEqual(result['state'], 'expired'); self.assertFalse(result['usable'])
        self.assertEqual(result['reviewed_at'], event['reviewed_at'])
        path = self.root / 'resources/compliance/review_history.jsonl'
        path.write_text(path.read_text(encoding='utf8').replace('curator-one', 'invented-person'), encoding='utf8')
        with self.assertRaises(ValueError): governance.read_ledger(self.root)

    def test_credits_use_photo_author_and_optional_title_and_keep_cc0_credit(self):
        self.species['photo']['license'] = 'cc0'; self.species['photo']['license_url'] = 'https://creativecommons.org/publicdomain/zero/1.0/'
        self.write('species.json', [self.species]); self.baseline()
        result = governance.generate_credits(self.root)
        text = governance.render_credits(result)
        self.assertIn(self.photo['attribution'], text); self.assertIn('CC0', text)
        self.assertNotIn('Different observer', text); self.assertIsNone(result['photos']['7']['title'])
        self.assertIn('original_collection_date_unknown', result['photos']['7']['rights_issues'])

    def test_missing_or_incompatible_permission_evidence_is_a_release_block(self):
        self.baseline(); metadata = governance.load(self.root / 'resources/data/photo_provenance.json')['7']
        self.assertIn('permission_evidence_missing', governance.permission_issues(self.species['photo'], metadata, self.root))
        p = dict(self.photo, id='8'); o = dict(self.observation, photos=[p])
        relative = governance.archive_permission(self.root, o, p, 'legacy_cache')
        metadata['evidence_paths'] = [relative]
        self.assertIn('permission_identity', governance.permission_issues(self.species['photo'], metadata, self.root))
        self.assertTrue(distribution.release_gate(self.root)['distribution_blocked'])

    def test_noncommercial_exception_needs_explicit_policy_and_scoped_documented_grant(self):
        evidence = self.root / 'grant.txt'; evidence.write_text('Test-only permission fixture.', encoding='utf8')
        photo = dict(self.photo, license_code='cc-by-nc')
        settings = governance.policy(self.root)
        settings['exceptions'] = [{'photo_id': '7', 'license': 'cc-by-nc', 'source_url': photo['url'], 'grantor': 'Fixture holder',
            'recorded_by': 'Fixture curator', 'recorded_at': governance.now(), 'justification': 'Test only', 'scopes': ['download'],
            'evidence_path': 'grant.txt', 'evidence_sha256': hashlib.sha256(evidence.read_bytes()).hexdigest()}]
        self.write('content_policy.json', settings)
        self.assertFalse(governance.license_allowed(photo, self.root))
        settings['noncommercial'] = {'enabled': True, 'policy_document': 'grant.txt'}; settings['distribution_mode'] = 'noncommercial'
        self.write('content_policy.json', settings)
        self.assertTrue(governance.license_allowed(photo, self.root))
        self.assertFalse(governance.license_allowed(photo, self.root, 'distribution'))

    def test_online_check_proposes_license_and_taxon_changes_without_mutating_published_content(self):
        self.baseline(); governance.generate_credits(self.root)
        before = {p.relative_to(self.root): p.read_bytes() for p in (self.root/'resources').rglob('*') if p.is_file()}
        changed = copy.deepcopy(self.observation); changed['photos'][0]['license_code'] = None
        changed['taxon']['name'] = 'Another species'
        class Client:
            def get(inner, url, as_json=False):
                if '/observations/' in url: return {'results': [changed]}
                if '/taxa/' in url: return {'results': [changed['taxon']]}
                return {'http_status': 200}
        result = review_updates.propose_updates(self.root, True, 1, Client())
        changes = result['checks'][0]['changes']
        self.assertIn('license_changed_requires_analysis_not_automatic_revocation', changes)
        self.assertIn('possible_identification_or_taxonomy_change', changes)
        after = {p.relative_to(self.root): p.read_bytes() for p in (self.root/'resources').rglob('*') if p.is_file()}
        self.assertEqual(before, after)

    def test_extra_uninventoried_runtime_file_blocks_distribution(self):
        package = self.root/'package'; package.mkdir(); (package/'unknown.dll').write_bytes(b'unknown'); (package/'unknown.ico').write_bytes(b'unknown-icon')
        errors = distribution.dependency_errors({'binaries': [], 'packages': []}, package)
        self.assertIn('dependency_not_inventoried', {e['error'] for e in errors})
        self.assertEqual(sum(e['error']=='dependency_not_inventoried' for e in errors), 2)

    def test_stale_packaged_binary_cannot_be_cleared_by_current_metadata(self):
        self.baseline(); governance.generate_credits(self.root)
        (self.root/'resources.qrc').write_text('<RCC><qresource/></RCC>', encoding='utf8')
        package=self.root/'package'; package.mkdir(); binary=package/'projeto_glossata.exe'; binary.write_bytes(b'fixture-binary')
        manifest=distribution.record_build(binary, self.root)
        governance.save(package/'compliance/build-manifest.json', manifest)
        binary.write_bytes(b'older-or-replaced-binary')
        result=distribution.release_gate(self.root, package)
        self.assertIn('packaged_binary_or_compiled_sources_stale', {e['error'] for e in result['technical_errors']})

    def test_complete_permission_needs_version_proof_real_capture_and_a_bound_human_confirmation(self):
        self.baseline()
        relative = governance.archive_permission(self.root, self.observation, self.photo, 'live_api', governance.now())
        proof = self.root / 'resources/compliance/version-proof.txt'
        proof.write_text('Synthetic fixture for a human-reviewed photo-specific license version.', encoding='utf8')
        current = governance.load(self.root / 'resources/data/photo_provenance.json')['7']
        paths = sorted(current['evidence_paths'] + [relative])
        bound = {'photo_id': '7', 'sha256': self.species['photo']['sha256'], 'license': 'cc-by',
            'license_url': self.species['photo']['license_url'], 'license_version': '4.0', 'evidence_paths': paths}
        decision = {'photo_id': '7', 'license_version': '4.0', 'evidence_paths': [relative],
                    'reviewer': 'fixture-human', 'actor_type': 'human', 'reviewed_at': governance.now(), 'rationale': 'Fixture only, not a real approval.',
                    'content_sha256': governance.digest(bound), 'license_version_evidence': {'path': proof.relative_to(self.root).as_posix(),
                    'sha256': hashlib.sha256(proof.read_bytes()).hexdigest(), 'source_url': 'https://example.org/photos/7', 'locator': 'License link for photo 7'}}
        confirmed = governance.confirm_permission(self.root, decision)
        self.assertEqual(governance.permission_issues(self.species['photo'], confirmed, self.root), [])
        self.assertIsNone(confirmed['original_collected_at'])
        self.assertIsNotNone(confirmed['first_evidenced_at'])
        self.assertEqual(len(governance.read_chain(self.root/'resources/compliance/permission_history.jsonl')), 1)
        confirmed['license_version'] = '3.0'
        self.assertIn('license_version_url_mismatch', governance.permission_issues(self.species['photo'], confirmed, self.root))

    def test_attribution_document_and_private_data_are_audited_independently(self):
        self.baseline(); governance.generate_credits(self.root)
        (self.root / 'docs/PHOTO_CREDITS.md').write_text('Stale manual credits', encoding='utf8')
        data = governance.load(self.root/'resources/data/education.json')
        data['coordinates'] = [1, 2]; self.write('education.json', data)
        errors = {e['error'] for e in governance.audit_governance(self.root)['technical_errors']}
        self.assertIn('attribution_document_out_of_date', errors)
        self.assertIn('unnecessary_private_fields', errors)

    def test_new_review_cannot_leave_the_published_metadata_silently_stale(self):
        self.baseline(); governance.generate_credits(self.root)
        data=governance.load(self.root/'resources/data/education.json')
        governance.annotate_education(self.root, data); self.write('education.json', data)
        governance.append_review(self.root, 'species:1:summary', self.decision())
        errors=governance.audit_governance(self.root)['technical_errors']
        self.assertTrue(any(e.get('key')=='species:1:summary' and e['error']=='generated_review_metadata_stale' for e in errors))
        governance.annotate_education(self.root, data); self.write('education.json', data)
        errors=governance.audit_governance(self.root)['technical_errors']
        self.assertFalse(any(e.get('key')=='species:1:summary' and e['error']=='generated_review_metadata_stale' for e in errors))

    def test_image_metadata_privacy_is_detected_without_printing_coordinates_or_changing_bytes(self):
        path=self.root/'photo.jpg'
        header=b'II'+(42).to_bytes(2,'little')+(8).to_bytes(4,'little')
        entry=(0x8825).to_bytes(2,'little')+(4).to_bytes(2,'little')+(1).to_bytes(4,'little')+(26).to_bytes(4,'little')
        original=b'\xff\xd8Exif\x00\x00'+header+(1).to_bytes(2,'little')+entry+b'\x00'*4
        path.write_bytes(original)
        self.assertEqual(governance.image_privacy_findings(path), ['exif_gps_metadata'])
        self.assertEqual(path.read_bytes(), original)
        tiff=header+(1).to_bytes(2,'little')+entry+b'\x00'*4
        path=self.root/'photo.png'
        path.write_bytes(b'\x89PNG\r\n\x1a\n'+len(tiff).to_bytes(4,'big')+b'eXIf'+tiff+b'\x00'*4)
        self.assertEqual(governance.image_privacy_findings(path), ['exif_gps_metadata'])

    def test_taxonomic_move_and_ancestor_rename_are_reported_even_when_species_name_is_unchanged(self):
        nodes={'root': {'source_taxon_id':'1','name':'Animalia','rank':'kingdom'},
               'genus': {'source_taxon_id':'2','name':'Example','rank':'genus'}}
        species=dict(self.species, taxonomy=['root','genus'])
        unchanged={'id':22,'name':'Example species','ancestor_ids':[99,1,2],
                   'ancestors':[{'id':1,'name':'Animalia','rank':'kingdom'},{'id':2,'name':'Example','rank':'genus'}]}
        self.assertFalse(review_updates.hierarchy_changed(species, unchanged, nodes))
        moved=dict(unchanged, ancestor_ids=[99,1,3])
        self.assertTrue(review_updates.hierarchy_changed(species, moved, nodes))
        renamed=copy.deepcopy(unchanged); renamed['ancestors'][1]['name']='Renamed'
        self.assertTrue(review_updates.hierarchy_changed(species, renamed, nodes))


if __name__ == '__main__': unittest.main()
