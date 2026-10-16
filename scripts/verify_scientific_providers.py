"""Small reproducible API contract checks; offline cache by default, no publication."""
import argparse
from pathlib import Path
from scientific_sources.adapters import GbifProvider, WormsProvider, ColProvider, CommonsProvider
from scientific_sources.model import DiscoveryScope, digest
from scientific_sources.transport import CachedClient, ProviderError, now, write

ROOT=Path(__file__).resolve().parents[1]


def verify(online=False, refresh=False):
    client=CachedClient(ROOT/'build/scientific-sources/cache',online=online,refresh=refresh,max_requests=20)
    gbif=GbifProvider(client);worms=WormsProvider(client);col=ColProvider(client);commons=CommonsProvider(client)
    report={'schema_version':1,'checked_at':now(),'mode':'online_allowed' if online else 'offline_cache',
            'scope':'API contract/evidence checks only. No human scientific/legal review or publication.', 'checks':[]}
    def check(name,query,operation,condition):
        try:
            rows=operation()
            if not rows or not condition(rows):raise ProviderError('Expected API identity/relationship not established; review source changes')
            report['checks'].append({'check':name,'query':query,'passed':True,'record_count':len(rows),
                'identities':[r.serialize()['identity'] for r in rows],
                'assertion_sha256':[digest(r.serialize()) for r in rows],
                'evidence':[ev for r in rows for ev in r.provenance],
                'rights_evidence':[r.rights.get('evidence') for r in rows if r.rights.get('evidence')]})
        except (ProviderError,ValueError,KeyError,TypeError) as error:
            report['checks'].append({'check':name,'query':query,'passed':False,'error':str(error)})
    check('gbif_exact_name','Panthera leo',lambda:gbif.search('Panthera leo',2),lambda rows:all(r.canonical_name=='Panthera leo' for r in rows))
    for sid,name,identifier in [('archaea','Archaea','2'),('mollusca','Mollusca','52')]:
        scope=DiscoveryScope(sid,name,(name,),{'gbif':{'id':identifier,'name':name}})
        check('gbif_descendants_'+sid,name,lambda s=scope:gbif.discover(s,3),
              lambda rows,n=name:all(any(t['name']==n for t in r.lineage) for r in rows))
    check('worms_synonym','Macrocystis angustifolia; AphiaID 214347',lambda:[worms.fetch('214347')],
          lambda rows:rows[0].accepted is not None and rows[0].accepted.record_id=='232231')
    check('worms_accepted_target','AphiaID 232231',lambda:[worms.fetch('232231')],
          lambda rows:rows[0].status=='accepted' and rows[0].canonical_name=='Macrocystis pyrifera')
    check('col_exact_name','Panthera leo; release 316165',lambda:col.search('Panthera leo',2),lambda rows:all(r.canonical_name=='Panthera leo' for r in rows))
    check('col_direct_record','Q7GY3; release 316165',lambda:[col.fetch('Q7GY3')],lambda rows:rows[0].canonical_name=='Macrocystis pyrifera')
    check('commons_individual_permission','File:Danaus plexippus 100029190.jpg',
          lambda:commons.fetch_media('File:Danaus plexippus 100029190.jpg'),
          lambda rows:rows[0].identity.record_id=='126550368' and rows[0].rights['license']=='cc0' and rows[0].rights['version']=='1.0')
    report['network_requests']=client.requests;report['passed']=all(c['passed'] for c in report['checks'])
    report['image_downloads']=0;report['catalog_writes']=0
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--online',action='store_true');parser.add_argument('--refresh',action='store_true')
    args=parser.parse_args()
    if args.refresh and not args.online:parser.error('--refresh requires --online')
    report=verify(args.online,args.refresh)
    path=ROOT/'build/scientific-sources/provider-verification.json';write(path,report)
    print('Verificações:',sum(c['passed'] for c in report['checks']),'/',len(report['checks']),'; requisições:',report['network_requests'])
    print(path)
    raise SystemExit(0 if report['passed'] else 1)
