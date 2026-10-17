# SPDX-License-Identifier: Apache-2.0
"""Build offline content; loaded educational prose has independent rights."""
import json
from import_inaturalist import ROOT, save_json
from content_governance import annotate_education, load
import hashlib

RANKS=["species","genus","family","order","class","phylum","kingdom"]
GROUP_SOURCE = "group_education.json"


def load_group_content(root):
    """Read prose only from this workspace; never fall back to the active catalog."""
    content = json.loads((root / "resources/data" / GROUP_SOURCE).read_text(encoding="utf8"))
    if not isinstance(content, dict) or content.get("schema_version") != 1:
        raise ValueError("Unsupported group education schema")
    definitions, notes = content.get("definitions"), content.get("plain_notes")
    if not isinstance(definitions, list) or not isinstance(notes, dict):
        raise ValueError("Invalid group education structure")
    names = set()
    for item in definitions:
        if (not isinstance(item, list) or len(item) != 7 or any(not isinstance(v, str) for v in item)
                or not item[0] or item[0] in names or item[1] not in RANKS + ["educational_kingdom"]
                or not item[2].startswith("https://") or not item[3] or not item[4]):
            raise ValueError("Invalid or duplicate group definition")
        names.add(item[0])
    if any(name not in names or not isinstance(values, list) or len(values) != 2
           or any(not isinstance(v, str) for v in values) for name, values in notes.items()):
        raise ValueError("Invalid group fallback notes")
    return definitions, notes


def load_profiles(root=None):
    root = root or ROOT
    profiles=json.loads((root/"resources/data/species_profiles.json").read_text(encoding="utf-8"))
    catalog=json.loads((root/"resources/data/species.json").read_text(encoding="utf-8"))
    identities={str(s["id"]):s["name"] for s in catalog}
    if set(profiles)!=set(identities):
        raise ValueError("As fichas devem corresponder exatamente ao catálogo ativo.")
    required={"summary","distribution","morphology","conservation","hint","curiosity"}
    for sid,profile in profiles.items():
        if profile.get("scientific_name")!=identities[sid] or not profile.get("display_name"):
            raise ValueError("Identidade incompatível na ficha: "+sid)
        for field in required | ({"nutrition"} if "nutrition" in profile else set()):
            item=profile.get(field,{})
            if not item.get("text") or not item.get("source_url","").startswith("https://"):
                raise ValueError("Nota sem texto, revisão ou fonte: "+sid+":"+field)
        conservation=profile["conservation"]
        if conservation.get("status") not in {"LC","NT","VU","EN","CR","EX","EW","DD","unconfirmed"}:
            raise ValueError("Categoria de conservação inválida: "+sid)
        if conservation["status"]!="unconfirmed" and conservation.get("scope_name")!="Espécie, global":
            raise ValueError("Não transferir avaliação regional ou de outro táxon: "+sid)
    return profiles

