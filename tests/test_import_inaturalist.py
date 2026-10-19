"""Offline import and provenance regression tests; no requests to the live API."""
import contextlib
import functools
import hashlib
import importlib.util
import io
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile
from unittest import mock

PROJECT = pathlib.Path(__file__).resolve().parents[1]
REPOSITORY = PROJECT


@functools.lru_cache(maxsize=2)
def historical_catalog_archive(commit):
    return subprocess.check_output(['git','archive','--format=zip',commit,
        'resources','docs','scripts','src'],cwd=REPOSITORY)


def at_catalog_commit(commit):
    """Historical batch assertions run at their real state, never today's grants."""
    def decorate(method):
        @functools.wraps(method)
        def run(self):
            with tempfile.TemporaryDirectory(prefix='glossata-historical-catalog-') as directory:
                root=pathlib.Path(directory)
                with zipfile.ZipFile(io.BytesIO(historical_catalog_archive(commit))) as archive:
                    for entry in archive.infolist():
                        self.assertTrue((root/entry.filename).resolve().is_relative_to(root.resolve()))
                    archive.extractall(root)
                with mock.patch.object(sys.modules[__name__],'PROJECT',root):
                    return method(self)
        return run
    return decorate
sys.path.insert(0, str(PROJECT / "scripts"))
spec = importlib.util.spec_from_file_location("importer", PROJECT / "scripts/import_inaturalist.py")
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)
education_spec = importlib.util.spec_from_file_location("education", PROJECT / "scripts/build_education.py")
education = importlib.util.module_from_spec(education_spec)
with mock.patch.dict(sys.modules, {"import_inaturalist": importer}):
    education_spec.loader.exec_module(education)


