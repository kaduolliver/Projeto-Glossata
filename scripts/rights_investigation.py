"""Investigation proposals from existing governance/gates, never decisions or grants.

All writes are confined to the existing build/update-proposals staging area.
Remote metadata is photo-specific, minimized and kept separate from published data.
"""
import copy
from collections import Counter
import hashlib
import http.client
import json
from pathlib import Path
import re
import subprocess
import urllib.error
from urllib.parse import urlsplit
import xml.etree.ElementTree as ET

import content_governance as g
import distribution
from curation_requests import private_data, request_id
from maintenance_http import PublicClient, UnsafeDestination
from review_updates import changes_for_photo

WORK = Path('build/update-proposals')
EVIDENCE = WORK / 'rights-evidence'
QUERIES = WORK / 'rights-queries'
FOCUS = ('707161720', '659755043')
MEDIA = {'.jpg', '.jpeg', '.png', '.ico', '.svg', '.webp', '.ttf', '.otf'}
INPUTS = ('resources/data/species.json', 'resources/data/photo_provenance.json',
          'resources/data/catalog_manifest.json', 'resources/data/exploration_photos.json',
          'resources/compliance/asset_rights.json', 'resources.qrc')


def file_hash(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


def observation_id(photo):
    url = urlsplit(photo.get('source') or '')
    match = re.fullmatch(r'/observations/([1-9][0-9]*)/?', url.path)
    return match[1] if url.scheme == 'https' and url.hostname in {'www.inaturalist.org', 'inaturalist.org'} and match else None


def staged(root):
    """Only intact, dated, public live snapshots; never interpret approval fields."""
    found = {}; errors = []
    # The committed investigation is an archive, not a new consultation. Replay
    # its minimized snapshots when the ignored build cache is absent.
    archive = root / 'docs/SANEAMENTO_DIREITOS.json'
    archived = g.load(archive, {}).get('photos', [])
    entries = [(path, None) for path in sorted((root / EVIDENCE).glob('*.json'))]
    entries += [(Path(row['public_proposal']['path']), row['public_proposal'])
                for row in archived if row.get('kind') == 'quiz_photo' and row.get('public_proposal')]
    for path, previous in entries:
        try:
            data = previous['snapshot'] if previous else g.load(path)
            date = g.parse_time(data.get('collected_at'))
            if (path.is_symlink() or g.digest(data) != path.stem or data.get('origin') != 'live_api'
                    or not date or date > g.parse_time(g.now()) or private_data(data)):
                raise ValueError('Invalid staged evidence')
            if previous:
                if path.parent.as_posix() != EVIDENCE.as_posix():
                    raise ValueError('Unexpected archived evidence path')
                raw = (json.dumps(data, ensure_ascii=False, indent=2) + '\n').encode('utf8')
                if hashlib.sha256(raw).hexdigest() != previous['sha256']:
                    raise ValueError('Invalid archived snapshot bytes')
            pid = data['expected_photo_id']; oid = data['record']['observation_id']
            if not re.fullmatch(r'[0-9]+', pid) or not re.fullmatch(r'[0-9]+', oid): raise ValueError('Invalid identity')
            entry = dict(previous, archive_source='docs/SANEAMENTO_DIREITOS.json',
                         archive_locator='photos[photo_id=' + pid + '].public_proposal') if previous else {
                             'snapshot': data, 'path': path.relative_to(root).as_posix(), 'sha256': file_hash(path)}
            if pid not in found or date > g.parse_time(found[pid]['snapshot']['collected_at']): found[pid] = entry
            elif date == g.parse_time(found[pid]['snapshot']['collected_at']):
                if data != found[pid]['snapshot']: raise ValueError('Conflicting snapshots at the same consultation time')
                if previous: found[pid] = entry
        except (ValueError, KeyError, TypeError, OSError):
            errors.append({'path': str(path) if previous else path.relative_to(root).as_posix(), 'error': 'staged_evidence_invalid'})
    return found, errors


def public_history_media(root, tracked):
    """Path/object inventory reachable from fetched origin refs, not local branches.

    This is a Git reachability inventory, not proof of publication date, removal
    from caches or a forensic inventory of every version of currently used paths.
    """
    try:
        result = subprocess.run(['git', '-C', str(root), 'rev-list', '--objects', '--remotes=origin'],
                                capture_output=True, timeout=30, check=True)
        refs = subprocess.run(['git', '-C', str(root), 'for-each-ref', '--format=%(refname)', 'refs/remotes/origin'],
                              capture_output=True, timeout=15, check=True)
        if not refs.stdout.strip() or tracked is None: raise ValueError('No fetched public refs')
        rows = []
        for line in result.stdout.decode('utf8').splitlines():
            oid, _, name = line.partition(' ')
            if name and Path(name).suffix.lower() in MEDIA and name not in tracked:
                rows.append({'path': name, 'git_object_id': oid, 'scope': 'history_only_in_fetched_origin',
                             'priority': 1, 'state': 'pending',
                             'actions': ['Assess provenance and public exposure; no deletion or history rewrite authorized.']})
        return {'assessed': True, 'refs': refs.stdout.decode('utf8').splitlines(), 'resources': rows}
    except (OSError, ValueError, UnicodeError, subprocess.SubprocessError):
        return {'assessed': False, 'refs': [], 'resources': [], 'reason': 'Git/origin inventory unavailable; not a zero finding.'}


def scientific_context(species, proposal):
    if not proposal: return {'state': 'insufficient_evidence', 'approval_created': False}
    snapshot = proposal['snapshot']; record = snapshot['record']; taxon = record.get('taxon', {})
    photo = species['photo']; context = snapshot.get('observation_context', {})
    association = snapshot.get('photo_present') and str(record.get('photo', {}).get('id')) == str(photo['id'])
    taxon_matches = str(taxon.get('id')) == str(photo.get('observation_taxon_id', photo.get('taxon_id')))
    community_matches = str(record.get('community_taxon_id')) == str(taxon.get('id'))
    return {'quality_grade': record.get('quality_grade'), 'captive': context.get('captive'),
            'wild_disagreement_count': sum(m.get('metric') == 'wild' and m.get('agree') is False
                                          for m in context.get('quality_metrics', [])),
            'photo_association_matches': bool(association), 'taxon_id_matches': taxon_matches,
            'community_taxon_matches': community_matches, 'source_name': taxon.get('name'),
            'research_grade_policy_met': record.get('quality_grade') == 'research',
            'incorrect_identification_established': False,
            'limitation': 'Casual/captivity is not proof of incorrect identification. Matching source IDs is not independent visual validation.',
            'approval_created': False}


def screenshot_inventory(root, locate, quiz):
    rows = []
    for name in ('tela-inicial', 'quiz'):
        relative = 'docs/images/' + name + '.png'; metadata = g.load(root / ('docs/images/' + name + '.json'), {})
        if not (root / relative).is_file(): continue
        pid = str(metadata.get('photo', {}).get('photo_id', ''))
        linked = next((p for p in quiz if p['photo_id'] == pid), None)
        rows.append({'path': relative, 'scope': locate(relative), 'priority': 0 if linked and linked['rights_issues'] else 1,
                     'capture_record': 'docs/images/' + name + '.json', 'captured_at': metadata.get('captured_at'),
                     'sha256': file_hash(root / relative),
                     'capture_hash_matches': file_hash(root / relative) == metadata.get('image_sha256'),
                     'explicit_photo_id': pid or None, 'linked_rights_issues': linked['rights_issues'] if linked else [],
                     'state': 'pending', 'limitation': 'Capture provenance does not grant rights in depicted media. Inputs alone do not establish which photographs are visible.',
                     'actions': ['Prioritize publicly displayed derived media; document depicted photo/branding rights or propose replacement/suspension for maintainer decision.']})
    return rows


def documentary_summary(report):
    """Resource register without duplicating the already archived API snapshots."""
    value = copy.deepcopy(report)
    value['report_kind'] = 'catalog_documentary_readiness'
    for row in value['photos']:
        proposal = row.get('public_proposal')
        if not proposal: continue
        snapshot = proposal.pop('snapshot')
        proposal['collected_at'] = snapshot['collected_at']
        proposal['snapshot_content_sha256'] = g.digest(snapshot)
        proposal['notice'] = 'Read the exact snapshot in archive_source/archive_locator or the original staging path; no new consultation.'
    return value


def public_review(review):
    value = {key: review.get(key) for key in ('state', 'review_kind', 'reviewed_at', 'content_sha256')}
    value['reviewer'] = review.get('reviewer') if not private_data(review.get('reviewer')) else None
    value['private_details_withheld'] = private_data(review)
    return value


def collect_public(root, species, maximum=25, selected=(), refresh=False, client=None):
    """Batched public observation GETs only. No HTML scraping, media or credentials."""
    if type(maximum) is not int or not 1 <= maximum <= 120: raise ValueError('Use 1..120 bounded API requests')
    photos = {str(s['photo']['id']): s for s in species}
    if set(selected) - photos.keys(): raise ValueError('Unknown requested photo ID')
    cached, _ = staged(root); groups = {}
    for pid in sorted(photos, key=lambda p: (p not in FOCUS, p)):
        if selected and pid not in selected: continue
        oid = observation_id(photos[pid]['photo'])
        previous = cached.get(pid, {}).get('snapshot', {})
        if not refresh and previous.get('record', {}).get('observation_id') == oid: continue
        if oid: groups.setdefault(oid, []).append(pid)
    client = client or PublicClient(); calls = []; ids = list(groups)
    batches = [ids[start:start + 10] for start in range(0, len(ids), 10)]
    while batches and len(calls) < maximum:
        batch = batches.pop(0)
        url = 'https://api.inaturalist.org/v1/observations/' + ','.join(batch)
        call = {'requested_at': g.now(), 'request_url': url, 'observation_ids': batch, 'evidence': [], 'state': 'pending'}
        try:
            try:
                response = client.get(url, as_json=True)
            except UnsafeDestination:
                call['state'] = 'destination_refused'; calls.append(call)
                g.save(root / QUERIES / (g.digest(call) + '.json'), call); break
            except (ValueError, http.client.HTTPException):
                # Keep the transport byte/JSON limits. Smaller batches still count
                # against the same request budget and use the client's rate limit.
                call['state'] = 'response_rejected_by_transport'
                if len(batch) > 1:
                    middle = len(batch) // 2; batches[:0] = [batch[:middle], batch[middle:]]
                calls.append(call); g.save(root / QUERIES / (g.digest(call) + '.json'), call); continue
            when = g.now()
            results = response.get('results', [])
            if not isinstance(results, list): raise ValueError('Invalid results')
            incoming = {str(item.get('id')): item for item in results}
            if len(incoming) != len(results) or set(incoming) - set(batch): raise ValueError('Contradictory response identity')
            for oid in batch:
                if oid not in incoming:
                    call.setdefault('missing_observations', []).append(oid); continue
                observation = incoming[oid]
                for pid in groups[oid]:
                    matches = [p for p in observation.get('photos', []) if str(p.get('id')) == pid]
                    if len(matches) > 1: raise ValueError('Ambiguous photo identity')
                    photo = matches[0] if matches else {}
                    record = g.sanitized_observation(observation, photo)
                    # Public observation flags describe this record, never an entire species.
                    context = {'captive': observation.get('captive') if type(observation.get('captive')) is bool else None,
                        'quality_metrics': [{'metric': m['metric'], 'agree': m['agree']} for m in observation.get('quality_metrics', [])
                            if isinstance(m, dict) and re.fullmatch(r'[a-z_]{1,80}', str(m.get('metric', ''))) and type(m.get('agree')) is bool]}
                    snapshot = {'schema_version': 1, 'origin': 'live_api', 'collected_at': when,
                        'request_url': url, 'expected_photo_id': pid, 'photo_present': bool(matches),
                        'record': record, 'observation_context': context,
                        'notice': 'Public source metadata only; no permission, review or identification approved.'}
                    if private_data(snapshot):
                        call.setdefault('withheld_photo_ids', []).append(pid); continue
                    path = root / EVIDENCE / (g.digest(snapshot) + '.json')
                    g.save(path, snapshot)
                    call['evidence'].append({'photo_id': pid, 'path': path.relative_to(root).as_posix(), 'sha256': file_hash(path)})
            call['state'] = 'public_metadata_collected'
        except urllib.error.HTTPError as error:
            call.update(state='http_error', http_status=error.code)
            # Python 3.11 leaves the wrapper uninitialized for fp=None;
            # newer versions allocate an empty response that must be closed.
            if error.fp is not None: error.close()
            # Stop rather than evade a rate limit, restriction or unavailable service.
            if error.code in {401, 403, 429} or error.code >= 500:
                calls.append(call); g.save(root / QUERIES / (g.digest(call) + '.json'), call); break
        except Exception as error:
            call.update(state='collection_failed', error_type=type(error).__name__)
        calls.append(call); g.save(root / QUERIES / (g.digest(call) + '.json'), call)
    return calls


def surfaces(root, package):
    try:
        result = subprocess.run(['git', '-C', str(root), 'ls-files', '-z'], capture_output=True, timeout=15, check=True)
        tracked = set(result.stdout.decode('utf8').split('\0')) - {''}
    except (OSError, ValueError, subprocess.SubprocessError): tracked = None
    qrc = root / 'resources.qrc'
    bundled = {e.text for e in ET.parse(qrc).getroot().iter('file')} if qrc.exists() else set()
    inventory = g.load(package / 'compliance/dependencies.json', {}) if package else {}
    loose = {}
    for entry in inventory.get('binaries', []):
        path = g.local_file(package, entry['path'])
        if path.is_file() and file_hash(path) == entry.get('sha256'):
            loose.setdefault(entry['sha256'], []).append(entry['path'])
    def locate(relative):
        path = g.local_file(root, relative)
        return {'in_checkout': path.is_file(), 'tracked_in_current_git': relative in tracked if tracked is not None else None,
                'qrc_build_input': relative in bundled,
                'package_loose_same_bytes_paths': sorted(loose.get(file_hash(path), [])),
                'package_embedded_presence': 'not_verified' if package and relative in bundled else 'not_assessed',
                'notice': 'Source QRC membership is not proof of bytes embedded in an existing executable.'}
    return locate, tracked, inventory


def required_actions(issues, scientific=()):
    actions = []
    if any('credit' in s or 'attribution' in s for s in issues): actions.append('Compare the photo-specific credit; do not substitute the observer for the photographer.')
    if issues: actions.append('Document the file-specific grant/version, attribution and human rationale through the existing permission workflow.')
    if scientific: actions.append('Review the photo/observation/taxon association and community identification; retain the scientific block pending a legitimate decision.')
    actions.append('No approval, contact, removal or redistribution is performed by this report.')
    return actions


def editorial_policy_errors(root=g.ROOT):
    """An administrative default is neither a whole-corpus grant nor a review."""
    policy = g.policy(root)
    editorial = policy.get('editorial_text_policy')
    if editorial is None:
        # Compatible with older inventories, but not a dangling new decision.
        return (['editorial_default_missing'] if any(d.get('kind') == 'editorial_default_only'
            for d in policy.get('maintainer_decisions', [])) else [])
    expected = {'default_license': 'CC-BY-4.0',
        'license_url': 'https://creativecommons.org/licenses/by/4.0/',
        'scope': 'eligible_original_educational_expression_only',
        'automatic_grants': False, 'per_material_evidence_required': True,
        'document': 'docs/propostas/LICENCIAMENTO_TEXTOS.md'}
    if not isinstance(editorial, dict) or any(editorial.get(k) != v for k, v in expected.items()):
        return ['editorial_default_invalid_or_overbroad']
    errors = []
    if policy.get('own_material_licenses', {}).get('texts') is not None:
        errors.append('editorial_default_is_not_a_corpus_grant')
    decisions = policy.get('maintainer_decisions', [])
    matches = [d for d in decisions if d.get('id') == editorial.get('decision_id')]
    if (len(matches) != 1 or matches[0].get('license') != editorial['default_license']
            or matches[0].get('kind') != 'editorial_default_only'
            or matches[0].get('scope') != editorial['document']
            or not matches[0].get('authority_basis') or not matches[0].get('recorded_on')):
        errors.append('editorial_default_decision_not_linked')
    return errors


def profile_batch_binding_issues(root, record):
    """Check an explicit partial grant against its immutable confirmed proposal.

    This verifies declared scope and bytes, never identity, ownership or consent.
    Other evidence formats retain their existing validation.
    """
    try:
        for ref in record.get('evidence', []):
            proof = g.load(g.local_file(root, ref['path']), {})
            if proof.get('kind') != 'educational_profile_batch_confirmation': continue
            changes = proof['proposed_changes']; confirmation = proof['license_confirmation']
            recorded = g.parse_time(confirmation.get('recorded_on'))
            if (not isinstance(changes, list) or not changes
                    or g.digest(changes) != proof.get('proposal_fingerprint')
                    or proof.get('file') != 'resources/data/species_profiles.json'
                    or not confirmation.get('declared_by') or not confirmation.get('statement')
                    or not confirmation.get('scope') or not recorded or recorded > g.parse_time(g.now())
                    or confirmation.get('license') != 'CC-BY-4.0'
                    or record.get('declared_license') != confirmation['license']
                    or confirmation.get('license_url') != 'https://creativecommons.org/licenses/by/4.0/'):
                raise ValueError('Invalid confirmed batch')
            expected = set()
            for change in changes:
                note = change['proposed_note']
                if (g.digest(note) != change['proposed_note_sha256']
                        or hashlib.sha256(note['text'].encode('utf8')).hexdigest() != change['text_sha256']):
                    raise ValueError('Changed proposal')
                expected.add((proof['file'], change['pointer'] + '/text', change['text_sha256']))
            occurrence_set = lambda rows: {tuple(o[k] for k in ('file', 'pointer', 'text_sha256')) for o in rows}
            bound = record.get('occurrences', [record])
            if occurrence_set(proof['occurrences']) != expected or not occurrence_set(bound) <= expected:
                raise ValueError('Grant extends beyond confirmed wordings')
    except (KeyError, TypeError, ValueError, OSError, AttributeError):
        return ['profile_batch_grant_binding_invalid']
    return []


def educational_rights_metadata(root, passages):
    """Read optional documentary records in the existing asset inventory.

    Bind each occurrence, not just a shared text hash. These records never grant
    permission, replace review events or change the distribution authority.
    """
    records = g.load(root / 'resources/compliance/asset_rights.json', {}).get('educational_text_rights', [])
    errors = []; rows = []; seen = set()
    bindings = {(o['file'], o['pointer'], p['text_sha256']) for p in passages for o in p['occurrences']}
    if not isinstance(records, list):
        return {'technical_errors': ['text_rights_records_not_a_list'], 'records': [], 'grants_created': 0}
    origins = {'original', 'external_reproduction', 'adaptation', 'translation',
               'independent_factual_synthesis', 'inconclusive'}
    allowed = {'file', 'pointer', 'text_sha256', 'occurrences', 'origin_kind', 'declared_origin', 'source_text',
               'transformations', 'rights_available', 'declared_license', 'evidence',
               'investigation_state', 'editorial_review_key', 'attribution'}
    for i, record in enumerate(records):
        label = 'text_rights:' + str(i)
        if not isinstance(record, dict) or set(record) - allowed:
            errors.append(label + ':invalid_fields'); continue
        keys = ('file', 'pointer', 'text_sha256')
        grouped = 'occurrences' in record
        occurrences = record.get('occurrences') if grouped else [{k: record.get(k) for k in keys}]
        if (not isinstance(occurrences, list) or not occurrences
                or (grouped and any(k in record for k in keys))):
            errors.append(label + ':invalid_binding'); continue
        bound = []
        for occurrence in occurrences:
            if not isinstance(occurrence, dict) or set(occurrence) != set(keys):
                errors.append(label + ':invalid_binding'); continue
            binding = tuple(occurrence[k] for k in keys)
            if not all(isinstance(v, str) for v in binding):
                errors.append(label + ':invalid_binding'); continue
            if binding not in bindings: errors.append(label + ':stale_or_unknown_occurrence')
            if binding in seen: errors.append(label + ':duplicate_occurrence')
            seen.add(binding); bound.append(occurrence)
        if not isinstance(record.get('origin_kind'), str) or record['origin_kind'] not in origins:
            errors.append(label + ':invalid_origin_kind')
        source = record.get('source_text') or {}
        transforms = record.get('transformations', [])
        evidence = record.get('evidence', [])
        if (not isinstance(source, dict) or not isinstance(transforms, list)
                or any(not isinstance(t, str) or t not in {'translation', 'adaptation'} for t in transforms)
                or not isinstance(evidence, list)):
            errors.append(label + ':invalid_metadata'); continue
        if (not isinstance(record.get('investigation_state'), str) or record['investigation_state'] not in
                {'pending_human_review', 'documented', 'inconclusive', 'rejected', 'suspended'}):
            errors.append(label + ':invalid_investigation_state')
        for item in evidence:
            try:
                if (not isinstance(item, dict) or not item.get('sha256')
                        or file_hash(g.local_file(root, item['path'])) != item['sha256']):
                    raise ValueError('Evidence does not match')
            except (KeyError, TypeError, ValueError, OSError):
                errors.append(label + ':invalid_evidence')
        issues = profile_batch_binding_issues(root, record)
        if record.get('origin_kind') == 'inconclusive': issues.append('origin_inconclusive')
        if not evidence or not record.get('rights_available'): issues.append('rights_evidence_incomplete')
        declared_license = record.get('declared_license')
        if not declared_license: issues.append('passage_license_undetermined')
        elif not isinstance(declared_license, str): errors.append(label + ':invalid_declared_license')
        attribution = record.get('attribution') or {}
        if not isinstance(attribution, dict):
            errors.append(label + ':invalid_attribution')
        elif isinstance(declared_license, str) and (not attribution.get('license_url') or
                (declared_license.upper().startswith('CC-BY') and not attribution.get('author_credit')
                 and attribution.get('credit_not_supplied_by_source') is not True)):
            issues.append('passage_attribution_incomplete')
        if record.get('origin_kind') in ('external_reproduction', 'adaptation', 'translation') and not source.get('url'):
            issues.append('textual_source_missing')
        source_license = source.get('license') or ''
        if not isinstance(source_license, str):
            errors.append(label + ':invalid_source_license'); continue
        if record.get('origin_kind') in ('external_reproduction', 'adaptation', 'translation') and not source_license:
            issues.append('textual_source_license_missing')
        # Compare common CC labels without rewriting the recorded license/version.
        conditions = set(source_license.upper().replace(' ', '-').split('-'))
        if 'SA' in conditions and record.get('declared_license') == 'CC-BY-4.0':
            issues.append('sharealike_cannot_be_replaced_by_default')
        if 'NC' in conditions: issues.append('noncommercial_conditions_require_review')
        if 'ND' in conditions and (transforms or record.get('origin_kind') in ('translation', 'adaptation')):
            issues.append('no_derivatives_conditions_require_review')
        row = {'declared_metadata': {k: v for k, v in record.items() if k != 'occurrences'},
            'rights_issues': issues, 'permission_created': False}
        if grouped: row['bindings'] = bound
        elif bound: row['binding'] = bound[0]
        rows.append(row)
    return {'technical_errors': errors, 'records': rows, 'grants_created': 0}


def text_permission_binding_issues(root, asset, grant):
    """Bind a scoped batch grant to its exact bytes, not a whole corpus.

    Checks consistency of a declared confirmation, not human authentication or
    legal ownership. Other, independently documented grant formats stay intact.
    """
    try:
        proof = g.load(g.local_file(root, grant.get('evidence_path', '')), {})
        if not isinstance(proof, dict):
            return ['text_grant_binding_invalid']
        if (proof.get('kind') != 'educational_group_batch_confirmation'
                and grant.get('scope') != 'eligible_group_wording_only'):
            return []
        confirmation = proof.get('license_confirmation', {})
        if not isinstance(confirmation, dict):
            return ['text_grant_binding_invalid']
        date = g.parse_time(confirmation.get('recorded_on'))
        if (grant.get('scope') != 'eligible_group_wording_only'
                or proof.get('kind') != 'educational_group_batch_confirmation'
                or proof.get('file') != asset['path']
                or proof.get('file_sha256') != asset['sha256']
                or not confirmation.get('declared_by') or not confirmation.get('statement')
                or not date or date > g.parse_time(g.now())
                or confirmation.get('license') != 'CC-BY-4.0'
                or grant.get('license') != confirmation['license']
                or not confirmation.get('scope')
                or confirmation.get('license_url') != 'https://creativecommons.org/licenses/by/4.0/'):
            return ['text_grant_binding_invalid']
    except (KeyError, TypeError, ValueError, OSError):
        return ['text_grant_binding_invalid']
    return []


def text_resource_issues(inventory, path):
    """A file grant cannot cover only one component of a mixed prose resource.

    Full coverage and valid evidence are necessary, never sufficient to create
    permission. The explicit resource grant remains a separate requirement.
    """
    metadata = inventory['rights_metadata']
    expected = {(o['file'], o['pointer'], p['text_sha256']) for p in inventory['passages']
                for o in p['occurrences'] if o['file'] == path}
    covered = set(); pending = False
    for row in metadata['records']:
        for binding in row.get('bindings', [row.get('binding', {})]):
            if binding.get('file') != path: continue
            covered.add(tuple(binding.get(k) for k in ('file', 'pointer', 'text_sha256')))
            pending |= bool(row['rights_issues']) or row['declared_metadata'].get('investigation_state') != 'documented'
    issues = []
    if not expected or covered != expected: issues.append('text_component_coverage_incomplete')
    if pending: issues.append('text_component_rights_pending')
    if metadata['technical_errors']: issues.append('text_component_metadata_invalid')
    return issues


def educational_text_inventory(root=g.ROOT):
    """Deduplicate exact prose and retain every source binding; never decide rights.

    The existing rights report owns this inventory. No corpus, review date or
    permission is written, and different citations of identical text stay distinct.
    """
    names = ('group_education', 'species_profiles', 'hint_education', 'progressive_hints')
    data = {n: g.load(root / ('resources/data/' + n + '.json'), {}) for n in names}
    passages = {}; counts = {n: 0 for n in names}

    def add(name, pointer, text, source=None, role='educational_assertion', derived_from=None):
        if not isinstance(text, str) or not text.strip(): return
        key = hashlib.sha256(text.encode('utf8')).hexdigest()
        row = passages.setdefault(key, {'text_sha256': key, 'characters': len(text),
            'originality_state': 'inconclusive', 'permission_created': False, 'occurrences': []})
        source = source or {}
        occurrence = {'file': 'resources/data/' + name + '.json', 'pointer': pointer,
            'role': role, 'source_url': source.get('source_url'), 'source_locator': source.get('locator')}
        if derived_from: occurrence['derived_from'] = derived_from
        row['occurrences'].append(occurrence); counts[name] += 1

    definitions = data['group_education'].get('definitions', [])
    by_group = {d[0]: {'source_url': d[2]} for d in definitions}
    for i, definition in enumerate(definitions):
        for j in (4, 5, 6):
            add('group_education', '/definitions/' + str(i) + '/' + str(j), definition[j], by_group[definition[0]])
    for name, notes in data['group_education'].get('plain_notes', {}).items():
        for j, text in enumerate(notes):
            add('group_education', '/plain_notes/' + name + '/' + str(j), text, by_group.get(name))

    def walk(name, value, pointer='', source=None):
        if isinstance(value, dict):
            source = value if value.get('source_url') else source
            for key, child in sorted(value.items()):
                part = pointer + '/' + key.replace('~', '~0').replace('/', '~1')
                if key in {'text', 'rationale', 'prefix'}:
                    add(name, part, child, source,
                        'pedagogical_rationale' if key == 'rationale' else 'educational_assertion')
                else: walk(name, child, part, source)
        elif isinstance(value, list):
            for i, child in enumerate(value): walk(name, child, pointer + '/' + str(i), source)

    for name in ('species_profiles', 'hint_education'):
        walk(name, data[name])
    for sid, item in data['progressive_hints'].get('species', {}).items():
        for i, recipe in enumerate(item.get('recipes', [])):
            pointer = '/species/' + sid + '/recipes/' + str(i)
            origin = recipe.get('source_note') or ('taxonomy.json / ' + recipe.get('node', {}).get('id', ''))
            for field in ('text', 'source_text', 'rationale'):
                add('progressive_hints', pointer + '/' + field, recipe.get(field), recipe,
                    'generated_pedagogical_rationale' if field == 'rationale' else 'generated_copy', origin)
    for field in ('rationale', 'limitations'):
        add('progressive_hints', '/evaluation/' + field, data['progressive_hints'].get('evaluation', {}).get(field),
            role='automated_evaluation_metadata')
    projection = {'state': 'not_present', 'matches_generator': None}
    if data['progressive_hints']:
        try:
            import review_hints
            current = {k: v for k, v in data['progressive_hints'].items() if k != 'evaluation'}
            projection = {'state': 'compared_without_evaluation', 'matches_generator': current == review_hints.prepare(root)}
        except (KeyError, TypeError, ValueError, OSError):
            projection = {'state': 'source_incomplete_or_invalid', 'matches_generator': False}
    return {'scope': 'All nonempty educational text/rationale/prefix fields in four sources, including unused group definitions; no semantic originality certification.',
        'files': {n + '.json': {'sha256': file_hash(root / ('resources/data/' + n + '.json')),
                               'passage_occurrences': counts[n]} for n in names},
        'definition_count': len(definitions), 'plain_note_sets': len(data['group_education'].get('plain_notes', {})),
        'profile_count': len(data['species_profiles']), 'hint_identity_count': len(data['hint_education']),
        'generated_identity_count': len(data['progressive_hints'].get('species', {})),
        'unique_passages': len(passages), 'total_occurrences': sum(counts.values()),
        'projection': projection, 'decisions_created': 0, 'external_consultations': 0,
        'editorial_default': g.policy(root).get('editorial_text_policy'),
        'rights_metadata': educational_rights_metadata(root, list(passages.values())),
        'passages': [passages[k] for k in sorted(passages)]}


def other_materials_resolution(materials, inventory, technical_errors=()):
    """Summarize the existing material gate; never creates decisions or closes Issues.

    Generated copies keep their source bindings. Counts are review work, not
    independent works or evidence that permission has been established.
    """
    rows = []; unresolved = set(); roles = Counter()
    for material in materials:
        path = material['path']; documented = set(); issues = set(material.get('issues', []))
        expected = {(o['file'], o['pointer'], p['text_sha256']) for p in inventory['passages']
                    for o in p['occurrences'] if o['file'] == path}
        for row in inventory['rights_metadata']['records']:
            metadata = row['declared_metadata']
            for binding in row.get('bindings', [row.get('binding', {})]):
                if binding.get('file') != path: continue
                pending = bool(row['rights_issues']) or metadata.get('investigation_state') != 'documented'
                if not pending:
                    key = tuple(binding[k] for k in ('file', 'pointer', 'text_sha256'))
                    documented.add(key)
        remaining = expected - documented; unresolved.update(remaining)
        bindings = [dict(zip(('file', 'pointer', 'text_sha256'), b)) for b in sorted(remaining)]
        if expected: issues.update(text_resource_issues(inventory, path))
        rows.append({'path': path, 'resource_issues': sorted(issues),
                     'pending_occurrences': bindings, 'pending_count': len(bindings),
                     'resource_ready_for_closure_review': not issues and not bindings,
                     'note': 'No whole-resource permission is inferred from partial or shared wording.'})
    for passage in inventory['passages']:
        for occurrence in passage['occurrences']:
            if (occurrence['file'], occurrence['pointer'], passage['text_sha256']) in unresolved:
                roles[occurrence['role']] += 1
    errors = list(technical_errors) + inventory['rights_metadata']['technical_errors']
    pending_resources = [r['path'] for r in rows if not r['resource_ready_for_closure_review']]
    return {'scope': 'Issue licensing / other-materials only; excludes photo, science and dependency decisions.',
            'ready_for_maintainer_closure_review': bool(rows) and not pending_resources and not errors,
            'automatic_closure': False, 'decisions_created': 0, 'permissions_created': 0,
            'pending_resources': pending_resources, 'technical_errors': errors,
            'pending_occurrences': len(unresolved),
            'pending_unique_texts': len({b[2] for b in unresolved}), 'pending_by_role': dict(roles),
            'materials': rows,
            'required_next_steps': ['Resolve authorial source and available rights per current binding; replace only when justified.',
                                    'Record actual decisions through the existing governance bridge; no inherited human approval.',
                                    'Regenerate derived content and rerun the authoritative asset gate.',
                                    'Review the integrated main commit before explicitly closing the Issue.'],
            'limitations': 'A closure summary, closed Issue or tests never authorize a resource or approve the package.'}


def report(root=g.ROOT, package=None, online=False, maximum=25, selected=(), refresh=False, client=None, at=None):
    root = Path(root); package = Path(package) if package else None
    source = distribution.release_gate(root)  # Existing authority, never replaced by proposal findings.
    package_gate = distribution.release_gate(root, package) if package else None
    species = g.load(root / 'resources/data/species.json', [])
    calls = collect_public(root, species, maximum, selected, refresh, client) if online else []
    cached, cache_errors = staged(root)
    provenance = g.load(root / 'resources/data/photo_provenance.json', {})
    credits = g.credits(root); locate, tracked, inventory = surfaces(root, package)
    current = g.records(root, include_presentation=True)
    baseline = g.load(root / 'resources/compliance/review_baseline.json', {})
    try: reviews = g.read_ledger(root)
    except ValueError: reviews = []  # Existing gate reports integrity failure, never infer a review.
    rows = []; known = set()
    for pid, credit in sorted(credits['photos'].items()):
        s = next(s for s in species if str(s['photo']['id']) == pid); photo = s['photo']; meta = provenance.get(pid, {})
        known.update(x['photo']['path'] for x in species if str(x['photo']['id']) == pid)
        rights = list(credit['rights_issues']); science = list(credit['scientific_source_issues'])
        if not photo.get('license'): rights.append('license_missing')
        if not photo.get('author'): rights.append('attribution_incomplete')
        if meta.get('license') and meta['license'] != photo.get('license'): rights.append('provenance_license_disagrees')
        if meta.get('credit_as_supplied') and meta['credit_as_supplied'] != photo.get('author'): rights.append('provenance_credit_disagrees')
        latest = cached.get(pid); proposal = None; source_changes = []
        if latest:
            snapshot = latest['snapshot']; record = snapshot['record']; p = record.get('photo', {})
            if record.get('observation_id') != observation_id(photo):
                science.append('staged_observation_identity_disagrees')
            else:
                proposal = dict(latest)
                community = record.get('community_taxon_id', '')
                incoming = dict(record, community_taxon_id=int(community) if str(community).isdigit() else None,
                                photos=[p] if snapshot.get('photo_present') else [])
                source_changes, _ = changes_for_photo(s, incoming)
                science.extend('current_' + c for c in source_changes
                               if c in {'possible_identification_or_taxonomy_change', 'taxon_not_active'})
                if not snapshot.get('photo_present') or str(p.get('id')) != pid: science.append('photo_not_associated_in_current_response')
                if p.get('license_code') != photo.get('license'): rights.append('current_license_changed_requires_analysis_not_automatic_revocation')
                if not p.get('license_code'): rights.append('current_license_missing')
                elif not g.license_allowed(dict(photo, license=p['license_code'], url=p.get('url', photo.get('url'))), root, 'distribution'):
                    rights.append('current_license_requires_policy_review')
                if photo.get('author') not in {p.get('attribution'), p.get('attribution_name')}: rights.append('current_credit_differs_from_source')
                if p.get('hidden') or p.get('flags'): rights.append('current_photo_hidden_or_flagged')
                if record.get('quality_grade') != 'research': science.append('current_observation_not_research_grade')
                t = record.get('taxon', {})
                if str(record.get('community_taxon_id')) != str(t.get('id')): science.append('current_community_taxon_disagrees')
                if str(t.get('id')) != str(photo.get('observation_taxon_id', photo.get('taxon_id'))): science.append('current_source_taxon_changed')
        rights = sorted(set(rights)); science = sorted(set(science))
        evidence = []
        for relative in meta.get('evidence_paths', []):
            path = g.local_file(root, relative); data = g.load(path, {})
            evidence.append({'path': relative, 'sha256': file_hash(path), 'origin': data.get('origin'),
                'collected_at': data.get('collected_at'), 'source_url': data.get('source_url'),
                'content_address_matches': g.digest(data) == path.stem,
                'locators': ['record.photo.license_code', 'record.photo.license_url', 'record.photo.attribution', 'record.taxon', 'record.quality_grade']})
        rows.append({'id': 'photo:' + pid, 'kind': 'quiz_photo', 'photo_id': pid, 'organisms': credit['organisms'],
            'source': photo.get('source'), 'photo_url': credit['photo_url'], 'download_url': photo.get('url'),
            'local_path': photo['path'], 'author': photo.get('author'), 'title': credit.get('title'),
            'credit_comparison_limit': 'API attribution can be a licensing notice rather than a person, notably CC0. Do not replace a recorded author with that notice or with the observer.',
            'declared_license': photo.get('license'), 'declared_license_url': photo.get('license_url'),
            'evidenced_license_version': meta.get('license_version'), 'modifications': photo.get('modifications'),
            'original_collected_at': meta.get('original_collected_at'), 'first_evidenced_at': meta.get('first_evidenced_at'),
            'last_public_collection': proposal['snapshot']['collected_at'] if proposal else None,
            'recorded_sha256': photo.get('sha256'), 'local_sha256': file_hash(g.local_file(root, photo['path'])),
            'integrity_matches': bool(photo.get('sha256')) and file_hash(g.local_file(root, photo['path'])) == photo.get('sha256'),
            'scope': locate(photo['path']), 'evidence': evidence, 'public_proposal': proposal,
            'public_source_changes': source_changes,
            'attribution_conditions': ['Preserve source credit, title when supplied, specific origin, evidenced license link and modifications.',
                                       'CC0 retains credit by editorial policy; no author/source endorsement implied.'],
            'registered_permission_state': meta.get('permission_state'), 'investigation_state': 'pending_human_review',
            'scientific_reviews': [{'organism_id': organism['id'], **public_review(g.status(
                f"topic:{organism['id']}:photo", current[f"topic:{organism['id']}:photo"], baseline, reviews, g.policy(root)))}
                for organism in credit['organisms']],
            'rights_issues': rights, 'scientific_issues': science,
            'scientific_context': scientific_context(s, proposal),
            'review_preparation': {
                'state': 'public_metadata_ready_for_conference' if proposal else 'public_metadata_missing',
                'license_version_publicly_explicit': bool(proposal and proposal['snapshot']['record'].get('photo', {}).get('license_url')),
                'permission_is_confirmed_by_collection': False,
                'remaining': rights},
            'priority': 0 if science or set(rights) & {'license_outside_policy', 'current_license_requires_policy_review', 'current_photo_hidden_or_flagged'} else 1,
            'curation_request_id': request_id('licensing', 'quiz-photos'),
            'scientific_request_id': request_id('photography', 'photo') if science else None,
            'actions': required_actions(rights, science)})
    for key, p in sorted(credits.get('exploration_photos', {}).items()):
        known.add(p['path'])
        record_key = 'presentation:' + key
        review = g.status(record_key, current[record_key], baseline, reviews, g.policy(root))
        rows.append({'id': 'presentation:' + key, 'kind': 'presentation_photo', 'photo_id': p['photo_id'],
            'subject': p['subject'], 'source': p.get('source'), 'photo_url': p['photo_url'], 'author': p['author'], 'title': p.get('title'),
            'local_path': p['path'], 'download_url': p.get('download_url'),
            'declared_license': p['license'], 'evidenced_license_version': p['license_version'], 'declared_license_url': p['license_url'],
            'last_public_collection': p['collected_at'], 'local_sha256': p['sha256'], 'scope': locate(p['path']),
            'integrity_matches': file_hash(g.local_file(root, p['path'])) == p['sha256'],
            'evidence': [{'path': p['evidence_path'], 'sha256': p['evidence_sha256'], 'source_revision_id': p.get('source_revision_id')}],
            'modifications': p.get('modifications'), 'rights_issues': p['rights_issues'],
            'attribution_conditions': ['Credit the supplied author/title, specific source, license/version link and declared image/UI modifications; CC0 credit is editorial policy.'],
            'registered_permission_state': p.get('permission_state'), 'scientific_declaration': p.get('scientific_review'),
            'registered_scientific_state': review['state'],
            'investigation_state': 'pending_human_review', 'priority': 1 if p['rights_issues'] else 2,
            'curation_request_id': request_id('photography', 'presentation-identification'),
            'actions': ['Preserve file-specific Commons evidence and attribution; review scientific representativeness without presuming expert approval.']})
    materials = []
    for asset in g.load(root / 'resources/compliance/asset_rights.json', {}).get('assets', []):
        relative = asset['path']
        if relative in known: continue
        known.add(relative)
        materials.append({'id': 'asset:' + g.digest(relative)[:24], 'path': relative, 'kind': asset.get('kind'),
            'source_description': asset.get('source'), 'author': asset.get('permission', {}).get('holder'),
            'declared_license': asset.get('license'), 'permission_evidence': asset.get('permission', {}).get('evidence_path'),
            'local_sha256': file_hash(g.local_file(root, relative)), 'scope': locate(relative),
            'registered_state': asset.get('state'), 'investigation_state': 'pending_human_review', 'priority': 1,
            'issues': [f['error'] for f in source['asset_licensing'] if f.get('path') == relative],
            'origin_statements': [asset[k] for k in ('origin_statement', 'origin_addendum') if asset.get(k)],
            'administrative_policy': asset.get('administrative_policy'),
            'curation_request_id': request_id('licensing', 'other-materials'),
            'actions': (['Use the recorded origin declarations and chosen visual policy; do not repeat identity selection. Establish available rights and the legitimate scope of official distribution; no exclusive ownership or trademark registration is presumed.']
                if asset.get('kind') == 'branding' and asset.get('administrative_policy') else
                ['Document rights, third-party portions and the applicable grant; for educational prose use educational_texts to review passage-specific origin. Citations and generated copies are not permission.'])})
    repository_media = [{'path': p, 'sha256': file_hash(g.local_file(root, p)), 'scope': locate(p),
        'usage': 'test_fixture' if p.startswith('tests/') else 'documentation_or_other_media',
        'priority': 3 if p.startswith('tests/') else 1, 'investigation_state': 'pending_human_review',
        'action': 'Review fixture generation/authorship or derived-media contributions separately from the quiz and package; no infringement inferred.'}
        for p in sorted(tracked or ()) if Path(p).suffix.lower() in MEDIA and p not in known]
    dependencies = []
    for component in inventory.get('packages', []):
        name = component['name']
        dependencies.append({'id': name, 'version': component.get('version'), 'declared_licenses': component.get('declared_licenses'),
            'source': component.get('upstream_url'), 'license_files': component.get('license_files'),
            'content_sha256': component.get('content_sha256'), 'registered_review': component.get('obligations_review'),
            'issues': [e['error'] for e in package_gate['dependency_errors'] if e.get('package') == name],
            'investigation_state': 'pending_human_review', 'priority': 1,
            'curation_request_id': request_id('licensing', 'dependencies'),
            'actions': ['Review exact component/version, notices, embedded third parties and source/replacement provision through distribution.py record-review.']})
    historical = g.load(root / 'docs/INVESTIGACAO_EVIDENCIAS.json', {}).get('baseline_governance', {})
    quiz = [r for r in rows if r['kind'] == 'quiz_photo']
    history = public_history_media(root, tracked)
    screenshots = screenshot_inventory(root, locate, quiz)
    text_inventory = educational_text_inventory(root)
    return {'schema_version': 1, 'report_kind': 'rights_investigation', 'checked_at': at or g.now(),
        'mode': 'public_metadata_proposals' if online else 'offline',
        'versions': {'application': (root / 'VERSION.txt').read_text(encoding='utf8').strip() if (root / 'VERSION.txt').is_file() else None,
            'catalog': g.load(root / 'resources/data/catalog_manifest.json', {}).get('version'),
            'catalog_declared_status': g.load(root / 'resources/data/catalog_manifest.json', {}).get('status')},
        'notice': 'Documentary/scientific pending items are not proven copyright violations. Nonprofit status is not inferred permission.',
        'remote_freshness': 'Source comparisons use the explicitly dated last archived consultation, not a new verification of the remote service. checked_at is the local audit date only.',
        'decisions_created': 0, 'scientific_approvals_created': 0,
        'assurances': g.assurances(),
        'source_inputs': {p: file_hash(root / p) for p in INPUTS},
        'historical_comparison': {'source': 'docs/INVESTIGACAO_EVIDENCIAS.json',
            'photo_findings_then': historical.get('photo_permission_findings'), 'photo_findings_now': len(source['photo_licensing']),
            'other_findings_then': historical.get('other_asset_findings'), 'other_findings_now': len(source['asset_licensing']),
            'scientific_blocks_then': historical.get('priority_scientific_blocks'), 'scientific_blocks_now': source['scientific_release_blocks']},
        'summary': {'quiz_photos': len(quiz), 'quiz_permission_pending': len(source['photo_licensing']),
            'credit_divergences_registered': sum('credit_differs_from_source' in r['rights_issues'] for r in quiz),
            'photos_with_staged_public_metadata': sum(bool(r['public_proposal']) for r in quiz),
            'queries_this_run': len(calls),
            'presentation_photos': len(rows) - len(quiz), 'other_material_findings': len(source['asset_licensing']),
            'presentation_permission_findings': sum(bool(r['rights_issues']) for r in rows if r['kind'] == 'presentation_photo'),
            'presentation_scientific_review_pending': sum(r['registered_scientific_state'] not in g.APPROVALS for r in rows if r['kind'] == 'presentation_photo'),
            'scientific_release_blocks': len(source['scientific_release_blocks']), 'other_tracked_media': len(repository_media),
            'readme_screenshots_assessed': len(screenshots),
            'history_only_media_paths': len({r['path'] for r in history['resources']}) if history['assessed'] else None,
            'local_unbundled_media': len(source['unbundled_media']), 'dependency_components': len(dependencies)},
        'photos': sorted(rows, key=lambda r: (r['priority'], r['id'])), 'materials': materials,
        'educational_texts': text_inventory,
        'other_materials_resolution': other_materials_resolution(materials, text_inventory, source['technical_errors']),
        'other_tracked_media': repository_media,
        'readme_screenshots': screenshots, 'public_git_history': history,
        'local_unbundled_media': [dict(f, scope=locate(f['path']), priority=1 if locate(f['path'])['tracked_in_current_git'] else 3,
            curation_request_id=request_id('licensing', 'historical-media')) for f in source['unbundled_media']],
        'dependencies': {'assessed': bool(package), 'inventory_date': inventory.get('inventoried_at'), 'components': dependencies,
            'binary_records': len(inventory.get('binaries', [])), 'errors': package_gate['dependency_errors'] if package else [],
            'technical_errors': package_gate['technical_errors'] if package else [],
            'notice': 'Read-only assessment of existing inventory/bytes; no export, notice copying or review approval.'},
        'distribution_gate': {'source_blocked': source['distribution_blocked'], 'package_assessed': bool(package),
            'package_blocked': package_gate['distribution_blocked'] if package else None,
            'authoritative_command': 'python scripts/distribution.py check', 'proposal_metadata_does_not_satisfy_gate': True},
        'collection_calls': calls, 'staged_evidence_errors': cache_errors}
