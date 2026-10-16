"""Deterministic GitHub integration replay; no credentials or remote writes."""
import copy
import io
import json
import os
from pathlib import Path
import sys
import unittest
from unittest import mock
import urllib.error
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import curation_github as github
import curation_requests as requests


class FakeGitHub:
    def __init__(self):
        self.config=requests.settings();self.issues=[];self.calls=[];self.fail=None
    def pages(self,path,maximum):
        if '/labels' in path:return [{'name':k} for k in github.labels(self.config)]
        return copy.deepcopy(self.issues)
    def request(self,method,path,data=None):
        self.calls.append((method,path,copy.deepcopy(data)))
        if self.fail and method!='GET':raise github.GitHubError(self.fail)
        if method=='GET':
            if '/commits/' in path:return {'sha':path.rsplit('/',1)[1]}
            if '/issues/' in path:return copy.deepcopy(next(i for i in self.issues if i['number']==int(path.rsplit('/',1)[1])))
            return {'full_name':self.config['repository'],'has_issues':True,'archived':False}
        if method=='POST' and path.endswith('/issues'):
            issue=dict(data,number=len(self.issues)+1,node_id='I_'+str(len(self.issues)+1),state='open')
            issue['labels']=[{'name':v} for v in data['labels']];self.issues.append(issue);return copy.deepcopy(issue)
        if method=='PATCH':
            issue=next(i for i in self.issues if i['number']==int(path.rsplit('/',1)[1]));issue.update(data)
            issue['labels']=[{'name':v} for v in data['labels']];return copy.deepcopy(issue)
        raise AssertionError((method,path))


def queue(number=1):
    config=requests.settings();rows=[]
    for i in range(number):
        rows.append({'id':requests.request_id('science',str(i)),'area':'science','bucket':str(i),
                     'priority':2,'findings':[],'fingerprint':f'{i:064x}'})
    return {'repository':config['repository'],'requests':rows}


class GitHubTests(unittest.TestCase):
    def setUp(self):
        self.config=requests.settings();self.client=FakeGitHub();self.sha='a'*40
        self.env=mock.patch.dict(os.environ,{},clear=True);self.env.start();self.addCleanup(self.env.stop)
    def sync(self,q=None,apply=True):return github.sync(q or queue(),self.config,self.sha,apply,self.client)
    def test_creation_is_bounded_and_reruns_do_not_duplicate(self):
        q=queue(8);first=self.sync(q);self.assertEqual(len(first['created']),3)
        second=self.sync(q);self.assertEqual(len(second['created']),3)
        third=self.sync(q);self.assertEqual(len(third['created']),2)
        fourth=self.sync(q);self.assertEqual(fourth['created'],[]);self.assertEqual(len(self.client.issues),8)
    def test_update_preserves_human_notes_labels_title_and_status(self):
        self.sync();issue=self.client.issues[0]
        issue['body']+='\nHuman notes: retain this exactly.';issue['title']='Human title'
        issue['labels']=[{'name':'curadoria:automatica'},{'name':'curadoria:em-analise'},{'name':'manual'}]
        q=queue();q['requests'][0]['fingerprint']='f'*64
        result=self.sync(q);self.assertEqual(len(result['updated']),1)
        saved=self.client.issues[0];self.assertTrue(saved['body'].endswith('Human notes: retain this exactly.'))
        self.assertEqual(saved['title'],'Human title');self.assertIn({'name':'manual'},saved['labels'])
        self.assertIn({'name':'curadoria:em-analise'},saved['labels'])
    def test_closed_issue_never_approves_reopens_or_creates_a_replacement(self):
        self.sync();self.client.issues[0]['state']='closed';self.client.calls.clear()
        result=self.sync();self.assertEqual(len(result['closed_unresolved']),1)
        self.assertEqual(len(self.client.issues),1);self.assertTrue(all(m=='GET' for m,_,_ in self.client.calls))
    def test_duplicate_markers_are_conflicts_not_more_issues(self):
        self.sync();self.client.issues.append(copy.deepcopy(self.client.issues[0]))
        result=self.sync();self.assertEqual(len(result['conflicts']),1);self.assertEqual(result['created'],[])
    def test_dry_run_has_no_writes_and_missing_token_has_no_network(self):
        self.assertEqual(self.sync(apply=False)['created'],[])
        self.assertTrue(all(m=='GET' for m,_,_ in self.client.calls))
        with mock.patch('urllib.request.build_opener') as net:
            self.assertFalse(github.sync(queue(),self.config,self.sha)['remote_checked']);net.assert_not_called()
    def test_forbidden_permission_fails_without_retry(self):
        self.client.fail='github_http_403';result=self.sync()
        self.assertEqual(result['errors'],['github_http_403']);self.assertEqual(self.client.issues,[])
        self.assertEqual(sum(m=='POST' for m,_,_ in self.client.calls),1)
    def test_incomplete_pagination_never_publishes(self):
        self.client.pages=mock.Mock(side_effect=github.GitHubError('github_pagination_incomplete'))
        result=self.sync();self.assertEqual(result['created'],[])
        self.assertTrue(all(m=='GET' for m,_,_ in self.client.calls))
    def test_unpublished_revision_never_publishes(self):
        original=self.client.request
        self.client.request=lambda m,p,d=None: {'sha':'b'*40} if '/commits/' in p else original(m,p,d)
        result=self.sync();self.assertEqual(result['errors'],['github_source_revision_not_available'])
        self.assertEqual(self.client.issues,[])
    def test_ambiguous_block_is_not_overwritten(self):
        self.sync();self.client.issues[0]['body']+=github.START
        q=queue();q['requests'][0]['fingerprint']='b'*64
        result=self.sync(q);self.assertEqual(result['errors'],['github_managed_block_ambiguous'])
    def test_public_forms_join_project_without_body_processing(self):
        self.client.issues=[{'number':10,'node_id':'MANUAL','state':'open','body':'untrusted text',
                             'labels':[{'name':'curadoria:science'},{'name':'curadoria:pendente'}]}]
        board=mock.Mock();board.synchronize.return_value={'changed':1}
        github.sync(queue(),self.config,self.sha,True,self.client,board)
        issues=board.synchronize.call_args.args[0];self.assertIn('MANUAL',[i['node_id'] for i in issues])
    def test_http_error_does_not_expose_body_or_token_and_never_retries(self):
        secret='synthetic-test-value'
        transport=mock.Mock(side_effect=urllib.error.HTTPError('https://api.github.com/test',403,'refused',{},io.BytesIO(secret.encode())))
        client=github.GitHub(secret,transport=transport,pace=0)
        with self.assertRaisesRegex(github.GitHubError,'github_http_403') as error:client.request('POST','/test',{})
        self.assertNotIn(secret,str(error.exception));self.assertEqual(transport.call_count,1)
    def test_redirects_and_budget_are_guarded(self):
        with self.assertRaises(github.GitHubError):github.NoRedirect().redirect_request(None,None,None,None,None,None)
        with mock.patch('urllib.request.build_opener',side_effect=AssertionError('Offline budget check initialized TLS')) as opener:
            client=github.GitHub('synthetic',limit=0)
            with self.assertRaisesRegex(github.GitHubError,'budget'):client.request('GET','/test')
            with self.assertRaises(github.GitHubError):client.request('GET','//other.example')
            opener.assert_not_called()


