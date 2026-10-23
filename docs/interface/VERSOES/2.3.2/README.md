# 2.3.2 — Modernização da tela Estatísticas

Esta página preserva a entrega inicial aprovada da PR #36, suas 99 capturas e resultados. A apresentação atual está na [revisão de cores e dicas em andamento](cores-e-dicas/README.md), preservando os [painéis no topo e rolagem compartilhada](refinamentos-topo/README.md). Nenhuma evidência anterior foi substituída.

Base: `c6db713c9308e2e08cca6b7fea8f6ed214eeb116`, integração da PR #35. Branch: `feat/issue-22-statistics-2.3.2`. Escopo restrito à apresentação de Estatísticas e sua validação. Evidências anteriores preservadas.

[PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36), relacionada à Issue #22. Código e testes: [`c435304a7243c1bc6396fa4e4d97297fe83426a9`](https://github.com/kaduolliver/Projeto-Glossata/commit/c435304a7243c1bc6396fa4e4d97297fe83426a9). Sem merge, release ou avanço para 2.3.3.

## Diagnóstico anterior à alteração

Foram examinados `StatisticsView.qml`, `StatCard.qml`, `Theme.qml`, o contrato e as implementações de `GameController`, `SessionMetrics`, `TaxonomicPerformance`, `SqliteGameHistoryRepository` e a documentação da interface.

| Contexto | Contrato existente | Significado e limites |
| --- | --- | --- |
| Partida atual | `answeredCount`, `correctAnswers`, `sessionMetrics` | Somente respostas confirmadas. Erros = respondidas − acertos. Métricas continuam disponíveis após encerrar e são recuperadas pelo checkpoint. Não dependem do modo de ranking selecionado. |
| Desempenho da partida | `sessionMetrics.average`, `efficiency`, `unassistedCorrect` | Média arredondada a duas casas, eficiência e precisão a uma casa, em formato decimal português fornecido pelo C++. Eficiência compara pontos obtidos ao máximo das respostas confirmadas. |
| Dicas da partida | `hintedQuestions`, `revealedCategories`, `taxonomic`, `etymological`, `morphological` | Questões assistidas diferem da quantidade de dicas. As contagens pertencem às respostas confirmadas: revelar dica na pergunta ainda aberta não incrementa essas métricas. Categorias científicas não equivalem a níveis Leve/Moderada/Forte. |
| Histórico do perfil | `playerMetrics` | ID permanente do perfil selecionado; somente respostas detalhadas disponíveis de partidas concluídas e da chave de ranking selecionada. Pode ser parcial: resultados antigos sem respostas detalhadas não permitem reconstruir métricas. Não agrega automaticamente Livre e Competitivo. |
| Ranking | `rankingMode`, `ranking`, `personalRecord`, `identityNotice` | Até cinco melhores jogadores retornados pelo backend, na ordem original. Competitivo usa a configuração/regras/conteúdo atuais; Livre reúne treinamentos de duração e grupos variáveis. Preservar recorde, última pontuação/data, nome atual, nome registrado e marcadores de legado/exclusão. |
| Taxonomia da partida | `taxonomicStats` (alias de `categoryStats`) | Projeção das respostas confirmadas, por grupo de apresentação e filo/divisão resolvido. Só contém grupos respondidos normalmente; preservar IDs, ordem, títulos e subgrupos. Subgrupo traz precisão inteira; grupo traz contagens. Uma barra do grupo pode representar a razão real acertos/respostas, sem criar métrica persistente. Ausência de classificação permanece explícita. |

A tela anterior usa três indicadores principais (sem erros), texto corrido para dicas/histórico e painéis taxonômico/ranking lado a lado em todas as larguras. Vários estilos locais antecedem o Design System. A modernização manterá as mesmas regiões e informações, com tokens existentes, adaptação de colunas e distinção explícita entre os contextos acima.

Não é necessária mudança no contrato C++, esquema, dados, regras, fontes científicas ou licenças. `StatCard` e `Theme` serão reutilizados sem alterar seus padrões globais. O conteúdo científico e as outras telas permanecem fora do escopo.

## Implementação

- Cabeçalho e partida atual: jogador da sessão, andamento/encerramento e modalidade; quatro `StatCard` para respostas, acertos, erros e precisão decimal do backend. A pergunta aberta não é contada.
- Desempenho: média, eficiência e acertos sem dicas com explicações curtas. Dicas: duas contagens distintas e três categorias, sem interpretar categorias como assistência ou criar percentuais.
- Histórico: nome do perfil e contexto Livre/Competitivo atual explícitos, com precisão/média/eficiência, contagens e categorias já disponíveis. Ausência e possível cobertura parcial são informadas mesmo quando existe recorde.
- Taxonomia: ordem/IDs/hierarquia originais; grupo e subgrupo têm contagens e barras reais. O grupo usa divisão com proteção para zero e percentual inteiro; o subgrupo conserva o percentual inteiro recebido. Nenhuma estatística permanente é inferida.
- Ranking: mesmos botões, limite de cinco posições, ordenação e elegibilidade. Três primeiras posições destacadas; recorde pessoal, última partida, nome registrado acessível e marcadores de legado/exclusão preservados. Nomes quebram linha e todas as posições recebem foco por Tab, com enquadramento automático pelo shell.
- Layout: indicadores principais em duas/quatro colunas; métricas em duas/três conforme largura útil. Taxonomia/ranking empilham abaixo de 900 pixels lógicos. Painéis seguem o conteúdo, sem alturas fixas, listas internas ou novas animações. `StatCard`, `Theme` e outros componentes compartilhados conservam seus contratos.

