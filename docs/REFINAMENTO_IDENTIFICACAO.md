# Refinamento técnico e científico da identificação

> Documento histórico. A organização atual por domínios e linhagens, com cobertura e limites atualizados, está em [CLASSIFICACAO_FILOGENETICA.md](CLASSIFICACAO_FILOGENETICA.md). Os agrupamentos antigos deste registro não são táxons naturais ativos.

> Registro hist?rico: marca??es antigas de verifica??o/aprova??o n?o demonstram revis?o humana rastre?vel. Consulte a [governan?a atual, matriz e pend?ncias](CONFORMIDADE.md). Cobertura t?cnica n?o ? certifica??o cient?fica ou jur?dica.

Este relatório registra a etapa anterior às substituições solicitadas pelo usuário. As contagens atuais e a exceção de Monera estão em [Substituições do catálogo](SUBSTITUICOES_CATALOGO.md).

Validação em 14/10/2026, no workspace `C:\glossata_game`, branch `feat/refinamento-identificacao`. Comparação com o commit `c494a669b3d983042240230febca3c90f978579d`. Este relatório descreve o estado final deste catálogo, não uma certificação taxonômica independente.

## Causas encontradas e correções

1. **Fotografia divergente entre pergunta e revisão:** a pergunta ocultava imagens segundo `visualQuiz`, enquanto a revisão recuperava `Species.image`. Gomphosphaeria podia receber arte decorativa antes da resposta e foto depois. Agora `PhotoResolver` valida um objeto Photo e Question mantém sua identidade, créditos e arquivo. O controller preserva o snapshot até Próxima pergunta. Pergunta e revisão usam esse mesmo arquivo. Gomphosphaeria permanece cadastrada e está excluída do sorteio: identificação diagnóstica e autoria ainda pendentes.
2. **Validação insuficiente:** existência de caminho e licença não provavam decodificação, integridade nem adequação. A validação agora exige associação nominal/táxon da fonte, fonte verificada, ID compatível com arquivo/URL, autoria identificada, CC0 ou CC BY e link correspondente, declaração de modificações, SHA-256, recurso RCC acessível, decodificação real e pelo menos 300 pixels em ambas as dimensões. Esses critérios técnicos não substituem a inspeção do organismo ou a revisão por nível.
3. **Observação não equivale à identidade de cada foto:** uma observação de Chiton incluía um escorpião como primeira fotografia; outras observações mostravam carro, pessoas ou cão. Seis vínculos foram substituídos por fotos individuais do organismo. Todas as 120 associações atuais foram conferidas contra a observação, táxon e licença na API. A triagem visual e as restrições de nível estão no manifesto por ID de fotografia.
4. **Perguntas textuais e pistas que revelavam nomes:** o serviço utilizava classificação com gênero/família como pista principal, e os 100 registros importados tinham dica sobre sua origem. O enunciado passou a ser neutro e o conteúdo biológico vem somente de EducationalContent, com fonte e revisão. Os textos históricos foram preservados no catálogo, mas não são usados pelo quiz. Não foi criada uma alternativa textual ou decorativa para registros sem foto elegível.
5. **Controles herdavam estados nativos:** TextAction e TaxonComboBox definem conteúdo, fundo, indicador, popup e estados de foco/pressão. A paleta da janela fornece as cores dos novos controles. O seletor de perfis também reutiliza o componente. Foi corrigido o padding dos links para evitar truncamento.
6. **Encerramento espalhado:** o botão superior agora é Voltar, com um único modal escuro arredondado, texto solicitado e foco inicial em Quero continuar jogando. Continuar/ESC preservam a partida; Enter e mouse foram testados. Confirmar registra uma vez e volta à exploração anterior. Falha de gravação leva ao resultado pendente e à tentativa de salvar novamente. O botão inferior de encerramento foi removido do quiz.
7. **Estatísticas por nome de família:** TaxonomicPerformance agrega cada resposta válida uma vez por reino didático e ID do filo/divisão da espécie. Perguntas de gênero e família contam na mesma classificação do organismo. Nomes iguais em IDs diferentes não são mesclados. Filo ausente, vazio ou ambíguo usa Classificação não disponível. Os ícones são os símbolos dos reinos. A tela cresce com os grupos e usa a rolagem externa, evitando sobreposição no modo compacto.

