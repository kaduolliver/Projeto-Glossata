"""Offline import and provenance regression tests; no requests to the live API."""
import contextlib
import hashlib
import importlib.util
import io
import json
import pathlib
import shutil
import tempfile
import unittest
from unittest import mock

PROJECT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("importer", PROJECT / "scripts/import_inaturalist.py")
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)


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
            counts[item["educational_kingdom"]] += 1
            parent = "edu:" + item["educational_kingdom"]
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
                if item["educational_kingdom"] == "Monera":
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
                self.assertEqual(species['educational_kingdom'],'Monera')
                self.assertEqual(photo['supporting_for'],species['name'])
            else:self.assertEqual(photo['scientific_name'],species['name'])
            self.assertTrue(photo['source_verified'])
            if photo['approved_ranks']:
                self.assertNotEqual(photo['author'],'no rights reserved')
                review=reviews[species.get('source_taxon_id') or 'legacy:'+str(species['id'])]
                self.assertEqual(review['photo_id'],photo['id'])
                self.assertEqual(review['approved_ranks'],photo['approved_ranks'])

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
