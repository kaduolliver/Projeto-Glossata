"""Build offline sourced content with independent, explicit review metadata."""
import json
from import_inaturalist import ROOT, save_json
from content_governance import annotate_education, load
import hashlib

OS="https://openstax.org/books/biology-2e/pages/"
RANKS=["species","genus","family","order","class","phylum","kingdom"]
DEFINITIONS=[
("Animalia","educational_kingdom",OS+"27-1-features-of-the-animal-kingdom","OpenStax — Biology 2e",
 "Animalia reúne organismos eucariontes multicelulares que obtêm alimento de outros seres vivos ou de matéria orgânica. Suas células não possuem parede celular. O grupo apresenta diversidade de formas corporais, modos de reprodução e relações ecológicas, incluindo organismos aquáticos, terrestres e parasitas.",
 "As células deste grupo não possuem parede celular.", "O desenvolvimento embrionário e a organização dos tecidos ajudam a compreender a diversidade de planos corporais."),
("Plantae","educational_kingdom",OS+"25-1-early-plant-life","OpenStax — Biology 2e",
 "Plantae reúne linhagens de organismos eucariontes com paredes celulares que contêm celulose. Muitas realizam fotossíntese, embora existam formas parasitas. Nas plantas terrestres, a proteção do embrião e adaptações contra a perda de água foram importantes para a ocupação de ambientes fora da água.",
 "A parede celular contém celulose, que contribui para a sustentação.", "A colonização terrestre envolveu adaptações para conservar água e proteger estruturas reprodutivas."),
("Fungi","educational_kingdom",OS+"24-1-characteristics-of-fungi","OpenStax — Biology 2e",
 "Fungi reúne organismos eucariontes heterotróficos que absorvem nutrientes do ambiente. Muitos crescem como filamentos chamados hifas, formando um micélio, enquanto outros são unicelulares. Suas paredes celulares contêm quitina. Decomposição, associações com plantas e relações parasitárias estão entre os papéis ecológicos presentes neste grupo.",
 "A nutrição ocorre por absorção após digestão externa.", "Enzimas liberadas no ambiente decompõem moléculas; os produtos menores podem então ser absorvidos."),
("Protista","educational_kingdom",OS+"23-2-characteristics-of-protists","OpenStax — Biology 2e",
 "Protista é uma referência histórica que reuniu linhagens eucarióticas diversas, sem constituir um clado. O termo não é usado como grupo natural na navegação atual.",
 "", "A diversidade inclui formas com nutrição, organização celular e estruturas de locomoção diferentes."),
("Monera","educational_kingdom",OS+"22-1-prokaryotic-diversity","OpenStax — Biology 2e",
 "Monera é uma referência histórica para organismos procarióticos, não um grupo natural. A navegação atual distingue Bacteria e Archaea como domínios didáticos.",
 "", ""),
("Arthropoda","phylum",OS+"28-6-superphylum-ecdysozoa-arthropods","OpenStax — Biology 2e",
 "Arthropoda reúne animais com corpo segmentado, apêndices articulados e exoesqueleto. O revestimento externo fornece proteção e pontos de inserção muscular, mas exige mudas durante o crescimento. Seus representantes ocupam ambientes terrestres e aquáticos, com apêndices especializados para locomoção, alimentação e percepção do ambiente.",
 "Apêndices articulados e revestimento externo sustentam o corpo.", ""),
("Insecta","class",OS+"28-6-superphylum-ecdysozoa-arthropods","OpenStax — Biology 2e",
 "Insecta reúne artrópodes cujo corpo adulto se organiza em cabeça, tórax e abdome. Possuem três pares de pernas ligados ao tórax e um par de antenas. As asas, quando presentes, ampliam a capacidade de dispersão. O grupo participa de processos como polinização e de numerosas relações alimentares.",
 "", ""),
("Chordata","phylum",OS+"29-1-chordates","OpenStax — Biology 2e",
 "Chordata reúne animais que apresentam notocorda, tubo nervoso dorsal e outras características em algum momento do desenvolvimento. Nos vertebrados, algumas dessas estruturas são modificadas durante a formação do corpo. O filo também inclui linhagens sem coluna vertebral, mostrando que cordados e vertebrados não são termos equivalentes.",
 "Uma estrutura de suporte axial aparece durante o desenvolvimento.", "Algumas características compartilhadas aparecem apenas no embrião e mudam posteriormente."),
("Tracheophyta","phylum",OS+"25-4-seedless-vascular-plants","OpenStax — Biology 2e",
 "Tracheophyta reúne plantas com tecidos vasculares especializados no transporte interno de substâncias. O xilema participa da condução de água e minerais, enquanto o floema transporta compostos orgânicos. Essa organização está associada à diferenciação de órgãos e permite compreender a diversidade de formas das plantas vasculares.",
 "Tecidos especializados transportam água e compostos orgânicos.", "Transporte interno e sustentação foram importantes na diversificação das plantas vasculares."),
("Ascomycota","phylum",OS+"24-2-classifications-of-fungi","OpenStax — Biology 2e",
 "Ascomycota é uma linhagem de fungos caracterizada pela formação de esporos sexuais em estruturas chamadas ascos. Inclui formas unicelulares e filamentosas. Seus representantes apresentam modos de vida variados, incluindo decomposição, parasitismo e associações simbióticas. As estruturas reprodutivas são importantes para reconhecer e estudar seus grupos.",
 "", ""),
("Basidiomycota","phylum",OS+"24-2-classifications-of-fungi","OpenStax — Biology 2e",
 "Basidiomycota reúne fungos que produzem esporos sexuais associados a estruturas chamadas basídios. O grupo inclui muitos cogumelos, mas também outras formas de organização. Alguns representantes decompõem matéria orgânica ou estabelecem associações com plantas. Estruturas microscópicas e reprodutivas complementam a observação do corpo de frutificação.",
 "A reprodução sexual envolve estruturas produtoras de esporos.", ""),
("Amoebozoa","phylum",OS+"23-3-groups-of-protists","OpenStax — Biology 2e",
 "Amoebozoa reúne eucariontes com formas de organização variadas, incluindo organismos ameboides e linhagens com fases multinucleadas. Pseudópodes lobosos ou tubulares participam do movimento e da alimentação em muitos representantes. Algumas linhagens possuem estruturas protetoras, enquanto outras apresentam fases e corpos produtores de esporos.",
 "Projeções celulares podem participar de locomoção e alimentação.", ""),
("Myxogastria","class",OS+"23-3-groups-of-protists","OpenStax — Biology 2e",
 "Myxogastria reúne organismos com uma fase plasmodial, formada por uma massa celular multinucleada. Durante a fase de alimentação, partículas podem ser englobadas. A formação de estruturas com esporos lembra superficialmente alguns fungos, mas esse aspecto não significa que os organismos tenham o mesmo modo de alimentação ou organização.",
 "", ""),
("Lepidoptera","order","https://www.amentsoc.org/insects/fact-files/orders/lepidoptera.html","Amateur Entomologists' Society",
 "Lepidoptera é a ordem de insetos que inclui borboletas e mariposas. As asas dos adultos apresentam pequenas escamas, e o ciclo de vida passa por ovo, lagarta, pupa e adulto. Características das asas, das antenas e de outras estruturas ajudam a reconhecer seus diferentes grupos.",
 "O ciclo de vida inclui uma fase de pupa entre lagarta e adulto.", "A cobertura de pequenas escamas participa dos padrões observados nas asas."),
("Papilionidae","family","https://genent.cals.ncsu.edu/insect-identification/order-lepidoptera/family-papilionidae/","North Carolina State University",
 "Papilionidae reúne borboletas entre as quais muitas apresentam prolongamentos nas asas posteriores. Esse caráter ajuda na observação, embora não esteja presente em todas as espécies. A identificação dos representantes deve considerar um conjunto de estruturas e padrões, em vez de depender somente de cor ou tamanho.",
 "Muitas formas apresentam prolongamentos nas asas posteriores.", ""),
("Saturniidae","family","https://pmc.ncbi.nlm.nih.gov/articles/PMC5723595/","Publicação científica — Ecology and Evolution",
 "Saturniidae reúne mariposas que incluem numerosas formas de grande porte. Em muitas espécies, os adultos têm vida breve e não se alimentam, dependendo de reservas acumuladas anteriormente. A diversidade apresenta particularidades de comportamento e desenvolvimento. A identificação pode exigir a comparação cuidadosa de características e dados adicionais.",
 "Em muitas espécies, o adulto depende de reservas da fase larval.", ""),
]
DEFINITIONS.extend([
("Mycetozoa","phylum","https://ucmp.berkeley.edu/protista/slimemolds.html","University of California, Berkeley — Museum of Paleontology",
 "Mycetozoa é um agrupamento conservado na classificação da fonte deste catálogo. Seus representantes incluem formas com fases ameboides e plasmodiais, além de estruturas produtoras de esporos. A semelhança visual com fungos em certas fases não indica a mesma organização celular ou uma relação de parentesco próxima.",
 "Algumas fases são massas celulares com numerosos núcleos.", "A formação de estruturas com esporos não significa que esse organismo seja um fungo."),
("Euglenida","class","https://pmc.ncbi.nlm.nih.gov/articles/PMC7052976/","Publicação científica — Open Biology",
 "Euglenida reúne eucariontes unicelulares flagelados com uma película rica em proteínas. O grupo inclui linhagens fotossintetizantes e heterotróficas, com diversidade de alimentação e de organização celular. A película pode permitir mudanças de forma em alguns representantes. Características microscópicas e dados moleculares são importantes na identificação.",
 "", ""),
("Chiton","genus","https://www.museum.qld.gov.au/learn-and-discover/animals-of-queensland/molluscs/chitons","Queensland Museum",
 "Chiton é um gênero de moluscos pertencente à classe Polyplacophora. Os caracteres compartilhados nessa classe incluem oito placas de concha e uma faixa de tecidos ao redor delas. A organização do revestimento e do pé muscular ajuda esses animais a permanecerem associados a superfícies rochosas.",
 "O revestimento apresenta uma sequência de placas articuladas.", "As placas permitem proteção e flexibilidade sobre superfícies irregulares."),
])

