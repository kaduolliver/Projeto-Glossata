# Arquitetura visual

## Quiz — 2.3.1

Modernização restrita à apresentação em `QuizView`, `HintPanel` e à variante opcional `SpeciesPortrait.quizPresentation`. Grupo, nível, alternativas, progresso, pontuação e dicas vêm das APIs existentes. Não há cálculo científico, seleção de questões ou pontuação paralela em QML. O estado local da seleção dura uma pergunta; diálogos não o limpam. O backend continua responsável por aceitação, checkpoint e recuperação. [Contratos e evidências](VERSOES/2.3.1/README.md).

## PR #33 — painel compacto e fontes contextuais

O ajuste fica em `TaxonomyView.qml`: apresentação adaptativa dos mesmos controles, diálogo local para dados científicos disponíveis e posicionamento inicial da página do grupo. O timer `positionPlayTop` aguarda o layout e cancela a inércia antes de colocar a página taxonômica de Jogar no topo. A margem do botão Voltar conserva sua moldura visível. Não altera `Main`, `CategoryView`, `playPresentation` ou sua restauração; em Explorar, o timer não reposiciona a página. O diálogo não consulta rede para mostrar os dados e não deriva aprovações. [Validação e capturas](VERSOES/2.2.3/configuracao-compacta/README.md).

## PR #33 — retorno ao contexto de seleção

O shell conserva `playPresentation` separadamente de `explorePresentation`; ambos são estado efêmero de apresentação, com restauração compartilhada em `CategoryView`. Só a página de seleção grava o mapa de Jogar, impedindo que o reset do Loader em outra tela substitua a rolagem salva. As entradas de grupo e os atalhos registram o contexto antes da navegação. Configuração do grupo não repete os painéis globais nem escreve dificuldade no controlador. Funções de início, regras científicas, persistência e recuperação permanecem intactas. [Evidências](VERSOES/2.2.3/refinamentos-navegacao/README.md).

## Jogar — 2.2.3

`TaxonomyIndex.playEntry/playGroups/playChildren` acrescentam projeções de apresentação usando os IDs, descendentes, atalhos e `identificationLevels` existentes. Não alteram `describe`, busca, critérios científicos nem geração de partidas. `GameController` expõe os mapas/listas; QML escolhe somente qual seção mostrar. Reinos usa os três atalhos reais; Domínios usa as raízes; Grupos avançados mostra os descendentes imediatos de Eukaryota, com acesso aos demais níveis pela árvore original. São entradas de navegação, não uma nova classificação.

`CategoryView` mantém os fluxos de Explorar e Jogar distintos. `ExploreCard.playSelection` opta pela composição fotográfica já existente, com disponibilidade por níveis e ação de seleção; `encyclopedic` conserva a enciclopédia. Não há nova cópia dos créditos ou da política de atribuição. Modos/configuração são apresentados por `PlayModeSelector`; ações de início continuam nos métodos existentes de `Main.qml`/backend, que permanecem sem alteração. Quiz e persistência não são reformulados. [Registro e validação](VERSOES/2.2.3/README.md).

## PR #32 — dimensionamento e entrada de rolagem

As correções de usabilidade ficam no QML: altura intrínseca compartilhada entre cartões, hover local e estados curtos de indisponibilidade. A ciência continua projetada pelo índice, sem alterar seleção ou dados. `Main.qml` centraliza a faixa da barra e o tratamento exclusivo da roda convencional em Início/Explorar; eventos de precisão e eixos horizontais continuam nativos. O ajuste não aplica animação genérica à restauração de `contentY` e não muda regras de Jogar. [Registro atual](VERSOES/2.2.2/usabilidade/README.md).

## PR #32 — atribuição contextual e interação

