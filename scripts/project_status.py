"""Generate/check current documentation from repository sources. Offline; never releases."""
import argparse
from datetime import date
import json
from pathlib import Path
import re
import subprocess

import release

ROOT = Path(__file__).resolve().parents[1]
SNAPSHOT = 'docs/PROJECT_STATUS.json'
DOCUMENTS = ('README.md', 'PROJECT_CONTEXT.md')
START, END = '<!-- glossata:status:start -->', '<!-- glossata:status:end -->'
HISTORY_START, HISTORY_END = '<!-- glossata:historical:start -->', '<!-- glossata:historical:end -->'
SOURCES = ('VERSION.txt', 'CHANGELOG.md', 'docs/CATALOGO_CHANGELOG.md', release.CATALOG,
           'src/domain/Hint.h', 'src/domain/GameConfiguration.h', 'src/domain/SessionCheckpoint.h',
           'src/infrastructure/persistence/SqliteGameHistoryRepository.cpp',
           'src/infrastructure/content/SqliteSpeciesCatalog.cpp', 'resources/data/species.json',
           'resources/data/taxonomy.json', 'resources/data/exploration_photos.json', 'src/app/main.cpp')


def text(root, path):
    return (root / path).read_text(encoding='utf8')


def unique(pattern, body, description):
    values = re.findall(pattern, body, re.M)
    if len(values) != 1:
        raise ValueError(f'{description}: expected exactly one supported declaration')
    return values[0]


def stage(root, path, version):
    sections = re.findall(r'^## \[([^]]+)\]([^\n]*)', text(root, path), re.M)
    if not sections or sections[0][0] != version:
        raise ValueError(f'{path}: first version entry must match {version}')
    heading = sections[0][1]
    if re.search(r'não lançado|em desenvolvimento|unreleased', heading, re.I):
        return 'development'
    stamp = unique(r'(\d{4}-\d{2}-\d{2})', heading, path)
    if date.fromisoformat(stamp) > date.today():
        raise ValueError(f'{path}: future release date')
    return 'release_prepared'  # A dated heading does not prove a published release.


def test_registry(root):
    paths = [root / 'CMakeLists.txt', *sorted((root / 'tests').rglob('CMakeLists.txt'))]
    names = []
    for path in paths:
        body = re.sub(r'#[^\n]*', '', path.read_text(encoding='utf8'))
        found = re.findall(r'\badd_test\s*\(\s*NAME\s+([A-Za-z_][A-Za-z0-9_.-]*)\s', body)
        if len(found) != len(re.findall(r'\badd_test\s*\(', body)):
            raise ValueError(f'{path.relative_to(root)}: dynamic/unsupported add_test; extend the inventory explicitly')
        names.extend(found)
    if not names or len(names) != len(set(names)):
        raise ValueError('Empty or duplicate CTest registrations')
    return sorted(names), paths


def sql_schema(root, path, variable):
    body = text(root, path)
    written = int(unique(r'PRAGMA user_version=(\d+)', body, path + ' migration'))
    supported = int(unique(r'if\s*\(\s*' + variable + r'\s*>\s*(\d+)\s*\)', body, path + ' version guard'))
    if written != supported:
        raise ValueError(f'{path}: migration and supported schema disagree')
    return written


