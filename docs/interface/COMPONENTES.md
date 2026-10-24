# Componentes compartilhados

## Conquistas — 2.3.3

O refinamento final combina filtros de estado/categoria/`visualLevel` com AND, persistidos em `Main.achievementsPresentation` durante a navegação. A Platina acrescenta placas facetadas e reflexos estáticos em `QtQuick.Shapes`; os três outros materiais e os 30 glifos permanecem iguais. [Filtro e comparação Prata/Platina](VERSOES/2.3.3/filtros-nivel-platina/README.md).

`AchievementBadge.qml` recebe `glyph`, `visualLevel` e `unlocked`, com geometria/cor e quatro materiais em `AchievementBadgePaths.js`. A view consome nível e nome projetados pelo backend; não decide classe por posição, nome ou progresso. Desenha moldura, fitas e glifo em coordenadas normalizadas, escaladas pela menor dimensão; fallback para folha, sem animação interna nem validação de progresso. A view define tamanho e resposta à pressão; o componente atenua separadamente moldura e símbolo quando bloqueado, preservando os ornamentos da classe. `AchievementsView` reutiliza `UiButton`, `UiIcon`, `Theme`, `Dialog` e `ScrollView`; filtros e cartões possuem nomes acessíveis, foco visível e ativação por teclado. O diálogo devolve foco ao cartão de origem. [Evidências](VERSOES/2.3.3/README.md).

## Estatísticas — cores e dicas em andamento

`StatisticsView.groupColor(id)` usa os IDs do catálogo e um fallback determinístico, sem consultar a posição dos grupos. Barras principais e percentuais recebem a cor do grupo; subgrupos usam um clareamento discreto da mesma cor. Nomes, contagens e taxas continuam presentes. `statisticsRankBorder0..4` e `statisticsRankPosition0..4` identificam bordas/números: ouro, prata, bronze e padrão. O foco substitui a borda por `Theme.focus`/`focusWidth`, sem alterar a ordem ou os marcadores.

`visualHints` é uma projeção derivada de `sessionMetrics`, `hints`, `answerPending` e `gameFinished`. Soma apenas dicas disponíveis e reveladas, distintas por ID, da pergunta ativa ainda sem resposta; incrementa questões com dicas uma única vez. `extraHintChanged`, `scoreChanged`, `answerDetailsChanged` e `gameFinishedChanged` invalidam as dependências existentes, sem acumulador, novos sinais ou escritas. Durante feedback ou após encerramento, usa somente os totais confirmados. Recuperar ou recriar a página recompõe a projeção. `statisticsHintsNotice` informa essa distinção; histórico e demais métricas continuam consolidados. [Decisões e validação](VERSOES/2.3.2/cores-e-dicas/README.md).

## Estatísticas — refinamentos da PR #36

`StatisticsView` apresenta `statisticsTopPanels` depois de `statisticsHeader`, antes dos indicadores e histórico. A partir de 900 pixels úteis, ambos os painéis mantêm proporções e alinhamento superior; abaixo disso, `statisticsTaxonomyTab`/`statisticsRankingTab` alternam sua visibilidade sem remover modelos/dados. `Main.statisticsPresentation.panel` guarda a seleção em memória, conservada ao recriar o Loader e ao redimensionar. Os seletores usam `UiButton.compact`, estado marcado, Tab/Espaço e foco do Design System. Se o redimensionamento ocultar o painel focado, o foco passa ao seletor correspondente ao painel selecionado.

`Main.pageWheelArea` inclui Estatísticas nas mesmas condições existentes de Início/Explorar. Continua usando `conventionalWheel`, 110 pixels por incremento, animação pelos tokens, caminho nativo para pixels/ângulos parciais, interrupção por movimento/foco/pressão da barra e faixa de segurança compartilhada. Não há rolagem interna nos painéis. [Revisão e testes](VERSOES/2.3.2/refinamentos-topo/README.md).

## Estatísticas — 2.3.2 (contrato inicial)

`StatisticsView` mantém `app`/`backend` e apenas projeta contratos existentes. `statisticsAnswered`, `statisticsCorrect`, `statisticsIncorrect` e `statisticsAccuracy` reutilizam `StatCard` sem mudar o componente compartilhado. Desempenho e histórico usam textos decimais do controlador. Os indicadores se reorganizam conforme a largura útil; `statisticsTaxonomyPanel` e `statisticsRankingPanel` empilham abaixo de 900 pixels lógicos. Alturas dos painéis seguem o conteúdo, com a rolagem única do shell.

