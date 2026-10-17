"""Prevent maintainer lockout, accidental release gates and token escalation."""
import copy
import json
from pathlib import Path
import re
import sys
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import repository_protection as protection


class ProtectionTests(unittest.TestCase):
    def setUp(self):
        self.proposal = json.loads(protection.PROPOSAL.read_text(encoding='utf-8'))

    def test_proposal_keeps_single_maintainer_operational(self):
        self.assertEqual(protection.proposal_errors(self.proposal), [])
        for key in ('required_approving_review_count', 'require_code_owner_review', 'require_last_push_approval'):
            changed = copy.deepcopy(self.proposal)
            pr = next(r['parameters'] for r in changed['rules'] if r['type'] == 'pull_request')
            pr[key] = 1 if key.endswith('count') else True
            self.assertTrue(protection.proposal_errors(changed), key)

    def test_release_gate_wrong_app_bypass_and_missing_history_protection_are_refused(self):
        for change in ('release', 'app', 'bypass', 'force_push', 'deletion', 'all_branches', 'exclude_main', 'strict'):
            value = copy.deepcopy(self.proposal)
            params = next(r['parameters'] for r in value['rules'] if r['type'] == 'required_status_checks')
            if change == 'release': params['required_status_checks'].append({'context': 'distribution-permissions'})
            elif change == 'app': params['required_status_checks'][0]['integration_id'] = 1
            elif change == 'bypass': value['bypass_actors'] = [{'actor_type': 'RepositoryRole', 'actor_id': 5, 'bypass_mode': 'always'}]
            elif change in ('force_push', 'deletion'):
                value['rules'] = [r for r in value['rules'] if r['type'] != ('non_fast_forward' if change == 'force_push' else 'deletion')]
            elif change == 'all_branches': value['conditions']['ref_name']['include'] = ['~ALL']
            elif change == 'exclude_main': value['conditions']['ref_name']['exclude'] = ['refs/heads/main']
            else: params['strict_required_status_checks_policy'] = False
            self.assertTrue(protection.proposal_errors(value), change)

    def test_remote_reader_uses_only_get_and_redacts_failures(self):
        result = mock.Mock(returncode=0, stdout='{"protected": false}')
        with mock.patch.object(protection.subprocess, 'run', return_value=result) as run:
            self.assertEqual(protection.read_api('repos/' + protection.REPOSITORY + '/branches/main'), {'protected': False})
            command = run.call_args.args[0]
            self.assertEqual(command[:4], ['gh', 'api', '--method', 'GET'])
        with mock.patch.object(protection.subprocess, 'run') as run:
            with self.assertRaises(ValueError): protection.read_api('repos/other/repository/rulesets')
            run.assert_not_called()
        result = mock.Mock(returncode=1, stderr='synthetic-private-error', stdout='')
        with mock.patch.object(protection.subprocess, 'run', return_value=result):
            with self.assertRaisesRegex(ValueError, '^github_read_failed') as error:
                protection.read_api('repos/' + protection.REPOSITORY)
            self.assertNotIn('synthetic-private-error', str(error.exception))

    def test_snapshot_does_not_claim_activation_and_permission_failure_is_unknown(self):
        base = 'repos/' + protection.REPOSITORY
        values = {base: {'visibility': 'public', 'default_branch': 'main'},
                  base + '/branches/main': {'protected': False, 'commit': {'sha': 'a' * 40}},
                  base + '/rulesets?per_page=100': [], base + '/rules/branches/main?per_page=100': [],
                  base + '/commits/' + 'a' * 40 + '/check-runs?per_page=100': {'check_runs': []},
                  base + '/actions/permissions': {'enabled': True},
                  base + '/actions/permissions/workflow': {'default_workflow_permissions': 'read'}}
        result = protection.inspect_remote(values.__getitem__)
        self.assertFalse(result['activation_performed'])
        self.assertFalse(result['branch']['protected'])
        with self.assertRaises(ValueError):
            protection.inspect_remote(mock.Mock(side_effect=ValueError('permission_denied')))
        with self.assertRaises(ValueError): protection.bounded_list([{}] * 100)

    def test_existing_remote_rules_are_reported_without_silently_applying_local_proposal(self):
        def api(path):
            if path.endswith('/branches/main'):
                return {'name': 'main', 'protected': True, 'commit': {'sha': 'b' * 40}}
            if path.endswith('/rulesets?per_page=100'): return [{'id': 42}]
            if path.endswith('/rules/branches/main?per_page=100'): return [{'type': 'deletion'}]
            if path.endswith('/rulesets/42'):
                return {'id': 42, 'target': 'branch', 'enforcement': 'active', 'rules': [{'type': 'deletion'}]}
            if '/check-runs?' in path: return {'check_runs': []}
            return {}
        result = protection.inspect_remote(api)
        self.assertTrue(result['branch']['protected'])
        self.assertEqual(result['effective_rules'], [{'type': 'deletion'}])
        self.assertEqual(result['rulesets'][0]['rules'], [{'type': 'deletion'}])
        self.assertIsNone(result['rulesets'][0]['bypass_actors'])  # Hidden/absent is unknown, not [].
        self.assertFalse(result['activation_performed'])

    def test_workflows_keep_unique_technical_check_read_tokens_and_no_privileged_pr_trigger(self):
        quality = (ROOT / '.github/workflows/quality.yml').read_text(encoding='utf-8')
        curation = (ROOT / '.github/workflows/curation.yml').read_text(encoding='utf-8')
        self.assertRegex(quality, r'(?m)^  offline-validation:\n')
        self.assertRegex(quality, r'(?m)^    name: offline-validation$')
        self.assertIn('python scripts/repository_protection.py check', quality)
        self.assertIn('python scripts/content_governance.py audit-history --base "$BASE_SHA"', quality)
        self.assertIn('github.event.pull_request.base.sha || github.event.before', quality)
        self.assertIn('build/governance-history.json', quality)
        for text in (quality, curation):
            self.assertNotIn('write-all', text)
            for permission in ('contents', 'pull-requests', 'actions', 'id-token', 'checks', 'statuses'):
                self.assertNotRegex(text, permission + r':\s*write')
            self.assertNotRegex(text, r'(?m)^  pull_request_target:')
            for action in re.findall(r'uses: ([^\s#]+)', text):
                self.assertRegex(action, r'@[0-9a-f]{40}$')
            self.assertEqual(text.count('uses: actions/checkout@'), text.count('persist-credentials: false'))
        self.assertRegex(quality, r'(?m)^permissions:\n  contents: read$')
        self.assertEqual(curation.count('issues: write'), 1)
        self.assertNotRegex(curation, r'(?m)^  offline-validation:')
        technical, release = quality.split('  distribution-permissions:', 1)
        self.assertNotRegex(technical.split('    steps:', 1)[0], r'(?m)^    if:')
        self.assertNotIn('distribution.py check', technical)
        self.assertIn('distribution.py check', release)
        # Required checks must run for documentation-only changes and PRs too.
        for entry in ('  push:', '  pull_request:', '  workflow_dispatch:'):
            self.assertIn(entry, quality)
        self.assertNotRegex(quality, r'(?m)^\s+(paths|paths-ignore):')
        for path in (ROOT / '.github/workflows').glob('*.yml'):
            if path.name != 'quality.yml':
                text = path.read_text(encoding='utf-8')
                self.assertNotRegex(text, r'(?m)^  offline-validation:|^    name: offline-validation$')

    def test_sensitive_paths_have_visible_owner_without_fictitious_review_requirement(self):
        owners = (ROOT / '.github/CODEOWNERS').read_text(encoding='utf-8')
        for path in ('/.github/', '/resources/compliance/', '/resources/data/', '/scripts/', '/VERSION.txt'):
            self.assertIn(path + ' @kaduolliver', owners)
        self.assertIn('/.github/', owners)  # CODEOWNERS covers itself too.


if __name__ == '__main__':
    unittest.main()
