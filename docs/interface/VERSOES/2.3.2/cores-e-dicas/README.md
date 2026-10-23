# 2.3.2 — Cores e dicas em andamento

Revisão pontual da [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36), sobre `3429e09b82f4a7a1f7a98ba8282701d30c02d007`. Preserva a organização aprovada e as evidências da [entrega inicial](../README.md) e dos [painéis no topo](../refinamentos-topo/README.md).

Escopo: cores taxonômicas estáveis por ID, ouro/prata/bronze no ranking e projeção visual das dicas ainda não confirmadas. Somente `StatisticsView.qml` muda no código da aplicação. Backend, banco, pontuação, checkpoints, catálogo, fotografias, licenças e planejamento multiplayer permanecem intactos.

Aplicativo 1.11.0 ainda não lançado; catálogo 2.4.0 preservado. Implementação e testes no commit [`79ccb7067dccfcb02677d42a9a7554a04108b3f4`](https://github.com/kaduolliver/Projeto-Glossata/commit/79ccb7067dccfcb02677d42a9a7554a04108b3f4). Esta revisão registra 72 capturas reais e seus resultados, sem substituir evidências anteriores. A Home e seu registro oficial permanecem intactos, pois o shell e os componentes compartilhados não mudaram.

## Decisões de apresentação

As barras principais e seus percentuais usam o ID científico existente, nunca o índice da lista ou o nome traduzido. As cores representam identidade visual, sem classificação de desempenho. Nomes, contagens e percentuais permanecem visíveis. Os subgrupos recebem um clareamento de aproximadamente 9,4% da cor do grupo; proporções continuam sendo acertos divididos por respostas, sem arredondamento da barra.

| Grupo existente | ID | Cor |
| --- | --- | --- |
| Plantae | `taxon:56956496cf600779dc1f` | Verde `#85DCAA` |
| Bacteria | `domain:Bacteria` | Azul `#8ABCF0` |
| SAR | `clade:SAR` | Turquesa `#74D3CB` |
| Fungi | `taxon:31fdb10b7300d45a9dcf` | Dourado `#E2C477` |
| Animalia | `taxon:cac47417164883d60794` | Laranja `#E4A77D` |
| Amoebozoa | `clade:Amoebozoa` | Violeta `#BDA5DE` |
| Discoba | `clade:Discoba` | Azul discreto `#A6C4E8` |

Para um ID não mapeado, um hash de seus caracteres UTF-16 (`hash * 31 + caractere`, inteiro sem sinal de 32 bits) seleciona uma das seis cores-base por módulo. Esse fallback permanece estável entre recriações/reordenações; IDs futuros podem compartilhar cores. Ele não inventa um grupo ou relação científica. Verde/dourado reutilizam `Theme`; os demais tons são locais a Estatísticas, sem alterar o tema compartilhado.

As três primeiras posições do ranking usam dourado `#E2C477`, prata `#C6D0DA` e bronze `#D6A27B`, tanto no número quanto na borda. Demais bordas continuam `Theme.border`; fundo verde discreto do primeiro colocado, nomes, recordes, marcas de legado/exclusão e modalidades permanecem. Ao receber foco, a borda usa prioritariamente `Theme.focus` e espessura de dois pixels, inclusive no pódio. Não há medalhas, animações novas ou mudanças nas regras.

## Contrato das dicas

O diagnóstico confirmou que `GameController.sessionMetrics` expõe somente respostas confirmadas, com `scoreChanged`. `hints` representa a pergunta **exibida**, com IDs, categorias, disponibilidade e estado revelado; `extraHintChanged` informa revelações. Durante feedback, a pergunta continua exibida, mas `answerPending` já é verdadeiro e suas dicas já estão consolidadas. `nextQuestion()` limpa as revelações exibidas. `resumeSession()` restaura ambas as fontes a partir do checkpoint existente; `stopGame()` sinaliza `gameFinishedChanged`, sem confirmar a pergunta não respondida.

`StatisticsView.visualHints` combina essas fontes apenas na apresentação: totais confirmados mais dicas distintas por ID, disponíveis e reveladas, da pergunta ativa sem resposta. A primeira dica acrescenta uma questão com dicas; as seguintes acrescentam somente dicas/categorias. Os valores são derivados, sem acumulador em memória ou escrita no banco. A confirmação deixa de somar a contribuição atual e passa a usar sua consolidação; repetir uma dica ou recriar a página não incrementa nada. Categorias reais são preservadas, inclusive quando várias dicas têm a mesma categoria; elas não representam níveis de assistência.

Ao encerrar uma partida com dicas de uma pergunta não respondida, a apresentação passa a mostrar somente as confirmadas. Isso corresponde ao histórico oficial existente, sem apagar nem reescrever evidências. Recuperar uma partida em andamento inclui novamente suas revelações atuais. A nota abaixo dos indicadores informa que podem incluir a pergunta em andamento e que o histórico só registra respostas confirmadas. Pontos, respostas/acertos/erros, demais métricas e histórico continuam usando seus contratos anteriores.

## Verificação da implementação

Build Release/Ninja/GCC 16.2.0, Qt 6.11.2 em Windows 11; lint de todos os QML e smoke aprovados. **26/26 suítes CTest passaram em 442,80 segundos**, com 29 casos QtTest de interface incluindo preparação/limpeza. Os cinco testes de Estatísticas passaram em 100%, 125% e 150%, nas três resoluções exigidas: sete casos por execução incluindo preparação/limpeza.

`statisticsLiveHintsAndRecovery` verifica zero/uma/duas/três dicas, repetição de uma dica, categorias reais (inclusive repetidas), uma contribuição por questão, confirmação sem duplicação, nova pergunta, questões consecutivas, Quiz↔Estatísticas, recuperação com revelações abertas, confirmação após recuperação, encerramento e dicas indisponíveis. Compara os valores oficiais a cada confirmação e comprova que revelar dicas não emite `scoreChanged` nem altera pontuação/métricas consolidadas. A recuperação usa um novo controlador e o SQLite temporário existente, sem alterar checkpoints.

`statisticsStableColorsAndPodium` verifica IDs/cores/proporções reais, contraste mínimo de 4,5:1 dos tons apresentados contra suas superfícies, variações discretas nos subgrupos, reordenação com os mesmos IDs e fallback estável. Confere as cinco bordas/números, foco por Tab, cor/espessura prioritárias e retorno à borda normal. A reordenação e o ID não mapeado são fixtures de apresentação isoladas, restauradas antes das capturas. O ranking publicado contém cinco perfis/resultados temporários inseridos pelo repositório de teste, sem dados de jogadores reais.

Os três testes anteriores de Estatísticas foram preservados integralmente e repetidos: painéis/seletores/rolagem, contextos/métricas confirmadas e ranking responsivo. A suíte completa também verifica Quiz, ficha científica, Jogar, Explorar, Início, identidades, sessões e governança. Não houve alterações nessas telas ou nos componentes compartilhados.

[CTest completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt), [resumo verificável](verificacoes/resultados.json). Casos QtTest incluindo preparação/limpeza: domínio 62, interface 29, identidade 32 e recuperação 21. Execuções direcionadas: [100%](verificacoes/statistics-colors-targeted.txt), [125%](verificacoes/statistics-colors-dpi-125.txt), [150%](verificacoes/statistics-colors-dpi-150.txt).

Passaram status/metadados/histórico, licenciamento do código, catálogo, mídias e auditoria do repositório, além dos testes do inventário de arquivos. Os 1.053 eventos de governança foram preservados. [Comandos e retornos](verificacoes/technical-audits.json), [comparação histórica](verificacoes/governance-history.json). O gate de distribuição continua bloqueado pelas pendências anteriores de permissões/revisões, com retorno 2 esperado, sem erros técnicos/de dependências: [registro](verificacoes/distribution-summary.json). Testes não certificam curadoria ou conformidade jurídica.

Comandos principais, com Qt no PATH:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.3.2-cores-dicas
cmake --build build -j3
ctest --test-dir build --output-on-failure -j3
```

Em `build/tests`, executar `../2.3.2-cores-dicas/glossata_ui_tests.exe` com os cinco métodos `statisticsLiveHintsAndRecovery`, `statisticsStableColorsAndPodium`, `statisticsTopPanelsAndSharedScrolling`, `statisticsContextsAndConfirmedMetrics` e `statisticsResponsiveRankingAndCaptures`. As execuções usam `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`.

## Capturas reais

**72 PNGs da janela Qt real**, gerados por `QQuickWindow::grabWindow()` com QML/controlador de produção, SQLite temporário e Discord desativado. Os dois testes novos produzem 63 quadros; nove enquadramentos dos grupos inferiores vêm do teste responsivo anterior, executado com esta mesma implementação. O [manifesto](capturas.json) registra commit, hashes de fontes/binários/PNGs, timestamps UTC, dimensões físicas/lógicas, escala, rolagem, métricas oficiais e, quando aplicável, dicas atuais/cores/ranking. Não há edição de pixels, dados de jogador real ou alteração científica nas capturas.

| Estado — 100% | 1000×700 | 1240×820 | 1920×1080 |
| --- | --- | --- | --- |
| Cores taxonômicas no topo | [PNG](capturas/taxonomy-colors-1000x700.png) | [PNG](capturas/taxonomy-colors-1240x820.png) | [PNG](capturas/taxonomy-colors-1920x1080.png) |
| Grupos inferiores | [PNG](capturas/taxonomy-lower-groups-1000x700.png) | [PNG](capturas/taxonomy-lower-groups-1240x820.png) | [PNG](capturas/taxonomy-lower-groups-1920x1080.png) |
| Ouro, prata e bronze | [PNG](capturas/ranking-podium-1000x700.png) | [PNG](capturas/ranking-podium-1240x820.png) | [PNG](capturas/ranking-podium-1920x1080.png) |
| Foco por teclado | [PNG](capturas/ranking-keyboard-focus-1000x700.png) | [PNG](capturas/ranking-keyboard-focus-1240x820.png) | [PNG](capturas/ranking-keyboard-focus-1920x1080.png) |
| Três dicas, pergunta em andamento | [PNG](capturas/live-three-hints-1000x700.png) | [PNG](capturas/live-three-hints-1240x820.png) | [PNG](capturas/live-three-hints-1920x1080.png) |
| Resposta confirmada, sem duplicação | [PNG](capturas/confirmed-hints-1000x700.png) | [PNG](capturas/confirmed-hints-1240x820.png) | [PNG](capturas/confirmed-hints-1920x1080.png) |
| Recuperação com dicas abertas | [PNG](capturas/recovered-live-hints-1000x700.png) | [PNG](capturas/recovered-live-hints-1240x820.png) | [PNG](capturas/recovered-live-hints-1920x1080.png) |
| Encerramento, somente confirmadas | [PNG](capturas/stopped-confirmed-only-1000x700.png) | [PNG](capturas/stopped-confirmed-only-1240x820.png) | [PNG](capturas/stopped-confirmed-only-1920x1080.png) |

Todos os estados em 125%/150% têm sufixos `-125.png`/`-150.png`, registrados individualmente no manifesto. Exemplos: [cores em 125%](capturas/taxonomy-colors-1240x820-125.png), [dicas em 150%](capturas/live-three-hints-1000x700-150.png), [foco no ranking em 150%](capturas/ranking-keyboard-focus-1920x1080-150.png). Os quadros de dicas e grupos inferiores têm rolagem intencional; o cabeçalho continua acessível no topo da página.

Limitações: renderização software/offscreen e fatores Qt, sem ensaio de GPU, escala física do Windows ou leitor de tela físico. Os testes verificam foco/teclado e exposição dos controles, sem certificar uma combinação específica de tecnologia assistiva. O fallback de cores pode repetir tons para IDs futuros; nomes, taxas e contagens continuam distinguindo os grupos.

## Auditoria de arquivos — somente simulação

Após os testes, `python scripts/cleanup_build.py` e a execução complementar com `--include-old-builds` foram usados em **dry-run**. A idade/retenção e as proteções resultaram em **zero bytes automaticamente elegíveis**. Nenhuma política foi alterada; nenhum arquivo foi excluído ou movido.

O [inventário complementar](verificacoes/auditoria-arquivos.json) registra **3.822.437.609 bytes (3,56 GiB)** para revisão manual, com caminhos, tamanhos, motivos e riscos de builds antigos, caches e capturas locais. Os 16 grupos de possíveis duplicações usam somente nome/tamanho, sem afirmar igualdade de conteúdo ou espaço recuperável confirmado. Exemplos retidos: `build/2.3.1-descricao`, `build/2.3.1-restaurado`, `build/2.2.3-dificuldade` e pastas anteriores de `build/tests/previews`.

Todos os builds `build/2.3.2-*`, as capturas locais atuais/de referência e os arquivos de rastreabilidade permanecem protegidos, assim como dados/perfis/SQLite, backups, `resources`, `package`, licenças, governança e evidências versionadas. A PR #36 continua aberta, sem merge, release ou avanço para 2.3.3.
