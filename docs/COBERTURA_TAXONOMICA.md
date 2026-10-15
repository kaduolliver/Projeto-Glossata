# Diagnóstico de cobertura taxonômica

> Registro hist?rico: marca??es antigas de verifica??o/aprova??o n?o demonstram revis?o humana rastre?vel. Consulte a [governan?a atual, matriz e pend?ncias](CONFORMIDADE.md). Cobertura t?cnica n?o ? certifica??o cient?fica ou jur?dica.

Contagens extraídas do catálogo ativo por `python scripts/audit_catalog.py --strict`, em 14/10/2026. Os nomes e níveis científicos são os da fonte preservada; os cinco reinos são agrupamentos didáticos. Não são estimativas nem contagens de organismos novos importados.

| Reino didático | Registros | Linhagens de reino científico | Etimologia: ao menos um elemento | Pistas morfológicas |
|---|---:|---|---:|---:|
| Animalia | 42 | Animalia: 42 | 3 | 37 |
| Plantae | 20 | Plantae: 20 | 6 | 17 |
| Fungi | 20 | Fungi: 20 | 18 | 19 |
| Protista | 20 | Protozoa: 10, Chromista: 10 | 2 | 11 |
| Monera | 20 | Bacteria: 20 | 20 | 20 |

## Distribuição atual por filo/divisão e classe

### Animalia

Filos/divisões: Arthropoda (30), Chordata (11), Mollusca (1).

Classes: Insecta (29), Mammalia (6), Polyplacophora (1), Arachnida (1), Actinopterygii (1), Aves (2), Amphibia (2).

### Plantae

Filos/divisões: Bryophyta (1), Tracheophyta (17), Marchantiophyta (2).

Classes: Bryopsida (1), Magnoliopsida (9), Marchantiopsida (2), Polypodiopsida (1), Liliopsida (5), Ginkgoopsida (1), Lycopodiopsida (1).

### Fungi

Filos/divisões: Basidiomycota (16), Ascomycota (4).

Classes: Agaricomycetes (16), Lecanoromycetes (1), Pezizomycetes (1), Sordariomycetes (2).

### Protista

Filos/divisões: Mycetozoa (10), Ochrophyta (10).

Classes: Myxomycetes (9), Phaeophyceae (10), Ceratiomyxomycetes (1).

### Monera

Filos/divisões: Proteobacteria (8), Cyanobacteria (10), Firmicutes (1), Chlamydiae (1).

Classes: Alphaproteobacteria (1), Cyanophyceae (10), Gammaproteobacteria (6), Mollicutes (1), Betaproteobacteria (1), unclassified (1).

## Fotografias e elegibilidade

Há 122 registros com integridade de arquivo, metadados e licença compatível: 102 imagens associadas à identificação e 20 imagens de apoio de Monera. Isso não certifica cientificamente o sujeito fotografado. Plodia interpunctella teve a aprovação suspensa após divergência visual; o cadastro e a fotografia foram preservados. O validador Python confere SHA-256 e dimensões; os testes Qt executam a decodificação real.

| Nível | Elegíveis na política base, com quatro respostas distintas |
|---|---:|
| species | 40 |
| genus | 100 |
| family | 100 |
| order | 100 |
| class | 120 |
| phylum | 121 |
| kingdom | 121 |

O Competitivo possui 80 organismos elegíveis em gênero com fotos de identificação, excluindo imagens de apoio. A política Mais próximas pode reduzir a disponibilidade; a auditoria acima mede a base de fotografias/revisões e respostas distintas, sem prometer disponibilidade em qualquer filtro/dificuldade.

Há 16 introduções de táxons, 49 organismos com etimologia ao menos parcialmente documentada e 104 registros morfológicos. O relatório JSON completo lista IDs e campos pendentes.

## Grupos sub-representados e expansão controlada

- Animalia concentra 30 de 42 registros em Arthropoda, dos quais 29 são Insecta. Mollusca e Arachnida têm um representante cada. Avaliar primeiro outros moluscos e aracnídeos e filos ausentes como Annelida, Cnidaria e Echinodermata.
- Plantae concentra 17 de 20 registros em Tracheophyta. Ampliar briófitas e hepáticas antes de acrescentar mais angiospermas; não transferir algas automaticamente para Protista se a fonte científica as classifica de outra forma.
- Fungi concentra 16 de 20 registros em Basidiomycota. Ampliar diversidade de Ascomycota e avaliar outras linhagens com fotografias e identificação suficientes, evitando priorizar somente cogumelos vistosos.
- Protista possui somente Mycetozoa e Ochrophyta, com dez registros de cada; todos os Ochrophyta são Phaeophyceae. Ciliados, dinoflagelados, diatomáceas, euglenas e outras linhagens eucarióticas precisam de uma avaliação própria de identificação microscópica. Esses exemplos não são uma nova classificação aplicada ao catálogo.
- Monera tem 20 Bacteria e zero Archaea, com dez cianobactérias. Investigar a viabilidade educativa de Archaea e outras linhagens bacterianas; exigir documentação e imagens de cultura/microscopia corretamente atribuídas. A aparência, isoladamente, geralmente não comprova espécie ou gênero.