O aplicativo passa a **1.11.0**, MINOR pela ampliação compatível da apresentação/navegação. Catálogo **2.4.0** intacto. A captura oficial da Home é renovada com o executável desta versão, sem alteração de composição ou funcionalidades da Home. Não há tag, release ou autorização de distribuição.

## Comparação anterior

As [quatro capturas anteriores](anterior/capturas.json) são da interface da `main` acima, compilada antes das mudanças, por `navigateAndConfirmAnswers`: [visão geral](anterior/statistics.png), [compacta](anterior/compact-full-statistics.png), [grupos inferiores](anterior/compact-statistics-lower-groups.png), [Competitivo](anterior/competitive-statistics.png). Dados e nomes são de testes, em SQLite temporário. O [resultado](anterior/teste.txt) e o manifesto registram método, binário, dimensões, horários de gravação e hashes. Não representam dados pessoais reais.

Antes: três indicadores principais, dicas/histórico concentrados em parágrafos, taxonomia/ranking sempre lado a lado e lista interna no ranking. Depois: erros explícitos, semântica de partida/histórico/modalidade junto às métricas, dicas compactas, todas as contagens taxonômicas e rolagem única com nomes completos. As informações existentes foram mantidas; não foram criadas séries temporais, conquistas, novos critérios de desempenho ou comparações entre modos.

## Validação e evidências

Build Release/Ninja/GCC 16.2.0 e Qt 6.11.2 em Windows 11. **26/26 suítes CTest aprovadas**, em 392,93 segundos com `-j3`; casos QtTest incluindo preparação/limpeza: domínio 62, interface 26, identidade 32 e recuperação 21. [Resultado completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt), [resumo verificável](verificacoes/resultados.json).

Os dois testes novos (`statisticsContextsAndConfirmedMetrics`, `statisticsResponsiveRankingAndCaptures`) passaram também nas escalas Qt 125% e 150%: quatro casos por execução, incluindo preparação/limpeza, nas três resoluções. [100%](verificacoes/statistics-targeted.txt), [125%](verificacoes/statistics-dpi-125.txt), [150%](verificacoes/statistics-dpi-150.txt). Cobrem:

- Zero respostas, acertos/erros, precisão 66,7%, valores decimais do backend e ausência de incremento antes da confirmação ou em confirmação repetida.
- Questões com dicas versus dicas reveladas, contagens pelas categorias efetivas e acertos sem dicas; a pergunta aberta não altera essas métricas.
- Histórico vazio/parcial/preenchido, troca entre perfis por ID e entre Livre/Competitivo sem modificar sessão, pontuação ou dificuldade. Um recorde sem detalhamento continua exibível sem presumir respostas históricas.
- Sessão suspensa/recuperada com métricas, grupos e alternativas preservados; encerramento grava o histórico correspondente.
- Ranking vazio e cinco posições, recorde pessoal, perfis excluídos, legado/nome longo e Competitivo completo de 20 respostas. Tab/Shift+Tab/Espaço percorrem controles e posições com foco enquadrado. A suíte anterior também testa mouse, início de partidas e recuperação.
- Todos os grupos/subgrupos recebidos, árvore extensa nos testes existentes, caso de zero respostas por fixture de apresentação, distribuição/limites horizontais dos painéis e ausência de avisos QML. Regressões de Início, Explorar, Jogar, Quiz, ficha, Estatísticas e Conquistas aprovadas.

Lint de todos os QML, smoke do executável, status do projeto, metadados/histórico, licenciamento do código, auditoria técnica de catálogo/mídias e integridade dos **1.053 eventos** de governança aprovados. [Auditorias e comandos](verificacoes/technical-audits.json), [governança](verificacoes/governance-history.json). O gate de distribuição conserva suas pendências: retorno esperado 2, sem erros técnicos ou de dependências no relatório. [Resumo do bloqueio](verificacoes/distribution-summary.json). Não houve nova aprovação ou tentativa de liberar distribuição.

### Capturas reais

São **99 PNGs**: onze estados × três resoluções × três escalas Qt. [Manifesto](capturas.json) registra commit, binários, hashes de fontes/PNGs, horários UTC, dimensões lógicas/físicas, escala, rolagem e valores reais. Os testes usam `GameController`, QML de produção e SQLite temporário, Discord desativado. O motor determina a ordem dos organismos; respostas certas/erradas e uso de dicas são controlados pelo teste. As métricas de cada execução estão registradas, podendo variar com as dicas disponíveis. Nenhum banco pessoal foi acessado.