`freeRankingButton` e `competitiveRankingButton` preservam os comandos e expõem o estado selecionado. As cinco posições máximas retornadas pelo backend são focáveis (`statisticsRank0..4`), com nome integral, pontuação, último resultado/data, nome registrado na descrição acessível e indicadores de legado/exclusão. Tab/Shift+Tab preservam a ordem visual, com foco visível e enquadramento pelo shell. As barras taxonômicas representam acertos/respostas, sem interpolação animada ou novas classificações. [Semântica e validação](VERSOES/2.3.2/README.md).

## Quiz — 2.3.1

`QuizView` recupera o layout anterior, preservando `answerOption0..3`, `questionPortrait`, `possibleScoreLabel`, `hintButton`, `confirmAnswerButton` e `backFromQuizButton`. A esquerda tem largura limitada de 37%; a direita contém `quizAnswersScroll`, com alternativas e dicas, sem impor sua altura mínima à foto. Dica/Confirmar ficam fixos na base da coluna. Foco local enquadra o controle dentro da rolagem direita. Títulos `revealedHintTitle0..2` consultam assistência/categoria do backend, sem inferir nível pelo índice. `quizAnswerCounts` projeta `correctAnswers` e `answeredCount - correctAnswers`; não escreve no banco.

`SpeciesPortrait` retorna à geometria, cores e `Image.PreserveAspectFit` anteriores, sem variante exclusiva ou recorte. Créditos e avisos permanecem completos. `HintPanel` recupera sua largura de 540 pixels, título, botões e cores anteriores; conserva tetos, fonte, revisão, relato, Esc e retorno do foco. `InfoView.speciesDescriptionRegion` preserva a estrutura abaixo da foto e créditos e mostra somente "Descrição educativa detalhada ainda não disponível." Sem `description`, `hasDescription`, campo persistente ou associação a outro tópico. `profile.morphology` pertence ao modelo original de seções, entre Nutrição e Conservação na coluna direita, preservando ícone, título, nota, referências e revisão. `speciesSummary` continua à direita. [Correção e testes](VERSOES/2.3.1/descricao-independente/README.md). [Documentação atual](VERSOES/2.3.1/restauracao-visual/README.md).

## PR #33 — configuração compacta e fontes do grupo

Nas janelas amplas, a dificuldade acompanha o seletor com intervalo de `spaceLg` (20 pixels lógicos), alinhada à esquerda em uma coluna flexível; o espaço excedente fica depois do texto e o início conserva sua largura intrínseca na margem direita. Nas compactas, a dificuldade fica abaixo do seletor, ao lado do início à direita. Os controles de cada linha mantêm alinhamento vertical central. [Verificação atual e escalas](VERSOES/2.2.3/alinhamento-dificuldade/README.md); [evidências anteriores](VERSOES/2.2.3/alinhamento-iniciar/README.md).

`TaxonomyView.playConfigurationPanel` calcula a altura pelo conteúdo e usa margens de `spaceMd`, título discreto e espaçamento `spaceXs`. Com 820 pixels úteis nos controles, identificação, dificuldade informativa e Iniciar partida ficam lado a lado; abaixo desse limiar, o início passa para uma segunda linha. O campo de identificação fica limitado à largura do conjunto de 360 pixels, sem esticar com a janela. A linha informativa de elegibilidade fica abaixo e pode quebrar sem corte. Modelo, dificuldade, condição de habilitação e chamada de início são os mesmos.

`playTaxonSourcesButton` abre o diálogo local `playTaxonSourcesDialog`, exclusivo da configuração de Jogar. Ele apresenta identificação, enquadramento, referência educativa, estado/limitações de revisão e proveniência já expostos pelo controlador. Os links preservam suas URLs, a rolagem acompanha o foco e Esc/Fechar devolvem o foco ao botão de origem. Sem fotografia própria nesta página, os créditos fotográficos continuam vinculados aos cartões descendentes. Avisos científicos permanecem visíveis fora do diálogo. Os diálogos globais e a apresentação do Explorar não foram reformulados. [Evidências](VERSOES/2.2.3/configuracao-compacta/README.md).

