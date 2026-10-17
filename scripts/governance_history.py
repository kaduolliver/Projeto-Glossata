"""Read-only governance receipts and comparison with an explicitly selected Git base.

The caller supplies the trust root. Neither hashes nor Git author fields authenticate
reviewers. No decisions, signatures, trust keys or historical records are created.
"""
import hashlib
from pathlib import Path
import re
import subprocess

from governance_storage import LEDGERS, chain_summary, governance_lock

PREFIX = 'resources/compliance/'
BASELINE = PREFIX + 'review_baseline.json'
PATHS = tuple(PREFIX + name for name in sorted(LEDGERS)) + (BASELINE,)


def assurances():
    return {'integrity_scope': 'Internal SHA-256 linkage; optional byte-prefix comparison with caller-selected Git commit.',
            'reviewer_identity': 'Declared, not authenticated by these checks.',
            'authorship_authenticated': False, 'digital_signatures_verified': False,
            'independent_review_verified': False, 'specialist_qualification_verified': False,
            'trust_root': 'Local files and verifier; for historical comparison, independently selected and retained Git commit.'}


def _git(root, *arguments):
    # Replacement refs must not silently substitute a caller-pinned object.
    result = subprocess.run(['git', '--no-replace-objects', '-C', str(root), *arguments],
                            capture_output=True, check=False, timeout=15)
    if result.returncode:
        raise ValueError('Git reference unavailable, invalid or unrelated; retain/fetch the exact base and inspect manually.')
    return result.stdout


def _reference(root, base):
    if not re.fullmatch(r'[0-9a-f]{40}|[0-9a-f]{64}', base):
        raise ValueError('Historical reference must be a full commit object ID, not a mutable branch/tag name.')
    if _git(root, 'cat-file', '-t', base).strip() != b'commit':
        raise ValueError('Historical reference must identify a commit.')
    _git(root, 'merge-base', '--is-ancestor', base, 'HEAD')
    entries = _git(root, 'ls-tree', '-z', base, '--', *PATHS).split(b'\0')
    found = {}
    for entry in filter(None, entries):
        metadata, path = entry.split(b'\t', 1)
        mode, kind, _ = metadata.split()
        if kind != b'blob' or mode not in (b'100644', b'100755'):
            raise ValueError('Historical governance paths must be regular files.')
        name = path.decode('utf8')
        found[name] = _git(root, 'show', f'{base}:{name}')
    return found


def _summary(path, data):
    if path == BASELINE:
        return {'present': data is not None, 'bytes_sha256': hashlib.sha256(data).hexdigest() if data is not None else None}
    return dict(chain_summary(data or b''), present=data is not None)


def audit_history(root, base=None):
    """Coherent receipt under the same cooperative lock as all legitimate writers.

    No base is an explicit integrity-only audit, suitable for offline source ZIPs.
    Missing/invalid requested commits fail closed; absent ledgers are not approvals.
    """
    root = Path(root)
    result = {'schema_version': 1, 'assurances': assurances(), 'errors': [], 'files': {},
              'comparison': {'status': 'not_requested', 'commit': None}, 'consistent': False}
    try:
        with governance_lock(root):
            reference = None
            if base in ('0' * 40, '0' * 64):
                result['comparison']['status'] = 'unavailable_initial_push'
            elif base:
                result['comparison'] = {'status': 'not_checked', 'commit': base}
                reference = _reference(root, base)
            for relative in PATHS:
                path = root / relative
                try:
                    if path.is_symlink(): raise ValueError('Governance paths must be regular files.')
                    data = path.read_bytes() if path.exists() else None
                    entry = {'current': _summary(relative, data)}
                    result['files'][relative] = entry
                    if reference is not None:
                        previous = reference.get(relative)
                        entry['reference'] = _summary(relative, previous)
                        preserved = (previous is None or (data is not None and
                                     (data == previous if relative == BASELINE else data.startswith(previous))))
                        entry['reference_preserved'] = preserved
                        if not preserved:
                            result['errors'].append({'path': relative, 'error': 'historical_prefix_changed_or_removed'})
                except (ValueError, OSError):
                    # Do not export malformed event bodies, reviewer details or evidence.
                    result['errors'].append({'path': relative, 'error': 'invalid_or_unreadable_history'})
            if reference is not None:
                result['comparison']['status'] = 'mismatch' if result['errors'] else 'prefix_preserved'
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        result['errors'].append({'error': 'reference_or_snapshot_unavailable', 'detail': str(error)})
    result['consistent'] = not result['errors']
    return result
