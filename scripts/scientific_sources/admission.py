"""Offline admission proposals using existing evidence, reviews and permissions.

No function writes the active catalog, creates a review or confirms permission.
Only a proposed delta is exported; integration remains a reviewed Git change.
"""
import hashlib
import json
from pathlib import Path
import re
import uuid

import content_governance as g
from curation_requests import private_data
from governance_storage import LOCK_RELATIVE
from .model import Assertion, Identity, digest, name_key
from .reconcile import reconcile, external_identity

DRAFTS = Path('build/scientific-sources/drafts')
BASE_INPUTS = ('resources/data/species.json', 'resources/data/taxonomy.json',
               'resources/data/catalog_manifest.json', 'resources/data/phylogeny.json',
               'resources/compliance/review_history.jsonl', 'resources/compliance/permission_history.jsonl')


def file_hash(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


def base_inputs(root):
    from release import catalog_inputs
    return {**{name: file_hash(root / name) for name in BASE_INPUTS},
            'catalog_inputs_sha256': digest(catalog_inputs(root))}


def workspace(root, value):
    root = Path(root).resolve()
    expected_base = root / DRAFTS
    if expected_base.resolve() != expected_base:
        raise ValueError('Draft base cannot be redirected through a filesystem link')
    target = (Path(root) / value).resolve()
    target.relative_to(expected_base)
    if target == expected_base: raise ValueError('Select a dedicated draft directory')
    return target


def pointer(value, location):
    """Small RFC 6901 reader; provider evidence must locate actual ID/name fields."""
    if not isinstance(location, str) or not location.startswith('/'): raise ValueError('JSON pointer required')
    for token in location[1:].split('/'):
        token = token.replace('~1', '/').replace('~0', '~')
        value = value[int(token)] if isinstance(value, list) else value[token]
    return value


def template(root, draft, candidate):
    draft = workspace(root, draft)
    path = draft / 'admission-template.json'
    body = {'schema_version': 1, 'candidate': candidate, 'candidate_sha256': digest(candidate),
            'base_inputs': base_inputs(root), 'species_id': None, 'provider_evidence': [],
            'source_use': {layer: {'license': None, 'citation': None, 'compatibility_rationale': None,
                                  'scopes': [], 'evidence': {'path': None, 'sha256': None, 'source_url': None, 'locator': None}}
                           for layer in ('taxonomy_data', 'educational_text')},
            'unavailable_content': {},
            'notice': 'Template only. Complete a standard data workspace and copy this dossier to resources/compliance/admissions/<id>.json. No approval generated.'}
    if path.exists() and g.load(path) != body: raise ValueError('Existing template differs; use a new draft, never overwrite')
    if not path.exists(): g.save(path, body)
    return path


def assess(root, sid, existing, taxons=None):
    """Validate a proposed organism, excluding it from existing during active audit.

    Old baseline entries retain their existing policy. New entries cannot inherit
    legacy approval. Lack of a hint is acceptable when explicitly documented.
    """
    root = Path(root); sid = str(sid); errors = []; reviews = {}; evidence = []
    def fail(code, **context): errors.append({'error': code, **context})
    def proof(item):
        if not isinstance(item, dict): raise ValueError('Evidence object required')
        path = g.local_file(root, item['path'])
        if not path.is_file() or file_hash(path) != item.get('sha256'): raise ValueError('Evidence bytes missing or changed')
        if not g.https_url(item.get('source_url')) or not item.get('locator'): raise ValueError('Evidence origin/locator required')
        if path.suffix == '.json' and private_data(g.load(path)): raise ValueError('Private evidence refused')
        evidence.append({'path': item['path'], 'sha256': item['sha256']})
        return path
    try:
        if not re.fullmatch(r'[1-9][0-9]*', sid): raise ValueError('Permanent numeric organism ID required')
        species = g.load(root / 'resources/data/species.json', [])
        selected = [s for s in species if str(s.get('id')) == sid]
        if len(selected) != 1: raise ValueError('Exactly one matching organism required')
        s = selected[0]
        if type(s.get('id')) is not int: fail('organism_id_must_be_integer')
        dossier = g.load(root / f'resources/compliance/admissions/{sid}.json', {})
        if dossier.get('schema_version') != 1 or str(dossier.get('species_id')) != sid:
            raise ValueError('Admission dossier missing or mismatched')
        if private_data(dossier): raise ValueError('Private dossier refused')
        candidate = dossier['candidate']; assertion = Assertion.from_dict(candidate['assertion'])
        expected_id = 'candidate:' + str(uuid.uuid5(uuid.NAMESPACE_URL, assertion.identity.key))
        if (digest(candidate) != dossier.get('candidate_sha256') or candidate.get('candidate_id') != expected_id
                or candidate.get('scientific_state') != 'pending' or candidate.get('legal_state') != 'pending'):
            fail('candidate_integrity_or_state')
        if assertion.kind != 'taxon' or assertion.rank != 'species' or assertion.status != 'accepted':
            fail('accepted_species_assertion_required')
        if assertion.accepted and assertion.accepted != assertion.identity: fail('resolve_accepted_taxon_before_admission')
        if not assertion.identity.provider or not assertion.identity.record_id: fail('external_identity_missing')
        if name_key(s['name']) != name_key(assertion.canonical_name): fail('scientific_identity_diverges')
        identities = [external_identity(value) for value in s.get('external_ids', [])]
        if s.get('source') and s.get('source_taxon_id'):
            identities.append(Identity(s['source'], str(s['source_taxon_id'])))
        if assertion.identity not in identities: fail('candidate_external_identity_not_preserved')
        reserved = {str(row['id']) for row in existing}
        replacements = g.load(root / 'resources/data/catalog_replacements.json', {}).get('replacements', [])
        for row in replacements:
            reserved.add(str(row.get('old', {}).get('id'))); reserved.add(str(row.get('new_id')))
        if sid in reserved: fail('organism_id_already_used')
        comparison = reconcile([assertion], existing, taxons or {})['records'][assertion.identity.key]
        if comparison['relationship'] != 'unmatched': fail('possible_duplicate_requires_resolution', comparison=comparison)
        if not assertion.provenance or not dossier.get('provider_evidence'): fail('taxonomy_evidence_missing')
        proven = set()
        for item in dossier.get('provider_evidence', []):
            path = proof(item); snapshot = g.load(path); when = g.parse_time(snapshot.get('retrieved_at'))
            binding = {'snapshot_sha256': digest(snapshot), 'url': snapshot.get('url'),
                       'retrieved_at': snapshot.get('retrieved_at'), 'response_sha256': snapshot.get('response_sha256')}
            if (not when or when > g.parse_time(g.now()) or digest(snapshot) != path.stem
                    # The provider transport preserves a minimized payload; the
                    # hash of the original HTTP bytes cannot be recomputed from
                    # that subset. Its snapshot hash and provenance are binding.
                    or not re.fullmatch(r'[0-9a-f]{64}', str(snapshot.get('response_sha256', '')))
                    or snapshot.get('url') != item['source_url'] or binding not in assertion.provenance):
                fail('provider_snapshot_invalid')
            payload = snapshot['payload']
            if str(pointer(payload, item['identity_locator'])) != assertion.identity.record_id:
                fail('provider_identity_diverges')
            if name_key(str(pointer(payload, item['name_locator']))) != name_key(assertion.canonical_name):
                fail('provider_name_diverges')
            proven.add(digest(binding))
        if {digest(p) for p in assertion.provenance} - proven: fail('provider_provenance_not_archived')
        for layer in ('taxonomy_data', 'educational_text'):
            use = dossier['source_use'][layer]; proof(use['evidence'])
            if (not use.get('license') or use['license'].lower() in {'unknown', 'all-rights-reserved'}
                    or not use.get('citation') or not use.get('compatibility_rationale')
                    or not {'offline_catalog', 'public_repository'}.issubset(use.get('scopes', []))):
                fail('data_text_redistribution_not_documented', layer=layer)
        # Reuse the current phylogenetic projection without changing any source.
        import phylogeny
        policy = g.load(root / 'resources/data/phylogeny.json', {})
        nodes = g.load(root / 'resources/data/taxonomy.json', [])
        projected, projected_nodes = phylogeny.project([s], nodes, policy)
        structural = phylogeny.audit(projected, projected_nodes, policy)
        if projected[0] != s or structural['errors']: fail('taxonomy_structural_inconsistency')
        index = {n['id']: n for n in nodes}
        if any(index.get(n['id']) != n for n in projected_nodes): fail('taxonomy_node_diverges')
        # Initial quiz-media adapter deliberately retains the existing iNaturalist
        # contract; Commons presentation media have a separate audited adapter.
        photo = s['photo']; pid = str(photo['id'])
        path = g.local_file(root, photo['path'])
        if not path.is_file() or file_hash(path) != photo.get('sha256'): fail('photo_bytes_missing_or_changed')
        elif g.image_privacy_findings(path): fail('photo_private_metadata')
        if name_key(photo.get('scientific_name', '')) != name_key(s['name']): fail('photo_scientific_identity_diverges')
        if not photo.get('author'): fail('photo_credit_missing')
        if not photo.get('license'): fail('photo_license_missing')
        meta = g.permission_provenance(root).get(pid, {})
        for code in g.permission_issues(photo, meta, root): fail(code)
        if g.permission_recovery(root)['state'] != 'consistent': fail('permission_projection_inconsistent')
        eligible = False
        for relative in meta.get('evidence_paths', []):
            snap = g.load(g.local_file(root, relative), {}); record = snap.get('record', {}); t = record.get('taxon', {})
            if str(record.get('photo', {}).get('id')) != pid: continue
            if str(t.get('id')) != str(photo.get('taxon_id')): fail('photo_taxon_diverges')
            if name_key(t.get('name', '')) != name_key(s['name']): fail('photo_source_name_diverges')
            if (record.get('quality_grade') == 'research' and record.get('community_taxon_id') == str(t.get('id'))
                    and str(t.get('id')) == str(photo.get('taxon_id')) and t.get('is_active') is True
                    and t.get('rank') == 'species' and snap.get('origin') == 'live_api'
                    and g.parse_time(snap.get('collected_at'))): eligible = True
            else: fail('observation_does_not_meet_import_policy')
        if not eligible: fail('eligible_photo_evidence_missing')
        education = g.load(root / 'resources/data/education.json', {})
        from build_education import build
        if build(root, persist=False) != education: fail('education_not_regenerated_from_editable_sources')
        entry = education.get('species', {}).get(sid, {}); notes = education.get('notes', {})
        if entry.get('scientific_name') != s['name'] or not entry.get('common_name'): fail('educational_identity_incomplete')
        for topic in ('summary', 'distribution', 'morphology', 'conservation', 'curiosity'):
            if entry.get(topic) not in notes: fail('educational_topic_missing', topic=topic)
        refs = set()
        def collect(value):
            if isinstance(value, str) and value in notes: refs.add(value)
            elif isinstance(value, list):
                for child in value: collect(child)
            elif isinstance(value, dict):
                for key, child in value.items():
                    if key not in {'governance', 'phylogeny', 'review'}: collect(child)
        collect(entry)
        for ref in refs:
            note = notes[ref]
            if not note.get('text') or not note.get('source_title') or not g.https_url(note.get('source_url')):
                fail('educational_source_missing', key=ref)
        unavailable = dossier.get('unavailable_content', {})
        for key in ('etymology_hints', 'morphological_hint', 'taxonomic_assistance', 'hints'):
            if not entry.get(key) and not unavailable.get(key): fail('missing_content_not_explained', key=key)
        current = g.records(root); events = g.read_ledger(root); settings = g.policy(root)
        required = {f'topic:{sid}:{t}' for t in ('taxonomy', 'photo', 'morphology', 'education', 'phylogeny')}
        if entry.get('etymology_hints') or entry.get('name_origin'): required.add(f'topic:{sid}:etymology')
        required |= refs | {'admission:' + sid}
        required |= {key for key, value in current.items() if value.get('kind') == 'assistance' and value.get('species_id') == sid}
        for key in sorted(required):
            if key not in current: fail('review_record_missing', key=key); continue
            review = g.status(key, current[key], {}, events, settings)  # Never inherit a legacy exemption for a new organism.
            reviews[key] = review
            if review['state'] not in g.APPROVALS: fail('human_review_required', key=key, state=review['state'])
        for key, value in current.items():
            if value.get('kind') == 'assistance' and value.get('species_id') == sid:
                a = value['payload']['assessment']
                if (a.get('grade') not in {'light', 'moderate', 'strong'} or not a.get('rationale') or not a.get('ranks')
                        or a.get('context') not in {'safe_broad_ancestor', 'any_safe_options', 'diagnostic_contrast'}):
                    fail('hint_assistance_incomplete', key=key)
    except (ValueError, KeyError, TypeError, OSError, IndexError, AttributeError) as error:
        fail('admission_incomplete_or_invalid', reason=str(error))
    return {'schema_version': 1, 'species_id': sid, 'ready_for_proposal': not errors, 'errors': errors,
            'reviews': reviews, 'evidence': evidence, 'approvals_created': 0, 'active_catalog_modified': False,
            'notice': 'Technical readiness only; human declarations are not authenticated identities or legal/scientific certification.'}


def prepare(root, draft, sid):
    root = Path(root); draft = workspace(root, draft); sid = str(sid)
    if not re.fullmatch(r'[1-9][0-9]*', sid): raise ValueError('Permanent numeric organism ID required')
    # Same synchronization as direct CLI and bridge decisions. The order is
    # always active root then its draft; no admission command reverses it.
    with g.governance_lock(root), g.governance_lock(draft):
        return _prepare(root, draft, sid)


def draft_files(draft):
    result = {}
    for path in (draft / 'resources').rglob('*'):
        if not path.is_file(): continue
        name = path.relative_to(draft).as_posix()
        if name == LOCK_RELATIVE: continue
        if path.is_symlink() or re.search(r'\.(?:sqlite\d*|db)(?:-|$)|\.part$', name):
            raise ValueError('Operational/private artifact in draft resources')
        path.resolve().relative_to(draft.resolve())
        result[name] = file_hash(path)
    return result


def _prepare(root, draft, sid):
    before = draft_files(draft)
    dossier = g.load(draft / f'resources/compliance/admissions/{sid}.json', {})
    if dossier.get('base_inputs') != base_inputs(root): raise ValueError('Active base changed; reconcile the draft and reviews explicitly')
    # Draft ledgers must extend, never replace/rechain, the current histories.
    for name in ('review_history.jsonl', 'permission_history.jsonl'):
        old = g.read_chain(root / 'resources/compliance' / name)
        new = g.read_chain(draft / 'resources/compliance' / name)
        if new[:len(old)] != old: raise ValueError('Draft does not preserve the existing event prefix')
    existing = g.load(root / 'resources/data/species.json', [])
    result = assess(draft, sid, existing, {n['id']: n for n in g.load(root / 'resources/data/taxonomy.json', [])})
    if not result['ready_for_proposal']: return result
    proposed = next(s for s in g.load(draft / 'resources/data/species.json') if str(s['id']) == sid)
    # The delta records all inputs, not just a mutable "approved" boolean.
    files = draft_files(draft)
    if files != before or dossier['base_inputs'] != base_inputs(root):
        raise ValueError('Draft/base changed during validation; retry only after reconciliation')
    body = {'schema_version': 1, 'state': 'candidates_only', 'operation': 'proposed_catalog_admission',
            'species_id': sid, 'base_inputs': base_inputs(root), 'organism': proposed, 'workspace_files': files,
            'review_head': g.read_ledger(draft)[-1]['event_sha256'], 'validation': result,
            'preserve_existing_ids': [row['id'] for row in existing],
            'workspace_is_full_catalog_replacement': False,
            'integration': 'Explicit Git change/PR; preserve existing IDs, evidence and event prefixes; regenerate derived content and run all gates.',
            'approvals_created': 0, 'active_catalog_modified': False}
    destination = root / 'build/scientific-sources/admissions' / (digest(body) + '.json')
    if destination.exists() and g.load(destination) != body: raise ValueError('Proposal hash collision or corruption')
    if not destination.exists():
        g.atomic_write(destination, (json.dumps(body, ensure_ascii=False, indent=2) + '\n').encode('utf8'))
    return dict(result, proposal=destination.relative_to(root).as_posix())


def new_entry_errors(root):
    """Close unmarked manual-copy bypasses, without changing legacy approvals."""
    baseline = g.load(root / 'resources/compliance/review_baseline.json', {}).get('records', {})
    # Synthetic/minimal repositories without a migration baseline are audited by
    # their ordinary validators; the shipped project has a fixed 122-entry base.
    if not baseline: return []
    species = g.load(root / 'resources/data/species.json', []); errors = []
    nodes = {n['id']: n for n in g.load(root / 'resources/data/taxonomy.json', [])}
    for s in species:
        sid = str(s['id'])
        if 'topic:' + sid + ':taxonomy' in baseline: continue
        result = assess(root, sid, [row for row in species if str(row['id']) != sid], nodes)
        errors.extend(dict(error, species_id=sid) for error in result['errors'])
    return errors
