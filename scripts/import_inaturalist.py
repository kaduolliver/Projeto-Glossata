"""Import licensed, research-grade records into the offline Glossata catalog.

Only the import command uses the network. Existing IDs/facts are retained.
Python 3.11+ standard library; no application or imaging dependencies.
"""
import argparse
import collections
import hashlib
import http.client
import json
import pathlib
import re
import time
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
KINGDOM_ROOTS = {
    "Animalia": {1: "Animalia"}, "Plantae": {47126: "Plantae"},
    "Fungi": {47170: "Fungi"}, "Protista": {47686: "Protozoa", 48222: "Chromista"},
    "Monera": {67333: "Bacteria", 151817: "Archaea"},
}
ALLOWED_LICENSES = {"cc0", "cc-by"}
LICENSE_URLS = {"cc0": "https://creativecommons.org/publicdomain/zero/1.0/",
                "cc-by": "https://creativecommons.org/licenses/by/4.0/"}


def educational_kingdom(ancestors):
    matches = {kingdom for kingdom, roots in KINGDOM_ROOTS.items()
               if any(t.get("id") in roots and t.get("name") == roots[t["id"]]
                      and t.get("rank") == "kingdom" for t in ancestors)}
    return next(iter(matches)) if len(matches) == 1 else None


def permitted_photo(photo):
    return (photo.get("license_code") in ALLOWED_LICENSES
            and not photo.get("hidden") and not photo.get("flags")
            and bool(photo.get("attribution")) and bool(photo.get("url"))
            and min(photo.get("original_dimensions", {}).get("width", 0),
                    photo.get("original_dimensions", {}).get("height", 0)) >= 320)


def apply_photo_reviews(species):
    path = ROOT / "resources/data/photo_reviews.json"
    reviews = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
    for item in species:
        if not item.get("photo"):
            continue
        photo = item["photo"]
        review = reviews.get(item.get("source_taxon_id") or "legacy:" + str(item["id"]), {})
        matched = review.get("photo_id") == photo.get("id")
        photo["approved_ranks"] = review.get("approved_ranks", []) if matched and photo.get("source_verified") else []
        photo["visual_quiz"] = "species" in photo["approved_ranks"]
        photo["review"] = review.get("note", "Triagem visual pendente.") if matched else "Triagem visual pendente."
        item["image"] = "qrc:/" + photo["path"] if photo.get("path") and (not matched or review.get("display", True)) else ""


class Api:
    def __init__(self, offline=False):
        self.cache = ROOT / "build/import-cache"
        self.cache.mkdir(parents=True, exist_ok=True)
        self.offline = offline
        self.last_request = 0

    def get(self, path, **params):
        url = "https://api.inaturalist.org/v1/" + path + "?" + urllib.parse.urlencode(params)
        cached = self.cache / (hashlib.sha256(url.encode()).hexdigest() + ".json")
        if cached.exists():
            return json.loads(cached.read_text(encoding="utf-8"))
        if self.offline:
            raise RuntimeError("Resposta não disponível no cache offline: " + url)
        for attempt in range(4):
            time.sleep(max(0, 1.1 - (time.monotonic() - self.last_request)))
            self.last_request = time.monotonic()
            try:
                request = urllib.request.Request(url, headers={"User-Agent": "GlossataQuiz/1.0 catalog-import"})
                with urllib.request.urlopen(request, timeout=30) as response:
                    data = json.load(response)
                if not isinstance(data.get("results"), list):
                    raise ValueError("Resposta sem results")
                cached.write_text(json.dumps(data, ensure_ascii=False), encoding="utf-8")
                return data
            except (urllib.error.URLError, OSError, http.client.HTTPException, ValueError) as error:
                if attempt == 3:
                    raise RuntimeError(f"Falha em {url}: {error}") from error
                time.sleep(2 ** attempt)


