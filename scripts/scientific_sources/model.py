"""Provider-neutral assertions, not approved Glossata organisms."""
from dataclasses import dataclass, field, asdict
import hashlib
import json
import unicodedata


def canonical(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(',', ':')).encode('utf8')


def digest(value):
    return hashlib.sha256(canonical(value)).hexdigest()


def name_key(name):
    # No spelling correction, authorship stripping, accent removal or fuzzy match.
    return ' '.join(unicodedata.normalize('NFC', name).casefold().split())


@dataclass(frozen=True)
class Identity:
    provider: str
    record_id: str
    dataset: str = ''

    @property
    def key(self):
        return canonical([self.provider, self.dataset, self.record_id]).decode('utf8')


@dataclass
class Assertion:
    identity: Identity
    scientific_name: str
    canonical_name: str
    rank: str
    status: str
    authorship: str = ''
    accepted: Identity | None = None
    accepted_name: str = ''
    lineage: list = field(default_factory=list)
    external_ids: list = field(default_factory=list)
    references: list = field(default_factory=list)
    rights: dict = field(default_factory=dict)
    provenance: list = field(default_factory=list)
    source_updated_at: str | None = None
    kind: str = 'taxon'
    media: dict = field(default_factory=dict)

    def serialize(self):
        return asdict(self)

    @classmethod
    def from_dict(cls, data):
        values=dict(data);values['identity']=Identity(**values['identity'])
        if values.get('accepted'):values['accepted']=Identity(**values['accepted'])
        return cls(**values)


@dataclass(frozen=True)
class DiscoveryScope:
    id: str
    label: str
    membership: tuple[str, ...]
    roots: dict
    target: int = 10
