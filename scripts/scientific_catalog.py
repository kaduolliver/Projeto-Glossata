"""Stage scientific candidates from independent provider adapters; offline by default.

No command in this tool publishes an organism, grants permission or approves a review.
"""
import argparse
import json
from pathlib import Path
from scientific_sources.model import Assertion, DiscoveryScope
from scientific_sources.reconcile import reconcile
from scientific_sources.registry import configured_registry
from scientific_sources.store import CandidateStore
from scientific_sources.transport import CachedClient, write
from scientific_sources.workflow import gather, coverage

ROOT=Path(__file__).resolve().parents[1]
PLAN=Path(__file__).with_name('scientific_sources')/'provider_plan.json'


def run(args):
    if args.command.startswith('admission-'):
        from scientific_sources import admission
        if not args.draft: raise ValueError('--draft is required')
        draft = admission.workspace(ROOT, args.draft)
        if args.command == 'admission-template':
            if not args.candidate_id: raise ValueError('--candidate-id is required')
            path = ROOT / 'build/scientific-sources/candidates.sqlite'
            if not path.is_file(): raise ValueError('Discover/reconcile candidates first')
            store = CandidateStore(path)
            try: rows = [r for r in store.latest() if r['candidate_id'] == args.candidate_id]
            finally: store.close()
            if len(rows) != 1: raise ValueError('Candidate not found')
            print(admission.template(ROOT, draft, rows[0])); return 0
        if not args.species_id: raise ValueError('--species-id is required')
        if args.command == 'admission-prepare': result = admission.prepare(ROOT, draft, args.species_id)
        else:
            result = admission.assess(draft, args.species_id, json.loads((ROOT/'resources/data/species.json').read_bytes()),
                                     {n['id']: n for n in json.loads((ROOT/'resources/data/taxonomy.json').read_bytes())})
        print(json.dumps(result, ensure_ascii=False, indent=2)); return 0 if result['ready_for_proposal'] else 2
    plan=json.loads(PLAN.read_bytes())
    active=json.loads((ROOT/'resources/data/species.json').read_bytes())
    nodes={n['id']:n for n in json.loads((ROOT/'resources/data/taxonomy.json').read_bytes())}
    scopes=[DiscoveryScope(s['id'],s['label'],tuple(s['membership']),s['roots'],s['target']) for s in plan['discovery_scopes']]
    if args.scope:
        unknown=set(args.scope)-{s.id for s in scopes}
        if unknown:raise ValueError('Unknown discovery scope: '+', '.join(sorted(unknown)))
        scopes=[s for s in scopes if s.id in args.scope]
    if args.command=='providers':
        print(json.dumps(plan,ensure_ascii=False,indent=2));return 0
    if args.command=='coverage':
        print(json.dumps([{**{k:v for k,v in row.items() if k!='scope'},'scope':row['scope'].id,'label':row['scope'].label}
                          for row in coverage(scopes,active,nodes)],ensure_ascii=False,indent=2));return 0
    work=ROOT/'build/scientific-sources'
    client=CachedClient(work/'cache',online=args.online,refresh=args.refresh,max_requests=args.max_requests)
    registry=configured_registry(client)
    selected=args.providers.split(',') if args.providers else None
    names=args.name or []
    if args.command=='reconcile' and not names:raise ValueError('Supply --name; quoted canonical scientific name required')
    if args.command=='discover':names=[]
    rows,report=gather(registry,active,nodes,names,scopes if args.command=='discover' else [],
        selected,args.limit,args.offset,args.with_media,args.media_id or [])
    store=CandidateStore(work/'candidates.sqlite')
    try:
        report['storage']=store.put_many(rows)
        report['candidates']=store.latest()
        report['reconciliation']=reconcile([Assertion.from_dict(r['assertion']) for r in report['candidates']],active,nodes)
        report['provider_diagnostics']={name:p.diagnostics for name,(p,caps) in registry.entries.items() if getattr(p,'diagnostics',None)}
    finally:store.close()
    report['network_requests']=client.requests
    report['mode']='online' if args.online else 'offline_replay'
    # One immutable report per content hash; last.json is only a convenience.
    from scientific_sources.model import digest
    write(work/'reports'/(digest(report)+'.json'),report);write(work/'last.json',report)
    print(json.dumps({k:v for k,v in report.items() if k not in {'candidates','reconciliation'}},ensure_ascii=False,indent=2))
    print('Candidatos e evidências: '+str(work)+'. Nenhum recurso do jogo foi alterado.')
    return 1 if report['failures'] else 0


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command',choices=['providers','coverage','reconcile','discover',
                                         'admission-template','admission-check','admission-prepare'])
    parser.add_argument('--draft',type=Path,help='Dedicated workspace under build/scientific-sources/drafts')
    parser.add_argument('--candidate-id');parser.add_argument('--species-id')
    parser.add_argument('--name',action='append');parser.add_argument('--scope',action='append')
    parser.add_argument('--providers',help='Comma-separated integrated provider IDs')
    parser.add_argument('--online',action='store_true',help='Explicitly allow bounded API requests')
    parser.add_argument('--refresh',action='store_true',help='Archive new evidence; never renew review dates')
    parser.add_argument('--with-media',action='store_true',help='Search Commons metadata only; no image downloads')
    parser.add_argument('--media-id',action='append',help='Explicit metadata lookup, e.g. commons:File:Example.jpg')
    parser.add_argument('--limit',type=int,default=5);parser.add_argument('--offset',type=int,default=0)
    parser.add_argument('--max-requests',type=int,default=40)
    args=parser.parse_args()
    if args.refresh and not args.online:parser.error('--refresh requires --online')
    try:raise SystemExit(run(args))
    except (ValueError,OSError) as error:parser.exit(1,str(error)+'\n')
