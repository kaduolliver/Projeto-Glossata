# SPDX-License-Identifier: Apache-2.0
"""Read-only documentary inventory and checks of the explicitly authorized code scope.

Reuses media authorities and distribution validators. Classification by location
is not evidence of ownership. Package metadata is kept separate from source Git.
"""
import argparse
from collections import Counter
from pathlib import Path
import subprocess

import content_governance as g
import distribution
from rights_investigation import file_hash

ARCHIVE = 'docs/INVENTARIO_LICENCIAMENTO.json'
CODE_SCOPE = 'docs/ESCOPO_LICENCIAMENTO.json'
APACHE_SOURCE = 'https://www.apache.org/licenses/LICENSE-2.0.txt'
# Official ASF text downloaded without editing, including its standard appendix.
APACHE_SHA256 = 'cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30'
CREATOR = 'Carlos Eduardo Lima de Oliveira'
CATEGORIES = {
    'application_code': 'Código do aplicativo, testes e configuração; titularidade a conferir',
    'maintenance_scripts': 'Scripts de manutenção; titularidade a conferir',
    'qml_components': 'Componentes QML do projeto; titularidade a conferir',
    'documentation': 'Documentação e procedimentos; originalidade/titularidade a conferir',
    'educational_text': 'Textos educativos editáveis; direitos e fontes por tópico',
    'external_scientific_data': 'Dados, metadados de fontes e sínteses científicas; direitos por camada',
    'third_party_photographs': 'Fotografias de terceiros; licença por arquivo',
    'visual_identity': 'Identidade visual e elementos gráficos; direitos pendentes',
    'external_dependencies': 'Dependências e ferramentas de terceiros; fora da licença própria',
    'generated_or_derived': 'Gerados, evidências de processo, capturas e fixtures; direitos herdados',
}
GENERATED_DATA = {'education.json', 'credits.json', 'catalog_manifest.json', 'progressive_hints.json',
                  'import_report.json', 'photo_audit.json', 'photo_provenance.json'}


def category(name):
    p = Path(name)
    if p.suffix == '.md': return 'documentation'
    if name.startswith('resources/branding/'): return 'visual_identity'
    if name.startswith('resources/images/'): return 'third_party_photographs'
    if name.startswith('docs/images/') or name.startswith('tests/fixtures/'): return 'generated_or_derived'
    if name.startswith('resources/qml/'): return 'qml_components'
    if name.startswith('scripts/') or (name.startswith('package/') and p.suffix == '.cmd'):
        return 'maintenance_scripts'
    if name.startswith('resources/data/'):
        if p.name in GENERATED_DATA: return 'generated_or_derived'
        if p.name in {'species_profiles.json', 'hint_education.json'}: return 'educational_text'
        if p.name in {'discord.json', 'content_policy.json'}: return 'application_code'
        if p.name in {'curation_reviews.json', 'photo_reviews.json'}: return 'documentation'
        return 'external_scientific_data'
    if name.startswith('resources/compliance/'): return 'generated_or_derived'
    if name.startswith('docs/') and p.suffix == '.json': return 'generated_or_derived'
    if name.startswith('docs/'): return 'documentation'
    if name.startswith(('vendor/', 'third_party/')): return 'external_dependencies'
    return 'application_code'


def tracked(root):
    result = subprocess.run(['git', '--no-replace-objects', '-C', str(root), 'ls-files', '-z'],
                            capture_output=True, check=True, timeout=30)
    return sorted(set(result.stdout.decode('utf8').split('\0')) - {''})


def code_candidate(name):
    """Inventory candidates only. This predicate NEVER grants a license."""
    p = Path(name)
    if name in {'CMakeLists.txt', 'resources.qrc', '.gitignore', '.gitattributes',
                '.githooks/pre-commit', 'package/run_glossata_game.cmd'}:
        return True
    if name.startswith('src/'):
        return p.suffix in {'.cpp', '.h'} or name.endswith('.rc.in')
    if name.startswith('resources/qml/'):
        return p.suffix == '.qml'
    if name.startswith('scripts/'):
        return p.suffix in {'.py', '.ps1'}
    if name.startswith('tests/') and not name.startswith('tests/fixtures/'):
        return p.suffix in {'.cpp', '.py', '.qrc'} or p.name == 'CMakeLists.txt'
    return name.startswith('.github/workflows/') and p.suffix in {'.yml', '.yaml'}


