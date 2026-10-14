"""Audit the real offline catalog and independent review gates; never fetch data."""
import argparse
import collections
import hashlib
import json
import pathlib
from refine_photos import image_dimensions

ROOT = pathlib.Path(__file__).resolve().parents[1]
RANKS = ('species', 'genus', 'family', 'order', 'class', 'phylum', 'kingdom')
STATES = {'pending', 'in_review', 'approved', 'rejected'}
LICENSES = {'cc0': 'https://creativecommons.org/publicdomain/zero/1.0/',
            'cc-by': 'https://creativecommons.org/licenses/by/4.0/'}

def audit(root=ROOT):
    def read(name):
        return json.loads((root / 'resources/data' / name).read_text(encoding='utf8'))
    species, taxons = read('species.json'), read('taxonomy.json')
    nodes = {t['id']: t for t in taxons}
    hints, reviews, education = read('hint_education.json'), read('curation_reviews.json'), read('education.json')
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
        # Scientific roots are attached to educational roots in the navigation
        # tree; the species' scientific lineage can omit that synthetic parent.
        seen = {'edu:' + s.get('educational_kingdom', '')} if lineage and lineage[0]['rank'] != 'educational_kingdom' else set()
        for node in lineage:
            if node['id'] in seen or (node.get('parent_id') and node['parent_id'] not in seen):
                errors.append({'id': sid, 'error': 'lineage_order_or_cycle'})
            seen.add(node['id'])
        scientific_kingdom = next((t['name'] for t in lineage if t['rank']=='kingdom'), 'unclassified')
        kingdom = s.get('educational_kingdom', 'unclassified')
        expected = {'Animalia': {'Animalia'}, 'Plantae': {'Plantae'}, 'Fungi': {'Fungi'},
                    'Protista': {'Protozoa', 'Chromista'}, 'Monera': {'Bacteria', 'Archaea'}}
        if scientific_kingdom not in expected.get(kingdom, set()): errors.append({'id': sid, 'error': 'educational_mapping'})
        group = diversity.setdefault(kingdom, {'count': 0, 'scientific_kingdoms': collections.Counter(), 'phyla': collections.Counter(), 'classes': collections.Counter()})
        group['count'] += 1; group['scientific_kingdoms'][scientific_kingdom] += 1
        for rank, key in (('phylum', 'phyla'), ('class', 'classes')):
            group[key][next((t['name'] for t in lineage if t['rank']==rank), 'unclassified')] += 1
        p = s.get('photo', {}); path = root / p.get('path', '')
        association = p.get('scientific_name') == s['name'] if p.get('role', 'identification') == 'identification' else p.get('supporting_for') == s['name'] and kingdom == 'Monera'
        license_ok = p.get('license') in LICENSES and p.get('license_url') == LICENSES.get(p.get('license'))
        if not license_ok: incompatible_licenses.append(sid)
        valid = association and license_ok and p.get('source_verified') and p.get('author') and p.get('author') != 'no rights reserved' and path.is_file()
        if valid:
            valid = hashlib.sha256(path.read_bytes()).hexdigest() == p.get('sha256') and min(image_dimensions(path)) >= 300
        if valid: valid_photos.append(sid)
        else: errors.append({'id': sid, 'error': 'photo_integrity_or_metadata'})
        approved = valid and review.get('photo') == 'approved' and review.get('photo_id') == p.get('id') and review.get('taxonomy') == 'approved'
        for rank in RANKS:
            answer = s['name'] if rank == 'species' else next((t['name'] for t in lineage if t['rank']==rank), '')
            if approved and rank in p.get('approved_ranks', []) and answer: eligible[rank].append((sid, answer))
        if review.get('etymology') == 'approved' and any(n.get('verified') and n.get('source_url', '').startswith('https://') and 'inaturalist.org' not in n['source_url'] for n in content.get('etymology_hints', {}).values()): etymology.append(sid)
        m = content.get('morphological_hint', {})
        if review.get('morphology') == 'approved' and m.get('photo_id') == p.get('id') and m.get('reference'): morphology.append(sid)
        pending = [f for f in ('taxonomy', 'photo', 'etymology', 'morphology', 'education') if review.get(f) != 'approved']
        if pending: incomplete.append({'id': sid, 'name': s['name'], 'fields': pending})
    for rank, values in eligible.items():
        if len({name.casefold() for _, name in values}) < 4: eligible[rank] = []
    by_id = {str(s['id']): s for s in species}
    competitive = [sid for sid, _ in eligible['genus'] if by_id[sid]['photo'].get('role', 'identification') == 'identification']
    return {'total_species': len(species), 'valid_photo_records': len(valid_photos),
            'photo_validation': 'SHA-256, metadata and dimensions; full decoder validation belongs to Qt tests',
            'eligible_by_rank': {r: len(v) for r, v in eligible.items()}, 'eligible_ids_by_rank': {r: [sid for sid, _ in v] for r, v in eligible.items()},
            'competitive_pool': len(competitive), 'verified_etymology': len(etymology), 'verified_morphology': len(morphology),
            'taxon_introductions': len(education.get('taxons', {})), 'diversity': diversity,
            'incomplete_records': incomplete, 'incompatible_photo_licenses': incompatible_licenses, 'errors': errors}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=pathlib.Path, default=ROOT / 'build/catalog-quality.json')
    parser.add_argument('--strict', action='store_true')
    args = parser.parse_args(); result = audit()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({k: result[k] for k in ('total_species', 'valid_photo_records', 'eligible_by_rank', 'competitive_pool', 'verified_etymology', 'verified_morphology', 'taxon_introductions')}, ensure_ascii=False))
    if args.strict and (result['errors'] or result['incompatible_photo_licenses']): raise SystemExit(1)
