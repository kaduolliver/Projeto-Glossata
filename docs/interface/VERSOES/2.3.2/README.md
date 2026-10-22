# 2.3.2 — Modernização da tela Estatísticas

Base: `c6db713c9308e2e08cca6b7fea8f6ed214eeb116`, integração da PR #35. Branch: `feat/issue-22-statistics-2.3.2`. Escopo restrito à apresentação de Estatísticas e sua validação. Evidências anteriores preservadas.

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

Verificações e capturas da implementação serão registradas após a execução. Os testes usam `GameController`, QML de produção e SQLite temporário, Discord desativado. Fixtures de apresentação para legado/nome longo/contagens zero são identificadas separadamente e não alteram catálogo, bancos reais ou evidências de governança.