def check_code_license(root=g.ROOT, paths=None):
    """Verify declared scope/official bytes, not copyright title or human identity.

    No permission writers, network, policy mutation or scientific approvals.
    New candidate paths require an explicit scope decision, never folder inheritance.
    """
    root = Path(root)
    scope = g.load(root / CODE_SCOPE, {})
    errors = []
    if file_hash(root / 'LICENSE') != APACHE_SHA256:
        errors.append('apache_text_missing_or_modified')
    if scope.get('license') != 'Apache-2.0' or scope.get('schema_version') != 1:
        errors.append('code_scope_missing_or_invalid')
    notice_path = root / 'NOTICE'
    notice = notice_path.read_text(encoding='utf8') if notice_path.is_file() else ''
    for required in ('Projeto Glossata', CREATOR, 'https://github.com/kaduolliver/Projeto-Glossata',
                     'Apache License, Version 2.0', CODE_SCOPE):
        if required not in notice:
            errors.append('notice_missing_attribution_or_scope')
            break
    covered = scope.get('files', [])
    pending = scope.get('pending_files', {})
    if not isinstance(covered, list) or not isinstance(pending, dict):
        return errors + ['invalid_scope_structure']
    if len(covered) != len(set(covered)) or set(covered).intersection(pending):
        errors.append('duplicate_or_conflicting_scope')
    for name in covered + list(pending):
        try:
            path = g.local_file(root, name)
            if (not code_candidate(name) or '\\' in name or name != Path(name).as_posix()
                    or not path.is_file()):
                errors.append('noncode_or_missing_scope_path:' + name)
        except (ValueError, TypeError):
            errors.append('unsafe_scope_path')
    if any(not isinstance(reason, str) or not reason.strip() for reason in pending.values()):
        errors.append('pending_scope_reason_missing')
    candidates = {name for name in (tracked(root) if paths is None else paths) if code_candidate(name)}
    if candidates != set(covered) | set(pending):
        errors.append('code_scope_inventory_differs')
    decision = scope.get('maintainer_decision', {})
    policy = g.policy(root)
    declarations = policy.get('own_material_licenses', {})
    assets = g.load(root / 'resources/compliance/asset_rights.json', {})
    if any(a.get('path') in covered and a.get('state', '').startswith('pending')
           for a in assets.get('assets', [])):
        errors.append('pending_material_in_code_scope')
    if declarations != {'code': 'Apache-2.0', 'texts': None, 'collection': None}:
        errors.append('own_license_declarations_differ')
    if assets.get('own_material_licenses') != declarations:
        errors.append('asset_and_policy_declarations_differ')
    if (not decision.get('id') or decision.get('scope') != CODE_SCOPE
            or decision.get('license') != 'Apache-2.0'
            or decision.get('original_creator') != CREATOR
            or decision not in policy.get('maintainer_decisions', [])):
        errors.append('administrative_decision_not_linked')
    if scope.get('license_text') != {'source_url': APACHE_SOURCE, 'sha256': APACHE_SHA256}:
        errors.append('official_license_reference_differs')
    return errors


