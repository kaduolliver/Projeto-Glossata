"""Composition only: register a new adapter without changing reconciliation/workflow."""
import importlib
import json
from pathlib import Path


class Registry:
    def __init__(self):self.entries={}

    def register(self, name, provider, capabilities):
        if not name or provider.name!=name:raise ValueError('Provider registration and identity must agree')
        if name in self.entries:raise ValueError('Duplicate provider registration: '+name)
        self.entries[name]=(provider,frozenset(capabilities))

    def with_capability(self, capability, selected=None):
        selected=set(self.entries) if selected is None else set(selected)
        unknown=selected-set(self.entries)
        if unknown:raise ValueError('Provider not integrated: '+', '.join(sorted(unknown)))
        return [p for name,(p,caps) in self.entries.items() if name in selected and capability in caps]


def configured_registry(client, path=None):
    plan=json.loads((Path(path) if path else Path(__file__).with_name('provider_plan.json')).read_text(encoding='utf8'))
    if plan.get('schema_version')!=1:raise ValueError('Unsupported provider configuration schema')
    registry=Registry()
    for name,spec in plan['providers'].items():
        if not spec.get('adapter'):continue
        module,classname=spec['adapter'].split(':')
        adapter=getattr(importlib.import_module(module),classname)(client,**spec.get('options',{}))
        registry.register(name,adapter,spec['capabilities'])
    return registry
