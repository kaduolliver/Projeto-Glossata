"""Collect/audit small offline presentation photographs. No quiz data or player data is changed."""
import argparse
from datetime import datetime, timezone
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import time
import urllib.error
import urllib.parse
import urllib.request

from content_governance import load, save, local_file, image_privacy_findings

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = 'resources/data/exploration_photos.json'
UA = 'ProjetoGlossata/1.4 (https://github.com/kaduolliver/Projeto-Glossata)'
FIELDS = ('ObjectName','ImageDescription','Artist','Credit','LicenseShortName','LicenseUrl','Restrictions')
_last_request = {}
_blocked_hosts = set()


class PlainText(HTMLParser):
    def __init__(self): super().__init__(); self.parts=[]
    def handle_data(self, data): self.parts.append(data)
    def handle_starttag(self, tag, attrs):
        if tag in {'br','p','div','li','tr','td'}:self.parts.append(' ')
    def handle_endtag(self, tag):
        if tag in {'p','div','li','tr','td'}:self.parts.append(' ')


def plain(value):
    parser=PlainText();parser.feed(str(value));return ' '.join(''.join(parser.parts).split())


def license_from(metadata):
    """Decide from the file's license fields, never from Commons' site footer."""
    value=lambda key:metadata.get(key,{}).get('value','')
    name=value('LicenseShortName');url=urllib.parse.urlparse(value('LicenseUrl'))
    if value('Restrictions') or url.hostname!='creativecommons.org': raise ValueError('Unproved or restricted file license')
    path=url.path.rstrip('/')
    if name=='CC0' and path in {'/publicdomain/zero/1.0','/publicdomain/zero/1.0/deed.en'}:
        return 'cc0','1.0','https://creativecommons.org/publicdomain/zero/1.0/'
    match=re.fullmatch(r'CC BY (1\.0|2\.0|2\.5|3\.0|4\.0)',name)
    if match and path==f'/licenses/by/{match[1]}':
        return 'cc-by',match[1],f'https://creativecommons.org/licenses/by/{match[1]}/'
    raise ValueError('License absent, unknown or outside the CC0/CC BY policy')


def fetch(url):
    from scientific_sources.transport import SameHostRedirect
    host=urllib.parse.urlparse(url).hostname
    if urllib.parse.urlparse(url).scheme!='https' or host not in {'commons.wikimedia.org','upload.wikimedia.org','thumb.wikimedia.org'}:
        raise ValueError('Unexpected presentation resource endpoint')
    if host in _blocked_hosts:raise ValueError('Host rate limited; retry in a later, explicit run')
    time.sleep(max(0,1.1-(time.monotonic()-_last_request.get(host,0))))
    _last_request[host]=time.monotonic()
    request=urllib.request.Request(url,headers={'User-Agent':UA})
    try:
        with urllib.request.build_opener(SameHostRedirect()).open(request,timeout=30) as response:
            if urllib.parse.urlparse(response.url).hostname!=host:raise ValueError('Cross-host redirect refused')
            data=response.read(5_000_001)
            if len(data)>5_000_000:raise ValueError('Presentation resource exceeds size budget')
            return data
    except urllib.error.HTTPError as error:
        if error.code==429:_blocked_hosts.add(host)
        raise


def rendition_modifications(download, original):
    clean=lambda url:urllib.parse.urlsplit(url)._replace(query='',fragment='').geturl()
    origin='Arquivo original fornecido pelo Wikimedia Commons.' if clean(download)==clean(original) else 'Imagem reduzida fornecida pelo Wikimedia Commons.'
    return origin+' Na interface: preenchimento proporcional com recorte centrado no ponto focal e gradiente horizontal de transparência; sem retoque do organismo.'


