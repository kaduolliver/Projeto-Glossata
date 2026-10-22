# PR #33 — configuração direta e retorno à seleção

Base: `a2197591b48fe877beb0b6cc528c90c5b3836511`, mesma branch `feat/issue-22-play-2.2.3` e [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33). Correção compatível no aplicativo 1.9.0 ainda não lançado; catálogo 2.4.0 preservado. Evidências da entrega anterior permanecem intactas.

Escopo: remover os painéis duplicados da página do grupo e conservar a aba/rolagem da seleção enquanto o aplicativo estiver aberto. Sem alterações em banco, regras científicas, elegibilidade, mecanismos de início de partidas ou outras subetapas.

Implementação, testes e captura oficial da Home em [24b225f](https://github.com/kaduolliver/Projeto-Glossata/commit/24b225fbdc8e92740605a18ccce7d88334829889). O commit das fontes é o mesmo para a execução completa, os testes direcionados e as novas capturas; os manifestos guardam também os hashes dos executáveis.

## Configuração e retorno

Os painéis completos Livre e Competitivo ficam somente na seleção principal de Jogar. A página do grupo apresenta identificação, resumo científico existente e suas fontes/avisos, configuração do nível de identificação, contagem elegível, botão Iniciar partida e descendentes. A dificuldade escolhida na tela principal continua no controlador e aparece como informação na configuração; a navegação não a redefine. Competitivo conserva o início global pela seleção principal. As chamadas `beginTaxonGame` e `beginCompetitiveGame` e suas condições permanecem preservadas.

`Main.playPresentation` guarda somente aba, posição vertical, controle de origem e indicação de foco por teclado. `CategoryView` reutiliza as rotinas `rememberPresentation` e `restorePresentation` de Explorar, com mapas separados. O retorno de um grupo restaura Reinos, Domínios ou Grupos avançados e a posição anterior. Por teclado, restaura também o controle acionado; por mouse, não força foco que deslocaria um cartão parcialmente visível. A posição é limitada à área disponível após redimensionamento. O estado permanece ao visitar outra seção e termina ao fechar o aplicativo, sem persistência no banco.

`Main.qml` recebe apenas a propriedade de apresentação e o comentário correspondente. Os componentes compartilhados, o código C++ de produção, Quiz, Início, Estatísticas, Conquistas, dados, mídias e governança permanecem iguais à base desta revisão. Explorar conserva seus filtros, paginação, chave de pesquisa e restauração próprios. Os contratos atuais estão em [Componentes](../../../COMPONENTES.md) e [Arquitetura](../../../ARQUITETURA_UI.md).

## Testes

A suíte completa `ctest --test-dir build --output-on-failure` passou em **26/26 suítes**, incluindo **62 casos QtTest de domínio, 20 de interface, 32 de identidade e 21 de recuperação**, contando inicialização/limpeza; tempo total real informado pelo CTest: **489,40 s**. O [log completo](verificacoes/ctest-results.txt), o [log da interface](verificacoes/ui-test-results.txt) e os [resultados com hashes e proveniência](verificacoes/resultados.json) registram a execução sobre as fontes finais desta revisão.

O novo `playConfigurationReturnsToSelectionContext` cobre as três resoluções e todos os onze cartões das três abas: 33 percursos de ida/volta por teclado. Confere IDs, aba marcada, foco, rolagem com diferença inferior a um pixel, dificuldade anterior, ausência dos painéis redundantes e disponibilidade do início. Exercita também clique por mouse em cartão parcialmente visível, retorno após visitar Início, acesso rápido, grupo vazio e grupo limitado. A sessão recuperável é criada pelo próprio jogo em SQLite temporário; permanece inalterada durante a navegação e é retomada com a mesma questão, alternativas, fotografia e pontuação. A navegação posterior entre Explorar, Início e Jogar preserva a partida em andamento.

O teste existente `playSelectionNavigationAndCaptures` foi ajustado ao novo contrato da página do grupo e continua verificando o início Livre, Competitivo e misto nas três resoluções. Os demais testes mantêm as regressões de pesquisa, filtros, paginação, breadcrumbs, créditos, fotografias, dicas, pontuação e recuperação. A execução [direcionada final](verificacoes/play-context-targeted.txt) passou com quatro casos; as execuções adicionais em [125%](verificacoes/play-context-dpi-125.txt) e [150%](verificacoes/play-context-dpi-150.txt) passaram com três casos cada, incluindo preparação/limpeza.

Build Release, lint QML, escopo de licenciamento do código, status do projeto, metadados/histórico de versões e [smoke do executável](verificacoes/smoke-test.txt) passaram. As [auditorias técnicas](verificacoes/technical-audits.json) preservam 122 organismos, 122 registros fotográficos válidos, 114 fotos de exploração, 533 vínculos e 623 cartões. A comparação com a base conserva os [1.053 eventos de governança](verificacoes/governance-history.json). O [gate de distribuição](verificacoes/distribution-summary.json) mantém os bloqueios anteriores, sem erros técnicos ou de dependências; nenhuma aprovação foi criada ou contornada.

## Capturas reais

São **24 novos PNGs** da interface e controlador reais, obtidos por `QQuickWindow::grabWindow` em QtTest: seis estados em cada resolução a 100%, mais a configuração nas três resoluções a 125% e 150%. O [manifesto](capturas.json) registra horário, tela, táxon, aba, rolagem, dimensões lógicas/físicas, fator Qt, commit das fontes e hashes dos PNGs e executáveis. Os bancos são temporários e o Discord está desativado. As capturas mostram um aviso de sessão recuperável criado pelo teste; esse aviso continua fora da área de configuração e pode exigir rolagem para vê-la por inteiro nas janelas menores.

| Resolução lógica | Configuração | Retorno Reinos | Retorno Domínios | Retorno Avançados | Sem partida |
| --- | --- | --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/configure-1000x700.png) | [Abrir](capturas/return-kingdoms-1000x700.png) | [Abrir](capturas/return-domains-1000x700.png) | [Abrir](capturas/return-advanced-1000x700.png) | [Vazio](capturas/empty-group-1000x700.png) / [limitado](capturas/limited-group-1000x700.png) |
| 1240×820 | [Abrir](capturas/configure-1240x820.png) | [Abrir](capturas/return-kingdoms-1240x820.png) | [Abrir](capturas/return-domains-1240x820.png) | [Abrir](capturas/return-advanced-1240x820.png) | [Vazio](capturas/empty-group-1240x820.png) / [limitado](capturas/limited-group-1240x820.png) |
| 1920×1080 | [Abrir](capturas/configure-1920x1080.png) | [Abrir](capturas/return-kingdoms-1920x1080.png) | [Abrir](capturas/return-domains-1920x1080.png) | [Abrir](capturas/return-advanced-1920x1080.png) | [Vazio](capturas/empty-group-1920x1080.png) / [limitado](capturas/limited-group-1920x1080.png) |

