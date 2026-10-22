# PR #33 — dificuldade junto ao seletor

Ajuste de apresentação sobre `bb202b03621732b51521d56d8f925bad18229183`, na mesma [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33) e branch `feat/issue-22-play-2.2.3`. Aplicativo 1.9.0 não lançado; catálogo 2.4.0 preservado. Sem merge, release ou avanço de subetapa.

Código e testes em [da23116](https://github.com/kaduolliver/Projeto-Glossata/commit/da23116e837f1cf3de9b30572883748964a058dc). As execuções e capturas correspondem às mesmas fontes e executáveis finais.

Em `TaxonomyView.qml`, a dificuldade passa a acompanhar o seletor com intervalo de 20 pixels lógicos (`spaceLg`), alinhada à esquerda na coluna flexível. O espaço disponível permanece depois do texto, mantendo o botão Iniciar partida compacto na margem interna direita. Nas janelas compactas, a dificuldade passa para baixo do seletor, na mesma linha do início à direita. Os controles de cada linha ficam centralizados verticalmente. Altura derivada do conteúdo, margens, espaçamento vertical, contador, modelos, dificuldade e ação de início são preservados.

O teste existente `compactPlayConfigurationAndContextualSources` confere a distância de 16–24 pixels lógicos nas janelas amplas e a posição abaixo do seletor nas compactas. Conserva as verificações de borda direita do início, largura intrínseca, centralização vertical, ausência de sobreposição e altura compacta. Escala Qt altera o raster físico; o espaçamento continua em pixels lógicos.

As evidências de [alinhamento do início](../alinhamento-iniciar/README.md) e da [configuração compacta](../configuracao-compacta/README.md) permanecem intactas.

## Validação e capturas

Os três testes direcionados de interface — `compactPlayConfigurationAndContextualSources`, `playConfigurationReturnsToSelectionContext` e `playSelectionNavigationAndCaptures` — passaram: **cinco casos**, contando inicialização/limpeza. As execuções adicionais do primeiro teste nas escalas Qt [125%](verificacoes/play-difficulty-dpi-125.txt) e [150%](verificacoes/play-difficulty-dpi-150.txt) passaram com três casos cada, também incluindo preparação/limpeza. [Log direcionado](verificacoes/play-difficulty-alignment.txt) e [resultados](verificacoes/resultados.json).

Isso inclui as três resoluções, distâncias e alinhamentos dos controles, seleção de níveis, dificuldade preservada, grupos vazios/limitados, fontes contextuais, mouse, teclado, foco, redução de movimento, retorno à aba/rolagem e início Livre, Competitivo e misto. A suíte CTest completa anterior permanece registrada; não foi repetida para este ajuste exclusivamente de apresentação. Build Release, lint do QML alterado, metadados/histórico, status do projeto, escopo de licenciamento do código, whitespace e [smoke do executável](verificacoes/smoke-test.txt) passaram.

São **nove capturas reais**, obtidas por `QQuickWindow::grabWindow`, com QML/controlador de produção, SQLite temporário e Discord desativado. O [manifesto](capturas.json) registra dimensões lógicas/físicas, fator Qt, horário, fontes, executáveis e hashes. Sem montagem, retoque ou edição das fotografias.

| Resolução lógica | Qt 100% | Qt 125% | Qt 150% |
| --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/configure-1000x700.png) | [Abrir](capturas/configure-1000x700-125.png) | [Abrir](capturas/configure-1000x700-150.png) |
| 1240×820 | [Abrir](capturas/configure-1240x820.png) | [Abrir](capturas/configure-1240x820-125.png) | [Abrir](capturas/configure-1240x820-150.png) |
| 1920×1080 | [Abrir](capturas/configure-1920x1080.png) | [Abrir](capturas/configure-1920x1080-125.png) | [Abrir](capturas/configure-1920x1080-150.png) |

Para repetir, compile com `cmake --build build -j3`. A partir de `build/tests`, execute `../2.2.3-dificuldade/glossata_ui_tests.exe compactPlayConfigurationAndContextualSources`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. Os caminhos dependem do diretório de executáveis configurado no CMake. O teste gera os quadros em `build/tests/previews/play-compact-2.2.3/<escala>/`; esta revisão arquiva somente a configuração.

Windows 11/Qt 6.11.2 software/offscreen. Fatores Qt ampliam o raster sem alterar a resolução lógica ou a escala física do Windows. Eventos de mouse/teclado pelo QtTest, sem ensaio de GPU física ou leitor de tela. Dados científicos, fotografias, licenças, governança, backend e outras telas permanecem intactos. As fontes da Home não mudaram, portanto sua captura oficial foi preservada.
