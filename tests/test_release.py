"""Release metadata, stale catalog/capture and fail-closed tag regression tests."""
import base64
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import release
import distribution


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.put('VERSION.txt', '1.1.0\n')
        self.put('CMakeLists.txt', '# test fixture\n')
        self.put('CHANGELOG.md', '## [1.1.0] — Não lançado\n')
        self.put('docs/CATALOGO_CHANGELOG.md', '## [1.0.0] — Não lançado\n')
        self.put('README.md', '![Tela inicial real do programa em execução](docs/images/tela-inicial.png)\n')
        self.put('resources/data/species.json', '[]\n')
        self.put('resources/qml/Main.qml', '// synthetic test fixture, never shipped as screenshot\n')
        self.put('resources.qrc', '<RCC><qresource/></RCC>\n')
        release.refresh_catalog('1.0.0', self.root)
        # A tiny PNG is a unit-test fixture only; capture_home never uses it.
        png = base64.b64decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=')
        path = self.root / release.SCREENSHOT; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(png)
        release.write(self.root / release.CAPTURE, {
            'image_sha256': release.sha(path), 'inputs': release.home_inputs(self.root),
            'method': 'QQuickWindow::grabWindow / running application', 'captured_at': '2026-01-01T00:00:00+00:00'})

    def put(self, path, text):
        target = self.root / path; target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding='utf8')

    def test_semver_rejects_missing_patch_leading_zeros_and_prefix(self):
        for bad in ('1.0', 'v1.0.0', '01.0.0', '1.2.3.4', '1.1.-1'):
            with self.subTest(bad=bad), self.assertRaises(ValueError): release.version(bad)
        self.assertLess(release.version('1.9.0'), release.version('1.10.0'))

    def test_app_version_and_catalog_version_are_independent(self):
        self.assertEqual(release.check_metadata(self.root), [])
        self.put('VERSION.txt', '1.1.1\n'); self.put('CHANGELOG.md', '## [1.1.1] — Não lançado\n')
        self.assertEqual(release.read(self.root / release.CATALOG)['version'], '1.0.0')
        self.assertEqual(release.check_metadata(self.root), [])

    def test_new_permission_evidence_and_review_history_invalidate_catalog(self):
        original = release.read(self.root / release.CATALOG)['content_sha256']
        self.put('resources/compliance/permissions/proof.json', '{"license":"cc-by"}\n')
        self.assertTrue(any('Manifesto' in e for e in release.check_metadata(self.root)))
        release.refresh_catalog('1.0.0', self.root)
        self.assertNotEqual(release.read(self.root / release.CATALOG)['content_sha256'], original)
        self.put('resources/compliance/review_history.jsonl', '{"review":"fixture only"}\n')
        self.assertTrue(any('Manifesto' in e for e in release.check_metadata(self.root)))

    def test_released_catalog_cannot_be_overwritten_with_same_version(self):
        manifest = release.read(self.root / release.CATALOG); manifest['status'] = 'released'
        release.write(self.root / release.CATALOG, manifest)
        self.put('resources/data/species.json', '[{"id":1}]\n')
        with self.assertRaises(ValueError): release.refresh_catalog('1.0.0', self.root)
        release.refresh_catalog('1.0.1', self.root)
        with self.assertRaises(ValueError): release.refresh_catalog('0.9.0', self.root)

    def test_visual_change_and_replaced_png_require_real_recapture(self):
        self.put('resources/qml/Main.qml', '// changed home\n')
        self.assertTrue(release.screenshot_errors(self.root))
        self.put(release.SCREENSHOT, 'invalid PNG')
        self.assertIn('Captura não é PNG válido.', release.screenshot_errors(self.root))

    def test_failed_mandatory_check_never_creates_tag(self):
        with mock.patch.object(release, 'git', return_value=''), \
             mock.patch.object(release, 'run', side_effect=subprocess.CalledProcessError(2, ['distribution.py'])) as run, \
             mock.patch.object(release.subprocess, 'run') as mutation:
            with self.assertRaises(subprocess.CalledProcessError): release.create_tag(self.root)
            self.assertEqual(run.call_args.args[0][-2:], ['scripts/distribution.py', 'check'])
            mutation.assert_not_called()

    def test_dirty_tree_blocks_before_checks_or_tag(self):
        with mock.patch.object(release, 'git', return_value=' M VERSION.txt'), mock.patch.object(release, 'run') as run:
            with self.assertRaises(ValueError): release.create_tag(self.root)
            run.assert_not_called()

    def test_tag_preparation_requires_full_suite_and_both_distribution_gates(self):
        self.put('CHANGELOG.md', '## [1.1.0] — 2026-01-01\n')
        self.put('docs/CATALOGO_CHANGELOG.md', '## [1.0.0] — 2026-01-01\n')
        catalog = release.read(self.root / release.CATALOG); catalog['status'] = 'released'
        release.write(self.root / release.CATALOG, catalog)
        with mock.patch.object(release, 'git', return_value=''), mock.patch.object(release, 'run') as run:
            self.assertEqual(release.prepare_tag(self.root, 'package', 'build'), 'v1.1.0')
            commands = [list(map(str, call.args[0])) for call in run.call_args_list]
            self.assertIn(['ctest', '--test-dir', 'build', '--output-on-failure'], commands)
            self.assertTrue(any(c[1:] == ['scripts/distribution.py', 'check'] for c in commands))
            self.assertTrue(any(c[1:] == ['scripts/distribution.py', 'check', '--package', 'package'] for c in commands))
            self.assertTrue(any('scripts/validate_package.ps1' in c for c in commands))

    def test_passing_checks_do_not_release_an_undated_development_entry(self):
        with mock.patch.object(release, 'git', return_value=''), mock.patch.object(release, 'run'):
            with self.assertRaisesRegex(ValueError, 'entrada datada'):
                release.prepare_tag(self.root, 'package', 'build')

    def test_app_and_catalog_versions_are_bound_to_binary_manifest(self):
        binary = self.root / 'app.exe'; binary.write_bytes(b'fixture-binary')
        manifest = distribution.record_build(binary, self.root)
        self.assertEqual(manifest['application_version'], '1.1.0')
        self.assertEqual(manifest['catalog_version'], '1.0.0')
        self.assertEqual(manifest['catalog_sha256'], release.read(self.root / release.CATALOG)['content_sha256'])
        self.assertEqual(manifest['binary_sha256'], release.sha(binary))
        original = manifest['inputs']; self.put('VERSION.txt', '1.2.0\n')
        self.assertNotEqual(original, distribution.build_inputs(self.root))

    def test_visual_commit_needs_png_in_same_commit(self):
        with mock.patch.object(release, 'git', side_effect=['abcdef123456', 'resources/qml/views/HomeView.qml']):
            self.assertTrue(release.visual_history_errors(self.root, 'base'))
        with mock.patch.object(release, 'git', side_effect=['abcdef123456', 'resources/qml/views/HomeView.qml\n' + release.SCREENSHOT + '\n' + release.CAPTURE]):
            self.assertEqual(release.visual_history_errors(self.root, 'base'), [])

    def test_tag_history_prevents_reusing_version_even_if_manifest_state_was_reset(self):
        old = release.read(self.root / release.CATALOG); old['content_sha256'] = 'old hash'
        with mock.patch.object(release, 'git', side_effect=['v1.0.0', json.dumps(old)]):
            self.assertTrue(any('incremente' in e for e in release.historical_version_errors(self.root)))


if __name__ == '__main__': unittest.main()
