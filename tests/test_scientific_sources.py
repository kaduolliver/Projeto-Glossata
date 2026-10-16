"""Offline adversarial tests. API-shaped payloads below are synthetic, not human reviews."""
import copy
import io
import json
from pathlib import Path
import sqlite3
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch
import urllib.error

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from scientific_sources.model import Assertion, Identity, DiscoveryScope, digest
from scientific_sources.adapters import GbifProvider,WormsProvider,ColProvider,CommonsProvider
from scientific_sources.registry import Registry,configured_registry
from scientific_sources.transport import CachedClient,CacheMiss,ProviderError,RateLimited
from scientific_sources.store import CandidateStore
from scientific_sources.workflow import gather,coverage
from scientific_sources.reconcile import reconcile
from scientific_sources.audit import staging_errors


DATASET='d7dddbf4-2cf0-4f39-9b2a-bb099caae36c'


class Response(io.BytesIO):
    status=200
    def __init__(self,value):super().__init__(json.dumps(value).encode())


def assertion(provider='sample',identifier='1',name='Example species',rank='species',parent='Animalia'):
    ident=Identity(provider,identifier)
    return Assertion(ident,name,name,rank,'accepted',accepted=ident,accepted_name=name,
        lineage=[{'name':parent,'rank':'kingdom','source_id':'ancestor'}],
        provenance=[{'retrieved_at':'2026-01-01T00:00:00+00:00','snapshot_sha256':'a'*64}],rights={'review_state':'pending'})


class ClientFixture:
    def __init__(self,responses):self.responses=responses;self.calls=[]
    def get(self,url,params=None,select=lambda d:d):
        self.calls.append((url,params))
        value=self.responses[url] if url in self.responses else self.responses['default']
        return select(copy.deepcopy(value)),{'url':url,'retrieved_at':'2026-01-01T00:00:00+00:00','snapshot_sha256':'a'*64,'response_sha256':'b'*64}


class ScientificSourceTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)

    def test_offline_default_never_requests_network_even_for_cache_miss(self):
        opener=Mock();client=CachedClient(self.root,opener=opener)
        with self.assertRaises(CacheMiss):client.get('https://api.gbif.org/v1/species/1')
        opener.assert_not_called()

    def test_cached_replay_keeps_collection_date_and_hash_without_network(self):
        opener=Mock(return_value=Response({'id':1,'coordinates':[1,2],'contact':'do not retain'}))
        client=CachedClient(self.root,online=True,opener=opener)
        value,evidence=client.get('https://api.gbif.org/v1/species/1',select=lambda d:{'id':d['id']})
        replay=CachedClient(self.root,opener=Mock(side_effect=AssertionError('network forbidden')))
        second,proof=replay.get('https://api.gbif.org/v1/species/1')
        self.assertEqual(second,{'id':1});self.assertEqual(evidence,proof)
        self.assertEqual(len(list(self.root.rglob('*[0-9a-f].json'))),1)
        self.assertNotIn('coordinates',''.join(p.read_text() for p in self.root.rglob('*.json')))

    def test_refresh_preserves_both_snapshots_and_updates_only_pointer(self):
        for value in [1,2]:
            client=CachedClient(self.root,online=True,refresh=True,opener=Mock(return_value=Response({'id':value})))
            client.get('https://api.gbif.org/v1/species/1')
        snapshots=[p for p in self.root.rglob('*.json') if p.name!='latest.json']
        self.assertEqual(len(snapshots),2)
        values=[json.loads(p.read_bytes())['payload']['id'] for p in snapshots]
        self.assertEqual(set(values),{1,2})

    def test_cache_tampering_and_endpoint_identity_are_rejected(self):
        client=CachedClient(self.root,online=True,opener=Mock(return_value=Response({'id':1})))
        client.get('https://api.gbif.org/v1/species/1')
        path=next(p for p in self.root.rglob('*.json') if p.name!='latest.json')
        data=json.loads(path.read_bytes());data['payload']['id']=2;path.write_text(json.dumps(data))
        with self.assertRaises(ProviderError):CachedClient(self.root).get('https://api.gbif.org/v1/species/1')

    def test_429_stops_host_without_retry_and_keeps_retry_after(self):
        error=urllib.error.HTTPError('https://api.gbif.org',429,'limited',{'Retry-After':'60'},None)
        opener=Mock(side_effect=error);client=CachedClient(self.root,online=True,opener=opener)
        with self.assertRaisesRegex(RateLimited,'60'):client.get('https://api.gbif.org/v1/species/1')
        with self.assertRaises(RateLimited):client.get('https://api.gbif.org/v1/species/2')
        self.assertEqual(opener.call_count,1)

    def test_budget_insecure_hosts_and_malformed_responses_fail_closed(self):
        opener=Mock(return_value=Response({'unexpected':[]}))
        client=CachedClient(self.root,online=True,max_requests=1,opener=opener)
        for url in ['http://api.gbif.org/v1/species/1','https://localhost/private','https://api.gbif.org@evil.test/data']:
            with self.assertRaises(ProviderError):client.get(url)
        with self.assertRaisesRegex(ProviderError,'schema'):
            client.get('https://api.gbif.org/v1/species/1',select=lambda d:d['results'])
        with self.assertRaisesRegex(ProviderError,'budget'):client.get('https://api.gbif.org/v1/species/2')
        self.assertFalse(list(self.root.rglob('latest.json')))

    def test_gbif_dataset_license_identity_and_exact_name_filter(self):
        row={'key':3,'datasetKey':DATASET,'canonicalName':'Example species','scientificName':'Example species Author',
             'rank':'SPECIES','taxonomicStatus':'ACCEPTED','nameType':'SCIENTIFIC','kingdom':'Animalia','kingdomKey':1}
        client=ClientFixture({'default':{'results':[row,dict(row,key=4,canonicalName='Example other')]},
            'https://api.gbif.org/v1/dataset/'+DATASET:{'key':DATASET,'license':'http://creativecommons.org/licenses/by/4.0/legalcode','citation':{'text':'Synthetic dataset citation'}}})
        rows=GbifProvider(client).search('Example species')
        self.assertEqual(len(rows),1);self.assertEqual(rows[0].rights['version'],'4.0')
        self.assertEqual(rows[0].identity,Identity('gbif','3',DATASET))
        with self.assertRaises(ProviderError):GbifProvider(client).normalize(dict(row,datasetKey='other'),{})

    def test_gbif_discovery_uses_verified_ancestor_not_a_text_hit(self):
        provider=GbifProvider(ClientFixture({}));provider.fetch=Mock(return_value=assertion('gbif','2','Archaea','kingdom','Archaea'))
        correct=assertion(name='Microbe species',parent='Archaea');correct.lineage[0]['source_id']='2'
        wrong=assertion(name='Fungus archaea',parent='Fungi')
        provider.query=Mock(return_value=[correct,wrong]);scope=DiscoveryScope('archaea','Archaea',('Archaea',),{'gbif':{'id':'2','name':'Archaea'}})
        self.assertEqual(provider.discover(scope),[correct])
        self.assertEqual(provider.query.call_args.args[0]['highertaxon_key'],'2')
        provider.fetch.return_value.canonical_name='Wrong root'
        with self.assertRaises(ProviderError):provider.discover(scope)

    def test_worms_explicit_synonym_keeps_both_names_citation_and_unknown_version(self):
        row={'AphiaID':11,'scientificname':'Old species','authority':'Original author','rank':'Species','status':'unaccepted',
             'valid_AphiaID':12,'valid_name':'New species','kingdom':'Animalia','url':'https://www.marinespecies.org/aphia.php?p=taxdetails&id=11',
             'citation':'Synthetic source citation','lsid':'urn:synthetic:11'}
        result=WormsProvider(ClientFixture({'default':[row]})).search('Old species')[0]
        self.assertEqual(result.accepted,Identity('worms','12'));self.assertEqual(result.canonical_name,'Old species')
        self.assertEqual(result.accepted_name,'New species');self.assertIsNone(result.rights['version'])
        self.assertEqual(result.references[0]['citation'],'Synthetic source citation')

    def test_col_pins_release_and_handles_search_wrapper_and_direct_usage(self):
        usage={'id':'ABC','datasetKey':316165,'status':'accepted','identifier':['inat:99'],'name':{'scientificName':'Example species','authorship':'Author','rank':'species'},'modified':'2026-01-01'}
        meta={'key':316165,'title':'Synthetic release','version':'synthetic','license':'cc by','citation':'Synthetic citation','doi':'synthetic'}
        base='https://api.checklistbank.org/dataset/316165'
        client=ClientFixture({base:meta,base+'/nameusage/ABC':usage,'default':{'result':[{'id':'ABC','usage':usage,'classification':[{'id':'K','name':'Animalia','rank':'kingdom'}]}]}})
        provider=ColProvider(client);search=provider.search('Example species')[0];fetched=provider.fetch('ABC')
        self.assertEqual(search.identity,fetched.identity);self.assertEqual(fetched.canonical_name,'Example species')
        self.assertEqual(search.rights['declared'],'cc by');self.assertIsNone(search.rights['version'])
        self.assertEqual(search.external_ids[1]['id'],'inat:99')
        self.assertNotIn('modifiedBy',ColProvider.selected({'usage':dict(usage,modifiedBy=42)}))

    def commons(self, license='CC0', license_url='http://creativecommons.org/publicdomain/zero/1.0/deed.en'):
        meta={k:{'value':v} for k,v in {'Artist':'<a>Source photographer</a>','ObjectName':'Actual source title','LicenseShortName':license,'LicenseUrl':license_url,'Restrictions':''}.items()}
        return {'query':{'pages':{'7':{'pageid':7,'lastrevid':8,'title':'File:Example.jpg','imageinfo':[{'url':'https://upload.wikimedia.org/Example.jpg','descriptionurl':'https://commons.wikimedia.org/wiki/File:Example.jpg','sha1':'c'*40,'width':1000,'height':800,'extmetadata':meta}]}}}}

    def test_commons_has_individual_license_and_no_observer_or_image_request(self):
        client=ClientFixture({'default':self.commons()});provider=CommonsProvider(client)
        rows=provider.fetch_media('File:Example.jpg');self.assertEqual(len(rows),1)
        self.assertEqual(rows[0].rights['author'],'Source photographer')
        self.assertEqual(rows[0].rights['version'],'1.0');self.assertEqual(rows[0].kind,'media')
        self.assertEqual(rows[0].canonical_name,'') # Search terms never become a scientific identity.
        self.assertEqual(len(client.calls),1);self.assertTrue(client.calls[0][0].startswith('https://commons.wikimedia.org/'))
        self.assertEqual(rows[0].status,'unverified_media_association')

    def test_commons_rejects_nc_sa_nd_absent_and_mismatched_permissions(self):
        for name in ['CC BY-SA 4.0','CC BY-NC 4.0','CC BY-ND 4.0','','CC BY 3.0']:
            provider=CommonsProvider(ClientFixture({'default':self.commons(name,'https://creativecommons.org/licenses/by/4.0/')}))
            self.assertEqual(provider.search_media('Example species'),[])
            self.assertEqual(len(provider.diagnostics),1)

    def test_reconciliation_follows_scoped_synonyms_without_overwriting_catalog(self):
        a=assertion('sample','1','Old species');a.accepted=Identity('sample','2');a.accepted_name='New species'
        b=assertion('sample','2','New species')
        catalog=[{'id':17,'name':'New species','source':'sample','source_taxon_id':'2','taxonomy':[]}]
        before=copy.deepcopy(catalog);report=reconcile([a,b],catalog,{})
        self.assertEqual(report['records'][a.identity.key]['relationship'],'known_external_id')
        self.assertTrue(report['records'][a.identity.key]['accepted_target_resolved'])
        self.assertEqual(catalog,before)

    def test_cross_provider_same_numeric_id_is_not_a_duplicate(self):
        a=assertion('other','17','Different species')
        catalog=[{'id':2,'name':'Original species','source':'inaturalist','source_taxon_id':'17','taxonomy':[]}]
        self.assertEqual(reconcile([a],catalog,{})['records'][a.identity.key]['relationship'],'unmatched')
        self.assertNotEqual(Identity('col','17','release-a'),Identity('col','17','release-b'))

    def test_homonyms_authorship_status_and_classification_divergences_remain_visible(self):
        a=assertion('a',parent='Animalia');b=assertion('b',parent='Plantae');b.authorship='Different author';a.authorship='Author'
        b.status='synonym'
        catalog=[{'id':1,'name':'Example species','source_taxonomy':[{'name':'Animalia','rank':'kingdom'}]}]
        report=reconcile([a,b],catalog,{})
        self.assertEqual(report['records'][b.identity.key]['relationship'],'ambiguous_homonym')
        self.assertEqual(set(report['comparisons'][0]['divergences']),{'kingdom','authorship','taxonomic_status'})
        b.lineage=[]
        self.assertEqual(reconcile([b],catalog,{})['records'][b.identity.key]['divergences'],[])

    def test_registry_extension_and_failure_isolation_without_core_changes(self):
        class Custom:
            name='new_source'
            def search(self,name,limit):return [assertion(self.name)]
            def fetch(self,identifier):return assertion(self.name,identifier)
        class Failing(Custom):
            name='unavailable'
            def search(self,name,limit):raise ProviderError('Service unavailable')
        registry=Registry();registry.register('new_source',Custom(),['taxonomy']);registry.register('unavailable',Failing(),['taxonomy'])
        rows,report=gather(registry,[],{},names=['Example species'])
        self.assertEqual(len(rows),1);self.assertEqual(report['failures'][0]['provider'],'unavailable')
        with self.assertRaises(ValueError):registry.with_capability('taxonomy',['not_integrated'])

    def test_synonym_target_id_mismatch_is_not_accepted(self):
        row=assertion('test','1');row.accepted=Identity('test','2')
        class Fake:
            name='test'
            def search(self,name,limit):return [row]
            def fetch(self,identifier):return assertion('test','wrong')
        registry=Registry();registry.register('test',Fake(),['taxonomy'])
        rows,report=gather(registry,[],{},names=['Example species'])
        self.assertEqual(len(rows),1);self.assertIn('mismatch',report['failures'][0]['error'])

    def test_discovery_prioritizes_missing_groups_and_reports_unsupported_route(self):
        scopes=[DiscoveryScope('animalia','Animalia',('Animalia',),{}),DiscoveryScope('archaea','Archaea',('Archaea',),{})]
        active=[{'id':1,'name':'Example species','taxonomy':['animal']}];nodes={'animal':{'name':'Animalia'}}
        self.assertEqual(coverage(scopes,active,nodes)[0]['scope'].id,'archaea')
        rows,report=gather(Registry(),active,nodes,scopes=scopes)
        self.assertIn('unavailable_reason',report['coverage'][0]);self.assertEqual(rows,[])

    def test_candidate_store_is_idempotent_versioned_and_always_pending(self):
        path=self.root/'candidates.sqlite';store=CandidateStore(path);self.addCleanup(store.close)
        row=assertion();self.assertEqual(store.put_many([row])['new'],1)
        self.assertEqual(store.put_many([row])['unchanged'],1);identifier=store.latest()[0]['candidate_id']
        row.canonical_name='Changed species';self.assertEqual(store.put_many([row])['changed'],1)
        self.assertEqual(store.db.execute('SELECT COUNT(*) FROM revisions').fetchone()[0],2)
        self.assertEqual(store.latest()[0]['candidate_id'],identifier)
        self.assertEqual(store.latest()[0]['scientific_state'],'pending');self.assertEqual(store.latest()[0]['legal_state'],'pending')
        with self.assertRaises(sqlite3.IntegrityError):store.db.execute("UPDATE candidates SET scientific_state='approved'")

    def test_store_reopens_and_batch_failure_rolls_back_without_losing_prior_revision(self):
        path=self.root/'candidates.sqlite';store=CandidateStore(path);store.put_many([assertion()]);store.close()
        store=CandidateStore(path);self.addCleanup(store.close);self.assertEqual(len(store.latest()),1)
        bad=assertion(identifier='3');bad.serialize=Mock(side_effect=ValueError('bad assertion'))
        with self.assertRaises(ValueError):store.put_many([assertion(identifier='2'),bad])
        self.assertEqual(len(store.latest()),1)

    def test_store_refuses_player_database_and_detects_corrupt_revision(self):
        path=self.root/'player.sqlite'
        with sqlite3.connect(path) as db:db.execute('CREATE TABLE game_runs(id INTEGER)');db.execute('INSERT INTO game_runs VALUES(123)')
        db.close()
        with self.assertRaises(ValueError):CandidateStore(path)
        with sqlite3.connect(path) as db:self.assertEqual(db.execute('SELECT name FROM sqlite_master WHERE type="table"').fetchall(),[('game_runs',)])
        db.close()
        store=CandidateStore(self.root/'candidate.sqlite');self.addCleanup(store.close);store.put_many([assertion()])
        store.db.execute("UPDATE revisions SET payload='{}'")
        with self.assertRaisesRegex(ValueError,'hash'):store.latest()

    def test_staging_guard_blocks_reports_and_candidate_ids_from_qrc_catalog(self):
        folder=self.root/'resources/data';folder.mkdir(parents=True)
        (folder/'report.json').write_text(json.dumps({'state':'candidates_only'}))
        (folder/'species.json').write_text(json.dumps([{'id':1,'candidate_id':'candidate:example'}]))
        (self.root/'resources.qrc').write_text('<RCC><qresource><file>resources/data/report.json</file><file>build/scientific-sources/cache/a.json</file></qresource></RCC>')
        self.assertEqual({e['error'] for e in staging_errors(self.root)},
                         {'candidate_report_bundled','candidate_artifact_bundled','candidate_in_active_catalog'})
        self.assertEqual(staging_errors(ROOT),[])

    def test_plan_has_eight_sources_four_real_adapters_and_explicit_future_limits(self):
        plan=json.loads((ROOT/'scripts/scientific_sources/provider_plan.json').read_bytes())
        self.assertEqual(set(plan['providers']),{'gbif','worms','col','commons','ncbi','lpsn','powo','mycobank'})
        registry=configured_registry(CachedClient(self.root))
        self.assertEqual(set(registry.entries),{'gbif','worms','col','commons'})
        self.assertTrue(all(p['terms'] for p in plan['providers'].values()))

    def test_legacy_helpers_respect_lpsn_no_automated_html_rule(self):
        import research_etymology,review_updates
        with patch('urllib.request.urlopen',side_effect=AssertionError('restricted HTML must not be requested')) as request:
            result=research_etymology.inspect('https://lpsn.dsmz.de/genus/example')
            self.assertFalse(result['retrieved']);self.assertIn('official_api',result['state'])
            with self.assertRaises(PermissionError):review_updates.PublicClient().get('https://www.lpsn.dsmz.de/genus/example')
            request.assert_not_called()


if __name__=='__main__':unittest.main()
