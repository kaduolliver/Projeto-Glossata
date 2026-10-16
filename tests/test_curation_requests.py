"""Public request generation: grouping, hashes and confidential evidence."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import content_governance as governance
import curation_requests as requests


class QueueTests(unittest.TestCase):
    def setUp(self):
        self.folder=tempfile.TemporaryDirectory();self.addCleanup(self.folder.cleanup)
        self.root=Path(self.folder.name);self.config=requests.settings()
        governance.save(self.root/'resources/data/species.json',[])
        governance.save(self.root/'resources/data/education.json',{'notes':{},'species':{}})
        self.audit={'scientific_findings':[], 'automated_pedagogy':{'usable':True}}

    def queue(self,audit=None):
        return requests.build_queue(self.root,audit or self.audit,{}, {},self.config)

    def test_stable_identity_grouping_and_duplicate_findings(self):
        a=copy.deepcopy(self.audit)
        f={'photo_id':'7','errors':['license_version_missing','license_version_missing']}
        a['photo_licensing']=[f,f]
        first=self.queue(a);second=self.queue(a)
        self.assertEqual(first['requests'],second['requests'])
        photos=next(r for r in first['requests'] if r['bucket']=='quiz-photos')
        self.assertEqual(len(photos['findings']),1)
        a['photo_licensing'].append({'photo_id':'8','errors':['license_version_missing']})
        changed=next(r for r in self.queue(a)['requests'] if r['bucket']=='quiz-photos')
        self.assertEqual(changed['id'],photos['id']);self.assertNotEqual(changed['fingerprint'],photos['fingerprint'])

    def test_raw_private_fields_urls_and_reasons_are_not_published(self):
        private='sensitive person@example.org / private coordinates 12.345,67.890'
        a=copy.deepcopy(self.audit)
        a['scientific_findings']=[{'key':'topic:7:taxonomy','reason':private,'severity':'high','state':'needs_review'}]
        a['technical_errors']=[{'error':'unnecessary_private_fields','private_location':private}]
        q=self.queue(a);raw=json.dumps(q)
        self.assertNotIn(private,raw);self.assertNotIn('person@example',raw)
        for r in q['requests']:
            self.assertNotIn(private,requests.render(r,self.config['repository'],'a'*40))

    def test_public_evidence_is_hashed_and_private_evidence_withheld(self):
        name='resources/data/species.json'
        public=requests.public_reference(self.root,name);self.assertEqual(len(public['sha256']),64)
        governance.save(self.root/name,[{'coordinates':[12,34]}])
        self.assertIsNone(requests.public_reference(self.root,name))
        self.assertIsNone(requests.public_reference(self.root,'../outside.json'))
        self.assertIsNone(requests.public_reference(self.root,'resources/compliance/private/permission.json'))
        self.assertIsNone(requests.public_reference(self.root,'C:/private/permission.json'))

    def test_individual_findings_are_bounded_in_issue_but_complete_in_queue(self):
        a=copy.deepcopy(self.audit)
        a['photo_licensing']=[{'photo_id':str(i),'errors':['license_version_missing']} for i in range(120)]
        r=next(r for r in self.queue(a)['requests'] if r['bucket']=='quiz-photos')
        self.assertEqual(len(r['findings']),120)
        body=requests.render(r,self.config['repository'],'a'*40,3)
        self.assertEqual(body.count('| `photo:'),3)
        self.assertIn('120 achados',body);self.assertIn('Lista resumida a 3',body)

    def test_priority_does_not_confuse_missing_documentation_with_forbidden_license(self):
        a=copy.deepcopy(self.audit);a['photo_licensing']=[{'photo_id':'1','errors':['license_version_missing']}]
        self.assertEqual(next(r for r in self.queue(a)['requests'] if r['bucket']=='quiz-photos')['priority'],1)
        a['photo_licensing'][0]['errors']=['license_outside_policy']
        self.assertEqual(next(r for r in self.queue(a)['requests'] if r['bucket']=='quiz-photos')['priority'],0)


if __name__=='__main__':unittest.main()