O adaptador de mídia projeta atribuição contextual para as versões CC BY verificadas sem pedido adicional, incluindo 2.0/3.0; `featuredOrganisms` projeta a mesma apresentação para os créditos legados CC BY vinculados à evidência individual. Não muda a seleção, a versão declarada nem a decisão de distribuição. QML mantém os avisos e apresenta o conteúdo local no diálogo. A linha de créditos separa a área do botão do espaço inerte; observação passiva de hover evita depender da propagação entre controles aninhados. Nenhuma regra de pesquisa, sessão ou navegação muda. [Decisões e validação](VERSOES/2.2.2/correcoes-creditos/README.md).

## PR #32 — refinamentos finais

`TaxonomyIndex.describe` acrescenta `educationalNote` e `classificationSummary` aos mapas existentes, conservando API, pesquisa, filtros, IDs e contagens. O primeiro reutiliza nota de `EducationalContent` com revisão utilizável editorial/especializada e fonte completa; o segundo usa apenas ranks/nós existentes e prefere família/gênero. QML limita linhas e organiza a apresentação; não decide elegibilidade científica nem infere classificação. O adaptador de mídia expõe versão/créditos/estado já documentados e uma indicação conservadora de atribuição contextual para fotos verificadas; dados e permissões originais ficam intactos.

Diálogos dos cartões são criados sob demanda e pertencem ao componente, com foco/rolagem internos e ação independente da navegação. O estado efêmero de pesquisa, filtros, lotes, foco e rolagem permanece no contrato da rodada 2. Só Explorar reorganiza seu rodapé, fotografias e créditos. [Registro final](VERSOES/2.2.2/rodada-final/README.md).

## PR #32 — rodada 2

`GameController` e `TaxonomyIndex` conservam integralmente seus contratos e regras científicas. `Main.explorePresentation` guarda estado efêmero da view, sem persistência: chave de consulta/filtros, limites progressivos, foco e posições de rolagem. `CategoryView` restaura o estado quando a chave coincide e reinicia os limites quando ela muda; timers pertencentes à view aguardam sua recriação. Não há consulta, filtragem científica ou deduplicação nova no QML.

Galeria horizontal e página vertical verificam conjuntamente a visibilidade das fotografias; não há carga por controles fora dos dois viewports. Atributos integrais, avisos de revisão/identificação e metadados permanecem nos mapas existentes. `TaxonomyView` organiza sua apresentação em identificação, conteúdo educativo, classificação e fontes, mantendo origem/URL fotográfica de ambos os mapas existentes. A variante de Jogar permanece isolada por `browsingOnly`/`encyclopedic`. `Main.qml` ganha somente a propriedade de memória; não há mudança visual do shell ou das outras telas. [Contratos e evidências atuais](VERSOES/2.2.2/rodada-2/README.md).

## PR #32 — rodada 1

Busca e filtros científicos permanecem em `TaxonomyIndex`, sem alteração de ordenação ou deduplicação. A apresentação usa prefixos progressivos de 12 registros por seção em QML. `GameController` recebe a consulta atual a cada edição e separa suas notificações das de domínio/rank; o timer de 220 ms posterga a atualização dos resultados apresentados, sem guardar consultas antigas. Filtros/limpeza finalizam a espera e a consulta permanece em memória ao destruir/recriar a view.

Cartões calculam a interseção da moldura fotográfica com o `Flickable` real antes de fornecer a fonte ao `Image`. A imagem inteira é ajustada proporcionalmente, com foco somente no espaço de margem. Breadcrumbs compactos são uma projeção visual do caminho integral, com expansão e acesso aos mesmos IDs. Os limites de página, as colunas e a expansão são estado de apresentação; não há banco novo, alteração de esquema, modificação de dados ou escrita em sessões. [Registro desta revisão](VERSOES/2.2.2/rodada-1/README.md).

## Explorar — Subetapa 2.2.2

