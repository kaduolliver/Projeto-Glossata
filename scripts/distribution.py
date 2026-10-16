"""Inventory a local Windows package and gate redistribution, never grant rights."""
import argparse
import hashlib
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET
from content_governance import ROOT, audit_governance, digest, load, local_file, now, parse_time, read_chain, save
from datetime import datetime, timezone
AUDITED_EXTENSIONS = {'.exe', '.dll', '.qml', '.ttf', '.otf', '.png', '.jpg', '.jpeg', '.ico', '.svg', '.webp', '.js'}


def pacman_fields(path):
    result = {}; key = None
    for line in path.read_text(encoding='utf8', errors='replace').splitlines():
        if line.startswith('%') and line.endswith('%'): key = line.strip('%'); result[key] = []
        elif key and line: result[key].append(line)
    return result


def dependencies(package, prefix=Path('C:/msys64/ucrt64')):
    """Match actual bytes to installed package owners; no guessed DLL license."""
    db = prefix.parent / 'var/lib/pacman/local'; owners = {}; descriptions = {}; by_name = {}; source_hashes = {}
    if db.is_dir():
        for path in db.iterdir():
            if not (path / 'desc').is_file() or not (path / 'files').is_file(): continue
            desc = pacman_fields(path / 'desc'); files = pacman_fields(path / 'files').get('FILES', [])
            name = desc.get('NAME', [path.name])[0]
            descriptions[name] = (desc, files)
            for filename in files:
                if filename.startswith(prefix.name + '/'):
                    relative = filename[len(prefix.name)+1:].lower()
                    owners[relative] = name
                    if not filename.endswith('/'):
                        by_name.setdefault(relative.rsplit('/', 1)[-1], []).append(relative)
    binaries = []; used = set()
    project_media = {s.get('photo', {}).get('sha256') for s in load(ROOT / 'resources/data/species.json', [])}
    project_media.update(a.get('sha256') for a in load(ROOT / 'resources/compliance/asset_rights.json', {}).get('assets', []))
    for path in sorted(package.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in AUDITED_EXTENSIONS: continue
        if 'compliance' in path.relative_to(package).parts: continue
        sha = hashlib.sha256(path.read_bytes()).hexdigest()
        # Qt deploy changes its directory layout; find the exact file by name/hash.
        candidates = by_name.get(path.name.lower(), [])
        found = None
        for candidate in candidates:
            source = prefix / candidate
            if source.is_file() and candidate not in source_hashes:
                source_hashes[candidate] = hashlib.sha256(source.read_bytes()).hexdigest()
            if source_hashes.get(candidate) == sha:
                found = owners[candidate]; used.add(found); break
        own = path.name == 'projeto_glossata.exe'
        own_media = sha in project_media and path.suffix.lower() not in {'.exe', '.dll', '.qml', '.js'}
        binaries.append({'path': path.relative_to(package).as_posix(), 'sha256': sha, 'package': found,
                         'kind': 'project_executable' if own else 'project_media' if own_media else 'third_party',
                         'state': 'own_material_license_decision_pending' if own else 'inventoried_pending_obligations_review' if found else 'owner_not_identified'})
    packages = []
    license_root = package / 'compliance/dependency-licenses'; license_root.mkdir(parents=True, exist_ok=True)
    for name in sorted(used):
        desc, files = descriptions[name]; copied = []
        for relative in files:
            if '/share/licenses/' not in relative or relative.endswith('/'): continue
            source = prefix.parent / relative
            if source.is_file():
                target = license_root / name / relative.split('/share/licenses/', 1)[1]; target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target); copied.append({'path': target.relative_to(package).as_posix(), 'sha256': hashlib.sha256(target.read_bytes()).hexdigest()})
        component = {'name': name, 'version': desc.get('VERSION', []), 'declared_licenses': desc.get('LICENSE', []),
                         'upstream_url': desc.get('URL', []), 'license_files': copied,
                         'source_provision': None, 'obligations_review': 'pending',
                         'note': 'Installed package declarations are inventory evidence, not a determination of every component obligation.'}
        component['content_sha256'] = digest({'name': name, 'version': component['version'], 'declared_licenses': component['declared_licenses'],
            'license_files': copied, 'binary_hashes': sorted(b['sha256'] for b in binaries if b.get('package') == name)})
        prior = next((r for r in reversed(read_chain(ROOT / 'resources/compliance/dependency_reviews.jsonl'))
                      if r.get('package') == name and r.get('content_sha256') == component['content_sha256']), None)
        if prior and not dependency_decision_errors(prior, ROOT):
            component.update({key: prior.get(key) for key in ('source_provision', 'reviewer', 'reviewed_at', 'evidence_path', 'evidence_sha256')})
            component['obligations_review'] = 'documented'
        packages.append(component)
    return {'schema_version': 1, 'inventoried_at': now(), 'binaries': binaries, 'packages': packages,
            'linking': 'Qt libraries dynamically loaded; source provision/replacement instructions still require maintainer review.',
            'scope': 'All packaged DLL/EXE/QML/font files scanned. Third-party portions embedded in Qt still require module/SBOM review.'}