def collect_commons(title, identifier, subject, root=ROOT, width=640, identification_category=None, usage='card'):
    if not re.fullmatch(r'[a-z0-9][a-z0-9_-]*',identifier):raise ValueError('Invalid local photograph identifier')
    budget=3072 if usage=='home' else 1024
    if not 320<=width<=budget:raise ValueError('Image width outside budget')
    params=dict(action='query',format='json',prop='imageinfo|info',iiprop='url|size|sha1|extmetadata',
                iiextmetadatafilter='|'.join(FIELDS),iiurlwidth=width,iiurlheight=width,titles=title,maxlag=5)
    if identification_category:
        params.update(prop='imageinfo|info|categories',clcategories=identification_category)
    endpoint='https://commons.wikimedia.org/w/api.php?'+urllib.parse.urlencode(params)
    data=json.loads(fetch(endpoint));page=next(iter(data['query']['pages'].values()))
    info=page['imageinfo'][0];metadata=info.get('extmetadata',{})
    license,version,license_url=license_from(metadata) # Always before the image request.
    author=plain(metadata.get('Artist',{}).get('value',''))
    if not author:raise ValueError('Source author credit missing')
    categories=[c['title'] for c in page.get('categories',[])]
    if identification_category and identification_category not in categories:raise ValueError('Requested source identification category not demonstrated')
    original_size=(info.get('width',0),info.get('height',0))
    selected=info['url'] if original_size[0] and max(original_size)<=width else info.get('thumburl',info['url'])
    download=urllib.parse.urlsplit(selected)._replace(query='',fragment='').geturl()
    if urllib.parse.urlparse(download).hostname not in {'upload.wikimedia.org','thumb.wikimedia.org'}:
        raise ValueError('Unexpected image host')
    raw=fetch(download)
    from refine_photos import image_dimensions
    folder=root/'resources/images/exploration';folder.mkdir(parents=True,exist_ok=True)
    temporary=folder/(identifier+'.part');temporary.write_bytes(raw)
    try:
        width,height=image_dimensions(temporary)
        if min(width,height)<160 or max(width,height)>budget:raise ValueError('Inadequate thumbnail dimensions')
        if original_size[0] and (width>original_size[0] or height>original_size[1]):raise ValueError('Upscaled source rejected')
        if image_privacy_findings(temporary):raise ValueError('Sensitive metadata in thumbnail')
        extension='.png' if raw.startswith(b'\x89PNG\r\n\x1a\n') else '.jpg' if raw.startswith(b'\xff\xd8') else None
        if not extension:raise ValueError('Unsupported bitmap format')
        path=folder/(identifier+extension);temporary.replace(path)
    finally:
        if temporary.exists():temporary.unlink()
    collected=datetime.now(timezone.utc).isoformat()
    evidence={'schema_version':1,'origin':'live_commons_file_metadata','collected_at':collected,'source_url':endpoint,
              'record':{'page_id':page['pageid'],'revision_id':page['lastrevid'],'file_title':page['title'],
                        'original_url':info['url'],'original_sha1':info['sha1'],'download_url':download,
                        'original_width':original_size[0],'original_height':original_size[1],
                        'metadata':{k:metadata[k] for k in FIELDS if k in metadata}}}
    if identification_category:evidence['record']['identification_category']=identification_category
    proof=f'resources/compliance/exploration/{identifier}.json';save(root/proof,evidence)
    return {'photo_id':'commons:'+str(page['pageid']),'subject':subject,'author':author,
            'title':plain(metadata.get('ObjectName',{}).get('value','')) or None,
            'photo_url':info['descriptionurl'],'source':info['descriptionurl'],
            'source_revision_id':page['lastrevid'],'download_url':download,'license':license,
            'license_version':version,'license_url':license_url,'collected_at':collected,
            'path':path.relative_to(root).as_posix(),'sha256':hashlib.sha256(raw).hexdigest(),
            'width':width,'height':height,'bytes':len(raw),'evidence_path':proof,
            'evidence_sha256':hashlib.sha256((root/proof).read_bytes()).hexdigest(),
            'modifications':rendition_modifications(download,info['url']),
            'permission_state':'source_license_evidenced','scientific_review':'source_identification; human review not declared',
            **({'usage':'home'} if usage=='home' else {})}


