# Substituições do catálogo e imagens de apoio

> Documento histórico. A organização atual por domínios e linhagens, com cobertura e limites atualizados, está em [CLASSIFICACAO_FILOGENETICA.md](CLASSIFICACAO_FILOGENETICA.md). Os agrupamentos antigos deste registro não são táxons naturais ativos.

> Registro hist?rico: marca??es antigas de verifica??o/aprova??o n?o demonstram revis?o humana rastre?vel. Consulte a [governan?a atual, matriz e pend?ncias](CONFORMIDADE.md). Cobertura t?cnica n?o ? certifica??o cient?fica ou jur?dica.

O usuário autorizou substituir organismos sem foto adequada e, depois, pediu para manter Monera com fotografias de apoio, inclusive quando não permitam reconhecer a espécie. Essa exceção se aplica somente a Monera.

## Resultado

- 26 organismos substituídos: 8 Animalia, 2 Plantae, 6 Fungi e 10 Protista.
- 122 organismos ativos, todos com imagem local que passa em licença, autoria, fonte, integridade, decodificação e resolução.
- 102 organismos com fotografia aprovada em pelo menos um nível de identificação visual.
- 20 organismos originais de Monera mantidos, com imagens de apoio e identificação por pistas de classificação.
- 120 arquivos fotográficos distintos: 113 CC BY e 7 CC0. As entradas de catálogo usam 115 créditos CC BY e 7 CC0, pois três questões compartilham uma fotografia de apoio.
- 101 registros no jogo misto de gêneros: 81 visuais e 20 de Monera por pistas.
- 41 registros em espécie: 21 visuais e 20 de Monera por pistas. Não se declara identificação bacteriana pela foto.

Fotos foram selecionadas individualmente, com autoria e licença da imagem e correspondência taxonômica da observação. Para observações identificadas até subespécie, a ancestralidade confirma o vínculo à espécie usada no jogo, preservando também a identificação nativa nos metadados JSON. A triagem não equivale à validação independente de um especialista.

## Substituições

| ID anterior | Organismo anterior | Novo ID | Substituto |
|---|---|---|---|
| 9 | Bombyx mori | 125 | Panthera tigris |
| 17 | Morpho peleides | 126 | Panthera leo |
| 22 | Caligo memnon | 127 | Panthera onca |
| 29 | Latia neritoides | 128 | Lemur catta |
| 32 | Podilymbus podiceps | 129 | Pavo cristatus |
| 33 | Polyergus longicornis | 130 | Agalychnis callidryas |
| 39 | Bassia bassensis | 131 | Rhinoceros unicornis |
| 40 | Teleogryllus commodus | 132 | Equus quagga |
| 56 | Betula neoalaskana | 133 | Ginkgo biloba |
| 59 | Coptis trifolia | 134 | Dionaea muscipula |
| 66 | Dendrostilbella smaragdina | 135 | Clathrus ruber |
| 69 | Schizophyllum commune | 136 | Amanita muscaria |
| 72 | Mycena globulispora | 137 | Morchella esculenta |
| 74 | Erysiphe abbreviata | 138 | Phallus impudicus |
| 77 | Pandora batallata | 139 | Geastrum triplex |
| 78 | Psathyrella aquatica | 140 | Hericium erinaceus |
| 86 | Tubifera ferruginosa | 141 | Macrocystis pyrifera |
| 87 | Arcella conica | 142 | Nereocystis luetkeana |
| 91 | Planocarina marginata | 143 | Fucus vesiculosus |
| 92 | Galeripora dentata | 144 | Himanthalia elongata |
| 93 | Lepocinclis fusca | 145 | Egregia menziesii |
| 94 | Difflugia bacillifera | 146 | Dictyota dichotoma |
| 98 | Netzelia corona | 147 | Sargassum muticum |
| 100 | Nebela penardiana | 148 | Durvillaea antarctica |
| 101 | Cyclopyxis pirini | 149 | Alaria esculenta |
| 102 | Awerintzewia cyclostoma | 150 | Turbinaria ornata |

Os substitutos recebem IDs novos. `catalog_replacements.json` arquiva os registros completos anteriores, sua linhagem e a correspondência com os novos IDs. Os organismos antigos deixam o catálogo ativo; seus arquivos fotográficos não foram apagados.

## Monera

Os 20 nomes e IDs de Monera continuam iguais. Imagens com autoria ausente receberam outras fontes CC BY; nenhuma autoria foi inventada. Anabaena torulosa e Hassallia byssoidea usam uma imagem relacionada de Dolichospermum lemmermannii como apoio explícito. O táxon da foto permanece o da origem e `supporting_for` registra o organismo da questão.

A propriedade `role=supporting` permite o uso contextual somente em Monera. Pergunta e ficha mostram “Imagem de apoio · não comprova a identificação da espécie”. Créditos também identificam esse uso. As pistas principais vêm dos níveis superiores da classificação registrada e não exibem gênero ou família. A etapa seguinte acrescentou descrições biológicas e dicas em linguagem simples aos 20 organismos; consulte [Fichas das espécies](FICHAS_ESPECIES.md).

Nos demais reinos, a imagem precisa corresponder ao organismo e ter aprovação para o nível selecionado. Os fungos e algas adicionados mantêm restrições de espécie quando a foto não fornece caracteres diagnósticos suficientes.

## Fontes

- Fotografias e linhagens: [API do iNaturalist](https://api.inaturalist.org/v1/docs/); créditos individuais em [PHOTO_CREDITS.md](PHOTO_CREDITS.md).
- Características dos animais consultadas no [Animal Diversity Web](https://animaldiversity.org/).
- Folhas e armadilhas das novas plantas: [Ginkgo — NC State](https://plants.ces.ncsu.edu/plants/ginkgo-biloba/) e [Dionaea — NC State](https://plants.ces.ncsu.edu/plants/dionaea-muscipula/).
- Estruturas de macrofungos: [US Forest Service — Collecting and Describing Macrofungi](https://research.fs.usda.gov/download/treesearch/7111.pdf).
- Algas pardas: [SeaNet — Stanford](https://seanet.stanford.edu/Ochrophyta), [Monterey Bay Aquarium](https://www.montereybayaquarium.org/animals-the-ocean/animals-a-to-z/giant-kelp) e [MarLIN — Marine Biological Association](https://www.marlin.ac.uk/species).

## Persistência e regressão

SQLite ganha uma coluna aditiva `active`. Apenas IDs/nome correspondentes às substituições explícitas são inativados, após verificar que o substituto existe. As linhas anteriores, táxons, perfis e ranking permanecem. Registros locais não relacionados continuam disponíveis. Consultas e contagens consideram apenas os organismos ativos. O manifesto de substituições participa do hash de atualização.

Testes verificam a migração/reabertura, manutenção da linha antiga com active=0, criação do substituto, chaves estrangeiras e preservação de pontuação/perfil. A identificação da questão usa speciesId, evitando confusão quando duas espécies usam a mesma imagem de apoio.

Validação executada: build Release; quatro suítes CTest aprovadas; lint QML sem avisos; capturas em 1240×820 e 1000×700, incluindo quiz e ficha de Monera. O pacote do launcher foi atualizado e seu executável foi conferido por SHA-256.

As introduções educativas agora cobrem 16 táxons ativos: dois grupos com texto da etapa anterior ficaram sem representantes após as substituições. Os relatórios antigos permanecem como histórico, com aviso para consultar este documento. A amostra mantém 42 Animalia, 20 Plantae, 20 Fungi, 20 Protista e 20 Monera.
