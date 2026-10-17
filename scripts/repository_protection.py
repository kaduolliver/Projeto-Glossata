"""Validate the solo-maintainer proposal or inspect GitHub using GET only.

No apply command exists. A checked-in ruleset is not an activated protection.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PROPOSAL = ROOT / '.github/rulesets/main-solo.json'
REPOSITORY = 'kaduolliver/Projeto-Glossata'
ACTIONS_APP_ID = 15368  # Observed GitHub Actions app, not a token or secret.


def proposal_errors(value):
    """Project-specific safety contract, not a general GitHub schema validator."""
    errors = []
    if not isinstance(value, dict):
        return ['invalid_proposal_object']
    if value.get('target') != 'branch' or value.get('enforcement') != 'active':
        errors.append('proposal_must_target_active_branch_rules')
    if value.get('conditions') != {'ref_name': {'include': ['refs/heads/main'], 'exclude': []}}:
        errors.append('proposal_must_target_only_main_without_exclusions')
    if value.get('bypass_actors') != []:
        errors.append('no_permanent_bypass_actors')
    rules = value.get('rules', [])
    if not isinstance(rules, list) or any(not isinstance(r, dict) for r in rules):
        return errors + ['invalid_rule_list']
    types = [r.get('type') for r in rules]
    expected = {'deletion', 'non_fast_forward', 'pull_request', 'required_status_checks'}
    if set(types) != expected or len(types) != len(expected):
        errors.append('exactly_history_pr_and_technical_check_rules_required')
    by_type = {r.get('type'): r for r in rules}
    pr = by_type.get('pull_request', {}).get('parameters', {})
    if type(pr.get('required_approving_review_count')) is not int or pr.get('required_approving_review_count') != 0:
        errors.append('solo_maintainer_requires_zero_approvals')
    if pr.get('require_code_owner_review') is not False or pr.get('require_last_push_approval') is not False:
        errors.append('must_not_require_another_reviewer')
    if pr.get('dismiss_stale_reviews_on_push') is not False or pr.get('required_review_thread_resolution') is not True:
        errors.append('unexpected_review_conversation_policy')
    checks = by_type.get('required_status_checks', {}).get('parameters', {})
    if checks.get('required_status_checks') != [{'context': 'offline-validation', 'integration_id': ACTIONS_APP_ID}]:
        errors.append('only_offline_validation_from_github_actions_is_required')
    if checks.get('strict_required_status_checks_policy') is not True:
        errors.append('technical_check_must_cover_current_base')
    return errors


def read_api(path):
    """Never issue mutations or return raw CLI errors/credentials to reports."""
    if not path.startswith('repos/' + REPOSITORY + '/') and path != 'repos/' + REPOSITORY:
        raise ValueError('unexpected_repository_endpoint')
    result = subprocess.run(['gh', 'api', '--method', 'GET', path], capture_output=True,
                            text=True, encoding='utf-8', timeout=45)
    if result.returncode:
        raise ValueError('github_read_failed_check_auth_permissions_and_platform_support')
    try:
        return json.loads(result.stdout)
    except ValueError:
        raise ValueError('github_read_invalid_json') from None


def bounded_list(value):
    # Fail instead of claiming a complete view when pagination could be needed.
    if not isinstance(value, list) or len(value) >= 100:
        raise ValueError('github_list_invalid_or_incomplete')
    return value


def inspect_remote(api=read_api):
    base = 'repos/' + REPOSITORY
    meta = api(base)
    branch = api(base + '/branches/main')
    sha = branch['commit']['sha']
    if not re.fullmatch('[0-9a-f]{40}', sha) or branch.get('name') not in (None, 'main'):
        raise ValueError('unexpected_branch_identity')
    summaries = bounded_list(api(base + '/rulesets?per_page=100'))
    effective = bounded_list(api(base + '/rules/branches/main?per_page=100'))
    details = []
    for item in summaries:
        detail = api(base + '/rulesets/' + str(int(item['id'])))
        details.append({key: detail.get(key) for key in
                        ('id', 'name', 'target', 'enforcement', 'conditions', 'rules', 'bypass_actors')})
    checks = bounded_list(api(base + '/commits/' + sha + '/check-runs?per_page=100')['check_runs'])
    classic = None
    if branch.get('protected'):
        # A branch may be protected by rulesets alone; a classic 404 is not proof
        # that rulesets are absent. Inspect classic settings separately in the UI.
        classic = 'Check classic settings in Settings > Branches; not inferred from rulesets.'
    return {
        'schema_version': 1, 'queried_at': datetime.now(timezone.utc).isoformat(),
        'repository': REPOSITORY, 'activation_performed': False,
        'repository_state': {k: meta.get(k) for k in ('visibility', 'default_branch', 'archived')},
        'branch': {'name': 'main', 'protected': branch.get('protected'), 'head': sha},
        'rulesets': details, 'effective_rules': effective, 'classic_protection_note': classic,
        'actions_permissions': api(base + '/actions/permissions'),
        'workflow_permissions': api(base + '/actions/permissions/workflow'),
        'checks': [{'name': c['name'], 'conclusion': c.get('conclusion'),
                    'app_id': c['app']['id'], 'app_slug': c['app']['slug']} for c in checks],
        'notice': 'Read-only snapshot; missing privileges/API failures are not absence of protection. '
                  'Inspect bypasses and all matching rules before claiming enforcement.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['check', 'inspect'])
    parser.add_argument('--output', type=Path, default=ROOT / 'build/repository-protection-inspection.json')
    args = parser.parse_args()
    try:
        value = json.loads(PROPOSAL.read_text(encoding='utf-8'))
        errors = proposal_errors(value)
        if errors:
            print(json.dumps({'proposal_errors': errors})); return 1
        if args.action == 'check':
            print('Solo-maintainer proposal valid; remote activation NOT checked or performed.')
            return 0
        result = inspect_remote()
        result['proposal_sha256'] = hashlib.sha256(PROPOSAL.read_bytes()).hexdigest()
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')
        print(json.dumps(result, ensure_ascii=True))
        return 0
    except (ValueError, KeyError, TypeError, AttributeError, OSError, subprocess.SubprocessError) as error:
        print(json.dumps({'inspection': 'unknown_or_invalid', 'error_type': type(error).__name__}))
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
