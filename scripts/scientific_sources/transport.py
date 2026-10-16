"""Bounded, paced API access with immutable evidence and offline replay by default."""
from datetime import datetime, timezone
import hashlib
import http.client
import json
from pathlib import Path
import time
import urllib.error
import urllib.parse
import urllib.request
from .model import canonical, digest


class ProviderError(RuntimeError): pass
class CacheMiss(ProviderError): pass
class RateLimited(ProviderError): pass


def now(): return datetime.now(timezone.utc).isoformat()


def write(path, data):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    temporary=path.with_suffix(path.suffix+'.part')
    temporary.write_bytes(canonical(data)+b'\n');temporary.replace(path)


class SameHostRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        old=urllib.parse.urlparse(req.full_url);new=urllib.parse.urlparse(newurl)
        if new.scheme!='https' or old.hostname!=new.hostname:
            raise ProviderError('Cross-host or insecure redirect refused')
        return super().redirect_request(req,fp,code,msg,headers,newurl)


class CachedClient:
    HOSTS={'api.gbif.org','api.checklistbank.org','www.marinespecies.org','commons.wikimedia.org'}

    def __init__(self, cache, online=False, refresh=False, max_requests=40, interval=1.1, opener=None):
        if not 1<=max_requests<=100 or interval<1.1:raise ValueError('Request budget/pace outside policy')
        self.cache=Path(cache);self.online=online;self.refresh=refresh
        self.max_requests=max_requests;self.interval=interval;self.requests=0;self.last={};self.blocked=set()
        self.opener=opener or urllib.request.build_opener(SameHostRedirect()).open

    def get(self, base, params=None, select=lambda value:value):
        parts=urllib.parse.urlparse(base)
        if parts.scheme!='https' or parts.hostname not in self.HOSTS or parts.username or parts.port not in {None,443}:
            raise ProviderError('Unapproved API endpoint')
        url=base+('?' + urllib.parse.urlencode(sorted((params or {}).items())) if params else '')
        folder=self.cache/hashlib.sha256(url.encode()).hexdigest();pointer=folder/'latest.json'
        if pointer.is_file() and (not self.online or not self.refresh):
            try:
                reference=json.loads(pointer.read_bytes());sha=reference['snapshot_sha256']
                if not isinstance(sha,str) or len(sha)!=64 or any(c not in '0123456789abcdef' for c in sha):raise ValueError('Invalid evidence pointer')
                evidence_path=folder/(sha+'.json');snapshot=json.loads(evidence_path.read_bytes())
            except (OSError,ValueError,KeyError,TypeError) as error:
                raise ProviderError('Cached evidence missing/corrupt; explicit refresh required: '+url) from error
            if digest(snapshot)!=sha or snapshot.get('url')!=url or snapshot.get('schema_version')!=1:
                raise ProviderError('Cached evidence identity/hash/schema mismatch')
            return snapshot['payload'],dict(snapshot_sha256=sha,**{k:snapshot[k] for k in ['url','retrieved_at','response_sha256']})
        if not self.online:raise CacheMiss('No offline evidence: '+url)
        host=parts.hostname
        if host in self.blocked:raise RateLimited('Host already rate limited; no further requests')
        if self.requests>=self.max_requests:raise ProviderError('Request budget exhausted')
        time.sleep(max(0,self.interval-(time.monotonic()-self.last.get(host,0))))
        self.last[host]=time.monotonic();self.requests+=1
        request=urllib.request.Request(url,headers={'User-Agent':'ProjetoGlossata/1.3 (scientific-candidates; https://github.com/kaduolliver/Projeto-Glossata)', 'Accept':'application/json'})
        try:
            with self.opener(request,timeout=25) as response:
                raw=response.read(4_000_001);status=response.status
            if len(raw)>4_000_000:raise ProviderError('API response exceeds size limit')
            if status==204:payload=[]
            else:payload=json.loads(raw)
        except urllib.error.HTTPError as error:
            if error.code==429:
                self.blocked.add(host)
                raise RateLimited('HTTP 429; stop host; Retry-After: '+str(error.headers.get('Retry-After'))) from error
            raise ProviderError('HTTP '+str(error.code)+' at '+url) from error
        except (OSError,ValueError,http.client.HTTPException) as error:raise ProviderError('API unavailable or invalid JSON: '+url) from error
        if isinstance(payload,dict) and ('error' in payload or 'errors' in payload):raise ProviderError('API returned an error payload')
        # Select before persistence: no contacts, coordinates, observation users,
        # sequences, full descriptions or images in the maintenance cache.
        try:selected=select(payload)
        except (ValueError,KeyError,TypeError,AttributeError,IndexError) as error:
            raise ProviderError('Unexpected API schema: '+url) from error
        snapshot={'schema_version':1,'url':url,'retrieved_at':now(),
                  'response_sha256':hashlib.sha256(raw).hexdigest(),'payload':selected}
        sha=digest(snapshot);write(folder/(sha+'.json'),snapshot);write(pointer,{'snapshot_sha256':sha})
        return snapshot['payload'],dict(snapshot_sha256=sha,**{k:snapshot[k] for k in ['url','retrieved_at','response_sha256']})