def photo_errors(photo, root=ROOT):
    errors=[]
    try:
        path=local_file(root,photo['path']);evidence_path=local_file(root,photo['evidence_path'])
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=photo['sha256']:errors.append('photo_hash')
        if not evidence_path.is_file() or hashlib.sha256(evidence_path.read_bytes()).hexdigest()!=photo['evidence_sha256']:
            return errors+['permission_evidence_hash']
        proof=load(evidence_path);record=proof['record'];metadata=record['metadata']
        identity=license_from(metadata)
        if tuple(photo.get(k) for k in ('license','license_version','license_url'))!=identity:errors.append('license_evidence_mismatch')
        if photo['author']!=plain(metadata['Artist']['value']):errors.append('credit_evidence_mismatch')
        if photo.get('title')!=(plain(metadata.get('ObjectName',{}).get('value','')) or None):errors.append('title_evidence_mismatch')
        if photo['photo_id']!='commons:'+str(record['page_id']) or photo['source_revision_id']!=record['revision_id']:errors.append('photo_identity')
        if photo['download_url']!=record['download_url']:errors.append('download_identity')
        collected=datetime.fromisoformat(photo['collected_at'])
        if photo['collected_at']!=proof['collected_at'] or not collected.tzinfo or collected>datetime.now(timezone.utc):errors.append('collection_date')
        if not photo.get('modifications') or not photo.get('subject'):errors.append('missing_scope_or_modifications')
        if any(not isinstance(photo.get(k,0.5),(float,int)) or not 0<=photo.get(k,0.5)<=1 for k in ('focus_x','focus_y')):errors.append('invalid_focus')
        scope=photo.get('scope',{})
        if scope.get('classification_evidence'):
            classification=local_file(root,scope['classification_evidence'])
            if not classification.is_file() or hashlib.sha256(classification.read_bytes()).hexdigest()!=scope.get('classification_evidence_sha256'):errors.append('classification_evidence_hash')
            else:
                assertion=load(classification)['assertion']
                if assertion['canonical_name']!=photo['subject'] or assertion['status']!='accepted' or assertion['references'][0]['url']!=scope.get('classification_source'):errors.append('classification_identity')
        if not photo['photo_url'].startswith('https://commons.wikimedia.org/wiki/File:'):errors.append('source_url')
        if urllib.parse.unquote(urllib.parse.urlparse(photo['photo_url']).path).replace('_',' ')!='/wiki/'+record['file_title'].replace('_',' '):errors.append('source_record_identity')
        if photo.get('bytes')!=path.stat().st_size:errors.append('image_size')
        from refine_photos import image_dimensions
        if image_dimensions(path)!=(photo['width'],photo['height']):errors.append('image_dimensions')
        budget=3072 if photo.get('usage')=='home' else 1024
        if min(photo['width'],photo['height'])<160 or max(photo['width'],photo['height'])>budget:errors.append('image_budget')
        if photo.get('usage')=='home' and (not record.get('original_width') or not record.get('original_height')
            or photo['width']>record['original_width'] or photo['height']>record['original_height']):errors.append('unproved_native_resolution')
    except (KeyError,ValueError,TypeError,OSError):errors.append('incomplete_or_invalid_photo')
    return sorted(set(errors))


def matching_variant(base, variant, root=ROOT):
    """A larger rendition must depict the very same source file, not a new identification."""
    fields=('photo_id','subject','author','license','license_version','license_url','photo_url')
    if variant.get('usage')!='home' or any(base.get(k)!=variant.get(k) for k in fields):return False
    try:
        first=load(local_file(root,base['evidence_path']))['record']
        second=load(local_file(root,variant['evidence_path']))['record']
        return bool(first.get('original_sha1')) and first['original_sha1']==second.get('original_sha1')
    except (KeyError,ValueError,OSError):return False


def matching_replacement(base, replacement, root=ROOT):
    """A different Home photo needs its own evidence, not the base photo's permission or review."""
    if (replacement.get('usage')!='home' or replacement.get('subject')!=base.get('subject')
        or replacement.get('catalog_species_id')!=base.get('catalog_species_id')
        or replacement.get('width',0)<1920 or replacement.get('height',0)<1080):return False
    try:
        species=next(s for s in load(root/'resources/data/species.json') if s['id']==replacement['catalog_species_id'])
        record=load(local_file(root,replacement['evidence_path']))['record']
        category=record.get('identification_category','')
        return species['name']==replacement['subject'] and bool(re.match(
            r'^Category:'+re.escape(species['name'])+r'(?:$|[ (])',category))
    except (KeyError,ValueError,OSError,StopIteration):return False