def dependency_errors(inventory, package):
    errors = []
    known = {b['path'] for b in inventory.get('binaries', [])}
    for path in package.rglob('*'):
        if path.is_file() and (path.suffix.lower() in {'.sqlite', '.sqlite3', '.db', '.log', '.key', '.pem'} or path.name == '.env'):
            errors.append({'path': path.relative_to(package).as_posix(), 'error': 'local_or_sensitive_file_in_distribution'})
        if path.is_file() and path.suffix.lower() in AUDITED_EXTENSIONS and 'compliance' not in path.relative_to(package).parts:
            if path.relative_to(package).as_posix() not in known:
                errors.append({'path': path.relative_to(package).as_posix(), 'error': 'dependency_not_inventoried'})
    for binary in inventory.get('binaries', []):
        path = local_file(package, binary['path'])
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != binary.get('sha256'):
            errors.append({'path': binary['path'], 'error': 'inventory_changed'})
        if binary.get('kind') == 'third_party' and not binary.get('package'):
            errors.append({'path': binary['path'], 'error': 'dependency_owner_unknown'})
    packages = inventory.get('packages', [])
    if not packages: errors.append({'error': 'dependency_inventory_missing'})
    for component in packages:
        for notice in component.get('license_files', []):
            path = local_file(package, notice['path'])
            if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != notice.get('sha256'):
                errors.append({'package': component['name'], 'error': 'dependency_license_notice_changed_or_missing'})
        if not component.get('license_files') or component.get('obligations_review') != 'documented':
            errors.append({'package': component['name'], 'error': 'dependency_obligations_unresolved'})
        if not component.get('reviewer') or not component.get('reviewed_at') or not component.get('evidence_path'):
            errors.append({'package': component['name'], 'error': 'dependency_review_not_traceable'})
        # Offer/complete source delivery must be decided and supported for copyleft.
        if any('GPL' in value for value in component.get('declared_licenses', [])) and not component.get('source_provision'):
            errors.append({'package': component['name'], 'error': 'source_provision_not_documented'})
    return errors


def dependency_decision_errors(decision, root):
    errors = []
    when = parse_time(decision.get('reviewed_at'))
    if decision.get('actor_type') != 'human' or not decision.get('reviewer') or not decision.get('rationale') or not when or when > datetime.now(timezone.utc):
        errors.append('human_dependency_review_missing')
    try:
        evidence = local_file(root, decision['evidence_path'])
        if not evidence.is_file() or hashlib.sha256(evidence.read_bytes()).hexdigest() != decision.get('evidence_sha256'):
            errors.append('dependency_review_evidence_invalid')
    except (KeyError, ValueError): errors.append('dependency_review_evidence_missing')
    provision = decision.get('source_provision', {})
    if not isinstance(provision, dict) or provision.get('method') not in {'bundled_source', 'written_offer', 'not_required'}:
        errors.append('source_provision_decision_missing')
    elif provision['method'] != 'not_required':
        for field in ('source_delivery', 'replacement_instructions'):
            proof = provision.get(field, {})
            try:
                path = local_file(root, proof['path'])
                if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != proof.get('sha256'):
                    errors.append('source_or_replacement_evidence_invalid')
            except (KeyError, ValueError): errors.append('source_or_replacement_evidence_missing')
    return errors