| Estado (100%) | 1000×700 | 1240×820 | 1920×1080 |
| --- | --- | --- | --- |
| Sem partidas/respostas | [PNG](capturas/empty-1000x700.png) | [PNG](capturas/empty-1240x820.png) | [PNG](capturas/empty-1920x1080.png) |
| Partida em andamento | [PNG](capturas/active-game-1000x700.png) | [PNG](capturas/active-game-1240x820.png) | [PNG](capturas/active-game-1920x1080.png) |
| Partida encerrada | [PNG](capturas/current-game-1000x700.png) | [PNG](capturas/current-game-1240x820.png) | [PNG](capturas/current-game-1920x1080.png) |
| Desempenho e dicas | [PNG](capturas/performance-and-hints-1000x700.png) | [PNG](capturas/performance-and-hints-1240x820.png) | [PNG](capturas/performance-and-hints-1920x1080.png) |
| Histórico do perfil | [PNG](capturas/profile-history-1000x700.png) | [PNG](capturas/profile-history-1240x820.png) | [PNG](capturas/profile-history-1920x1080.png) |
| Grupos taxonômicos | [PNG](capturas/taxonomy-1000x700.png) | [PNG](capturas/taxonomy-1240x820.png) | [PNG](capturas/taxonomy-1920x1080.png) |
| Grupos inferiores | [PNG](capturas/taxonomy-lower-groups-1000x700.png) | [PNG](capturas/taxonomy-lower-groups-1240x820.png) | [PNG](capturas/taxonomy-lower-groups-1920x1080.png) |
| Competitivo vazio | [PNG](capturas/competitive-empty-1000x700.png) | [PNG](capturas/competitive-empty-1240x820.png) | [PNG](capturas/competitive-empty-1920x1080.png) |
| Ranking Livre com foco | [PNG](capturas/ranking-free-1000x700.png) | [PNG](capturas/ranking-free-1240x820.png) | [PNG](capturas/ranking-free-1920x1080.png) |
| Fixture de legado/nome longo | [PNG](capturas/ranking-markers-long-name-1000x700.png) | [PNG](capturas/ranking-markers-long-name-1240x820.png) | [PNG](capturas/ranking-markers-long-name-1920x1080.png) |
| Ranking Competitivo preenchido | [PNG](capturas/ranking-competitive-1000x700.png) | [PNG](capturas/ranking-competitive-1240x820.png) | [PNG](capturas/ranking-competitive-1920x1080.png) |

As mesmas cenas em 125%/150% usam sufixos `-125.png`/`-150.png`, listados individualmente no manifesto. Exemplos: [compacta em 125%](capturas/current-game-1000x700-125.png), [dicas em 150%](capturas/performance-and-hints-1000x700-150.png), [ampla em 150%](capturas/current-game-1920x1080-150.png). As cenas inferiores são enquadramentos de rolagem/foco; elementos acima podem ficar naturalmente fora do recorte da janela.

Fixtures de apresentação para legado/nome longo/contagens zero são identificadas separadamente e não alteram catálogo, bancos reais ou evidências de governança. O nome longo usa 32 caracteres; os IDs e a estrutura taxonômica da fixture de zero reutilizam a projeção real. A interface de produção não fabrica esses estados.

Limitações: renderização software/offscreen e fatores Qt; não foram testados GPU, escala física do Windows ou leitor de tela físico. As capturas e verificações não certificam aprovação científica ou jurídica. O conjunto conserva a identidade anterior e não implementa Conquistas 2.3.3.

### Auditoria não destrutiva

Executados `python scripts/cleanup_build.py` e, complementarmente, `--include-old-builds`, ambos **dry-run**, depois de builds/testes. Os limites de idade/retenção e a proteção dos artefatos deixam **zero bytes elegíveis automaticamente**. Também passaram os testes próprios do auditor. Nenhuma política foi alterada.

O [inventário complementar](verificacoes/auditoria-arquivos.json) registra **3.823.192.894 bytes (3,56 GiB)** de builds anteriores, caches e capturas locais para revisão manual, com caminhos, tamanhos, motivos e riscos. Há 16 grupos de possíveis duplicações apenas por nome/tamanho; não foi afirmada igualdade de conteúdo nem somado um ganho de espaço fictício.

| Exemplos de caminhos | Bytes | Risco/decisão |
| --- | ---: | --- |
| `build/2.3.1-descricao` | 259.158.491 | Build anterior; pode ser necessário reproduzir evidências. Preservado. |
| `build/2.3.1-restaurado` | 259.152.701 | Build anterior de interface aprovada; preservar rastreabilidade. |
| `build/2.2.3-dificuldade` | 259.051.239 | Comparar com manifestos e requisitos de reprodução antes de decisão humana. |
| `build/2.3.2-estatisticas` | 259.231.479 | Build ativo, protegido e excluído do total manual. |
| `build/2.3.2-before` | 259.158.491 | Referência de comparação, protegida e excluída do total manual. |

**Zero exclusões ou movimentações.** Builds em uso, manifestos, documentação, evidências/capturas versionadas, catálogo, mídias, licenças, governança, backups, `package/` e dados pessoais permanecem preservados. O inventário não constitui autorização para apagar arquivos; qualquer limpeza segue o procedimento independente do `AGENTS.md`.