## Cobertura fotográfica verificada

| Indicador | Quantidade |
|---|---:|
| Espécies cadastradas, sem remoção de IDs | 122 |
| Fotografias atuais registradas e embarcadas | 120 |
| Registros que passam na validação técnica completa do PhotoResolver | 105 |
| Registros aprovados em pelo menos um nível visual | 76 |
| Registros temporariamente fora de todos os sorteios visuais | 46 |
| Fotografias acrescentadas aos registros originais | 20 |
| Fotografias substituídas por outro ID de foto | 6 |
| Mesmo ID de foto baixado em resolução maior | 6 |
| Arquivos atuais CC BY 4.0 | 100 |
| Arquivos atuais CC0 1.0 | 20 |
| Fotografias CC0 com autoria ainda não identificada | 13 |

Os 105 registros tecnicamente válidos incluem fotos de organismos cujo nível diagnóstico ainda não foi aprovado. Os outros 15 são 13 com autoria não identificada e dois registros de hospedeiro/sintoma com imagem ocultada. Nenhum desses registros entra por consequência da existência do arquivo. A pesquisa dos dois registros sem fotografia, Morpho peleides e Caligo memnon, não encontrou correspondência de espécie ativa com o nome exato no iNaturalist; não foi adotado um sinônimo ou organismo diferente sem revisão.

| Reino didático | Cadastradas | Aprovadas em algum nível | Disponíveis em gênero | Disponíveis em espécie |
|---|---:|---:|---:|---:|
| Animalia | 42 | 34 | 22 | 13 |
| Plantae | 20 | 18 | 10 | 0 |
| Fungi | 20 | 14 | 14 | 0 |
| Protista | 20 | 10 | 9 | 0 |
| Monera | 20 | 0 | 0 | 0 |

| Nível solicitado no catálogo misto | Registros elegíveis | Dica validada sem nomes proibidos |
|---|---:|---:|
| Espécie | 13 | 13 |
| Gênero | 55 | 55 |
| Família | 55 | 55 |
| Ordem | 55 | 55 |
| Classe | 76 | 76 |
| Filo/divisão | 76 | 76 |
| Reino científico | 76 | 76 |

Essas contagens são do catálogo completo; um grupo específico pode não ter quatro alternativas distintas naquele nível e fica bloqueado antes da partida. Todas as espécies inicia em gênero para representar Animalia, Plantae, Fungi e Protista. Cada card separa cadastro e cobertura; o explorador informa o número do nível efetivamente selecionado. Monera continua navegável, com zero perguntas visuais aprovadas. Não foi declarada cobertura visual dos cinco reinos.

As licenças são da fotografia individual, não da observação ou de sua foto padrão. Autoria não identificada não foi substituída pelo nome do observador. Arquivos medium/large são variantes originais do iNaturalist: nenhuma foto selecionada foi recortada, retocada ou gerada artificialmente. Os seis aumentos de resolução mantêm o ID do original. [PHOTO_CREDITS.md](PHOTO_CREDITS.md) registra arquivo, crédito, origem, licença e modificações por foto.

### Substituições

