# 2.3.2 — Painéis no topo e rolagem compartilhada

Revisão da [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36) sobre `e02e43bf8bc8f4d2d190402edff3657cdce3524d`. Preserva a composição visual aprovada, o diagnóstico de métricas e todas as evidências da [entrega inicial](../README.md).

Escopo: colocar taxonomia/ranking logo após o cabeçalho, oferecer alternância em telas compactas com contexto em memória e habilitar a rolagem convencional existente em Estatísticas. A documentação multiplayer será uma proposta separada, sem implementação de infraestrutura, mudança de backend, SQLite, dados ou regras. Aplicativo 1.11.0 ainda não lançado e catálogo 2.4.0 preservados.

## Alterações

A ordem passa a ser cabeçalho, painéis taxonômico/ranking, partida atual, desempenho, dicas e histórico. Em larguras úteis de pelo menos 900 pixels, os painéis permanecem lado a lado, com as proporções anteriores e alinhamento superior; cada um acompanha a altura do conteúdo. Em larguras menores, os seletores **Desempenho taxonômico** e **Ranking** exibem um painel de cada vez, logo após o cabeçalho. Modelos, grupos e posições permanecem disponíveis.

`Main.statisticsPresentation.panel` guarda somente a seleção de apresentação em memória. A escolha sobrevive à navegação para outras páginas e ao redimensionamento, sem escrita em banco ou mudança de sessão. Os botões exclusivos permanecem marcados mesmo ao clicar novamente na opção ativa. Tab/Espaço e o foco visível são preservados; quando o redimensionamento oculta o painel focado, o foco passa ao seletor do painel selecionado.

`pageWheelArea` apenas inclui `statistics` no mesmo mecanismo de Início/Explorar. O passo de 110 pixels, animação e redução de movimento são os existentes. Pixels de touchpad e ângulos parciais continuam nativos. A pressão da barra já interrompe a animação compartilhada; não foi criado outro Flickable ou rolagem interna nos painéis. O espaço reservado para a barra permanece igual.

## Planejamento multiplayer

[ESTATISTICAS_MULTIPLAYER.md](../../../../architecture/ESTATISTICAS_MULTIPLAYER.md) separa diagnóstico existente de propostas futuras. Descreve perfis, resultados, rankings e histórico locais; possibilidades de cliente/API/autenticação/banco online; modelo conceitual; validação de resultados; idempotência; sincronização; privacidade e compatibilidade offline. REST, OAuth/OIDC e PostgreSQL são candidatos para avaliação, sem decisão de implementação presumida.

O documento distingue histórico local importado de partida validada pelo servidor. Hash, UUID ou consistência matemática local não tornam um resultado competitivo confiável. Uma futura seleção **Local | Global** é somente proposta; o aplicativo continua apresentando **Ranking local**. Nenhum dado de jogador foi enviado a um serviço multiplayer, e o roadmap principal não foi alterado.

## Rastreabilidade e preservação

Somente `StatisticsView.qml` e `Main.qml` mudam no código da aplicação. Backend, banco, perfis, pontos, dificuldade, dicas, recuperação, catálogo e governança permanecem intactos. Os testes iniciais de Estatísticas são mantidos, com a adaptação de navegação necessária para o novo seletor compacto. As capturas, manifestos e resultados iniciais da PR #36 permanecem nos mesmos arquivos.

Como `Main.qml` integra o shell compartilhado, a regra de captura do projeto exige renovar a imagem real da Home. A captura oficial em `docs/images/tela-inicial.png` passa a mostrar o topo, com manifesto correspondente e sem alterar a Home. A [imagem anterior da Descoberta do Dia](anterior-home.png) e seu [registro original](anterior-home.json), copiados dos bytes de `e02e43b`, são preservados nesta revisão; o README mantém ambos os enquadramentos e o caminho oficial. A diferença entre PNGs vem de enquadramento real do programa, sem edição de pixels.

