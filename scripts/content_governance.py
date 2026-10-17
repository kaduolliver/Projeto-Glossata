"""Offline evidence, review history and attribution. Checks are not certification.

No network, no player databases, no inferred authors or retrospective review dates.
The legacy baseline preserves functionality, never proves human/scientific review.
"""
import argparse
import copy
from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from urllib.parse import urlparse
from governance_storage import (append_event, atomic_write, canonical, digest, governance_lock,
                                read_chain, require_governance_lock)
from governance_history import assurances, audit_history

ROOT = Path(__file__).resolve().parents[1]
TOPICS = ('taxonomy', 'photo', 'etymology', 'morphology', 'education')
APPROVALS = {'approved_editorial', 'approved_expert'}
DEFAULT_LICENSES = {'cc0', 'cc-by'}
LABELS = {'legacy': 'Legado sem revisão humana rastreável', 'draft': 'Rascunho; revisão humana pendente',
          'needs_review': 'Conteúdo ou evidência alterados; nova revisão necessária', 'expired': 'Revisão vencida',
          'approved_editorial': 'Revisão editorial registrada', 'approved_expert': 'Revisão por especialista registrada',
          'pending': 'Revisão pendente', 'rejected': 'Conteúdo suspenso'}


def now():
    return datetime.now(timezone.utc).isoformat()


def load(path, default=None):
    return json.loads(Path(path).read_text(encoding='utf8')) if Path(path).exists() else copy.deepcopy(default)


def save(path, value):
    path = Path(path); path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.part')
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf8', newline='\n')
    temporary.replace(path)


def policy(root=ROOT):
    return load(root / 'resources/data/content_policy.json', load(ROOT / 'resources/data/content_policy.json'))


def local_file(root, relative):
    """Never permit evidence/permission paths to escape the project."""
    target = (root / relative).resolve()
    target.relative_to(root.resolve())
    return target


def https_url(value):
    p = urlparse(value or '')
    return p.scheme == 'https' and bool(p.hostname) and not p.username and not p.password


def image_privacy_findings(path):
    """Detect EXIF GPS/camera serial and common XMP GPS tags, without exposing values.

    This is a conservative detector, not a complete metadata-forensics tool.
    Never silently strip metadata or change licensed bytes during an audit.
    """
    data = path.read_bytes(); findings = set()
    if any(tag in data for tag in (b'GPSLatitude', b'GPSLongitude', b'exif:GPS')):
        findings.add('xmp_gps_metadata')
    start = data.find(b'Exif\x00\x00')
    tiff = data[start+6:] if start >= 0 else None
    if data.startswith(b'\x89PNG\r\n\x1a\n'):
        cursor = 8
        while cursor+12 <= len(data):
            size = int.from_bytes(data[cursor:cursor+4], 'big')
            if cursor+12+size > len(data): break
            if data[cursor+4:cursor+8] == b'eXIf': tiff = data[cursor+8:cursor+8+size]; break
            cursor += size+12
    if tiff is None: return sorted(findings)
    order = 'little' if tiff[:2] == b'II' else 'big' if tiff[:2] == b'MM' else None
    if not order or len(tiff) < 8: return ['exif_metadata_requires_manual_check']
    def integer(offset, length):
        if offset < 0 or offset+length > len(tiff): raise ValueError('Truncated EXIF')
        return int.from_bytes(tiff[offset:offset+length], order)
    try:
        pending = [integer(4, 4)]; visited = set()
        while pending:
            offset = pending.pop()
            if not offset or offset in visited: continue
            visited.add(offset); count = integer(offset, 2)
            if count > 4096: raise ValueError('Oversized EXIF directory')
            for index in range(count):
                entry = offset+2+12*index; tag = integer(entry, 2)
                if tag == 0x8825: findings.add('exif_gps_metadata')
                if tag in {0xA431, 0xA435}: findings.add('exif_camera_or_lens_serial')
                if tag == 0x8769: pending.append(integer(entry+8, 4))
            pending.append(integer(offset+2+12*count, 4))
    except ValueError: findings.add('exif_metadata_requires_manual_check')
    return sorted(findings)


def parse_time(value):
    try:
        dt = datetime.fromisoformat(value.replace('Z', '+00:00'))
        return dt.replace(tzinfo=timezone.utc) if dt.tzinfo is None else dt.astimezone(timezone.utc)
    except (ValueError, TypeError, AttributeError):
        return None


def documented_exception(photo, scope, root=ROOT):
    """An alternate grant must be explicit, scoped and bound to a source/photo.

    A generic permission for an observation cannot authorize all its images.
    License conditions are not automatically interpreted or waived by this code.
    """
    p = policy(root)
    for grant in p.get('exceptions', []):
        if (str(grant.get('photo_id')) != str(photo.get('id')) or scope not in grant.get('scopes', [])
                or not grant.get('grantor') or not grant.get('recorded_by') or not grant.get('justification')
                or grant.get('source_url') != photo.get('url') or not parse_time(grant.get('recorded_at'))):
            continue
        if grant.get('license') != photo.get('license_code', photo.get('license')):
            continue
        if scope == 'distribution' and grant.get('sha256') != photo.get('sha256'):
            continue
        if photo.get('license_code', photo.get('license')) == 'cc-by-nc':
            nc = p.get('noncommercial', {})
            if not nc.get('enabled') or p.get('distribution_mode') != 'noncommercial' or not nc.get('policy_document'):
                continue
            if not local_file(root, nc['policy_document']).is_file():
                continue
        if grant.get('expires_at') and (not parse_time(grant['expires_at']) or parse_time(grant['expires_at']) <= datetime.now(timezone.utc)):
            continue
        try:
            evidence = local_file(root, grant['evidence_path'])
            if evidence.is_file() and hashlib.sha256(evidence.read_bytes()).hexdigest() == grant.get('evidence_sha256'):
                return grant
        except (KeyError, ValueError):
            pass
    return None


