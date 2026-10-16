"""Offline adversarial checks for the separately licensed presentation images."""
import copy
import hashlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
import exploration_media as media
import content_governance as governance


class ExplorationMediaTests(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory();self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name)
        for path in ['resources/data/exploration_photos.json','resources/data/taxonomy.json','resources/data/species.json']:
            target=self.root/path;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/path,target)
        self.data=governance.load(self.root/media.MANIFEST)
        for photo in self.data['photos'].values():
            for key in ['path','evidence_path']:
                target=self.root/photo[key];target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/photo[key],target)

    def persist(self): governance.save(self.root/media.MANIFEST,self.data)

    def test_real_corpus_rights_identity_size_and_no_private_metadata(self):
        report=media.audit(ROOT)
        self.assertEqual(report['errors'],[])
        self.assertEqual(report['total_photos'],8)
        self.assertLess(report['total_bytes'],300000)
        for photo in self.data['photos'].values():
            self.assertEqual(governance.image_privacy_findings(ROOT/photo['path']),[])
            self.assertIn('ImageDescription',governance.load(ROOT/photo['evidence_path'])['record']['metadata'])
        self.assertEqual(len(governance.load(ROOT/'resources/data/species.json')),122)

    def test_only_per_file_cc0_and_by_with_matching_versions_are_accepted(self):
        for name,url in [('CC0','https://creativecommons.org/publicdomain/zero/1.0/'),('CC BY 4.0','https://creativecommons.org/licenses/by/4.0/')]:
            self.assertIn(media.license_from({'LicenseShortName':{'value':name},'LicenseUrl':{'value':url}})[0],['cc0','cc-by'])
        for name in ['', 'CC BY-SA 4.0','CC BY-ND 4.0','CC BY-NC 4.0','Copyrighted','CC BY 3.0']:
            with self.subTest(name=name),self.assertRaises(ValueError):
                media.license_from({'LicenseShortName':{'value':name},'LicenseUrl':{'value':'https://creativecommons.org/licenses/by/4.0/'}})

    def test_denial_occurs_before_any_image_download(self):
        response={'query':{'pages':{'1':{'imageinfo':[{'extmetadata':{'LicenseShortName':{'value':'CC BY-SA 4.0'}}}]}}}}
        with patch.object(media,'fetch',return_value=json.dumps(response).encode()) as fetch:
            with self.assertRaises(ValueError):media.collect_commons('File:Rejected.jpg','rejected','Organism',self.root)
            self.assertEqual(fetch.call_count,1)
        self.assertFalse((self.root/'resources/images/exploration/rejected.jpg').exists())

    def test_tampered_license_credit_and_evidence_are_detected(self):
        photo=self.data['photos']['lion']
        for field,value,error in [('license_version','4.0','license_evidence_mismatch'),('author','Uncredited observer','credit_evidence_mismatch'),('source_revision_id',0,'photo_identity'),('evidence_sha256','0'*64,'permission_evidence_hash')]:
            with self.subTest(field=field):
                changed=dict(photo,**{field:value});self.assertIn(error,media.photo_errors(changed,self.root))
        path=self.root/photo['path'];path.write_bytes(b'not an image')
        self.assertIn('photo_hash',media.photo_errors(photo,self.root))

    def test_misidentified_representative_and_missing_scope_are_detected(self):
        self.data['bindings']['domain:Bacteria']='lion';self.persist()
        errors={e['error'] for e in media.audit(self.root)['errors']}
        self.assertIn('representative_identity_mismatch',errors);self.assertIn('micrograph_required',errors)
        self.data['photos']['sunflower'].pop('scope');self.persist()
        self.assertIn('representative_scope_missing',{e['error'] for e in media.audit(self.root)['errors']})

    def test_unknown_bindings_do_not_fabricate_photographs(self):
        self.assertNotIn('clade:Discoba',self.data['bindings'])
        self.data['bindings']['nonexistent:taxon']='lion';self.persist()
        self.assertIn('invalid_binding',{e['error'] for e in media.audit(self.root)['errors']})

    def test_generated_credit_fields_keep_actual_source_and_modifications(self):
        records=media.credit_records(self.root)
        self.assertEqual(records['lion']['author'],'Kevin Pluck')
        self.assertEqual(records['bacteria']['kind'],'micrograph')
        self.assertIn('colorizada',records['bacteria']['identification_note'])
        document=governance.render_credits({'notice':'test','photos':{},'exploration_photos':records})
        for record in records.values():
            self.assertIn(record['license_url'],document)
            self.assertIn(record['photo_url'],document)
            self.assertIn(record['modifications'],document)
            self.assertEqual(record['rights_issues'],[])


if __name__=='__main__':unittest.main()
