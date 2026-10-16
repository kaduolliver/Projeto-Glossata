"""Collect/audit small offline presentation photographs. No quiz data or player data is changed."""
import argparse
from datetime import datetime, timezone
import hashlib
import html
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import urllib.parse
import urllib.request

from content_governance import load, save, local_file, image_privacy_findings

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = 'resources/data/exploration_photos.json'
UA = 'ProjetoGlossata/1.2 (https://github.com/kaduolliver/Projeto-Glossata)'
FIELDS = ('ObjectName','ImageDescription','Artist','Credit','LicenseShortName','LicenseUrl','Restrictions')


class PlainText(HTMLParser):
    def __init__(self): super().__init__(); self.parts=[]
    def handle_data(self, data): self.parts.append(data)


def plain(value):
    parser=PlainText();parser.feed(str(value));return ' '.join(html.unescape(''.join(parser.parts)).split())


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
    request=urllib.request.Request(url,headers={'User-Agent':UA})
    with urllib.request.urlopen(request,timeout=30) as response:
        data=response.read(5_000_001)
        if len(data)>5_000_000:raise ValueError('Presentation resource exceeds size budget')
        return data


def collect_commons(title, identifier, subject, root=ROOT):
    params=dict(action='query',format='json',prop='imageinfo|info',iiprop='url|size|sha1|extmetadata',
                iiextmetadatafilter='|'.join(FIELDS),iiurlwidth=320,titles=title)
    endpoint='https://commons.wikimedia.org/w/api.php?'+urllib.parse.urlencode(params)
    data=json.loads(fetch(endpoint));page=next(iter(data['query']['pages'].values()))
    info=page['imageinfo'][0];metadata=info.get('extmetadata',{})
    license,version,license_url=license_from(metadata) # Always before the image request.
    author=plain(metadata.get('Artist',{}).get('value',''))
    if not author:raise ValueError('Source author credit missing')
    download=urllib.parse.urlsplit(info['thumburl'])._replace(query='',fragment='').geturl()
    if urllib.parse.urlparse(download).hostname not in {'upload.wikimedia.org','thumb.wikimedia.org'}:
        raise ValueError('Unexpected image host')
    raw=fetch(download)
    from refine_photos import image_dimensions
    folder=root/'resources/images/exploration';folder.mkdir(parents=True,exist_ok=True)
    temporary=folder/(identifier+'.part');temporary.write_bytes(raw)
    try:
        width,height=image_dimensions(temporary)
        if min(width,height)<160 or max(width,height)>1024:raise ValueError('Inadequate thumbnail dimensions')
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
                        'metadata':{k:metadata[k] for k in FIELDS if k in metadata}}}
    proof=f'resources/compliance/exploration/{identifier}.json';save(root/proof,evidence)
    return {'photo_id':'commons:'+str(page['pageid']),'subject':subject,'author':author,
            'title':plain(metadata.get('ObjectName',{}).get('value','')) or None,
            'photo_url':info['descriptionurl'],'source':info['descriptionurl'],
            'source_revision_id':page['lastrevid'],'download_url':download,'license':license,
            'license_version':version,'license_url':license_url,'collected_at':collected,
            'path':path.relative_to(root).as_posix(),'sha256':hashlib.sha256(raw).hexdigest(),
            'width':width,'height':height,'bytes':len(raw),'evidence_path':proof,
            'evidence_sha256':hashlib.sha256((root/proof).read_bytes()).hexdigest(),
            'modifications':'Miniatura reduzida fornecida pelo Wikimedia Commons. Na interface: ajuste proporcional e suavização das bordas por transparência; sem edição do organismo.',
            'permission_state':'source_license_evidenced','scientific_review':'source_identification; human review not declared'}


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
        if not photo['photo_url'].startswith('https://commons.wikimedia.org/wiki/File:'):errors.append('source_url')
        if urllib.parse.unquote(urllib.parse.urlparse(photo['photo_url']).path).replace('_',' ')!='/wiki/'+record['file_title'].replace('_',' '):errors.append('source_record_identity')
        if photo.get('bytes')!=path.stat().st_size:errors.append('image_size')
        from refine_photos import image_dimensions
        if image_dimensions(path)!=(photo['width'],photo['height']):errors.append('image_dimensions')
        if min(photo['width'],photo['height'])<160 or max(photo['width'],photo['height'])>1024:errors.append('image_budget')
    except (KeyError,ValueError,TypeError,OSError):errors.append('incomplete_or_invalid_photo')
    return sorted(set(errors))


def audit(root=ROOT):
    data=load(root/MANIFEST,{'photos':{},'bindings':{}});errors=[]
    nodes={n['id']:n for n in load(root/'resources/data/taxonomy.json',[])}
    species={s['id']:s for s in load(root/'resources/data/species.json',[])}
    photos=data.get('photos',{})
    for key,photo in photos.items():
        errors += [{'photo':key,'error':e} for e in photo_errors(photo,root)]
    for taxon,photo in data.get('bindings',{}).items():
        if taxon not in nodes or photo not in photos:errors.append({'taxon':taxon,'error':'invalid_binding'})
        elif photos[photo].get('catalog_species_id') is not None:
            organism=species.get(photos[photo]['catalog_species_id'],{})
            if organism.get('name')!=photos[photo]['subject'] or taxon not in organism.get('taxonomy',[]):
                errors.append({'taxon':taxon,'error':'representative_identity_mismatch'})
        else:
            scope=photos[photo].get('scope',{})
            if taxon not in scope.get('taxon_ids',[]) or not scope.get('rationale') or scope.get('evidence_url')!=photos[photo]['photo_url']:
                errors.append({'taxon':taxon,'error':'representative_scope_missing'})
        if taxon in {'domain:Bacteria','domain:Archaea'} and photos.get(photo,{}).get('kind')!='micrograph':
            errors.append({'taxon':taxon,'error':'micrograph_required'})
    if data.get('mixed_photo') not in photos and photos:errors.append({'error':'mixed_photo'})
    return {'total_photos':len(photos),'bindings':len(data.get('bindings',{})),
            'total_bytes':sum(p.get('bytes',0) for p in photos.values()),'errors':errors,
            'notice':'File-level source evidence and technical checks, not expert scientific review or legal certification.'}


def credit_records(root=ROOT):
    data=load(root/MANIFEST,{'photos':{}})
    return {key:dict(p,rights_issues=photo_errors(p,root)) for key,p in data['photos'].items()}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--collect-plan',type=Path)
    args=parser.parse_args()
    if args.collect_plan:
        plan=load(args.collect_plan);photos={}
        import time
        for entry in plan['photos']:
            photo=collect_commons(entry['file'],entry['id'],entry['subject'])
            photo.update({k:entry[k] for k in ('kind','catalog_species_id','identification_note','scope') if k in entry})
            photos[entry['id']]=photo;time.sleep(1)
        save(ROOT/MANIFEST,dict(schema_version=1,photos=photos,bindings=plan['bindings'],mixed_photo=plan['mixed_photo']))
    report=audit();save(ROOT/'build/exploration-media-audit.json',report);print(report)
    raise SystemExit(bool(report['errors']))
