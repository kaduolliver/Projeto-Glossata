"""Fetch missing photographs and audit source links without changing species IDs.

New candidates require visual review. Existing photographs are never replaced
automatically; the maintenance report records source disagreements and gaps.
"""
import argparse
import hashlib
import json
import pathlib
import struct
from datetime import datetime, timezone
from import_inaturalist import Api, ROOT, Taxonomy, download_photo, permitted_photo, LICENSE_URLS, save_json

def image_dimensions(path):
    data=path.read_bytes()
    if data.startswith(b"\x89PNG") and len(data) >= 24:
        return struct.unpack(">II",data[16:24])
    if not data.startswith(b"\xff\xd8"): return (0,0)
    i=2
    while i+4<len(data):
        if data[i]!=255: return (0,0)
        marker=data[i+1]; i+=2
        if marker in (216,217): continue
        length=int.from_bytes(data[i:i+2],"big")
        if marker in (192,193,194,195,197,198,199,201,202,203,205,206,207):
            return (int.from_bytes(data[i+5:i+7],"big"),int.from_bytes(data[i+3:i+5],"big"))
        if length<2: return (0,0)
        i+=length
    return (0,0)


def photo_metadata(photo, observation, species, path, url):
    return {"id":str(photo["id"]), "scientific_name":observation["taxon"]["name"],
            "taxon_id":str(observation["taxon"]["id"]), "license":photo["license_code"],
            "license_url":LICENSE_URLS[photo["license_code"]], "author":photo.get("attribution",""),
            "source":f"https://www.inaturalist.org/observations/{observation['id']}",
            "url":url, "path":path, "sha256":hashlib.sha256((ROOT/path).read_bytes()).hexdigest(),
            "modifications":"Nenhuma. Arquivo original, sem recorte ou retoque.",
            "research_grade":observation.get("quality_grade")=="research", "visual_quiz":False, "approved_ranks":[],
            "review":"Candidato: requer triagem do organismo e do nível de identificação."}


def audit(offline=True):
    # Compatibility entry point: maintenance now emits proposals exclusively.
    # Existing files, taxonomy, licenses and review dates must not be rewritten
    # by an HTTP response, including a later license or identification change.
    from review_updates import propose_updates, write_proposals
    report = propose_updates(ROOT, online=not offline)
    print(write_proposals(report, ROOT))
    return report

if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--offline",action="store_true")
    parser.add_argument("--online",action="store_true")
    args = parser.parse_args()
    if args.online and args.offline: parser.error('Escolha somente --online ou --offline.')
    audit(not args.online)
