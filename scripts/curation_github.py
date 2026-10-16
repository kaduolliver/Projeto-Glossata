"""Bounded GitHub Issues/Projects adapter. No scientific decisions are imported."""
import json
import os
import re
import time
import urllib.error
import urllib.parse
import urllib.request

from curation_requests import render

START = '<!-- glossata-curation:start -->'
END = '<!-- glossata-curation:end -->'
MARKER = re.compile(r'<!-- glossata-curation:v1:(curation-v1-[0-9a-f]{24}) -->')


class GitHubError(ValueError):
    """Only controlled error codes, never raw responses containing personal data."""


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self,*args,**kwargs):
        raise GitHubError('github_redirect_refused')


class GitHub:
    def __init__(self, token, limit=200, transport=None, pace=1.0):
        if not token: raise GitHubError('github_token_missing')
        self.token=token;self.limit=limit;self.calls=0;self.pace=pace;self.last=0
        self.transport=transport or urllib.request.build_opener(NoRedirect()).open

    def request(self, method, path, data=None):
        if not path.startswith('/') or path.startswith('//') or '..' in path:
            raise GitHubError('github_path_refused')
        if self.calls>=self.limit:raise GitHubError('github_request_budget_exceeded')
        self.calls+=1
        if method!='GET':time.sleep(max(0,self.pace-(time.monotonic()-self.last)))
        headers={'Authorization':'Bearer '+self.token, 'Accept':'application/vnd.github+json',
                 'X-GitHub-Api-Version':'2026-03-10','User-Agent':'ProjetoGlossata-Curation/1.0'}
        body=json.dumps(data,ensure_ascii=False).encode() if data is not None else None
        if body is not None:headers['Content-Type']='application/json'
        request=urllib.request.Request('https://api.github.com'+path,data=body,headers=headers,method=method)
        try:
            with self.transport(request,timeout=30) as response:
                raw=response.read(8*1024*1024+1)
                if len(raw)>8*1024*1024:raise GitHubError('github_response_too_large')
                result=json.loads(raw) if raw else {}
                if isinstance(result,dict) and result.get('errors'):raise GitHubError('github_graphql_refused')
                return result
        except json.JSONDecodeError:
            raise GitHubError('github_invalid_json_no_retry') from None
        except urllib.error.HTTPError as e:
            # No write is retried: an interrupted POST may already have succeeded.
            raise GitHubError('github_http_'+str(e.code)) from None
        except (urllib.error.URLError,TimeoutError,OSError):
            raise GitHubError('github_transport_error_no_retry') from None
        finally:
            if method!='GET':self.last=time.monotonic()

    def pages(self,path,maximum):
        for page in range(1,maximum+1):
            values=self.request('GET',path+('&' if '?' in path else '?')+f'per_page=100&page={page}')
            if not isinstance(values,list):raise GitHubError('github_invalid_page')
            yield from values
            if len(values)<100:return
        raise GitHubError('github_pagination_incomplete')


def labels(config):
    result={config['managed_label']:('385A64','Solicitação organizada automaticamente; sem aprovação de conteúdo.')}
    result.update({'curadoria:'+area:('315A82','Área da solicitação de curadoria.') for area in
                   ('taxonomy','photography','science','pedagogy','licensing','integrity')})
    result.update({f'curadoria:P{i}':(color,'Prioridade de triagem; não certifica gravidade jurídica.')
                   for i,color in enumerate(('B60205','D93F0B','E4B544','6F7A83'))})
    result.update({key:('405A69','Estado administrativo; não altera a governança.') for key in config['statuses']})
    return result


def merge_block(existing, incoming):
    if existing.count(START)!=1 or existing.count(END)!=1 or existing.index(END)<existing.index(START):
        raise GitHubError('github_managed_block_ambiguous')
    a=existing.index(START);b=existing.index(END)+len(END)
    c=incoming.index(START);d=incoming.index(END)+len(END)
    return existing[:a]+incoming[c:d]+existing[b:]


def administrative_state(issue, config):
    selected=[label['name'] for label in issue.get('labels',[]) if label['name'] in config['statuses']]
    return config['statuses'][selected[0]] if len(selected)==1 else None