def license_allowed(photo, root=ROOT, scope='download'):
    code = photo.get('license_code', photo.get('license'))
    # The maintainer cannot widen the allow-list to every CC license by accident.
    return code in DEFAULT_LICENSES or bool(documented_exception(photo, scope, root))


def observation_eligible(observation, photo, taxon_id=None):
    t = observation.get('taxon') or {}
    return (observation.get('quality_grade') == 'research' and t.get('rank') == 'species'
            and t.get('is_active') is True and observation.get('community_taxon_id') == t.get('id')
            and (taxon_id is None or str(t['id']) == str(taxon_id))
            and any(str(p.get('id')) == str(photo.get('id')) and p == photo for p in observation.get('photos', [])))


def sanitized_observation(observation, photo):
    """Whitelist; never archive entire API responses or observer/coordinate data."""
    t = observation.get('taxon') or {}
    return {'observation_id': str(observation.get('id', '')), 'community_taxon_id': str(observation.get('community_taxon_id', '')),
            'quality_grade': observation.get('quality_grade'),
            'taxon': {k: t.get(k) for k in ('id', 'name', 'rank', 'is_active', 'ancestor_ids')},
            'photo': {k: photo[k] for k in ('id', 'license_code', 'license_url', 'attribution', 'attribution_name',
                                         'title', 'url', 'hidden') if k in photo} | {'flags': bool(photo.get('flags'))}}


def archive_permission(root, observation, photo, origin='live_api', collected_at=None):
    if not any(str(p.get('id')) == str(photo.get('id')) for p in observation.get('photos', [])):
        raise ValueError('A fotografia não pertence à observação fornecida.')
    if origin == 'live_api' and not parse_time(collected_at):
        raise ValueError('Consulta nova exige data real de coleta da resposta.')
    snapshot = {'schema_version': 1, 'origin': origin, 'collected_at': collected_at, 'archived_at': now(),
                'source_url': 'https://api.inaturalist.org/v1/observations/' + str(observation['id']),
                'record': sanitized_observation(observation, photo)}
    relative = 'resources/compliance/permissions/' + digest(snapshot) + '.json'
    save(local_file(root, relative), snapshot)
    return relative


def records(root=ROOT, education=None, include_presentation=False):
    """Fingerprint source content, identities and evidence independently by topic."""
    folder = root / 'resources/data'
    species = load(folder / 'species.json', [])
    taxons = {n['id']: n for n in load(folder / 'taxonomy.json', [])}
    education = education if education is not None else load(folder / 'education.json', {})
    result = {}
    for key, note in education.get('notes', {}).items():
        # Pedagogical assistance is reviewed independently. Adding/regrading it
        # must not erase the scientific history of the original wording.
        value = {k: v for k, v in note.items() if k not in {'governance', 'assistance'}}
        result[key] = {'kind': 'claim', 'topic': 'conservation' if key.endswith(':conservation') else 'education',
                       'payload': value, 'source_url': note.get('source_url', ''), 'prior_state': 'verified' if note.get('verified') else 'draft'}
    reviews = load(folder / 'curation_reviews.json', {})
    provenance = load(folder / 'photo_provenance.json', {})
    for s in species:
        sid = str(s['id']); entry = education.get('species', {}).get(sid, {})
        photo = s.get('photo', {})
        for topic in TOPICS:
            if topic == 'taxonomy':
                # Original source assertions and the new synthesis have separate reviews.
                payload = {'name': s['name'], 'source_taxon_id': s.get('source_taxon_id'),
                           'educational_kingdom': s.get('legacy_educational_kingdom', s.get('educational_kingdom')),
                           'lineage': s.get('source_taxonomy', [taxons.get(n) for n in s.get('taxonomy', [])])}
            elif topic == 'photo':
                payload = {'name': s['name'], 'photo': photo, 'provenance': provenance.get(photo.get('id'), {})}
            else:
                keys = ('etymology_hints', 'name_origin') if topic == 'etymology' else ('morphological_hint', 'morphology_traits') if topic == 'morphology' else ('common_name', 'display_name', 'summary', 'distribution', 'nutrition', 'morphology', 'conservation', 'hints', 'curiosity')
                refs = {k: entry[k] for k in keys if k in entry}
                note_keys = set()
                def collect(value):
                    if isinstance(value, str) and value in result: note_keys.add(value)
                    elif isinstance(value, list):
                        for v in value: collect(v)
                    elif isinstance(value, dict):
                        for v in value.values(): collect(v)
                collect(refs)
                payload = {'name': s['name'], 'content': refs, 'notes': {k: result[k]['payload'] for k in sorted(note_keys)}}
                if topic == 'morphology': payload['photo_sha256'] = photo.get('sha256')
            result[f'topic:{sid}:{topic}'] = {'kind': 'topic', 'topic': topic, 'species_id': sid, 'payload': payload,
                                             'prior_state': reviews.get(sid, {}).get(topic, 'pending')}
        if 'classification' in s:
            result[f'topic:{sid}:phylogeny'] = {'kind': 'topic', 'topic': 'phylogeny', 'species_id': sid,
                'prior_state': 'draft', 'payload': {'name': s['name'], 'classification': s['classification'],
                    'lineage': [taxons.get(n) for n in s['taxonomy']], 'policy': load(folder / 'phylogeny.json', {})}}
    for s in species:
        sid = str(s['id']); entry = education.get('species', {}).get(sid, {})
        assessments = [('taxonomic', entry.get('taxonomic_assistance'), None)]
        assessments += [('etymological', education.get('notes', {}).get(ref, {}).get('assistance'), ref)
                        for ref in entry.get('etymology_hints', {}).values() if isinstance(ref, str)]
        ref = entry.get('morphological_hint')
        if isinstance(ref, str): assessments.append(('morphological', education.get('notes', {}).get(ref, {}).get('assistance'), ref))
        for category, assessment, note_key in assessments:
            if not isinstance(assessment, dict): continue
            key = 'assistance:' + (note_key if note_key else f'taxonomic:{sid}')
            source = result[note_key]['payload'] if note_key else result[f'topic:{sid}:taxonomy']['payload']
            result[key] = {'kind': 'assistance', 'topic': 'assistance', 'species_id': sid, 'note_key': note_key,
                           'category': category, 'prior_state': 'draft', 'payload': {
                               'scientific_name': s['name'], 'species_id': sid, 'category': category,
                               'source': source, 'photo': {'id': s.get('photo', {}).get('id'), 'sha256': s.get('photo', {}).get('sha256')},
                               'assessment': {k:v for k,v in assessment.items() if k not in {'governance','binding'}}}}
    if include_presentation:
        # Same review ledger/status rules; these presentation-only records do not
        # enter education generation or confer quiz eligibility.
        presentation = load(folder / 'exploration_photos.json', {})
        for pid, photo in presentation.get('photos', {}).items():
            result['presentation:'+pid] = {'kind':'presentation', 'topic':'photo', 'prior_state':'draft',
                'source_url':photo.get('photo_url',''), 'payload':{
                    'photo_id':pid, 'photo':{k:v for k,v in photo.items() if k!='scientific_review'},
                    'bindings':sorted(k for k,v in presentation.get('bindings',{}).items() if v==pid)}}
    # Admission dossiers use the same review ledger as published topics. They
    # are not approvals or a second registry. Bind both declarations and bytes
    # so replacing a source snapshot invalidates a prior editorial review.
    for s in species:
        sid = str(s['id']); dossier_path = root / f'resources/compliance/admissions/{sid}.json'
        if not dossier_path.is_file(): continue
        dossier = load(dossier_path); evidence_hashes = {}
        def bind_files(value):
            if isinstance(value, dict):
                if isinstance(value.get('path'), str):
                    path = local_file(root, value['path'])
                    evidence_hashes[value['path']] = hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None
                for child in value.values(): bind_files(child)
            elif isinstance(value, list):
                for child in value: bind_files(child)
        bind_files(dossier)
        result['admission:' + sid] = {'kind': 'admission', 'topic': 'licensing', 'species_id': sid,
            'prior_state': 'draft', 'payload': {'dossier': dossier, 'evidence_hashes': evidence_hashes,
                'content': {key: digest(value['payload']) for key, value in result.items()
                            if value.get('species_id') == sid or key.startswith('species:' + sid + ':')}}}
    for item in result.values(): item['content_sha256'] = digest(item['payload'])
    return result


