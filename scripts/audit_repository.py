"""Inspect Git's index without exposing suspected secrets or modifying tracked content."""
import argparse
from io import BytesIO
import json
from pathlib import Path, PurePosixPath
import re
import subprocess

ROOT=Path(__file__).resolve().parents[1]
ARTIFACTS={'.exe','.dll','.o','.obj','.pdb','.ilk','.sqlite','.sqlite3','.db','.log','.pyc','.tmp','.part','.bak','.dmp','.stackdump','.p12','.pfx'}
PRIVATE_NAMES={'id_rsa','id_ed25519','credentials.json'}
SECRET_PATTERNS={
    'private_key':re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
    'github_token':re.compile(rb'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,})\b'),
    'aws_access_key':re.compile(rb'\b(?:AKIA|ASIA)[A-Z0-9]{16}\b'),
    'discord_token':re.compile(rb'\bmfa\.[A-Za-z0-9_-]{80,}\b')}


def path_issues(name):
    path=PurePosixPath(name);parts=path.parts;lower=name.lower();issues=[]
    if path.suffix.lower() in ARTIFACTS or any(p.lower() in {'build','cmakefiles','__pycache__','.aws','.codex','.agents','.vs'} or p.lower().startswith(('build-','cmake-build-')) for p in parts):issues.append('generated_or_local_artifact')
    if path.name.lower() in PRIVATE_NAMES or (path.name.lower().startswith('.env') and path.name.lower()!='.env.example'):issues.append('credential_container')
    if lower.startswith('package/') and name not in {'package/run_glossata_game.cmd','package/qt.conf'}:issues.append('deployed_runtime_not_source')
    if path.name in {'CMakeCache.txt','build.ninja','compile_commands.json','.governance.lock'}:issues.append('generated_or_local_artifact')
    return sorted(set(issues))


def audit(root=ROOT):
    index=subprocess.check_output(['git','ls-files','--stage','-z'],cwd=root)
    entries=[]
    for entry in index.split(b'\0'):
        if not entry:continue
        metadata,name=entry.split(b'\t',1);mode,oid,stage=metadata.split()
        entries.append((name.decode('utf8'),oid.decode(),mode,stage))
    # Read the staged blobs, never an unstaged replacement or a symlink target.
    raw=subprocess.check_output(['git','cat-file','--batch'],cwd=root,input=b''.join(oid.encode()+b'\n' for _,oid,_,_ in entries))
    stream=BytesIO(raw);findings=[]
    for name,oid,mode,stage in entries:
        header=stream.readline().split();body=stream.read(int(header[2]));stream.read(1)
        issues=path_issues(name)
        if stage!=b'0':issues.append('unmerged_index_entry')
        if mode==b'120000':issues.append('symlink_requires_explicit_review')
        if b'\0' not in body:
            issues.extend(code for code,pattern in SECRET_PATTERNS.items() if pattern.search(body))
        findings.extend({'path':name,'issue':issue} for issue in sorted(set(issues)))
    return {'schema_version':1,'scope':'Current staged blobs and paths; not a full Git-history or universal secret scan.',
            'tracked_files':len(entries),'findings':findings,'values_redacted':True}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--staged',action='store_true',help='Pre-commit/index check; do not write a report')
    args=parser.parse_args();report=audit()
    if not args.staged:
        path=ROOT/'build/repository-audit.json';path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
    print(json.dumps(report,ensure_ascii=True))
    return bool(report['findings'])


if __name__=='__main__':raise SystemExit(main())