# Group fallback notes remain useful when a species clue would reveal an answer.
# The introductions retain the scientific detail needed for group exploration.
PLAIN_NOTES={
    "Animalia":("Obtém alimento de outros seres vivos ou de matéria orgânica.","Os animais ocupam ambientes muito diferentes: água, solo, árvores e até o corpo de outros seres."),
    "Plantae":("Muitas formas usam a luz para produzir seu próprio alimento.","Existem plantas que se reproduzem por esporos e não produzem flores."),
    "Fungi":("Obtém nutrientes do ambiente por absorção.","O cogumelo visível é apenas uma parte do organismo; outra parte pode crescer na madeira ou no solo."),
    "Protista":("", "Neste agrupamento didático há organismos muito diferentes, desde formas microscópicas até algas grandes."),
    "Monera":("É microscópico e não tem um núcleo separado por membrana.","Alguns integrantes vivem na água, outros no solo ou associados a plantas e animais."),
    "Arthropoda":("Observe o revestimento externo e as pernas articuladas.","O revestimento externo precisa ser trocado para que muitos integrantes cresçam."),
    "Insecta":("No adulto, procure três pares de pernas e um par de antenas.","As asas aparecem na fase adulta de muitas espécies, mas há integrantes sem asas."),
    "Chordata":("", "Peixes, aves e mamíferos compartilham parentesco, apesar das diferenças de aparência."),
    "Tracheophyta":("Água e nutrientes são transportados dentro da planta.","Há estruturas internas que levam água das raízes para outras partes da planta."),
    "Basidiomycota":("", "Muitos representantes produzem estruturas visíveis acima do solo ou da madeira."),
    "Mycetozoa":("Algumas fases lembram uma pequena massa sobre superfícies úmidas.","Esses organismos podem parecer fungos, mas têm um modo de vida diferente."),
    "Lepidoptera":("Antes de ter asas, passa pelas fases de lagarta e pupa.","As asas têm pequenas escamas que participam de seus desenhos e cores."),
    "Papilionidae":("Muitas formas têm prolongamentos nas asas de trás.","Os prolongamentos das asas ajudam na observação, mas não aparecem em todas as espécies."),
    "Saturniidae":("Em muitas espécies, o adulto vive das reservas acumuladas pela lagarta.","Algumas mariposas adultas não se alimentam durante sua curta vida."),
    "Chiton":("Procure placas alinhadas sobre um corpo oval.","As placas protegem o corpo sem impedir que ele acompanhe as irregularidades das rochas.")}