## PR #33 — configuração e contexto da seleção

`PlayModeSelector` fica exclusivamente na seleção principal de Jogar. `TaxonomyView`, no contexto de jogo, mostra resumo existente, nível de identificação, `playDifficultyNotice` somente informativo, contagem elegível e início explícito antes dos descendentes. A dificuldade vem do controlador e não é reconfigurada pela navegação.

`Main.playPresentation` guarda em memória aba, rolagem, controle de origem e indicação de foco por teclado. `CategoryView.rememberPresentation` e o timer `restorePresentation` atendem tanto Jogar quanto Explorar, mantendo mapas separados e a chave científica da pesquisa intacta. O retorno por teclado recupera o controle original; o retorno por mouse conserva a posição sem forçar o enquadramento de um cartão parcialmente visível. Posições são limitadas à área disponível após mudança de tamanho da janela. Sem escrita em banco ou alteração de sessões. [Contrato e testes](VERSOES/2.2.3/refinamentos-navegacao/README.md).

## Jogar — 2.2.3

`ExploreCard.playSelection` reutiliza a fotografia integral/centralizada, carregamento por viewport, hover passivo, foco, dimensões intrínsecas e diálogo de créditos da variante moderna. `entry.playLevels` é uma projeção de `identificationLevels`, não uma aprovação. O cartão informa quantidade catalogada e número de níveis disponíveis; o painel `playEligibleCount` mostra a contagem elegível e as respostas distintas no nível escolhido. A dificuldade pode restringir a seleção final, validada pelo backend. Grupos indisponíveis continuam navegáveis. O jogo misto mantém o gênero e sua contagem elegível existentes, com ação explícita de início.

`playTab_kingdoms/domains/advanced` são botões compactos em grupo exclusivo. A grade usa largura mínima de 280 pixels lógicos e a maior altura intrínseca da seção; descendentes usam o mesmo contrato. `PlayModeSelector` apresenta Livre e Competitivo em painéis responsivos com descrições curtas; a dificuldade pertence somente ao Livre e o início competitivo continua global. Os IDs de controles existentes e as APIs de início são preservados. Em Explorar, o botão contextual preserva padding `spaceMd` e a ação principal recebe a mesma margem esquerda, além da margem superior `spaceXs`, alinhando os textos sem ampliar a área clicável. [Capturas e testes](VERSOES/2.2.3/README.md).

## PR #32 — uniformidade e rolagem

`ExploreCard` distingue o hover local dos créditos com fundo/borda, conservando largura, espaço inerte, foco e transições. `encyclopediaCardPhotoState` mostra mensagem curta quando a foto falta; `pendingReason` completo fica no diálogo. Resumos têm duas linhas; atribuições obrigatórias não são elididas. Grids compartilham a maior `implicitHeight` em `cardHeight`/`Layout.preferredHeight`; a galeria usa `RowLayout` e `Layout.fillHeight`. Isso mantém o rodapé alinhado e recalcula a altura ao variar a largura/tipografia, sem um teto fixo que corte texto.

`Main.pageScroll` reserva `contentGutter` para a barra em todas as páginas. Botões de carregamento usam `UiButton.compact`; o componente compartilhado não muda. `pageWheelArea`, uma `MouseArea` atrás do conteúdo e sem captura de botões do mouse, trata somente incrementos verticais convencionais em Início/Explorar: 110 pixels por detente, animação curta opcional e aceitação exclusiva. `scrollGestureEnabled: false` mantém gestos sintetizados nativos; eventos precisos/horizontais são recusados para seguir seu fluxo original. Foco, barra, movimento nativo e troca de página interrompem a animação pendente. [Contratos e validação](VERSOES/2.2.2/usabilidade/README.md).

## PR #32 — correções de créditos e interação

`photo.contextualAttribution` indica somente a apresentação: fotos CC0/CC BY verificadas sem pedido adicional e os créditos legados CC BY vinculados à evidência usam o diálogo adjacente. Não determina elegibilidade, versão, permissão ou distribuição. Autoria, título, licença, origem e modificações continuam integrais offline. `encyclopediaCardLicenseNotice` preserva o aviso curto de versão não comprovada; ausência da indicação ou pedido adicional mantém o fallback visível.

