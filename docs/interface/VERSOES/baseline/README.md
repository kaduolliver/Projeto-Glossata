# Linha de base visual anterior à reformulação

Interface de produção no commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e` (`git checkout b201e20c2d83fda6d790bc6a96217c0fdc75307e` em checkout separado para consultar o código). Aplicativo 1.7.0 em desenvolvimento; este registro **não é um lançamento**. As imagens são da área de conteúdo de `QQuickWindow`, sem moldura do Windows.

As 14 imagens abaixo foram produzidas em 2026-10-19 pelo teste existente `QmlInterfaceTests.navigateAndConfirmAnswers`: `QQmlApplicationEngine` carregou o `Main.qml` de produção, com `GameController` real e SQLite em `QTemporaryDir`, e `QQuickWindow::grabWindow` gravou cada PNG. O teste percorreu telas/estados reais, inclusive partida e resultados de teste; o perfil e o ranking capturados são temporários. Discord não foi iniciado. Backend Qt offscreen/software, escala 1, 96 DPI, Qt Quick Controls Basic. O [manifesto](capturas.json) inclui SHA-256, dimensão 1240 × 820, horário UTC por arquivo, nome de saída do teste e hash do executável de teste.

O teste inicia e interrompe sessões em sequência. Por isso, algumas capturas posteriores, como o painel de dicas, exibem um aviso de sessão recuperável; ele é parte do estado real alcançado pelo fluxo de teste, não um estado inicial da tela.

| Estado capturado | Imagem |
| --- | --- |
| Início | [inicio.png](capturas/inicio.png) |
| Exploração e filtros | [explorar.png](capturas/explorar.png) |
| Navegação do domínio Eukaryota | [dominio-eukaryota.png](capturas/dominio-eukaryota.png) |
| Atalho Animalia | [grupo-animalia.png](capturas/grupo-animalia.png) |
| Quiz de quatro alternativas | [quiz.png](capturas/quiz.png) |
| Painel de dicas | [painel-dicas.png](capturas/painel-dicas.png) |
| Resposta correta | [resposta-correta.png](capturas/resposta-correta.png) |
| Resposta incorreta | [resposta-incorreta.png](capturas/resposta-incorreta.png) |
| Ficha de espécie | [ficha-especie.png](capturas/ficha-especie.png) |
| Resultado de partida de teste | [resultado.png](capturas/resultado.png) |
| Estatísticas e ranking de teste | [estatisticas.png](capturas/estatisticas.png) |
| Conquistas | [conquistas.png](capturas/conquistas.png) |
| Perfis e ajustes | [perfis-ajustes.png](capturas/perfis-ajustes.png) |
| Créditos e transparência | [creditos.png](capturas/creditos.png) |

Para reproduzir: configure e compile `glossata_ui_tests`, defina as variáveis do manifesto, execute `build/tests/glossata_ui_tests.exe navigateAndConfirmAnswers` **a partir de `build/tests`** e confira `build/tests/previews/`. O teste exercita fluxos completos; alguns estados dependem dos dados de catálogo do commit. A captura oficial da tela inicial no README principal permanece em `docs/images/tela-inicial.png` e tem registro próprio. Estas imagens não são conceitos gerados e não substituem aquela captura.

Limitações: o renderizador offscreen não representa molduras do sistema operacional, fontes/antialiasing de toda GPU ou comportamento de leitor de tela. Foram arquivadas imagens em 1240 × 820; o teste também gera estados compactos e largura maior em `build/tests/previews/`, mas estes não integram o conjunto fixo. Algumas fotografias e o branding vistos nas capturas mantêm seus direitos e pendências individuais; este registro não altera autorização de distribuição.
