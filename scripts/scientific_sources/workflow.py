"""Use provider capabilities to stage proposals. Never imports into the game."""
from collections import Counter
from .model import DiscoveryScope, name_key
from .reconcile import reconcile
from .transport import ProviderError, now


def coverage(scopes, active, nodes):
    result=[]
    for scope in scopes:
        count=sum(any(nodes.get(t,{}).get('name') in scope.membership for t in s.get('taxonomy',[])) for s in active)
        result.append({'scope':scope,'count':count,'deficit':max(0,scope.target-count)})
    return sorted(result,key=lambda row:(row['count']/max(1,row['scope'].target),row['scope'].id))


def gather(registry, active, nodes, names=(), scopes=(), selected=None, limit=5, offset=0, with_media=False, media_ids=()):
    if not 1<=limit<=20 or offset<0:raise ValueError('Candidate query size out of bounds')
    assertions={};failures=[];scope_report=[]
    def attempt(provider, operation, callback):
        try:
            records=callback()
            for row in records:
                if row.identity.provider!=provider.name:raise ProviderError('Adapter identity does not match provider')
                assertions[row.identity.key]=row
            return records
        except (ProviderError,ValueError,KeyError,TypeError) as error:
            failures.append({'provider':provider.name,'operation':operation,'error':str(error)})
            return []
    taxonomy=registry.with_capability('taxonomy',selected)
    for name in names:
        for provider in taxonomy:attempt(provider,'search:'+name,lambda p=provider:p.search(name,limit))
    if scopes:
        for item in coverage(scopes,active,nodes):
            scope=item['scope'];entry={k:v for k,v in item.items() if k!='scope'};entry.update(scope=scope.id,label=scope.label,results=0)
            entry['providers_attempted']=[]
            for provider in registry.with_capability('discovery',selected):
                if provider.name not in scope.roots or entry['deficit']==0:continue
                entry['providers_attempted'].append(provider.name)
                records=attempt(provider,'discover:'+scope.id,lambda p=provider:p.discover(scope,limit,offset))
                entry['results']+=len(records)
            scope_report.append(entry)
            if not entry['providers_attempted']:entry['unavailable_reason']='No integrated discovery route or target already covered; not evidence of absence.'
    # Only explicit accepted-name links within a provider/release are followed.
    # The resolved target is stored separately; the original assertion survives.
    for row in list(assertions.values()):
        if row.accepted and row.accepted!=row.identity and row.accepted.key not in assertions:
            provider=next((p for p in taxonomy if p.name==row.identity.provider),None)
            if not provider:continue
            def resolve(p=provider,target=row.accepted):
                found=p.fetch(target.record_id)
                if found.identity!=target:raise ProviderError('Accepted-name identity mismatch')
                if found.status!='accepted':raise ProviderError('Accepted-name target is not accepted; chain requires review')
                return [found]
            attempt(provider,'accepted_target',resolve)
    if with_media:
        media_names=list(dict.fromkeys(list(names)+[r.canonical_name for r in assertions.values() if r.rank=='species' and r.status=='accepted']))[:limit]
        for name in media_names:
            for provider in registry.with_capability('media',selected):attempt(provider,'media:'+name,lambda p=provider:p.search_media(name,limit))
    for identifier in media_ids:
        source,separator,external=identifier.partition(':')
        if not separator or not external:raise ValueError('Expected media ID: provider:identifier')
        providers=registry.with_capability('media_lookup',[source])
        if not providers:raise ValueError('Provider has no explicit media lookup capability')
        for provider in providers:attempt(provider,'media_id:'+external,lambda p=provider:p.fetch_media(external))
    rows=list(assertions.values())
    return rows,{'schema_version':1,'generated_at':now(),'state':'candidates_only',
        'coverage':scope_report,'failures':failures,'providers':dict(Counter(r.identity.provider for r in rows)),
        'reconciliation':reconcile(rows,active,nodes),
        'notice':'No human reviews, permissions, photographs, game IDs or published classifications are created by this report.'}
