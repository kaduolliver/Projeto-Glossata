"""Guard the boundary between maintenance candidates and shipped content."""
import json
from pathlib import Path
import xml.etree.ElementTree as ET


def staging_errors(root):
    root=Path(root);errors=[]
    qrc=root/'resources.qrc'
    if qrc.exists():
        for item in ET.parse(qrc).getroot().iter('file'):
            name=item.text or ''
            if name.startswith('build/') or 'candidates.sqlite' in name:
                errors.append({'error':'candidate_artifact_bundled','path':name})
            path=root/name
            if path.suffix=='.json' and path.is_file():
                data=json.loads(path.read_bytes())
                if isinstance(data,dict) and data.get('state')=='candidates_only':
                    errors.append({'error':'candidate_report_bundled','path':name})
    species=root/'resources/data/species.json'
    if species.is_file():
        for row in json.loads(species.read_bytes()):
            if row.get('candidate_id') or str(row.get('id','')).startswith('candidate:'):
                errors.append({'error':'candidate_in_active_catalog','id':row.get('id')})
        from .admission import new_entry_errors
        errors.extend(new_entry_errors(root))
    return errors
