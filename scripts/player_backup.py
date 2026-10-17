"""Consistent SQLite player backup/restore to a NEW file; never replace a live database."""
import argparse
from contextlib import closing
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import shutil
import tempfile
import time


def inspect(path):
    with closing(sqlite3.connect(Path(path).resolve().as_uri() + '?mode=ro', uri=True)) as db:
        if db.execute('PRAGMA integrity_check').fetchall() != [('ok',)]:
            raise ValueError('SQLite integrity check failed')
        if db.execute('PRAGMA foreign_key_check').fetchall():
            raise ValueError('SQLite foreign key check failed')
        version = db.execute('PRAGMA user_version').fetchone()[0]
        if not 0 <= version <= 6:
            raise ValueError('Unsupported database schema; preserve original')
        return version


def manifest_path(path):
    return Path(str(path) + '.manifest.json')


def file_hash(path):
    result = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            result.update(block)
    return result.hexdigest()


def verify(path):
    path = Path(path).resolve()
    wal = Path(str(path) + '-wal')
    if wal.exists() and wal.stat().st_size:
        raise ValueError('Archive has an active WAL; create a new consistent backup')
    if manifest_path(path).stat().st_size > 65536:
        raise ValueError('Oversized backup manifest')
    manifest = json.loads(manifest_path(path).read_text(encoding='utf8'))
    if manifest.get('schema') != 1 or file_hash(path) != manifest.get('sha256'):
        raise ValueError('Backup hash/manifest mismatch')
    if inspect(path) != manifest.get('sqlite_schema'):
        raise ValueError('Backup schema mismatch')
    return manifest


def new_destination(destination):
    destination = Path(destination).resolve()
    if destination.exists() or manifest_path(destination).exists():
        raise ValueError('Use a NEW destination; existing files are never overwritten')
    destination.parent.mkdir(parents=True, exist_ok=True)
    return destination


def publish(temporary, destination, report):
    os.link(temporary, destination)  # Exclusive publication, including races.
    with manifest_path(destination).open('x', encoding='utf8') as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write('\n'); stream.flush(); os.fsync(stream.fileno())
    verify(destination)
    return report


def create(source, destination):
    source, destination = Path(source).resolve(), new_destination(destination)
    if not source.is_file() or source == destination:
        raise ValueError('Use an existing source and a NEW destination')
    fd, temporary = tempfile.mkstemp(prefix='.glossata-backup-', suffix='.sqlite', dir=destination.parent)
    os.close(fd)
    temporary = Path(temporary)
    deadline = time.monotonic() + 30
    def progress(status, remaining, total):
        if time.monotonic() > deadline:
            raise TimeoutError('Backup exceeded 30 seconds; original preserved')
    try:
        # SQLite's backup API includes committed WAL data and obtains a coherent
        # view while writers may exist. Do not copy the main file alone.
        with closing(sqlite3.connect(source.as_uri() + '?mode=ro', uri=True)) as original:
            with closing(sqlite3.connect(temporary)) as backup:
                original.backup(backup, pages=128, progress=progress, sleep=0.05)
        version = inspect(temporary)
        with temporary.open('r+b') as stream:
            os.fsync(stream.fileno())
        report = {'schema': 1, 'sqlite_schema': version,
                  'sha256': file_hash(temporary),
                  'created_at': datetime.now(timezone.utc).isoformat(),
                  'notice': 'Local player data; keep private. Integrity is not identity authentication.'}
        # Exclusive publication: link refuses an existing destination, including
        # a destination created between the initial check and this operation.
        return publish(temporary, destination, report)
    finally:
        temporary.unlink(missing_ok=True)


def restore(source, destination):
    report = verify(source)
    destination = new_destination(destination)
    fd, temporary = tempfile.mkstemp(prefix='.glossata-restore-', suffix='.sqlite', dir=destination.parent)
    temporary = Path(temporary)
    try:
        # Restore precisely the bytes certified by the manifest. A mutable WAL
        # or a concurrent modification must not introduce unverified data.
        with os.fdopen(fd, 'wb') as output, Path(source).open('rb') as original:
            shutil.copyfileobj(original, output, length=1024 * 1024)
            output.flush(); os.fsync(output.fileno())
        if file_hash(temporary) != report['sha256'] or inspect(temporary) != report['sqlite_schema']:
            raise ValueError('Archive changed during restoration; original preserved')
        restored = dict(report, restored_at=datetime.now(timezone.utc).isoformat())
        return publish(temporary, destination, restored)
    finally:
        temporary.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=['create', 'verify', 'restore'])
    parser.add_argument('--source', required=True, type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.operation != 'verify' and args.output is None:
        parser.error('--output is required; existing destinations are always refused')
    try:
        report = verify(args.source) if args.operation == 'verify' else (restore if args.operation == 'restore' else create)(args.source, args.output)
    except (OSError, ValueError, sqlite3.Error, TimeoutError) as error:
        parser.exit(1, f'Backup/restore refused: {error}\n')
    print(json.dumps(report, ensure_ascii=False))


if __name__ == '__main__':
    main()