| Resolução lógica | Configuração Qt 125% | Configuração Qt 150% |
| --- | --- | --- |
| 1000×700 | [Abrir](capturas/configure-1000x700-125.png) | [Abrir](capturas/configure-1000x700-150.png) |
| 1240×820 | [Abrir](capturas/configure-1240x820-125.png) | [Abrir](capturas/configure-1240x820-150.png) |
| 1920×1080 | [Abrir](capturas/configure-1920x1080-125.png) | [Abrir](capturas/configure-1920x1080-150.png) |

Para reproduzir, use `cmake --build build -j3` e `ctest --test-dir build --output-on-failure` no ambiente Qt configurado. Para a execução direcionada: `build/2.2.3-contexto/glossata_ui_tests.exe playConfigurationReturnsToSelectionContext`, com diretório de trabalho `build/tests`, `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR` igual a `1`, `1.25` ou `1.5`. O teste gera seis estados por resolução em `build/tests/previews/play-context-2.2.3/<escala>/`. O caminho do executável é relativo à raiz do repositório; ao executá-lo de `build/tests`, use `../2.2.3-contexto/glossata_ui_tests.exe`.

A [Home oficial](../../../../images/tela-inicial.png) e seu [registro](../../../../images/tela-inicial.json) foram renovados pelo script de captura após recompilar: `python scripts/capture_home.py --binary build/2.2.3-contexto/projeto_glossata.exe --section discovery`. O PNG acompanha a alteração de `Main.qml` no mesmo commit; não há alteração de comportamento em `HomeView.qml` ou montagem de imagens.

## Limites e preservação

Windows 11, Qt 6.11.2, renderização software/offscreen; os fatores Qt ampliam o raster sem alterar a resolução lógica. Não houve alteração da escala física do Windows, teste com outras GPUs, touchpad físico ou leitores de tela. Os eventos de mouse/teclado são enviados pelo QtTest; a validação não representa certificação universal de acessibilidade.

As [evidências anteriores de 2.2.3](../README.md) e de [2.2.2](../../2.2.2/README.md) permanecem intactas em seus diretórios originais, salvo o apontamento da revisão atual no README de 2.2.3. Mantêm-se as pendências anteriores de distribuição e o diagnóstico histórico da Home com frame zero. Esta revisão não altera conteúdo científico, fotografia, licença, permissão, sessão de jogador ou pontuação e não realiza merge, lançamento ou avanço para 2.3.1.