def sync(queue,config,revision,apply=False,client=None,board=None):
    if queue.get('repository')!=config['repository']:raise GitHubError('github_repository_mismatch')
    if not re.fullmatch('[a-f0-9]{40}',revision):raise GitHubError('github_source_revision_invalid')
    result={'apply':apply,'remote_checked':False,'created':[],'updated':[],'unchanged':[],
            'deferred':[],'closed_unresolved':[],'conflicts':[],'errors':[],
            'project':'not_configured','notice':'Issue/project states never authorize content or media.'}
    if client is None and not os.environ.get('GH_TOKEN') and not apply:
        result['deferred']=[r['id'] for r in queue['requests']]
        result['notice']='Offline preview; no token, network call or publication.'
        return result
    client=client or GitHub(os.environ.get('GH_TOKEN'),config['max_api_calls'])
    repo='/repos/'+config['repository']
    try:
        metadata=client.request('GET',repo)
        if metadata.get('full_name','').casefold()!=config['repository'].casefold() or metadata.get('archived') or not metadata.get('has_issues',True):
            raise GitHubError('github_repository_not_writable')
        # Prevent links to unpublished evidence and publication from an unpushed local tree.
        commit=client.request('GET',repo+'/commits/'+revision)
        if commit.get('sha')!=revision:raise GitHubError('github_source_revision_not_available')
        issues=list(client.pages(repo+'/issues?state=all',config['max_pages']))
        managed={}
        for issue in issues:
            if issue.get('pull_request'):continue
            ids=MARKER.findall(issue.get('body') or '')
            if len(ids)==1:managed.setdefault(ids[0],[]).append(issue)
        result['remote_checked']=True
        if apply:
            existing={label['name'] for label in client.pages(repo+'/labels',config['max_pages'])}
            for name,(color,description) in labels(config).items():
                if name not in existing:client.request('POST',repo+'/labels',{'name':name,'color':color,'description':description})
        project_issues=[]
        for request in queue['requests']:
            rid=request['id'];matches=managed.get(rid,[])
            if len(matches)>1:
                result['conflicts'].append(rid);continue
            if matches and matches[0].get('state')=='closed':
                result['closed_unresolved'].append(rid);project_issues.append(matches[0]);continue
            body=render(request,config['repository'],revision,config['max_body_rows'])
            expected_labels=[config['managed_label'],'curadoria:'+request['area'],f"curadoria:P{request['priority']}"]
            if not matches:
                if len(result['created'])>=config['max_creates']:
                    result['deferred'].append(rid);continue
                if not apply:
                    result['deferred'].append(rid);continue
                issue=client.request('POST',repo+'/issues',{'title':f"[Curadoria] {request['area']} / {request['bucket']}",
                    'body':body,'labels':expected_labels+['curadoria:pendente']})
                result['created'].append({'id':rid,'number':issue['number']})
                managed[rid]=[issue];project_issues.append(issue);continue
            issue=matches[0]
            # Only blocks explicitly marked as managed are touched. Public/manual
            # issues are not ingested or interpreted as commands or permissions.
            if config['managed_label'] not in [v['name'] for v in issue.get('labels',[])]:
                result['conflicts'].append(rid);continue
            fingerprint='<!-- glossata-fingerprint:'+request['fingerprint']+' -->'
            if fingerprint in (issue.get('body') or ''):
                result['unchanged'].append(rid);project_issues.append(issue);continue
            if not apply or len(result['updated'])>=config['max_updates']:
                result['deferred'].append(rid);continue
            # Fresh read preserves notes/labels edited after the paginated listing.
            fresh=client.request('GET',repo+'/issues/'+str(issue['number']))
            if fresh.get('state')=='closed':result['closed_unresolved'].append(rid);continue
            if MARKER.findall(fresh.get('body') or '')!=[rid]:raise GitHubError('github_issue_identity_changed')
            merged=merge_block(fresh.get('body') or '',body)
            human_labels=[v['name'] for v in fresh.get('labels',[]) if not re.fullmatch('curadoria:P[0-3]',v['name'])]
            if not any(v in config['statuses'] for v in human_labels):human_labels.append('curadoria:pendente')
            issue=client.request('PATCH',repo+'/issues/'+str(issue['number']),
                {'body':merged,'labels':sorted(set(human_labels+expected_labels))})
            result['updated'].append({'id':rid,'number':issue['number']});project_issues.append(issue)
        # Public forms and manual triage join the same Project. Only IDs and
        # administrative labels are used; issue text never becomes a decision.
        known_nodes={i['node_id'] for i in project_issues}
        areas={'curadoria:'+a for a in ('taxonomy','photography','science','pedagogy','licensing','integrity')}
        for issue in issues:
            if not issue.get('pull_request') and issue.get('node_id') not in known_nodes and areas.intersection(v['name'] for v in issue.get('labels',[])):
                project_issues.append(issue);known_nodes.add(issue['node_id'])
        if board is None and os.environ.get('GLOSSATA_PROJECT_ID'):
            board=ProjectBoard(GitHub(os.environ.get('GLOSSATA_PROJECT_TOKEN'),config['max_api_calls']),
                               os.environ['GLOSSATA_PROJECT_ID'],config)
        if board is not None:
            result['project']=board.synchronize(project_issues,apply)
    except GitHubError as e:
        result['errors'].append(str(e))
    return result


