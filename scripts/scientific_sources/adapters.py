"""API-specific normalization. None of these adapters writes to resources/."""
import re
from urllib.parse import quote, urlparse
from .model import Assertion, Identity, name_key
from .transport import ProviderError

RANKS=('domain','superkingdom','kingdom','phylum','class','order','family','genus','species')


def pick(data, fields): return {k:data[k] for k in fields if k in data}


def positive_id(value):
    text=str(value)
    if not re.fullmatch(r'[1-9][0-9]*',text):raise ProviderError('Invalid numeric source identifier')
    return text


def lineage(data, id_suffix='Key'):
    return [dict(name=data[rank],rank=rank,source_id=str(data.get(rank+id_suffix,''))) for rank in RANKS if data.get(rank)]


def dataset_rights(metadata, evidence):
    declared=metadata.get('license','');url=urlparse(declared)
    match=re.fullmatch(r'/licenses/by/(1\.0|2\.0|2\.5|3\.0|4\.0)(?:/legalcode)?/?',url.path)
    valid=url.hostname=='creativecommons.org' and bool(match)
    return {'declared':declared,'license':'cc-by' if valid else None,'version':match[1] if valid else None,
            'license_url':f'https://creativecommons.org/licenses/by/{match[1]}/' if valid else None,
            'dataset_title':metadata.get('title'),'dataset_doi':metadata.get('doi'),
            'citation':metadata.get('citation'),'evidence':evidence,'review_state':'pending',
            'notice':'Dataset terms do not authorize photographs, copied descriptions or other databases.'}


class GbifProvider:
    name='gbif'
    BASE='https://api.gbif.org/v1/'
    FIELDS=('key','datasetKey','constituentKey','nubKey','taxonID','scientificName','canonicalName','authorship','rank',
            'taxonomicStatus','acceptedKey','accepted','publishedIn','lastInterpreted','nameType')+tuple(RANKS)+tuple(r+'Key' for r in RANKS)

    def __init__(self, client, dataset='d7dddbf4-2cf0-4f39-9b2a-bb099caae36c'):
        if not re.fullmatch(r'[0-9a-f-]{36}',dataset):raise ValueError('GBIF checklist UUID required')
        self.client=client;self.dataset=dataset;self._rights=None

    def rights(self):
        if self._rights is None:
            data,ev=self.client.get(self.BASE+'dataset/'+self.dataset,select=lambda d:pick(d,('key','title','doi','license','citation','version')))
            if data.get('key')!=self.dataset:raise ProviderError('Checklist identity mismatch')
            self._rights=dataset_rights(data,ev)
        return self._rights

    def normalize(self, row, evidence):
        if row.get('datasetKey')!=self.dataset:raise ProviderError('Result from an unexpected GBIF checklist')
        sid=positive_id(row['key']);accepted=str(row.get('acceptedKey') or sid)
        return Assertion(Identity(self.name,sid,self.dataset),row['scientificName'],row['canonicalName'],row['rank'].lower(),
            row.get('taxonomicStatus','unknown').lower(),row.get('authorship',''),
            Identity(self.name,positive_id(accepted),self.dataset),row.get('accepted') or row['canonicalName'],
            lineage(row),[{'provider':'gbif','id':sid,'dataset':self.dataset},{'provider':'source_taxonID','id':row.get('taxonID')}],
            [{'url':'https://www.gbif.org/species/'+sid,'locator':'taxon '+sid},
             {'citation':row.get('publishedIn'),'locator':'publishedIn'},
             {'upstream_dataset_key':row.get('constituentKey'),'locator':'constituentKey'},
             {'value':row.get('lastInterpreted'),'locator':'lastInterpreted (processing date, not scientific review)'}],self.rights(),[evidence])

    def fetch(self, record_id):
        sid=positive_id(record_id)
        data,ev=self.client.get(self.BASE+'species/'+sid,select=lambda d:pick(d,self.FIELDS))
        if str(data.get('key'))!=sid:raise ProviderError('Taxon identity mismatch')
        return self.normalize(data,ev)

    def query(self, params):
        data,ev=self.client.get(self.BASE+'species/search',params,
            select=lambda d:{'results':[pick(r,self.FIELDS) for r in d['results']],'endOfRecords':d.get('endOfRecords')})
        return [self.normalize(r,ev) for r in data['results'] if r.get('canonicalName') and r.get('nameType')=='SCIENTIFIC']

    def search(self, name, limit=5):
        rows=self.query({'q':name,'datasetKey':self.dataset,'limit':min(100,max(10,limit))})
        return [r for r in rows if name_key(r.canonical_name)==name_key(name)][:limit]

    def discover(self, scope, limit=5, offset=0):
        root=scope.roots.get(self.name)
        if not root:return []
        ancestor=self.fetch(root['id'])
        if name_key(ancestor.canonical_name)!=name_key(root['name']):raise ProviderError('Discovery root identity mismatch')
        rows=self.query({'highertaxon_key':root['id'],'datasetKey':self.dataset,'rank':'SPECIES',
                         'status':'ACCEPTED','limit':limit,'offset':offset})
        # A full-text hit for "Archaea" is not evidence of membership in Archaea.
        return [r for r in rows if any(str(n['source_id'])==str(root['id']) and name_key(n['name'])==name_key(root['name']) for n in r.lineage)]


