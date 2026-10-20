"""Offline version, catalog and release checks. Never pushes or publishes."""
import argparse
from datetime import date
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from governance_storage import LOCK_RELATIVE

ROOT = Path(__file__).resolve().parents[1]
CATALOG = 'resources/data/catalog_manifest.json'
SCREENSHOT = 'docs/images/tela-inicial.png'
CAPTURE = 'docs/images/tela-inicial.json'
SEMVER = re.compile(r'(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)\Z')
# Conservative list: shared components may affect the home screen.
HOME_PATHS = ('resources/qml/Main.qml', 'resources/qml/views/HomeView.qml',
              *(f'resources/qml/components/{name}.qml' for name in
                ('LifeArtwork', 'KingdomArtwork', 'UiButton', 'NavItem', 'StatCard', 'UiIcon')),
              'resources/branding/glossata.png', 'resources/branding/glossata.ico')


def version(value):
    if not SEMVER.fullmatch(value):
        raise ValueError('Use uma versão estável X.Y.Z, sem prefixo v ou zeros à esquerda.')
    return tuple(map(int, value.split('.')))


def app_version(root=ROOT):
    value = (root / 'VERSION.txt').read_text(encoding='utf8').strip()
    version(value)
    return value


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def object_sha(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, ensure_ascii=False,
                                    separators=(',', ':')).encode('utf8')).hexdigest()


