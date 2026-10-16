"""Photographic preparation only; source identification is never human approval."""
from collections import Counter
from datetime import datetime, timezone
import xml.etree.ElementTree as ET

from content_governance import load, save
from scientific_sources.adapters import CommonsProvider
from scientific_sources.transport import CachedClient, ProviderError


def discover(root, online=False, offset=0, limit=40, search_page=0, cc0=False):
    if offset<0 or not 1<=limit<=90 or search_page not in range(3):raise ValueError('Discovery requires a batch of 1..90, nonnegative offset and search page 0..2')
    client=CachedClient(root/'build/exploration-discovery/cache',online=online,max_requests=100)
    class EvidenceClient:
        evidence=None
        def get(self,base,params=None,select=lambda x:x):
            params=dict(params or {})
            if search_page:params['gsroffset']=8*search_page
            if cc0:params['gsrsearch']+=' haswbstatement:P275=Q6938433'
            payload,self.evidence=client.get(base,params,select)
            return payload,self.evidence
    recorder=EvidenceClient();provider=CommonsProvider(recorder)
    species=load(root/'resources/data/species.json')
    subjects=[{'name':s['name'],'catalog_species_id':s['id']} for s in species]
    subjects.append({'name':'Euglena gracilis','scope':['clade:Discoba']})
    subjects.append({'name':'Trypanosoma brucei','scope':['clade:Discoba']})
    destination=root/'build/exploration-discovery/results.json'
    report=load(destination,{'schema_version':1,'subjects':{}})
    for subject in subjects[offset:offset+limit]:
        name=subject['name'];start=len(provider.diagnostics)
        if (search_page or cc0) and any(p['subject']==name for p in load(root/'resources/data/exploration_photos.json')['photos'].values()):continue
        recorder.evidence=None
        try:
            found=provider.search_media(name,limit=8)
            candidates=[r.serialize() for r in found]
            error=None
        except ProviderError as exception:candidates=[];error=str(exception)
        prior=report['subjects'].get(name,{})
        merged={c['identity']['record_id']:c for c in prior.get('candidates',[])+candidates}
        queries=prior.get('queries',[])+[{'page':search_page,'cc0_search':cc0,'evidence':recorder.evidence,'error':error,'returned_eligible':len(candidates)}]
        unique={}
        for query in queries:
            ev=query.get('evidence') or {}
            unique[(query['page'],query.get('cc0_search',False),ev.get('snapshot_sha256'),query.get('error'))]=query
        queries=list(unique.values())
        report['subjects'][name]=dict(subject,candidates=list(merged.values()),queries=queries,
            rejected=prior.get('rejected',[])+provider.diagnostics[start:],error=error,
            searched_at=datetime.now(timezone.utc).isoformat(),
            notice='Search proposals only; file, source identification and actual photograph need inspection. No human review declared.')
        save(destination,report)
        print(name+': '+str(len(candidates))+' permitted metadata candidates'+(' / '+error if error else ''),flush=True)
    print('Network requests: '+str(client.requests),flush=True)
    return report


def complete_coverage(root):
    """Write each binding explicitly from a compatible descendant, never a runtime ancestor fallback."""
    from exploration_media import MANIFEST
    data=load(root/MANIFEST);nodes=load(root/'resources/data/taxonomy.json')
    species=load(root/'resources/data/species.json');by_id={s['id']:s for s in species}
    discovery=load(root/'build/exploration-discovery/results.json',{'subjects':{}})['subjects']
    bindings=data['bindings'];pending={};uses=Counter(bindings.values())
    for node in nodes:
        taxon=node['id']
        if taxon in bindings:continue
        choices=[]
        for key,photo in data['photos'].items():
            organism=by_id.get(photo.get('catalog_species_id'))
            if organism and photo['subject']==organism['name'] and taxon in organism['taxonomy']:
                choices.append(key)
            elif not organism and taxon in photo.get('scope',{}).get('taxon_ids',[]):choices.append(key)
        if choices:
            chosen=min(choices,key=lambda key:(uses[key],key));bindings[taxon]=chosen;uses[chosen]+=1
            continue
        names=[s['name'] for s in species if taxon in s['taxonomy']]
        if taxon=='clade:Discoba':names=['Euglena gracilis','Trypanosoma brucei']
        attempts=[discovery.get(name,{}) for name in names]
        if not discovery and taxon in data.get('pending',{}):
            pending[taxon]=data['pending'][taxon]
            continue # A clean checkout must not erase prior queries/evidence or rewrite their explanation.
        if not names or any(not a for a in attempts):reason='Descoberta ainda pendente para representantes deste grupo; nenhuma fotografia correspondente foi preparada.'
        elif any(a.get('error') for a in attempts):reason='Consulta incompleta ou indisponível; resultados insuficientes para preparar fotografia com identidade e permissão verificáveis.'
        elif not any(a.get('candidates') for a in attempts):reason='Nenhum arquivo elegível nas páginas de oito resultados consultadas por representante: licença fora da política, crédito/resolução insuficientes ou ausência de resultados. Não demonstra inexistência de fotografia.'
        else:reason='Resultados de busca existentes, mas sem fotografia preparada com identificação e enquadramento visual adequados. Não se usa um resultado ou uma ilustração como prova de identidade.'
        notes=[data.get('selection_notes',{}).get(name) for name in names]
        if any(notes):reason=' '.join(note for note in notes if note)
        pending[taxon]={'reason':reason,'attempted_subjects':names or [node['name']],
            'queries':[q for a in attempts for q in a.get('queries',[])],
            'state':'awaiting_suitable_photograph','responsibility':'Curadoria do projeto; revisão humana não declarada.'}
    data['schema_version']=2;data['pending']=pending
    save(root/MANIFEST,data)
    return data


def sync_resources(root):
    """Inventory only files whose individual public license is evidenced; never clear legacy blockers."""
    from exploration_media import MANIFEST, photo_errors
    data=load(root/MANIFEST)
    inventory=load(root/'resources/compliance/asset_rights.json')
    assets={a['path']:a for a in inventory['assets']}
    qrc=root/'resources.qrc';text=qrc.read_text(encoding='utf8')
    paths={e.text for e in ET.fromstring(text).iter('file')}
    additions=[]
    for photo in data['photos'].values():
        if photo_errors(photo,root):raise ValueError('Cannot inventory invalid photograph: '+photo['subject'])
        assets[photo['path']]={'path':photo['path'],'kind':'exploration_photograph','sha256':photo['sha256'],
            'source':photo['photo_url'],'license':photo['license'],'license_version':photo['license_version'],
            'license_url':photo['license_url'],'permission':{'holder':photo['author'],
            'holder_basis':'Crédito indicado pela fonte; não inferido do carregador/observador.',
            'evidence_path':photo['evidence_path'],'evidence_sha256':photo['evidence_sha256'],
            'distribution_allowed':True,'basis':'Licença pública CC0/CC BY por arquivo evidenciada na coleta; sem revisão humana presumida.'},
            'state':'source_license_evidenced'}
        for path in (photo['path'],photo['evidence_path']):
            if path not in paths:additions.append('        <file>'+path+'</file>');paths.add(path)
    inventory['assets']=list(assets.values());save(root/'resources/compliance/asset_rights.json',inventory)
    if additions:qrc.write_text(text.replace('    </qresource>','\n'.join(additions)+'\n    </qresource>'),encoding='utf8',newline='\n')
