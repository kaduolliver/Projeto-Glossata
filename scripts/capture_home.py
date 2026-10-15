"""Capture the real application's initial QQuickWindow, with an isolated profile."""
import argparse
from datetime import datetime, timezone
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

import release


def capture(binary, root=release.ROOT):
    from distribution import build_inputs
    manifest = release.read(root / 'build/build-manifest.json')
    if manifest.get('inputs') != build_inputs(root) or manifest.get('binary_sha256') != release.sha(binary):
        raise ValueError('Recompile primeiro: executável ou manifesto não corresponde aos fontes atuais.')
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen', QT_QUICK_BACKEND='software',
               QT_QUICK_CONTROLS_STYLE='Basic', QT_SCALE_FACTOR='1', QT_FONT_DPI='96')
    with tempfile.TemporaryDirectory(prefix='glossata-home-') as folder:
        png = Path(folder) / 'home.png'
        result = subprocess.run([str(binary.resolve()), '--capture-home', str(png)],
                                cwd=root, env=env, capture_output=True, timeout=30)
        if result.returncode or not png.is_file():
            raise ValueError(f'A captura real falhou ({result.returncode}): {result.stderr.decode("utf8", errors="replace")}')
        image = png.read_bytes()
        if not image.startswith(b'\x89PNG\r\n\x1a\n'):
            raise ValueError('O programa não produziu PNG válido.')
        width, height = struct.unpack('>II', image[16:24])
        destination = root / release.SCREENSHOT
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(png, destination)
    release.write(root / release.CAPTURE, {
        'schema_version': 1, 'captured_at': datetime.now(timezone.utc).isoformat(),
        'method': 'QQuickWindow::grabWindow / running application',
        'application_version': release.app_version(root), 'binary_sha256': release.sha(binary),
        'image_sha256': release.sha(destination), 'width': width, 'height': height,
        'rendering': 'Qt offscreen/software, scale 1, 96 DPI; real production QML and controller.',
        'privacy': 'Temporary database and default profile; Discord disabled; no player data accessed.',
        'inputs': release.home_inputs(root)})
    print(f'Captura real salva: {release.SCREENSHOT} ({width}x{height}).')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=release.ROOT / 'build/projeto_glossata.exe')
    args = parser.parse_args()
    try:
        capture(args.binary)
    except (ValueError, OSError, subprocess.TimeoutExpired) as exc:
        parser.exit(1, str(exc) + '\n')
