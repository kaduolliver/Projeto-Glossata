"""Human-decision bridge against temporary real governance ledgers."""
import copy
import hashlib
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import test_content_governance as fixtures
import content_governance as governance
import curation_requests as requests
import curation_decisions as decisions


class DecisionTests(unittest.TestCase):
    def setUp(self):
        self.fixture=fixtures.GovernanceTests('test_legacy_migration_is_idempotent_and_never_creates_a_human_review')
        self.fixture.setUp();self.addCleanup(self.fixture.doCleanups)
        self.root=self.fixture.root;self.fixture.baseline();self.config=requests.settings()
        self.rid=requests.request_id('science','education');self.key='species:1:summary'
    def envelope(self):
        envelope=decisions.template(self.root,self.config,'review',self.key,self.rid,1)
        envelope['decision']=self.fixture.decision()
        return envelope
    def test_templates_never_create_approval_or_history(self):
        before=(self.root/'resources/compliance/review_baseline.json').read_bytes()
        draft=decisions.template(self.root,self.config,'review',self.key,self.rid,1)
        self.assertIsNone(draft['decision']['reviewer']);self.assertEqual(draft['decision']['state'],'pending')
        self.assertEqual(before,(self.root/'resources/compliance/review_baseline.json').read_bytes())
        self.assertFalse((self.root/'resources/compliance/review_history.jsonl').exists())
    def test_approved_decision_routes_to_existing_history_and_replay_is_idempotent(self):
        envelope=self.envelope();first=decisions.record_decision(self.root,self.config,envelope)
        path=self.root/'resources/compliance/review_history.jsonl';before=path.read_bytes()
        second=decisions.record_decision(self.root,self.config,envelope)
        self.assertEqual(first['event_sha256'],second['event_sha256']);self.assertEqual(second['state'],'already_recorded')
        self.assertEqual(before,path.read_bytes())
        event=governance.read_ledger(self.root)[0];self.assertEqual(event['curation']['request_id'],self.rid)
        self.assertEqual(event['reviewer'],envelope['decision']['reviewer'])
    def test_changed_content_cannot_reuse_an_old_decision(self):
        envelope=self.envelope();decisions.record_decision(self.root,self.config,envelope)
        path=self.root/'resources/compliance/review_history.jsonl';before=path.read_bytes()
        education=governance.load(self.root/'resources/data/education.json');education['notes'][self.key]['text']='Changed.'
        self.fixture.write('education.json',education)
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,envelope)
        self.assertEqual(before,path.read_bytes())
    def test_new_decision_requires_knowledge_of_the_previous_human_decision(self):
        first=self.envelope();decisions.record_decision(self.root,self.config,first)
        stale=copy.deepcopy(first);stale['decision']['state']='rejected';stale['decision']['rationale']='New fixture decision.'
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,stale)
        fresh=decisions.template(self.root,self.config,'review',self.key,self.rid,1,stale)
        decisions.record_decision(self.root,self.config,fresh)
        events=governance.read_ledger(self.root);self.assertEqual(len(events),2)
        self.assertEqual(events[0]['state'],'approved_editorial');self.assertEqual(events[1]['state'],'rejected')
    def test_issue_closure_labels_and_automated_actor_never_authorize_a_resource(self):
        draft=decisions.template(self.root,self.config,'review',self.key,self.rid,1)
        draft['issue_state']='closed';draft['labels']=['curadoria:aprovada']
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,draft)
        draft=self.envelope();draft['decision']['actor_type']='automated'
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,draft)
        self.assertEqual(governance.read_ledger(self.root),[])
    def test_private_evidence_is_refused_before_recording(self):
        envelope=self.envelope();envelope['decision']['evidence'][0]['url']='https://example.org/private?access_token=synthetic'
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,envelope)
        envelope=self.envelope();envelope['decision']['reviewer']='private@example.org'
        with self.assertRaises(ValueError):decisions.record_decision(self.root,self.config,envelope)
        self.assertEqual(governance.read_ledger(self.root),[])
    def public_proof(self):
        data={'source_url':'https://example.org/photos/7','text':'Synthetic public license evidence, fixture only.'}
        name='resources/compliance/permissions/'+governance.digest(data)+'.json'
        governance.save(self.root/name,data)
        return {'path':name,'sha256':hashlib.sha256((self.root/name).read_bytes()).hexdigest(),
                'source_url':'https://example.org/photos/7','locator':'photo-specific license, test fixture'}
    def test_complete_photo_permission_reuses_real_validator_and_preserves_trace(self):
        live=governance.archive_permission(self.root,self.fixture.observation,self.fixture.photo,'live_api',governance.now())
        draft=decisions.template(self.root,self.config,'permission','7',requests.request_id('licensing','quiz-photos'),2)
        draft['decision'].update(reviewer='fixture-human',reviewed_at=governance.now(),rationale='Fixture only.',
            license_version='4.0',evidence_paths=[live],license_version_evidence=self.public_proof())
        draft=decisions.template(self.root,self.config,'permission','7',draft['curation']['request_id'],2,draft)
        decisions.record_decision(self.root,self.config,draft)
        self.assertEqual(decisions.record_decision(self.root,self.config,draft)['state'],'already_recorded')
        history=governance.read_chain(self.root/'resources/compliance/permission_history.jsonl')
        self.assertEqual(len(history),1);self.assertEqual(history[0]['curation']['request_id'],draft['curation']['request_id'])
    def test_dependency_review_is_bound_to_current_inventory(self):
        governance.save(self.root/'package/compliance/dependencies.json',{'packages':[
            {'name':'synthetic-component','content_sha256':'a'*64,'declared_licenses':['MIT']}]})
        draft=decisions.template(self.root,self.config,'dependency','synthetic-component',requests.request_id('licensing','dependencies'),3)
        proof=self.public_proof();draft['decision'].update(reviewer='fixture-human',reviewed_at=governance.now(),
            rationale='Fixture only.',evidence_path=proof['path'],evidence_sha256=proof['sha256'],source_provision={'method':'not_required'})
        decisions.record_decision(self.root,self.config,draft)
        self.assertEqual(decisions.record_decision(self.root,self.config,draft)['state'],'already_recorded')
    def test_authorization_preparation_never_sends_or_infers_consent(self):
        draft=decisions.authorization_draft(self.root,self.config,'7')
        self.assertEqual(draft['contact_status'],'not_sent');self.assertIsNone(draft['consent'])
        self.assertEqual(draft['credit_as_provided'],self.fixture.species['photo']['author'])
    def test_presentation_review_uses_same_ledger_without_changing_quiz_records(self):
        original=governance.records(self.root)
        manifest={'photos':{'synthetic':{'subject':'Example species','sha256':'a'*64,
                    'photo_url':'https://commons.wikimedia.org/wiki/File:synthetic.jpg',
                    'scientific_review':'source only; no human declared'}},'bindings':{'domain:Eukaryota':'synthetic'}}
        self.fixture.write('exploration_photos.json',manifest)
        self.assertEqual(original,governance.records(self.root))
        key='presentation:synthetic';rid=requests.request_id('photography','presentation-identification')
        draft=decisions.template(self.root,self.config,'review',key,rid,2)
        draft['decision'].update(self.fixture.decision());draft['decision']['claim_scope']='group'
        draft['decision']['content_sha256']=governance.records(self.root,include_presentation=True)[key]['content_sha256']
        decisions.record_decision(self.root,self.config,draft)
        records=governance.records(self.root,include_presentation=True)
        status=governance.status(key,records[key],{},governance.read_ledger(self.root),governance.policy(self.root))
        self.assertTrue(status['usable']);self.assertEqual(status['state'],'approved_editorial')
        manifest['bindings']['domain:Bacteria']='synthetic';self.fixture.write('exploration_photos.json',manifest)
        record=governance.records(self.root,include_presentation=True)[key]
        self.assertEqual(governance.status(key,record,{},governance.read_ledger(self.root),governance.policy(self.root))['state'],'needs_review')
    def test_another_decision_import_holding_the_lock_preserves_history(self):
        lock=self.root/'build/curation/decision.lock';lock.parent.mkdir(parents=True);lock.write_text('active')
        with self.assertRaises(FileExistsError):decisions.record_decision(self.root,self.config,self.envelope())
        self.assertEqual(governance.read_ledger(self.root),[]);self.assertEqual(lock.read_text(),'active')


if __name__=='__main__':unittest.main()