A seleção de linhagens é uma proposta de expansão, não uma identificação ou importação realizada. Referências de contexto: [diversidade de procariontes](https://openstax.org/books/biology-2e/pages/22-1-prokaryotic-diversity) e [diversidade e convergência entre protistas](https://openstax.org/books/biology-2e/pages/23-3-groups-of-protists). O esquema evolutivo desses materiais é apresentado como hipótese de organização, sem substituição automática da taxonomia registrada.

Proposta: avaliar um lote de seis a dez candidatos, priorizando lacunas diferentes, antes de qualquer inclusão. Cada candidato deve passar separadamente por identificação, licença/fotografia, elegibilidade por nível e conteúdo educativo. Elementos opcionais pendentes não impedem aprovação dos elementos já verificados. Quando a identificação fotográfica não for segura, manter o organismo para estudo descritivo ou níveis adequados, sem inventar provas visuais. Nenhum candidato foi importado nesta etapa.

## Pendências científicas

Os seguintes organismos ainda não possuem notas etimológicas aprovadas. A triagem de referências e as partes ausentes nos demais estão em resources/data/etymology_research.json; uma falha de acesso ou ausência de explicação explícita não autoriza deduzir o significado.

**Animalia (39):** Papilio machaon, Vanessa atalanta, Acherontia atropos, Brahmaea wallichii, Lymantria dispar, Danaus plexippus, Aglais io, Vanessa cardui, Panthera tigris, Attacus atlas, Actias luna, Hyalophora cecropia, Automeris io, Antheraea polyphemus, Papilio ulysses, Panthera leo, Pieris brassicae, Galleria mellonella, Plodia interpunctella, Manduca sexta, Panthera onca, Titanomis sisyrota, Macrocilix maia, Chiton glaucus, Geminaria canalis, Stegodyphus dumicola, Astroscopus zephyreus, Xenocnema spinipes, Sphaerocoris annulus, Pavo cristatus, Agalychnis callidryas, Dicarnosis erythrocephala, Pachypodistes angulata, Chortophaga viridifasciata, Laterallus rogersi, Cyphonia clavata, Equus quagga, Ovalimantis maculata, Incilius periglenes.

**Plantae (14):** Kindbergia oregana, Dictyanthus pavonii, Hydnora esculenta, Lunularia cruciata, Polypodium glycyrrhiza, Marah macrocarpa, Mastigostyla macbridei, Tulipa regelii, Asterella drummondii, Viburnum opulus, Xanthosoma robustum, Pleurothallis maitamae, Huperzia lucidula, Euphorbia duckei.

**Fungi (2):** Cladonia imperialis, Wawelia microspora.

**Protista (18):** Comatricha nigra, Hemitrichia serpula, Metatrichia vesparia, Macrocystis pyrifera, Nereocystis luetkeana, Physarum psittacinum, Ceratiomyxa fruticulosa, Arcyria denudata, Fucus vesiculosus, Himanthalia elongata, Egregia menziesii, Dictyota dichotoma, Heterotrichia versicolor, Leocarpus fragilis, Badhamia polycephala, Sargassum muticum, Diderma effusum, Turbinaria ornata.

**Monera (0):** Nenhum registro sem ao menos um elemento documentado.

**Morfologia sem pista aprovada (18):** Galleria mellonella, Plodia interpunctella, Titanomis sisyrota, Geminaria canalis, Xenocnema spinipes, Asterella drummondii, Huperzia lucidula, Euphorbia duckei, Wawelia microspora, Comatricha nigra, Hemitrichia serpula, Metatrichia vesparia, Physarum psittacinum, Ceratiomyxa fruticulosa, Heterotrichia versicolor, Leocarpus fragilis, Badhamia polycephala, Diderma effusum.

Uma confirmação especializada continua necessária para esses casos; textos genéricos, metadados comunitários ou associações por semelhança de palavras não substituem a evidência.
