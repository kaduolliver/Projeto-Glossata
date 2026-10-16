"""Small contracts; the workflow does not know any remote API schema."""
from typing import Protocol
from .model import Assertion, DiscoveryScope


class TaxonomyProvider(Protocol):
    name: str
    def search(self, name: str, limit: int = 5) -> list[Assertion]: ...
    def fetch(self, record_id: str) -> Assertion: ...


class DiscoveryProvider(Protocol):
    name: str
    def discover(self, scope: DiscoveryScope, limit: int = 5, offset: int = 0) -> list[Assertion]: ...


class MediaProvider(Protocol):
    name: str
    def search_media(self, name: str, limit: int = 5) -> list[Assertion]: ...
    def fetch_media(self, identifier: str) -> list[Assertion]: ...
