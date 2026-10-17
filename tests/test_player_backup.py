from pathlib import Path
from contextlib import closing
import sqlite3
import sys
import tempfile
import unittest

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


if __name__ == '__main__':
    unittest.main()
