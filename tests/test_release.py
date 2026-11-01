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
import capture_home


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
        self.pngs = dict(zip(('top', 'discovery'), map(base64.b64decode, (
            'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=',
            'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+aZXsAAAAASUVORK5CYII='))))
        for section, (image, evidence, framing) in release.HOME_CAPTURES.items():
            path = self.root / image; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(self.pngs[section])
            release.write(self.root / evidence, {
                'section': section, 'framing': framing, 'width': 1, 'height': 1,
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

    def test_local_historical_media_does_not_change_shipped_catalog_identity(self):
        before = release.catalog_inputs(self.root)
        self.put('resources/images/local-cache.jpg', 'historical fixture, not shipped')
        self.assertEqual(release.catalog_inputs(self.root), before)
        self.put('resources.qrc', '<RCC><qresource><file>resources/images/local-cache.jpg</file></qresource></RCC>\n')
        self.assertIn('resources/images/local-cache.jpg', release.catalog_inputs(self.root))

    def test_visual_change_and_replaced_png_require_real_recapture(self):
        self.put('resources/qml/Main.qml', '// changed home\n')
        self.assertTrue(release.screenshot_errors(self.root))
        self.put(release.SCREENSHOT, 'invalid PNG')
        self.assertTrue(any('Captura não é PNG válido.' in e for e in release.screenshot_errors(self.root)))

    def test_capture_cli_sections_never_overwrite_the_other_pair(self):
        binary = self.root / 'app.exe'; binary.write_bytes(b'fixture-binary')
        distribution.record_build(binary, self.root)

        def running_fixture(command, **kwargs):
            # Only this isolated unit test substitutes subprocess; never published evidence.
            section = 'discovery' if '--capture-home-discovery' in command else 'top'
            Path(command[2]).write_bytes(self.pngs[section])
            self.assertEqual(command[:2], [str(binary.resolve()), '--capture-home'])
            self.assertEqual(kwargs['env']['QT_QPA_PLATFORM'], 'offscreen')
            return subprocess.CompletedProcess(command, 0, stderr=b'')

        for section in ('discovery', 'top', None):
            with self.subTest(section=section):
                selected = section or 'top'
                other = 'top' if selected == 'discovery' else 'discovery'
                protected = [self.root / name for name in release.HOME_CAPTURES[other][:2]]
                before = [(p.read_bytes(), p.stat().st_mtime_ns) for p in protected]
                args = ['--binary', str(binary)] + (['--section', section] if section else [])
                with mock.patch.object(release, 'ROOT', self.root), \
                     mock.patch.object(capture_home.subprocess, 'run', side_effect=running_fixture) as run, \
                     mock.patch('builtins.print'):
                    capture_home.main(args)
                run.assert_called_once()
                self.assertEqual([(p.read_bytes(), p.stat().st_mtime_ns) for p in protected], before)
                self.assertEqual(release.screenshot_errors(self.root), [])

    def test_stale_binary_and_invalid_section_do_not_write_any_capture(self):
        binary = self.root / 'app.exe'; binary.write_bytes(b'fixture-binary')
        distribution.record_build(binary, self.root)
        binary.write_bytes(b'stale-binary')
        paths = [self.root / name for row in release.HOME_CAPTURES.values() for name in row[:2]]
        before = [p.read_bytes() for p in paths]
        with mock.patch.object(capture_home.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'Recompile'):
                capture_home.capture(binary, self.root, 'discovery')
            with self.assertRaises(ValueError):
                capture_home.capture(binary, self.root, 'unknown')
            run.assert_not_called()
        self.assertEqual([p.read_bytes() for p in paths], before)

    def test_failed_or_truncated_application_output_preserves_both_pairs(self):
        binary = self.root / 'app.exe'; binary.write_bytes(b'fixture-binary')
        distribution.record_build(binary, self.root)
        paths = [self.root / name for row in release.HOME_CAPTURES.values() for name in row[:2]]
        before = [p.read_bytes() for p in paths]
        for status in (1, 0):
            def failed_capture(command, **kwargs):
                Path(command[2]).write_bytes(b'\x89PNG\r\n\x1a\n')
                return subprocess.CompletedProcess(command, status, stderr=b'fixture failure')
            with self.subTest(status=status), mock.patch.object(capture_home.subprocess, 'run', side_effect=failed_capture):
                with self.assertRaises(ValueError): capture_home.capture(binary, self.root, 'discovery')
            self.assertEqual([p.read_bytes() for p in paths], before)

    def test_home_destinations_reject_swapped_sections_even_with_matching_hashes(self):
        paths = [self.root / row[1] for row in release.HOME_CAPTURES.values()]
        records = [release.read(path) for path in paths]
        for path, original, swapped in zip(paths, records, reversed(records)):
            release.write(path, dict(original, section=swapped['section'], framing=swapped['framing']))
        errors = release.screenshot_errors(self.root)
        self.assertEqual(sum('seção/enquadramento' in e for e in errors), 2)
        self.assertFalse(any('Hash' in e for e in errors))

    def test_home_pair_requires_distinct_images(self):
        image, evidence, _ = release.HOME_CAPTURES['discovery']
        (self.root / image).write_bytes(self.pngs['top'])
        record = release.read(self.root / evidence)
        release.write(self.root / evidence, dict(record, image_sha256=release.sha(self.root / image)))
        self.assertTrue(any('mesmo PNG' in e for e in release.screenshot_errors(self.root)))

    def test_both_home_sections_require_png_json_hash_dimensions_and_origin(self):
        for section, (image, evidence, _) in release.HOME_CAPTURES.items():
            path = self.root / evidence
            original = path.read_text(encoding='utf8')
            for change, message in (({'image_sha256': 'wrong'}, 'Hash'),
                                    ({'width': 2}, 'Dimensões'),
                                    ({'captured_at': ''}, 'Origem'),
                                    ({'inputs': {}}, 'desatualizada'),
                                    ({'section': ''}, 'seção/enquadramento')):
                with self.subTest(section=section, change=change):
                    release.write(path, dict(json.loads(original), **change))
                    self.assertTrue(any(message in e for e in release.screenshot_errors(self.root)))
            for invalid in ('not JSON', '[]'):
                path.write_text(invalid, encoding='utf8')
                self.assertTrue(any('registro de origem ausente ou inválido' in e for e in release.screenshot_errors(self.root)))
            path.unlink()
            self.assertTrue(any('registro de origem ausente' in e for e in release.screenshot_errors(self.root)))
            path.write_text(original, encoding='utf8')
            png = self.root / image; png.unlink()
            self.assertTrue(any('PNG da captura ausente' in e for e in release.screenshot_errors(self.root)))
            png.write_bytes(self.pngs[section])

    def test_readme_checks_local_links_and_individual_capture_records(self):
        with (self.root / 'README.md').open('a', encoding='utf8') as stream:
            stream.write('[Missing](docs/missing.md)\n![Captured quiz](docs/images/quiz.png)\n')
        self.assertEqual(len(release.readme_errors(self.root)), 2)
        image = self.root / 'docs/images/quiz.png'; image.write_bytes(self.pngs['top'])
        self.assertTrue(any('registro de origem ausente' in e for e in release.readme_errors(self.root)))
        release.write(image.with_suffix('.json'), release.read(self.root / release.CAPTURE))
        self.put('docs/missing.md', 'Present now\n')
        self.assertEqual(release.readme_errors(self.root), [])
        image.write_bytes(self.pngs['discovery'])
        self.assertTrue(any('Hash' in e for e in release.readme_errors(self.root)))

    def test_readme_uses_original_shared_historical_manifest(self):
        name, evidence = next(iter(release.README_CAPTURE_MANIFESTS.items()))
        image = self.root / name; image.parent.mkdir(parents=True, exist_ok=True); image.write_bytes(self.pngs['top'])
        self.put('README.md', f'![Historical real capture]({name})\n[Origin]({evidence})\n')
        release.write(self.root / evidence, {
            'method': 'QQuickWindow::grabWindow / production QML and GameController in QtTest',
            'captures': [{'file': 'capturas/' + image.name, 'width': 1, 'height': 1,
                          'sha256': release.sha(image), 'captured_at': '2026-01-01T00:00:00+00:00'}]})
        self.assertEqual(release.readme_errors(self.root), [])
        image.write_bytes(self.pngs['discovery'])
        self.assertTrue(any('Hash' in e for e in release.readme_errors(self.root)))

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
            self.assertTrue(any(command[1:4] == ['scripts/project_status.py', 'check', '--build'] for command in commands))
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
        discovery_paths = '\n'.join(release.HOME_CAPTURES['discovery'][:2])
        with mock.patch.object(release, 'git', side_effect=['abcdef123456', 'resources/qml/views/HomeView.qml\n' + discovery_paths]):
            self.assertTrue(release.visual_history_errors(self.root, 'base'))

    def test_tag_history_prevents_reusing_version_even_if_manifest_state_was_reset(self):
        old = release.read(self.root / release.CATALOG); old['content_sha256'] = 'old hash'
        with mock.patch.object(release, 'git', side_effect=['v1.0.0', json.dumps(old)]):
            self.assertTrue(any('incremente' in e for e in release.historical_version_errors(self.root)))


if __name__ == '__main__': unittest.main()