def read_ledger(root=ROOT):
    return read_chain(root / 'resources/compliance/review_history.jsonl')


def event_errors(event, at=None):
    at = at or datetime.now(timezone.utc); errors = []
    when = parse_time(event.get('reviewed_at'))
    if not when or when > at: errors.append('review_date')
    if not event.get('reviewer') or not event.get('rationale'): errors.append('review_responsibility')
    if event.get('actor_type') != 'human': errors.append('review_requires_human_declaration')
    if event.get('state') not in APPROVALS | {'pending', 'rejected'}: errors.append('review_state')
    if event.get('state') in APPROVALS:
        if event.get('review_kind') not in {'editorial', 'specialist'}: errors.append('review_kind')
        if event['state'] == 'approved_expert' and (event.get('review_kind') != 'specialist' or not event.get('expertise')):
            errors.append('specialist_evidence')
        sources = event.get('evidence', [])
        if not sources or any(not https_url(e.get('url')) or not e.get('locator') or not e.get('supports') for e in sources):
            errors.append('review_evidence_locator')
        if event.get('claim_scope') not in {'photographic_observation', 'point_occurrence', 'known_distribution', 'species', 'group', 'nomenclature', 'taxonomy', 'hint_assistance'}:
            errors.append('claim_scope')
    return errors


def status(key, record, baseline, events, settings, at=None):
    at = at or datetime.now(timezone.utc)
    selected = next((e for e in reversed(events) if e.get('key') == key), None)
    prior = baseline.get('records', {}).get(key)
    state = 'draft'; reviewer = reviewed = kind = None; evidence = []; reason = 'Sem revisão humana documentada.'
    if selected:
        if event_errors(selected, at) or (record['kind']=='assistance' and selected.get('claim_scope')!='hint_assistance'):
            state = 'needs_review'; reason = 'Registro de revisão incompleto ou inválido.'
        elif selected.get('content_sha256') != record['content_sha256']:
            state = 'needs_review'; reason = 'Conteúdo, identidade ou evidência diferem da revisão registrada.'
        else:
            state = selected['state']; reviewer = selected['reviewer']; reviewed = selected['reviewed_at']
            kind = selected.get('review_kind'); evidence = selected.get('evidence', []); reason = selected['rationale']
    elif prior:
        if prior.get('content_sha256') == record['content_sha256']:
            state = 'legacy' if prior.get('prior_state') in {'approved', 'verified'} else prior.get('prior_state', 'pending')
            if state not in {'legacy', 'pending', 'rejected', 'draft', 'in_review'}: state = 'pending'
            reason = 'Registro anterior preservado; autor, data real e evidência de revisão não demonstrados.'
        else:
            state = 'needs_review'; reason = 'Conteúdo legado alterado; revisão humana necessária.'
    days = settings['review_days'].get(record['topic'], settings['review_days']['education'])
    due = (parse_time(reviewed) + timedelta(days=days)).isoformat() if reviewed else None
    if state in APPROVALS and parse_time(due) <= at:
        state = 'expired'; reason = 'Prazo configurado de revisão excedido.'
    allowed = state in APPROVALS or (state == 'legacy' and settings.get('allow_unchanged_legacy_for_local_learning'))
    return {'state': state, 'label': LABELS.get(state, 'Revisão pendente'), 'reviewer': reviewer, 'reviewed_at': reviewed,
            'review_kind': kind, 'due_at': due, 'reason': reason, 'evidence': evidence, 'content_sha256': record['content_sha256'],
            'claim_scope': selected.get('claim_scope') if selected else None, 'usable': bool(allowed),
            'limitation': 'Revisão editorial não equivale a validação por especialista. Fontes e testes não certificam exatidão.'}