- ID 25, **Chiton glaucus**: fotografia `4016368` → `155300806`. [Observação selecionada](https://www.inaturalist.org/observations/93687784).
- ID 43, **Kindbergia oregana**: fotografia `137831916` → `613353234`. [Observação selecionada](https://www.inaturalist.org/observations/337405964).
- ID 47, **Polypodium glycyrrhiza**: fotografia `31018368` → `13657957`. [Observação selecionada](https://www.inaturalist.org/observations/9948504).
- ID 51, **Mimetes chrysanthus**: fotografia `120009929` → `371617112`. [Observação selecionada](https://www.inaturalist.org/observations/210059896).
- ID 79, **Panellus stipticus**: fotografia `178409678` → `178409675`. [Observação selecionada](https://www.inaturalist.org/observations/106224223).
- ID 97, **Badhamia polycephala**: fotografia `448498951` → `291002427`. [Observação selecionada](https://www.inaturalist.org/observations/167950949).

As fotos anteriores e candidatos não selecionados foram preservados localmente. A revisão automática de aprovação bloqueou sua limpeza em lote por risco de apagar arquivos não versionados. Eles não são referenciados em resources.qrc e não entram no executável. A lista das 120 fotos selecionadas continua sendo a fonte do empacotamento.

## Conteúdo educativo e seus limites

Há **18 táxons com introdução**, todos com 40–47 palavras, ID associado e link científico consultável. O recurso funciona para reino, filo/divisão, classe, ordem, família e gênero. O catálogo possui 630 táxons incluindo espécies e níveis intermediários; grupos sem introdução validada mostram uma mensagem discreta. Não foram inventados textos para completar a árvore.

Existem **duas espécies com dica e curiosidade específicas revisadas**: Papilio machaon e Danaus plexippus. Os demais registros elegíveis usam conteúdo biológico de escopo taxonômico validado, quando disponível, e a ficha identifica o escopo da curiosidade. No catálogo completo, 76 registros elegíveis dispõem de uma dica segura em algum nível; isso não equivale a 76 monografias de espécie. A ausência de conteúdo e de dicas compatíveis foi testada com registros sem escopo validado.

A seleção de dicas rejeita referências não verificadas, conteúdos sem fonte, ranks incompatíveis e menções ao nome científico, gênero, alternativas ou nomes da linhagem. A referência da dica pode ser consultada após responder. Curiosidade e dica têm campos e finalidade separados. A validação de nomes não é uma prova automática de ausência de toda pista semântica; textos e alternativas permanecem sujeitos à curadoria editorial.

Fontes dos textos:

- [Animal Diversity Web — University of Michigan](https://animaldiversity.org/accounts/Danaus_plexippus/).
- [Animal Diversity Web — University of Michigan](https://animaldiversity.org/accounts/Papilio_machaon/).
- [North Carolina State University](https://genent.cals.ncsu.edu/insect-identification/order-lepidoptera/family-papilionidae/).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/22-1-prokaryotic-diversity).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/23-2-characteristics-of-protists).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/23-3-groups-of-protists).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/24-1-characteristics-of-fungi).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/24-2-classifications-of-fungi).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/25-1-early-plant-life).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/25-4-seedless-vascular-plants).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/27-1-features-of-the-animal-kingdom).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/28-6-superphylum-ecdysozoa-arthropods).
- [OpenStax — Biology 2e](https://openstax.org/books/biology-2e/pages/29-1-chordates).
- [Publicação científica — Ecology and Evolution](https://pmc.ncbi.nlm.nih.gov/articles/PMC5723595/).
- [Publicação científica — BMC Biology (2020), DOI: 10.1186/s12915-020-0754-1](https://pmc.ncbi.nlm.nih.gov/articles/PMC7052976/). Identificação bibliográfica corrigida em 2026-10-17 durante o PR #12; o registro histórico e a descrição científica foram preservados.
- [University of California, Berkeley — Museum of Paleontology](https://ucmp.berkeley.edu/protista/slimemolds.html).
- [Amateur Entomologists' Society](https://www.amentsoc.org/insects/fact-files/orders/lepidoptera.html).
- [Queensland Museum](https://www.museum.qld.gov.au/learn-and-discover/animals-of-queensland/molluscs/chitons).

A API do iNaturalist é a fonte das fotografias e da classificação registrada. OpenStax, universidades, instituições de pesquisa/museu e publicações científicas fundamentam os textos. O mapeamento didático Protista/Monera permanece separado da classificação da fonte: Mycetozoa não foi renomeado para Amoebozoa nas estatísticas. O grau research do iNaturalist e esta inspeção editorial não equivalem a diagnóstico independente por especialista.

### Registros fora do quiz visual

Motivos podem se sobrepor. Os 46 registros e os respectivos motivos atuais são:

- `9` **Bombyx mori** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `17` **Morpho peleides** — Sem fotografia com correspondência de espécie ativa no nome exato; requer revisão de nomenclatura.
- `22` **Caligo memnon** — Sem fotografia com correspondência de espécie ativa no nome exato; requer revisão de nomenclatura.
- `29` **Latia neritoides** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `32` **Podilymbus podiceps** — Autor da fotografia não identificado pela API. Não confundir autor com observador.
- `33` **Polyergus longicornis** — Autor da fotografia não identificado pela API. Não confundir autor com observador.
- `39` **Bassia bassensis** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `40` **Teleogryllus commodus** — Autor da fotografia não identificado pela API. Não confundir autor com observador.
- `56` **Betula neoalaskana** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `59` **Coptis trifolia** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `66` **Dendrostilbella smaragdina** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `69` **Schizophyllum commune** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `72` **Mycena globulispora** — Fotografia com fase imatura, contexto, luminescência ou detalhes insuficientes para identificação confiável.
- `74` **Erysiphe abbreviata** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `77` **Pandora batallata** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `78` **Psathyrella aquatica** — Autor da fotografia não identificado pela API. Não confundir autor com observador.
- `86` **Tubifera ferruginosa** — Autor da fotografia não identificado pela API. Não confundir autor com observador.
- `87` **Arcella conica** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `91` **Planocarina marginata** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `92` **Galeripora dentata** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `93` **Lepocinclis fusca** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `94` **Difflugia bacillifera** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `98` **Netzelia corona** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `100` **Nebela penardiana** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `101` **Cyclopyxis pirini** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `102` **Awerintzewia cyclostoma** — Estruturas microscópicas ou fungo no hospedeiro; identificação diagnóstica ainda não validada.
- `103` **Sphingomonas phyllosphaerae** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `104` **Dolichospermum lemmermannii** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `105` **Serratia marcescens** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `106` **Thiohalocapsa halophila** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `107` **Merismopedia convoluta** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `108` **Arthrospira jenneri** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `109` **Beggiatoa alba** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `110` **Gomphosphaeria aponina** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `111` **Anabaena torulosa** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `112` **Phytoplasma asteris** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `113` **Gloeotrichia intermedia** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `114` **Leptothrix discophora** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `115` **Johanseninema constrictum** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `116` **Chlamydophila pecorum** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `117` **Hassallia byssoidea** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `118` **Aeromonas caviae** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `119` **Pseudomonas fluorescens** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `120` **Achromatium oxaliferum** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `122` **Chroococcus turgidus** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.
- `124` **Merismopedia tranquilla** — Identificação de microrganismo exige revisão diagnóstica; não usar fotografia genérica, colônia, hospedeiro ou sintomas.

Além desses registros, as identificações de espécie não aprovadas em plantas, fungos e protistas precisam de revisão diagnóstica antes de ampliar ranks. Dados de distribuição legados e as curiosidades arquivadas também continuam pendentes. Não foi utilizada uma micrografia genérica para simular identificação bacteriana.

## Preservação de dados e arquitetura

O catálogo continua com IDs originais e campos históricos. A linhagem dos lepidópteros com correspondência exata recebeu a classificação atual da fonte, incluindo correção de família quando aplicável. A importação mantém deduplicação e cache, aceita somente CC0/CC BY e não aprova automaticamente uma fotografia nova. Reimportação com a meta já atendida mantém o catálogo sem rede. `refine_photos.py` permite auditar fontes e buscar candidatos; uma falha de cache/rede não revoga uma revisão anterior, enquanto uma divergência real na fonte retira a elegibilidade. `build_education.py` reproduz o corpus declarativo.

A alteração SQLite é aditiva: coluna photo_json, sem exclusão de tabelas, espécies locais, perfis ou histórico. O hash do catálogo aplica atualizações por upsert transacional. O banco de jogadores mantém esquema e caminho. Estatísticas e conquistas continuam referentes à partida atual/última, sem persistência cumulativa nova.

A política do ranking permanece: uma linha com a última partida concluída por nome aparado, com unicidade NOCASE do SQLite, mesmo se a nova pontuação for menor. Isso não implementa equivalência de todos os caracteres Unicode. Uma sessão sem respostas não gera resultado. Uma sessão com respostas grava o resultado também no encerramento voluntário, uma vez; falha mantém os pontos em memória, bloqueia nova partida e permite nova tentativa. A pontuação pendente continua dependendo de manter o aplicativo aberto até salvar, como antes.

## Testes e validação executados

- Build Release C++17/Qt 6.11.2/GCC 16.2.0 com CMake/Ninja: concluído.
- CTest: quatro suítes aprovadas, zero falhas.
- GlossataTests: 27 casos, mais inicialização/finalização (29 resultados aprovados). Cobertura de todos os ranks jogáveis, quatro alternativas distintas, licença, hash, arquivo ausente, ID divergente, fonte não verificada, falha de decodificação, resolução insuficiente, hints sem vazamento nominal, snapshot da revisão, agrupamento por IDs, divisão, classificação ausente/ambígua, ranking único/retry e migração com dados anteriores.
- GlossataInterfaceTests: fluxo integrado com mouse e teclado aprovado, com três resultados incluindo inicialização/finalização. Percorre cinco reinos, confirma bloqueio de Monera, Protista → Protozoa → Mycetozoa → introdução → quiz, jogo misto, acerto/erro, ficha, próxima pergunta, modal cancelado e confirmado, retorno ao grupo, ranking e estatísticas com quatro reinos reais.
- GlossataDiscordTests: três casos aprovados mais inicialização/finalização. Usa servidores IPC fictícios e não publica presença real.
- GlossataImportTests: nove testes Python aprovados sem requisições ao serviço remoto. Verificam identidade/textos históricos, licenças, taxonomia, hashes, recursos, aprovação por ID/fonte, reimportação idempotente e falha segura sem cache.
- qmllint em todos os QML: concluído sem avisos.
- Capturas em 1240×820 e 1000×700 inspecionadas, incluindo popup, foco/breadcrumbs, confirmação, informações e estatísticas superiores/inferiores. O teste de interface rejeita avisos QML. Conteúdo extenso usa rolagem, sem sobreposição de controles.
- Empacotamento local atualizado. SHA-256 do executável em build e package conferido e idêntico. O launcher e o ícone existente foram preservados.

Evidências reproduzíveis: `build/tests/core-results.txt`, `ui-test-results.txt`, `catalog-audit.json`, `previews/` e `resources/data/photo_audit.json`. Os primeiros são artefatos gerados e não entram no Git. Os testes técnicos verificam associação, licença e integridade; não demonstram por si só a identidade biológica de uma foto. Essa parte depende da conferência individual e da política conservadora registrada.

## Arquivos entregues

A lista abaixo inclui arquivos de código/conteúdo e fotografias selecionadas criados ou alterados nesta etapa. Não foram removidos arquivos do projeto. Build, pacote e capturas são gerados e ignorados. Candidatos fotográficos não selecionados permanecem locais e não são recursos da aplicação.

- `CMakeLists.txt`.
- `PROJECT_CONTEXT.md`.
- `README.md`.
- `docs/CINCO_REINOS.md`.
- `docs/PHOTO_CREDITS.md`.
- `docs/REFINAMENTO_IDENTIFICACAO.md`.
- `resources.qrc`.
- `resources/data/education.json`.
- `resources/data/import_report.json`.
- `resources/data/photo_audit.json`.
- `resources/data/photo_reviews.json`.
- `resources/data/species.json`.
- `resources/data/taxonomy.json`.
- `resources/images/inaturalist/13657957.jpg`.
- `resources/images/inaturalist/137265535.jpg`.
- `resources/images/inaturalist/155300806.jpeg`.
- `resources/images/inaturalist/175474265.jpeg`.
- `resources/images/inaturalist/178409675.jpeg`.
- `resources/images/inaturalist/221780102.jpeg`.
- `resources/images/inaturalist/280989671.jpeg`.
- `resources/images/inaturalist/291002427.jpg`.
- `resources/images/inaturalist/320741885.jpeg`.
- `resources/images/inaturalist/321803659.jpg`.
- `resources/images/inaturalist/322926715.jpg`.
- `resources/images/inaturalist/34980868.jpg`.
- `resources/images/inaturalist/357771622.jpeg`.
- `resources/images/inaturalist/359063525.jpeg`.
- `resources/images/inaturalist/371617112.jpeg`.
- `resources/images/inaturalist/376647594.jpeg`.
- `resources/images/inaturalist/401685857.jpg`.
- `resources/images/inaturalist/407087233.jpg`.
- `resources/images/inaturalist/43280362.jpg`.
- `resources/images/inaturalist/44218494.jpg`.
- `resources/images/inaturalist/45110154.jpeg`.
- `resources/images/inaturalist/45341427.jpeg`.
- `resources/images/inaturalist/51317278.jpeg`.
- `resources/images/inaturalist/560140869.jpg`.
- `resources/images/inaturalist/5777965.jpeg`.
- `resources/images/inaturalist/586365614.jpg`.
- `resources/images/inaturalist/60643931.jpg`.
- `resources/images/inaturalist/613353234.jpg`.
- `resources/images/inaturalist/654470799.jpg`.
- `resources/images/inaturalist/655494534.jpg`.
- `resources/images/inaturalist/664105606.jpg`.
- `resources/images/inaturalist/749769318.jpg`.
- `resources/qml/Main.qml`.
- `resources/qml/components/EndGameDialog.qml`.
- `resources/qml/components/ExploreCard.qml`.
- `resources/qml/components/SpeciesPortrait.qml`.
- `resources/qml/components/TaxonComboBox.qml`.
- `resources/qml/components/TextAction.qml`.
- `resources/qml/views/CategoryView.qml`.
- `resources/qml/views/FeedbackView.qml`.
- `resources/qml/views/InfoView.qml`.
- `resources/qml/views/QuizView.qml`.
- `resources/qml/views/SettingsView.qml`.
- `resources/qml/views/StatisticsView.qml`.
- `resources/qml/views/TaxonomyView.qml`.
- `scripts/build_education.py`.
- `scripts/import_inaturalist.py`.
- `scripts/refine_photos.py`.
- `src/application/EducationalContent.cpp`.
- `src/application/EducationalContent.h`.
- `src/application/GameService.cpp`.
- `src/application/GameService.h`.
- `src/application/TaxonomyIndex.cpp`.
- `src/application/TaxonomyIndex.h`.
- `src/domain/Photo.h`.
- `src/domain/Question.h`.
- `src/domain/QuizSelection.h`.
- `src/domain/Species.h`.
- `src/domain/TaxonomicPerformance.h`.
- `src/infrastructure/content/JsonSpeciesCatalog.cpp`.
- `src/infrastructure/content/PhotoResolver.cpp`.
- `src/infrastructure/content/PhotoResolver.h`.
- `src/infrastructure/content/SqliteSpeciesCatalog.cpp`.
- `src/presentation/GameController.cpp`.
- `src/presentation/GameController.h`.
- `tests/CMakeLists.txt`.
- `tests/GameServiceTests.cpp`.
- `tests/QmlInterfaceTests.cpp`.
- `tests/fixtures/invalid.jpg`.
- `tests/fixtures/small.png`.
- `tests/photo_fixtures.qrc`.
- `tests/test_import_inaturalist.py`.

## Pendências práticas

A cobertura de microscopia e diagnóstico não foi resolvida por improvisação: os registros não aprovados permanecem fora do sorteio. Há grupos sem textos específicos, apenas duas revisões educativas específicas de espécie e classificação/distribuição ainda sujeitas à revisão. A homologação do pacote em um Windows sem Qt/MSYS2 instalado continua pendente. O empacotador informou ausência das traduções Qt e de DLLs opcionais de Direct3D 12; os textos próprios do jogo estão em português e os testes desta máquina passaram.

Esta etapa está registrada na branch local de refinamento; a publicação da expansão já existente em main não foi substituída.
