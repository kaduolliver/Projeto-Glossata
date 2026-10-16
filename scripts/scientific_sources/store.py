"""Dedicated candidate DB, immutable evidence/revisions, no publication capability."""
from datetime import datetime, timezone
import json
from pathlib import Path
import sqlite3
import uuid
from .model import canonical, digest


class CandidateStore:
    def __init__(self, path):
        self.path=Path(path);self.path.parent.mkdir(parents=True,exist_ok=True)
        self.db=sqlite3.connect(self.path)
        tables={row[0] for row in self.db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
        if tables-{'candidate_store_meta','candidates','revisions'}:
            self.db.close();raise ValueError('Refusing to modify an existing non-candidate database')
        self.db.execute('PRAGMA foreign_keys=ON')
        with self.db:
            self.db.execute('BEGIN IMMEDIATE')
            self.db.execute('CREATE TABLE IF NOT EXISTS candidate_store_meta(version INTEGER NOT NULL)')
            version=self.db.execute('SELECT version FROM candidate_store_meta').fetchone()
            if version and version[0]!=1:raise ValueError('Unsupported candidate store schema; no migration applied')
            if not version:self.db.execute('INSERT INTO candidate_store_meta VALUES(1)')
            self.db.execute('''CREATE TABLE IF NOT EXISTS candidates(
                id TEXT PRIMARY KEY, identity TEXT NOT NULL UNIQUE, latest_sha TEXT NOT NULL,
                scientific_state TEXT NOT NULL CHECK(scientific_state='pending'),
                legal_state TEXT NOT NULL CHECK(legal_state='pending'))''')
            self.db.execute('''CREATE TABLE IF NOT EXISTS revisions(
                candidate_id TEXT NOT NULL REFERENCES candidates(id), sha TEXT NOT NULL, payload TEXT NOT NULL,
                recorded_at TEXT NOT NULL, PRIMARY KEY(candidate_id,sha))''')

    def close(self):self.db.close()

    def put_many(self, assertions):
        counts={'new':0,'changed':0,'unchanged':0}
        with self.db:
            for record in assertions:
                if not record.identity.provider or not record.identity.record_id:
                    raise ValueError('Candidate requires a scoped external identity')
                data=record.serialize();sha=digest(data)
                identifier='candidate:'+str(uuid.uuid5(uuid.NAMESPACE_URL,record.identity.key))
                previous=self.db.execute('SELECT latest_sha FROM candidates WHERE identity=?',(record.identity.key,)).fetchone()
                self.db.execute("INSERT INTO candidates VALUES(?,?,?,'pending','pending') ON CONFLICT(identity) DO UPDATE SET latest_sha=excluded.latest_sha",
                                (identifier,record.identity.key,sha))
                self.db.execute('INSERT OR IGNORE INTO revisions VALUES(?,?,?,?)',
                    (identifier,sha,canonical(data).decode('utf8'),datetime.now(timezone.utc).isoformat()))
                counts['new' if not previous else 'unchanged' if previous[0]==sha else 'changed']+=1
        return counts

    def latest(self):
        result=[]
        for identifier,state,legal,payload,sha in self.db.execute('SELECT c.id,c.scientific_state,c.legal_state,r.payload,r.sha FROM candidates c JOIN revisions r ON r.candidate_id=c.id AND r.sha=c.latest_sha ORDER BY c.identity'):
            data=json.loads(payload)
            if digest(data)!=sha:raise ValueError('Candidate revision hash mismatch')
            result.append({'candidate_id':identifier,'scientific_state':state,'legal_state':legal,'assertion':data})
        return result