def annotate_education(root, data):
    baseline = load(root / 'resources/compliance/review_baseline.json', {})
    events = read_ledger(root); settings = policy(root)
    all_records = records(root, data)
    for key, record in all_records.items():
        review = status(key, record, baseline, events, settings)
        if record['kind'] == 'claim':
            data['notes'][key]['governance'] = review
        elif record['kind'] == 'assistance':
            target = (data['notes'][record['note_key']]['assistance'] if record['note_key']
                      else data['species'][record['species_id']]['taxonomic_assistance'])
            target['governance'] = review
            target['binding'] = record['payload']
        else:
            data['species'][record['species_id']].setdefault('governance', {})[record['topic']] = review
    data['governance_policy'] = {'issue_url': settings['issue_url'], 'taxonomy_system': settings['taxonomy_system'],
                                 'educational_system': settings['educational_system']}
    return data


def migrate(root=ROOT):
    """One-time honest legacy migration; re-running never renews review dates."""
    with governance_lock(root):
        return _migrate(root)


def _migrate(root):
    require_governance_lock(root)
    folder = root / 'resources/data'; baseline_path = root / 'resources/compliance/review_baseline.json'
    if baseline_path.exists(): return load(baseline_path)
    if read_chain(root / 'resources/compliance/permission_history.jsonl'):
        raise ValueError('Permission events already exist; do not recreate a missing legacy baseline automatically')
    species = load(folder / 'species.json', []); needed = {s['photo']['id'] for s in species}; candidates = {}
    for path in sorted((root / 'build/import-cache').glob('*.json')):
        cached = load(path, {})
        for obs in cached.get('results', []):
            if 'quality_grade' not in obs: continue
            for photo in obs.get('photos', []):
                if str(photo.get('id')) in needed:
                    candidates[(str(obs['id']), str(photo['id']))] = (obs, photo)
    provenance = permission_provenance(root)
    for s in species:
        p = s['photo']; pid = p['id']
        if pid in provenance: continue
        oid = urlparse(p.get('source', '')).path.rstrip('/').split('/')[-1]
        matching = candidates.get((oid, pid)); evidence = []
        if matching:
            evidence.append(archive_permission(root, *matching, origin='legacy_cache', collected_at=None))
        # URL/version from old metadata remain assertions, not proof of collection.
        provenance[pid] = {'photo_id': pid, 'observation_id': oid, 'photo_url': 'https://www.inaturalist.org/photos/' + pid,
                           'observation_url': p.get('source'), 'download_url': p.get('url'), 'credit_as_supplied': p.get('author'),
                           'title': matching[1].get('title') if matching else None, 'license': p.get('license'),
                           'declared_license_url': p.get('license_url'), 'license_version': None,
                           'original_collected_at': None, 'evidence_paths': evidence, 'sha256': p.get('sha256'),
                           'modifications': p.get('modifications'), 'permission_state': 'legacy_evidence_incomplete',
                           'note': 'Cache antigo arquivado agora; data original e versão da licença não demonstradas. Não é uma nova coleta nem autorização humana.'}
    atomic_write(folder / 'photo_provenance.json',
                 (json.dumps(provenance, ensure_ascii=False, indent=2) + '\n').encode('utf8'))
    values = records(root)
    data = {'schema_version': 1, 'migrated_at': now(), 'purpose': 'Legacy snapshots, not approvals or scientific certification',
            'records': {key: {k: v for k, v in rec.items() if k != 'payload'} for key, rec in values.items()}}
    atomic_write(baseline_path, (json.dumps(data, ensure_ascii=False, indent=2) + '\n').encode('utf8'))
    return data


def permission_issues(photo, metadata, root=ROOT):
    errors = []
    if not license_allowed(dict(photo, url=metadata.get('download_url', photo.get('url'))), root, 'distribution'):
        errors.append('license_outside_policy')
    if not metadata.get('evidence_paths'): errors.append('permission_evidence_missing')
    if not parse_time(metadata.get('original_collected_at')) and not parse_time(metadata.get('first_evidenced_at')):
        errors.append('original_collection_date_unknown')
    if not metadata.get('license_version') or metadata.get('declared_license_url') != photo.get('license_url'):
        errors.append('license_version_not_evidenced')
    elif ('/' + metadata['license_version'] + '/') not in photo.get('license_url', ''):
        errors.append('license_version_url_mismatch')
    confirmation = metadata.get('permission_confirmation', {})
    bound = {'photo_id': photo.get('id'), 'sha256': photo.get('sha256'), 'license': photo.get('license'),
             'license_url': photo.get('license_url'), 'license_version': metadata.get('license_version'),
             'evidence_paths': metadata.get('evidence_paths', [])}
    reviewed = parse_time(confirmation.get('reviewed_at'))
    if (not confirmation.get('reviewer') or not confirmation.get('rationale') or not reviewed
            or confirmation.get('actor_type') != 'human' or reviewed > datetime.now(timezone.utc)
            or confirmation.get('content_sha256') != digest(bound)):
        errors.append('permission_confirmation_missing_or_changed')
    proof = confirmation.get('license_version_evidence', {})
    try:
        path = local_file(root, proof['path'])
        if not proof.get('locator') or not https_url(proof.get('source_url')) or not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != proof.get('sha256'):
            errors.append('license_version_evidence_invalid')
    except (KeyError, ValueError): errors.append('license_version_evidence_missing')
    if metadata.get('sha256') != photo.get('sha256'): errors.append('permission_content_changed')
    matched = False
    for relative in metadata.get('evidence_paths', []):
        try:
            evidence = load(local_file(root, relative), {})
            if not evidence or digest(evidence) != Path(relative).stem:
                errors.append('permission_evidence_integrity'); continue
            raw = evidence['record']; p = raw['photo']
            if str(p.get('id')) != str(photo.get('id')) or raw.get('observation_id') != metadata.get('observation_id'):
                errors.append('permission_identity'); continue
            if p.get('license_code') != photo.get('license'): errors.append('license_evidence_disagrees')
            if p.get('hidden') or p.get('flags'): errors.append('photo_hidden_or_flagged')
            # Source credit is authoritative, never the observer's identity.
            if photo.get('author') not in {p.get('attribution'), p.get('attribution_name')}:
                errors.append('credit_differs_from_source')
            matched = True
        except (KeyError, ValueError, TypeError): errors.append('permission_evidence_invalid')
    if metadata.get('evidence_paths') and not matched: errors.append('permission_not_demonstrated')
    if not photo.get('author') or photo.get('author') == 'no rights reserved': errors.append('credit_not_identified')
    if not photo.get('modifications'): errors.append('modifications_missing')
    return sorted(set(errors))


