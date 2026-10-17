"""Concurrent writers against isolated synthetic catalogs; no network or real decisions."""
from concurrent.futures import ThreadPoolExecutor
import copy
import json
from pathlib import Path
import shutil
import subprocess
import sys
import threading
import time
import unittest
from unittest import mock

PROJECT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT/'scripts'))
import content_governance as governance
import curation_decisions as decisions
import governance_storage as storage
import distribution
import release
import test_curation_decisions as fixtures

WORKER = r'''
import json,os,runpy,sys,time
from pathlib import Path
root=Path(sys.argv[1]); token=sys.argv[2]
sys.path.insert(0,str(root/'scripts'))
import content_governance as g, governance_storage as s, curation_decisions as d
import curation_requests as r, distribution as dependency
spec=json.loads((root/(token+'.json')).read_text(encoding='utf8'))
def wait_for(path):
    deadline=time.monotonic()+20
    while not path.exists():
        if time.monotonic()>deadline: raise TimeoutError('Synthetic start barrier expired')
        time.sleep(.005)
def checkpoint(stage,path):
    if stage==spec.get('pause_stage') and path.name==spec.get('ledger'):
        (root/(token+'.paused')).touch()
        if spec.get('crash'): os._exit(73)
        if not sys.stdin.readline(): raise RuntimeError('Synthetic coordinator closed')
s._checkpoint=checkpoint
def execute():
    route=spec['route']; payload=spec.get('payload',{})
    if route=='hold':
        with s.governance_lock(root):
            (root/(token+'.paused')).touch()
            if not sys.stdin.readline(): raise RuntimeError('Synthetic coordinator closed')
        return
    if route=='direct_review': g.append_review(root,spec['key'],payload)
    elif route=='direct_dependency': dependency.record_dependency_review(root,root/'package',payload)
    elif route=='direct_permission': g.confirm_permission(root,payload)
    elif route=='bridge': d.record_decision(root,r.settings(),payload)
    elif route=='recover': g.permission_recovery(root,apply=True)
    elif route=='migrate': g.migrate(root)
    elif route=='import': g.register_photo_provenance(root,'8',{'photo_id':'8'})
    elif route.startswith('cli_'):
        kind=route[4:]; filename='distribution.py' if kind=='dependency' else 'content_governance.py'
        arguments=['record-review','--package',str(root/'package')] if kind=='dependency' else (
            ['record-review','--key',spec['key']] if kind=='review' else ['confirm-permission'])
        sys.argv=[str(root/'scripts'/filename),*arguments,'--decision',str(root/(token+'.decision.json'))]
        (root/(token+'.decision.json')).write_text(json.dumps(payload),encoding='utf8')
        runpy.run_path(sys.argv[0],run_name='__main__')
    else: raise ValueError('Unknown synthetic route')
(root/(token+'.ready')).touch(); wait_for(root/(token+'.start'))
deadline=time.monotonic()+20
while True:
    try:
        execute(); print(json.dumps({'state':'written'})); break
    except FileExistsError:
        if not spec.get('retry'):
            print(json.dumps({'state':'busy'})); break
        if time.monotonic()>deadline: raise
        time.sleep(.01)
'''


