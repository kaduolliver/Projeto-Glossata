"""Offline, explicit pedagogical assessment; never a scientific/human approval.

Reuses existing source assertions only. Run --evaluate after inspecting changes;
normal education generation never renews dates or approves changed inputs.
"""
import argparse
import hashlib
from datetime import timedelta
from content_governance import ROOT, load, save, digest, now, parse_time

POLICY = 'progressive-pedagogy-v1'
RANKS = ['kingdom', 'phylum', 'class', 'order', 'family', 'genus', 'species']


def inputs(root):
    names = ['species.json', 'taxonomy.json', 'hint_education.json', 'species_profiles.json',
             'curation_reviews.json', 'content_policy.json']
    result = {name: digest(load(root / 'resources/data' / name)) for name in names}
    for name in ['scripts/review_hints.py', 'src/application/EducationalContent.cpp', 'src/domain/Hint.h',
                 'src/infrastructure/content/ReviewStatus.h',
                 'resources/compliance/review_history.jsonl', 'resources/compliance/review_baseline.json']:
        result[name] = hashlib.sha256((root/name).read_bytes() if (root/name).exists() else b'').hexdigest()
    return result


def prepare(root=ROOT):
    species = load(root/'resources/data/species.json')
    hints = load(root/'resources/data/hint_education.json')
    profiles = load(root/'resources/data/species_profiles.json')
    nodes = {t['id']: t for t in load(root/'resources/data/taxonomy.json')}
    rows = {}
    for s in species:
        sid = str(s['id']); h = hints[sid]; recipes = []
        # Source-specific lineage records, not draft synthesized clades.
        for n in (nodes[key] for key in s['taxonomy']):
            if n['rank'] not in RANKS[:-1] or n.get('source') != 'inaturalist' or not n.get('source_taxon_id'): continue
            recipes.append({'id': 'ancestor-'+n['id'], 'category': 'taxonomic', 'kind': 'ancestor',
                'node': n, 'source_url': 'https://www.inaturalist.org/taxa/'+n['source_taxon_id'],
                'source_title': 'iNaturalist · registro taxonômico do ancestral',
                'locator': 'taxon '+n['source_taxon_id']+' / ancestors; catálogo local '+str(s['id']),
                'rationale': 'A pista apresenta contexto de um ancestral. Seu grau depende das regras de especificidade e das linhagens efetivamente conhecidas nas alternativas; a quantidade de alternativas compatíveis é apenas uma salvaguarda, sem medir dificuldade humana.'})
        m = h.get('morphological_hint', {})
        a = m.get('assistance', {})
        if m and a.get('suitability') != 'inadequate':
            note = profiles[sid][m['reference']]
            original_text=m.get('prefix','')+note['text']
            scope=[r for r in s['photo'].get('approved_ranks',[]) if r in RANKS]
            recipes.append({'id': 'appearance', 'kind': 'note', 'category': 'morphological',
                'text': original_text, 'source_text': original_text, 'ranks': scope,
                'original_ranks':m['ranks'], 'organism_scope':True,
                'source_url': note['source_url'], 'source_title': note['source_title'],
                'locator': note.get('locator', 'Ficha local '+sid+' / '+m['reference']),
                'grade': a['grade'], 'rationale': a['rationale']+' Quando o nível solicitado é mais amplo, a descrição continua restrita ao organismo do registro; ela não caracteriza todos os integrantes do táxon.', 'scientific_topic': 'morphology',
                'source_note': 'species:'+sid+':morphological_hint'})
        for rank, n in h.get('etymology_hints', {}).items():
            a = n['assistance']
            recipes.append({'id': 'origin-'+rank, 'kind': 'note', 'category': 'etymological',
                'text': n['text'], 'ranks': [rank], 'source_url': n['source_url'],
                'source_title': n['source_title'], 'locator': n.get('locator', 'resources/data/hint_education.json / '+sid+'/etymology_hints/'+rank),
                'grade': a['grade'], 'rationale': a['rationale'],
                'association_token': a.get('association_token',''), 'scientific_topic': 'etymology',
                'source_note': 'species:'+sid+':etymology:'+rank})
        rows[sid] = {'scientific_name': s['name'], 'photo': {'id': s['photo']['id'], 'sha256': s['photo']['sha256']},
                     'recipes': recipes}
    return {'policy': POLICY, 'inputs': inputs(root), 'species': rows}


def evaluate(root=ROOT):
    data = prepare(root); previous = load(root/'resources/data/progressive_hints.json', {})
    fingerprint = digest(data)
    if previous.get('evaluation',{}).get('content_sha256') == fingerprint and valid(previous, root):
        return previous  # Idempotent: no clock-based renewal.
    stamp = now()
    days=load(root/'resources/data/content_policy.json')['review_days']['assistance']
    data['evaluation'] = {'state':'approved_automated', 'actor_type':'automated',
        'process':'scripts/review_hints.py + contextual validation in EducationalContent',
        'policy_version':POLICY, 'reviewed_at':stamp,
        'due_at':(parse_time(stamp)+timedelta(days=days)).isoformat(), 'content_sha256':fingerprint,
        'rationale':'Esta avaliação é automática e se limita aos controles pedagógicos implementados: vínculos de identidade e mídia, linhagens conhecidas, restrições de revelação e progressão de pistas. As decisões editoriais, científicas e de direitos seguem os fluxos próprios.',
        'limitations':'O resultado automático não comprova dificuldade para pessoas, revisão por especialista ou permissão de redistribuição. A condição de cada conteúdo e evidência continua sujeita aos controles independentes de governança.'}
    save(root/'resources/data/progressive_hints.json', data)
    return data


def valid(data, root=ROOT):
    from datetime import datetime, timezone
    ev=data.get('evaluation',{}); body={k:v for k,v in data.items() if k!='evaluation'}
    when=parse_time(ev.get('reviewed_at')); due=parse_time(ev.get('due_at')); current=datetime.now(timezone.utc)
    return (body==prepare(root) and ev.get('content_sha256')==digest(body)
            and ev.get('state')=='approved_automated' and ev.get('actor_type')=='automated'
            and ev.get('policy_version')==POLICY and when and due and when<=current<due)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--evaluate',action='store_true')
    args=parser.parse_args()
    data=evaluate() if args.evaluate else load(ROOT/'resources/data/progressive_hints.json',{})
    print({'policy':POLICY,'organisms':len(data.get('species',{})),'valid':bool(valid(data))})
    raise SystemExit(not valid(data))