def source_findings(photo, metadata, root=ROOT):
    findings = []
    for relative in metadata.get('evidence_paths', []):
        evidence = load(local_file(root, relative), {}); record = evidence.get('record', {}); taxon = record.get('taxon', {})
        if record.get('quality_grade') != 'research': findings.append('legacy_observation_not_research_grade')
        if record.get('community_taxon_id') != str(taxon.get('id', '')): findings.append('community_taxon_disagrees')
        expected = photo.get('observation_taxon_id', photo.get('taxon_id'))
        if str(taxon.get('id', '')) != str(expected): findings.append('source_taxon_identity_requires_review')
    return sorted(set(findings))


def credits(root=ROOT):
    species = load(root / 'resources/data/species.json', []); provenance = load(root / 'resources/data/photo_provenance.json', {})
    recovery = permission_recovery(root)
    photos = {}
    for s in species:
        p = s['photo']; metadata = provenance.get(p['id'], {})
        record = photos.setdefault(p['id'], {'photo_id': p['id'], 'author': p.get('author'), 'title': metadata.get('title'),
             'photo_url': 'https://www.inaturalist.org/photos/' + p['id'], 'source': p.get('source'), 'download_url': p.get('url'),
             'license': p.get('license'), 'license_url': p.get('license_url'), 'license_version': metadata.get('license_version'),
             'modifications': p.get('modifications'), 'sha256': p.get('sha256'), 'organisms': [],
             'rights_issues': permission_issues(p, metadata, root), 'evidence_paths': metadata.get('evidence_paths', [])})
        record['distribution_exception'] = bool(documented_exception(p, 'distribution', root))
        if recovery['errors'] or p['id'] in recovery['pending_photo_ids']:
            record['rights_issues'].append('permission_projection_inconsistent')
        record['scientific_source_issues'] = source_findings(p, metadata, root)
        record['organisms'].append({'id': s['id'], 'name': s['name'], 'role': p.get('role', 'identification'), 'subject': p.get('scientific_name')})
    from exploration_media import credit_records
    return {'schema_version': 1, 'notice': 'Créditos mantidos também para CC0. Os créditos não implicam endosso dos autores, do iNaturalist ou do Wikimedia Commons. Uma referência não concede permissão de reprodução.', 'photos': photos, 'exploration_photos': credit_records(root)}


def render_credits(data):
    def safe(value): return str(value or '').replace('\n', ' ').replace('[', '\\[').replace(']', '\\]').replace('<', '&lt;')
    lines = ['# Créditos automáticos das fotografias', '', data['notice'], '',
             'Metadados legados podem ter pendências indicadas abaixo. Não é declaração de regularização. Títulos ausentes não são inventados.', '']
    for pid, p in sorted(data['photos'].items()):
        title = f" — título: {safe(p['title'])}" if p['title'] else ''
        names = ', '.join(safe(s['name']) for s in p['organisms'])
        lines += [f"- **{names}** — {safe(p['author']) or 'Autoria não identificada'}{title} — [foto {pid}]({p['photo_url']}) — [observação]({p['source']}) — [{safe(p['license']).upper()}]({p['license_url']}) — {safe(p['modifications'])}",
                  f"  SHA-256: `{p['sha256']}`. Pendências: {', '.join(p['rights_issues']) or 'nenhuma detectada automaticamente'}. Mais metadados: resources/data/photo_provenance.json."]
    lines += ['', '## Fotografias dos cartões de exploração', '',
              'Representantes visuais dos grupos; não substituem fotos de identificação do quiz nem revisão por especialista. Micrografias e alterações estão identificadas.', '']
    for key,p in sorted(data.get('exploration_photos',{}).items()):
        title=f" — título: {safe(p['title'])}" if p.get('title') else ''
        lines += [f"- **{safe(p['subject'])}** — {safe(p['author'])}{title} — [arquivo específico]({p['photo_url']}) — [{safe(p['license']).upper()} {safe(p['license_version'])}]({p['license_url']}) — {safe(p['modifications'])}",
                  f"  {safe(p.get('identification_note'))} SHA-256: `{p['sha256']}`. Coleta: {p['collected_at']}. Evidência: `{p['evidence_path']}` (revisão da fonte {p['source_revision_id']}). Pendências: {', '.join(p['rights_issues']) or 'nenhuma detectada automaticamente'}. Escopo: `resources/data/exploration_photos.json`."]
    return '\n'.join(lines) + '\n'


def generate_credits(root=ROOT):
    data = credits(root)
    save(root / 'resources/data/credits.json', data)
    (root / 'docs').mkdir(parents=True, exist_ok=True)
    (root / 'docs/PHOTO_CREDITS.md').write_text(render_credits(data), encoding='utf8', newline='\n')
    return data