class WormsProvider:
    name='worms'
    BASE='https://www.marinespecies.org/rest/'
    FIELDS=('AphiaID','url','scientificname','authority','status','unacceptreason','valid_AphiaID','valid_name',
            'valid_authority','rank','lsid','citation','modified','isMarine','isExtinct')+RANKS

    def __init__(self, client):self.client=client

    def normalize(self, row, evidence):
        sid=positive_id(row['AphiaID']);target=row.get('valid_AphiaID')
        return Assertion(Identity(self.name,sid),row['scientificname'],row['scientificname'],row['rank'].lower(),
            row.get('status','unknown'),row.get('authority',''),Identity(self.name,positive_id(target)) if target else None,
            row.get('valid_name') or '',lineage(row),[{'provider':'worms','id':sid},{'provider':'lsid','id':row.get('lsid')}],
            [{'url':row['url'],'locator':'AphiaID '+sid,'citation':row.get('citation')},
             {'locator':'unacceptreason','value':row.get('unacceptreason')}],
            {'declared':'CC BY; individual exceptions and database redistribution limits apply',
             'license':'cc-by','version':None,'terms_url':'https://www.marinespecies.org/about.php',
             'citation':row.get('citation'),'review_state':'pending',
             'notice':'No gallery images or whole-database replication. Version and record-specific conditions require human review.'},
            [evidence],row.get('modified'))

    def fetch(self, record_id):
        sid=positive_id(record_id)
        data,ev=self.client.get(self.BASE+'AphiaRecordByAphiaID/'+sid,select=lambda d:pick(d,self.FIELDS))
        if str(data.get('AphiaID'))!=sid:raise ProviderError('AphiaID mismatch')
        return self.normalize(data,ev)

    def search(self, name, limit=5):
        data,ev=self.client.get(self.BASE+'AphiaRecordsByName/'+quote(name,safe=''),{'like':'false','marine_only':'false'},
            select=lambda rows:[pick(r,self.FIELDS) for r in rows])
        return [self.normalize(r,ev) for r in data if name_key(r.get('scientificname',''))==name_key(name)][:limit]


class ColProvider:
    name='col'
    BASE='https://api.checklistbank.org/dataset/'

    def __init__(self, client, dataset='316165'):
        self.client=client;self.dataset=positive_id(dataset);self._rights=None

    @staticmethod
    def selected(row):
        usage=row.get('usage',row if 'name' in row else {})
        return {'id':row.get('id',usage.get('id')),'classification':[pick(n,('id','name','rank','authorship')) for n in row.get('classification',[])],
            'usage':dict(pick(usage,('id','datasetKey','status','acceptedId','acceptedName','identifier','link','modified','merged','sectorKey')),
                         name=pick(usage.get('name',{}),('scientificName','authorship','rank','id','code'))),
            'sectorDatasetKey':row.get('sectorDatasetKey')}

    def rights(self):
        if self._rights is None:
            meta,ev=self.client.get(self.BASE+self.dataset,select=lambda d:pick(d,('key','title','version','doi','license','citation')))
            if str(meta.get('key'))!=self.dataset:raise ProviderError('CoL release identity mismatch')
            self._rights=dataset_rights(meta,ev);self._rights['dataset_version']=meta.get('version')
            # "cc by" alone does not establish a version. Do not fill in 4.0.
        return self._rights

    def normalize(self, row, evidence):
        usage=row['usage'];name=usage['name'];sid=str(usage['id'])
        if str(usage.get('datasetKey'))!=self.dataset:raise ProviderError('Unexpected CoL release')
        accepted=str(usage.get('acceptedId') or sid) if usage.get('status')=='accepted' or usage.get('acceptedId') else None
        return Assertion(Identity(self.name,sid,self.dataset),name['scientificName'],name['scientificName'],name['rank'],
            usage.get('status','unknown'),name.get('authorship',''),Identity(self.name,accepted,self.dataset) if accepted else None,
            usage.get('acceptedName') or (name['scientificName'] if usage.get('status')=='accepted' else ''),
            [dict(name=n['name'],rank=n.get('rank','unranked'),source_id=n['id']) for n in row.get('classification',[])],
            [{'provider':'col','id':sid,'dataset':self.dataset}]+[{'provider':'source_identifier','id':i} for i in usage.get('identifier',[])],
            [{'url':f'https://www.checklistbank.org/dataset/{self.dataset}/taxon/{quote(sid,safe="")}','locator':'nameusage '+sid},
             {'url':usage.get('link'),'locator':'original source','sector_dataset':row.get('sectorDatasetKey'),'merged':usage.get('merged')}],
            self.rights(),[evidence],usage.get('modified'))

    def search(self, name, limit=5):
        data,ev=self.client.get(self.BASE+self.dataset+'/nameusage/search',{'q':name,'limit':min(100,max(10,limit))},
            select=lambda d:{'result':[self.selected(r) for r in d['result']]})
        return [self.normalize(r,ev) for r in data['result'] if name_key(r['usage']['name'].get('scientificName',''))==name_key(name)][:limit]

    def fetch(self, record_id):
        if not re.fullmatch(r'[A-Za-z0-9_-]+',record_id):raise ProviderError('Invalid CoL identifier')
        data,ev=self.client.get(self.BASE+self.dataset+'/nameusage/'+quote(record_id,safe=''),select=self.selected)
        if str(data['usage'].get('id'))!=record_id:raise ProviderError('CoL record identity mismatch')
        return self.normalize(data,ev)