`exploreCardCreditsRow` ocupa a largura disponível, mas `exploreCardCreditsButton` ocupa somente `min(implicitWidth, row.width)`, alinhado à esquerda. O espaço restante absorve cliques sem executar nenhuma ação; gestos de rolagem continuam disponíveis ao Flickable. Tab/Space e foco acessível permanecem. Um `HoverHandler` passivo acompanha o cartão enciclopédico inteiro, enquanto o hover automático do `Button` externo fica desativado. O `TextAction` conserva seu hover/foco e animação; o componente compartilhado e a variante Jogar não mudam. [Política por licença, capturas e testes](VERSOES/2.2.2/correcoes-creditos/README.md).

## PR #32 — refinamentos finais

`ExploreCard.educationalNote` apresenta o mapa elegível projetado pelo índice em no máximo duas linhas; não cria resumo alternativo. `shortClassification` lê `entry.classificationSummary`, com ranks fornecidos por C++. `exploreCardCreditsButton` abre um `Dialog` criado sob demanda, separado da ação principal. Mantém autoria/licença/versão, origem, modificações históricas, fontes e avisos. Tab revela os links na rolagem interna; Escape/Fechar devolvem foco ao botão. `photo.contextualAttribution` é indicação conservadora de apresentação para metadados verificados, não aprovação: quando ausente/falsa, a autoria/licença e eventuais atribuições específicas continuam junto à foto. O corpo do diálogo distingue histórico literal e apresentação atual.

Imagens enciclopédicas ficam centralizadas em ambos os eixos com escala máxima 1, sem recorte ou deslocamento por foco. Fotografia de Jogar permanece igual. `TaxonomyView.exploreScientificFooter` fica depois de `taxonChildrenGrid`/estado informativo, com os expansores existentes de classificação, fontes e foto; identificação e conteúdo educativo precedem os descendentes. A galeria mantém seleção/controles/setas, acrescentando intervalo visível e indicação de cartões fora da área. [Contratos, política contextual e evidências](VERSOES/2.2.2/rodada-final/README.md). Os registros abaixo são históricos.

## PR #32 — rodada 2

`ExploreCard.encyclopedic` compacta molduras e textos; `landscape` dispõe foto/texto lado a lado para grupos com largura ≥450 px. `shortClassification` apresenta somente os dois ancestrais mais próximos do `pathText` existente, sem inferir ranks; o caminho integral permanece acessível. Autoria/licença não são truncadas. A fotografia preserva todo o raster e precisa intersectar os viewports horizontal e vertical para receber `source`; `outerViewport` acompanha a rolagem externa da galeria. A variante padrão permanece visualmente igual em Jogar.

`CategoryView` reúne busca/cabeçalho, `clearExploreQuery` (texto) e `clearExploreSearch` (todos os filtros). `exploreFeaturedGrid` passa a ser um Flickable horizontal com quatro registros existentes, controles `previousFeaturedOrganisms`/`nextFeaturedOrganisms` e setas esquerda/direita nos cartões. `Main.explorePresentation` guarda em memória chave de texto/domínio/rank, limites, rolagem vertical/horizontal e controle de retorno; a view restaura lotes e foco após o Loader recriá-la. Nova consulta/filtro reinicia limites. Contratos C++ e debounce da rodada 1 permanecem iguais.

`TaxonomyView` distingue grupo/organismo, preserva os breadcrumbs e mantém atribuição/avisos visíveis. `exploreClassification` e `exploreSources` expandem classificação e fontes; `explorePhotoCredits` conserva o acesso ao título, modificações e URLs. A origem fotográfica usa `photoUrl || sourceUrl`, conforme o mapa existente. Seções de Jogar e diálogos gerais não são reformulados. [Contratos, capturas e limites atuais](VERSOES/2.2.2/rodada-2/README.md). Os registros abaixo descrevem as entregas anteriores.

## PR #32 — rodada 1

`CategoryView` mantém limites de apresentação independentes, começando em 12 grupos e 12 organismos, com Carregar mais para cada seção. O prefixo das listas de `TaxonomyIndex` conserva relevância, totais e IDs; edição/filtro reinicia ambos os limites. Voltar de um táxon conserva consulta/filtros e reabre a primeira página. O foco por teclado passa ao primeiro cartão recém-solicitado. Colunas são calculadas pela largura disponível e mínimo de 280 px, com distribuição equilibrada dos quatro destaques; não há novo desenho dos cartões ou da galeria.

