from pathlib import Path
from contextlib import closing
import json
import sqlite3
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import player_backup


class PlayerBackupTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / 'source.sqlite'
        self.db = sqlite3.connect(self.source)
        self.addCleanup(self.db.close)
        self.db.executescript("""PRAGMA journal_mode=WAL;
            PRAGMA user_version=6;
            CREATE TABLE players(id TEXT PRIMARY KEY, name TEXT);
            CREATE TABLE results(player_id TEXT REFERENCES players(id), units INTEGER);
            INSERT INTO players VALUES('permanent-id','João');
            INSERT INTO results VALUES('permanent-id',17);""")
        self.db.commit()

    def test_wal_backup_and_verified_restore_keep_committed_ids_and_points(self):
        self.assertTrue(Path(str(self.source) + '-wal').exists())
        backup, restored = self.root / 'backup.sqlite', self.root / 'restored.sqlite'
        player_backup.create(self.source, backup)
        player_backup.verify(backup)
        player_backup.restore(backup, restored)
        with closing(sqlite3.connect(restored)) as db:
            self.assertEqual(db.execute('SELECT id,name,units FROM players JOIN results ON id=player_id').fetchall(), [('permanent-id', 'João', 17)])

    def test_schema8_backup_preserves_achievements_and_visits(self):
        self.db.executescript("""PRAGMA user_version=8;
            CREATE TABLE achievement_progress(player_id TEXT, achievement_id TEXT, progress INTEGER, unlocked_at TEXT, retrospective INTEGER);
            CREATE TABLE species_visits(player_id TEXT, species_id INTEGER, scientific_name TEXT, visited_at TEXT);
            INSERT INTO achievement_progress VALUES('permanent-id','visit_1',1,'synthetic-date',0);
            INSERT INTO species_visits VALUES('permanent-id',1,'Synthetic organism','synthetic-date');""")
        self.db.commit()
        backup, restored = self.root / 'v8-backup.sqlite', self.root / 'v8-restored.sqlite'
        player_backup.create(self.source, backup)
        self.assertEqual(player_backup.verify(backup)['sqlite_schema'], 8)
        player_backup.restore(backup, restored)
        with closing(sqlite3.connect(restored)) as db:
            for table in ('players', 'results', 'achievement_progress', 'species_visits'):
                self.assertEqual(db.execute('SELECT * FROM ' + table).fetchall(), self.db.execute('SELECT * FROM ' + table).fetchall())

    def test_existing_destination_and_source_are_never_overwritten(self):
        backup = self.root / 'backup.sqlite'
        player_backup.create(self.source, backup)
        original = player_backup.file_hash(backup)
        with self.assertRaises(ValueError):
            player_backup.create(self.source, backup)
        with self.assertRaises(ValueError):
            player_backup.restore(backup, self.source)
        self.assertEqual(player_backup.file_hash(backup), original)

    def test_incomplete_or_corrupted_backup_cannot_be_restored(self):
        backup, restored = self.root / 'backup.sqlite', self.root / 'restored.sqlite'
        player_backup.create(self.source, backup)
        manifest = player_backup.manifest_path(backup)
        content = manifest.read_text(encoding='utf8')
        manifest.unlink()  # Simulate interruption between backup and manifest publication.
        with self.assertRaises(OSError):
            player_backup.restore(backup, restored)
        self.assertFalse(restored.exists())
        manifest.write_text(content, encoding='utf8')
        with backup.open('ab') as stream:
            stream.write(b'synthetic corruption')
        with self.assertRaises(ValueError):
            player_backup.restore(backup, restored)
        self.assertFalse(restored.exists())

    def test_broken_foreign_keys_or_future_schemas_preserve_source_and_refuse_export(self):
        backup = self.root / 'backup.sqlite'
        self.db.execute("INSERT INTO results VALUES('unknown-id',6)"); self.db.commit()
        with self.assertRaises(ValueError):
            player_backup.create(self.source, backup)
        self.assertFalse(backup.exists())
        self.assertEqual(self.db.execute('SELECT COUNT(*) FROM results').fetchone()[0], 2)
        self.db.execute("DELETE FROM results WHERE player_id='unknown-id'")
        self.db.execute('PRAGMA user_version=99'); self.db.commit()
        with self.assertRaises(ValueError):
            player_backup.create(self.source, backup)
        self.assertFalse(backup.exists())

    def test_archive_modified_through_wal_is_refused(self):
        backup = self.root / 'backup.sqlite'
        player_backup.create(self.source, backup)
        with closing(sqlite3.connect(backup)) as writer:
            writer.execute('PRAGMA journal_mode=WAL')
            writer.execute("UPDATE players SET name='Changed after backup'"); writer.commit()
            with self.assertRaises(ValueError):
                player_backup.restore(backup, self.root / 'refused.sqlite')

    def test_malformed_manifest_shapes_fail_with_a_controlled_error(self):
        backup = self.root / 'backup.sqlite'
        player_backup.create(self.source, backup)
        good = player_backup.verify(backup)
        for index, value in enumerate(([], None, 'invalid', 1, {**good, 'schema': True},
                                      {**good, 'sqlite_schema': 6.0}, {**good, 'sha256': 'not-a-digest'})):
            with self.subTest(value=value):
                refused = self.root / f'refused-{index}.sqlite'
                player_backup.manifest_path(backup).write_text(json.dumps(value), encoding='utf8')
                with self.assertRaises(ValueError):
                    player_backup.restore(backup, refused)
                self.assertFalse(refused.exists())

    def test_existing_transaction_sidecars_refuse_publication_before_writing(self):
        backup = self.root / 'valid.sqlite'
        player_backup.create(self.source, backup)
        for suffix in ('-wal', '-shm', '-journal'):
            for operation in ('create', 'restore'):
                with self.subTest(sidecar=suffix, operation=operation):
                    destination = self.root / (operation + suffix + '.sqlite')
                    sidecar = Path(str(destination) + suffix)
                    sidecar.write_bytes(b'synthetic existing transaction sidecar')
                    source = backup if operation == 'restore' else self.source
                    with self.assertRaises(ValueError):
                        getattr(player_backup, operation)(source, destination)
                    self.assertFalse(destination.exists())
                    self.assertEqual(sidecar.read_bytes(), b'synthetic existing transaction sidecar')

    def test_interrupted_publication_is_unrestorable_and_never_overwritten(self):
        backup = self.root / 'interrupted.sqlite'
        with mock.patch.object(player_backup.json, 'dump', side_effect=OSError('synthetic manifest interruption')):
            with self.assertRaises(OSError):
                player_backup.create(self.source, backup)
        self.assertTrue(backup.exists())  # DB publication preceded the failed manifest.
        original = player_backup.file_hash(backup)
        with self.assertRaises(ValueError):
            player_backup.restore(backup, self.root / 'refused.sqlite')
        with self.assertRaises(ValueError):
            player_backup.create(self.source, backup)
        self.assertEqual(player_backup.file_hash(backup), original)
        player_backup.create(self.source, self.root / 'new-valid.sqlite')

    def test_interrupted_restore_does_not_publish_partial_bytes(self):
        backup, restored = self.root / 'backup.sqlite', self.root / 'restored.sqlite'
        player_backup.create(self.source, backup)
        def interrupted(original, output, **kwargs):
            output.write(original.read(128))
            raise OSError('synthetic copy interruption')
        with mock.patch.object(player_backup.shutil, 'copyfileobj', side_effect=interrupted):
            with self.assertRaises(OSError):
                player_backup.restore(backup, restored)
        self.assertFalse(restored.exists())
        self.assertFalse(player_backup.manifest_path(restored).exists())
        player_backup.verify(backup)
        player_backup.restore(backup, restored)

    def test_backup_excludes_uncommitted_wal_changes(self):
        self.db.execute("UPDATE results SET units=999")
        backup = self.root / 'committed.sqlite'
        player_backup.create(self.source, backup)
        with closing(sqlite3.connect(backup)) as db:
            self.assertEqual(db.execute('SELECT units FROM results').fetchone()[0], 17)
        self.assertEqual(self.db.execute('SELECT units FROM results').fetchone()[0], 999)
        self.db.rollback()

    def test_sidecar_created_while_preparing_backup_is_not_published_over(self):
        destination = self.root / 'late-sidecar.sqlite'
        sidecar = Path(str(destination) + '-journal')
        original_hash = player_backup.file_hash
        def competing_sidecar(path):
            digest = original_hash(path)
            sidecar.write_bytes(b'synthetic concurrent transaction file')
            return digest
        with mock.patch.object(player_backup, 'file_hash', side_effect=competing_sidecar):
            with self.assertRaises(ValueError):
                player_backup.create(self.source, destination)
        self.assertFalse(destination.exists())
        self.assertEqual(sidecar.read_bytes(), b'synthetic concurrent transaction file')


if __name__ == '__main__':
    unittest.main()
