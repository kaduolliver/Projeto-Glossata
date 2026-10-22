# PR #35 — Interface anterior restaurada e quatro ajustes pontuais

**Correção posterior da ficha:** [Morfologia restaurada à direita e Descrição educativa independente](../descricao-independente/README.md). Este registro e suas capturas da ficha correspondem à implementação anterior à correção; não representam sua apresentação atual. As evidências de Quiz, dicas, contadores e estabilidade do retrato permanecem válidas.


Esta é a entrega atual da [PR #35](https://github.com/kaduolliver/Projeto-Glossata/pull/35), relacionada à [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). O mantenedor rejeitou a reformulação visual inicial; as [evidências históricas](../proposta-rejeitada.md) permanecem identificadas como proposta rejeitada, sem representar aprovação ou versão final.

A referência é a interface anterior à PR #35, no commit `8293408a650ac2f9738d42c4c9c462115919bdfc`, após a PR #33. Não houve reset da `main`, rebase ou reescrita de histórico. A PR #34 já integrada em `25a882bd87d7d8cfd2fff458d296fa0334f0ee3b` é preservada; nenhuma alteração sua é desfeita ou reimplementada nesta branch.

Implementação e testes: [`4ef3ed0`](https://github.com/kaduolliver/Projeto-Glossata/commit/4ef3ed0). Aplicativo 1.10.0 em desenvolvimento; catálogo 2.4.0 inalterado. A implementação atual aguarda revisão visual do mantenedor; testes não constituem aprovação editorial ou científica.

## Aparência e funcionamento

`QuizView` recupera fotografia à esquerda (37% da largura útil), alternativas à direita, cabeçalho original, progresso Livre no rodapé, número/limite Competitivo, painel do jogador, controles Dica/Confirmar, botões, cores e espaçamentos anteriores. `HintPanel` volta a 540 pixels, ao título e aos cartões originais. `SpeciesPortrait` recupera integralmente a geometria, cores e `Image.PreserveAspectFit` anteriores; apenas nomes de objetos de teste são acrescentados. Não há nova variante visual, recorte, arquivo fotográfico ou alteração de créditos.

Os quatro ajustes ficam na apresentação:

- Cada dica revelada mostra título com a assistência efetiva (`Leve`, `Moderada`, `Forte`) e sua categoria fornecidas pelo backend. Preserva texto integral e ordem do modelo; o índice não determina assistência. Verde/dourado e a cor de assistência forte existente no painel distinguem os títulos sem nova paleta.
- O painel do jogador conserva nome e pontuação e acrescenta acertos/erros. Acertos vêm de `correctAnswers`; erros são `answeredCount - correctAnswers`. Seleção e revelação de dicas não contam a pergunta atual. Ambos os modos e a recuperação usam os mesmos dados existentes, sem campos novos no banco.
- `InfoView` reserva `speciesDescriptionRegion` abaixo da fotografia e dos créditos. A associação inicial com Morfologia foi corrigida: atualmente a região é independente e mostra apenas "Descrição educativa detalhada ainda não disponível." Morfologia permanece na coluna direita com os demais tópicos, fonte e revisão. [Implementação e capturas atuais da ficha](../descricao-independente/README.md).
- A coluna direita contém rolagem independente para enunciado, alternativas e dicas. Sua altura mínima não cresce com o texto e sua largura não comprime o retrato. Dica e Confirmar ficam fora dessa rolagem. A altura do Quiz vem do conteúdo estrutural e da área disponível, preservando o retrato enquanto a janela permanece no mesmo tamanho. O foco enquadra o controle dentro da coluna direita; nomes e dicas longos quebram linha, sem elipse.

Confirmação exige pergunta ativa, foto pronta e alternativa válida; a proteção local de reentrada e a aceitação do backend continuam em vigor. Seleção permanece ao consultar dicas/encerramento. Atalhos 1–4, Enter, Tab/Shift+Tab, Esc, foco, redução de movimento, fontes e relato local foram preservados. Nenhuma regra de dicas, pontuação, dificuldade, elegibilidade, seleção de questões, ranking, histórico, checkpoint, recuperação ou identificação científica foi alterada.

## Etapa 3

Descrições educativas completas exigirão conteúdo e revisão próprios na **Etapa 3 — Enciclopédia Científica Interativa**, com fontes e decisões concretas no fluxo de curadoria. A estrutura independente apenas prepara a região; os tópicos existentes permanecem em seus contextos originais. Não cria descrição em massa, autor, fonte, licença, revisão especializada ou aprovação fictícia.

## Comparação e validação

A [captura da interface anterior](referencia-main/quiz-lion-1240x820.png) foi novamente obtida pelo executável de testes 1.9.0 preservado, com fotografia de *Panthera leo*, QML/controlador reais e banco temporário. O [registro](referencia-main/captura.json) confirma o hash do binário anteriormente documentado e a igualdade dos três componentes de referência com a `main` anterior à PR #35. Não é uma recriação gráfica. A prévia rejeitada do README também está [arquivada](historico-proposta-rejeitada/quiz.png), com seu [registro original](historico-proposta-rejeitada/quiz.json).

**26/26 suítes CTest aprovadas**, em **365,28 s** de tempo real com `-j3`: 62 casos QtTest de domínio, 24 de interface, 32 de identidade e 21 de recuperação, incluindo preparação/limpeza. [Log completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt) e [resultados com proveniência](verificacoes/resultados.json). Os três testes direcionados passaram também em [100%](verificacoes/quiz-restored-targeted.txt), [125%](verificacoes/quiz-restored-dpi-125.txt) e [150%](verificacoes/quiz-restored-dpi-150.txt), com cinco casos por execução, contando preparação/limpeza.

Isso inclui seleção/Enter e confirmação única, Tab/Shift+Tab, hover estável, assistência efetiva, texto integral das três dicas, conservação da seleção nos diálogos, pontuação após dicas, acerto/erro, contagem somente após confirmação, ambos os modos, recuperação de sessão já com resposta/dica, foto ausente/de apoio e textos extensos. O teste compara exatamente os retângulos do retrato e da imagem no quadro renderizado com zero, uma, duas e três dicas; verifica controles dentro da área útil e foto estável ao rolar o texto da direita. A ficha confere o texto original de morfologia, resumo sem duplicação, fonte acionada, estado de revisão e ausência de nota. Os valores controlados de texto longo/ausência são apenas fixtures de apresentação, sem mudanças no catálogo e sem capturas científicas artificiais.

Build Release, [lint QML](verificacoes/qml-lint.txt), [smoke do executável](verificacoes/smoke-test.txt), metadados/histórico, status do projeto, escopo de licenciamento, integridade e hashes passaram. As [auditorias técnicas](verificacoes/technical-audits.json) e a [comparação de governança](verificacoes/governance-history.json) mantêm dados e eventos. O [resumo de distribuição](verificacoes/distribution-summary.json) conserva as pendências anteriores, sem erros técnicos/de dependências no escopo auditado.

O [manifesto atual](capturas.json) reúne **48 capturas reais**, com horário, dimensões lógicas/físicas, escala Qt, estado da partida, retângulo do retrato, contagens, créditos, fontes e hashes de arquivos/binários. Em 100%, são dez estados do Quiz e duas posições da ficha por resolução; em 125% e 150%, arquivam-se a leitura com três dicas e a descrição abaixo da foto. As execuções testam todos os estados nas três escalas. Não houve montagem, retoque ou geração de imagens.

| Resolução lógica | Sem dicas | Três dicas em leitura | Descrição abaixo da foto |
| --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/hints-0-1000x700.png) | [Abrir](capturas/hints-reading-1000x700.png) | [Abrir](capturas/species-description-reading-1000x700.png) |
| 1240×820 | [Abrir](capturas/hints-0-1240x820.png) | [Abrir](capturas/hints-reading-1240x820.png) | [Abrir](capturas/species-description-reading-1240x820.png) |
| 1920×1080 | [Abrir](capturas/hints-0-1920x1080.png) | [Abrir](capturas/hints-reading-1920x1080.png) | [Abrir](capturas/species-description-reading-1920x1080.png) |

[Uma dica](capturas/hints-1-1240x820.png) · [Duas dicas](capturas/hints-2-1240x820.png) · [Contadores após respostas](capturas/counters-after-answers-1240x820.png) · [125%](capturas/hints-3-1240x820-125.png) · [150%](capturas/hints-3-1240x820-150.png).

A prévia atual do README usa novamente *Panthera leo* com os créditos originais e a interface restaurada, em `docs/images/quiz.png` e seu registro. A comparação com a [referência anterior](referencia-main/quiz-lion-1240x820.png) conserva fotografia à esquerda, alternativas à direita, cores, cabeçalho, rodapé e controles; a limitação explícita das colunas elimina a dependência da largura em textos e o painel acrescenta as contagens. Perguntas, alternativas e pontuações podem diferir por serem sessões temporárias reais. A Home e seus arquivos de captura não foram modificados nesta revisão.

Para reproduzir, use `cmake --build build -j3` e `ctest --test-dir build --output-on-failure -j3`. Nesta execução, os binários estão em `build/2.3.1-restaurado/`. A partir de `build/tests`, execute `../2.3.1-restaurado/glossata_ui_tests.exe restoredQuizHintsCountersAndStablePortrait restoredQuizRecoveryAndUnavailableStates speciesDescriptionReusesExistingReviewedNote`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. Quadros reais são gerados em `build/tests/previews/quiz-restored-2.3.1/<escala>/`.

## Auditoria de arquivos sem exclusões

O [inventário](verificacoes/inventario-limpeza.json) registra a execução em modo **dry-run** do script `scripts/cleanup_build.py` integrado à `main` em `25a882b`. Como esta branch nasceu antes da integração da PR #34, o script foi lido daquele commit e executado como cópia idêntica em `build/auditoria-main/scripts/cleanup_build.py`, com `--root C:/glossata_game --include-old-builds`. Seu hash e origem estão registrados. Não foi necessário importar alterações, fazer merge, reset ou rebase para usá-lo.

Com os parâmetros padrão de idade mínima de 14 dias e retenção de dois conjuntos por classe, o script propôs **zero bytes** para limpeza; builds recentes/protegidos foram preservados. Um inventário complementar somente de nomes e tamanhos registra **3.108.390.845 bytes, aproximadamente 2,90 GiB**, em runtimes anteriores `build/2.2.1-B-verified`, `build/2.2.2-*`, `build/2.2.3-*`, a proposta rejeitada `build/2.3.1-quiz` e caches Python. Eles não foram declarados elegíveis à exclusão pelo script. Possíveis duplicações têm apenas o mesmo nome/tamanho, sem confirmação de identidade de conteúdo.

Nenhum arquivo foi excluído ou movido. O runtime atual `build/2.3.1-restaurado`, os artefatos necessários à compilação, capturas/evidências versionadas, dados científicos, licenças, SQLite, backups, históricos e governança permanecem preservados. Espaço lógico inventariado não é garantia de espaço recuperável nem autorização para remover arquivos.

## Limitações

Validação visual em Windows 11/Qt 6.11.2 software/offscreen, com resoluções lógicas 1000×700, 1240×820 e 1920×1080 e fatores Qt 100%, 125% e 150%. Esses fatores não alteram a configuração física de escala do Windows. Eventos de mouse/teclado são exercitados pelo QtTest; não equivalem a ensaio de GPU, touchpad ou leitor de tela físicos.

Textos e estados científicos vêm das projeções existentes, sem novas aprovações. Catálogo 2.4.0 e aplicativo 1.10.0 em desenvolvimento, sem release ou avanço para 2.3.2. As pendências de distribuição e revisão permanecem; capturas e testes não certificam ciência ou direitos.