`TaxonomyIndex` usa as leituras `loadCurrentSpecies/loadCurrentTaxons` da porta `SpeciesCatalog`. O adaptador SQLite preserva suas leituras históricas para compatibilidade e oferece o snapshot instalado para a navegação; não remove linhas nem altera esquemas. O índice deduplica IDs, mantém grupos vazios e constrói a pesquisa científica. Consulta por termos em nomes/linhagens, normalização somente para comparação, relevância, filtros por domínio/rank, distinção de grupos/organismos e seleção de destaques pertencem a essa camada. Os destaques reutilizam o predicado da descoberta diária; regras de quiz permanecem no domínio.

`GameController` expõe consulta/filtros em memória e projeções notificáveis. Voltar da taxonomia recupera a consulta anterior sem persistir preferências ou modificar checkpoint/sessão. `CategoryView` aplica uma espera de digitação de 220 ms, apresenta seções de resultados e solicita `openTaxon(id)`. `TaxonomyView` preserva breadcrumbs científicos, mostra fonte/estado/ressalvas existentes e créditos contextuais expansíveis. O shell e o contexto de navegação existentes isolam Explorar de Jogar; `browsingOnly` e `ExploreCard.encyclopedic` limitam a modernização visual. [Contrato, evidências e limites](VERSOES/2.2.2/README.md).

## Descoberta e indicadores — Subetapa 2.2.1-B

`TaxonomyIndex.homeCatalog(localDate)` produz conjuntamente a descoberta e os três indicadores a partir de um snapshot de `SpeciesCatalog`. IDs únicos ordenados definem a rotação por dia civil. A elegibilidade reutiliza `approvedForQuiz` no nível espécie, exige foto de identificação e um resumo utilizável de `EducationalContent.profile` com revisão editorial/especializada e fonte. Imagens de apoio não identificam uma espécie e são excluídas dessa cobertura. Domínios só são contados quando ocorrem nos caminhos dos organismos; Archaea vazio não aumenta o indicador.

`GameController` expõe `dailyDiscovery` e `homeCatalogIndicators`, notificando alterações reais. Data local é injetável para testes; na produção vem de `QDate::currentDate`. Um timer de 30 segundos e a ativação da aplicação atualizam a projeção offline; o adaptador SQLite existente conserva o snapshot instalado até nova inicialização. Não há gravação de descoberta, novo banco ou mudança no estado de jogo. `HomeView` apenas apresenta a projeção e abre o nó da espécie usando `openExplore()` seguido de `openTaxon(id)`, preservando sessões. [Critérios, limites e reprodução](VERSOES/2.2.1-B/README.md).

O refinamento da PR #31 cria um diálogo contextual independente, ligado aos metadados da fotografia da descoberta, inclusive se a data mudar enquanto está aberto. Versão de licença ausente continua não comprovada, mesmo havendo um endereço de termos; nenhuma permissão é inferida. A tela geral de créditos permanece no rodapé. O nome “Identificação por espécie” apenas esclarece a métrica já existente; critérios e valores não mudam. [Registro do refinamento](VERSOES/2.2.1-B/refinamento-1/README.md).

## Início fotográfico — Subetapa 2.2.1-A

`HomeView.qml` mantém o estado transitório do carrossel, sem persistir preferências ou alterar partidas. `GameController.homeSlides` expõe quatro representantes explícitos de `TaxonomyIndex`: Animalia, Arthropoda, Plantae e Fungi, selecionados pelos identificadores estáveis para evitar homônimos históricos preservados no SQLite. A mesma porta `ExplorationMediaCatalog` valida fotos, licenças e evidências já utilizadas na exploração. Introduções passam por `EducationalContent`; só a primeira frase completa do texto utilizável é exibida, com referência preservada. Não há novas afirmações científicas ou aprovação automática.

Fotografias indisponíveis ou introduções sem revisão utilizável não geram slides; a Home continua oferecendo Explorar e Jogar mesmo com zero slides. A Home usa dois buffers nativos `Image`, com recorte proporcional e gradiente independente; Canvas pinta o degradê inferior da legenda e os recortes dos cantos, sem repintar as fotografias. O componente `FeatheredPhoto` dos cartões permanece intacto. Um Timer de oito segundos avança somente com a Home visível e a janela ativa, sem interação ou créditos; redução de movimento o desativa. Não há download em execução normal ou nova dependência. Créditos usam um diálogo rolável com dados locais; links externos só abrem por ação do usuário. [Revisão e limites](VERSOES/2.2.1-A/README.md).

