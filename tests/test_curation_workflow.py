"""Security invariants of the public automation, independent of YAML parsing."""
import json
from pathlib import Path
import re
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import curation_github
import curation_requests

ROOT=Path(__file__).resolve().parents[1]


class AutomationTests(unittest.TestCase):
    def test_workflow_is_scheduled_manual_default_branch_and_not_public_event_execution(self):
        text=(ROOT/'.github/workflows/curation.yml').read_text(encoding='utf8')
        self.assertIn("cron: '17 12 * * 1'",text);self.assertIn('workflow_dispatch:',text)
        for trigger in ('pull_request_target:','issue_comment:','issues:','pull_request:'):
            self.assertNotRegex(text,r"(?m)^  "+re.escape(trigger))
        self.assertIn('github.event.repository.default_branch',text)
        self.assertIn('cancel-in-progress: false',text)
        self.assertIn('default: false',text)
    def test_permissions_no_code_writes_no_decisions_and_no_private_artifacts(self):
        text=(ROOT/'.github/workflows/curation.yml').read_text(encoding='utf8')
        self.assertEqual(text.count('issues: write'),1);self.assertNotIn('contents: write',text)
        self.assertNotIn('record-decision',text);self.assertNotIn('confirm-permission',text)
        self.assertNotIn('pull_request.body',text);self.assertNotIn('issue.body',text)
        self.assertNotIn('path: build/curation\n',text);self.assertNotIn('private/',text)
        for step in re.findall(r'uses: ([^\s#]+)',text):self.assertRegex(step,r'@[0-9a-f]{40}$')
    def test_forms_use_declared_labels_and_request_no_private_documents(self):
        allowed=curation_github.labels(curation_requests.settings())
        files=list((ROOT/'.github/ISSUE_TEMPLATE').glob('*.yml'))
        self.assertEqual(len(files),5)
        for path in files:
            if path.name=='config.yml':continue
            text=path.read_text(encoding='utf8');line=re.search(r'^labels: (\[.*\])$',text,re.M)
            self.assertTrue(line);self.assertTrue(set(json.loads(line[1])).issubset(allowed))
            self.assertIn('required: true',text);self.assertIn('privacidade',text)
            self.assertNotIn('upload',text.casefold())


if __name__=='__main__':unittest.main()