Código, testes e planejamento no commit [`e2678efc8c422f7d8a42dce4f90b013833ef6796`](https://github.com/kaduolliver/Projeto-Glossata/commit/e2678efc8c422f7d8a42dce4f90b013833ef6796), nesta mesma PR #36. A versão 1.11.0 continua em desenvolvimento, sem release, merge ou avanço para 2.3.3.

## Validação

**26/26 suítes CTest aprovadas** na implementação final, em **414,68 segundos** com `-j3`. Casos QtTest incluindo preparação/limpeza: domínio 62, interface 27, identidade 32 e recuperação 21. [Resultado completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt), [resumo verificável](verificacoes/resultados.json).

Os três testes de Estatísticas (`statisticsTopPanelsAndSharedScrolling`, `statisticsContextsAndConfirmedMetrics`, `statisticsResponsiveRankingAndCaptures`) passaram nas três resoluções e nas escalas Qt 100%, 125% e 150%: cinco casos por execução, incluindo preparação/limpeza. [100%](verificacoes/statistics-top-targeted.txt), [125%](verificacoes/statistics-top-dpi-125.txt), [150%](verificacoes/statistics-top-dpi-150.txt). A primeira execução completa antecedeu o ajuste de exclusividade; a suíte e os testes direcionados foram repetidos depois dele, e os registros publicados são da implementação final.

O teste novo verifica painéis após o cabeçalho, alinhamento superior nas amplas, visibilidade alternada nas compactas, seleção marcada inclusive no clique repetido, Tab/Espaço, mouse, retorno de outra página e preservação ao redimensionar. Confere todos os IDs/grupos/subgrupos recebidos, largura disponível, faixa de segurança e foco transferido do painel ocultado ao seletor. A alternância entre Livre/Competitivo não muda sessão, dificuldade ou pontos.

Na rolagem, verifica 110 pixels por incremento com movimento reduzido; dois incrementos animados acumulados; pixels de touchpad e ângulos parciais comparados diretamente ao caminho nativo do mesmo Flickable; pressão da barra durante uma animação; estabilidade enquanto pressionada; arrasto real do indicador e ausência de retomada inesperada depois de soltar. As regressões de Início/Explorar incluem seus testes anteriores de roda, dispositivos de alta precisão, fotografia, pesquisa e contexto. Jogar, Quiz, ficha, histórico, identidades e recuperação também passaram na suíte completa.

Build Release/Ninja/GCC 16.2.0, Qt 6.11.2 em Windows 11; lint de todos os QML e smoke do executável aprovados. Também passaram status do projeto, metadados/histórico, licenciamento do código, auditorias técnicas e preservação dos 1.053 eventos de governança. [Comandos e resultados](verificacoes/technical-audits.json), [histórico de governança](verificacoes/governance-history.json). O gate de distribuição permanece bloqueado pelas mesmas pendências de permissões/revisões, com retorno esperado 2 e sem erros técnicos/de dependências: [resumo](verificacoes/distribution-summary.json). Testes não certificam curadoria ou conformidade jurídica.

Comandos principais de reprodução, na raiz do projeto, com Qt no PATH:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.3.2-topo
cmake --build build -j3
ctest --test-dir build --output-on-failure -j3
python scripts/capture_home.py --binary build/2.3.2-topo/projeto_glossata.exe --section top
```

As execuções direcionadas usam `glossata_ui_tests.exe` em `build/tests`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. Os métodos C++ geram os quadros e seus registros UTC em `build/tests/previews/statistics-top-2.3.2/<escala>/`; o teste anterior preservado também produz a cena de Competitivo preenchido. Todos usam SQLite temporário e Discord desativado.

## Capturas atuais

**72 capturas reais de Estatísticas:** 63 do teste novo e nove de Competitivo preenchido, geradas pelo teste anterior preservado. O [manifesto](capturas.json) identifica cada teste, commit, hashes de fontes/PNGs/binários, datas UTC, dimensões lógicas/físicas, escala, rolagem e métricas. Não há fixture científica ou de jogador global nas imagens publicadas. Outros casos de regressão continuam testando legado, nomes longos e grupos sem respostas por fixtures de apresentação isoladas.

| Estado — 100% | 1000×700 | 1240×820 | 1920×1080 |
| --- | --- | --- | --- |
| Taxonomia vazia | [PNG](capturas/empty-taxonomy-1000x700.png) | [PNG](capturas/empty-taxonomy-1240x820.png) | [PNG](capturas/empty-taxonomy-1920x1080.png) |
| Ranking vazio | [PNG](capturas/empty-ranking-1000x700.png) | [PNG](capturas/empty-ranking-1240x820.png) | [PNG](capturas/empty-ranking-1920x1080.png) |
| Taxonomia no topo | [PNG](capturas/top-taxonomy-1000x700.png) | [PNG](capturas/top-taxonomy-1240x820.png) | [PNG](capturas/top-taxonomy-1920x1080.png) |
| Ranking Livre no topo | [PNG](capturas/top-ranking-free-1000x700.png) | [PNG](capturas/top-ranking-free-1240x820.png) | [PNG](capturas/top-ranking-free-1920x1080.png) |
| Contexto Competitivo vazio | [PNG](capturas/top-ranking-competitive-1000x700.png) | [PNG](capturas/top-ranking-competitive-1240x820.png) | [PNG](capturas/top-ranking-competitive-1920x1080.png) |
| Competitivo preenchido | [PNG](capturas/ranking-competitive-1000x700.png) | [PNG](capturas/ranking-competitive-1240x820.png) | [PNG](capturas/ranking-competitive-1920x1080.png) |
| Barra após arrasto | [PNG](capturas/scrollbar-drag-1000x700.png) | [PNG](capturas/scrollbar-drag-1240x820.png) | [PNG](capturas/scrollbar-drag-1920x1080.png) |
| Histórico e métricas inferiores | [PNG](capturas/profile-history-1000x700.png) | [PNG](capturas/profile-history-1240x820.png) | [PNG](capturas/profile-history-1920x1080.png) |

Os mesmos estados em 125%/150% têm sufixos `-125.png`/`-150.png` e constam individualmente do manifesto. Exemplos: [taxonomia compacta em 150%](capturas/top-taxonomy-1000x700-150.png), [ranking compacto em 125%](capturas/top-ranking-free-1000x700-125.png), [painéis amplos em 150%](capturas/top-taxonomy-1920x1080-150.png). Nas amplas os dois painéis estão visíveis, independentemente do nome do enquadramento. Cenas de foco/rolagem podem deixar o cabeçalho fora da janela; não representam corte permanente do conteúdo.

Limitações: renderização Qt software/offscreen e fatores Qt, sem ensaio de GPU, escala física do Windows ou leitor de tela físico. A alta precisão foi verificada com eventos Qt sintéticos equivalentes ao caminho nativo; não foi usado um touchpad físico. Não há implantação ou teste de infraestrutura multiplayer.

## Auditoria de arquivos — somente simulação

Após terminar builds/testes, foram executados `python scripts/cleanup_build.py` e, complementarmente, `--include-old-builds`, ambos em **dry-run**. Os testes próprios do auditor também passaram. Os limites de idade/retenção e as proteções resultaram em **zero bytes automaticamente elegíveis**; nenhuma política foi alterada e nenhum arquivo foi excluído ou movido.

O [inventário complementar](verificacoes/auditoria-arquivos.json) registra **3.823.784.367 bytes (3,56 GiB)** para revisão manual, com caminhos, tamanhos, motivos e riscos de builds antigos, caches e capturas locais. Os 16 grupos de possíveis duplicações usam apenas nome/tamanho; não se afirmou igualdade de conteúdo nem ganho de espaço confirmado. Exemplos de caminhos retidos: `build/2.3.1-descricao`, `build/2.3.1-restaurado`, `build/2.2.3-dificuldade` e pastas anteriores de `build/tests/previews`.

Os builds `build/2.3.2-topo`, `build/2.3.2-estatisticas` e `build/2.3.2-before`, assim como as capturas locais desta subetapa, ficam protegidos para reprodução. Evidências versionadas anteriores, licenças, catálogo, governança, backups, dados de jogadores, `package/` e políticas de limpeza permanecem intactos. O inventário não é autorização de exclusão; qualquer limpeza segue procedimento independente do `AGENTS.md`.