def load_profiles():
    profiles=json.loads((ROOT/"resources/data/species_profiles.json").read_text(encoding="utf-8"))
    catalog=json.loads((ROOT/"resources/data/species.json").read_text(encoding="utf-8"))
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

def build():
    taxons=json.loads((ROOT/"resources/data/taxonomy.json").read_text(encoding="utf-8"))
    data={"notes":{},"taxons":{},"group_hints":{},"group_curiosities":{},"species":{}}
    previous = load(ROOT / 'resources/data/education.json', {})
    for number,(name,rank,url,title,description,hint,curiosity) in enumerate(DEFINITIONS):
        hint,curiosity=PLAIN_NOTES.get(name,(hint,curiosity))
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
    for sid,profile in load_profiles().items():
        values={key:profile[key] for key in ("scientific_name","common_name","display_name","reviewed_on")}
        for field in ("summary","distribution","nutrition","morphology","conservation","hint","curiosity"):
            if field not in profile: continue
            key=f"species:{sid}:{field}"
            data["notes"][key]=profile[field]
            values["hints" if field=="hint" else field]=[key] if field=="hint" else key
        data["species"][sid]=values
    policy = load(ROOT / 'resources/data/phylogeny.json', {})
    for species in load(ROOT / 'resources/data/species.json', []):
        if 'classification' not in species: continue
        if not policy.get('sources'): raise ValueError('Enquadramento filogenético sem fontes ou política.')
        data['species'][str(species['id'])]['phylogeny'] = dict(species['classification'],
            sources=[policy['sources'][key] for key in dict.fromkeys(species['classification']['source_refs'] + policy.get('alternative_source_refs', []))],
            system_limitations=policy['limitations'])
    hints=json.loads((ROOT/"resources/data/hint_education.json").read_text(encoding="utf8"))
    profiles=load_profiles()
    reviews=json.loads((ROOT/"resources/data/curation_reviews.json").read_text(encoding="utf8"))
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
    annotate_education(ROOT, data)
    source_names = ['species.json', 'taxonomy.json', 'species_profiles.json', 'hint_education.json', 'curation_reviews.json']
    if (ROOT / 'resources/data/phylogeny.json').is_file(): source_names.append('phylogeny.json')
    data['generated_source_sha256'] = {name: hashlib.sha256((ROOT / 'resources/data' / name).read_bytes()).hexdigest()
        for name in source_names}
    save_json(ROOT/"resources/data/education.json",data)
    print("Introductions:",len(data["taxons"]),"species-specific reviews:",len(data["species"]))
    return data

if __name__=="__main__": build()
