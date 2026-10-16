"""Maintenance-only reference triage. Never approves or invents etymology.

Checks the species' existing scientific reference and group nomenclature bases.
HTTP success and a keyword are search leads, not verification. Human review of
an explicit etymological statement remains necessary before updating the corpus.
"""
import datetime
import json
import pathlib
import re
import argparse
import urllib.request
from urllib.parse import urlparse

ROOT = pathlib.Path(__file__).resolve().parents[1]

def inspect(url):
    from scientific_sources.access import access_issue
    restriction=access_issue(url)
    if restriction:return {'url':url,'retrieved':False,'state':'manual_or_official_api_required','reason':restriction}
    try:
        request = urllib.request.Request(url, headers={'User-Agent': 'GlossataScientificReview/1.0'})
        with urllib.request.urlopen(request, timeout=12) as response:
            body = response.read(2_000_000).decode('utf8', errors='replace')
            body = re.sub(r'<[^>]*>', ' ', body)
            return {'url': url, 'retrieved': True, 'etymology_keyword': bool(re.search(r'etymolog|origem do nome|name derivation', body, re.I))}
    except Exception as error:
        return {'url': url, 'retrieved': False, 'error': type(error).__name__}

def research(online=False):
    folder = ROOT / 'resources/data'
    catalog = json.loads((folder / 'species.json').read_text(encoding='utf8'))
    hints = json.loads((folder / 'hint_education.json').read_text(encoding='utf8'))
    profiles = json.loads((folder / 'species_profiles.json').read_text(encoding='utf8'))
    target = folder / 'etymology_research.json'
    previous = json.loads(target.read_text(encoding='utf8')) if target.exists() else {}
    sources, entries = set(), {}
    for species in catalog:
        sid = str(species['id']); genus = species['name'].split()[0].lower()
        if hints[sid].get('etymology_hints'):
            urls = [n['source_url'] for n in hints[sid]['etymology_hints'].values()]
        else:
            urls = [profiles[sid]['morphology']['source_url']]
            if species.get('domain') in {'Bacteria','Archaea'}: urls += ['https://lpsn.dsmz.de/genus/' + genus]
            if any(n['name']=='Fungi' for n in species.get('source_taxonomy',[])): urls += ['https://www.first-nature.com/fungi/' + species['name'].lower().replace(' ', '-') + '.php']
        # Classification alone is never searched as an etymology reference.
        urls = sorted({u for u in urls if urlparse(u).hostname not in {'www.inaturalist.org', 'inaturalist.org'}})
        existing = previous.get('species', {}).get(sid, {})
        entries[sid] = dict(existing) if existing.get('scientific_name') == species['name'] else {}
        urls = sorted(set(urls) | set(entries[sid].get('sources_checked', [])))
        entries[sid].update(scientific_name=species['name'], verified_ranks=list(hints[sid].get('etymology_hints', {})),
                            pending_parts=[r for r in ('genus', 'species') if r not in hints[sid].get('etymology_hints', {})], sources_checked=urls)
        sources.update(urls)
    if online:
        from time import sleep
        checks = {}
        for url in sorted(sources):
            checks[url] = inspect(url)
            sleep(1.1)
    else:
        checks = {url: {'url': url, 'state': 'not_checked_offline'} for url in sorted(sources)}
    data = {'checked_on': datetime.date.today().isoformat(),
            'verified_ranks_notice': 'Legacy availability field; not new scientific approval. See governance/review history.',
            'scope': 'Reference triage, not an exhaustive search or automatic scientific approval.',
            'species': entries, 'reference_checks': checks}
    output = ROOT / 'build/update-proposals' / ('etymology-' + datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ') + '.json')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print('Species:', len(entries), 'references:', len(checks), 'retrieved:', sum(c.get('retrieved', False) for c in checks.values()))
    return data

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--online', action='store_true')
    research(parser.parse_args().online)