class ImportTests(unittest.TestCase):
    def test_priority_photo_replacement_preserves_catalog_recipes_and_history(self):
        base = 'afabb70186cf99906f6148585e09749243626db5'
        old = json.loads(self.historical_bytes(base, 'resources/data/species.json'))
        self.assertEqual(len(self.species), 122)
        self.assertEqual([s['id'] for s in old], [s['id'] for s in self.species])
        for previous, current in zip(old, self.species):
            if current['id'] in (125, 128):
                self.assertEqual({k: v for k, v in previous.items() if k not in ('photo', 'image')},
                                 {k: v for k, v in current.items() if k not in ('photo', 'image')})
                self.assertNotEqual(previous['photo']['id'], current['photo']['id'])
                self.assertEqual(hashlib.sha256((PROJECT / previous['photo']['path']).read_bytes()).hexdigest(),
                                 previous['photo']['sha256'])
            else:
                self.assertEqual(previous, current)
        before = json.loads(self.historical_bytes(base, 'resources/data/progressive_hints.json'))
        after = json.loads((PROJECT / 'resources/data/progressive_hints.json').read_text(encoding='utf8'))
        self.assertEqual(sum(len(s['recipes']) for s in after['species'].values()), 855)
        for sid in before['species']:
            self.assertEqual(before['species'][sid]['recipes'], after['species'][sid]['recipes'])
        ledger = 'resources/compliance/review_history.jsonl'
        prefix = self.historical_bytes(base, ledger)
        current = (PROJECT / ledger).read_bytes()
        self.assertTrue(current.startswith(prefix))
        added = [json.loads(line) for line in current[len(prefix):].splitlines()]
        self.assertEqual({e['key'] for e in added}, {'topic:125:photo', 'topic:128:photo',
            'species:125:morphological_hint', 'species:128:morphological_hint'})
        self.assertEqual(len(added), 6)
        corrections = [e for e in added if 'metadata_correction' in e]
        self.assertEqual(len(corrections), 2)
        original_by_hash = {e['event_sha256']: e for e in added[:4]}
        for event in corrections:
            original = original_by_hash[event['metadata_correction']['prior_event_sha256']]
            self.assertEqual(event['reviewed_at'], original['reviewed_at'])
            self.assertEqual(event['reviewer'], original['reviewer'])
            self.assertEqual(event['evidence'], original['evidence'])
            reference = event['metadata_correction']
            proof = (PROJECT / reference['confirmation_path']).read_bytes()
            self.assertEqual(proof, self.historical_bytes('285fc96', reference['confirmation_path']))
            self.assertEqual(hashlib.sha256(proof).hexdigest(), reference['confirmation_sha256'])
            self.assertEqual(json.loads(proof)['answer'], 'Aprovo editorialmente as duas associações e dicas')
            self.assertFalse(reference['new_human_decision'])
        self.assertTrue(all(e['state'] == 'approved_editorial' for e in added))
        for path in ('resources/data/content_policy.json', 'resources/data/taxonomy.json',
                     'resources/data/species_profiles.json', 'resources/compliance/review_baseline.json'):
            self.assertEqual(self.historical_bytes(base, path), (PROJECT / path).read_bytes())

    def test_priority_photo_evidence_and_editorial_scope(self):
        import content_governance as governance
        import rights_investigation as rights
        inventory = rights.educational_text_inventory(PROJECT)
        self.assertEqual(inventory['rights_metadata']['technical_errors'], [])
        self.assertFalse(any(row['rights_issues'] for row in inventory['rights_metadata']['records']))
        document = governance.load(PROJECT / 'docs/ETAPA_1_FOTOGRAFIAS.json')
        provenance = governance.permission_provenance(PROJECT)
        content = governance.load(PROJECT / 'resources/data/education.json')
        qrc = (PROJECT / 'resources.qrc').read_text(encoding='utf8')
        for change in document['changes']:
            sid = str(change['organism_id']); photo = change['new_photo']
            source = governance.load(PROJECT / change['source_evidence'])['record']
            old = governance.load(PROJECT / change['old_source_evidence'])
            self.assertTrue(old['observation_context']['captive'])
            self.assertEqual(old['record']['quality_grade'], 'casual')
            self.assertEqual(source['quality_grade'], 'research')
            self.assertEqual(source['community_taxon_id'], str(source['taxon']['id']))
            self.assertEqual(str(source['photo']['id']), photo['id'])
            self.assertEqual(source['photo']['attribution'], photo['author'])
            self.assertEqual(source['photo']['license_code'], photo['license'])
            self.assertIn(int(photo['taxon_id']), source['taxon']['ancestor_ids'])
            self.assertEqual(str(source['taxon']['id']), photo['observation_taxon_id'])
            self.assertEqual(governance.source_findings(photo, provenance[photo['id']], PROJECT), [])
            pending = governance.permission_issues(photo, provenance[photo['id']], PROJECT)
            self.assertIn('license_version_not_evidenced', pending)
            self.assertIn('permission_confirmation_missing_or_changed', pending)
            self.assertNotIn('permission_confirmation', provenance[photo['id']])
            self.assertIn(photo['path'], qrc)
            self.assertNotIn(change['old_photo']['path'], qrc)
            note = content['notes']['species:' + sid + ':morphological_hint']
            self.assertEqual(note['photo_id'], photo['id'])
            self.assertEqual(note['governance']['state'], 'approved_editorial')
            self.assertTrue(content['species'][sid]['governance']['photo']['usable'])
            self.assertFalse(content['species'][sid]['governance']['morphology']['usable'])
            # A further change must not inherit the editorial approval just recorded.
            changed = json.loads(json.dumps(content))
            changed['notes']['species:' + sid + ':morphological_hint']['photo_id'] = '999999'
            governance.annotate_education(PROJECT, changed)
            self.assertEqual(changed['notes']['species:' + sid + ':morphological_hint']['governance']['state'],
                             'needs_review')

    def historical_bytes(self, revision, relative):
        return subprocess.check_output(['git','show',revision+':'+relative],cwd=REPOSITORY)

    def setUp(self):
        self.species = json.loads((PROJECT / "resources/data/species.json").read_text(encoding="utf-8"))

    def test_all_original_fields_and_ids_are_preserved(self):
        original = json.loads((PROJECT / "tests/fixtures/legacy_species.json").read_text(encoding="utf-8"))
        self.assertEqual(len(original), 22)
        current = {s["id"]: s for s in self.species}
        retired={x['old']['id']:x['old'] for x in json.loads((PROJECT/'resources/data/catalog_replacements.json').read_text(encoding='utf8'))['replacements']}
        for item in original:
            for key in ("id", "name", "fact", "hint", "region"):
                value=item[key]
                record=current.get(item['id'],retired.get(item['id']))
                self.assertEqual(record[key], value, (item["name"], key))

    def test_individual_license_and_quality_gate(self):
        photo = {"license_code": "cc-by", "attribution": "Author", "url": "https://example.test/photo.jpg",
                 "original_dimensions": {"width": 1000, "height": 800}}
        for license_code in ("cc-by-nc", "cc-by-sa", "cc-by-nd", "cc-by-nc-sa", "", None):
            self.assertFalse(importer.permitted_photo(dict(photo, license_code=license_code)))
        for license_code in ("cc0", "cc-by"):
            self.assertTrue(importer.permitted_photo(dict(photo, license_code=license_code)))
        self.assertFalse(importer.permitted_photo(dict(photo, hidden=True)))
        self.assertFalse(importer.permitted_photo(dict(photo, attribution="")))
        self.assertFalse(importer.permitted_photo(dict(photo, flags=[{"flag": "copyright"}])))
        self.assertFalse(importer.permitted_photo(dict(photo, original_dimensions={"width": 100, "height": 100})))

    def test_conservative_educational_mapping(self):
        roots = lambda *ids: [{"id": n, "name": name, "rank": "kingdom"}
                             for values in importer.KINGDOM_ROOTS.values() for n, name in values.items() if n in ids]
        self.assertEqual(importer.educational_kingdom(roots(47686)), "Protista")
        self.assertEqual(importer.educational_kingdom(roots(48222)), "Protista")
        self.assertEqual(importer.educational_kingdom(roots(67333)), "Monera")
        self.assertEqual(importer.educational_kingdom(roots(151817)), "Monera")
        self.assertIsNone(importer.educational_kingdom(roots(67333, 47126)))
        self.assertIsNone(importer.educational_kingdom([{"id": 67333, "name": "Other", "rank": "kingdom"}]))
        self.assertIsNone(importer.educational_kingdom([]))

    def test_tree_counts_provenance_and_photo_hashes(self):
        nodes = json.loads((PROJECT / "resources/data/taxonomy.json").read_text(encoding="utf-8"))
        index = {node["id"]: node for node in nodes}
        self.assertEqual(len(index), len(nodes))
        seen_sources, counts = set(), dict.fromkeys(importer.KINGDOM_ROOTS, 0)
        for item in self.species:
            counts[item["legacy_educational_kingdom"]] += 1
            parent = ""
            visited = set()
            for node_id in item["taxonomy"]:
                self.assertNotIn(node_id, visited)
                visited.add(node_id)
                node = index[node_id]
                self.assertEqual(node["parent_id"], parent)
                parent = node_id
            self.assertEqual(index[parent]["name"], item["name"])
            if item["source"] == "inaturalist":
                identity = (item["source"], item["source_taxon_id"])
                self.assertNotIn(identity, seen_sources); seen_sources.add(identity)
                photo = item["photo"]
                self.assertIn(photo["license"], importer.ALLOWED_LICENSES)
                self.assertEqual(hashlib.sha256((PROJECT / photo["path"]).read_bytes()).hexdigest(), photo["sha256"])
                if item["domain"] == "Bacteria":
                    self.assertFalse(photo["visual_quiz"])
        self.assertEqual(counts, {"Animalia": 42, "Plantae": 20, "Fungi": 20, "Protista": 20, "Monera": 20})
        self.assertEqual(len(seen_sources), 103)

    def test_reimport_is_idempotent_and_offline(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            shutil.copytree(PROJECT / "resources/data", root / "resources/data")
            shutil.copytree(PROJECT / "resources/images", root / "resources/images")
            (root / "docs").mkdir()
            (root / "resources.qrc").write_text('<RCC><qresource prefix="/"/></RCC>', encoding="utf-8")
            before = json.loads((root / "resources/data/species.json").read_text(encoding="utf-8"))
            with mock.patch.object(importer, "ROOT", root), mock.patch.object(importer.Api, "get", side_effect=AssertionError("Network forbidden")), contextlib.redirect_stdout(io.StringIO()):
                report = importer.import_catalog(20, offline=True)
            after = json.loads((root / "resources/data/species.json").read_text(encoding="utf-8"))
            self.assertEqual(before, after)
            self.assertTrue(all(v["added"] == 0 and v["shortfall"] == 0 for v in report["kingdoms"].values()))

    def test_unreviewed_or_changed_photo_cannot_be_visual_question(self):
        with tempfile.TemporaryDirectory() as directory:
            with mock.patch.object(importer, "ROOT", pathlib.Path(directory)):
                record = {"source": "inaturalist", "source_taxon_id": "1", "educational_kingdom": "Animalia",
                          "photo": {"id": "2", "visual_quiz": True}}
                importer.apply_photo_reviews([record])
                self.assertFalse(record["photo"]["visual_quiz"])

    def test_review_is_bound_to_photo_identity_and_verified_source(self):
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            (root / 'resources/data').mkdir(parents=True)
            (root / 'resources/data/photo_reviews.json').write_text(json.dumps({'1': {'photo_id':'2','approved_ranks':['genus']}}))
            with mock.patch.object(importer, 'ROOT', root):
                record={'id':1,'source_taxon_id':'1','photo':{'id':'2','path':'resources/images/2.jpg','source_verified':True}}
                importer.apply_photo_reviews([record])
                self.assertEqual(record['photo']['approved_ranks'],['genus'])
                record['photo']['source_verified']=False
                importer.apply_photo_reviews([record])
                self.assertEqual(record['photo']['approved_ranks'],[])
                record['photo'].update(id='3',source_verified=True)
                importer.apply_photo_reviews([record])
                self.assertEqual(record['photo']['approved_ranks'],[])

    def test_every_photo_is_local_licensed_and_registered(self):
        import xml.etree.ElementTree as ET
        resources={e.text for e in ET.parse(PROJECT/'resources.qrc').findall('.//file')}
        reviews=json.loads((PROJECT/'resources/data/photo_reviews.json').read_text(encoding='utf8'))
        for species in self.species:
            photo=species.get('photo')
            if not photo: continue
            self.assertIn(photo['path'],resources)
            self.assertEqual(hashlib.sha256((PROJECT/photo['path']).read_bytes()).hexdigest(),photo['sha256'])
            if photo.get('license_status') == 'source_declared_version_unverified':
                import content_governance as governance
                metadata = governance.permission_provenance(PROJECT)[photo['id']]
                self.assertIsNone(photo['license_url'])
                self.assertIsNone(photo['license_version'])
                self.assertTrue(governance.source_declared_photo_license(photo, metadata, PROJECT))
            else:
                self.assertEqual(photo['license_url'],importer.LICENSE_URLS[photo['license']])
            if photo.get('role')=='supporting':
                self.assertEqual(species['domain'],'Bacteria')
                self.assertEqual(photo['supporting_for'],species['name'])
            else:self.assertEqual(photo['scientific_name'],species['name'])
            self.assertTrue(photo['source_verified'])
            if photo['approved_ranks']:
                self.assertNotEqual(photo['author'],'no rights reserved')
                review=reviews[species.get('source_taxon_id') or 'legacy:'+str(species['id'])]
                self.assertEqual(review['photo_id'],photo['id'])
                self.assertEqual(review['approved_ranks'],photo['approved_ranks'])

    def test_education_generation_is_offline_and_reproducible(self):
        expected=json.loads((PROJECT/'resources/data/education.json').read_text(encoding='utf8'))
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            (root/'resources/data').mkdir(parents=True)
            for name in ('species.json','taxonomy.json','species_profiles.json','hint_education.json','curation_reviews.json',
                         'photo_provenance.json','content_policy.json','education.json','phylogeny.json','progressive_hints.json','group_education.json'):
                shutil.copyfile(PROJECT/'resources/data'/name,root/'resources/data'/name)
            shutil.copytree(PROJECT/'resources/compliance', root/'resources/compliance')
            for relative in ('scripts/review_hints.py','src/application/EducationalContent.cpp','src/domain/Hint.h',
                             'src/infrastructure/content/ReviewStatus.h'):
                target=root/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PROJECT/relative,target)
            with mock.patch.object(education,'ROOT',root), mock.patch.object(importer.Api,'get',side_effect=AssertionError('Network forbidden')), contextlib.redirect_stdout(io.StringIO()):
                actual=education.build()
            self.assertEqual(actual,expected)

    def test_group_redaction_draft_binds_every_occurrence_and_invalidates_only_group_notes(self):
        import content_governance as governance
        import review_hints
        report=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_text(encoding='utf8'))
        packet=report['educational_regularization']['group_batch']
        draft_path=PROJECT/packet['proposed_content_path']
        self.assertEqual(hashlib.sha256(draft_path.read_bytes()).hexdigest(),packet['proposed_file_sha256'])
        draft=json.loads(draft_path.read_bytes())
        self.assertEqual(len(packet['proposed_changes']),68)
        seen=set()
        for change in packet['proposed_changes']:
            self.assertNotIn(change['pointer'],seen);seen.add(change['pointer'])
            value=draft
            for part in change['pointer'].split('/')[1:]:
                value=value[int(part)] if isinstance(value,list) else value[part]
            self.assertEqual(hashlib.sha256(value.encode()).hexdigest(),change['text_sha256'])
            self.assertNotEqual(change['previous_text_sha256'],change['text_sha256'])
            self.assertTrue(change['scientific_sources'])
            for reference in change['scientific_sources']:
                self.assertEqual(hashlib.sha256((PROJECT/reference['path']).read_bytes()).hexdigest(),reference['sha256'])
                self.assertTrue(reference['locator'])
        if packet['state']=='draft':
            self.assertIsNone(packet['reviewer']);self.assertIsNone(packet['reviewed_at'])
            self.assertFalse(packet['active_content_changed'])
            self.assertEqual(hashlib.sha256((PROJECT/packet['target']).read_bytes()).hexdigest(),packet['previous_file_sha256'])
        else:
            self.assertEqual(packet['state'],'applied_after_explicit_confirmation')
            self.assertEqual((PROJECT/packet['target']).read_bytes(),draft_path.read_bytes())
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            shutil.copytree(PROJECT/'resources/data',root/'resources/data')
            shutil.copytree(PROJECT/'resources/compliance',root/'resources/compliance')
            for relative in ('scripts/review_hints.py','src/application/EducationalContent.cpp','src/domain/Hint.h',
                             'src/infrastructure/content/ReviewStatus.h'):
                target=root/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PROJECT/relative,target)
            if packet['state']!='draft':
                # Restore the real pre-confirmation fixture in this temporary
                # workspace. The actual ledger is never edited by this test.
                for relative in (packet['target'],'resources/data/education.json',
                                 'resources/data/progressive_hints.json','resources/compliance/review_history.jsonl'):
                    (root/relative).write_bytes(self.historical_bytes(packet['base_commit'],relative))
            before=education.build(root,persist=False)
            recipes=review_hints.prepare(root)
            governance.save(root/packet['target'],draft)
            after=education.build(root,persist=False)
            changed={key for key in before['notes'] if before['notes'][key]!=after['notes'][key]}
            self.assertEqual(len(changed),38)
            self.assertTrue(all(key.startswith('note:') for key in changed))
            for key in changed:
                self.assertEqual(after['notes'][key]['governance']['state'],'needs_review')
                self.assertFalse(after['notes'][key]['governance']['usable'])
                self.assertFalse(after['notes'][key]['verified'])
                self.assertIsNone(after['notes'][key]['reviewed_on'])
            self.assertEqual(before['species'],after['species'])
            self.assertEqual(before['taxons'],after['taxons'])
            self.assertEqual(before['group_hints'],after['group_hints'])
            self.assertEqual(recipes,review_hints.prepare(root))
            governance.save(root/'resources/data/education.json',after)
            self.assertEqual(after,education.build(root,persist=False))

    @at_catalog_commit('1dc3c75')
    def test_confirmed_group_batch_is_scoped_and_changed_wording_requires_new_review(self):
        import content_governance as governance
        import rights_investigation
        packet=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())['educational_regularization']['group_batch']
        self.assertEqual(packet['state'],'applied_after_explicit_confirmation')
        reference=packet['confirmation_evidence']
        self.assertEqual(hashlib.sha256((PROJECT/reference['path']).read_bytes()).hexdigest(),reference['sha256'])
        proof=json.loads((PROJECT/reference['path']).read_bytes())
        self.assertEqual(proof['file_sha256'],packet['proposed_file_sha256'])
        self.assertEqual(proof['editorial_confirmation']['statement'],'Aprovo editorialmente essa versão')
        self.assertEqual(proof['license_confirmation']['statement'],'Confirmo a aplicação delimitada ao lote')
        self.assertFalse(proof['legal_or_scientific_certification'])
        inventory=rights_investigation.educational_text_inventory(PROJECT)
        self.assertEqual(inventory['rights_metadata']['technical_errors'],[])
        self.assertEqual(rights_investigation.text_resource_issues(inventory,packet['target']),[])
        assets=governance.load(PROJECT/'resources/compliance/asset_rights.json')
        self.assertIsNone(assets['own_material_licenses']['texts'])
        for asset in assets['assets']:
            if asset['kind']!='text_authorship': continue
            self.assertEqual(asset['permission']['distribution_allowed'],asset['path']==packet['target'])
        current=education.build(PROJECT,persist=False)
        recorded={row['key']:row for row in packet['recorded_decisions']}
        self.assertEqual(len(recorded),38)
        events={row['event_sha256']:row for row in governance.read_ledger(PROJECT)}
        for key,receipt in recorded.items():
            note=current['notes'][key]
            self.assertEqual(note['governance']['state'],'approved_editorial')
            self.assertEqual(note['governance']['review_kind'],'editorial')
            self.assertFalse(note['verified']);self.assertIsNone(note['reviewed_on'])
            event=events[receipt['event_sha256']]
            self.assertEqual(event['confirmation_evidence'],reference)
            self.assertEqual(event['content_sha256'],note['governance']['content_sha256'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            shutil.copytree(PROJECT/'resources/data',root/'resources/data')
            shutil.copytree(PROJECT/'resources/compliance',root/'resources/compliance')
            for relative in ('scripts/review_hints.py','src/application/EducationalContent.cpp','src/domain/Hint.h',
                             'src/infrastructure/content/ReviewStatus.h'):
                target=root/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PROJECT/relative,target)
            original_ledger=(root/'resources/compliance/review_history.jsonl').read_bytes()
            groups=governance.load(root/packet['target'])
            groups['definitions'][0][4]+=' Synthetic change in a temporary fixture.'
            governance.save(root/packet['target'],groups)
            changed=education.build(root,persist=False)
            self.assertEqual(changed['notes']['note:0:intro']['governance']['state'],'needs_review')
            self.assertFalse(changed['notes']['note:0:intro']['governance']['usable'])
            self.assertEqual(changed['species']['82'],current['species']['82'])
            for key in recorded:
                if key!='note:0:intro': self.assertEqual(changed['notes'][key],current['notes'][key])
            self.assertEqual((root/'resources/compliance/review_history.jsonl').read_bytes(),original_ledger)

    def test_profile_redaction_drafts_preserve_other_topics_and_do_not_inherit_reviews(self):
        import content_governance as governance
        import review_hints
        report=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())
        packet=report['educational_regularization']['profile_openstax_batch']
        # Exercise invalidation from the exact pre-confirmation baseline, even
        # after the separately confirmed batch has been incorporated.
        base=packet['promotion']['proposal_git_commit']
        source_bytes=self.historical_bytes(base,packet['target'])
        self.assertEqual(hashlib.sha256(source_bytes).hexdigest(),packet['previous_file_sha256'])
        profiles=json.loads(source_bytes)
        replacements=json.loads(json.dumps(profiles))
        pointers=set()
        for change in packet['proposed_changes']:
            pointer=change['pointer'];self.assertNotIn(pointer,pointers);pointers.add(pointer)
            _,sid,field=pointer.split('/')
            self.assertNotEqual(sid,'82')
            self.assertEqual(change['scientific_name'],profiles[sid]['scientific_name'])
            self.assertEqual(governance.digest(profiles[sid][field]),change['previous_note_sha256'])
            proposed=change['proposed_note']
            self.assertEqual(governance.digest(proposed),change['proposed_note_sha256'])
            self.assertEqual(hashlib.sha256(proposed['text'].encode()).hexdigest(),change['text_sha256'])
            self.assertNotEqual(change['previous_text_sha256'],change['text_sha256'])
            self.assertFalse(proposed['verified']);self.assertIsNone(proposed['reviewed_on'])
            for reference in change['scientific_sources']:
                evidence_path=PROJECT/reference['path']
                self.assertEqual(hashlib.sha256(evidence_path.read_bytes()).hexdigest(),reference['sha256'])
                evidence=json.loads(evidence_path.read_bytes())
                self.assertTrue(reference['locator'])
                self.assertFalse(evidence['scientific_approval'])
                self.assertEqual(evidence['license_check']['license'],'CC-BY-4.0')
            replacements[sid][field]=proposed
        self.assertEqual(len(pointers),34)
        self.assertEqual(replacements['82'],profiles['82'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            shutil.copytree(PROJECT/'resources/data',root/'resources/data')
            shutil.copytree(PROJECT/'resources/compliance',root/'resources/compliance')
            for relative in ('scripts/review_hints.py','src/application/EducationalContent.cpp','src/domain/Hint.h',
                             'src/infrastructure/content/ReviewStatus.h'):
                target=root/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PROJECT/relative,target)
            for relative in (packet['target'],'resources/data/education.json','resources/data/progressive_hints.json',
                             'resources/compliance/review_history.jsonl','resources/compliance/asset_rights.json'):
                (root/relative).write_bytes(self.historical_bytes(base,relative))
            before=education.build(root,persist=False)
            before_recipes=review_hints.prepare(root)
            governance.save(root/packet['target'],replacements)
            after=education.build(root,persist=False)
            changed={key for key in before['notes'] if before['notes'][key]!=after['notes'][key]}
            expected={'species:'+p.split('/')[1]+':'+p.split('/')[2] for p in pointers}
            # The existing morphological alias also reflects the changed source note.
            expected.add('species:110:morphological_hint')
            self.assertEqual(changed,expected)
            for key in changed:
                self.assertFalse(after['notes'][key]['governance']['usable'])
                self.assertEqual(after['notes'][key]['governance']['state'],'needs_review')
            self.assertEqual(before['species']['82'],after['species']['82'])
            self.assertEqual(before['taxons'],after['taxons'])
            self.assertEqual(before['group_hints'],after['group_hints'])
            self.assertEqual(before['group_curiosities'],after['group_curiosities'])
            self.assertFalse(after['automated_pedagogy']['usable'])
            # An explicit automated regeneration refreshes only input hashes and
            # the affected appearance recipe. Assistance grades stay unchanged.
            after_recipes=review_hints.prepare(root)
            for sid,old in before_recipes['species'].items():
                current=after_recipes['species'][sid]
                if sid!='110':
                    self.assertEqual(old,current)
                else:
                    self.assertEqual(len(old['recipes']),len(current['recipes']))
                    for old_recipe,new_recipe in zip(old['recipes'],current['recipes']):
                        for key,value in old_recipe.items():
                            if key not in {'text','source_text','source_url','source_title','locator'}:
                                self.assertEqual(value,new_recipe[key])
            review_hints.evaluate(root)
            regenerated=education.build(root,persist=False)
            self.assertTrue(regenerated['automated_pedagogy']['usable'])
            self.assertEqual(regenerated['species']['82'],before['species']['82'])
            for key in changed:
                self.assertFalse(regenerated['notes'][key]['governance']['usable'])

    @at_catalog_commit('1dc3c75')
    def test_confirmed_profile_batch_restores_only_approved_notes_and_keeps_rights_partial(self):
        import content_governance as governance
        import rights_investigation
        report=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())
        packet=report['educational_regularization']['profile_openstax_batch']
        self.assertEqual(packet['state'],'applied_after_explicit_confirmation')
        ref=packet['confirmation_evidence']
        self.assertEqual(hashlib.sha256((PROJECT/ref['path']).read_bytes()).hexdigest(),ref['sha256'])
        proof=json.loads((PROJECT/ref['path']).read_bytes())
        self.assertEqual(governance.digest(proof['proposed_changes']),packet['proposal_fingerprint'])
        self.assertEqual(proof['editorial_confirmation']['statement'],'Aprovo editorialmente esta versão completa')
        self.assertEqual(proof['license_confirmation']['statement'],'Confirmo a aplicação delimitada a este lote')
        self.assertFalse(proof['legal_or_scientific_certification'])
        profiles=governance.load(PROJECT/packet['target'])
        baseline=json.loads(self.historical_bytes(packet['promotion']['proposal_git_commit'],packet['target']))
        expected=set()
        for change in packet['proposed_changes']:
            _,sid,field=change['pointer'].split('/')
            self.assertEqual(profiles[sid][field],change['proposed_note'])
            expected.add((sid,field))
        self.assertEqual({(sid,k) for sid,p in profiles.items() for k,v in p.items() if v!=baseline[sid][k]},expected)
        inventory=rights_investigation.educational_text_inventory(PROJECT)
        self.assertEqual(inventory['rights_metadata']['technical_errors'],[])
        self.assertEqual(rights_investigation.text_resource_issues(inventory,packet['target']),['text_component_rights_pending'])
        rows=[r for r in inventory['rights_metadata']['records']
              if any(b['file']==packet['target'] for b in r.get('bindings',[]))]
        self.assertEqual(sorted((r['declared_metadata']['investigation_state'],len(r['bindings'])) for r in rows),
                         [('documented',34),('inconclusive',805)])
        current=education.build(PROJECT,persist=False)
        receipts={r['key']:r for r in packet['recorded_decisions']}
        self.assertEqual(len(receipts),35)
        events={e['event_sha256']:e for e in governance.read_ledger(PROJECT)}
        for key,receipt in receipts.items():
            note=current['notes'][key]
            self.assertEqual(note['governance']['state'],'approved_editorial')
            self.assertFalse(note['verified']);self.assertIsNone(note['reviewed_on'])
            self.assertEqual(events[receipt['event_sha256']]['confirmation_evidence'],ref)
            self.assertEqual(receipt['content_sha256'],note['governance']['content_sha256'])
        for sid in {sid for sid,_ in expected}:
            self.assertEqual(current['species'][sid]['governance']['education']['state'],'needs_review')
            self.assertFalse(current['species'][sid]['governance']['education']['usable'])
        self.assertFalse(current['notes']['species:110:morphological_hint']['assistance']['governance']['usable'])
        old_education=json.loads(self.historical_bytes(packet['promotion']['proposal_git_commit'],'resources/data/education.json'))
        def scientific_values(value):
            # Later, separately confirmed technical wording changes assessments
            # only. Their exact fields/grades are checked by the dedicated batch
            # regression; this historical profile test preserves every other field.
            if isinstance(value,dict):return {k:scientific_values(v) for k,v in value.items()
                                              if k not in {'assistance','taxonomic_assistance'}}
            if isinstance(value,list):return [scientific_values(v) for v in value]
            return value
        self.assertEqual(scientific_values(current['species']['82']),scientific_values(old_education['species']['82']))
        self.assertEqual(scientific_values({k:v for k,v in current['notes'].items() if k not in receipts}),
                         scientific_values({k:v for k,v in old_education['notes'].items() if k not in receipts}))
        assets=governance.load(PROJECT/'resources/compliance/asset_rights.json')
        self.assertIsNone(assets['own_material_licenses']['texts'])
        for asset in assets['assets']:
            if asset['path'] in {packet['target'],'resources/data/progressive_hints.json'}:
                self.assertFalse(asset['permission']['distribution_allowed'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            shutil.copytree(PROJECT/'resources/data',root/'resources/data')
            shutil.copytree(PROJECT/'resources/compliance',root/'resources/compliance')
            ledger=(root/'resources/compliance/review_history.jsonl').read_bytes()
            changed_profiles=governance.load(root/packet['target'])
            changed_profiles['41']['nutrition']['text']+=' Synthetic fixture change.'
            governance.save(root/packet['target'],changed_profiles)
            changed=education.build(root,persist=False)
            self.assertEqual(changed['notes']['species:41:nutrition']['governance']['state'],'needs_review')
            self.assertFalse(changed['notes']['species:41:nutrition']['governance']['usable'])
            for key in receipts:
                if key!='species:41:nutrition': self.assertEqual(changed['notes'][key],current['notes'][key])
            self.assertEqual((root/'resources/compliance/review_history.jsonl').read_bytes(),ledger)
            self.assertTrue(rights_investigation.educational_text_inventory(root)['rights_metadata']['technical_errors'])

    def test_profile_batch_revision_is_bound_to_history_and_exact_review_document(self):
        import content_governance as governance
        report=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())
        packet=report['educational_regularization']['profile_openstax_batch']
        revision=packet['proposal_revision']
        previous=json.loads(self.historical_bytes(revision['base_commit'],'docs/SANEAMENTO_ISSUE_2.json'))
        original=previous['educational_regularization']['profile_openstax_batch']['proposed_changes']
        changes=packet['proposed_changes']
        self.assertEqual(governance.digest(original),revision['previous_fingerprint'])
        self.assertEqual(governance.digest(changes),revision['final_fingerprint'])
        self.assertEqual(packet['proposal_fingerprint'],revision['final_fingerprint'])
        view=(PROJECT/'docs/propostas/REVISAO_FICHAS_LOTE_2.md').read_text(encoding='utf8')
        self.assertIn('`'+revision['final_fingerprint']+'`',view)
        self.assertEqual(len({c['pointer'].split('/')[1] for c in changes}),28)
        self.assertEqual(len({c['draft_kind'] for c in changes}),9)
        # A proposal revision cannot silently widen its original organism/field scope.
        self.assertEqual([c['pointer'] for c in changes],[c['pointer'] for c in original])
        changed={c['pointer'] for before,c in zip(original,changes) if before!=c}
        self.assertEqual(changed,set(revision['changed_pointers']))
        self.assertEqual(len(changed),9)
        self.assertFalse(revision['human_approval_recorded'])
        self.assertFalse(revision['specialist_review_performed'])
        for change in changes:
            self.assertIn(change['proposed_note']['text'],view)
            check=revision['checks_by_wording'][change['draft_kind']]
            self.assertTrue(check['evidence']);self.assertTrue(check['limitations'])
            self.assertIn(check['reason'],view)
        morphology=next(c for c in changes if c['pointer']=='/110/morphology')
        old_morphology=next(c for c in original if c['pointer']=='/110/morphology')
        self.assertEqual(morphology,old_morphology)
        self.assertIn('DSM 107014',morphology['proposed_note']['scope_name'])
        self.assertIn('não confirma',morphology['proposed_note']['text'])
        # The replacement fungal reference needs both independent factual scopes.
        fungal=next(c for c in changes if c['draft_kind']=='fungal_mycelium')
        ids={json.loads((PROJECT/ref['path']).read_bytes())['license_check']['pmcid']
             for ref in fungal['scientific_sources']}
        self.assertEqual(ids,{'PMC5638950','PMC9315710'})
        self.assertIn('PMC6909477',packet['source_evidence'])

    @at_catalog_commit('1dc3c75')
    def test_issue2_technical_wording_changes_only_bound_rationales(self):
        import copy
        import content_governance as governance
        proposal=json.loads((PROJECT/'docs/propostas/REDACAO_TECNICA_ISSUE_2.json').read_bytes())
        self.assertEqual(governance.digest(proposal['wordings']),proposal['fingerprint'])
        self.assertEqual(len(proposal['wordings']),15)
        self.assertIn(proposal['fingerprint'],(PROJECT/'docs/propostas/REDACAO_TECNICA_ISSUE_2.md').read_text(encoding='utf8'))
        self.assertFalse(proposal['human_approval_recorded'])
        originals={name:self.historical_bytes(proposal['base_commit'],name) for name in proposal['source_files']}
        for name,raw in originals.items():
            self.assertEqual(hashlib.sha256(raw).hexdigest(),proposal['source_files'][name])
        data={name:json.loads(raw) for name,raw in originals.items() if name.endswith('.json')}
        changed=copy.deepcopy(data);seen=set()
        for wording in proposal['wordings']:
            self.assertEqual(hashlib.sha256(wording['proposed_text'].encode('utf8')).hexdigest(),wording['text_sha256'])
            for binding in wording['occurrences']:
                key=(binding['file'],binding['pointer']);self.assertNotIn(key,seen);seen.add(key)
                parts=binding['pointer'].strip('/').split('/');node=changed[binding['file']]
                for part in parts[:-1]:node=node[int(part)] if isinstance(node,list) else node[part]
                field=int(parts[-1]) if isinstance(node,list) else parts[-1]
                self.assertEqual(hashlib.sha256(node[field].encode('utf8')).hexdigest(),binding['previous_text_sha256'])
                self.assertIn(parts[-1],{'rationale','limitations'})
                node[field]=wording['proposed_text']
        self.assertEqual(len(seen),1147)
        script=originals['scripts/review_hints.py'].decode('utf8')
        self.assertEqual(len(proposal['generator_literal_replacements']),4)
        for replacement in proposal['generator_literal_replacements']:
            self.assertEqual(script.count(replacement['previous_text']),1)
            script=script.replace(replacement['previous_text'],replacement['proposed_text'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            for folder in ('resources/data','resources/compliance'):
                shutil.copytree(PROJECT/folder,root/folder)
            for name in ('src/application/EducationalContent.cpp','src/domain/Hint.h'):
                (root/name).parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PROJECT/name,root/name)
            # Compare both versions inside the same isolated root. No production
            # file, review event or permission is written by this regression.
            for name,value in data.items():governance.save(root/name,value)
            before=governance.records(root,education.build(root,persist=False))
            governance.save(root/'resources/data/hint_education.json',changed['resources/data/hint_education.json'])
            target=root/'scripts/review_hints.py';target.parent.mkdir(parents=True)
            namespace={'__name__':'proposal_regression','__file__':str(target)}
            exec(compile(script,str(target),'exec'),namespace)
            generated=namespace['evaluate'](root)
            self.assertEqual(generated['species'],changed['resources/data/progressive_hints.json']['species'])
            for field in ('rationale','limitations'):
                self.assertEqual(generated['evaluation'][field],changed['resources/data/progressive_hints.json']['evaluation'][field])
            governance.save(root/'resources/data/progressive_hints.json',generated)
            after=governance.records(root,education.build(root,persist=False))
            self.assertEqual({k:v['content_sha256'] for k,v in before.items() if v['kind']!='assistance'},
                             {k:v['content_sha256'] for k,v in after.items() if v['kind']!='assistance'})

    @at_catalog_commit('1dc3c75')
    def test_confirmed_technical_wording_preserves_science_grades_and_partial_rights(self):
        import content_governance as governance
        import rights_investigation
        report=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())
        application=report['remaining_text_regularization']['technical_wording_application']
        ref=application['confirmation_evidence'];raw=(PROJECT/ref['path']).read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(),ref['sha256'])
        proof=json.loads(raw);proposal=json.loads((PROJECT/proof['proposal_document']).read_bytes())
        self.assertEqual(proof['wordings'],proposal['wordings'])
        self.assertEqual(proof['proposal_fingerprint'],governance.digest(proof['wordings']))
        self.assertEqual(proof['editorial_confirmation']['statement'],'Aprovo editorialmente esse lote')
        self.assertEqual(proof['license_confirmation']['statement'],'Confirmo a aplicação delimitada ao lote')
        self.assertFalse(any(proof['limits'].values()))
        self.assertEqual(proof['generator_literal_replacements'],proposal['generator_literal_replacements'])
        source_files={name:json.loads((PROJECT/name).read_bytes()) for name in proof['source_files'] if name.endswith('.json')}
        seen=set()
        for wording in proof['wordings']:
            for binding in wording['occurrences']:
                node=source_files[binding['file']]
                for part in binding['pointer'].strip('/').split('/'):
                    node=node[int(part)] if isinstance(node,list) else node[part]
                self.assertEqual(node,wording['proposed_text']);seen.add((binding['file'],binding['pointer']))
        self.assertEqual(len(seen),1147)
        def without_rationale(value):
            if isinstance(value,dict):return {k:without_rationale(v) for k,v in value.items() if k!='rationale'}
            if isinstance(value,list):return [without_rationale(v) for v in value]
            return value
        previous_hints=json.loads(self.historical_bytes(proof['base_commit'],'resources/data/hint_education.json'))
        self.assertEqual(without_rationale(source_files['resources/data/hint_education.json']),without_rationale(previous_hints))
        previous_recipes=json.loads(self.historical_bytes(proof['base_commit'],'resources/data/progressive_hints.json'))
        self.assertEqual(without_rationale(source_files['resources/data/progressive_hints.json']['species']),
                         without_rationale(previous_recipes['species']))
        current=education.build(PROJECT,persist=False)
        previous_education=json.loads(self.historical_bytes(proof['base_commit'],'resources/data/education.json'))
        old=governance.records(PROJECT,previous_education);new=governance.records(PROJECT,current)
        self.assertEqual({k:v['content_sha256'] for k,v in old.items() if v['kind']!='assistance'},
                         {k:v['content_sha256'] for k,v in new.items() if v['kind']!='assistance'})
        self.assertEqual((PROJECT/'resources/compliance/review_history.jsonl').read_bytes(),
                         self.historical_bytes(proof['base_commit'],'resources/compliance/review_history.jsonl'))
        inventory=rights_investigation.educational_text_inventory(PROJECT)
        self.assertEqual(inventory['rights_metadata']['technical_errors'],[])
        rows=[r for r in inventory['rights_metadata']['records']
              if any(e.get('path')==ref['path'] for e in r['declared_metadata'].get('evidence',[]))]
        self.assertEqual(len(rows),15);self.assertEqual(sum(len(r['bindings']) for r in rows),1147)
        self.assertFalse(any(r['rights_issues'] for r in rows))
        assets=governance.load(PROJECT/'resources/compliance/asset_rights.json')
        self.assertIsNone(assets['own_material_licenses']['texts']);self.assertIsNone(assets['own_material_licenses']['collection'])
        for asset in assets['assets']:
            if asset['path'] in source_files:self.assertFalse(asset['permission']['distribution_allowed'])

    def test_education_rejects_wrong_identity_missing_text_and_regional_status(self):
        original=json.loads((PROJECT/'resources/data/species_profiles.json').read_text(encoding='utf8'))
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory)
            (root/'resources/data').mkdir(parents=True)
            shutil.copyfile(PROJECT/'resources/data/species.json',root/'resources/data/species.json')
            for field,value in [('scientific_name','Unrelated replacement'),('summary',dict(original['126']['summary'],text='')),('conservation',dict(original['126']['conservation'],scope_name='Índia'))]:
                profiles=json.loads(json.dumps(original))
                profiles['126'][field]=value
                (root/'resources/data/species_profiles.json').write_text(json.dumps(profiles),encoding='utf8')
                with mock.patch.object(education,'ROOT',root),self.assertRaises(ValueError):
                    education.load_profiles()

    @at_catalog_commit('afabb70186cf99906f6148585e09749243626db5')
    def test_consolidated_confirmation_covers_exact_texts_without_approving_aggregates(self):
        import content_governance as governance
        import rights_investigation as rights
        assets=governance.load(PROJECT/'resources/compliance/asset_rights.json')
        grants=[a for a in assets['assets'] if a.get('permission',{}).get('scope')=='documented_educational_expression_only']
        self.assertEqual({a['path'] for a in grants},{'resources/data/species_profiles.json',
            'resources/data/hint_education.json','resources/data/progressive_hints.json'})
        reference=grants[0]['permission']
        raw=(PROJECT/reference['evidence_path']).read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(),reference['evidence_sha256'])
        proof=json.loads(raw)
        self.assertEqual(proof['kind'],'educational_consolidated_text_confirmation')
        self.assertEqual(proof['editorial_confirmation']['statement'],'Aprovo editorialmente o lote consolidado')
        self.assertEqual(proof['license_confirmation']['statement'],'Confirmo a aplicação delimitada ao lote')
        proposal=governance.load(PROJECT/proof['proposal']['path'])
        self.assertEqual(hashlib.sha256((PROJECT/proof['proposal']['path']).read_bytes()).hexdigest(),proof['proposal']['sha256'])
        self.assertEqual(governance.digest(proof['conference_scope']),proof['conference_fingerprint'])
        self.assertEqual(proposal['conference_scope'],proof['conference_scope'])
        self.assertEqual(governance.digest(proposal['changes']),proof['source_replacement_fingerprint'])
        self.assertEqual(len(proposal['changes']),887)
        self.assertEqual(len(proof['occurrences']),1153)
        seen=set()
        for binding in proof['occurrences']+proposal['protected_occurrences']:
            data=governance.load(PROJECT/binding['file'])
            for part in binding['pointer'].strip('/').split('/'):
                data=data[int(part)] if isinstance(data,list) else data[part]
            self.assertEqual(hashlib.sha256(data.encode('utf8')).hexdigest(),binding['text_sha256'])
            key=(binding['file'],binding['pointer']);self.assertNotIn(key,seen);seen.add(key)
        inventory=rights.educational_text_inventory(PROJECT)
        self.assertEqual(inventory['rights_metadata']['technical_errors'],[])
        self.assertFalse(any(r['rights_issues'] for r in inventory['rights_metadata']['records']))
        for asset in grants:
            self.assertTrue(asset['permission']['distribution_allowed'])
            self.assertEqual(rights.text_resource_issues(inventory,asset['path']),[])
        self.assertIsNone(assets['own_material_licenses']['texts'])
        self.assertIsNone(assets['own_material_licenses']['collection'])
        ledger=PROJECT/'resources/compliance/review_history.jsonl'
        self.assertTrue(ledger.read_bytes().startswith(self.historical_bytes('1dc3c75',ledger.relative_to(PROJECT).as_posix())))
        events=governance.read_ledger(PROJECT)
        self.assertEqual(len(events),1047)
        records=governance.records(PROJECT)
        for event in events[76:]:
            self.assertEqual(event['state'],'approved_editorial')
            self.assertEqual(records[event['key']]['kind'],'claim')
            self.assertEqual(records[event['key']]['content_sha256'],event['content_sha256'])
            self.assertEqual(event['confirmation_evidence']['path'],reference['evidence_path'])
        generated=education.build(PROJECT,persist=False)
        self.assertEqual(generated,governance.load(PROJECT/'resources/data/education.json'))
        for entry in generated['species'].values():
            self.assertEqual(entry['governance']['education']['state'],'needs_review')
            self.assertFalse(entry['governance']['education']['usable'])
        for event in events[76:]:
            self.assertEqual(generated['notes'][event['key']]['governance']['state'],'approved_editorial')

    def test_extracted_group_prose_matches_the_audited_pre_refactor_values(self):
        import content_governance as governance
        packet=json.loads((PROJECT/'docs/SANEAMENTO_ISSUE_2.json').read_bytes())['educational_regularization']['group_batch']
        # This is an extraction regression, not a requirement to keep legacy
        # wording active after a separately approved replacement.
        historical=json.loads(self.historical_bytes(packet['base_commit'],packet['target']))
        definitions, notes = historical['definitions'], historical['plain_notes']
        self.assertEqual(len(definitions), 19)
        self.assertEqual(len(notes), 15)
        # Preserve the extraction baseline, allowing only the verified bibliographic correction.
        # load_group_content returns fresh data: restoring the historical label here never edits a resource.
        euglenida = next(item for item in definitions if item[0] == 'Euglenida')
        self.assertEqual(euglenida[2], 'https://pmc.ncbi.nlm.nih.gov/articles/PMC7052976/')
        self.assertEqual(euglenida[3],
                         'Publicação científica — BMC Biology (2020), DOI: 10.1186/s12915-020-0754-1')
        euglenida[3] = 'Publicação científica — Open Biology'
        # Canonical digest measured before extraction from e2df447; no duplicated prose fixture.
        self.assertEqual(governance.digest({'definitions': definitions, 'plain_notes': notes}),
                         '6a41c03329d295cf1f7966fe110cf3cef0844bbf71cab41f7ba8dffc2e3a5e5e')

    def test_group_loader_uses_only_the_selected_workspace_and_rejects_invalid_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            with self.assertRaises(FileNotFoundError):
                education.load_group_content(root)
            folder = root / 'resources/data'; folder.mkdir(parents=True)
            source = folder / education.GROUP_SOURCE
            valid = {'schema_version': 1, 'definitions': [
                ['Synthetic', 'genus', 'https://example.org/evidence', 'Synthetic source',
                 'Synthetic prose only', '', '']], 'plain_notes': {'Synthetic': ['', '']}}
            source.write_text(json.dumps(valid), encoding='utf8')
            self.assertEqual(education.load_group_content(root), (valid['definitions'], valid['plain_notes']))
            invalids = [[], dict(valid, schema_version=2), dict(valid, definitions=valid['definitions'] * 2),
                        dict(valid, plain_notes={'Missing': ['', '']}), dict(valid, plain_notes={'Synthetic': ['one']}),
                        dict(valid, definitions=[['incomplete']])]
            for value in invalids:
                source.write_text(json.dumps(value), encoding='utf8')
                with self.assertRaises(ValueError):
                    education.load_group_content(root)

    def test_hint_generation_rejects_mismatched_species_and_taxonomy_as_etymology(self):
        original=json.loads((PROJECT/'resources/data/hint_education.json').read_text(encoding='utf8'))
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory);(root/'resources/data').mkdir(parents=True)
            for name in ('species.json','taxonomy.json','species_profiles.json','curation_reviews.json','phylogeny.json','group_education.json'):
                shutil.copyfile(PROJECT/'resources/data'/name,root/'resources/data'/name)
            for invalid_identity in (True,False):
                hints=json.loads(json.dumps(original))
                if invalid_identity:hints['135']['scientific_name']='Unrelated species'
                else:hints['135']['etymology_hints']['genus']['source_url']='https://www.inaturalist.org/taxa/1'
                (root/'resources/data/hint_education.json').write_text(json.dumps(hints),encoding='utf8')
                with mock.patch.object(education,'ROOT',root),self.assertRaises(ValueError):education.build()
                self.assertFalse((root/'resources/data/education.json').exists())

    def test_independent_review_states_and_catalog_audit(self):
        sys.path.insert(0,str(PROJECT/'scripts'))
        import audit_catalog
        result=audit_catalog.audit(PROJECT)
        self.assertEqual(result['total_species'],122)
        self.assertEqual(result['valid_photo_records'],122)
        self.assertEqual(result['competitive_pool'],80)
        # Current usable assertions come from hash-bound reviews, not raw flags.
        self.assertEqual(result['verified_etymology'],49)
        self.assertEqual(result['verified_morphology'],104)
        self.assertEqual(result['incompatible_photo_licenses'],[])
        self.assertEqual(result['errors'],[])
        self.assertNotIn('20',result['eligible_ids_by_rank']['species'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory);(root/'resources/data').mkdir(parents=True)
            for name in ('species.json','taxonomy.json','species_profiles.json','hint_education.json','curation_reviews.json','phylogeny.json','group_education.json'):
                shutil.copyfile(PROJECT/'resources/data'/name,root/'resources/data'/name)
            path=root/'resources/data/curation_reviews.json'
            reviews=json.loads(path.read_text(encoding='utf8'))
            reviews['135']['etymology']='pending'
            path.write_text(json.dumps(reviews),encoding='utf8')
            with mock.patch.object(education,'ROOT',root),contextlib.redirect_stdout(io.StringIO()):
                data=education.build()
            self.assertEqual(data['species']['135']['review']['etymology'],'pending')
            self.assertEqual(data['species']['135']['review']['photo'],'approved')
            reviews['135']['photo']='invented'
            path.write_text(json.dumps(reviews),encoding='utf8')
            with mock.patch.object(education,'ROOT',root),self.assertRaises(ValueError):education.build()

    def test_research_triage_preserves_manual_queries_and_never_approves_a_name(self):
        spec=importlib.util.spec_from_file_location("research",PROJECT/'scripts/research_etymology.py')
        research=importlib.util.module_from_spec(spec);spec.loader.exec_module(research)
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory);folder=root/'resources/data';folder.mkdir(parents=True)
            species=[{'id':1,'name':'Example species','educational_kingdom':'Animalia'}]
            hints={'1':{'scientific_name':'Example species'}}
            profiles={'1':{'morphology':{'source_url':'https://example.org/reference'}}}
            prior={'species':{'1':{'scientific_name':'Example species','search_queries':['manual original-description query']}}}
            for name,data in [('species.json',species),('hint_education.json',hints),('species_profiles.json',profiles),('etymology_research.json',prior)]:
                (folder/name).write_text(json.dumps(data),encoding='utf8')
            with mock.patch.object(research,'ROOT',root),mock.patch.object(research,'inspect',return_value={'retrieved':True,'etymology_keyword':True}),contextlib.redirect_stdout(io.StringIO()):
                report=research.research()
            result=report['species']['1']
            self.assertEqual(json.loads((folder/'etymology_research.json').read_text(encoding='utf8')),prior)
            self.assertEqual(result['search_queries'],prior['species']['1']['search_queries'])
            self.assertEqual(result['verified_ranks'],[])
            self.assertEqual(result['pending_parts'],['genus','species'])
            self.assertEqual(json.loads((folder/'hint_education.json').read_text(encoding='utf8')),hints)

    def test_missing_offline_cache_and_failed_import_leave_catalog_untouched(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "resources/data").mkdir(parents=True)
            content = (PROJECT / "tests/fixtures/legacy_species.json").read_bytes()
            path = root / "resources/data/species.json"
            path.write_bytes(content)
            with mock.patch.object(importer, "ROOT", root), contextlib.redirect_stdout(io.StringIO()):
                with self.assertRaises(RuntimeError):
                    importer.import_catalog(20, offline=True)
            self.assertEqual(path.read_bytes(), content)
            self.assertFalse((root / "resources/data/taxonomy.json").exists())


if __name__ == "__main__":
    unittest.main()
