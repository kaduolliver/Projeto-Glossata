"""Local crash-safe storage primitives; no scientific or rights decisions."""
from contextlib import contextmanager
import os
from pathlib import Path
import tempfile


def _checkpoint(stage, path):
    """Fault-injection seam. Production never interprets these notifications."""


def atomic_write(path, data):
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


@contextmanager
def governance_lock(root):
    """Nonblocking OS lock, released even if the process exits unexpectedly.

    Keep the file/inode: unlinking a lock allows two independent lock owners.
    The old exclusive-create sentinel is never assumed stale or removed.
    """
    folder = Path(root) / 'build/curation'
    folder.mkdir(parents=True, exist_ok=True)
    if (folder / 'decision.lock').exists():
        raise FileExistsError('Legacy decision.lock exists; verify its owner before removing it.')
    with (folder / 'governance.lock').open('a+b') as handle:
        if handle.seek(0, os.SEEK_END) == 0:
            handle.write(b'0')
            handle.flush()
        handle.seek(0)
        try:
            if os.name == 'nt':
                import msvcrt
                msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl
                fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError as error:
            raise FileExistsError('Another local governance writer holds the lock.') from error
        try:
            yield
        finally:
            handle.seek(0)
            if os.name == 'nt':
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
