# Dicas estratégicas e pontuação exata

> Registro hist?rico: marca??es antigas de verifica??o/aprova??o n?o demonstram revis?o humana rastre?vel. Consulte a [governan?a atual, matriz e pend?ncias](CONFORMIDADE.md). Cobertura t?cnica n?o ? certifica??o cient?fica ou jur?dica.

Registro da etapa inicial de dicas. Para a implementação atual — recordes, modo Competitivo, três alternativas compatíveis e etimologias ampliadas — consulte [EVOLUCAO_COMPETITIVA.md](EVOLUCAO_COMPETITIVA.md) e [COBERTURA_TAXONOMICA.md](COBERTURA_TAXONOMICA.md).

Implementação validada em 14/10/2026, mantendo o catálogo de 122 organismos, a navegação, os perfis, a sidebar e o encerramento voluntário pelo botão Voltar.

## Auditoria e funcionalidades

Antes desta etapa, cada acerto incrementava um único campo score. Esse campo também era usado como número de acertos nas estatísticas, conquistas e ranking. Havia uma dica por pergunta, revelada uma única vez e sem custo.

Agora o botão Dica abre um painel com categorias taxonômica, etimológica e morfológica. Abrir, fechar ou cancelar o painel não altera pontos. Somente revelar uma categoria disponível muda seu estado. O conteúdo permanece consultável; categorias podem ser combinadas e revisitar uma delas não aumenta a pontuação possível.

| Assistência efetivamente revelada | Unidades máximas | Apresentação |
|---|---:|---:|
| Nenhuma | 6 | 1,00 |
| Somente taxonômica | 6 | 1,00 |
| Etimológica, sem morfológica | 3 | 0,50 |
| Morfológica, com qualquer combinação | 2 | 0,33 |
| Resposta incorreta | 0 | 0,00 |

As unidades são inteiras de 64 bits no total. Cada questão inicia com seis unidades. O cálculo usa o conjunto de categorias reveladas e aplica o menor teto. Três acertos com assistência morfológica somam exatamente seis unidades, ou um ponto. A conversão com duas casas decimais e vírgula acontece somente para apresentação.

Acertos, erros, precisão, desempenho taxonômico e conquistas continuam contando respostas, independentemente do peso em pontos. O quiz mostra a pontuação acumulada e possível; a revisão mostra os pontos obtidos na questão; resultado e ranking usam a apresentação fracionária.

Após responder, Aprender com esta resposta permite consultar nome científico, classificação, origem documentada do nome e aspectos físicos. A informação independe das dicas utilizadas e não afeta os pontos já registrados. A ficha ampliada anterior continua disponível.

## Seleção científica por questão

A dica taxonômica é construída a partir da linhagem científica armazenada. Precisa citar um ancestral verificado mais abrangente que o nível solicitado, não conter uma alternativa literal e deixar **pelo menos duas alternativas distintas compatíveis**. Quando uma alternativa representa vários organismos, todos precisam concordar com aquele ancestral. Informações ausentes não são consideradas confirmação. Um reino compartilhado pelas quatro alternativas é descartado como contexto excessivamente genérico.

A geração tenta até oito novos conjuntos de distratores para encontrar um contexto taxonômico seguro, preservando quatro respostas diferentes, o nível solicitado e o grupo escolhido. Não altera a ordem de organismos nem cria uma categoria artificial quando não há conteúdo adequado.

Etimologias são selecionadas pelo nível solicitado: informação do epíteto para espécie e do gênero para gênero, conforme a referência disponível. O texto não contém a alternativa literal. A revisão posterior reutiliza as mesmas notas referenciadas. Classificação do iNaturalist não é aceita como fonte etimológica pelo gerador.

Pistas morfológicas reutilizam conteúdo revisado, vinculando **ID da espécie, nome científico e ID da foto**. Nesta revisão, estão curadas para espécie/gênero; pistas de habitat, dieta ou aparência pouco distintiva não são convertidas automaticamente em ajuda morfológica. Em Monera, informações celulares são explicitamente complementares e a fotografia continua identificada como apoio.