def audit_governance(root=ROOT, at=None):
    at = at or datetime.now(timezone.utc); settings = policy(root)
    baseline = load(root / 'resources/compliance/review_baseline.json', {})
    from exploration_media import audit as audit_exploration
    from scientific_sources.audit import staging_errors
    errors = [{'error':'exploration_media', 'detail':finding} for finding in audit_exploration(root)['errors']]
    errors.extend(staging_errors(root))
    try: events = read_ledger(root)
    except ValueError as e: events = []; errors.append({'error': 'review_history_integrity', 'detail': str(e)})
    recovery = permission_recovery(root)
    if recovery['state'] != 'consistent':
        errors.append({'error': 'permission_projection_inconsistent', 'detail': recovery})
    for event in events:
        for error in event_errors(event, at): errors.append({'key': event.get('key'), 'error': error})
    current = records(root); scientific = []; assistance = []
    published = load(root / 'resources/data/education.json', {})
    for key, record in current.items():
        s = status(key, record, baseline, events, settings, at)
        emitted = (published.get('notes', {}).get(key, {}).get('governance', {}) if record['kind'] == 'claim'
                   else published.get('species', {}).get(record['species_id'], {}).get('governance', {}).get(record['topic'], {}))
        if record['kind'] == 'assistance':
            target = (published.get('notes', {}).get(record['note_key'], {}).get('assistance', {}) if record['note_key']
                      else published.get('species', {}).get(record['species_id'], {}).get('taxonomic_assistance', {}))
            emitted = target.get('governance', {})
            if target.get('binding') != record['payload']:
                errors.append({'error':'assistance_binding_stale','key':key})
        fields = ('content_sha256', 'reviewer', 'reviewed_at', 'evidence', 'due_at', 'claim_scope', 'review_kind')
        if any(emitted.get(field) != s.get(field) for field in fields) or (
                s['state'] != 'expired' and (emitted.get('state') != s['state'] or emitted.get('usable') != s['usable'] or emitted.get('label') != s['label'])):
            errors.append({'error': 'generated_review_metadata_stale', 'key': key})
        if s['state'] not in APPROVALS:
            (assistance if record['kind']=='assistance' else scientific).append({'key': key, 'state': s['state'], 'severity': 'high' if s['state'] in {'needs_review', 'rejected'} else 'medium',
                               'reason': s['reason'], 'content_sha256': record['content_sha256']})
    from review_hints import valid as valid_pedagogy
    automated = load(root/'resources/data/progressive_hints.json', {})
    if automated and (not valid_pedagogy(automated, root) or published.get('automated_pedagogy') != dict(automated, usable=True)):
        errors.append({'error':'automated_pedagogy_stale_or_expired'})
    attribution = credits(root)
    for photo in attribution['photos'].values():
        for issue in photo['scientific_source_issues']:
            scientific.append({'key': 'photo:' + photo['photo_id'], 'state': 'source_review_required', 'severity': 'high', 'reason': issue})
    education = load(root / 'resources/data/education.json', {})
    for name, expected in education.get('generated_source_sha256', {}).items():
        path = local_file(root, 'resources/data/' + name)
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            errors.append({'error': 'generated_education_stale', 'source': name})
    licensing = [{'photo_id': p['photo_id'], 'errors': p['rights_issues']} for p in attribution['photos'].values() if p['rights_issues']]
    assets = load(root / 'resources/compliance/asset_rights.json', {'assets': []})
    asset_errors = []
    for asset in assets.get('assets', []):
        path = local_file(root, asset['path'])
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != asset.get('sha256'):
            asset_errors.append({'path': asset['path'], 'error': 'asset_changed_or_missing'})
        grant = asset.get('permission', {})
        if not grant.get('holder') or not grant.get('evidence_path') or not grant.get('distribution_allowed'):
            asset_errors.append({'path': asset['path'], 'error': 'asset_permission_missing'})
        else:
            try:
                ev = local_file(root, grant['evidence_path'])
                if not ev.is_file() or hashlib.sha256(ev.read_bytes()).hexdigest() != grant.get('evidence_sha256'):
                    asset_errors.append({'path': asset['path'], 'error': 'asset_permission_evidence_invalid'})
            except ValueError: asset_errors.append({'path': asset['path'], 'error': 'asset_permission_path'})
    if not assets.get('assets'): asset_errors.append({'error': 'asset_inventory_missing'})
    permitted_paths = {s.get('photo', {}).get('path') for s in load(root / 'resources/data/species.json', [])}
    permitted_paths.update(a['path'] for a in assets.get('assets', []))
    qrc = root / 'resources.qrc'
    media_suffixes = {'.png', '.jpg', '.jpeg', '.ico', '.svg', '.ttf', '.otf', '.webp'}
    bundled = set()
    if qrc.exists():
        for item in ET.parse(qrc).getroot().iter('file'):
            name = item.text or ''
            if Path(name).suffix.lower() in media_suffixes:
                bundled.add(name)
                if name not in permitted_paths: asset_errors.append({'path': name, 'error': 'resource_permission_not_inventoried'})
    unbundled = [{'path': p.relative_to(root).as_posix(), 'sha256': hashlib.sha256(p.read_bytes()).hexdigest(),
                  'state': 'not_bundled_permission_not_demonstrated'}
                 for p in (root / 'resources').rglob('*') if p.is_file() and p.suffix.lower() in media_suffixes
                 and p.relative_to(root).as_posix() not in bundled and p.relative_to(root).as_posix() not in permitted_paths]
    if load(root / 'resources/data/credits.json', {}) != attribution: errors.append({'error': 'credits_out_of_date'})
    credit_document = root / 'docs/PHOTO_CREDITS.md'
    if not credit_document.is_file() or credit_document.read_text(encoding='utf8') != render_credits(attribution):
        errors.append({'error': 'attribution_document_out_of_date'})
    forbidden = {'latitude', 'longitude', 'coordinates', 'private_location', 'private_geojson', 'geojson', 'observer_email'}
    def private_keys(value):
        if isinstance(value, dict): return set(value).intersection(forbidden) | set().union(*(private_keys(v) for v in value.values()), set())
        if isinstance(value, list): return set().union(*(private_keys(v) for v in value), set())
        return set()
    paths = [root / 'resources/data' / n for n in ('species.json','species_profiles.json','education.json','photo_provenance.json')]
    paths.extend((root / 'resources/compliance/permissions').glob('*.json'))
    for path in paths:
        found = private_keys(load(path, {}))
        if found: errors.append({'error': 'unnecessary_private_fields', 'file': path.relative_to(root).as_posix(), 'fields': sorted(found)})
    for path in (root / 'resources').rglob('*'):
        if path.is_file() and path.suffix.lower() in {'.jpg', '.jpeg', '.png'}:
            findings = image_privacy_findings(path)
            if findings: errors.append({'error': 'embedded_sensitive_metadata_requires_review', 'file': path.relative_to(root).as_posix(), 'fields': findings})
    return {'assurances': assurances(), 'technical_errors': errors, 'photo_licensing': licensing, 'asset_licensing': asset_errors, 'unbundled_media': unbundled,
            'scientific_findings': scientific, 'assistance_findings': assistance,
            'automated_pedagogy': {'state': automated.get('evaluation',{}).get('state'), 'usable': bool(automated and valid_pedagogy(automated, root)), 'scope':'Pedagogy only; human scientific reviews unchanged.'},
            'review_states': dict(__import__('collections').Counter(s['state'] for s in scientific)),
            'distribution_blocked': bool(errors or licensing or asset_errors),
            'repository_distribution_blocked': bool(errors or licensing or asset_errors or unbundled),
            'scope': 'Automated evidence/integrity checks, not legal or scientific certification.'}


