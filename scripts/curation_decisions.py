"""Explicit human decisions routed to existing ledgers; no GitHub event handler.

Templates are incomplete drafts. A closed Issue or an approval label is never
accepted as an evidence source or an approval instruction.
"""
import copy
from pathlib import Path
import re
from urllib.parse import urlparse

import content_governance as governance
from governance_storage import governance_lock
from curation_requests import private_data, public_reference


def trace(config,rid,issue):
    if not re.fullmatch(r'curation-v1-[a-f0-9]{24}',str(rid)) or not isinstance(issue,int) or issue<1:
        raise ValueError('An explicit curation request ID and Issue number are required')
    return {'request_id':rid,'issue_url':f"https://github.com/{config['repository']}/issues/{issue}"}


def template(root,config,kind,key,rid,issue,draft=None):
    link=trace(config,rid,issue);decision=copy.deepcopy((draft or {}).get('decision',{}))
    for field in ('reviewer','reviewed_at','rationale'):decision.setdefault(field,None)
    decision['actor_type']='human'  # Required declaration, not a fabricated reviewer.
    if kind=='review':
        current=governance.records(root,include_presentation=True)
        if key not in current:raise ValueError('Unknown scientific record')
        decision.setdefault('state','pending');decision.setdefault('review_kind','editorial')
        decision.setdefault('claim_scope','hint_assistance' if current[key]['kind']=='assistance' else 'group' if current[key]['kind']=='presentation' else None)
        decision.setdefault('evidence',[]);decision['content_sha256']=current[key]['content_sha256']
        ledger='resources/compliance/review_history.jsonl'
        prior=[e for e in governance.read_chain(root/ledger) if e.get('key')==key]
    elif kind=='permission':
        photo=next((s['photo'] for s in governance.load(root/'resources/data/species.json',[]) if str(s['photo']['id'])==str(key)),None)
        if photo is None:raise ValueError('Unknown photo ID')
        provenance=governance.permission_provenance(root).get(str(key),{})
        decision['photo_id']=str(key);decision.setdefault('license_version',provenance.get('license_version'))
        decision.setdefault('license_version_evidence',None)
        decision.setdefault('evidence_paths',provenance.get('evidence_paths',[]))
        binding={'photo_id':str(key),'sha256':photo.get('sha256'),'license':photo.get('license'),
                 'license_url':photo.get('license_url'),'license_version':decision['license_version'],
                 'evidence_paths':sorted(set(provenance.get('evidence_paths',[])+decision['evidence_paths']))}
        decision['content_sha256']=governance.digest(binding)
        ledger='resources/compliance/permission_history.jsonl'
        prior=[e for e in governance.read_chain(root/ledger) if e.get('photo_id')==str(key)]
    elif kind=='dependency':
        package=root/'package'
        component=next((c for c in governance.load(package/'compliance/dependencies.json',{}).get('packages',[]) if c['name']==key),None)
        if component is None:raise ValueError('Current package inventory required')
        decision['package']=key;decision['content_sha256']=component['content_sha256']
        decision.setdefault('evidence_path',None);decision.setdefault('evidence_sha256',None);decision.setdefault('source_provision',None)
        ledger='resources/compliance/dependency_reviews.jsonl'
        prior=[e for e in governance.read_chain(root/ledger) if e.get('package')==key]
    else:raise ValueError('Unknown decision kind')
    return {'schema_version':1,'kind':kind,'key':key,'curation':link,'decision':decision,
            'expected_previous_event_sha256':prior[-1]['event_sha256'] if prior else '',
            'notice':'Incomplete human draft; never imports Issue state, implies consent or sends a message.'}


def record_decision(root,config,envelope):
    with governance_lock(root):
        return _record_decision(root,config,envelope)