`GameController` notifica `exploreQuery`, domínio e rank separadamente. O texto atual é armazenado imediatamente; um `QTimer` de 220 ms adia somente a notificação de apresentação, exposta por `exploreSearchPending`. Filtros, Enter, término de edição e limpeza finalizam a espera; não existe cópia antiga do texto para restaurar por timeout. QML apresenta Atualizando pesquisa durante a espera e não mistura cartões antigos com o novo texto. `TaxonomyIndex` continua único responsável pela pesquisa científica.

`ExploreCard` enciclopédico só fornece `source` à imagem quando sua moldura intersecta o `Flickable` ancestral. O `FeatheredPhoto` oculto dessa variante fica sem fonte. A fotografia mantém proporção e raster integral; foco existente é usado apenas para posicionar a imagem nas margens livres. Ponto focal não é evidência de limites seguros para recorte. Verificações de viewport e foco após Carregar mais usam timers filhos dos componentes, cancelados ao destruir a view; não deixam callbacks em contextos QML removidos. A variante de Jogar preserva sua apresentação fotográfica. Contadores e nomes acessíveis flexionam singular/plural.

`TaxonomyView`, no contexto Explorar, mostra domínio, dois ancestrais próximos e nível atual, com controle para expandir os intermediários. Os botões conservam índices/IDs do caminho integral; o nível atual recebe seminegrito e identificação textual/acessível. Expansão admite Tab/Espaço, recolhe ao mudar de táxon e reduz novamente a altura do Flow. A trilha de Jogar permanece integral. [Evidências e limites](VERSOES/2.2.2/rodada-1/README.md).

## Explorar — 2.2.2

`ExploreCard.encyclopedic: false` preserva a variante anterior. Quando `true`, usa `Theme`, foto proporcional em destaque, rank, título científico com itálico nos níveis espécie/gênero, nome comum disponível, contagem ou caminho e autoria/licença visíveis. Altura acompanha texto, foco por teclado usa `visualFocus` dourado e hover respeita redução de movimento. `entry.kind` distingue `organism` e `group`; `entry.id` é o destino taxonômico e `speciesId` identifica o registro quando disponível. Tooltip extenso permanece somente na variante anterior de Jogar.

`CategoryView.browsingOnly` acrescenta `exploreSearchField`, `exploreDomainFilter`, `exploreRankFilter`, `clearExploreSearch`, cartões `featuredOrganism*`, `exploreGroupResult*` e `exploreOrganismResult*`, resumo e estado vazio. Busca/filtros são propriedades de `GameController`; não filtrar o catálogo novamente em QML. `TaxonomyView` acrescenta fotografia/cabeçalho e `explorePhotoCredits/explorePhotoDetails`, sem diálogo novo ou mudanças no quiz. Breadcrumbs limitam sua largura à página e mantêm o nome integral acessível; seleção de um ancestral/descendente traz o início da página ao foco.

## Descoberta e indicadores — 2.2.1-B

`HomeView` acrescenta `dailyDiscoveryPanel` abaixo dos controles do carrossel. A foto usa `Image.PreserveAspectFit`, com nome científico acessível, créditos locais e alternativa para falha de carregamento. Nome científico, nome comum disponível e resumo preservam texto simples e quebra de linha. `dailyDiscoverySource` mantém texto, URL acessível, clique e estados de interação, sem tooltip. `dailyDiscoveryExplore` abre o táxon exato no contexto Explorar. `dailyDiscoveryCredits` abre `dailyDiscoveryCreditsDialog`, com metadados vinculados à descoberta exibida, avisos e links da fotografia; o rodapé mantém acesso aos créditos gerais. Fechar ou Escape devolve foco ao botão. O diálogo do carrossel permanece independente e inalterado.

`StatCard.description` é opcional, vazia por padrão; nesse caso a altura anterior permanece 118 px. Com descrição, a altura acompanha o conteúdo a partir de 154 px. O nome acessível inclui título, valor e descrição. Os três usos novos têm identificadores `homeOrganismCount`, `homeDomainCount` e `homePhotoCount`, ligados a propriedades notificáveis do controlador. Os contratos dos demais usos são preservados.