def append_review(root, key, decision):
    with governance_lock(root):
        return _append_review(root, key, decision)


def _append_review(root, key, decision):
    require_governance_lock(root)
    current = records(root, include_presentation=True)
    if key not in current: raise ValueError('Tópico/afirmação não existe.')
    if current[key]['kind']=='assistance' and decision.get('claim_scope')!='hint_assistance':
        raise ValueError('A avaliação pedagógica exige escopo hint_assistance; revisão científica não aprova o peso da dica.')
    if event_errors(decision): raise ValueError('Revisão exige responsável humano, data real, escopo, justificativa e evidência localizada.')
    if decision.get('content_sha256') != current[key]['content_sha256']:
        raise ValueError('A decisão não corresponde ao hash atual; não aprovar conteúdo alterado.')
    events = read_ledger(root)
    return append_event(root, 'review_history.jsonl', dict(decision, key=key, recorded_at=now()),
                        events[-1]['event_sha256'] if events else '')


def _permission_plan(root):
    """Read only. A projection may advance from a known state, never an unknown one."""
    path = root / 'resources/data/photo_provenance.json'
    history_path = root / 'resources/compliance/permission_history.jsonl'
    errors = []; pending = []; history = []; provenance = {}
    try:
        history = read_chain(history_path)
        provenance = load(path, {})
        if not isinstance(provenance, dict):
            raise ValueError('Projection is not an object')
        if history and not path.is_file():
            raise ValueError('Missing projection: unrelated provenance cannot be reconstructed')
        states = {}; latest = {}; operations = set()
        for event in history:
            pid = event.get('photo_id')
            before, after = event.get('before'), event.get('after')
            if not isinstance(pid, str) or not isinstance(before, dict) or not isinstance(after, dict):
                raise ValueError('Incomplete permission event')
            if after.get('photo_id') != pid or after.get('permission_state') != 'documented_human_confirmation':
                raise ValueError('Incompatible permission event identity/state')
            if pid in latest and before != latest[pid]:
                raise ValueError('Divergent transitions for photo ' + pid)
            operation = event.get('operation_sha256')
            if operation and (operation != _permission_operation(pid, after, event.get('curation')) or operation in operations):
                raise ValueError('Invalid or duplicate permission operation')
            if operation: operations.add(operation)
            states.setdefault(pid, [before]).append(after)
            latest[pid] = after
        for pid, current in provenance.items():
            if (current.get('permission_confirmation') or current.get('permission_state') == 'documented_human_confirmation') and pid not in latest:
                raise ValueError('Confirmation without a permission event for photo ' + pid)
        photos = {str(s['photo']['id']): s['photo'] for s in load(root / 'resources/data/species.json', [])}
        projected = copy.deepcopy(provenance)
        for pid, after in latest.items():
            current = provenance.get(pid, {})
            if current not in states[pid]:
                raise ValueError('Unknown projection state for photo ' + pid + '; human reconciliation required')
            if current != after:
                issues = permission_issues(photos[pid], after, root) if pid in photos else ['photo_not_in_catalog']
                if issues:
                    raise ValueError('Recovery evidence invalid for photo ' + pid + ': ' + ', '.join(issues))
                projected[pid] = copy.deepcopy(after)
                pending.append(pid)
        return provenance, projected, history, pending, errors
    except (ValueError, TypeError, KeyError, AttributeError, OSError) as error:
        errors.append(str(error))
        return provenance, provenance, history, [], errors


def _permission_operation(pid, after, curation=None):
    identity = {'photo_id': pid, 'confirmation': after.get('permission_confirmation')}
    if curation: identity['curation'] = curation
    return digest(identity)


def _recover_permissions(root, apply=False):
    if apply: require_governance_lock(root)
    provenance, projected, history, pending, errors = _permission_plan(root)
    report = {'state': 'blocked' if errors else 'recovery_required' if pending else 'consistent',
              'event_count': len(history), 'pending_photo_ids': pending, 'errors': errors, 'applied': False}
    if apply and not errors and pending:
        atomic_write(root / 'resources/data/photo_provenance.json',
                     (json.dumps(projected, ensure_ascii=False, indent=2) + '\n').encode('utf8'))
        report.update(state='consistent', applied=True, pending_photo_ids=[])
    return report


def permission_recovery(root=ROOT, apply=False):
    """Audit by default; applying replays existing evidence-bound events only."""
    if not apply: return _recover_permissions(root)
    with governance_lock(root):
        return _recover_permissions(root, apply=True)


def permission_provenance(root=ROOT):
    provenance, _, _, pending, errors = _permission_plan(root)
    if errors or pending:
        raise ValueError('Permission state requires recovery or human reconciliation: ' + '; '.join(errors or pending))
    return provenance


