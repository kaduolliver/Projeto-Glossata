"""Conservative identity proposals. No majority vote or universal backbone is inferred."""
from collections import defaultdict
from itertools import combinations
from .model import Identity, name_key


def external_identity(value):
    provider=value.get('provider');identifier=value.get('id')
    if not identifier:return None
    if provider=='source_identifier':
        prefix,separator,identifier=str(identifier).partition(':')
        provider={'inat':'inaturalist','gbif':'gbif','worms':'worms','ncbitaxon':'ncbi'}.get(prefix)
        if not separator or not provider:return None
    if provider in {'source_taxonID','lsid',None}:return None
    return Identity(provider,str(identifier),str(value.get('dataset') or ''))


def context(record):
    return {n['rank']:name_key(n['name']) for n in record.lineage if n.get('rank') and n.get('name') and n['rank']!='species'}


def reconcile(assertions, active, taxons):
    names=defaultdict(list);identities=defaultdict(list)
    for organism in active:
        names[name_key(organism['name'])].append(organism)
        if organism.get('source') and organism.get('source_taxon_id'):
            identities[Identity(organism['source'],str(organism['source_taxon_id'])).key].append(organism)
        for external in organism.get('external_ids',[]):
            identity=external_identity(external)
            if identity:identities[identity.key].append(organism)
    by_key={r.identity.key:r for r in assertions if r.kind=='taxon'}
    reports={};pairs=[]
    for row in assertions:
        if row.kind!='taxon':continue
        target=by_key.get(row.accepted.key) if row.accepted else None
        effective=target or row
        ids=[row.identity]+([row.accepted] if row.accepted else [])
        ids += [value for ext in row.external_ids if (value:=external_identity(ext))]
        exact={s['id']:s for i in ids for s in identities.get(i.key,[])}
        similar=names.get(name_key(effective.canonical_name),[])
        possible=exact or {s['id']:s for s in similar}
        divergences=[]
        for existing in possible.values():
            source_context={n['rank']:name_key(n['name']) for n in existing.get('source_taxonomy',[])}
            if not source_context:
                source_context={taxons[t]['rank']:name_key(taxons[t]['name']) for t in existing.get('taxonomy',[]) if t in taxons}
            current=context(effective)
            for rank in set(source_context)&set(current)-{'species'}:
                if source_context[rank]!=current[rank]:divergences.append({'kind':'classification','rank':rank,'catalog':source_context[rank],'provider':current[rank],'species_id':existing['id']})
            if exact and name_key(existing['name'])!=name_key(effective.canonical_name):
                divergences.append({'kind':'name_or_synonym','catalog':existing['name'],'provider':effective.canonical_name,'species_id':existing['id']})
        relation='known_external_id' if exact else 'possible_existing_name' if similar else 'unmatched'
        if not exact and any(d.get('rank') in {'kingdom','domain','superkingdom'} for d in divergences):relation='ambiguous_homonym'
        reports[row.identity.key]={'relationship':relation,'catalog_ids':sorted(possible), 'divergences':divergences,
            'accepted_target_resolved':bool(target),'accepted_target':row.accepted.key if row.accepted else None,
            'notice':'Proposed relationship only. Same spelling, absent ranks and API acceptance do not establish taxon identity or scientific review.'}
    groups=defaultdict(list);compared=set()
    for row in by_key.values():
        groups[('name',name_key(row.canonical_name),row.rank)].append(row)
        if row.accepted:groups[('accepted',row.accepted.key)].append(row)
    for group in groups.values():
        for a,b in combinations(group,2):
            pair=tuple(sorted((a.identity.key,b.identity.key)))
            if pair in compared:continue
            compared.add(pair)
            shared=(a.accepted is not None and b.accepted is not None and a.accepted==b.accepted)
            same_name=name_key(a.canonical_name)==name_key(b.canonical_name) and a.rank==b.rank
            if not shared and not same_name:continue
            conflicts=[rank for rank in context(a).keys()&context(b).keys() if context(a)[rank]!=context(b)[rank]]
            if a.authorship and b.authorship and name_key(a.authorship)!=name_key(b.authorship):conflicts.append('authorship')
            if a.status!=b.status:conflicts.append('taxonomic_status')
            pairs.append({'a':a.identity.key,'b':b.identity.key,'relationship':'same_provider_accepted_id' if shared else 'possible_same_name',
                          'divergences':sorted(conflicts),'independence':'Not established; aggregators may share upstream datasets.'})
    return {'records':reports,'comparisons':pairs}
