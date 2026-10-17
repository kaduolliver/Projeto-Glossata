"""Temporary repositories exercise staged secrets, forced artifacts and source preservation."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from audit_repository import audit


class RepositoryAuditTests(unittest.TestCase):
    def setUp(self):
        folder=tempfile.TemporaryDirectory();self.addCleanup(folder.cleanup);self.root=Path(folder.name)
        subprocess.run(['git','init','-q'],cwd=self.root,check=True)

    def stage(self,name,content=b'fixture'):
        path=self.root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(content)
        subprocess.run(['git','add','-f','--',name],cwd=self.root,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)

    def test_forced_artifacts_and_credentials_are_rejected_without_values(self):
        for name in ['build/cache.json','package/test.exe','profile.sqlite','file.part','.env','id_ed25519']:
            self.stage(name)
        report=audit(self.root)
        self.assertEqual({x['path'] for x in report['findings']},{'build/cache.json','package/test.exe','profile.sqlite','file.part','.env','id_ed25519'})

    def test_source_photographs_credits_and_permission_evidence_are_preserved(self):
        for name in ['src/main.cpp','resources/images/photo.jpg','resources/compliance/permissions/evidence.json','docs/PHOTO_CREDITS.md','package/qt.conf','.env.example']:
            self.stage(name)
        self.assertEqual(audit(self.root)['findings'],[])

    def test_forced_runtime_governance_lock_is_rejected(self):
        self.stage('resources/compliance/.governance.lock',b'0')
        self.assertEqual(audit(self.root)['findings'],[
            {'path':'resources/compliance/.governance.lock','issue':'generated_or_local_artifact'}])

    def test_staged_secret_cannot_be_hidden_by_clean_worktree_replacement(self):
        token=('gh'+'p_'+'A'*36).encode();self.stage('config.txt',token)
        (self.root/'config.txt').write_text('safe unstaged replacement')
        report=audit(self.root)
        self.assertEqual(report['findings'],[{'path':'config.txt','issue':'github_token'}])
        self.assertNotIn(token.decode(),str(report))

    def test_private_keys_detected_without_logging_their_material(self):
        marker=('-----'+'BEGIN PRIVATE KEY'+'-----').encode();self.stage('unexpected.txt',marker)
        self.assertEqual(audit(self.root)['findings'][0]['issue'],'private_key')


if __name__=='__main__':unittest.main()