class Taxonomy:
    def __init__(self, nodes=()):
        self.nodes = {n["id"]: dict(n) for n in nodes}
        self.by_identity = {(n["parent_id"], n["rank"], n["name"].casefold()): n["id"] for n in nodes}
        for kingdom in KINGDOM_ROOTS:
            self.nodes.setdefault("edu:" + kingdom, {"id": "edu:" + kingdom, "name": kingdom,
                "rank": "educational_kingdom", "parent_id": "", "source": "educational", "source_taxon_id": ""})

    def add(self, name, rank, parent, source="legacy", source_id=""):
        identity = (parent, rank, name.casefold())
        key = self.by_identity.get(identity)
        if key is None:
            digest = hashlib.sha256((parent + "|" + rank + "|" + name).encode()).hexdigest()[:20]
            key = "taxon:" + digest
            self.by_identity[identity] = key
            self.nodes[key] = {"id": key, "name": name, "rank": rank, "parent_id": parent,
                               "source": source, "source_taxon_id": str(source_id)}
        elif source == "inaturalist" and not self.nodes[key].get("source_taxon_id"):
            self.nodes[key].update(source=source, source_taxon_id=str(source_id))
        return key

    def legacy_lineage(self, item):
        parent = "edu:Animalia"
        lineage = []
        names = [("kingdom", "Animalia"), ("phylum", "Arthropoda"), ("class", "Insecta"),
                 ("order", "Lepidoptera"), ("family", item["family"]),
                 ("genus", item["name"].split()[0]), ("species", item["name"])]
        for rank, name in names:
            parent = self.add(name, rank, parent)
            lineage.append(parent)
        return lineage

    def imported_lineage(self, taxon, kingdom):
        parent = "edu:" + kingdom
        lineage = []
        ancestry = [*taxon.get("ancestors", []), taxon]
        root_ids = KINGDOM_ROOTS[kingdom]
        root_index = next(i for i, node in enumerate(ancestry) if node.get("id") in root_ids)
        for node in ancestry[root_index:]:
            if not node.get("name") or not node.get("rank"):
                continue
            parent = self.add(node["name"], node["rank"], parent, "inaturalist", node["id"])
            lineage.append(parent)
        return lineage

    def reconcile_legacy(self, species):
        # Merge only unambiguous matches inside the same educational kingdom.
        def realm(node):
            seen = set()
            while node and node["id"] not in seen:
                seen.add(node["id"])
                if node["rank"] == "educational_kingdom":
                    return node["name"]
                node = self.nodes.get(node["parent_id"])
            return None
        redirects = {}
        for old in list(self.nodes.values()):
            if old["source"] != "legacy":
                continue
            matches = [node for node in self.nodes.values() if node["source"] == "inaturalist"
                       and node["rank"] == old["rank"] and node["name"] == old["name"]
                       and realm(node) == realm(old)]
            if len(matches) == 1:
                new = matches[0]
                redirects[new["id"]] = old["id"]
                old.update(parent_id=new["parent_id"], source=new["source"], source_taxon_id=new["source_taxon_id"])
        for new_id in redirects:
            del self.nodes[new_id]
        for node in self.nodes.values():
            node["parent_id"] = redirects.get(node["parent_id"], node["parent_id"])
        for item in species:
            leaf = redirects.get(item["taxonomy"][-1], item["taxonomy"][-1])
            lineage, visited = [], set()
            while leaf and leaf not in visited:
                visited.add(leaf)
                node = self.nodes[leaf]
                if node["rank"] != "educational_kingdom":
                    lineage.append(leaf)
                leaf = node["parent_id"]
            item["taxonomy"] = list(reversed(lineage))


def download_photo(photo, offline=False, size="large", refresh=False):
    if size not in {"medium","large"}:
        raise ValueError("Tamanho de fotografia inválido")
    source_url = photo["url"].replace("/square.", "/" + size + ".")
    host = urllib.parse.urlparse(source_url).hostname or ""
    if not (source_url.startswith("https://") and
            (host.endswith(".inaturalist.org") or host == "inaturalist-open-data.s3.amazonaws.com")):
        raise ValueError("Host de imagem não reconhecido")
    extension = pathlib.PurePosixPath(urllib.parse.urlparse(source_url).path).suffix.lower()
    if extension not in {".jpg", ".jpeg", ".png"}:
        raise ValueError("Formato de imagem não suportado")
    relative = f"resources/images/inaturalist/{photo['id']}{extension}"
    target = ROOT / relative
    if not target.exists() or refresh:
        if offline:
            raise RuntimeError("Foto ausente no cache offline")
        request = urllib.request.Request(source_url, headers={"User-Agent": "GlossataQuiz/1.0 catalog-import"})
        with urllib.request.urlopen(request, timeout=30) as response:
            data = response.read(10 * 1024 * 1024 + 1)
        if len(data) > 10 * 1024 * 1024 or not (data.startswith(b"\xff\xd8\xff") or data.startswith(b"\x89PNG\r\n\x1a\n")):
            raise ValueError("Resposta não é uma foto válida")
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_suffix(target.suffix + ".part")
        temporary.write_bytes(data)
        temporary.replace(target)
    return relative, source_url


def save_json(path, value):
    temporary = path.with_suffix(path.suffix + ".part")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(path)