def record_dependency_review(root, package, decision):
    inventory = load(package / 'compliance/dependencies.json', {})
    current = next((c for c in inventory.get('packages', []) if c['name'] == decision.get('package')), None)
    if not current or current.get('content_sha256') != decision.get('content_sha256') or dependency_decision_errors(decision, root):
        raise ValueError('Revisão de dependência exige hash do componente atual, pessoa, data, evidência e decisão de fornecimento de código.')
    if any('GPL' in value for value in current.get('declared_licenses', [])) and decision.get('source_provision', {}).get('method') == 'not_required':
        raise ValueError('Componentes com declaração GPL/LGPL exigem análise e meio documentado de fornecimento; não presumir dispensa.')
    path = root / 'resources/compliance/dependency_reviews.jsonl'; events = read_chain(path)
    event = dict(decision, recorded_at=now(), previous_sha256=events[-1]['event_sha256'] if events else '')
    event['event_sha256'] = digest(event); path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a', encoding='utf8', newline='\n') as output:
        import json
        output.write(json.dumps(event, ensure_ascii=False) + '\n')
    return event


def export_local(root, package, prefix):
    destination = package / 'compliance'; destination.mkdir(parents=True, exist_ok=True)
    for name in ('PHOTO_CREDITS.md', 'CONFORMIDADE.md', 'REPORTAR_PROBLEMA.md', 'DIREITOS_E_LICENCAS.md',
                 'RELATORIO_CONFORMIDADE.md', 'CATALOGO_CHANGELOG.md', 'VERSIONAMENTO.md',
                 'DICAS_E_PONTUACAO.md', 'AUDITORIA_ASSISTENCIA.json', 'CLASSIFICACAO_FILOGENETICA.md', 'AUDITORIA_FILOGENETICA.json', 'EXPLORACAO_E_FOTOGRAFIAS.md',
                 'PROVEDORES_CIENTIFICOS.md', 'VALIDACAO_PROVEDORES.json', 'AUDITORIA_FOTOGRAFIAS.json'):
        source = root / 'docs' / name
        if source.is_file(): shutil.copy2(source, destination / name)
    for name in ('credits.json', 'photo_provenance.json', 'content_policy.json', 'exploration_photos.json'):
        shutil.copy2(root / 'resources/data' / name, destination / name)
    for name in ('VERSION.txt', 'CHANGELOG.md', 'resources/data/catalog_manifest.json'):
        source = root / name
        if source.is_file(): shutil.copy2(source, destination / source.name)
    shutil.copytree(root / 'resources/compliance', destination / 'evidence', dirs_exist_ok=True)
    inventory = dependencies(package, prefix)
    save(destination / 'dependencies.json', inventory)
    manifest = root / 'build/build-manifest.json'
    if manifest.is_file(): shutil.copy2(manifest, destination / 'build-manifest.json')
    save(destination / 'audit.json', audit_governance(root))
    (package / 'LOCAL_DEVELOPMENT_ONLY.txt').write_text(
        'Pacote local de desenvolvimento. Redistribuição bloqueada enquanto houver permissões ou obrigações não demonstradas.\n'
        'Consulte compliance/audit.json e compliance/dependencies.json. Este arquivo não é uma licença.\n', encoding='utf8')
    return inventory


