"""Explicit, offline phylogenetic crosswalk. Never infers missing ranks or approves reviews."""
import argparse
import copy
from collections import Counter
from pathlib import Path

from content_governance import load, save, digest

ROOT = Path(__file__).resolve().parents[1]
POLICY = 'resources/data/phylogeny.json'


def placement(lineage, policy):
    names = {n['name'] for n in lineage}
    matches = [r for r in policy['placements'] if set(r['requires']).issubset(names)
               and (not r.get('requires_any') or names.intersection(r['requires_any']))]
    if len(matches) != 1:
        raise ValueError('Linhagem sem correspondência inequívoca: ' + ', '.join(sorted(names)))
    return matches[0]


def project(species, taxons, policy):
    """Pure projection, preserving an exact copy of each source lineage and all media."""
    index = {n['id']: n for n in taxons}
    result, nodes = [], {n['id']: copy.deepcopy(n) for n in policy['nodes']}
    for original in species:
        s = copy.deepcopy(original)
        source = s.get('source_taxonomy')
        if source is None:
            source = [copy.deepcopy(index[i]) for i in s['taxonomy']]
        rule = placement(source, policy)
        prefix = rule['path']
        retained = [copy.deepcopy(n) for n in source if n['rank'] != 'educational_kingdom'
                    and n['name'] not in rule.get('omit', [])]
        parent = prefix[-1]
        for node in retained:
            node['parent_id'] = parent
            previous = nodes.get(node['id'])
            if previous is not None and previous != node:
                raise ValueError('Táxon com pais incompatíveis: ' + node['id'])
            nodes[node['id']] = node
            parent = node['id']
        if not retained: raise ValueError('Registro sem táxon terminal: ' + s['name'])
        legacy = s.pop('educational_kingdom', s.get('legacy_educational_kingdom', ''))
        s['legacy_educational_kingdom'] = legacy
        s['source_taxonomy'] = source
        s['taxonomy'] = prefix + [n['id'] for n in retained]
        s['domain'] = nodes[prefix[0]]['name']
        s['classification'] = {
            'system': policy['version'], 'placement_id': rule['id'], 'source_refs': rule['source_refs'],
            'evidence_location': rule['evidence_location'], 'limitations': rule['limitations'],
            'divergences': [n for n in source if n['name'] in rule.get('omit', [])],
            'origin': 'ai_assisted_source_synthesis', 'review_state': 'draft',
        }
        result.append(s)
    return result, list(nodes.values())


def audit(species, taxons, policy):
    errors = []
    index = {n['id']: n for n in taxons}
    if len(index) != len(taxons): errors.append('duplicate_taxon_id')
    roots = {n['name'] for n in taxons if not n['parent_id']}
    if roots != {'Bacteria', 'Archaea', 'Eukaryota'}: errors.append('domain_roots')
    try:
        projected_species, projected_nodes = project(species, taxons, policy)
        if projected_species != species or {n['id']:n for n in projected_nodes} != index:
            errors.append('projection_differs_from_source_or_policy')
    except (KeyError, ValueError) as exc: errors.append(str(exc))
    for n in taxons:
        seen, current = set(), n['id']
        while current:
            if current in seen or current not in index:
                errors.append('cycle_or_missing_parent:' + n['id']); break
            seen.add(current); current = index[current]['parent_id']
        if n['name'] in {'Monera', 'Protista', 'Protozoa', 'Chromista'} or n['rank'] == 'educational_kingdom':
            errors.append('obsolete_active_group:' + n['id'])
    rows = []
    for s in species:
        try:
            path = [index[i] for i in s['taxonomy']]
            rule = placement(s['source_taxonomy'], policy)
            if s['taxonomy'][:len(rule['path'])] != rule['path']: raise ValueError('placement_path')
            parent = ''
            for node in path:
                if node['parent_id'] != parent: raise ValueError('incomplete_lineage')
                parent = node['id']
            domains = [n['name'] for n in path if n['rank'] == 'domain']
            if domains != [s['domain']]: raise ValueError('domain_mismatch')
            if s['classification']['source_refs'] != rule['source_refs']: raise ValueError('source_binding')
            if any(ref not in policy['sources'] for ref in rule['source_refs']): raise ValueError('missing_source')
            if not s['classification']['evidence_location']: raise ValueError('missing_locator')
            rows.append({'id': s['id'], 'name': s['name'], 'domain': s['domain'],
                         'supergroup': next((n['name'] for n in path if n['rank'] == 'supergroup'), None),
                         'kingdom': next((n['name'] for n in path if n['rank'] == 'kingdom'), None),
                         'source_refs': rule['source_refs'], 'review_state': s['classification']['review_state'],
                         'classification_sha256': digest({'lineage': path, 'classification': s['classification']})})
        except (KeyError, ValueError) as exc: errors.append(str(s['id']) + ':' + str(exc))
    return {'classification_system': policy['version'], 'total': len(species),
            'domains': dict(Counter(s['domain'] for s in species)),
            'supergroups': dict(Counter(r['supergroup'] for r in rows if r['supergroup'])),
            'errors': errors, 'records': rows,
            'notice': 'Technical crosswalk validation, not independent scientific review. All new placements are drafts.'}


def migrate(root=ROOT):
    species_path, taxonomy_path = root / 'resources/data/species.json', root / 'resources/data/taxonomy.json'
    policy = load(root / POLICY)
    species, taxons = project(load(species_path), load(taxonomy_path), policy)
    report = audit(species, taxons, policy)
    if report['errors']: raise ValueError(report['errors'])
    save(species_path, species); save(taxonomy_path, taxons)
    save(root / 'docs/AUDITORIA_FILOGENETICA.json', report)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    report = migrate() if args.apply else audit(load(ROOT / 'resources/data/species.json'),
        load(ROOT / 'resources/data/taxonomy.json'), load(ROOT / POLICY))
    if not args.apply: save(ROOT / 'build/phylogeny-audit.json', report)
    print({k: v for k, v in report.items() if k != 'records'})
    raise SystemExit(bool(report['errors']))
