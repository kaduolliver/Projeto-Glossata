"""Tests for safe generated-build cleanup (temporary directories only)."""
import importlib.util
import os
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import patch

MODULE_PATH = Path(__file__).resolve().parents[1] / "scripts" / "cleanup_build.py"
spec = importlib.util.spec_from_file_location("cleanup_build", MODULE_PATH)
cleanup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cleanup)


class CleanupBuildTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.build = self.root / "build"
        self.build.mkdir()
        self.now = time.time()

    def aged(self, path, days=30, extension=".png"):
        path.mkdir(parents=True)
        file = path / ("sample" + extension)
        file.write_bytes(b"sample data")
        timestamp = self.now - days * 86400
        os.utime(file, (timestamp, timestamp))
        os.utime(path, (timestamp, timestamp))
        return path

    def test_dry_run_keeps_files(self):
        parent = self.build / "tests" / "previews"
        oldest = self.aged(parent / "old", 80)
        self.aged(parent / "middle", 25)
        self.aged(parent / "recent", 1)
        report = cleanup.plan_cleanup(self.root, now=self.now)
        self.assertEqual([x["path"] for x in report["candidates"]], ["build/tests/previews/old"])
        self.assertTrue(oldest.exists())

    def test_protect_current_and_sensitive(self):
        active = self.aged(self.build / "CMakeFiles" / "current", 90)
        self.aged(self.build / "2.2.3-sensitive", 90, extension=".jpg")
        self.aged(self.build / "2.2.3-middle", 80)
        self.aged(self.build / "2.2.3-latest", 1)
        report = cleanup.plan_cleanup(self.root, retain=1, include_variants=True, now=self.now)
        paths = [x["path"] for x in report["candidates"]]
        self.assertTrue(active.exists())
        self.assertNotIn("build/2.2.3-sensitive", paths)
        self.assertIn("build/2.2.3-middle", paths)

    def test_execution_denied_without_clean_main(self):
        self.aged(self.build / "tests" / "previews" / "old", 90)
        report = cleanup.plan_cleanup(self.root, retain=1, now=self.now)
        with patch.object(cleanup, "require_clean_main", side_effect=ValueError("not clean")):
            with self.assertRaises(ValueError):
                cleanup.execute_plan(self.root, report)


if __name__ == "__main__":
    unittest.main()