class ProjectBoard:
    def __init__(self,client,project_id,config):
        if not re.fullmatch('[A-Za-z0-9_-]+',project_id):raise GitHubError('project_id_invalid')
        self.client=client;self.id=project_id;self.config=config

    def query(self,query,variables):
        return self.client.request('POST','/graphql',{'query':query,'variables':variables})['data']

    def fields(self):
        # No automatic rewrite of an existing field or its option IDs.
        after=None;all_fields=[]
        for _ in range(self.config['max_pages']):
            data=self.query('''query($id:ID!,$after:String){ node(id:$id){ ... on ProjectV2 {
                fields(first:100,after:$after){nodes{ ... on ProjectV2SingleSelectField {id name options{id name}} }
                pageInfo{hasNextPage endCursor}} } } }''',{'id':self.id,'after':after})
            if not data.get('node'):raise GitHubError('project_not_accessible')
            page=data['node']['fields'];all_fields+=page['nodes']
            if not page['pageInfo']['hasNextPage']:return all_fields
            after=page['pageInfo']['endCursor']
        raise GitHubError('project_fields_pagination_incomplete')

    def synchronize(self,issues,apply):
        matches=[f for f in self.fields() if f.get('name')==self.config['project_field']]
        if len(matches)!=1:raise GitHubError('project_curation_field_missing_or_ambiguous')
        field=matches[0];options={o['name']:o['id'] for o in field['options']}
        if not set(self.config['statuses'].values()).issubset(options):raise GitHubError('project_status_options_missing')
        items={};after=None
        for _ in range(self.config['max_pages']):
            data=self.query('''query($id:ID!,$after:String){ node(id:$id){ ... on ProjectV2 {
              items(first:100,after:$after){nodes{id content{ ... on Issue {id} }
              fieldValues(first:100){nodes{ ... on ProjectV2ItemFieldSingleSelectValue {optionId field{ ... on ProjectV2SingleSelectField {id} } } }}}
              pageInfo{hasNextPage endCursor}} } } }''',{'id':self.id,'after':after})
            page=data['node']['items']
            for item in page['nodes']:
                if item.get('content'):items[item['content']['id']]=item
            if not page['pageInfo']['hasNextPage']:break
            after=page['pageInfo']['endCursor']
        else:raise GitHubError('project_items_pagination_incomplete')
        changed=0;conflicts=0
        for issue in issues:
            status=administrative_state(issue,self.config)
            if not status:conflicts+=1;continue
            item=items.get(issue['node_id']);option=options[status]
            old=next((v.get('optionId') for v in (item or {}).get('fieldValues',{}).get('nodes',[])
                      if v.get('field',{}).get('id')==field['id']),None)
            if old==option:continue
            if not apply or changed>=self.config['max_project_updates']:continue
            if item is None:
                data=self.query('''mutation($project:ID!,$content:ID!){ addProjectV2ItemById(input:{projectId:$project,contentId:$content}){item{id}} }''',
                                {'project':self.id,'content':issue['node_id']})
                item=data['addProjectV2ItemById']['item'];items[issue['node_id']]=item
            self.query('''mutation($project:ID!,$item:ID!,$field:ID!,$option:String!){
              updateProjectV2ItemFieldValue(input:{projectId:$project,itemId:$item,fieldId:$field,value:{singleSelectOptionId:$option}}){projectV2Item{id}} }''',
                       {'project':self.id,'item':item['id'],'field':field['id'],'option':option})
            changed+=1
        return {'changed':changed,'ambiguous_statuses':conflicts}


def project_setup(config,apply=False):
    client=GitHub(os.environ.get('GLOSSATA_PROJECT_TOKEN'),config['max_api_calls'])
    board=ProjectBoard(client,os.environ.get('GLOSSATA_PROJECT_ID',''),config)
    existing=[f for f in board.fields() if f.get('name')==config['project_field']]
    if existing:return {'state':'exists','modified':False}
    if not apply:return {'state':'would_create_field','modified':False}
    colors=['GRAY','BLUE','YELLOW','GREEN','RED']
    values=[{'name':name,'color':color,'description':'Estado administrativo; não aprova conteúdo.'}
            for name,color in zip(config['statuses'].values(),colors)]
    board.query('''mutation($input:CreateProjectV2FieldInput!){createProjectV2Field(input:$input){projectV2Field{ ... on ProjectV2SingleSelectField {id}}}}''',
                {'input':{'projectId':board.id,'dataType':'SINGLE_SELECT','name':config['project_field'],'singleSelectOptions':values}})
    return {'state':'created_field','modified':True}