def read(path):
    return json.loads(path.read_text(encoding='utf8'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf8', newline='\n')


def catalog_inputs(root=ROOT):
    paths = set((root / 'resources/data').glob('*.json'))
    paths -= {root / CATALOG, root / 'resources/data/discord.json'}
    paths.update(p for p in (root / 'resources/compliance').rglob('*')
                 if p.is_file() and p != root / LOCK_RELATIVE)
    # Catalog identity covers shipped media, not local historical download caches.
    # distribution.py still audits historical/unbundled media separately.
    media = set()
    qrc = root / 'resources.qrc'
    if qrc.is_file():
        media.update(e.text for e in ET.parse(qrc).getroot().iter('file')
                     if e.text and e.text.startswith('resources/images/'))
    species = root / 'resources/data/species.json'
    if species.is_file():
        media.update(s.get('photo', {}).get('path') for s in read(species) if s.get('photo', {}).get('path'))
    for name in media:
        path = (root / name).resolve()
        if not path.is_relative_to(root.resolve()):
            raise ValueError('Mídia do catálogo fora da raiz do projeto.')
        if path.is_file(): paths.add(path)
    paths.update(p for p in (root / 'resources/branding').rglob('*') if p.is_file())
    return {p.relative_to(root).as_posix(): sha(p) for p in sorted(paths)}


def catalog_snapshot(value, root=ROOT):
    version(value)
    inputs = catalog_inputs(root)
    return {'schema_version': 1, 'version': value, 'status': 'unreleased',
            'inputs': inputs, 'content_sha256': object_sha(inputs),
            'limitations': 'Identifies bytes and review history; not scientific or legal certification.'}


def refresh_catalog(value, root=ROOT):
    """Explicit maintainer action; never changes review dates or approves content."""
    target = root / CATALOG
    previous = read(target) if target.is_file() else None
    current = catalog_snapshot(value, root)
    if previous:
        if version(value) < version(previous['version']):
            raise ValueError('A versão do catálogo não pode retroceder.')
        if previous.get('status') == 'released' and value == previous['version'] and current['inputs'] != previous['inputs']:
            raise ValueError('Catálogo lançado é imutável; incremente sua versão.')
        if previous['version'] == value and current['inputs'] == previous['inputs']:
            current['status'] = previous.get('status', 'unreleased')
    write(target, current)
    return current


def home_inputs(root=ROOT):
    paths = set()
    for entry in HOME_PATHS:
        path = root / entry
        paths.update(p for p in path.rglob('*') if p.is_file()) if path.is_dir() else paths.add(path)
    # Home counters and profile defaults are produced by the real controller.
    paths.update(root / p for p in ('src/presentation/GameController.cpp',
                                   'src/presentation/GameController.h', 'resources/data/species.json',
                                   'src/application/TaxonomyIndex.cpp', 'src/application/TaxonomyIndex.h',
                                   'src/application/EducationalContent.cpp', 'src/application/EducationalContent.h',
                                   'resources/data/education.json'))
    return {p.relative_to(root).as_posix(): sha(p) for p in sorted(paths) if p.is_file()}


def screenshot_errors(root=ROOT):
    errors = []
    png = root / SCREENSHOT
    evidence = root / CAPTURE
    if not png.is_file() or not evidence.is_file():
        return ['Captura real da tela inicial ou seu registro de origem ausente.']
    record = read(evidence)
    data = png.read_bytes()
    if not data.startswith(b'\x89PNG\r\n\x1a\n') or len(data) < 24:
        errors.append('Captura não é PNG válido.')
    if record.get('image_sha256') != sha(png) or record.get('inputs') != home_inputs(root):
        errors.append('Captura desatualizada: execute scripts/capture_home.py e inclua-a no commit visual.')
    if record.get('method') != 'QQuickWindow::grabWindow / running application' or not record.get('captured_at'):
        errors.append('Origem da captura real não documentada.')
    return errors


def check_metadata(root=ROOT):
    errors = []
    value = app_version(root)
    manifest = read(root / CATALOG)
    version(manifest['version'])
    inputs = catalog_inputs(root)
    if manifest.get('inputs') != inputs or manifest.get('content_sha256') != object_sha(inputs):
        errors.append('Manifesto do catálogo desatualizado; registre a versão e as mudanças de dados/licenças.')
    changelog = (root / 'CHANGELOG.md').read_text(encoding='utf8')
    if not re.search(r'^## \[' + re.escape(value) + r'\](?:\s|$)', changelog, re.M):
        errors.append('CHANGELOG.md não contém a versão do aplicativo.')
    catalog_log = (root / 'docs/CATALOGO_CHANGELOG.md').read_text(encoding='utf8')
    if not re.search(r'^## \[' + re.escape(manifest['version']) + r'\](?:\s|$)', catalog_log, re.M):
        errors.append('Histórico do catálogo não contém a sua versão.')
    readme = (root / 'README.md').read_text(encoding='utf8')
    if not re.search(r'!\[[^\]]{15,}\]\(docs/images/tela-inicial\.png\)', readme):
        errors.append('README sem captura relativa e texto alternativo descritivo.')
    return errors + screenshot_errors(root)


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True, encoding='utf8').strip()


def historical_version_errors(root=ROOT):
    errors = []
    current_app = version(app_version(root))
    current_catalog = read(root / CATALOG)
    for tag in git(root, 'tag', '--list', 'v*').splitlines():
        if not SEMVER.fullmatch(tag[1:]):
            continue
        if version(tag[1:]) >= current_app:
            errors.append(f'Aplicativo precisa de versão superior ao lançamento {tag}.')
        old = json.loads(git(root, 'show', f'{tag}:{CATALOG}'))
        if version(current_catalog['version']) < version(old['version']):
            errors.append(f'Versão do catálogo retrocede em relação a {tag}.')
        if old['version'] == current_catalog['version'] and old['content_sha256'] != current_catalog['content_sha256']:
            errors.append(f'Catálogo {old["version"]} já lançado em {tag}; incremente sua versão.')
    return errors


def visual_history_errors(root, base):
    errors = []
    if not base or set(base) == {'0'}:
        return errors
    for commit in git(root, 'rev-list', '--reverse', f'{base}..HEAD').splitlines():
        paths = set(git(root, 'diff-tree', '--no-commit-id', '--name-only', '-r', commit).splitlines())
        if any(p == h or (h.endswith('/') and p.startswith(h)) for p in paths for h in HOME_PATHS):
            if SCREENSHOT not in paths or CAPTURE not in paths:
                errors.append(f'{commit[:12]}: alteração visual compartilhada sem PNG e registro no mesmo commit.')
    return errors


def run(command, root=ROOT, env=None):
    print('+', ' '.join(map(str, command)), flush=True)
    subprocess.run(list(map(str, command)), cwd=root, env=env, check=True)


def prepare_tag(root, package, build):
    """Run all mandatory gates before creating ANY local release tag."""
    if git(root, 'status', '--porcelain'):
        raise ValueError('Faça os commits antes de preparar uma tag; árvore de trabalho deve estar limpa.')
    errors = check_metadata(root) + historical_version_errors(root)
    if errors:
        raise ValueError('\n'.join(errors))
    value = app_version(root)
    tag = f'v{value}'
    if git(root, 'tag', '--list', tag):
        raise ValueError('Tag já existe; não será substituída.')
    # Fail fast on rights/curation; never convert this gate into a warning.
    run([sys.executable, 'scripts/distribution.py', 'check'], root)
    run([sys.executable, 'scripts/build_education.py'], root)
    run([sys.executable, 'scripts/content_governance.py', 'credits'], root)
    run(['cmake', '-S', '.', '-B', build, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release'], root)
    run([sys.executable, 'scripts/project_status.py', 'check', '--build', build], root)
    run(['cmake', '--build', build, '--parallel', '4'], root)
    run(['ctest', '--test-dir', build, '--output-on-failure'], root)
    run(['qmllint', *sorted((root / 'resources/qml').rglob('*.qml')), '--unqualified', 'disable'], root)
    run([sys.executable, 'scripts/audit_catalog.py', '--strict'], root)
    run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', 'scripts/package.ps1'], root)
    run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', 'scripts/validate_package.ps1'], root)
    run([sys.executable, 'scripts/distribution.py', 'check', '--package', package], root)
    # Re-check metadata and cleanliness after generated reports/build steps.
    errors = check_metadata(root) + historical_version_errors(root)
    if errors or git(root, 'status', '--porcelain'):
        raise ValueError('Verificações alteraram arquivos versionados ou metadados: revise e faça commit.')
    catalog = read(root / CATALOG)
    for path, number in (('CHANGELOG.md', value), ('docs/CATALOGO_CHANGELOG.md', catalog['version'])):
        log = (root / path).read_text(encoding='utf8')
        section = re.search(r'^## \[' + re.escape(number) + r'\]([^\n]*)', log, re.M)
        stamp = re.search(r'\d{4}-\d{2}-\d{2}', section[1]) if section else None
        if not stamp or re.search(r'não lançado|em desenvolvimento|unreleased', section[1], re.I):
            raise ValueError(f'Finalize a entrada datada de {path} antes do lançamento.')
        if date.fromisoformat(stamp[0]) > date.today():
            raise ValueError('Data de lançamento futura; use a data real do lançamento.')
    if catalog.get('status') != 'released':
        raise ValueError('Finalize o estado e a entrada datada do catálogo antes do lançamento.')
    return tag


def create_tag(root=ROOT, package='package', build='build'):
    tag = prepare_tag(root, package, build)
    manifest = read(root / CATALOG)
    message = (f'Projeto Glossata {app_version(root)}\nCatálogo {manifest["version"]}\n'
               f'Catálogo SHA256: {manifest["content_sha256"]}\n'
               f'Commit: {git(root, "rev-parse", "HEAD")}\n'
               'Verificações automáticas não certificam conformidade jurídica ou científica.')
    # stdin preserves Unicode/newlines without shell interpolation. No remote writes.
    subprocess.run(['git', '-C', str(root), 'tag', '-a', tag, '-F', '-'], input=message,
                   text=True, encoding='utf8', check=True)
    print(f'Tag local criada: {tag}. Nenhum push ou release publicado.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['check', 'catalog', 'check-history', 'tag'])
    parser.add_argument('--version')
    parser.add_argument('--base')
    args = parser.parse_args()
    try:
        if args.action == 'catalog':
            if not args.version:
                parser.error('--version é obrigatório e deve refletir uma decisão de versionamento.')
            refresh_catalog(args.version)
        elif args.action == 'tag':
            create_tag()
        else:
            errors = visual_history_errors(ROOT, args.base) if args.action == 'check-history' else check_metadata()
            if errors:
                print('\n'.join(errors)); return 1
            print('Metadados/histórico íntegros. Isso não libera distribuição nem certifica curadoria.')
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr); return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