As categorias representam assistência pretendida, não uma medição experimental de dificuldade. Não há uma garantia de facilidade equivalente entre todos os organismos. Níveis superiores a gênero ainda não possuem notas etimológicas ou morfológicas adequadas no corpus desta etapa; as categorias ficam desabilitadas. Em reino, não há ancestral científico mais amplo verificado no catálogo para uma dica taxonômica segura.

## Cobertura do conteúdo

| Reino | Linhagem com referências | Organismos com etimologia documentada | Organismos com pista morfológica curada |
|---|---:|---:|---:|
| Animalia | 42 | 1 | 37 |
| Plantae | 20 | 4 | 17 |
| Fungi | 20 | 8 | 19 |
| Protista | 20 | 1 | 11 |
| Monera | 20 | 8 | 20 |
| Total | 122 | 22 | 104 |

Essas contagens são de **conteúdo disponível**, não de categorias garantidas em qualquer questão. As restrições fotográficas e o nível solicitado continuam em vigor. No conjunto misto elegível de gênero, 11 organismos possuem dica etimológica e 89 possuem pista morfológica adequada. Em espécie, são nove e 40, respectivamente, entre 41 organismos elegíveis.

A cobertura taxonômica depende também dos distratores. A suíte produz build/tests/hint-coverage.json com uma amostra determinística de quatro alternativas por organismo e nível; esse relatório não representa todas as combinações possíveis nem o resultado da reamostragem. Categorias podem ficar indisponíveis numa questão mesmo existindo uma nota para o organismo.

Na amostra de validação desta implementação, as categorias disponíveis foram:

| Nível solicitado | Organismos elegíveis | Taxonômica | Etimológica | Morfológica |
|---|---:|---:|---:|---:|
| Espécie | 41 | 13 | 9 | 40 |
| Gênero | 101 | 22 | 11 | 89 |
| Família | 101 | 22 | 0 | 0 |
| Ordem | 101 | 30 | 0 | 0 |
| Classe | 121 | 35 | 0 | 0 |
| Filo | 122 | 42 | 0 | 0 |
| Reino | 122 | 0 | 0 | 0 |

São contagens de perguntas dessa amostra, sem a busca adicional de até oito conjuntos de distratores aplicada durante o jogo. Não devem ser interpretadas como cobertura fixa de dicas taxonômicas por espécie.

## Fontes etimológicas

O corpus editável está em [hint_education.json](../resources/data/hint_education.json). Fontes e datas estão registradas por nota; referências morfológicas reutilizam [species_profiles.json](../resources/data/species_profiles.json). A classificação vem da linhagem do catálogo, com referências por táxon.