def audit(root=ROOT):
    data=load(root/MANIFEST,{'photos':{},'bindings':{}});errors=[]
    nodes={n['id']:n for n in load(root/'resources/data/taxonomy.json',[])}
    species={s['id']:s for s in load(root/'resources/data/species.json',[])}
    photos=data.get('photos',{})
    for key,photo in photos.items():
        errors += [{'photo':key,'error':e} for e in photo_errors(photo,root)]
    for base,key in data.get('home_variants',{}).items():
        if base not in photos or key not in photos or not matching_variant(photos[base],photos[key],root):
            errors.append({'photo':key,'error':'home_variant_identity'})
    for base,key in data.get('home_replacements',{}).items():
        if base not in photos or key not in photos or not matching_replacement(photos[base],photos[key],root):
            errors.append({'photo':key,'error':'home_replacement_identity'})
    for taxon,photo in data.get('bindings',{}).items():
        if taxon not in nodes or photo not in photos:errors.append({'taxon':taxon,'error':'invalid_binding'})
        elif photos[photo].get('usage')=='home':errors.append({'taxon':taxon,'error':'home_variant_used_for_card'})
        elif nodes[taxon]['rank']=='species' and photos[photo].get('subject')!=nodes[taxon]['name']:
            errors.append({'taxon':taxon,'error':'representative_species_mismatch'})
        elif photos[photo].get('catalog_species_id') is not None:
            organism=species.get(photos[photo]['catalog_species_id'],{})
            if organism.get('name')!=photos[photo]['subject'] or taxon not in organism.get('taxonomy',[]):
                errors.append({'taxon':taxon,'error':'representative_identity_mismatch'})
        else:
            scope=photos[photo].get('scope',{})
            if taxon not in scope.get('taxon_ids',[]) or not scope.get('rationale') or scope.get('evidence_url')!=photos[photo]['photo_url']:
                errors.append({'taxon':taxon,'error':'representative_scope_missing'})
            elif scope.get('classification_evidence'):
                assertion=load(local_file(root,scope['classification_evidence']),{}).get('assertion',{})
                names={r['name'] for r in assertion.get('lineage',[])}|{assertion.get('canonical_name')}
                if nodes[taxon]['name'] not in names:errors.append({'taxon':taxon,'error':'classification_scope_mismatch'})
        if taxon in {'domain:Bacteria','domain:Archaea'} and photos.get(photo,{}).get('kind')!='micrograph':
            errors.append({'taxon':taxon,'error':'micrograph_required'})
    if data.get('mixed_photo') not in photos and photos:errors.append({'error':'mixed_photo'})
    pending=data.get('pending',{})
    coverage=[]
    for taxon,node in nodes.items():
        binding=data.get('bindings',{}).get(taxon)
        issue=pending.get(taxon)
        if not binding and (not isinstance(issue,dict) or not issue.get('reason') or not issue.get('attempted_subjects')):
            errors.append({'taxon':taxon,'error':'undocumented_photograph_gap'})
        if binding and issue:errors.append({'taxon':taxon,'error':'conflicting_coverage_state'})
        coverage.append({'id':taxon,'name':node['name'],'rank':node['rank'],
                         'state':'photograph' if binding else 'pending','photo':binding,'pending':issue})
    for taxon in pending:
        if taxon not in nodes:errors.append({'taxon':taxon,'error':'unknown_pending_group'})
    mixed_valid=data.get('mixed_photo') in photos
    coverage.append({'id':'special:mixed','name':'Todas as espécies','rank':'special',
        'state':'photograph' if mixed_valid else 'pending','photo':data.get('mixed_photo'),'pending':None})
    return {'total_photos':len(photos),'bindings':len(data.get('bindings',{})),
            'total_cards':len(nodes)+1,'documented_pending':len(pending),'coverage':coverage,
            'taxon_cards':len(nodes),'photographed_cards':len(data.get('bindings',{}))+int(mixed_valid),
            'coverage_percent':round(100*(len(data.get('bindings',{}))+int(mixed_valid))/(len(nodes)+1),2),
            'total_bytes':sum(p.get('bytes',0) for p in photos.values()),'errors':errors,
            'notice':'File-level source evidence and technical checks, not expert scientific review or legal certification.'}


