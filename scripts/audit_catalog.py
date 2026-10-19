"""Audit the real offline catalog and independent review gates; never fetch data."""
import argparse
import collections
import hashlib
import json
import pathlib
from refine_photos import image_dimensions
from content_governance import audit_governance, records, status, policy, load, read_ledger, license_allowed, documented_exception, usable_scientific_hint_note, source_declared_photo_license

ROOT = pathlib.Path(__file__).resolve().parents[1]
RANKS = ('species', 'genus', 'family', 'order', 'class', 'phylum', 'kingdom', 'supergroup', 'domain')
STATES = {'pending', 'in_review', 'approved', 'rejected'}
LICENSES = {'cc0': 'https://creativecommons.org/publicdomain/zero/1.0/',
            'cc-by': 'https://creativecommons.org/licenses/by/4.0/'}

def audit(root=ROOT):
    def read(name):
        return json.loads((root / 'resources/data' / name).read_text(encoding='utf8'))
    species, taxons = read('species.json'), read('taxonomy.json')
    nodes = {t['id']: t for t in taxons}
    hints, reviews, education = read('hint_education.json'), read('curation_reviews.json'), read('education.json')
    baseline = load(root / 'resources/compliance/review_baseline.json', {})
    current = records(root)
    try: events = read_ledger(root)
    except ValueError: events = []  # The separate governance report fails integrity.
    settings = policy(root)
    provenance = read('photo_provenance.json')
    def usable(sid, topic):
        key = f'topic:{sid}:{topic}'
        return status(key, current[key], baseline, events, settings)['usable']
    def usable_note(sid, key, topic):
        if key not in current:
            return False
        note = dict(education['notes'][key], governance=status(key, current[key], baseline, events, settings))
        topic_key = f'topic:{sid}:{topic}'
        topic_review = status(topic_key, current[topic_key], baseline, events, settings)
        return usable_scientific_hint_note(note, topic_review, topic)
    eligible = {r: [] for r in RANKS}
    diversity, valid_photos, incompatible_licenses, errors, incomplete = {}, [], [], [], []
    etymology, morphology, competitive = [], [], []
    for s in species:
        sid = str(s['id']); review = reviews.get(sid, {}); content = hints.get(sid, {})
        if review.get('scientific_name') != s['name'] or content.get('scientific_name') != s['name']:
            errors.append({'id': sid, 'error': 'review_identity'})
        if any(review.get(f) not in STATES for f in ('taxonomy', 'photo', 'etymology', 'morphology', 'education')):
            errors.append({'id': sid, 'error': 'review_state'})
        lineage = [nodes[n] for n in s.get('taxonomy', []) if n in nodes]
        if len(lineage) != len(s.get('taxonomy', [])):
            errors.append({'id': sid, 'error': 'missing_taxon'})
        seen = set()
        for node in lineage:
            if node['id'] in seen or (node.get('parent_id') and node['parent_id'] not in seen):
                errors.append({'id': sid, 'error': 'lineage_order_or_cycle'})
            seen.add(node['id'])
        scientific_kingdom = next((t['name'] for t in lineage if t['rank']=='kingdom'), 'unclassified')
        kingdom = s.get('domain', 'unclassified')
        if kingdom not in {'Bacteria','Archaea','Eukaryota'}: errors.append({'id': sid, 'error': 'domain_mapping'})
        group = diversity.setdefault(kingdom, {'count': 0, 'scientific_kingdoms': collections.Counter(), 'phyla': collections.Counter(), 'classes': collections.Counter()})
        group['count'] += 1; group['scientific_kingdoms'][scientific_kingdom] += 1
        for rank, key in (('phylum', 'phyla'), ('class', 'classes')):
            group[key][next((t['name'] for t in lineage if t['rank']==rank), 'unclassified')] += 1
        p = s.get('photo', {}); path = root / p.get('path', '')
        association = p.get('scientific_name') == s['name'] if p.get('role', 'identification') == 'identification' else p.get('supporting_for') == s['name'] and kingdom in {'Bacteria','Archaea'}
        license_ok = ((p.get('license') in LICENSES and p.get('license_url') == LICENSES.get(p.get('license')))
                      or source_declared_photo_license(p, provenance.get(p.get('id'), {}), root)
                      or bool(documented_exception(p, 'distribution', root))) and license_allowed(p, root, 'distribution')
        if not license_ok: incompatible_licenses.append(sid)
        valid = association and p.get('source_verified') and path.is_file()
        if valid:
            try:
                valid = hashlib.sha256(path.read_bytes()).hexdigest() == p.get('sha256') and min(image_dimensions(path)) >= 300
            except (OSError, ValueError): valid = False
        if valid: valid_photos.append(sid)
        else: errors.append({'id': sid, 'error': 'photo_integrity_or_metadata'})
        approved = valid and license_ok and usable(sid, 'photo') and usable(sid, 'taxonomy')
        for rank in RANKS:
            if rank in {'domain','supergroup'} and not usable(sid,'phylogeny'): continue
            answer = s['name'] if rank == 'species' else next((t['name'] for t in lineage if t['rank']==rank), '')
            if approved and rank in p.get('approved_ranks', []) and answer: eligible[rank].append((sid, answer))
        if any(usable_note(sid, key, 'etymology') and 'inaturalist.org' not in education['notes'][key].get('source_url', '')
               for key in education.get('species', {}).get(sid, {}).get('etymology_hints', {}).values()): etymology.append(sid)
        m = content.get('morphological_hint', {})
        if (usable_note(sid, education.get('species', {}).get(sid, {}).get('morphological_hint'), 'morphology')
                and m.get('photo_id') == p.get('id') and m.get('reference')): morphology.append(sid)
        pending = [f for f in ('taxonomy', 'photo', 'etymology', 'morphology', 'education') if review.get(f) != 'approved']
        if pending: incomplete.append({'id': sid, 'name': s['name'], 'fields': pending})
    for rank, values in eligible.items():
        if len({name.casefold() for _, name in values}) < 4: eligible[rank] = []
    by_id = {str(s['id']): s for s in species}
    competitive = [sid for sid, _ in eligible['genus'] if by_id[sid]['photo'].get('role', 'identification') == 'identification']
    from audit_hint_assistance import audit as audit_assistance
    from phylogeny import audit as audit_phylogeny
    phylogeny = audit_phylogeny(species, list(nodes.values()), read('phylogeny.json'))
    errors.extend({'error': 'phylogeny', 'detail': error} for error in phylogeny['errors'])
    assistance_summary={k:v for k,v in audit_assistance(root).items() if k not in {'rows','missing_content'}}
    return {'governance': audit_governance(root), 'hint_assistance': assistance_summary, 'total_species': len(species), 'valid_photo_records': len(valid_photos),
            'photo_validation': 'Technical SHA-256, association and dimensions; decoder belongs to Qt tests. Not permission or scientific certification.',
            'legacy_metrics_notice': 'verified_etymology/morphology count organisms with a usable sourced assertion, including eligible legacy notes. They do not guarantee a hint for every rank, grade or four-option context; see runtime coverage.',
            'eligible_by_rank': {r: len(v) for r, v in eligible.items()}, 'eligible_ids_by_rank': {r: [sid for sid, _ in v] for r, v in eligible.items()},
            'competitive_pool': len(competitive), 'verified_etymology': len(etymology), 'verified_morphology': len(morphology),
            'taxon_introductions': len(education.get('taxons', {})), 'diversity': diversity,
            'incomplete_records': incomplete, 'incompatible_photo_licenses': incompatible_licenses, 'errors': errors}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=pathlib.Path, default=ROOT / 'build/catalog-quality.json')
    parser.add_argument('--strict', action='store_true')
    parser.add_argument('--distribution', action='store_true', help='Fail closed on permission and traceability gaps; not a certification')
    args = parser.parse_args(); result = audit()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({k: result[k] for k in ('total_species', 'valid_photo_records', 'eligible_by_rank', 'competitive_pool', 'verified_etymology', 'verified_morphology', 'taxon_introductions')}, ensure_ascii=False))
    if args.strict and (result['errors'] or result['incompatible_photo_licenses']): raise SystemExit(1)
    if result['governance']['technical_errors'] or result['hint_assistance']['errors']: raise SystemExit(1)
    if args.distribution and (result['governance']['repository_distribution_blocked']
                             or any(f['severity'] == 'high' for f in result['governance']['scientific_findings'])):
        print('DISTRIBUIÇÃO BLOQUEADA: consulte governança, permissões e decisões pendentes no relatório.')
        raise SystemExit(2)
