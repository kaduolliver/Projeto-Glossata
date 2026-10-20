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
import exploration_discovery as discovery
import content_governance as governance


class ExplorationMediaTests(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory();self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name)
        for path in ['resources/data/exploration_photos.json','resources/data/taxonomy.json','resources/data/species.json']:
            target=self.root/path;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/path,target)
        self.data=governance.load(self.root/media.MANIFEST)
        for photo in self.data['photos'].values():
            paths=[photo['path'],photo['evidence_path']]
            if photo.get('scope',{}).get('classification_evidence'):paths.append(photo['scope']['classification_evidence'])
            for relative in paths:
                target=self.root/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/relative,target)

    def persist(self): governance.save(self.root/media.MANIFEST,self.data)

    def test_real_corpus_rights_identity_size_and_no_private_metadata(self):
        report=media.audit(ROOT)
        self.assertEqual(report['errors'],[])
        nodes=governance.load(ROOT/'resources/data/taxonomy.json')
        self.assertEqual(report['total_cards'],len(nodes)+1)
        self.assertEqual(report['bindings']+report['documented_pending'],len(nodes))
        self.assertEqual(len(report['coverage']),report['total_cards'])
        self.assertEqual(sum(row['id']=='special:mixed' for row in report['coverage']),1)
        self.assertLess(sum(p['bytes'] for p in self.data['photos'].values() if p.get('usage')!='home'),25_000_000)
        self.assertLess(sum(p['bytes'] for p in self.data['photos'].values() if p.get('usage')=='home'),8_000_000)
        for photo in self.data['photos'].values():
            self.assertEqual(governance.image_privacy_findings(ROOT/photo['path']),[])
            proof=governance.load(ROOT/photo['evidence_path'])['record']
            self.assertTrue(proof['metadata'].get('ImageDescription') or proof.get('identification_category'),photo['photo_url'])
        self.assertEqual(len(governance.load(ROOT/'resources/data/species.json')),122)

    def test_documented_coverage_matches_the_current_published_manifest(self):
        recorded=governance.load(ROOT/'docs/AUDITORIA_FOTOGRAFIAS.json')
        actual=media.audit(ROOT)
        for field in ['total_photos','bindings','total_cards','documented_pending','total_bytes','coverage','errors']:
            self.assertEqual(recorded[field],actual[field],field)

    def test_only_per_file_cc0_and_by_with_matching_versions_are_accepted(self):
        for name,url in [('CC0','https://creativecommons.org/publicdomain/zero/1.0/'),('CC BY 4.0','https://creativecommons.org/licenses/by/4.0/')]:
            self.assertIn(media.license_from({'LicenseShortName':{'value':name},'LicenseUrl':{'value':url}})[0],['cc0','cc-by'])
        for name in ['', 'CC BY-SA 4.0','CC BY-ND 4.0','CC BY-NC 4.0','Copyrighted','CC BY 3.0']:
            with self.subTest(name=name),self.assertRaises(ValueError):
                media.license_from({'LicenseShortName':{'value':name},'LicenseUrl':{'value':'https://creativecommons.org/licenses/by/4.0/'}})

    def test_html_credit_blocks_and_entities_keep_the_sources_meaning(self):
        self.assertEqual(media.plain('Author<br>Institution<p>Credit &amp; source</p>'),'Author Institution Credit & source')
        self.assertEqual(media.plain('Literal &amp;amp; entity'),'Literal &amp; entity')
        self.assertEqual(media.plain('Institution <span><img alt="Edit this record"></span>'),'Institution')

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

    def test_home_variants_require_same_source_and_native_resolution(self):
        base=self.data['photos']['monarch'];variant=self.data['photos']['home-monarch']
        self.assertTrue(media.matching_variant(base,variant,self.root))
        self.assertEqual(media.photo_errors(variant,self.root),[])
        for key in ['subject','photo_id','author','license_version','photo_url']:
            changed=dict(variant,**{key:'different'})
            self.assertFalse(media.matching_variant(base,changed,self.root),key)
        changed=dict(variant,width=4000)
        self.assertIn('unproved_native_resolution',media.photo_errors(changed,self.root))
        self.data['home_variants']['monarch']='home-lion';self.persist()
        self.assertIn('home_variant_identity',{e['error'] for e in media.audit(self.root)['errors']})

    def test_home_variant_does_not_replace_card_binding(self):
        base=next(k for k,v in self.data['bindings'].items() if v=='monarch')
        self.data['bindings'][base]='home-monarch';self.persist()
        self.assertIn('home_variant_used_for_card',{e['error'] for e in media.audit(self.root)['errors']})

    def test_home_collection_rejects_server_upscaling(self):
        info={'url':'https://upload.wikimedia.org/fixture.jpg','thumburl':'https://upload.wikimedia.org/fixture-thumb.jpg',
              'width':100,'height':100,'extmetadata':{'Artist':{'value':'Fixture author'},
              'LicenseShortName':{'value':'CC0'},'LicenseUrl':{'value':'https://creativecommons.org/publicdomain/zero/1.0/'}}}
        response={'query':{'pages':{'1':{'imageinfo':[info]}}}}
        raw=(self.root/self.data['photos']['lion']['path']).read_bytes()
        with patch.object(media,'fetch',side_effect=[json.dumps(response).encode(),raw]):
            with self.assertRaisesRegex(ValueError,'Upscaled'):
                media.collect_commons('File:Fixture.jpg','upscaled','Fixture',self.root,width=2560,usage='home')
        self.assertFalse((self.root/'resources/images/exploration/upscaled.jpg').exists())

    def test_misidentified_representative_and_missing_scope_are_detected(self):
        self.data['bindings']['domain:Bacteria']='lion';self.persist()
        errors={e['error'] for e in media.audit(self.root)['errors']}
        self.assertIn('representative_identity_mismatch',errors);self.assertIn('micrograph_required',errors)
        self.data['photos']['sunflower'].pop('scope');self.persist()
        self.assertIn('representative_scope_missing',{e['error'] for e in media.audit(self.root)['errors']})

    def test_unknown_bindings_do_not_fabricate_photographs(self):
        self.data['bindings']['nonexistent:taxon']='lion';self.persist()
        self.assertIn('invalid_binding',{e['error'] for e in media.audit(self.root)['errors']})

    def test_an_external_representative_cannot_impersonate_a_different_species(self):
        node=next(n for n in governance.load(self.root/'resources/data/taxonomy.json') if n['rank']=='species')
        self.data['bindings'][node['id']]='physarum'
        self.data['photos']['physarum']['scope']['taxon_ids'].append(node['id'])
        self.data.get('pending',{}).pop(node['id'],None);self.persist()
        self.assertIn('representative_species_mismatch',{e['error'] for e in media.audit(self.root)['errors']})

    def test_external_classification_must_match_its_proof_and_group(self):
        key=next(k for k,p in self.data['photos'].items() if p.get('scope',{}).get('classification_evidence'))
        photo=self.data['photos'][key]
        photo['scope']['classification_evidence_sha256']='0'*64
        self.assertIn('classification_evidence_hash',media.photo_errors(photo,self.root))

    def test_every_gap_requires_a_reason_and_attempted_representatives(self):
        node=next(iter(self.data['bindings']))
        self.data['bindings'].pop(node);self.data.get('pending',{}).pop(node,None);self.persist()
        self.assertIn('undocumented_photograph_gap',{e['error'] for e in media.audit(self.root)['errors']})
        self.data.setdefault('pending',{})[node]={'reason':'No suitable file in inspected results','attempted_subjects':['Source organism']};self.persist()
        self.assertNotIn('undocumented_photograph_gap',{e['error'] for e in media.audit(self.root)['errors']})
        self.data['bindings'][node]='lion';self.persist()
        self.assertIn('conflicting_coverage_state',{e['error'] for e in media.audit(self.root)['errors']})

    def test_coverage_preparation_uses_descendants_without_changing_species(self):
        species=(self.root/'resources/data/species.json').read_bytes()
        self.data['bindings']={};self.persist()
        data=discovery.complete_coverage(self.root)
        self.assertEqual((self.root/'resources/data/species.json').read_bytes(),species)
        self.assertEqual(set(data['bindings'])|set(data['pending']),{n['id'] for n in governance.load(self.root/'resources/data/taxonomy.json')})
        self.assertEqual(media.audit(self.root)['errors'],[])
        first=(self.root/media.MANIFEST).read_bytes()
        discovery.complete_coverage(self.root)
        self.assertEqual((self.root/media.MANIFEST).read_bytes(),first)

    def test_focal_points_must_stay_within_the_photograph(self):
        for value in [-0.01,1.01,'left']:
            self.assertIn('invalid_focus',media.photo_errors(dict(self.data['photos']['lion'],focus_x=value),self.root))

    def test_discovery_missing_cache_stays_offline_and_never_changes_published_manifest(self):
        before=(self.root/media.MANIFEST).read_bytes()
        with patch('urllib.request.OpenerDirector.open',side_effect=AssertionError('Unexpected network')):
            result=discovery.discover(self.root,offset=0,limit=1)
        row=next(iter(result['subjects'].values()))
        self.assertIn('No offline evidence',row['error'])
        self.assertEqual(row['candidates'],[])
        self.assertEqual((self.root/media.MANIFEST).read_bytes(),before)

    def test_invalid_identifiers_are_rejected_before_network_or_writes(self):
        with patch.object(media,'fetch',side_effect=AssertionError('Unexpected network')):
            for value in ['../outside','path/file','UPPERCASE','']:
                with self.assertRaises(ValueError):media.collect_commons('File:Test.jpg',value,'Test',self.root)

    def test_unsupported_source_category_is_rejected_before_image_download(self):
        info={'extmetadata':{'LicenseShortName':{'value':'CC0'},'LicenseUrl':{'value':'https://creativecommons.org/publicdomain/zero/1.0/'},'Artist':{'value':'Fixture author'}}}
        response={'query':{'pages':{'1':{'imageinfo':[info],'categories':[{'title':'Category:Different organism'}]}}}}
        with patch.object(media,'fetch',return_value=json.dumps(response).encode()) as fetch:
            with self.assertRaisesRegex(ValueError,'identification category'):
                media.collect_commons('File:Test.jpg','test','Fixture organism',self.root,identification_category='Category:Expected organism')
            self.assertEqual(fetch.call_count,1)

    def test_resource_sync_preserves_unrelated_rights_and_is_idempotent(self):
        source=self.root/'resources/compliance/asset_rights.json';source.parent.mkdir(parents=True,exist_ok=True)
        governance.save(source,{'assets':[{'path':'legacy/icon.png','state':'permission_pending'}]})
        (self.root/'resources.qrc').write_text('<RCC>\n    <qresource>\n    </qresource>\n</RCC>',encoding='utf8')
        discovery.sync_resources(self.root)
        once=(self.root/'resources.qrc').read_bytes()
        discovery.sync_resources(self.root)
        self.assertEqual(once,(self.root/'resources.qrc').read_bytes())
        assets=governance.load(source)['assets']
        self.assertEqual(assets[0],{'path':'legacy/icon.png','state':'permission_pending'})
        self.assertEqual(len(assets),len(self.data['photos'])+1)

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
