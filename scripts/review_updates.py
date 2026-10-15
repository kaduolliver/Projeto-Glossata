"""Maintenance proposals only. Default is offline; never modifies published data.

Online checks are explicitly requested, bounded and paced. HTTP success is not
scientific evidence. A later CC license change does not automatically revoke a
valid prior grant; preserve the archived evidence and request human analysis.
"""
import argparse
from datetime import datetime, timezone, timedelta
import ipaddress
from pathlib import Path
import time
import urllib.error
import urllib.request
from urllib.parse import urlparse
from content_governance import ROOT, audit_governance, digest, load, now, parse_time, records, sanitized_observation, save


class PublicClient:
    def __init__(self): self.last = 0

    def get(self, url, as_json=False):
        import json
        host = urlparse(url).hostname or ''
        if urlparse(url).scheme != 'https' or not host or host == 'localhost' or urlparse(url).username:
            raise ValueError('Somente fontes HTTPS públicas.')
        try:
            if not ipaddress.ip_address(host).is_global: raise ValueError('Endereço privado bloqueado.')
        except ValueError:
            if re_is_ip(host): raise
        time.sleep(max(0, 1.1 - (time.monotonic() - self.last))); self.last = time.monotonic()
        request = urllib.request.Request(url, headers={'User-Agent': 'ProjetoGlossata/1.0 evidence-maintenance'})
        with urllib.request.urlopen(request, timeout=15) as response:
            body = response.read(4_000_001 if as_json else 1024)
            if as_json and len(body) > 4_000_000: raise ValueError('Resposta excede o limite de manutenção.')
            return json.loads(body) if as_json else {'http_status': response.status, 'final_url': response.url}


def re_is_ip(host):
    try: ipaddress.ip_address(host); return True
    except ValueError: return False


def changes_for_photo(species, observation):
    photo = species['photo']; current = next((p for p in observation.get('photos', []) if str(p.get('id')) == photo['id']), None)
    changes = []
    taxon = observation.get('taxon') or {}
    if not current: changes.append('photo_removed_from_observation')
    else:
        if current.get('license_code') != photo.get('license'): changes.append('license_changed_requires_analysis_not_automatic_revocation')
        if current.get('attribution') != photo.get('author'): changes.append('credit_changed_or_diverges')
        if current.get('hidden') or current.get('flags'): changes.append('photo_hidden_or_flagged')
    if (str(taxon.get('id')) != str(photo.get('observation_taxon_id', photo.get('taxon_id')))
            or taxon.get('name') != photo.get('observation_scientific_name', photo.get('scientific_name'))):
        changes.append('possible_identification_or_taxonomy_change')
    if observation.get('community_taxon_id') != taxon.get('id'): changes.append('community_taxon_disagrees')
    if observation.get('quality_grade') != 'research': changes.append('not_research_grade')
    if not taxon.get('is_active'): changes.append('taxon_not_active')
    return changes, sanitized_observation(observation, current or {'id': photo['id']})


def hierarchy_changed(species, incoming, nodes):
    stored = [nodes[n] for n in species.get('taxonomy', []) if n in nodes and nodes[n].get('source_taxon_id')
              and str(nodes[n]['source_taxon_id']) != str(incoming.get('id'))]
    previous = [str(n['source_taxon_id']) for n in stored]
    current = [str(i) for i in incoming.get('ancestor_ids', [])]
    if previous and current:
        if previous[0] not in current or current[current.index(previous[0]):] != previous: return True
    indexed = {str(n['id']): n for n in incoming.get('ancestors', [])}
    return any(str(n['source_taxon_id']) in indexed and
               (indexed[str(n['source_taxon_id'])].get('name') != n['name'] or indexed[str(n['source_taxon_id'])].get('rank') != n['rank']) for n in stored)