def import_catalog(count=20, max_pages=5, offline=False):
    species_path = ROOT / "resources/data/species.json"
    taxonomy_path = ROOT / "resources/data/taxonomy.json"
    species = json.loads(species_path.read_text(encoding="utf-8"))
    taxonomy = Taxonomy(json.loads(taxonomy_path.read_text(encoding="utf-8")) if taxonomy_path.exists() else [])
    for item in species:
        if "taxonomy" not in item:
            item.update(educational_kingdom="Animalia", taxonomy=taxonomy.legacy_lineage(item),
                        source="legacy", source_taxon_id="")
    api = Api(offline)
    known_names = {s["name"].casefold() for s in species}
    known_source_ids = {str(s.get("source_taxon_id")) for s in species if s.get("source_taxon_id")}
    report = {"imported_at": datetime.now(timezone.utc).isoformat(), "target_new_per_kingdom": count,
              "accepted_licenses": sorted(ALLOWED_LICENSES), "kingdoms": {}, "skipped": []}
    next_id = max((s["id"] for s in species), default=0) + 1
    for kingdom, roots in KINGDOM_ROOTS.items():
        already_imported = sum(s.get("source") == "inaturalist" and s.get("educational_kingdom") == kingdom for s in species)
        added = 0
        if already_imported < count:
            observations = {}
            exhausted = set()
            for page in range(1, max_pages + 1):
                for root_id, root_name in roots.items():
                    if root_id in exhausted:
                        continue
                    root = api.get(f"taxa/{root_id}")["results"][0]
                    if root.get("name") != root_name or root.get("rank") != "kingdom":
                        report["skipped"].append({"root": root_id, "reason": "Raiz taxonômica não confere"})
                        exhausted.add(root_id)
                        continue
                    rows = api.get("observations", taxon_id=root_id, quality_grade="research", rank="species",
                                   photos="true", photo_license="cc0,cc-by", per_page=100, page=page,
                                   order_by="votes", order="desc", locale="pt-BR")["results"]
                    if not rows:
                        exhausted.add(root_id)
                    for obs in rows:
                        taxon = obs.get("taxon") or {}
                        source_id = str(taxon.get("id", ""))
                        if (obs.get("quality_grade") != "research" or taxon.get("rank") != "species"
                            or not taxon.get("is_active") or obs.get("community_taxon_id") != taxon.get("id")
                            or source_id in known_source_ids or taxon.get("name", "").casefold() in known_names
                            or not re.fullmatch(r"[A-Z][a-z]+ [a-z][a-z-]+", taxon.get("name", ""))):
                            continue
                        photo = next((p for p in obs.get("photos", []) if permitted_photo(p)), None)
                        if photo:
                            observations.setdefault(source_id, (obs, photo))
                ids = list(observations)
                details = {}
                for offset in range(0, len(ids), 30):
                    if not ids[offset:offset + 30]:
                        continue
                    for taxon in api.get("taxa/" + ",".join(ids[offset:offset + 30]), locale="pt-BR")["results"]:
                        details[str(taxon["id"])] = taxon
                # Spread candidates across genera before accepting repeat genera.
                genus_seen = set()
                candidates = []
                repeats = []
                for source_id, observation in observations.items():
                    taxon = details.get(source_id)
                    genus = (taxon or {}).get("name", "").split(" ")[0]
                    (repeats if genus in genus_seen else candidates).append((source_id, observation, taxon))
                    genus_seen.add(genus)
                for source_id, (obs, photo), taxon in candidates + repeats:
                    if already_imported + added >= count:
                        break
                    if source_id in known_source_ids or not taxon:
                        continue
                    ancestors = [*taxon.get("ancestors", []), taxon]
                    mapped = educational_kingdom(ancestors)
                    if mapped != kingdom:
                        report["skipped"].append({"taxon": source_id, "reason": "Enquadramento didático ambíguo ou incompatível"})
                        continue
                    if taxon.get("rank") != "species" or not taxon.get("is_active"):
                        continue
                    try:
                        image_path, image_url = download_photo(photo, offline)
                    except (OSError, ValueError, urllib.error.URLError, http.client.HTTPException) as error:
                        report["skipped"].append({"taxon": source_id, "reason": str(error)})
                        continue
                    lineage = taxonomy.imported_lineage(taxon, kingdom)
                    family = next((t["name"] for t in ancestors if t.get("rank") == "family"), "")
                    genus = next((t["name"] for t in ancestors if t.get("rank") == "genus"), taxon["name"].split()[0])
                    source = f"https://www.inaturalist.org/observations/{obs['id']}"
                    item = {"id": next_id, "name": taxon["name"],
                        "common_name": taxon.get("preferred_common_name", ""),
                        "fact": "",
                        "hint": "",
                        "family": family, "region": "Distribuição não revisada; consulte o registro de origem.",
                        "image": "qrc:/" + image_path, "educational_kingdom": kingdom, "taxonomy": lineage,
                        "source": "inaturalist", "source_taxon_id": source_id,
                        "source_url": f"https://www.inaturalist.org/taxa/{source_id}",
                        "photo": {"id": str(photo["id"]), "scientific_name": taxon["name"], "taxon_id": source_id,
                            "license": photo["license_code"],
                            "license_url": LICENSE_URLS[photo["license_code"]], "author": photo["attribution"],
                            "source": source, "url": image_url, "path": image_path,
                            "sha256": hashlib.sha256((ROOT / image_path).read_bytes()).hexdigest(),
                            "research_grade": True, "visual_quiz": False, "approved_ranks": [],
                            "modifications": "Nenhuma. Arquivo original, sem recorte ou retoque.",
                            "review": "Identificação comunitária de grau científico; revisão editorial pendente."}}
                    species.append(item)
                    known_names.add(item["name"].casefold())
                    known_source_ids.add(source_id)
                    next_id += 1
                    added += 1
                if already_imported + added >= count or len(exhausted) == len(roots):
                    break
        total = sum(s.get("educational_kingdom") == kingdom for s in species)
        imported_total = sum(s.get("source") == "inaturalist" and s.get("educational_kingdom") == kingdom for s in species)
        report["kingdoms"][kingdom] = {"added": added, "imported_total": imported_total, "catalog_total": total,
                                      "shortfall": max(0, count - imported_total)}
        print(f"{kingdom}: +{added}; importadas={imported_total}; total={total}", flush=True)
    taxonomy.reconcile_legacy(species)
    used_taxons = {node_id for item in species for node_id in item["taxonomy"]}
    used_taxons.update("edu:" + kingdom for kingdom in KINGDOM_ROOTS)
    taxonomy.nodes = {key: node for key, node in taxonomy.nodes.items() if key in used_taxons}
    apply_photo_reviews(species)
    report["photos"] = dict(collections.Counter(s["photo"]["license"] for s in species if s.get("photo")))
    report["visual_quiz_records"] = sum(bool(s.get("photo", {}).get("visual_quiz")) for s in species)
    save_json(taxonomy_path, list(taxonomy.nodes.values()))
    save_json(species_path, species)
    save_json(ROOT / "resources/data/import_report.json", report)
    resource_path = ROOT / "resources.qrc"
    tree = ET.parse(resource_path)
    group = tree.getroot().find("qresource")
    photo_paths = {s["photo"]["path"] for s in species if s.get("photo")}
    for element in list(group.findall("file")):
        if element.text.startswith("resources/images/inaturalist/") and element.text not in photo_paths:
            group.remove(element)
    existing_files = {element.text for element in group.findall("file")}
    for name in ["resources/data/taxonomy.json", "resources/data/import_report.json", *sorted(photo_paths)]:
        if name not in existing_files:
            ET.SubElement(group, "file").text = name
    ET.indent(tree, space="    ")
    tree.write(resource_path, encoding="unicode")
    credits = ["# Créditos das fotografias", "", "Somente CC0 e CC BY; licença conferida por fotografia. Os arquivos são variantes originais do iNaturalist, sem recorte ou retoque. Uma licença aceita não constitui aprovação para o quiz. Autoria não identificada é uma pendência de curadoria.", ""]
    for item in species:
        photo = item.get("photo")
        if photo:
            author = photo['author'] if photo['author'] != 'no rights reserved' else 'Autoria não identificada; excluída do quiz'
            credits.append(f"- **{item['name']}** — {author} — [{photo['license'].upper()}]({photo['license_url']}) — [registro]({photo['source']}) — `{photo['path']}` — {photo.get('modifications', 'Modificações não verificadas.')} — níveis: {', '.join(photo.get('approved_ranks', [])) or 'não aprovado'}")
    (ROOT / "docs/PHOTO_CREDITS.md").write_text("\n".join(credits) + "\n", encoding="utf-8")
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--per-kingdom", type=int, default=20)
    parser.add_argument("--max-pages", type=int, default=5)
    parser.add_argument("--offline", action="store_true")
    args = parser.parse_args()
    if args.per_kingdom < 0 or args.max_pages < 1:
        parser.error("Quantidades inválidas")
    import_catalog(args.per_kingdom, args.max_pages, args.offline)