def _record_decision(root,config,envelope):
    if not isinstance(envelope,dict) or not isinstance(envelope.get('decision'),dict) or envelope.get('schema_version')!=1 or private_data(envelope):raise ValueError('Private or invalid decision')
    kind=envelope.get('kind');key=envelope.get('key');link=envelope.get('curation',{})
    match=re.fullmatch(r'https://github\.com/'+re.escape(config['repository'])+r'/issues/([1-9][0-9]*)',link.get('issue_url',''))
    if not match:raise ValueError('Issue reference must belong to this repository')
    trace(config,link.get('request_id'),int(match[1]))
    if kind=='permission':
        recovery=governance._recover_permissions(root,apply=True)
        if recovery['errors']:raise ValueError(str(recovery))
    # Recompute the actual source/permission/component binding before idempotency
    # checks; a past approval is not a fresh approval of changed content.
    expected=template(root,config,kind,key,link['request_id'],int(match[1]),envelope)
    decision=copy.deepcopy(envelope['decision'])
    if decision.get('content_sha256')!=expected['decision']['content_sha256']:
        raise ValueError('Decision content binding is stale')
    if decision.get('actor_type')!='human' or not all(decision.get(k) for k in ('reviewer','reviewed_at','rationale')):
        raise ValueError('A real human responsibility/date/rationale declaration is required')
    # Confidential documents remain outside the public ledgers. Existing evidence
    # must be redacted and reviewed before its public path can be registered.
    for path in decision.get('evidence_paths',[]):
        if public_reference(root,path) is None:raise ValueError('Evidence is not eligible for a public ledger')
    if not isinstance(decision.get('evidence',[]),list) or any(not isinstance(e,dict) for e in decision.get('evidence',[])):
        raise ValueError('Evidence must be structured')
    urls=[e.get('url','') for e in decision.get('evidence',[])]
    if kind=='permission':
        proof=decision.get('license_version_evidence')
        if not isinstance(proof,dict) or public_reference(root,proof.get('path')) is None:
            raise ValueError('Public redacted license evidence required')
        urls.append(proof.get('source_url',''))
    if kind=='dependency' and public_reference(root,decision.get('evidence_path')) is None:
        raise ValueError('Public redacted dependency evidence required')
    for url in urls:
        parsed=urlparse(url)
        if not governance.https_url(url) or parsed.query or parsed.hostname in {'localhost','127.0.0.1','::1'}:
            raise ValueError('Use public canonical evidence URLs, not credentials/private endpoints')
    decision_hash=governance.digest({k:v for k,v in envelope.items() if k!='notice'})
    ledger={'review':'review_history.jsonl','permission':'permission_history.jsonl','dependency':'dependency_reviews.jsonl'}[kind]
    events=governance.read_chain(root/'resources/compliance'/ledger)
    existing=next((e for e in events if e.get('curation',{}).get('decision_sha256')==decision_hash),None)
    if existing:
        if kind=='permission':
            decision['curation']=dict(link,decision_sha256=decision_hash)
            governance._confirm_permission(root,decision)
        return {'state':'already_recorded','event_sha256':existing['event_sha256']}
    if expected['expected_previous_event_sha256']!=envelope.get('expected_previous_event_sha256'):
        raise ValueError('Another human decision was recorded; compare it before proceeding')
    decision['curation']=dict(link,decision_sha256=decision_hash)
    if kind=='review':event=governance.append_review(root,key,decision)
    elif kind=='permission':
        governance._confirm_permission(root,decision)
        event=governance.read_chain(root/'resources/compliance'/ledger)[-1]
    else:
        from distribution import record_dependency_review
        event=record_dependency_review(root,root/'package',decision)
    return {'state':'recorded','event_sha256':event['event_sha256']}


def authorization_draft(root,config,photo_id):
    s=next((s for s in governance.load(root/'resources/data/species.json',[]) if str(s['photo']['id'])==str(photo_id)),None)
    if s is None:raise ValueError('Unknown photo')
    p=s['photo']
    return {'schema_version':1,'prepared_at':governance.now(),'photo_id':str(photo_id),'photo_sha256':p['sha256'],
            'source_url':p.get('source'),'credit_as_provided':p.get('author'),
            'contact_status':'not_sent','consent':None,
            'draft':'Solicitamos análise de autorização para incluir esta fotografia no Projeto Glossata, jogo educativo distribuído offline. É necessário confirmar a titularidade, escopo de redistribuição, condições, crédito e alterações permitidas. Preferimos CC0 ou CC BY com versão e URL verificáveis; outro acordo exige análise e evidência específicas. Nenhum consentimento é presumido.',
            'human_steps':['Conferir titularidade sem presumir que o observador é fotógrafo.',
                           'Escolher um contato individual legítimo; preencher e revisar a mensagem.',
                           'Registrar envio/resposta privadamente; publicar somente evidência redigida e autorizada.'],
            'notice':'Local preparation only; no email/contact discovery or sending capability exists.'}
