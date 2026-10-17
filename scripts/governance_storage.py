"""Local crash-safe storage primitives; no scientific or rights decisions."""
from contextlib import contextmanager
import errno
import hashlib
import json
import math
import os
from pathlib import Path
import tempfile
import threading
import time

LEDGERS = frozenset({'review_history.jsonl', 'permission_history.jsonl', 'dependency_reviews.jsonl'})
LOCK_RELATIVE = 'resources/compliance/.governance.lock'
_ownership = threading.local()


def canonical(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode('utf8')


def digest(value):
    return hashlib.sha256(canonical(value)).hexdigest()


def _chain(data):
    events = [json.loads(line) for line in data.decode('utf8').splitlines() if line.strip()]
    previous = ''
    for event in events:
        if not isinstance(event, dict): raise ValueError('Invalid governance event')
        body = {k: v for k, v in event.items() if k != 'event_sha256'}
        if event.get('previous_sha256') != previous or event.get('event_sha256') != digest(body):
            raise ValueError('Histórico de revisão alterado ou cadeia incompleta.')
        previous = event['event_sha256']
    return events


def read_chain(path):
    path = Path(path)
    return _chain(path.read_bytes()) if path.exists() else []


def _checkpoint(stage, path):
    """Fault-injection seam. Production never interprets these notifications."""


def atomic_write(path, data, *, before_replace=None):
    """Sync a sibling temporary file, then replace; never rewrite a live file."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(prefix='.' + path.name + '.', suffix='.part',
                                         dir=path.parent, delete=False) as output:
            temporary = Path(output.name)
            _checkpoint('temporary_opened', path)
            output.write(data)
            _checkpoint('temporary_written', path)
            output.flush()
            os.fsync(output.fileno())
            _checkpoint('temporary_synced', path)
        if before_replace is not None: before_replace()
        os.replace(temporary, path)
        _checkpoint('replaced', path)
        # Windows supports file fsync, but not this POSIX directory operation.
        if os.name != 'nt':
            descriptor = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(descriptor)
            finally:
                os.close(descriptor)
        _checkpoint('directory_synced', path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def _lock_path(root):
    return (Path(root) / LOCK_RELATIVE).resolve()


def require_governance_lock(root):
    """Private write paths cannot silently bypass the public synchronization."""
    path = _lock_path(root)
    key = os.path.normcase(str(path))
    lease = getattr(_ownership, 'leases', {}).get(key)
    if lease is None or lease[0] != os.getpid():
        raise RuntimeError('A governance writer must hold the common lock')
    actual, held = path.stat(), os.fstat(lease[1].fileno())
    if (actual.st_dev, actual.st_ino) != (held.st_dev, held.st_ino):
        raise RuntimeError('Governance lock identity changed; refusing to write')


@contextmanager
def governance_lock(root, timeout=0):
    """OS lock beside the ledgers; same-thread nesting, bounded optional wait.

    Keep the file/inode: unlinking a lock allows two independent lock owners.
    The old exclusive-create sentinel is never assumed stale or removed.
    """
    if not isinstance(timeout, (int, float)) or not math.isfinite(timeout) or not 0 <= timeout <= 30:
        raise ValueError('Governance lock timeout must be between 0 and 30 seconds')
    path = _lock_path(root)
    key = os.path.normcase(str(path))
    leases = getattr(_ownership, 'leases', None)
    if leases is None: leases = _ownership.leases = {}
    if key in leases and leases[key][0] == os.getpid():
        require_governance_lock(root)
        yield
        return
    if (Path(root) / 'build/curation/decision.lock').exists():
        raise FileExistsError('Legacy decision.lock exists; verify its owner before removing it.')
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a+b') as handle:
        if handle.seek(0, os.SEEK_END) == 0:
            handle.write(b'0')
            handle.flush()
        handle.seek(0)
        deadline = time.monotonic() + timeout
        while True:
            try:
                if os.name == 'nt':
                    import msvcrt
                    msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                else:
                    import fcntl
                    fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                break
            except OSError as error:
                if error.errno not in {errno.EACCES, errno.EAGAIN, errno.EBUSY, errno.EDEADLK}:
                    raise
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise FileExistsError('Governance writer busy; lock wait expired without writing.') from error
                time.sleep(min(.05, remaining))
        leases[key] = (os.getpid(), handle)
        try:
            require_governance_lock(root)
            yield
        finally:
            leases.pop(key, None)
            handle.seek(0)
            if os.name == 'nt':
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(handle.fileno(), fcntl.LOCK_UN)


def append_event(root, ledger, body, expected_previous_sha256):
    """The only ledger commit path: serialize, verify tip, preserve prefix, replace."""
    if ledger not in LEDGERS: raise ValueError('Unknown governance ledger')
    if {'previous_sha256', 'event_sha256'}.intersection(body):
        raise ValueError('Chain hashes belong to the common writer')
    with governance_lock(root):
        path = Path(root) / 'resources/compliance' / ledger
        prefix = path.read_bytes() if path.exists() else b''
        events = _chain(prefix)
        tip = events[-1]['event_sha256'] if events else ''
        if tip != expected_previous_sha256:
            raise ValueError('Governance history tip changed; compare the latest decision before retrying')
        _checkpoint('history_read', path)
        event = dict(body, previous_sha256=tip)
        event['event_sha256'] = digest(event)

        def verify_tip():
            require_governance_lock(root)
            current = path.read_bytes() if path.exists() else b''
            current_events = _chain(current)
            current_tip = current_events[-1]['event_sha256'] if current_events else ''
            if current_tip != tip or current != prefix:
                raise ValueError('Governance history changed before commit; no events were replaced')

        separator = b'\n' if prefix and not prefix.endswith(b'\n') else b''
        atomic_write(path, prefix + separator + (json.dumps(event, ensure_ascii=False) + '\n').encode('utf8'),
                     before_replace=verify_tip)
        return event