def build(root=None, *, persist=True):
    root = root or ROOT
    taxons=json.loads((root/"resources/data/taxonomy.json").read_text(encoding="utf-8"))
    data={"notes":{},"taxons":{},"group_hints":{},"group_curiosities":{},"species":{}}
    previous = load(root / 'resources/data/education.json', {})
    definitions, plain_notes = load_group_content(root)
    for number,(name,rank,url,title,description,hint,curiosity) in enumerate(definitions):
        hint,curiosity=plain_notes.get(name,(hint,curiosity))
        active_rank = "kingdom" if rank == "educational_kingdom" else rank
        nodes=[t for t in taxons if t["name"]==name and t["rank"]==active_rank]
        if not nodes: continue
        for kind,text in (("intro",description),("hint",hint),("curiosity",curiosity)):
            if not text: continue
            key=f"note:{number}:{kind}"
            data["notes"][key]={"text":text,"source_title":title,"source_url":url,"verified":previous.get('notes', {}).get(key, {}).get('verified', False),"ranks":RANKS,
                                "reviewed_on":previous.get('notes', {}).get(key, {}).get('reviewed_on'),"scope_name":name}
            for node in nodes:
                if kind=="intro": data["taxons"][node["id"]]=key
                elif kind=="hint": data["group_hints"][node["id"]]=[key]
                else: data["group_curiosities"][node["id"]]=key
        # Educational kingdom notes also supply explicitly scoped general biology.
        if rank=="educational_kingdom":
            for node in taxons:
                if node["rank"]=="kingdom" and node["name"]==name:
                    if hint: data["group_hints"][node["id"]]=[f"note:{number}:hint"]
                    if curiosity: data["group_curiosities"][node["id"]]=f"note:{number}:curiosity"
    for sid,profile in load_profiles(root).items():
        values={key:profile[key] for key in ("scientific_name","common_name","display_name","reviewed_on")}
        for field in ("summary","distribution","nutrition","morphology","conservation","hint","curiosity"):
            if field not in profile: continue
            key=f"species:{sid}:{field}"
            data["notes"][key]=profile[field]
            values["hints" if field=="hint" else field]=[key] if field=="hint" else key
        data["species"][sid]=values
    policy = load(root / 'resources/data/phylogeny.json', {})
    for species in load(root / 'resources/data/species.json', []):
        if 'classification' not in species: continue
        if not policy.get('sources'): raise ValueError('Enquadramento filogenético sem fontes ou política.')
        data['species'][str(species['id'])]['phylogeny'] = dict(species['classification'],
            sources=[policy['sources'][key] for key in dict.fromkeys(species['classification']['source_refs'] + policy.get('alternative_source_refs', []))],
            system_limitations=policy['limitations'])
    hints=json.loads((root/"resources/data/hint_education.json").read_text(encoding="utf8"))
    profiles=load_profiles(root)
    reviews=json.loads((root/"resources/data/curation_reviews.json").read_text(encoding="utf8"))
    if set(reviews)!=set(profiles): raise ValueError("Revisões não correspondem ao catálogo.")
    states={"pending","in_review","approved","rejected"}
    for sid,review in reviews.items():
        if review.get("scientific_name")!=profiles[sid]["scientific_name"]: raise ValueError("Revisão de outra espécie: "+sid)
        if any(review.get(field) not in states for field in ("taxonomy","photo","etymology","morphology","education")):
            raise ValueError("Estado de revisão inválido: "+sid)
        data["species"][sid]["review"]=review
    for sid,content in hints.items():
        if sid not in profiles or content.get("scientific_name")!=profiles[sid]["scientific_name"]:
            raise ValueError("Identidade incompatível no corpus de dicas: "+sid)
        values=data["species"][sid]
        if 'taxonomic_assistance' in content:
            values['taxonomic_assistance'] = content['taxonomic_assistance']
        values["morphology_traits"]=content.get("morphology_traits",[])
        for rank,item in content.get("etymology_hints",{}).items():
            url=item.get("source_url","")
            if rank not in RANKS or not url.startswith("https://") or "inaturalist.org" in url or not item.get("text"):
                raise ValueError("Etimologia sem fonte ou nível válido: "+sid)
            key=f"species:{sid}:etymology:{rank}"
            data["notes"][key]=dict(item,ranks=[rank])
            values.setdefault("etymology_hints",{})[rank]=key
        if "name_origin" in content:
            refs=[values.get("etymology_hints",{}).get(rank) for rank in content["name_origin"]]
            if not refs or not all(refs):raise ValueError("Origem sem etimologia referenciada: "+sid)
            values["name_origin"]={"parts":refs}
        if "morphological_hint" in content:
            review=content["morphological_hint"];reference=review["reference"]
            if reference not in profiles[sid]:raise ValueError("Morfologia sem conteúdo revisado: "+sid)
            if not review.get("photo_id") or not review.get("ranks") or any(rank not in RANKS for rank in review["ranks"]):
                raise ValueError("Morfologia sem fotografia ou nível válido: "+sid)
            item=dict(profiles[sid][reference]);item.update(ranks=review["ranks"],photo_id=review["photo_id"])
            if 'assistance' in review: item['assistance']=review['assistance']
            item["text"]=review.get("prefix","")+item["text"]
            key=f"species:{sid}:morphological_hint"
            data["notes"][key]=item;values["morphological_hint"]=key
    # A URL, a legacy verified flag, or successful generation is never a review.
    # Newly generated/AI-assisted content is draft until a human records evidence.
    annotate_education(root, data)
    from review_hints import valid
    automatic = load(root / 'resources/data/progressive_hints.json', {})
    data['automated_pedagogy'] = dict(automatic, usable=bool(automatic and valid(automatic, root)))
    source_names = [ 'species.json', 'taxonomy.json', 'species_profiles.json', 'hint_education.json', 'curation_reviews.json', GROUP_SOURCE]
    if automatic: source_names.append('progressive_hints.json')
    if (root / 'resources/data/phylogeny.json').is_file(): source_names.append('phylogeny.json')
    data['generated_source_sha256'] = {name: hashlib.sha256((root / 'resources/data' / name).read_bytes()).hexdigest()
        for name in source_names}
    if persist:
        save_json(root/"resources/data/education.json",data)
        print("Introductions:",len(data["taxons"]),"species-specific reviews:",len(data["species"]))
    return data

if __name__=="__main__": build()