def derive(root=ROOT):
    root = Path(root)
    app = release.app_version(root)
    catalog = release.read(root / release.CATALOG)
    release.version(catalog['version'])
    if catalog.get('status') not in ('unreleased', 'released') or catalog.get('content_sha256') != release.object_sha(catalog['inputs']):
        raise ValueError('Catalog manifest status/identity invalid; use the existing release/catalog checks')
    catalog_stage = stage(root, 'docs/CATALOGO_CHANGELOG.md', catalog['version'])
    if (catalog['status'] == 'unreleased') != (catalog_stage == 'development'):
        raise ValueError('Catalog status and current changelog heading disagree')
    hints = text(root, 'src/domain/Hint.h')
    rules = {key: unique(r'\b' + constant + r'\s*=\s*QStringLiteral\("([^"\n]+)"\)', hints, constant)
             for key, constant in [('current', 'CurrentScoringRules'), ('previous', 'PreviousScoringRules'), ('legacy', 'LegacyScoringRules')]}
    config = text(root, 'src/domain/GameConfiguration.h')
    if not re.search(r'GLOSSATA_APP_VERSION=\\?"\$\{PROJECT_VERSION\}', text(root, 'CMakeLists.txt')) or not re.search(
            r'setApplicationVersion\s*\(\s*QStringLiteral\s*\(\s*GLOSSATA_APP_VERSION\s*\)\s*\)', text(root, 'src/app/main.cpp')):
        raise ValueError('Application version wiring no longer uses the CMake project version')
    if not re.search(r'\brulesVersion\s*=\s*CurrentScoringRules\s*;', config):
        raise ValueError('New GameConfiguration no longer defaults to CurrentScoringRules')
    names, cmake_paths = test_registry(root)
    species = release.read(root / 'resources/data/species.json')
    taxonomy = release.read(root / 'resources/data/taxonomy.json')
    domains = {t['id']: t['name'] for t in taxonomy if t['rank'] == 'domain'}
    counts = {name: 0 for name in domains.values()}
    for organism in species:
        matches = [domains[node] for node in organism['taxonomy'] if node in domains]
        if len(matches) != 1:
            raise ValueError('Organism without exactly one documented domain')
        counts[matches[0]] += 1
    inputs = {path: release.sha(root / path) for path in SOURCES}
    inputs.update({p.relative_to(root).as_posix(): release.sha(p) for p in cmake_paths})
    return {'schema_version': 1,
            'application': {'version': app, 'stage': stage(root, 'CHANGELOG.md', app)},
            'catalog': {'version': catalog['version'], 'declared_status': catalog['status'], 'content_sha256': catalog['content_sha256']},
            'scoring': dict(rules, competitive_questions=int(unique(r'\bCompetitiveQuestionCount\s*=\s*(\d+)\s*;', config, 'competitive limit'))),
            'storage': {'history_schema': sql_schema(root, SOURCES[7], 'version'),
                        'catalog_schema': sql_schema(root, SOURCES[8], 'previousVersion'),
                        'checkpoint_snapshot': int(unique(r'\bSchema\s*=\s*(\d+)\s*;', text(root, SOURCES[6]), 'checkpoint schema'))},
            'content': {'organisms': len(species), 'distinct_quiz_images': len({s['image'] for s in species}), 'domains': dict(sorted(counts.items()))},
            'tests': {'registered_count': len(names), 'registered_names': names},
            'inputs': dict(sorted(inputs.items())),
            'limitations': 'Source metadata and registered tests, not execution results, publication evidence or distribution/scientific approval.'}


def block(status):
    app, catalog, scoring, storage, content = (status[k] for k in ('application', 'catalog', 'scoring', 'storage', 'content'))
    app_stage = 'em desenvolvimento; não lançado' if app['stage'] == 'development' else 'entrada datada; publicação não verificada'
    catalog_stage = 'não lançado' if catalog['declared_status'] == 'unreleased' else 'manifesto marcado released; publicação não verificada'
    lines = [START, '<!-- Gerado por scripts/project_status.py; não editar manualmente. -->',
             '| Informação atual | Valor derivado | Fonte |', '| --- | --- | --- |',
             f'| Aplicativo | **{app["version"]}** — {app_stage} | [VERSION.txt](VERSION.txt), [CHANGELOG](CHANGELOG.md) |',
             f'| Catálogo | **{catalog["version"]}** — {catalog_stage} | [Manifesto](resources/data/catalog_manifest.json), [histórico](docs/CATALOGO_CHANGELOG.md) |',
             f'| Regra padrão de pontuação | `{scoring["current"]}` | [Hint.h](src/domain/Hint.h) |',
             f'| Leitura histórica de pontuação | `{scoring["previous"]}`, `{scoring["legacy"]}` e `legacy` | [Hint.h](src/domain/Hint.h) |',
             f'| Competitivo | {scoring["competitive_questions"]} questões | [GameConfiguration.h](src/domain/GameConfiguration.h) |',
             f'| SQLite e snapshots | histórico {storage["history_schema"]}; catálogo {storage["catalog_schema"]}; checkpoint {storage["checkpoint_snapshot"]} | [Fontes e verificação](docs/ESTADO_DO_PROJETO.md) |',
             f'| Organismos e imagens do quiz | {content["organisms"]} registros; {content["distinct_quiz_images"]} arquivos distintos | [Catálogo](resources/data/species.json) |',
             f'| Domínios | {"; ".join(f"{name}: {count}" for name, count in content["domains"].items())} | [Taxonomia](resources/data/taxonomy.json) |',
             f'| Testes registrados | **{status["tests"]["registered_count"]} suítes CTest**; não representa resultado de execução | [Inventário gerado](docs/PROJECT_STATUS.json), [CMake](tests/CMakeLists.txt) |',
             '', 'Valores derivados das fontes locais. Este bloco não comprova publicação, execução da CI ou aprovação do gate de distribuição.', END]
    return '\n'.join(lines)


