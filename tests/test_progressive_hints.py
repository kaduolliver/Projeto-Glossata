"""Pedagogical evaluations remain distinct from human/scientific permissions."""
import copy
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from content_governance import ROOT, load, save, digest
from review_hints import evaluate, valid


class ProgressiveHints(unittest.TestCase):
    def setUp(self):
        self.folder=tempfile.TemporaryDirectory();self.addCleanup(self.folder.cleanup)
        self.root=Path(self.folder.name)
        for relative in ['resources/data','resources/compliance/review_baseline.json',
                         'scripts/review_hints.py','src/application/EducationalContent.cpp','src/domain/Hint.h',
                         'src/infrastructure/content/ReviewStatus.h']:
            source=ROOT/relative;target=self.root/relative;target.parent.mkdir(parents=True,exist_ok=True)
            if source.is_dir():shutil.copytree(source,target)
            else:shutil.copyfile(source,target)

    def test_idempotent_explicit_evaluation_does_not_renew_or_simulate_human(self):
        baseline=(self.root/'resources/compliance/review_baseline.json').read_bytes()
        first=evaluate(self.root);second=evaluate(self.root)
        self.assertEqual(first,second);self.assertTrue(valid(first,self.root))
        ev=first['evaluation'];self.assertEqual(ev['state'],'approved_automated')
        self.assertEqual(ev['actor_type'],'automated');self.assertNotIn('reviewer',ev)
        self.assertEqual(len(first['species']),122)
        self.assertEqual(baseline,(self.root/'resources/compliance/review_baseline.json').read_bytes())

    def test_changed_source_photo_or_policy_invalidates_without_renewal(self):
        original=evaluate(self.root);when=original['evaluation']['reviewed_at']
        path=self.root/'resources/data/species.json';data=load(path);data[0]['photo']['id']='changed';save(path,data)
        self.assertFalse(valid(original,self.root));self.assertEqual(original['evaluation']['reviewed_at'],when)
        updated=evaluate(self.root);self.assertTrue(valid(updated,self.root))
        self.assertNotEqual(original['evaluation']['content_sha256'],updated['evaluation']['content_sha256'])
        (self.root/'src/domain/Hint.h').write_text('changed policy',encoding='utf8')
        self.assertFalse(valid(updated,self.root))

    def test_scientific_use_policy_change_invalidates_automatic_assessment(self):
        original=evaluate(self.root)
        (self.root/'src/infrastructure/content/ReviewStatus.h').write_text('synthetic change',encoding='utf8')
        self.assertFalse(valid(original,self.root))

    def test_expiry_and_recipe_tampering_are_not_approval(self):
        original=evaluate(self.root);expired=copy.deepcopy(original)
        expired['evaluation']['due_at']='2000-01-01T00:00:00+00:00';self.assertFalse(valid(expired,self.root))
        bad=copy.deepcopy(original);next(iter(bad['species'].values()))['scientific_name']='Unrelated'
        self.assertFalse(valid(bad,self.root))

    def test_unsafe_morphology_does_not_become_approved_by_relabeling(self):
        data=evaluate(self.root);hints=load(self.root/'resources/data/hint_education.json')
        count=0
        for sid,content in hints.items():
            m=content.get('morphological_hint',{})
            if m.get('assistance',{}).get('suitability')=='inadequate':
                count+=1
                self.assertFalse(any(r['category']=='morphological' for r in data['species'][sid]['recipes']))
        self.assertEqual(count,4)


if __name__=='__main__':unittest.main()