## Navegação atual — Subetapa 2.1.3

Base de implementação: `4449f8468d0e70485b5fa3b90f3814595e6771f9`, após o PR #28. [Decisões, capturas e validação da revisão](VERSOES/2.1.3/README.md).

`Main.qml` continua responsável pelo shell: barra lateral, cabeçalho contextual, avisos globais e Loader rolável. A barra apresenta Início, Explorar, Jogar, Estatísticas, Conquistas e Perfis e ajustes. `currentSection` determina a seleção; Créditos permanece no contexto de Perfis e ajustes. O tema fornece cores, tipografia, margens e larguras compacta/normal.

| Entrada/estado | Tela reutilizada | Distinção funcional |
| --- | --- | --- |
| Explorar (`explore`) | `CategoryView`, `browsingOnly: true` | Domínios e atalhos; sem modo competitivo, dificuldade ou ação de jogo misto. Navegar não inicia sessão. |
| Grupo explorado (`taxonomy`, `taxonomyContext: explore`) | `TaxonomyView`, `browsingOnly: true` | Introdução, fontes, breadcrumbs e descendentes; sem seletor de quiz ou botão Começar. Voltar leva a Explorar. |
| Jogar (`categories`) | `CategoryView`, modo padrão | Livre/Competitivo, dificuldade, jogo misto e grupos, com contratos existentes. |
| Grupo para jogar (`taxonomy`, `taxonomyContext: play`) | `TaxonomyView`, modo padrão | Fontes, breadcrumbs, modos e níveis elegíveis. Voltar leva à seleção de Jogar. |
| Quiz, feedback, ficha e resultado | Views existentes, sem reformulação | Menu Jogar selecionado; alternar seções não chama encerramento nem cria partida. |

`openExplore()` abre os domínios. `openPlay()` prioriza resultado pendente, depois sessão em andamento e sua resposta/ficha, depois conclusão da sessão corrente, ou abre a seleção de modos. `lastGamePage` conserva o contexto de apresentação; o controlador permanece a autoridade sobre perguntas, respostas, dicas, pontos e checkpoints. Ao sair voluntariamente de um resultado concluído para a seleção, `sessionStarted` deixa de exigir retorno ao resultado anterior.

`openTaxon()` e os breadcrumbs reutilizam o índice científico do controlador. Alterar o grupo de navegação não muda o grupo da sessão. Avisos de sessão recuperável, erro de checkpoint e resultado pendente continuam globais. O cabeçalho oferece Retomar partida quando o usuário consulta outra seção durante um jogo.

Ao receber foco por teclado, controles da página são trazidos para a área visível do Flickable. A rolagem é limitada ao conteúdo da página; não desloca menu, cabeçalho ou diálogos.

Não há nova enciclopédia, busca ou fonte de dados. A mudança não modifica QuizView, StatisticsView, AchievementsView, persistência, catálogo ou regras. O rótulo do botão inicial passa a “Começar a jogar” para refletir seu destino; o restante da Home continua igual. A área principal permanece rolável em 1000 × 700; textos contextuais longos quebram linha. As views ainda não migradas conservam seus estilos próprios.

## Registro anterior — baseline da Subetapa 2.1.1