def replace_block(body, replacement):
    if body.count(START) != 1 or body.count(END) != 1 or body.index(START) >= body.index(END):
        raise ValueError('Missing, repeated or inverted current-status markers')
    a, b = body.index(START), body.index(END) + len(END)
    return body[:a] + replacement + body[b:]


def narrative_errors(body, status, path):
    # Explicitly scoped: historical records are retained, never rewritten into
    # current facts. This is not a natural-language truth/feature verifier.
    if body.count(HISTORY_START) != body.count(HISTORY_END) or body.count(HISTORY_START) > 1:
        return [f'{path}: invalid historical markers']
    if HISTORY_START in body and body.index(HISTORY_START) >= body.index(HISTORY_END):
        return [f'{path}: inverted historical markers']
    body = re.sub(re.escape(HISTORY_START) + r'.*?' + re.escape(HISTORY_END), '', body, flags=re.S)
    body = re.sub(re.escape(START) + r'.*?' + re.escape(END), '', body, flags=re.S)
    errors = []
    for noun, value in [('aplicativo', status['application']['version']), ('catálogo', status['catalog']['version'])]:
        for match in re.finditer(r'\b' + noun + r'[\s*`:—-]+v?(\d+\.\d+\.\d+)', body, re.I):
            if match[1] != value:
                errors.append(f'{path}: current {noun} claim {match[1]} disagrees with {value}; historical facts need explicit scope')
    if re.search(r'\b(?:são|há)\s+\w+\s+suítes|\b\d+\s+suítes', body, re.I):
        errors.append(f'{path}: keep current test counts in the generated block; scope execution results as history')
    for match in re.finditer(r'\bregras?\s+(?:atuais|atual|vigentes?)\s*[:—-]?\s*`?(identification-[\w-]+)', body, re.I):
        if match[1] != status['scoring']['current']:
            errors.append(f'{path}: current scoring rule disagrees with Hint.h')
    return errors


def configured_errors(root, build, status):
    build = Path(build)
    if not build.is_absolute(): build = root / build
    cache = (build / 'CMakeCache.txt').read_text(encoding='utf8')
    source = unique(r'^CMAKE_HOME_DIRECTORY:INTERNAL=(.*)$', cache, 'configured source')
    version = unique(r'^CMAKE_PROJECT_VERSION:STATIC=(.*)$', cache, 'configured application version')
    if Path(source).resolve() != root.resolve() or version != status['application']['version']:
        return ['Configured CMake tree/version is stale or belongs to another checkout; reconfigure']
    result = subprocess.run(['ctest', '--test-dir', str(build), '--show-only=json-v1'], capture_output=True, text=True,
                            encoding='utf8', check=True, timeout=30)
    names = sorted(t['name'] for t in json.loads(result.stdout)['tests'])
    return [] if names == status['tests']['registered_names'] else ['Configured CTest inventory differs from source; reconfigure or extend the supported inventory']


def check(root=ROOT, build=None):
    root = Path(root)
    status = derive(root)
    errors = []
    if release.read(root / SNAPSHOT) != status:
        errors.append(f'{SNAPSHOT}: stale generated metadata; run project_status.py write')
    for path in DOCUMENTS:
        body = text(root, path)
        if replace_block(body, block(status)) != body:
            errors.append(f'{path}: stale generated status block')
        errors.extend(narrative_errors(body, status, path))
    if build is not None: errors.extend(configured_errors(root, build, status))
    return errors


def write(root=ROOT, build=None):
    root = Path(root); status = derive(root)
    if build is not None:
        errors = configured_errors(root, build, status)
        if errors: raise ValueError('; '.join(errors))
    documents = {path: replace_block(text(root, path), block(status)) for path in DOCUMENTS}
    for path, body in documents.items():
        (root / path).write_text(body, encoding='utf8', newline='\n')
    release.write(root / SNAPSHOT, status)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=['check', 'write'])
    parser.add_argument('--build', help='Also compare the configured CMake version and actual CTest inventory (no test execution)')
    parser.add_argument('--report', type=Path, help='Write a local CI report, separate from the generated source snapshot')
    args = parser.parse_args()
    try:
        if args.operation == 'write': write(build=args.build)
        errors = check(build=args.build)
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as error:
        errors = [str(error)]
    report = {'consistent': not errors, 'errors': errors, 'tests_executed_by_this_command': False,
              'publication_verified': False, 'distribution_gate_evaluated': False}
    if args.report: release.write(args.report, report)
    print(json.dumps(report, ensure_ascii=False))
    return 1 if errors else 0


if __name__ == '__main__':
    raise SystemExit(main())