class ProjectTests(unittest.TestCase):
    def setUp(self):
        self.config=requests.settings();self.fields=[{'id':'F','name':'Curadoria',
            'options':[{'id':str(i),'name':v} for i,v in enumerate(self.config['statuses'].values())]}]
        self.client=mock.Mock();self.items=[];self.queries=[]
        self.board=github.ProjectBoard(self.client,'P_TEST',self.config)
        self.board.fields=lambda:self.fields
        def query(text,variables):
            self.queries.append((text,variables))
            if 'items(first:' in text:return {'node':{'items':{'nodes':self.items,'pageInfo':{'hasNextPage':False}}}}
            if 'addProjectV2ItemById' in text:return {'addProjectV2ItemById':{'item':{'id':'ITEM'}}}
            return {'updateProjectV2ItemFieldValue':{'projectV2Item':{'id':'ITEM'}}}
        self.board.query=query
        self.issue={'node_id':'ISSUE','labels':[{'name':'curadoria:aprovada'}],'state':'closed'}
    def test_project_add_and_update_are_idempotent_and_use_labels_not_closure(self):
        self.assertEqual(self.board.synchronize([self.issue],True)['changed'],1)
        self.assertEqual(self.queries[-1][1]['option'],'3')
        self.items=[{'id':'ITEM','content':{'id':'ISSUE'},'fieldValues':{'nodes':[{'optionId':'3','field':{'id':'F'}}]}}]
        self.queries.clear();self.assertEqual(self.board.synchronize([self.issue],True)['changed'],0)
        self.assertEqual(len(self.queries),1)
        self.issue['labels']=[{'name':'curadoria:pendente'}]
        self.board.synchronize([self.issue],True);self.assertEqual(self.queries[-1][1]['option'],'0')
    def test_missing_options_and_conflicting_status_are_explicit(self):
        self.fields[0]['options']=[]
        with self.assertRaisesRegex(github.GitHubError,'options_missing'):self.board.synchronize([],True)
        self.setUp();self.issue['labels'].append({'name':'curadoria:rejeitada'})
        self.assertEqual(self.board.synchronize([self.issue],True)['ambiguous_statuses'],1)
    def test_existing_non_select_field_is_not_recreated_or_used(self):
        self.fields=[{'id':'F','name':'Curadoria'}]
        with self.assertRaisesRegex(github.GitHubError,'field_incompatible'):
            self.board.synchronize([],True)
        with mock.patch.object(github,'ProjectBoard',return_value=self.board),mock.patch.object(github,'GitHub'):
            with self.assertRaisesRegex(github.GitHubError,'field_incompatible'):
                github.project_setup(self.config,True)
        self.assertEqual(self.queries,[])
    def test_incomplete_item_fields_prevent_project_writes(self):
        self.items=[{'id':'ITEM','content':{'id':'ISSUE'},'fieldValues':{'nodes':[],'pageInfo':{'hasNextPage':True}}}]
        with self.assertRaisesRegex(github.GitHubError,'pagination_incomplete'):
            self.board.synchronize([self.issue],True)
        self.assertEqual(len(self.queries),1)


if __name__=='__main__':unittest.main()
