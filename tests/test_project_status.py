"""Offline regression of current-state metadata; all mutations use temporary copies."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import project_status as status


class ProjectStatusTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        _, cmake = status.test_registry(status.ROOT)
        for relative in {*status.SOURCES, *(p.relative_to(status.ROOT).as_posix() for p in cmake)}:
            destination = self.root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(status.ROOT / relative, destination)
        for relative in status.DOCUMENTS:
            (self.root / relative).write_text('# Synthetic documentation\n' + status.START + '\n' + status.END + '\n', encoding='utf8')
        status.write(self.root)

    def edit(self, path, before, after):
        file = self.root / path
        body = file.read_text(encoding='utf8')
        self.assertIn(before, body)
        file.write_text(body.replace(before, after), encoding='utf8')

    def test_real_repository_and_sources_are_consistent(self):
        self.assertEqual(status.check(), [])
        actual = status.derive()
        self.assertEqual(actual['application']['version'], (status.ROOT / 'VERSION.txt').read_text().strip())
        self.assertEqual(actual['catalog']['version'], json.loads((status.ROOT / status.release.CATALOG).read_bytes())['version'])
        self.assertNotIn('passed', actual['tests'])

    def test_generation_is_idempotent_and_preserves_historical_records(self):
        path = self.root / 'PROJECT_CONTEXT.md'
        historical = status.HISTORY_START + '\nAplicativo 0.0.1; catálogo 0.0.2. São nove suítes.\n' + status.HISTORY_END
        path.write_text(path.read_text(encoding='utf8') + historical, encoding='utf8')
        status.write(self.root)
        before = {p: (self.root / p).read_bytes() for p in (*status.DOCUMENTS, status.SNAPSHOT)}
        status.write(self.root)
        self.assertEqual(before, {p: (self.root / p).read_bytes() for p in before})
        self.assertIn(historical, path.read_text(encoding='utf8'))
        self.assertEqual(status.check(self.root), [])

    def test_stale_versions_rules_or_counts_outside_generated_blocks_are_refused(self):
        for claim in ('Aplicativo **0.0.1**.', 'Catálogo 0.0.1.', 'Regras atuais: identification-v2', 'São nove suítes.'):
            with self.subTest(claim=claim):
                path = self.root / 'README.md'
                original = path.read_text(encoding='utf8')
                path.write_text(original + '\n' + claim, encoding='utf8')
                self.assertTrue(status.check(self.root))
                path.write_text(original, encoding='utf8')

    def test_version_and_manifest_cannot_be_changed_without_matching_logs(self):
        self.edit('VERSION.txt', status.release.app_version(self.root), '99.0.0')
        with self.assertRaises(ValueError): status.derive(self.root)
        shutil.copyfile(status.ROOT / 'VERSION.txt', self.root / 'VERSION.txt')
        manifest = json.loads((self.root / status.release.CATALOG).read_bytes())
        for key, value in [('version', '99.0.0'), ('status', 'released'), ('content_sha256', 'invalid')]:
            with self.subTest(key=key):
                (self.root / status.release.CATALOG).write_text(json.dumps(dict(manifest, **{key: value})), encoding='utf8')
                with self.assertRaises(ValueError): status.derive(self.root)

    def test_changed_rule_requires_regenerating_metadata_not_changing_software(self):
        previous = status.derive(self.root)['scoring']['current']
        self.edit('src/domain/Hint.h', previous, 'synthetic-test-policy')
        self.assertTrue(status.check(self.root))
        status.write(self.root)
        self.assertEqual(status.check(self.root), [])
        self.edit('src/domain/GameConfiguration.h', 'rulesVersion = CurrentScoringRules;', 'rulesVersion = LegacyScoringRules;')
        with self.assertRaises(ValueError): status.derive(self.root)

    def test_new_tests_and_unsupported_registration_are_detected(self):
        path = self.root / 'tests/CMakeLists.txt'
        original = path.read_text(encoding='utf8')
        before = status.derive(self.root)['tests']['registered_count']
        path.write_text(original + '\nadd_test(NAME SyntheticNewSuite COMMAND example)\n', encoding='utf8')
        self.assertTrue(status.check(self.root))
        status.write(self.root)
        self.assertEqual(status.derive(self.root)['tests']['registered_count'], before + 1)
        for declaration in ('add_test(NAME ${dynamic_name} COMMAND example)', 'add_test(NAME SyntheticNewSuite COMMAND other)'):
            path.write_text(path.read_text(encoding='utf8') + '\n' + declaration, encoding='utf8')
            with self.assertRaises(ValueError): status.derive(self.root)
            path.write_text(original + '\nadd_test(NAME SyntheticNewSuite COMMAND example)\n', encoding='utf8')

    def test_sqlite_migration_guard_and_application_wiring_are_checked(self):
        path = 'src/infrastructure/persistence/SqliteGameHistoryRepository.cpp'
        version = status.derive(self.root)['storage']['history_schema']
        self.edit(path, f'PRAGMA user_version={version}', f'PRAGMA user_version={version+1}')
        with self.assertRaises(ValueError): status.derive(self.root)
        shutil.copyfile(status.ROOT / path, self.root / path)
        self.edit('src/app/main.cpp', 'QStringLiteral(GLOSSATA_APP_VERSION)', 'QStringLiteral("0.0.1")')
        with self.assertRaises(ValueError): status.derive(self.root)

    def test_missing_and_repeated_markers_are_not_silently_repaired(self):
        path = self.root / 'README.md'
        original = path.read_text(encoding='utf8')
        for invalid in (original + status.START, original.replace(status.START, ''), status.END + '\n' + status.START):
            with self.subTest(invalid=invalid):
                path.write_text(invalid, encoding='utf8')
                with self.assertRaises(ValueError): status.write(self.root)
        # Mutate a generated value without coupling this test to the suite count.
        path.write_text(original.replace(status.derive(self.root)['application']['version'], '99.0.0'), encoding='utf8')
        self.assertTrue(status.check(self.root))

    def test_configured_ctest_inventory_and_version_must_match(self):
        build = self.root / 'build'; build.mkdir()
        version = status.derive(self.root)['application']['version']
        cache = f'CMAKE_HOME_DIRECTORY:INTERNAL={self.root.as_posix()}\nCMAKE_PROJECT_VERSION:STATIC={version}\n'
        (build / 'CMakeCache.txt').write_text(cache, encoding='utf8')
        names = status.derive(self.root)['tests']['registered_names']
        with patch.object(status.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0, json.dumps({'tests': [{'name': n} for n in names]}))) as run:
            self.assertEqual(status.check(self.root, build), [])
            self.assertIn('--show-only=json-v1', run.call_args[0][0])
            run.return_value.stdout = json.dumps({'tests': []})
            self.assertTrue(status.check(self.root, build))
        (build / 'CMakeCache.txt').write_text(cache.replace(version, '0.0.1'), encoding='utf8')
        with patch.object(status.subprocess, 'run') as run:
            self.assertTrue(status.check(self.root, build)); run.assert_not_called()
        (build / 'CMakeCache.txt').write_text(cache.replace(self.root.as_posix(), (self.root / 'another-clone').as_posix()), encoding='utf8')
        with patch.object(status.subprocess, 'run') as run:
            self.assertTrue(status.check(self.root, build)); run.assert_not_called()

    def test_dated_entry_means_prepared_not_verified_publication(self):
        path = self.root / 'CHANGELOG.md'
        body = path.read_text(encoding='utf8')
        body = body.replace(body.splitlines()[4], body.splitlines()[4].split('—')[0] + '— 2020-01-01')
        path.write_text(body, encoding='utf8')
        derived = status.derive(self.root)
        self.assertEqual(derived['application']['stage'], 'release_prepared')
        self.assertIn('publicação não verificada', status.block(derived))


if __name__ == '__main__':
    unittest.main()
