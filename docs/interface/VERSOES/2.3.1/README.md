# Subetapa 2.3.1 — Modernização da interface do Quiz

Base: `8293408a650ac2f9738d42c4c9c462115919bdfc`, integração da PR #33 na `main`, confirmada pela API. Branch `feat/issue-22-quiz-2.3.1`, vinculada à [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). Referência visual: exclusivamente a seção 4, tela do Quiz, da imagem fornecida; sem copiar fotografias, nomes, valores ou classificações.

Entrega na [PR #35](https://github.com/kaduolliver/Projeto-Glossata/pull/35). Implementação e testes: [`9ccbd60`](https://github.com/kaduolliver/Projeto-Glossata/commit/9ccbd60). O manifesto vincula os mesmos arquivos e binários às execuções e capturas arquivadas.

Aplicativo **1.10.0 não lançado**, MINOR pelos recursos de apresentação compatíveis; catálogo **2.4.0** preservado. Não altera regras, algoritmos, pontuação, critérios científicos, histórico, rankings ou persistência. Sem merge, release ou avanço de subetapa.

## Interface e contratos

`QuizView` apresenta cabeçalho com modo, grupo/nível e progresso; fotografia à esquerda; enunciado, quatro alternativas, pontuação e ações à direita. A composição usa duas colunas a partir de 700 pixels úteis e pode empilhar abaixo disso; títulos, nomes longos e demais conteúdos têm quebra de linha. As alternativas compartilham a maior altura intrínseca, mantêm A/B/C/D e distinguem seleção, hover, foco dourado e desabilitado. O marcador representa seleção, sem consultar ou destacar a resposta correta.

`SpeciesPortrait.quizPresentation`, opcional e desativado por padrão, centraliza o raster inteiro sem recorte ou ampliação. A geometria anterior permanece nos demais usos. Autoria, título, licença, origem, modificações e avisos continuam acessíveis como antes; não há política nova de atribuição. Imagens de apoio mantêm a limitação visível; fotografia indisponível bloqueia seleção e confirmação.

`HintPanel` mantém as três categorias e as classificações fornecidas pelo backend, sem associar categoria a um peso fixo. Mostra o impacto antes de revelar, motivos de indisponibilidade, condição provisória, fonte, avaliação, revisão e relato local. Textos completos revelados ficam no painel, com atalhos de consulta na pergunta. Valores 1,00/0,50/0,33 e a regra de menor teto ao combinar dicas são os existentes. Nenhuma dica é reclassificada.

Livre conserva seu ciclo; Competitivo usa o limite de questões do backend e a quantidade respondida. Seleção é estado local da pergunta e permanece ao abrir/fechar dicas ou encerramento. Atalhos 1–4, Enter, Tab/Shift+Tab, foco e redução de movimento são preservados. Confirmação exige fotografia pronta, alternativa válida e pergunta ativa; bloqueio local de reentrada complementa as proteções existentes do backend, inclusive em falha de checkpoint.

## Validação e evidências

**26/26 suítes CTest aprovadas**, em **366,26 s** de tempo real com `-j3`: 62 casos QtTest de domínio, 23 de interface, 32 de identidade e 21 de recuperação, incluindo preparação/limpeza. [Log completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt) e [resultados rastreáveis](verificacoes/resultados.json). As três execuções adicionais, em [100%](verificacoes/quiz-targeted.txt), [125%](verificacoes/quiz-dpi-125.txt) e [150%](verificacoes/quiz-dpi-150.txt), passaram com quatro casos cada, também contando preparação/limpeza.

Build Release, [lint QML](verificacoes/qml-lint.txt), [smoke do executável](verificacoes/smoke-test.txt), metadados/histórico, status do projeto, licenciamento do código, whitespace e hashes passaram. As [auditorias técnicas](verificacoes/technical-audits.json) conservam 122 organismos, 122 registros fotográficos válidos e o mesmo pool competitivo de 80 organismos. A [comparação histórica](verificacoes/governance-history.json) não altera os eventos de governança; o [gate de distribuição](verificacoes/distribution-summary.json) mantém as pendências anteriores, sem erros técnicos ou de dependências no escopo auditado.

Os dois testes novos usam QML e `GameController` reais, SQLite temporário e Discord desativado. `modernQuizNavigationHintsAndCaptures` cobre Livre/Competitivo, quatro alternativas, seleção única, Tab/Shift+Tab, atalhos numéricos, foco, hover estável com movimento habilitado, consulta às três dicas, seus tetos reais, conservação da seleção nos diálogos, acerto, erro e confirmação única. `modernQuizRecoveryAndUnavailableStates` retoma um checkpoint real durante a pergunta, com dicas já usadas, e compara fotografia, alternativas e pontos.

Ausência de fotografia, aviso de imagem de apoio e texto longo são também verificados com valores controlados apenas na apresentação, sem mudar catálogo ou banco. Essas situações artificiais não são publicadas como registros científicos. Os testes anteriores continuam cobrindo fontes, relato de problema, sessões, seleção taxonômica, pesquisa, filtros, carregamento progressivo, breadcrumbs e demais telas.

O [manifesto](capturas.json) reúne **36 capturas reais**, com horário, dimensões lógicas/físicas, fator Qt, estado da partida, organismo, créditos/proveniência, hashes de fontes e executáveis. São oito estados por resolução em 100%: pergunta Livre, seleção/hover, oferta de dicas, dica revelada, encerramento, feedback correto/incorreto e pergunta Competitiva. Em 125% e 150%, arquivam-se pergunta Livre e dica revelada nas três resoluções. Os feedbacks são evidências de regressão; suas telas não foram redesenhadas.

| Resolução lógica | Qt 100% | Qt 125% | Qt 150% |
| --- | --- | --- | --- |
| 1000×700 | [Pergunta](capturas/question-free-1000x700.png) | [Pergunta](capturas/question-free-1000x700-125.png) | [Pergunta](capturas/question-free-1000x700-150.png) |
| 1240×820 | [Pergunta](capturas/question-free-1240x820.png) | [Pergunta](capturas/question-free-1240x820-125.png) | [Pergunta](capturas/question-free-1240x820-150.png) |
| 1920×1080 | [Pergunta](capturas/question-free-1920x1080.png) | [Pergunta](capturas/question-free-1920x1080-125.png) | [Pergunta](capturas/question-free-1920x1080-150.png) |

[Dica revelada em 1000×700](capturas/hint-revealed-1000x700.png) · [Competitivo](capturas/question-competitive-1240x820.png) · [Seleção e foco](capturas/selected-hover-1240x820.png).

A prévia do Quiz no README foi novamente capturada pelo teste existente `navigateAndConfirmAnswers`, com a fotografia original de *Panthera leo* e os mesmos créditos. A [prévia anterior](referencia-anterior/quiz.png) e seu [registro original](referencia-anterior/quiz.json) foram arquivados sem alterações. A captura oficial da Home foi executada pelo script de produção após recompilar, devido à atualização da versão, sem reformular a tela inicial.

Para reproduzir, compile com `cmake --build build -j3` e execute `ctest --test-dir build --output-on-failure -j3`. Nesta revisão, os executáveis estão em `build/2.3.1-quiz/`. A partir de `build/tests`, execute `../2.3.1-quiz/glossata_ui_tests.exe modernQuizNavigationHintsAndCaptures modernQuizRecoveryAndUnavailableStates`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. Os quadros ficam em `build/tests/previews/quiz-2.3.1/<escala>/`.

## Inventário não destrutivo

`scripts/cleanup_build.py` não está integrado à branch; a PR #34 estava aberta e não integrada na criação desta branch. Nenhuma alteração sua foi importada. O [inventário de leitura](verificacoes/inventario-limpeza.json) enumera builds, caches e possíveis duplicações por nome/tamanho, sem ler conteúdos de bancos nem acessar perfis de jogadores. Não houve exclusão, movimentação ou limpeza automática.

Os candidatos para revisão manual são os runtimes anteriores `build/2.2.1-B-verified`, `build/2.2.2-*` e `build/2.2.3-*`, mais os caches Python explicitamente listados: **2.849.271.175 bytes, aproximadamente 2,65 GiB**, de tamanho lógico. O build atual `build/2.3.1-quiz`, dependências de compilação, testes, capturas, dados e governança permanecem preservados. Binários antigos podem ser úteis para reproduzir evidências históricas; a lista não declara sua exclusão segura. Possíveis duplicações não entram duas vezes na soma.

## Limitações

Capturas Windows 11/Qt 6.11.2 software/offscreen; escalas Qt ampliam o raster mantendo as dimensões lógicas, sem alterar a configuração física de escala do Windows. Os eventos de mouse e teclado são exercitados pelo QtTest; não houve ensaio de GPU física, touchpad ou leitor de tela. Fotografias pequenas não são ampliadas artificialmente. Nomes ou créditos excepcionalmente extensos podem aumentar a altura da página, com rolagem e foco acessíveis.

O gate de distribuição conserva suas pendências anteriores de permissões e revisão. Testes não certificam ciência, direitos ou acessibilidade universal, nem criam aprovações. Catálogo, imagens, licenças, evidências, fontes originais e 1.053 eventos de governança foram preservados. Backend, banco, pontuação, dificuldades, seleção de questões, checkpoints e telas externas ao escopo não foram alterados.