`StatCard.iconSize` e `iconContainerSize` são opcionais, com padrões anteriores de 23 e 42 px. Somente os três indicadores da Home passam 28 e 48 px. O contêiner fica centralizado verticalmente, com raio proporcional e sem mudança de cores/tipografia. `homePhotoCount` passa a se chamar visualmente “Identificação por espécie”; o valor e os filtros C++ permanecem os mesmos.

## Início — 2.2.1-A

`HomeView` reutiliza `UiButton`, `TextAction` e `StatCard`, sem alterar seus contratos. Dois buffers `Image` locais cuidam do carrossel; `FeatheredPhoto` permanece nos cartões. `homePlayButton` continua chamando `app.openPlay()`; `homeExploreButton` chama `app.openExplore()`. `homeCarousel.currentIndex` identifica o slide confirmado ao concluir a transição, mantendo título, descrição, nome e créditos anteriores durante o crossfade. Setas circulam, Home/End selecionam extremos e os indicadores de posição aceitam Tab/Espaço. A preferência existente de movimento controla o crossfade de 600 ms e desativa o avanço automático de oito segundos; hover na legenda/controles, botão pressionado, foco de teclado, créditos e visibilidade pausam o Timer. Hover nas demais regiões da Home permite o avanço. Os controles preservam foco visível e o diálogo devolve o foco ao botão de créditos ao fechar.

Os metadados de mídia expostos incluem título, autor, licença, URL, coleta, caminho e hash da evidência. Nenhum componente QML decide permissões ou registra revisão. O estado vazio não oferece créditos fictícios nem desativa as ações de navegação.

Os indicadores visuais usam somente pontos; a posição e o total permanecem no nome acessível de cada ponto. `homeCreditsButton` conserva a largura implícita de `TextAction`, alinhada à direita da legenda; hover e foco não se estendem ao restante do cartão.

`homePhotoCaption` é um layout transparente: título do grupo, descrição existente e linha de nome científico/Créditos. `homeCaptionShade` desenha somente o degradê inferior com transparência vertical e lateral, repintado em redimensionamentos e mudanças de altura da legenda. A camada permanece independente das imagens e do crossfade; os recortes dos cantos continuam sendo aplicados por cima.

## Integração de navegação — 2.1.3

- `Main.qml`: shell central com seis destinos, cabeçalho por seção e contexto, larguras/margens responsivas de `Theme.qml`. Os `NavItem` existentes recebem identificadores estáveis `navHome`, `navExplore`, `navPlay`, `navStatistics`, `navAchievements` e `navSettings`.
- `CategoryView` e `TaxonomyView`: nova propriedade opcional `browsingOnly`, padrão `false`. Ela oculta ações exclusivas de jogo na exploração; não muda os dados nem os contratos existentes. Os objectNames anteriores são mantidos.
- `Theme.qml`: acrescenta tokens de largura lateral, margem de página, legenda de seção e marca. Sem novos recursos gráficos ou dependências.

## Design system — 2.1.2

| Componente | Contrato preservado | Revisão visual |
| --- | --- | --- |
| `UiButton` | Button; `iconName`, `secondary`, `danger`, `compact`, `text`, sinais e dimensões gerais | Fundos verde/petróleo/perigo; foco dourado; transição de cor; desabilitado explícito. A precedência de secondary sobre danger permanece. |
| `NavItem` | Button; `iconName`, `selected`, `text` e clique | Seleção verde-floresta com detalhe dourado, foco de 2 px, hover e descrição acessível de seleção. |
| `StatCard` | Rectangle; `title`, `value`, `iconName`, `accent`, tamanho implícito 200 × 118 | Superfície, borda, texto e espaçamento centralizados; acentos das telas continuam válidos. |
| `TextAction` | Button; texto, fonte ajustável, clique e `textActionBackground` | Fundo discreto para hover/pressão, texto claro, foco dourado e desabilitado legível. |

Importação local: `import "../style" as Style`. Os valores vêm de `Style.Theme`. Não adicionar regras de jogo ou catálogo ao tema. Preservar propriedades públicas e objectName dos testes. Geometria específica de ícones não precisa compartilhar tokens apenas por coincidir numericamente.

Main.qml conecta a opção existente de movimento ao singleton e à transição de página. Cards de exploração e fotografias não foram reformulados. Ao migrar outros componentes em etapa autorizada, conferir estados, teclado, texto longo e janela mínima e arquivar capturas conforme [o procedimento](README.md).