class GovernanceConcurrencyTests(unittest.TestCase):
    def setUp(self):
        self.fixture = fixtures.DecisionTests('test_templates_never_create_approval_or_history')
        self.fixture.setUp(); self.addCleanup(self.fixture.doCleanups)
        self.root = self.fixture.root; self.counter = 0
        # Run the actual CLI entrypoints from a source copy whose ROOT is temporary.
        shutil.copytree(PROJECT/'scripts', self.root/'scripts',
                        ignore=shutil.ignore_patterns('__pycache__', '*.pyc'))

    def wait_for(self, path):
        deadline = time.monotonic()+15
        while not path.exists():
            if time.monotonic()>deadline: self.fail('Concurrent worker did not reach ' + path.name)
            time.sleep(.01)

    def start(self, specs):
        workers = []
        for spec in specs:
            self.counter += 1; token = 'worker-'+str(self.counter)
            governance.save(self.root/(token+'.json'), spec)
            process = subprocess.Popen([sys.executable, '-c', WORKER, str(self.root), token],
                                       stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            self.addCleanup(self.stop, process)
            workers.append((process, token))
        for _, token in workers: self.wait_for(self.root/(token+'.ready'))
        for _, token in workers: (self.root/(token+'.start')).touch()
        return workers

    @staticmethod
    def stop(process):
        if process.poll() is None:
            process.kill(); process.communicate(timeout=10)

    def finish(self, worker, expected='written'):
        process, _ = worker
        stdout, stderr = process.communicate(input='\n', timeout=25)
        self.assertEqual(process.returncode, 0, stderr)
        self.assertEqual(json.loads(stdout.strip().splitlines()[-1])['state'], expected)

    def review(self, token, route='direct_review', key='species:1:summary', **extra):
        decision = self.fixture.fixture.decision(key)
        decision['rationale'] = 'Synthetic concurrent decision '+token
        if route=='bridge':
            envelope = decisions.template(self.root, self.fixture.config, 'review', key, self.fixture.rid, 1)
            envelope['decision'] = decision
            decision = envelope
        return dict(route=route, key=key, payload=decision, **extra)

    def dependency(self, route='direct_dependency', name='synthetic-a', **extra):
        governance.save(self.root/'package/compliance/dependencies.json', {'packages':[
            {'name':'synthetic-'+n, 'content_sha256':'a'*64, 'declared_licenses':['MIT']} for n in ('a','b','c')]})
        draft = decisions.template(self.root, self.fixture.config, 'dependency', name, self.fixture.rid, 2)
        proof = self.fixture.public_proof()
        draft['decision'].update(reviewer='fixture-human', reviewed_at=governance.now(),
            rationale='Synthetic dependency test', evidence_path=proof['path'], evidence_sha256=proof['sha256'],
            source_provision={'method':'not_required'})
        return dict(route=route, payload=draft if route=='bridge' else draft['decision'], **extra)

    def permission(self):
        live = governance.archive_permission(self.root, self.fixture.fixture.observation,
            self.fixture.fixture.photo, 'live_api', governance.now())
        draft = decisions.template(self.root, self.fixture.config, 'permission', '7', self.fixture.rid, 3)
        draft['decision'].update(reviewer='fixture-human', reviewed_at=governance.now(),
            rationale='Synthetic permission only', license_version='4.0', evidence_paths=[live],
            license_version_evidence=self.fixture.public_proof())
        return decisions.template(self.root, self.fixture.config, 'permission', '7', self.fixture.rid, 3, draft)

    def events(self, name='review_history.jsonl'):
        return governance.read_chain(self.root/'resources/compliance'/name)

    def test_four_processes_preserve_every_review_and_existing_prefix(self):
        governance.append_review(self.root, 'species:1:summary', self.fixture.fixture.decision())
        path = self.root/'resources/compliance/review_history.jsonl'; prefix = path.read_bytes()
        workers = self.start([self.review(str(n), retry=True) for n in range(4)])
        for worker in workers: self.finish(worker)
        events = self.events()
        self.assertEqual(len(events), 5); self.assertTrue(path.read_bytes().startswith(prefix))
        self.assertEqual(len({e['previous_sha256'] for e in events}), 5)
        self.assertEqual({e['rationale'] for e in events[1:]}, {'Synthetic concurrent decision '+str(n) for n in range(4)})

    def test_real_review_cli_and_bridge_share_the_same_chain(self):
        workers = self.start([self.review('cli', 'cli_review', retry=True),
                              self.review('bridge', 'bridge', 'topic:1:taxonomy', retry=True)])
        for worker in workers: self.finish(worker)
        self.assertEqual(len(self.events()), 2)
        self.assertEqual({e['key'] for e in self.events()}, {'species:1:summary','topic:1:taxonomy'})

    def test_dependency_api_cli_and_bridge_are_all_serialized(self):
        workers = self.start([self.dependency(retry=True),
            self.dependency('cli_dependency','synthetic-b',retry=True), self.dependency('bridge','synthetic-c',retry=True)])
        for worker in workers: self.finish(worker)
        self.assertEqual({e['package'] for e in self.events('dependency_reviews.jsonl')},
                         {'synthetic-a','synthetic-b','synthetic-c'})

    def test_concurrent_permission_api_cli_and_bridge_preserve_inv02_idempotency(self):
        envelope = self.permission(); decision = copy.deepcopy(envelope['decision'])
        decision['curation'] = dict(envelope['curation'], decision_sha256=governance.digest(
            {k:v for k,v in envelope.items() if k!='notice'}))
        workers = self.start([{'route':'bridge','payload':envelope,'retry':True},
            {'route':'direct_permission','payload':decision,'retry':True},
            {'route':'cli_permission','payload':decision,'retry':True}])
        for worker in workers: self.finish(worker)
        self.assertEqual(len(self.events('permission_history.jsonl')), 1)
        self.assertEqual(governance.permission_recovery(self.root)['state'], 'consistent')

    def test_all_interfaces_refuse_while_another_writer_is_paused_after_reading_tip(self):
        held = self.start([self.review('holder', 'cli_review', pause_stage='history_read', ledger='review_history.jsonl')])[0]
        self.wait_for(self.root/(held[1]+'.paused'))
        envelope = self.permission()
        specs = [self.review('direct'), self.review('cli','cli_review'), self.review('bridge','bridge'),
                 self.dependency(), self.dependency('cli_dependency','synthetic-b'), self.dependency('bridge','synthetic-c'),
                 {'route':'direct_permission','payload':envelope['decision']}, {'route':'cli_permission','payload':envelope['decision']},
                 {'route':'bridge','payload':envelope}, {'route':'recover'}, {'route':'migrate'}, {'route':'import'}]
        for worker in self.start(specs): self.finish(worker, 'busy')
        self.assertEqual(self.events(), [])
        self.assertEqual(self.events('permission_history.jsonl'), [])
        self.assertEqual(self.events('dependency_reviews.jsonl'), [])
        self.finish(held); self.assertEqual(len(self.events()), 1)

    def test_removing_build_does_not_remove_the_effective_lock(self):
        held = self.start([self.review('holder', pause_stage='history_read', ledger='review_history.jsonl')])[0]
        self.wait_for(self.root/(held[1]+'.paused'))
        build = (self.root/'build').resolve(); build.mkdir(exist_ok=True)
        (build/'synthetic.txt').write_text('Temporary build fixture')
        self.assertTrue(build.is_relative_to(self.root.resolve()))
        shutil.rmtree(build)
        self.assertTrue((self.root/'resources/compliance/.governance.lock').is_file())
        self.finish(self.start([self.review('blocked','cli_review')])[0], 'busy')
        self.finish(held)
        self.finish(self.start([self.review('later')])[0]); self.assertEqual(len(self.events()), 2)

    def test_timeout_is_bounded_and_process_exit_releases_the_lock(self):
        held = self.start([{'route':'hold'}])[0]
        self.wait_for(self.root/(held[1]+'.paused'))
        started = time.monotonic()
        with self.assertRaises(FileExistsError):
            with storage.governance_lock(self.root, timeout=.15): self.fail('Lock acquired while held')
        self.assertGreaterEqual(time.monotonic()-started, .14)
        self.assertLess(time.monotonic()-started, 3)
        self.stop(held[0])
        self.finish(self.start([self.review('after killed holder')])[0])
        self.assertEqual(len(self.events()), 1)

    def test_bounded_wait_succeeds_after_another_thread_releases_ownership(self):
        entered, release_owner = threading.Event(), threading.Event()
        def hold():
            with storage.governance_lock(self.root):
                entered.set()
                if not release_owner.wait(timeout=5): raise TimeoutError('Synthetic owner did not release')
        with ThreadPoolExecutor(max_workers=1) as pool:
            future = pool.submit(hold)
            self.assertTrue(entered.wait(timeout=3))
            timer = threading.Timer(.1, release_owner.set); timer.start()
            try:
                with storage.governance_lock(self.root, timeout=2): storage.require_governance_lock(self.root)
            finally:
                release_owner.set(); timer.cancel()
            future.result(timeout=3)

    def test_competing_bridge_decisions_require_comparison_after_the_first_commit(self):
        workers = self.start([self.review('first','bridge',retry=True), self.review('second','bridge',retry=True)])
        successes = 0
        for process, _ in workers:
            stdout, stderr = process.communicate(timeout=25)
            if process.returncode == 0:
                successes += 1
                self.assertEqual(json.loads(stdout.strip().splitlines()[-1])['state'],'written')
            else:
                self.assertIn('Another human decision was recorded',stderr)
        self.assertEqual(successes,1); self.assertEqual(len(self.events()),1)
        self.assertIn(self.events()[0]['rationale'], {'Synthetic concurrent decision first','Synthetic concurrent decision second'})

    def test_process_crashes_before_and_after_review_and_dependency_commit_preserve_chain(self):
        for route, ledger in (('direct_review','review_history.jsonl'), ('direct_dependency','dependency_reviews.jsonl')):
            for stage in ('temporary_synced','replaced'):
                with self.subTest(route=route, stage=stage):
                    prior = len(self.events(ledger))
                    spec = self.review('crash') if route=='direct_review' else self.dependency()
                    spec.update(pause_stage=stage, ledger=ledger, crash=True)
                    worker = self.start([spec])[0]
                    stdout, stderr = worker[0].communicate(timeout=20)
                    self.assertEqual(worker[0].returncode, 73, stderr)
                    after_crash = self.events(ledger)
                    self.assertEqual(len(after_crash), prior+(stage=='replaced'))
                    prefix = (self.root/'resources/compliance'/ledger).read_bytes() if after_crash else b''
                    valid = self.review('after crash') if route=='direct_review' else self.dependency()
                    self.finish(self.start([valid])[0])
                    self.assertEqual(len(self.events(ledger)), len(after_crash)+1)
                    self.assertTrue((self.root/'resources/compliance'/ledger).read_bytes().startswith(prefix))

    def test_threads_have_distinct_ownership_and_nested_bridge_can_reuse_its_lock(self):
        barrier = threading.Barrier(4)
        def write(n):
            barrier.wait(timeout=5)
            deadline = time.monotonic()+5
            while True:
                try:
                    return governance.append_review(self.root,'species:1:summary',self.review(str(n))['payload'])
                except FileExistsError:
                    if time.monotonic()>deadline: raise
                    time.sleep(.01)
        with ThreadPoolExecutor(max_workers=4) as pool: list(pool.map(write, range(4)))
        self.assertEqual(len(self.events()), 4)
        envelope = self.review('nested','bridge','topic:1:taxonomy')['payload']
        with storage.governance_lock(self.root):
            with storage.governance_lock(self.root):
                decisions.record_decision(self.root,self.fixture.config,envelope)
        self.assertEqual(len(self.events()), 5)

    def test_stale_expected_tip_is_rejected_without_changing_history(self):
        governance.append_review(self.root,'species:1:summary',self.fixture.fixture.decision())
        path = self.root/'resources/compliance/review_history.jsonl'; before = path.read_bytes()
        with self.assertRaisesRegex(ValueError, 'tip changed'):
            storage.append_event(self.root,path.name,{'synthetic':'stale'},'')
        self.assertEqual(before,path.read_bytes()); self.assertEqual(len(self.events()),1)

    def test_tip_is_revalidated_after_temporary_sync_immediately_before_replace(self):
        governance.append_review(self.root,'species:1:summary',self.fixture.fixture.decision())
        path = self.root/'resources/compliance/review_history.jsonl'; first = self.events()[0]
        foreign = {'synthetic':'writer ignoring lock', 'previous_sha256':first['event_sha256']}
        foreign['event_sha256'] = governance.digest(foreign)
        prefix = path.read_bytes(); changed = prefix+(json.dumps(foreign)+'\n').encode('utf8')
        def interference(stage, target):
            if stage=='temporary_synced' and target==path: path.write_bytes(changed)
        with mock.patch.object(storage,'_checkpoint',side_effect=interference):
            with self.assertRaisesRegex(ValueError,'changed before commit'):
                storage.append_event(self.root,path.name,{'synthetic':'must not overwrite'},first['event_sha256'])
        self.assertEqual(changed,path.read_bytes()); self.assertEqual(len(self.events()),2)

    def test_corrupt_chains_are_preserved_and_never_extended(self):
        for ledger in storage.LEDGERS:
            with self.subTest(ledger=ledger):
                event = storage.append_event(self.root,ledger,{'synthetic':'fixture only'},'')
                path = self.root/'resources/compliance'/ledger
                data = path.read_bytes()+b'{"truncated":'
                path.write_bytes(data)
                with self.assertRaises(ValueError):
                    storage.append_event(self.root,ledger,{'synthetic':'blocked'},event['event_sha256'])
                self.assertEqual(data,path.read_bytes())

    def test_private_paths_require_ownership_and_invalid_timeouts_are_refused(self):
        with self.assertRaises(RuntimeError): governance._append_review(self.root,'species:1:summary',{})
        with self.assertRaises(RuntimeError): governance._confirm_permission(self.root,{})
        with self.assertRaises(RuntimeError): decisions._record_decision(self.root,self.fixture.config,{})
        for timeout in (-1,31,float('inf'),float('nan')):
            with self.assertRaises(ValueError):
                with storage.governance_lock(self.root,timeout=timeout): pass

    def test_runtime_lock_is_excluded_from_catalog_and_export_but_evidence_is_preserved(self):
        proof = self.fixture.public_proof()
        before = release.catalog_inputs(self.root)
        with storage.governance_lock(self.root):
            self.assertTrue((self.root/storage.LOCK_RELATIVE).is_file())
            self.assertNotIn(storage.LOCK_RELATIVE, release.catalog_inputs(self.root))
            self.assertEqual(before, release.catalog_inputs(self.root))
        for name in ('credits.json','exploration_photos.json'):
            governance.save(self.root/'resources/data'/name,{})
        with mock.patch.object(distribution,'dependencies',return_value={}), \
                mock.patch.object(distribution,'audit_governance',return_value={}):
            distribution.export_local(self.root,self.root/'package',self.root/'synthetic-prefix')
        exported = self.root/'package/compliance/evidence'
        self.assertFalse((exported/Path(storage.LOCK_RELATIVE).name).exists())
        self.assertEqual((exported/Path(proof['path']).relative_to('resources/compliance')).read_bytes(),
                         (self.root/proof['path']).read_bytes())


if __name__=='__main__': unittest.main()
