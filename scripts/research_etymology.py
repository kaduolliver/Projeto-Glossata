"""Maintenance-only reference triage. Never approves or invents etymology.

Checks the species' existing scientific reference and group nomenclature bases.
HTTP success and a keyword are search leads, not verification. Human review of
an explicit etymological statement remains necessary before updating the corpus.
"""
import concurrent.futures
import datetime
import json
import pathlib
import re
import urllib.request
from urllib.parse import urlparse

ROOT = pathlib.Path(__file__).resolve().parents[1]

def inspect(url):
    try:
        request = urllib.request.Request(url, headers={'User-Agent': 'GlossataScientificReview/1.0'})
        with urllib.request.urlopen(request, timeout=12) as response:
            body = response.read(2_000_000).decode('utf8', errors='replace')
            body = re.sub(r'<[^>]*>', ' ', body)
            return {'url': url, 'retrieved': True, 'etymology_keyword': bool(re.search(r'etymolog|origem do nome|name derivation', body, re.I))}
    except Exception as error:
        return {'url': url, 'retrieved': False, 'error': type(error).__name__}

def research():
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
            if species['educational_kingdom'] == 'Monera': urls += ['https://lpsn.dsmz.de/genus/' + genus]
            if species['educational_kingdom'] == 'Fungi': urls += ['https://www.first-nature.com/fungi/' + species['name'].lower().replace(' ', '-') + '.php']
        # Classification alone is never searched as an etymology reference.
        urls = sorted({u for u in urls if urlparse(u).hostname not in {'www.inaturalist.org', 'inaturalist.org'}})
        existing = previous.get('species', {}).get(sid, {})
        entries[sid] = dict(existing) if existing.get('scientific_name') == species['name'] else {}
        urls = sorted(set(urls) | set(entries[sid].get('sources_checked', [])))
        entries[sid].update(scientific_name=species['name'], verified_ranks=list(hints[sid].get('etymology_hints', {})),
                            pending_parts=[r for r in ('genus', 'species') if r not in hints[sid].get('etymology_hints', {})], sources_checked=urls)
        sources.update(urls)
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
        checks = dict(zip(sorted(sources), executor.map(inspect, sorted(sources))))
    data = {'checked_on': datetime.date.today().isoformat(),
            'scope': 'Reference triage, not an exhaustive search or automatic scientific approval.',
            'species': entries, 'reference_checks': checks}
    (folder / 'etymology_research.json').write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print('Species:', len(entries), 'references:', len(checks), 'retrieved:', sum(c['retrieved'] for c in checks.values()))

if __name__ == '__main__': research()