def propose_updates(root=ROOT, online=False, maximum=25, client=None):
    if maximum < 1: raise ValueError('Limite deve ser positivo.')
    report = {'schema_version': 1, 'checked_at': now(), 'mode': 'online_proposals' if online else 'offline',
              'notice': 'Propostas para revisão humana; nenhum conteúdo, licença ou data de revisão publicados foi substituído.',
              'overdue_and_pending': audit_governance(root)['scientific_findings'], 'checks': []}
    urls = sorted({r.get('source_url') for r in records(root).values() if r.get('source_url')})
    interval = load(root / 'resources/data/content_policy.json', {}).get('review_days', {}).get('links', 90)
    last = {}
    for path in (root / 'build/update-proposals').glob('*.json'):
        previous = load(path, {})
        for check in previous.get('checks', []):
            checked = parse_time(check.get('checked_at', previous.get('checked_at')))
            if check.get('type') == 'link' and checked and checked <= datetime.now(timezone.utc):
                url = check.get('url')
                if not last.get(url) or checked > last[url]: last[url] = checked
    report['reference_checks_due'] = []
    for url in urls:
        due = last[url] + timedelta(days=interval) if url in last else None
        report['reference_checks_due'].append({'url': url, 'interval_days': interval,
            'last_availability_check': last[url].isoformat() if url in last else None,
            'due_at': due.isoformat() if due else None, 'due': not due or due <= datetime.now(timezone.utc),
            'notice': 'Availability check date only; never a renewal of scientific review.'})
    if online:
        client = client or PublicClient(); species = load(root / 'resources/data/species.json', [])
        requests = 0; seen = set()
        for s in species:
            p = s['photo']; source = p.get('source', '')
            oid = urlparse(source).path.rstrip('/').split('/')[-1]
            if not oid.isdigit() or oid in seen or requests >= maximum: continue
            seen.add(oid); requests += 1
            try:
                data = client.get('https://api.inaturalist.org/v1/observations/' + oid, as_json=True)
                observation = data['results'][0]
                if str(observation.get('id')) != oid: raise ValueError('Identidade da resposta não confere.')
                changes, snapshot = changes_for_photo(s, observation)
                report['checks'].append({'type': 'observation', 'species_id': s['id'], 'photo_id': p['id'],
                                        'changes': changes, 'snapshot': snapshot, 'snapshot_sha256': digest(snapshot)})
            except urllib.error.HTTPError as error:
                report['checks'].append({'type': 'observation', 'species_id': s['id'], 'http_status': error.code,
                                         'retry_after': error.headers.get('Retry-After')})
                if error.code == 429: return report
            except Exception as error:
                report['checks'].append({'type': 'observation', 'species_id': s['id'], 'error': type(error).__name__})
        # A separate limit for taxonomy/conservation and reference availability.
        seen = set()
        nodes = {t['id']: t for t in load(root / 'resources/data/taxonomy.json', [])}
        for s in species:
            tid = s.get('source_taxon_id') or s['photo'].get('taxon_id')
            if tid in seen or len(seen) >= maximum: continue
            seen.add(tid)
            try:
                data = client.get('https://api.inaturalist.org/v1/taxa/' + str(tid), as_json=True)['results'][0]
                if str(data.get('id')) != str(tid): raise ValueError('Táxon da resposta incompatível.')
                snapshot = {k: data.get(k) for k in ('id', 'name', 'rank', 'is_active', 'ancestor_ids')}
                snapshot['ancestors'] = [{k: node.get(k) for k in ('id', 'name', 'rank')} for node in data.get('ancestors', [])]
                snapshot['conservation_statuses'] = [
                    {k: value.get(k) for k in ('id', 'taxon_id', 'taxon_rank', 'status', 'authority', 'iucn', 'url')}
                    | {'place_id': (value.get('place') or {}).get('id')}
                    for value in data.get('conservation_statuses', [])]
                profiles = load(root / 'resources/data/species_profiles.json', {})
                previous = profiles.get(str(s['id']), {}).get('conservation', {})
                global_iucn = [value for value in snapshot['conservation_statuses']
                    if str(value.get('taxon_id')) == str(tid) and value.get('taxon_rank') == 'species'
                    and value.get('authority') == 'IUCN Red List' and not value.get('place_id')]
                conservation_change = (previous.get('status') != 'unconfirmed' and
                    not any(value.get('status', '').upper() == previous.get('status') and value.get('url') == previous.get('assessment_url') for value in global_iucn))
                conservation_change |= previous.get('status') == 'unconfirmed' and bool(global_iucn)
                # Conservation changes are leads; iNaturalist is not an IUCN assessment.
                report['checks'].append({'type': 'taxonomy_conservation', 'species_id': s['id'], 'snapshot': snapshot,
                     'possible_change': data.get('name') != s['name'] or not data.get('is_active') or hierarchy_changed(s, data, nodes),
                     'possible_conservation_change': conservation_change,
                     'requires_comparison_with_cited_assessment': True, 'snapshot_sha256': digest(snapshot)})
            except urllib.error.HTTPError as error:
                report['checks'].append({'type': 'taxonomy_conservation', 'species_id': s['id'], 'http_status': error.code,
                                         'retry_after': error.headers.get('Retry-After')})
                if error.code == 429: return report
            except Exception as error:
                report['checks'].append({'type': 'taxonomy_conservation', 'species_id': s['id'], 'error': type(error).__name__})
        for url in urls[:maximum]:
            try: report['checks'].append({'type': 'link', 'url': url, 'checked_at': now(), **client.get(url)})
            except urllib.error.HTTPError as error:
                report['checks'].append({'type': 'link', 'url': url, 'http_status': error.code,
                                         'retry_after': error.headers.get('Retry-After'), 'requires_human_check': True})
                if error.code == 429: break  # Never hammer a rate-limited service.
            except Exception as error: report['checks'].append({'type': 'link', 'url': url, 'error': type(error).__name__})
    return report


def write_proposals(report, root=ROOT):
    path = root / 'build/update-proposals' / (datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ') + '.json')
    save(path, report); return path


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--online', action='store_true'); parser.add_argument('--max-checks', type=int, default=25)
    args = parser.parse_args()
    print(write_proposals(propose_updates(online=args.online, maximum=args.max_checks)))
