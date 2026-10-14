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
    if data.startswith(b"\x89PNG"):
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


def audit(offline=False):
    species=json.loads((ROOT/"resources/data/species.json").read_text(encoding="utf-8"))
    reviews=json.loads((ROOT/"resources/data/photo_reviews.json").read_text(encoding="utf-8"))
    api=Api(offline)
    taxonomy=Taxonomy(json.loads((ROOT/"resources/data/taxonomy.json").read_text(encoding="utf-8")))
    report={"date":datetime.now(timezone.utc).isoformat(),"candidates":[],"source_audit":[],"failures":[]}
    for s in species:
        p=s.get("photo")
        if p:
            try:
                observation=api.get("observations/"+p["source"].split("/")[-1])["results"][0]
                taxon=observation.get("taxon") or {}
                actual=next((x for x in observation["photos"] if str(x["id"])==p["id"]),None)
                if actual and min(image_dimensions(ROOT/p["path"]))<300:
                    path,url=download_photo(actual,offline,size="large",refresh=True)
                    p.update(url=url,sha256=hashlib.sha256((ROOT/path).read_bytes()).hexdigest())
                reliable=(observation.get("quality_grade")=="research" or
                          observation.get("captive") and observation.get("identifications_count",0)>=2)
                expected_name=p.get("scientific_name",s["name"]) if p.get("role")=="supporting" else s["name"]
                expected_id=int(p.get("taxon_id") or 0)
                taxon_match=(taxon.get("name")==expected_name or expected_id in taxon.get("ancestor_ids",[]))
                match=(reliable
                       and observation.get("community_taxon_id")==taxon.get("id")
                       and taxon_match and taxon.get("is_active") and actual and permitted_photo(actual))
                p.update(scientific_name=expected_name if match else taxon.get("name",""),
                         taxon_id=str(expected_id if match else taxon.get("id","")),
                         source_verified=bool(match),
                         research_grade=observation.get("quality_grade")=="research",
                         modifications="Nenhuma. Arquivo original, sem recorte ou retoque.")
                # Some CC0 responses omit the author. Use only a matching photo ID.
                if p.get("author")=="no rights reserved":
                    details=api.get("taxa/"+str(taxon["id"]))["results"][0]
                    exact=next((x["photo"] for x in details.get("taxon_photos",[]) if str(x["photo"]["id"])==p["id"]),{})
                    if exact.get("attribution_name"):
                        p["author"]=exact["attribution_name"]
                review_key=s.get("source_taxon_id") or "legacy:"+str(s["id"])
                review=reviews.get(review_key,{})
                if s.get("source")=="legacy" and match:
                    details=api.get("taxa/"+str(taxon["id"]))["results"][0]
                    s["taxonomy"]=taxonomy.imported_lineage(details,s["educational_kingdom"])
                    s["family"]=next((t["name"] for t in details["ancestors"] if t["rank"]=="family"),s["family"])
                # Rank decisions belong to the per-photo review manifest. A
                # community identification alone never approves a question.
                ranks=review.get("approved_ranks",[]) if match and review.get("photo_id")==p["id"] else []
                if p.get("author")=="no rights reserved": ranks=[]
                p["approved_ranks"]=ranks; p["visual_quiz"]=p.get("role","identification")=="identification" and "species" in ranks
                report["source_audit"].append({"id":s["id"],"photo_id":p["id"],"source_match":bool(match),"approved_ranks":ranks,
                     "author_available":p.get("author")!="no rights reserved"})
            except Exception as error:
                # Missing cache/network does not revoke a previously recorded
                # review. Only an actual source disagreement does so above.
                report["failures"].append({"id":s["id"],"reason":str(error)})
            continue
        try:
            taxons=api.get("taxa",q=s["name"],rank="species",per_page=30)["results"]
            taxon=next((t for t in taxons if t["name"]==s["name"] and t.get("is_active")),None)
            if not taxon:
                report["failures"].append({"id":s["id"],"reason":"Nome exato e espécie ativa não encontrados; não usar outro organismo."}); continue
            rows=api.get("observations",taxon_id=taxon["id"],quality_grade="research,casual",rank="species",
                         photos="true",photo_license="cc-by,cc0",per_page=30,order_by="votes",order="desc")["results"]
            selected=None
            for o in rows:
                if o.get("community_taxon_id")!=taxon["id"] or o.get("taxon",{}).get("name")!=s["name"]: continue
                if o.get("quality_grade")!="research" and not (o.get("captive") and o.get("identifications_count",0)>=2): continue
                photo=next((x for x in o["photos"] if permitted_photo(x) and x.get("attribution")!="no rights reserved"),None)
                if photo: selected=(o,photo); break
            if not selected:
                report["failures"].append({"id":s["id"],"reason":"Sem foto elegível com autoria verificável."}); continue
            observation,photo=selected
            path,url=download_photo(photo,offline)
            s["photo"]=photo_metadata(photo,observation,s,path,url)
            s["image"]="qrc:/"+path
            report["candidates"].append({"id":s["id"],"name":s["name"],"path":path,"photo_id":str(photo["id"]),"taxon_id":str(taxon["id"])})
            print("candidate",s["id"],s["name"],flush=True)
        except Exception as error:
            report["failures"].append({"id":s["id"],"reason":str(error)})
    taxonomy.reconcile_legacy(species)
    save_json(ROOT/"resources/data/taxonomy.json",list(taxonomy.nodes.values()))
    save_json(ROOT/"resources/data/species.json",species)
    save_json(ROOT/"resources/data/photo_audit.json",report)
    return report

if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--offline",action="store_true")
    audit(parser.parse_args().offline)