Base: commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e`. A interface de produção é `resources/qml/Main.qml`, embarcada por `resources.qrc` e carregada em `src/app/main.cpp`. `GameController` fornece o estado via contexto `gameController`; `DiscordRichPresence` fornece o estado opcional da presença. As telas são componentes em `resources/qml/views/`; os controles reutilizáveis ficam em `resources/qml/components/`.

`Main.qml` cria uma `ApplicationWindow` de 1240 × 820, mínima de 1000 × 700. Mantém barra lateral, cabeçalho, avisos de recuperação/salvamento e área rolável. A propriedade `page` seleciona um `Component` pelo `Loader`; `navigate`, `openPlay` e os métodos de início/continuação de partida conduzem as transições. `sessionStarted`, `playing` e os estados do controlador determinam para onde “Jogar” retorna.

| Tela (`page`) | Componente | Responsabilidade e entrada principal |
| --- | --- | --- |
| `home` | `HomeView` | Abertura, convite para jogar e resumo do catálogo; entrada pelo menu Início. |
| `categories` | `CategoryView` | Modo Livre/Competitivo, dificuldade das alternativas, acesso rápido e três domínios; entrada por Jogar. |
| `taxonomy` | `TaxonomyView` | Breadcrumbs, fonte/introdução, grupos filhos e quiz elegível no nível selecionado; entrada por cartão ou atalho. |
| `quiz` | `QuizView` | Fotografia, quatro alternativas, pontuação possível e painel de dicas; início ou continuação da partida. |
| `feedback` | `FeedbackView` | Resultado da resposta, aprendizado após responder e avanço; chega após confirmação. |
| `info` | `InfoView` | Ficha da espécie, classificação, tópicos, fontes e estados de revisão; chega pelo feedback. |
| `results` | `ResultsView` | Resumo e próximos caminhos; chega ao concluir a partida. |
| `statistics` | `StatisticsView` | Métricas da sessão, histórico por perfil, grupos e rankings Livre/Competitivo; menu lateral. |
| `achievements` | `AchievementsView` | Conquistas; menu lateral. |
| `settings` | `SettingsView` | Perfis, transições e presença no Discord; menu lateral. |
| `credits` | `CreditsView` | Créditos, fontes e transparência; acessada por Perfis e ajustes. |

Componentes compartilhados: `UiButton`, `NavItem`, `UiIcon`, `TextAction`, `TaxonComboBox` e `StatCard` compõem controles recorrentes; `PlayModeSelector` serve à exploração geral e aos grupos; `ExploreCard`, `FeatheredPhoto`, `SpeciesPortrait` e artes associadas apresentam organismos; `HintPanel` e `EndGameDialog` cobrem decisões durante a partida. A lógica de pontos e disponibilidade das dicas permanece no domínio/controlador, não na aparência desses componentes.

Fluxo principal: Início → Explorar → grupo opcional → Quiz → Feedback → Ficha opcional → próxima questão/Resultado. Menu lateral abre Estatísticas, Conquistas e Perfis; Créditos ficam dentro de Perfis. Uma sessão recuperável aparece em aviso global antes do conteúdo. Voltar de um quiz aciona confirmação de encerramento; algumas transições dependem de checkpoint ou resultado pendente.

## Observações de UX/UI para avaliação posterior

- `credits` não possui entrada em `pageTitles` de `Main.qml`; o cabeçalho usa o título genérico, embora a tela tenha conteúdo específico. O caminho por Ajustes também torna créditos menos visíveis.
- Cores, tamanhos, raios e espaçamentos são declarados repetidamente nas views e nos componentes. Mudanças globais exigem localizar valores em vários arquivos.
- A largura mínima de 1000 px e o `implicitHeight` mínimo de 950 px em Estatísticas exigem rolagem em janelas compactas; as capturas de 1240 × 820 não verificam uso com escala de sistema, leitores de tela ou outras plataformas.
- A seleção da barra lateral destaca “Jogar” em exploração, quiz, feedback, ficha e resultado; ela indica uma área ampla, enquanto o cabeçalho usa o nome de cada estado. Convém avaliar se essa hierarquia comunica claramente o contexto.
- `CategoryView` posiciona filtros e acesso rápido acima dos cartões. A área rolável pode ocultar os cartões inferiores; a versão compacta precisa continuar sendo conferida quando houver mudança de densidade.

Esses itens são observações da implementação e de capturas reais, **não correções nesta subetapa**.
