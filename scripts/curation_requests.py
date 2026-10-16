"""Local curation queue assembled from existing audits; never grants approval.

Only the explicit public DTO is eligible for GitHub. Raw audit reasons, observer
records, issue text and permission documents are never copied into the queue.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
from urllib.parse import quote

import content_governance as governance

ROOT = governance.ROOT
CONFIG = Path(__file__).with_name('curation_policy.json')
AREAS = {'taxonomy', 'photography', 'science', 'pedagogy', 'licensing', 'integrity'}
PUBLIC_FILES = {
    'resources/data/species.json', 'resources/data/taxonomy.json', 'resources/data/education.json',
    'resources/data/phylogeny.json', 'resources/data/photo_provenance.json', 'resources/data/credits.json',
    'resources/data/hint_education.json', 'resources/data/progressive_hints.json',
    'resources/data/catalog_manifest.json', 'resources/compliance/asset_rights.json',
    'resources/compliance/review_baseline.json', 'resources/compliance/review_history.jsonl',
    'resources/compliance/permission_history.jsonl', 'resources/compliance/dependency_reviews.jsonl',
    'docs/AUDITORIA_PROGRESSAO_DICAS.json', 'docs/AUDITORIA_FOTOGRAFIAS.json',
}
PRIVATE_KEYS = {'latitude', 'longitude', 'coordinates', 'private_location', 'private_geojson',
                'email', 'observer_email', 'phone', 'token', 'password', 'client_secret'}


def settings(path=CONFIG):
    value = governance.load(path)
    if value.get('schema_version') != 1 or not re.fullmatch(r'[\w.-]+/[\w.-]+', value['repository']):
        raise ValueError('Invalid curation configuration')
    for name, maximum in [('max_creates', 10), ('max_updates', 30), ('max_api_calls', 500),
                          ('max_pages', 100), ('max_body_rows', 50), ('max_project_updates', 30)]:
        if not isinstance(value[name], int) or not 1 <= value[name] <= maximum:
            raise ValueError('Invalid curation limit: '+name)
    return value


def request_id(area, bucket):
    return 'curation-v1-'+governance.digest([area, bucket])[:24]


def private_data(value):
    if isinstance(value, dict):
        return bool(PRIVATE_KEYS.intersection(k.lower() for k in value)) or any(private_data(v) for v in value.values())
    if isinstance(value, list):
        return any(private_data(v) for v in value)
    if isinstance(value, str):
        from audit_repository import SECRET_PATTERNS
        return bool(re.search(r'[\w.+-]+@[\w.-]+\.[a-zA-Z]{2,}', value)) or any(p.search(value.encode()) for p in SECRET_PATTERNS.values())
    return False


def public_reference(root, name):
    """Strict public paths; a confidential filename is not even returned."""
    if not isinstance(name, str) or (name not in PUBLIC_FILES and not re.fullmatch(
            r'resources/compliance/(permissions/[0-9a-f]{64}|exploration/[a-z0-9-]+)\.json', name)):
        return None
    try:
        path = governance.local_file(root, name)
        if not path.is_file(): return None
        raw = path.read_bytes()
        objects = [json.loads(line) for line in raw.decode('utf8').splitlines() if line.strip()] if name.endswith('.jsonl') else json.loads(raw)
        if private_data(objects): return None
        return {'path': name, 'sha256': hashlib.sha256(raw).hexdigest()}
    except (ValueError, UnicodeError, OSError):
        return None


def collect(root=ROOT, config=None):
    from exploration_media import audit as media_audit
    from audit_hint_assistance import audit as assistance_audit
    config = config or settings()
    audit = governance.audit_governance(root)
    media = media_audit(root)
    assistance = assistance_audit(root)
    return build_queue(root, audit, media, assistance, config)


def build_queue(root, audit, media, assistance, config):
    current = governance.records(root)
    groups = {}
    reference_cache = {}

    def add(area, bucket, code, key, priority=2, state='pending', content_hash='', paths=()):
        # No free text from scientific APIs, human evidence, or GitHub enters this DTO.
        if area not in AREAS: raise ValueError('Unknown curation area')
        code = code if re.fullmatch(r'[a-z][a-z0-9_]{0,90}', str(code)) else 'audit_finding'
        identity = str(key)
        public_key = identity if re.fullmatch(r'(?:topic:\d+:[a-z_]+|species:\d+:[a-z_:]+|assistance:[a-z_:0-9]+|photo:\d+|organism:\d+(?::[a-z]+)?|note:\d+:(?:intro|hint|curiosity)|taxon-id:(?:taxon:[a-f0-9]+|domain:(?:Bacteria|Archaea|Eukaryota)|clade:[A-Za-z]+)|node:[a-f0-9]{24}|asset:[a-f0-9]{24})', identity) else 'resource:'+governance.digest(identity)[:24]
        rid = request_id(area, bucket)
        item = groups.setdefault(rid, {'id': rid, 'area': area, 'bucket': bucket, 'priority': priority, 'findings': {}})
        item['priority'] = min(priority, item['priority'])
        fid = governance.digest([area, bucket, identity, code])[:32]
        references = []
        for name in sorted(set(paths)):
            if name not in reference_cache: reference_cache[name] = public_reference(root, name)
            if reference_cache[name]: references.append(reference_cache[name])
        item['findings'][fid] = {'id': fid, 'key': public_key, 'code': code,
            'state': state if state in {'pending','legacy','draft','needs_review','expired','rejected','in_review','source_review_required'} else 'pending',
            'content_sha256': content_hash if re.fullmatch('[a-f0-9]{64}', str(content_hash)) else '',
            'evidence': references, 'withheld_references': len(set(paths))-len(references)}

    for f in audit.get('technical_errors', []):
        add('integrity', 'audit-integrity', f.get('error','audit_finding'), governance.digest(f), 0,
            paths=('resources/data/catalog_manifest.json',))
    for f in audit.get('scientific_findings', []):
        key = f.get('key',''); record = current.get(key, {})
        topic = record.get('topic','photo')
        area = 'taxonomy' if topic in {'taxonomy','phylogeny'} else 'photography' if topic=='photo' else 'science'
        add(area, topic, 'scientific_review_required', key, 1 if f.get('severity')=='high' else 2,
            f.get('state','pending'), f.get('content_sha256',''),
            ('resources/data/education.json', 'resources/data/species.json', 'resources/compliance/review_history.jsonl'))
    for f in audit.get('photo_licensing', []):
        for code in f.get('errors', []):
            add('licensing', 'quiz-photos', code, 'photo:'+str(f.get('photo_id')), 0 if code=='license_outside_policy' else 1,
                paths=('resources/data/photo_provenance.json','resources/data/credits.json'))
    for f in audit.get('asset_licensing', []):
        add('licensing', 'other-materials', f.get('error','audit_finding'), 'asset:'+governance.digest(f.get('path',''))[:24], 1,
            paths=('resources/compliance/asset_rights.json',))
    for f in audit.get('unbundled_media', []):
        add('licensing', 'historical-media', 'unbundled_permission_not_demonstrated', 'asset:'+governance.digest(f.get('path',''))[:24], 1,
            content_hash=f.get('sha256',''), paths=('resources/compliance/asset_rights.json',))
    for f in audit.get('assistance_findings', []):
        # Legacy pedagogical drafts are not a failure of the current automated policy.
        add('pedagogy', 'legacy-assistance', 'legacy_assistance_review', f.get('key',''), 3,
            f.get('state','pending'), f.get('content_sha256',''), ('resources/data/hint_education.json',))
    if not audit.get('automated_pedagogy',{}).get('usable'):
        add('pedagogy', 'current-assistance', 'automated_assessment_unavailable', 'automatic', 1,
            paths=('resources/data/progressive_hints.json',))
    for f in assistance.get('missing_content', []):
        add('pedagogy', 'content-gaps', 'source_content_missing', 'organism:'+str(f.get('species_id')), 2,
            paths=('resources/data/hint_education.json',))
    coverage = governance.load(root/'docs/AUDITORIA_PROGRESSAO_DICAS.json', {})
    for row in coverage.get('rows', []):
        if row.get('available',3) < 3:
            add('pedagogy','content-gaps','progression_incomplete','organism:'+str(row.get('id'))+':'+str(row.get('rank')),2,
                paths=('docs/AUDITORIA_PROGRESSAO_DICAS.json',))
    for f in media.get('coverage', []):
        if f.get('state') != 'photograph':
            add('photography', 'presentation-coverage', 'suitable_photo_pending', 'taxon-id:'+str(f.get('id')),3,
                paths=('docs/AUDITORIA_FOTOGRAFIAS.json',))
    presentation = governance.load(root/'resources/data/exploration_photos.json',{}).get('photos',{})
    for pid, photo in presentation.items():
        review = photo.get('scientific_review')
        if isinstance(review,str) or not isinstance(review,dict) or review.get('state') not in governance.APPROVALS:
            add('photography','presentation-identification','representative_review_required','node:'+governance.digest(pid)[:24],2,
                paths=(photo.get('evidence_path',''),'docs/AUDITORIA_FOTOGRAFIAS.json'))
    # Weekly CI cannot infer the actual Windows bundle from a Linux checkout.
    # A source-level request keeps the existing dependency gate visible without
    # publishing machine-specific package inventories or asserting their contents.
    add('licensing','dependencies','distribution_dependency_review_required','dependencies',2,
        paths=('resources/compliance/dependency_reviews.jsonl',))
    requests=[]
    for item in groups.values():
        item['findings'] = sorted(item['findings'].values(),key=lambda f:f['id'])
        item['fingerprint'] = governance.digest(item)
        requests.append(item)
    return {'schema_version':1,'repository':config['repository'], 'generated_at':governance.now(),
        'catalog': governance.load(root/'resources/data/catalog_manifest.json',{}).get('version'),
        'requests':sorted(requests,key=lambda r:(r['priority'],r['id'])),
        'summary':{'requests':len(requests),'findings':sum(len(r['findings']) for r in requests),
                   'by_area':dict(Counter(r['area'] for r in requests))},
        'notice':'Public, minimized requests only. No decision, consent, review date or permission is created.'}


def render(request, repository, revision, maximum=20):
    if not re.fullmatch(r'[0-9a-f]{40}', revision): raise ValueError('Expected source commit SHA')
    rid=request['id']; start='<!-- glossata-curation:start -->'; end='<!-- glossata-curation:end -->'
    lines=[f'<!-- glossata-curation:v1:{rid} -->',start,
        '<!-- glossata-fingerprint:'+request['fingerprint']+' -->',
        f"## Solicitação automática: {request['area']} / {request['bucket']}",
        f"Prioridade **P{request['priority']}** · **{len(request['findings'])} achados** · ID `{rid}`.",
        'Triagem automática. Não representa aprovação científica, pedagógica humana ou autorização de uso.',
        'Fechar esta Issue ou alterar etiquetas não modifica a governança nem libera recursos.',
        '| Registro público | Código | Estado | Hash do conteúdo |','| --- | --- | --- | --- |']
    refs={}
    for f in request['findings']:
        refs.update({r['path']:r for r in f['evidence']})
    for f in request['findings'][:maximum]:
        lines.append(f"| `{f['key']}` | `{f['code']}` | `{f['state']}` | `{f['content_sha256'] or 'não fornecido'}` |")
    if len(request['findings'])>maximum:lines.append(f"Lista resumida a {maximum} linhas; fila completa no artefato público da execução da curadoria.")
    lines += ['### Evidências públicas selecionadas']
    for path,ref in sorted(refs.items())[:12]:
        url=f'https://github.com/{repository}/blob/{revision}/{quote(path,safe="/")}'
        lines.append(f"- [{path}]({url}) · SHA-256 `{ref['sha256']}`")
    if any(f['withheld_references'] for f in request['findings']):
        lines.append('Algumas referências não foram publicadas: caminho não autorizado, ausência ou necessidade de revisão de privacidade. Não anexar documentação confidencial aqui.')
    lines += ['### Encaminhamento humano',
        '- Conferir fonte, identidade, escopo, validade e evidências; registrar a decisão pelo fluxo de governança existente.',
        '- Para autorização: preparar o pedido individual e registrar o acompanhamento; nenhum contato foi enviado.',
        '- Manter documentos privados fora do GitHub/artefatos públicos. Não incluir coordenadas sensíveis ou dados pessoais.',end,
        '', '## Acompanhamento do mantenedor', '', 'Espaço preservado pela automação.']
    return '\n\n'.join(lines)


def write_queue(queue, output):
    governance.save(output,queue)
    return queue


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['plan'])
    parser.add_argument('--output',type=Path,default=ROOT/'build/curation/requests.json')
    args=parser.parse_args()
    queue=write_queue(collect(ROOT,settings()),args.output)
    print(json.dumps(queue['summary'],ensure_ascii=False))
    return 0


if __name__=='__main__':raise SystemExit(main())