def collect(root=g.ROOT, package=None, prefix=None, at=None):
    root = Path(root); package = Path(package) if package else None
    commit = subprocess.check_output(['git', '--no-replace-objects', '-C', str(root), 'rev-parse', 'HEAD'],
                                     timeout=15).decode('ascii').strip()
    credits = g.credits(root)
    photo_by_path = {s['photo']['path']: credits['photos'][str(s['photo']['id'])]
                     for s in g.load(root / 'resources/data/species.json', [])}
    presentation = {p['path']: p for p in credits.get('exploration_photos', {}).values()}
    assets = {a['path']: a for a in g.load(root / 'resources/compliance/asset_rights.json', {}).get('assets', [])}
    rows = []
    for name in tracked(root):
        if name == ARCHIVE: continue  # No self-referential report hash.
        path = g.local_file(root, name); kind = category(name)
        row = {'path': name, 'category': kind, 'sha256': file_hash(path),
               'ownership_confirmed_by_this_inventory': False}
        if name in photo_by_path:
            p = photo_by_path[name]
            row.update(rights_reference='photo:' + p['photo_id'], author_credit_as_supplied=p['author'],
                       declared_license=p['license'], evidenced_license_version=p['license_version'],
                       documentary_issues=p['rights_issues'])
        elif name in presentation:
            p = presentation[name]
            row.update(rights_reference=p['photo_url'], author_credit_as_supplied=p['author'],
                       declared_license=p['license'], evidenced_license_version=p['license_version'],
                       documentary_issues=p['rights_issues'])
        elif name in assets:
            a = assets[name]
            row.update(rights_reference='resources/compliance/asset_rights.json',
                       declared_license=a.get('license'), declared_state=a.get('state'))
        if name == 'scripts/build_education.py': row['also_contains'] = ['educational_text']
        rows.append(row)
    inventory = g.load(package / 'compliance/dependencies.json', {}) if package else {}
    components = []
    for p in inventory.get('packages', []):
        binaries = [b for b in inventory.get('binaries', []) if b.get('package') == p['name']]
        components.append({**p, 'packaged_files': len(binaries),
            'packaged_extensions': dict(sorted(Counter(Path(b['path']).suffix for b in binaries).items())),
            'sample_paths': [b['path'] for b in binaries[:4]],
            'notice_integrity': [{**notice, 'matches': file_hash(g.local_file(package, notice['path'])) == notice.get('sha256')}
                                 for notice in p.get('license_files', [])]})
    additional = []
    # Explicitly selected installation, never a new copy/grant. ICU stores its
    # notice outside share/licenses, which the current exporter alone searches.
    if prefix:
        for p in components:
            if p['name'].endswith('-icu') and p.get('version'):
                release = p['version'][0].rsplit('-', 1)[0]
                relative = 'share/icu/' + release + '/LICENSE'
                path = g.local_file(Path(prefix), relative)
                installed = g.local_file(Path(prefix).parent,
                    'var/lib/pacman/local/' + p['name'] + '-' + p['version'][0] + '/desc')
                metadata = distribution.pacman_fields(installed) if installed.is_file() else {}
                additional.append({'package': p['name'], 'version': p['version'],
                    'installation_relative_path': relative, 'exists': path.is_file(), 'sha256': file_hash(path),
                    'installed_package_version_matches': metadata.get('NAME') == [p['name']] and metadata.get('VERSION') == p['version'],
                    'action': 'Review and include the exact-version notice in a future export; no package changed or approved.'})
    gate = distribution.release_gate(root, package)
    return {'schema_version': 1, 'report_kind': 'licensing_preparation_only', 'checked_at': at or g.now(),
        'git_reference': commit, 'scope': 'Current tracked paths and worktree bytes; runtime package inventoried separately.',
        'classification_basis': 'Location and existing provenance only; category is not proof of authorship, originality or permission.',
        'excluded_self_reference': ARCHIVE, 'categories': CATEGORIES,
        'counts': {key: sum(r['category'] == key for r in rows) for key in CATEGORIES}, 'files': rows,
        'own_material_license_declarations': g.policy(root).get('own_material_licenses', {}),
        'license_selected_by_this_report': False, 'approvals_created': 0,
        'package': {'assessed': bool(package), 'inventory_path': 'package/compliance/dependencies.json' if package else None,
            'inventory_sha256': file_hash(package / 'compliance/dependencies.json') if package else None,
            'inventoried_at': inventory.get('inventoried_at'), 'file_count': len(inventory.get('binaries', [])),
            'components': components, 'additional_notice_candidates': additional,
            'errors': gate['dependency_errors'], 'technical_errors': gate['technical_errors'] if package else [],
            'limitations': 'Package owner/aggregate expression does not establish each Qt module, embedded component or chosen license branch. Not a complete SBOM.'},
        'gate': {'distribution_blocked': gate['distribution_blocked'], 'photo_findings': len(gate['photo_licensing']),
                 'other_material_findings': len(gate['asset_licensing']), 'scientific_blocks': len(gate['scientific_release_blocks'])}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', type=Path); parser.add_argument('--prefix', type=Path)
    parser.add_argument('--archive', action='store_true', help='Write the documentary snapshot in docs; never a license or approval')
    parser.add_argument('--check-code', action='store_true', help='Validate explicit code scope and official license offline; never confirms ownership')
    args = parser.parse_args()
    if args.check_code:
        if args.archive or args.package or args.prefix:
            parser.error('--check-code is read-only and does not collect/archive a package')
        errors = check_code_license()
        print({'code_license_errors': errors, 'ownership_or_scientific_approval': False})
        raise SystemExit(1 if errors else 0)
    result = collect(package=args.package, prefix=args.prefix)
    path = g.ROOT / (ARCHIVE if args.archive else 'build/licensing-inventory.json')
    g.save(path, result)
    print({'report': path.relative_to(g.ROOT).as_posix(), 'files': len(result['files']),
           'dependency_components': len(result['package']['components']), 'license_selected_by_this_report': False,
           'own_material_license_declarations': result['own_material_license_declarations'],
           'distribution_blocked': result['gate']['distribution_blocked']})


if __name__ == '__main__': main()
