# 2.3.2 — Cores e dicas em andamento

Revisão pontual da [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36), sobre `3429e09b82f4a7a1f7a98ba8282701d30c02d007`. Preserva a organização aprovada e as evidências da [entrega inicial](../README.md) e dos [painéis no topo](../refinamentos-topo/README.md).

Escopo: cores taxonômicas estáveis por ID, ouro/prata/bronze no ranking e projeção visual das dicas ainda não confirmadas. Somente `StatisticsView.qml` muda no código da aplicação. Backend, banco, pontuação, checkpoints, catálogo, fotografias, licenças e planejamento multiplayer permanecem intactos.

Aplicativo 1.11.0 ainda não lançado; catálogo 2.4.0 preservado. Capturas e resultados desta revisão serão registrados após recompilação e testes, sem substituir evidências anteriores.

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