class CommonsProvider:
    name='commons'

    def __init__(self, client):self.client=client;self.diagnostics=[]

    def search_media(self, name, limit=5):
        # No image download. File-specific rights are filtered using the same
        # policy as the existing, audited exploration photo acquisition flow.
        from exploration_media import FIELDS, license_from, plain
        params={'action':'query','format':'json','generator':'search','gsrnamespace':6,
                'gsrsearch':'"'+name+'" filetype:bitmap','gsrlimit':limit,'prop':'imageinfo|info',
                'iiprop':'url|size|sha1|extmetadata','iiextmetadatafilter':'|'.join(FIELDS),'maxlag':5}
        fields=('pageid','title','lastrevid','imageinfo')
        data,ev=self.client.get('https://commons.wikimedia.org/w/api.php',params,
            select=lambda d:{'pages':[pick(p,fields) for p in d.get('query',{}).get('pages',{}).values()]})
        return self.normalize(data,ev,name)

    def fetch_media(self, title):
        from exploration_media import FIELDS
        if not title.startswith('File:'):raise ProviderError('Specific Commons File title required')
        data,ev=self.client.get('https://commons.wikimedia.org/w/api.php',
            {'action':'query','format':'json','titles':title,'prop':'imageinfo|info','iiprop':'url|size|sha1|extmetadata',
             'iiextmetadatafilter':'|'.join(FIELDS),'maxlag':5},
            select=lambda d:{'pages':[pick(p,('pageid','title','lastrevid','imageinfo')) for p in d.get('query',{}).get('pages',{}).values()]})
        return self.normalize(data,ev,'')

    def normalize(self,data,ev,name):
        from exploration_media import license_from,plain
        records=[]
        for page in data['pages']:
            info=(page.get('imageinfo') or [{}])[0];meta=info.get('extmetadata',{})
            try:license,version,url=license_from(meta)
            except ValueError as error:
                self.diagnostics.append({'record':page.get('pageid'),'title':page.get('title'),'reason':str(error),'evidence':ev});continue
            author=plain(meta.get('Artist',{}).get('value',''))
            if not author or min(info.get('width',0),info.get('height',0))<320:
                self.diagnostics.append({'record':page.get('pageid'),'reason':'Missing author or inadequate resolution','evidence':ev});continue
            sid=str(page['pageid']);record=Assertion(Identity(self.name,sid),'','','','unverified_media_association',
                references=[{'url':info['descriptionurl'],'locator':'File page revision '+str(page['lastrevid'])}],
                rights={'license':license,'version':version,'license_url':url,'author':author,
                        'title':plain(meta.get('ObjectName',{}).get('value','')) or None,'review_state':'pending'},
                provenance=[ev],kind='media',media={'file_title':page['title'],'revision_id':page['lastrevid'],'search_query':name,
                    'original_url':info['url'],'original_sha1':info['sha1'],'width':info['width'],'height':info['height'],
                    'identification_evidence':plain(meta.get('ImageDescription',{}).get('value','')),
                    'modifications':'Nenhuma; somente metadados consultados. Arquivo não baixado.',
                    'association':'Search hit only: may be an illustration, wrong organism or unrelated context. Human check required.'})
            records.append(record)
        return records