- [First Nature — nomenclatura dos fungos](https://www.first-nature.com/fungi/~naming.php) e suas fichas específicas documentam nomes de Stropharia caerulea, Clathrus ruber, Trametes versicolor, Amanita muscaria, Coprinus comatus, Phallus impudicus, Geastrum triplex e Hericium erinaceus.
- [LPSN — DSMZ](https://lpsn.dsmz.de/) fornece entradas etimológicas específicas de Sphingomonas phyllosphaerae, Serratia marcescens, Thiohalocapsa halophila, Beggiatoa alba, Leptothrix discophora, Chlamydia pecorum, Aeromonas caviae e Pseudomonas fluorescens. O epíteto de pecorum também corresponde ao sinônimo registrado no catálogo como Chlamydophila pecorum.
- [Manual científico da International Rhino Foundation](https://rhinos.org/wp-content/uploads/2020/10/rhino-husbandry-manual.compressed.pdf) documenta os elementos de Rhinoceros unicornis.
- [NCSU — Ginkgo biloba](https://plants.ces.ncsu.edu/plants/ginkgo-biloba/) e [Verbascum thapsus](https://plants.ces.ncsu.edu/plants/verbascum-thapsus/) documentam sentidos linguísticos de seus nomes.
- [SANBI — Mimetes chrysanthus](https://pza.sanbi.org/mimetes-chrysanthus), [NZPCN — Pleurophyllum speciosum](https://www.nzpcn.org.nz/flora/species/pleurophyllum-speciosum/) e [MarLIN — Alaria esculenta](https://www.marlin.ac.uk/species/detail/1291) fornecem as outras notas. Em Pleurophyllum, somente o gênero foi incluído, pois a ficha consultada não explica o epíteto.

Não foram criados significados por semelhança de palavras. As 100 outras espécies não possuem etimologia verificada neste corpus; isso não significa que a origem de seus nomes seja desconhecida na literatura. A lista completa das pendências está no fim deste relatório.

## Banco e migração

O banco continua no mesmo caminho de glossata_history.sqlite. Na abertura, uma transação mantém a regra anterior de unicidade por nome e converte a coluna score para **score_units**, multiplicando os valores antigos por seis. A presença de score_units é o marcador da migração. Reabrir o banco não multiplica novamente, e uma falha reverte a transação.

Um resultado antigo de 10 pontos passa a 60 unidades e aparece como **10,00**. correct_answers e player_profiles são preservados. Não foram inventados registros de perguntas para partidas antigas. A consolidação de duplicatas anteriores à regra de unicidade continua seguindo a política já existente: último played_at e, em empate, maior ID.

history mantém o **resultado mais recente por nome**, com comparação COLLATE NOCASE e nomes aparados, inclusive quando a nova pontuação é menor. A ordenação usa score_units inteiro, nunca o texto arredondado da interface.

game_runs guarda UUID da partida, jogador capturado ao iniciar e horário de encerramento. answer_records guarda, para cada resposta:

- Número da questão, ID/nome científico do organismo, ID da fotografia e nível solicitado.
- Alternativa escolhida, resposta correta e se houve acerto.
- Máscara das categorias reveladas, máximo disponível e pontos obtidos em unidades.

A chave composta é game_id/question_number. O salvamento de partida, respostas e atualização do ranking é transacional. O UUID impede duplicação numa nova tentativa. Falhas mantêm o resultado pendente no fluxo existente. A quantidade de dicas e os pontos por categoria podem ser derivados da máscara e das unidades registradas, sem novos acumuladores persistidos.

## Arquivos relevantes

| Arquivo | Responsabilidade |
|---|---|
| src/domain/Hint.h | Tipos, máscara, tetos exatos, formato e registro de resposta |
| src/domain/GameSession.h e Question.h | Pontos/acertos separados, respostas e dicas da questão |
| src/application/GameService.* | Revelação, combinação, geração contextual, reinício e salvamento |
| src/application/EducationalContent.* | Identidade, fontes, nível, alternativas, fotografia e origem do nome |
| src/application/ports/GameHistoryRepository.h | Contrato de pontos em unidades e registros |
| src/infrastructure/persistence/SqliteGameHistoryRepository.* | Migração, ordenação e salvamento transacional |
| src/presentation/GameController.* | Snapshot das dicas, valores formatados e conteúdo após resposta |
| resources/qml/components/HintPanel.qml | Escolha, estados, conteúdo, consulta e indisponibilidade |
| Main.qml e views/QuizView, FeedbackView, ResultsView, StatisticsView | Pontos, aprendizado e métricas corretas |
| resources/data/hint_education.json e education.json | Corpus referenciado e conteúdo embarcado |
| scripts/build_education.py | Geração offline e validação científica estrutural |
| tests/GameServiceTests.cpp, QmlInterfaceTests.cpp e test_import_inaturalist.py | Regras, migração, registros, conteúdo e interface |

## Testes e validação

Quatro suítes CTest aprovadas. A suíte principal tem 34 resultados Qt aprovados, incluindo inicialização/finalização; a suíte Python tem 12 testes. O lint QML terminou sem avisos.

Foram verificados todos os oito conjuntos de categorias, com acerto e erro; três acertos morfológicos equivalendo a um ponto; 600 questões acumulando exatamente 200 pontos; consulta repetida; ausência de aumento do teto; reinício da questão; referências/identidade; ancestral mais amplo e pelo menos duas alternativas compatíveis; vínculo com a fotografia; migração de 10 para 60 unidades sem reconversão; precisão do ranking; resultado mais recente por nome; integridade/rollback e ausência de duplicação.

O teste de interface abre o painel sem penalização, seleciona as três categorias, revisita dicas, valida a atualização imediata e consulta o aprendizado sem alterar pontos. Exercita categorias indisponíveis e conserva os testes de encerramento, cinco reinos, navegação e snapshots. Capturas em build/tests/previews incluem hint-category-selection.png, compact-hint-categories.png e compact-answer-learning.png.

## Limitações registradas

Além das etimologias pendentes, 18 organismos ainda não têm uma pista morfológica suficientemente adequada nesta curadoria: Galleria mellonella, Plodia interpunctella, Titanomis sisyrota, Geminaria canalis, Xenocnema spinipes, Asterella drummondii, Huperzia lucidula, Euphorbia duckei, Wawelia microspora, Comatricha nigra, Hemitrichia serpula, Metatrichia vesparia, Physarum psittacinum, Ceratiomyxa fruticulosa, Heterotrichia versicolor, Leocarpus fragilis, Badhamia polycephala e Diderma effusum.

O histórico antigo permite preservar pontos/acertos agregados, mas não reconstruir as perguntas ou dicas que não eram armazenadas. A interface de estatísticas detalhadas por assistência não foi criada; os novos registros permitem uma expansão posterior. Os dados científicos são embarcados e só mudam com uma nova revisão/build, nunca com pesquisas durante uma partida.

## Organismos sem etimologia verificada no corpus

**Animalia (41):** Papilio machaon, Vanessa atalanta, Acherontia atropos, Brahmaea wallichii, Lymantria dispar, Danaus plexippus, Aglais io, Vanessa cardui, Panthera tigris, Attacus atlas, Actias luna, Hyalophora cecropia, Automeris io, Antheraea polyphemus, Heliconius melpomene, Papilio ulysses, Panthera leo, Pieris brassicae, Galleria mellonella, Plodia interpunctella, Manduca sexta, Panthera onca, Titanomis sisyrota, Macrocilix maia, Chiton glaucus, Geminaria canalis, Stegodyphus dumicola, Astroscopus zephyreus, Lemur catta, Xenocnema spinipes, Sphaerocoris annulus, Pavo cristatus, Agalychnis callidryas, Dicarnosis erythrocephala, Pachypodistes angulata, Chortophaga viridifasciata, Laterallus rogersi, Cyphonia clavata, Equus quagga, Ovalimantis maculata, Incilius periglenes.

**Plantae (16):** Kindbergia oregana, Dictyanthus pavonii, Hydnora esculenta, Lunularia cruciata, Polypodium glycyrrhiza, Marah macrocarpa, Mastigostyla macbridei, Tulipa regelii, Asterella drummondii, Viburnum opulus, Xanthosoma robustum, Pleurothallis maitamae, Dracaena cinnabari, Dionaea muscipula, Huperzia lucidula, Euphorbia duckei.

**Fungi (12):** Favolaschia claudopus, Cladonia imperialis, Ganoderma brownii, Marasmius haematocephalus, Morchella esculenta, Cortinarius archeri, Xylaria tentaculata, Panellus stipticus, Wawelia microspora, Entoloma trachyosporum, Pseudoboletus parasiticus, Psilocybe allenii.

**Protista (19):** Comatricha nigra, Hemitrichia serpula, Metatrichia vesparia, Macrocystis pyrifera, Nereocystis luetkeana, Physarum psittacinum, Ceratiomyxa fruticulosa, Arcyria denudata, Fucus vesiculosus, Himanthalia elongata, Egregia menziesii, Dictyota dichotoma, Heterotrichia versicolor, Leocarpus fragilis, Badhamia polycephala, Sargassum muticum, Diderma effusum, Durvillaea antarctica, Turbinaria ornata.

**Monera (12):** Dolichospermum lemmermannii, Merismopedia convoluta, Arthrospira jenneri, Gomphosphaeria aponina, Anabaena torulosa, Phytoplasma asteris, Gloeotrichia intermedia, Johanseninema constrictum, Hassallia byssoidea, Achromatium oxaliferum, Chroococcus turgidus, Merismopedia tranquilla.
