"""Offline import and provenance regression tests; no requests to the live API."""
import contextlib
import hashlib
import importlib.util
import io
import json
import pathlib
import shutil
import sys
import tempfile
import unittest
from unittest import mock

PROJECT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT / "scripts"))
spec = importlib.util.spec_from_file_location("importer", PROJECT / "scripts/import_inaturalist.py")
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)
education_spec = importlib.util.spec_from_file_location("education", PROJECT / "scripts/build_education.py")
education = importlib.util.module_from_spec(education_spec)
with mock.patch.dict(sys.modules, {"import_inaturalist": importer}):
    education_spec.loader.exec_module(education)


class ImportTests(unittest.TestCase):
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
                         'photo_provenance.json','content_policy.json','education.json','phylogeny.json'):
                shutil.copyfile(PROJECT/'resources/data'/name,root/'resources/data'/name)
            shutil.copytree(PROJECT/'resources/compliance', root/'resources/compliance')
            with mock.patch.object(education,'ROOT',root), mock.patch.object(importer.Api,'get',side_effect=AssertionError('Network forbidden')), contextlib.redirect_stdout(io.StringIO()):
                actual=education.build()
            self.assertEqual(actual,expected)

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

    def test_hint_generation_rejects_mismatched_species_and_taxonomy_as_etymology(self):
        original=json.loads((PROJECT/'resources/data/hint_education.json').read_text(encoding='utf8'))
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory);(root/'resources/data').mkdir(parents=True)
            for name in ('species.json','taxonomy.json','species_profiles.json','curation_reviews.json','phylogeny.json'):
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
        self.assertEqual(result['verified_etymology'],49)
        self.assertEqual(result['incompatible_photo_licenses'],[])
        self.assertEqual(result['errors'],[])
        self.assertNotIn('20',result['eligible_ids_by_rank']['species'])
        with tempfile.TemporaryDirectory() as directory:
            root=pathlib.Path(directory);(root/'resources/data').mkdir(parents=True)
            for name in ('species.json','taxonomy.json','species_profiles.json','hint_education.json','curation_reviews.json','phylogeny.json'):
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