def build_inputs(root=ROOT):
    paths = {root / 'resources.qrc', root / 'CMakeLists.txt'}
    if (root / 'VERSION.txt').is_file(): paths.add(root / 'VERSION.txt')
    paths.update(p for p in (root / 'src').rglob('*') if p.is_file())
    if (root / 'resources.qrc').exists():
        paths.update(local_file(root, e.text) for e in ET.parse(root / 'resources.qrc').getroot().iter('file'))
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}


def record_build(binary, root=ROOT):
    from release import app_version, read, CATALOG
    catalog = read(root / CATALOG) if (root / CATALOG).is_file() else {}
    manifest = {'schema_version': 1, 'built_at': now(), 'binary': binary.name,
                'application_version': app_version(root), 'catalog_version': catalog.get('version'),
                'catalog_sha256': catalog.get('content_sha256'),
                'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(), 'inputs': build_inputs(root),
                'notice': 'Build integrity manifest only, not permission or scientific approval.'}
    save(root / 'build/build-manifest.json', manifest)
    return manifest


def release_gate(root=ROOT, package=None):
    report = audit_governance(root)
    if package:
        manifest = load(package / 'compliance/build-manifest.json', {})
        executable = package / 'projeto_glossata.exe'
        if (not executable.is_file() or not manifest or manifest.get('inputs') != build_inputs(root)
                or manifest.get('binary_sha256') != hashlib.sha256(executable.read_bytes()).hexdigest()):
            report['technical_errors'].append({'error': 'packaged_binary_or_compiled_sources_stale'})
            report['distribution_blocked'] = True
        inventory = load(package / 'compliance/dependencies.json', {})
        report['dependency_errors'] = dependency_errors(inventory, package)
        try:
            decisions = read_chain(root / 'resources/compliance/dependency_reviews.jsonl')
            for component in inventory.get('packages', []):
                matching = next((d for d in reversed(decisions) if d.get('package') == component['name'] and d.get('content_sha256') == component.get('content_sha256')), None)
                if not matching or dependency_decision_errors(matching, root):
                    report['dependency_errors'].append({'package': component['name'], 'error': 'dependency_review_not_bound_to_inventory'})
        except ValueError: report['dependency_errors'].append({'error': 'dependency_review_history_integrity'})
    else:
        report['dependency_errors'] = []
        report['distribution_blocked'] |= report['repository_distribution_blocked']
    # High-severity scientific issues (changed/rejected content) require triage.
    report['scientific_release_blocks'] = [f for f in report['scientific_findings'] if f['severity'] == 'high']
    report['distribution_blocked'] |= bool(report['dependency_errors'] or report['scientific_release_blocks'])
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['check', 'export-local', 'record-review', 'record-build'])
    parser.add_argument('--package', type=Path)
    parser.add_argument('--prefix', type=Path, default=Path('C:/msys64/ucrt64'))
    parser.add_argument('--decision', type=Path)
    parser.add_argument('--binary', type=Path)
    args = parser.parse_args()
    package = args.package.resolve() if args.package else None
    if args.action == 'record-build':
        if not args.binary: parser.error('--binary é necessário')
        record_build(args.binary.resolve())
    elif args.action == 'record-review':
        if not package or not args.decision: parser.error('--package e --decision são necessários')
        print(record_dependency_review(ROOT, package, load(args.decision))['event_sha256'])
    elif args.action == 'export-local':
        if not package: parser.error('--package obrigatório')
        data = export_local(ROOT, package, args.prefix)
        print('Dependências inventariadas:', len(data['binaries']), 'pacotes identificados:', len(data['packages']))
    else:
        report = release_gate(package=package); save(ROOT / 'build/distribution-audit.json', report)
        if report['distribution_blocked']:
            print('DISTRIBUIÇÃO BLOQUEADA. Permissões/revisões/obrigações pendentes; consulte build/distribution-audit.json.')
            raise SystemExit(2)
        print('Verificações automáticas concluídas; isso não certifica conformidade jurídica ou científica.')