def credit_records(root=ROOT):
    data=load(root/MANIFEST,{'photos':{}})
    return {key:dict(p,rights_issues=photo_errors(p,root)) for key,p in data['photos'].items()}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--collect-plan',type=Path)
    parser.add_argument('--discover',action='store_true',help='Propose photographs for every catalog organism; never publishes a search hit')
    parser.add_argument('--online',action='store_true',help='Explicitly enable collection/discovery network access')
    parser.add_argument('--offset',type=int,default=0)
    parser.add_argument('--limit',type=int,default=40)
    parser.add_argument('--search-page',type=int,choices=range(3),default=0,help='0..2: next eight metadata results for unresolved subjects')
    parser.add_argument('--cc0-search',action='store_true',help='Prioritize structured CC0 results; still verify the individual file license')
    parser.add_argument('--by-search',action='store_true',help='Search the CC BY 4.0 category; still verify the individual file license')
    parser.add_argument('--by-version',choices=['2.0','3.0','4.0'],default='4.0')
    parser.add_argument('--discovery-plan',type=Path,help='Additional representative subjects; metadata proposals only')
    parser.add_argument('--complete-coverage',action='store_true',help='Generate explicit bindings and justified pending entries for every node')
    parser.add_argument('--sync-resources',action='store_true',help='Inventory prepared presentation media and add it to the offline QRC')
    args=parser.parse_args()
    if args.discover:
        from exploration_discovery import discover
        extra=load(args.discovery_plan)['subjects'] if args.discovery_plan else None
        discover(ROOT,args.online,args.offset,args.limit,args.search_page,args.cc0_search,args.by_version if args.by_search else False,extra)
        raise SystemExit(0)
    if args.collect_plan:
        if not args.online:parser.error('--collect-plan requires --online; auditing is offline by default')
        plan=load(args.collect_plan);data=load(ROOT/MANIFEST,{'photos':{},'bindings':{}});photos=data['photos']
        preparation=[]
        for entry in plan['photos']:
            prior=photos.get(entry['id'],{})
            prior_file=load(ROOT/prior['evidence_path'])['record']['file_title'] if prior else None
            if prior_file==entry['file'] and max(prior['width'],prior['height'])>=entry.get('width',640)*0.9 and not photo_errors(prior):continue
            attempts=[]
            for title in [entry['file']]+entry.get('retry_files',[]):
                try:
                    photo=collect_commons(title,entry['id'],entry['subject'],width=entry.get('width',640),identification_category=entry.get('identification_category'),usage=entry.get('usage','card'))
                    photo.update({k:entry[k] for k in ('kind','catalog_species_id','identification_note','scope','focus_x','focus_y','preparation') if k in entry})
                    if entry.get('variant_of'):
                        if not matching_variant(photos.get(entry['variant_of'],{}),photo):raise ValueError('Variant source identity differs from archived base')
                        data.setdefault('home_variants',{})[entry['variant_of']]=entry['id']
                    if entry.get('replacement_of'):
                        if not matching_replacement(photos.get(entry['replacement_of'],{}),photo):raise ValueError('Replacement species or source evidence incompatible')
                        data.setdefault('home_replacements',{})[entry['replacement_of']]=entry['id']
                    photos[entry['id']]=photo
                    save(ROOT/MANIFEST,data) # A bounded interrupted run preserves completed records.
                    print(entry['subject']+': prepared '+title,flush=True)
                    break
                except (ValueError,OSError) as error:
                    attempts.append({'file':title,'error':str(error)})
                    print(entry['subject']+': rejected '+str(error),flush=True)
            preparation.append({'subject':entry['subject'],'attempts':attempts,'prepared':entry['id'] in photos})
            save(ROOT/'build/exploration-discovery/preparation.json',preparation)
    if args.complete_coverage:
        from exploration_discovery import complete_coverage
        complete_coverage(ROOT)
    if args.sync_resources:
        from exploration_discovery import sync_resources
        sync_resources(ROOT)
    report=audit();save(ROOT/'build/exploration-media-audit.json',report)
    print({k:v for k,v in report.items() if k not in {'coverage','errors'}},'errors:',len(report['errors']))
    if report['errors']:print(report['errors'][:10])
    raise SystemExit(bool(report['errors']))