def register_photo_provenance(root, pid, metadata):
    """Importer may add provenance, never overwrite an existing human decision."""
    with governance_lock(root):
        report = _recover_permissions(root, apply=True)
        if report['errors']: raise ValueError(str(report))
        provenance = permission_provenance(root)
        if pid in provenance and provenance[pid] != metadata:
            raise ValueError('Existing photo provenance differs; explicit human reconciliation required')
        if pid not in provenance:
            provenance[pid] = metadata
            atomic_write(root / 'resources/data/photo_provenance.json',
                         (json.dumps(provenance, ensure_ascii=False, indent=2) + '\n').encode('utf8'))


def confirm_permission(root, decision):
    """Explicit human input. Commit the event first; materialization is replayable."""
    with governance_lock(root):
        return _confirm_permission(root, decision)


def _confirm_permission(root, decision):
    """Caller must hold governance_lock, including the Issue decision bridge."""
    require_governance_lock(root)
    report = _recover_permissions(root, apply=True)
    if report['errors']: raise ValueError(str(report))
    species = load(root / 'resources/data/species.json', [])
    pid = str(decision.get('photo_id')); photo = next((s['photo'] for s in species if s['photo']['id'] == pid), None)
    if not photo: raise ValueError('Fotografia ausente do catálogo.')
    path = root / 'resources/data/photo_provenance.json'; provenance = permission_provenance(root)
    current = provenance.get(pid, {}); updated = copy.deepcopy(current)
    updated['license_version'] = decision.get('license_version')
    updated['evidence_paths'] = sorted(set(current.get('evidence_paths', []) + decision.get('evidence_paths', [])))
    fresh_dates = []
    for relative in updated['evidence_paths']:
        evidence = load(local_file(root, relative), {})
        if evidence.get('origin') == 'live_api' and parse_time(evidence.get('collected_at')):
            fresh_dates.append(evidence['collected_at'])
    if fresh_dates: updated['first_evidenced_at'] = min(fresh_dates)
    binding = {'photo_id': pid, 'sha256': photo.get('sha256'), 'license': photo.get('license'),
               'license_url': photo.get('license_url'), 'license_version': updated['license_version'], 'evidence_paths': updated['evidence_paths']}
    updated['permission_confirmation'] = {k: decision.get(k) for k in ('reviewer', 'actor_type', 'reviewed_at', 'rationale', 'license_version_evidence')}
    if decision.get('content_sha256') != digest(binding):
        raise ValueError('Decisão deve vincular foto, bytes, licença, versão e todas as evidências pelo hash atual.')
    updated['permission_confirmation']['content_sha256'] = decision['content_sha256']
    if permission_issues(photo, updated, root):
        raise ValueError('Evidência ainda insuficiente: ' + ', '.join(permission_issues(photo, updated, root)))
    updated['permission_state'] = 'documented_human_confirmation'
    history_path = root / 'resources/compliance/permission_history.jsonl'
    history = read_chain(history_path)
    operation = _permission_operation(pid, updated, decision.get('curation'))
    existing = next((e for e in history if _permission_operation(e['photo_id'], e['after'], e.get('curation')) == operation), None)
    if existing:
        latest = next(e for e in reversed(history) if e['photo_id'] == pid)
        if latest['event_sha256'] != existing['event_sha256']:
            raise ValueError('Decision superseded by a later human decision; no rollback permitted')
        if current != existing['after']:
            raise ValueError('Decision effects differ from the materialized projection')
        return current
    event = {'photo_id': pid, 'recorded_at': now(), 'before': current, 'after': updated,
             'operation_sha256': operation}
    if decision.get('curation'): event['curation'] = decision['curation']
    append_event(root, 'permission_history.jsonl', event, history[-1]['event_sha256'] if history else '')
    recovery = _recover_permissions(root, apply=True)
    if recovery['state'] != 'consistent': raise ValueError(str(recovery))
    return updated


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['migrate', 'credits', 'audit', 'audit-history', 'record-review', 'confirm-permission', 'recover-permissions'])
    parser.add_argument('--base', help='For audit-history only: full, independently selected Git commit ID')
    parser.add_argument('--key'); parser.add_argument('--decision', type=Path)
    parser.add_argument('--workspace', type=Path, help='Isolated admission draft under build/scientific-sources/drafts; default is the project')
    parser.add_argument('--apply', action='store_true', help='Replay existing permission events; never create a decision')
    args = parser.parse_args()
    work_root = ROOT
    if args.workspace:
        from scientific_sources.admission import workspace
        try: work_root = workspace(ROOT, args.workspace)
        except ValueError as error: parser.error(str(error))
        if args.action in {'migrate', 'audit-history'}:
            parser.error('Legacy baselines and Git-history audits are not created in admission drafts')
    if args.base is not None and args.action != 'audit-history': parser.error('--base requires audit-history')
    if args.action == 'audit-history':
        report = audit_history(work_root, args.base)
        save(work_root / 'build/governance-history.json', report)
        print(json.dumps(report, ensure_ascii=False))
        raise SystemExit(0 if report['consistent'] else 2)
    elif args.action == 'migrate':
        print('Legacy baseline records:', len(migrate(work_root)['records']))
    elif args.action == 'credits': print('Unique photo credits:', len(generate_credits(work_root)['photos']))
    elif args.action == 'record-review':
        if not args.key or not args.decision: parser.error('--key e --decision são necessários')
        print(append_review(work_root, args.key, load(args.decision))['event_sha256'])
    elif args.action == 'confirm-permission':
        if not args.decision: parser.error('--decision é necessário')
        print(confirm_permission(work_root, load(args.decision))['permission_state'])
    elif args.action == 'recover-permissions':
        report = permission_recovery(work_root, apply=args.apply)
        print(json.dumps(report, ensure_ascii=False))
        raise SystemExit(0 if report['state'] == 'consistent' else 2)
    else:
        report = audit_governance(work_root); save(work_root / 'build/governance-audit.json', report)
        print(json.dumps({k: v for k, v in report.items() if k not in {'scientific_findings', 'photo_licensing'}}, ensure_ascii=False))
