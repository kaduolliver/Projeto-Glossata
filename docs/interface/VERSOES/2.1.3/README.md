# Revisão 2.1.3 — Navegação e estrutura

Subetapa exclusiva da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22), baseada em `4449f8468d0e70485b5fa3b90f3814595e6771f9` (PR #28 integrado). O número identifica a etapa da interface, não um lançamento do aplicativo. Commit e PR desta implementação serão registrados após a criação.

## Decisões e diferenças

A barra lateral mantém o logotipo oficial e acrescenta Explorar entre Início e Jogar. Usa os tokens do design system, seleção e foco dos NavItem existentes. O cabeçalho mostra seção, contexto e acesso a Retomar partida quando aplicável. A largura lateral é 200 px na janela compacta e 220 px nas demais; margens principais de 20/32 px. O conteúdo continua rolável.

Explorar reutiliza CategoryView/TaxonomyView com `browsingOnly`; apresenta domínios, atalhos, fontes, breadcrumbs e grupos filhos. Oculta controles de partida. Jogar mantém Livre/Competitivo, dificuldade e níveis elegíveis. Os cartões e as fotografias existentes não foram reformulados. O botão da Home agora diz “Começar a jogar”, mantendo sua função.

O contexto de navegação é separado do estado da sessão. Uma visita a outro grupo não muda questões, alternativas, dicas ou pontuação. Retornar pelo menu Jogar preserva feedback/ficha de resposta confirmada e resultados pendentes. Sair de uma partida continua exigindo o fluxo existente de encerramento; as decisões sobre recuperação permanecem explícitas.

Arquivos de produção alterados: `Main.qml`, `style/Theme.qml`, `views/CategoryView.qml`, `views/TaxonomyView.qml`, `views/HomeView.qml`. Testes: `tests/QmlInterfaceTests.cpp`. Não há alterações no controlador, no banco, nas views de Quiz/Estatísticas/Conquistas, nos 122 organismos ou nas 855 receitas.

## Capturas e validação

Foram arquivadas 12 capturas reais. O [manifesto](capturas.json) registra dimensões, horários reais, hashes de fontes/imagens/binário e método de captura. A Home de 1240 × 820 é a captura canônica produzida pelo executável via `scripts/capture_home.py`; as demais vêm da janela Qt real dos testes, com controlador e SQLite temporário. Não houve acesso a perfis reais.

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Início | [Captura](capturas/home-1000x700.png) | [Captura](capturas/home-1240x820.png) | [Captura](capturas/home-1920x1080.png) |
| Explorar | [Captura](capturas/explore-1000x700.png) | [Captura](capturas/explore-1240x820.png) | [Captura](capturas/explore-1920x1080.png) |
| Jogar | [Captura](capturas/play-1000x700.png) | [Captura](capturas/play-1240x820.png) | [Captura](capturas/play-1920x1080.png) |

Também foram preservadas [linhagem com breadcrumbs](capturas/lineage-1240x820.png), [sessão recuperável](capturas/recovery-1000x700.png) e [resultado pendente](capturas/pending-1000x700.png). O último estado foi produzido por falha de escrita injetada exclusivamente no banco temporário. Os testes exercitam esses estados nas três resoluções, embora o arquivo documental selecione apenas um tamanho para cada um desses três exemplos.

Validação local em Windows 11/Qt 6.11.2:

- Compilação Release concluída; 26/26 suítes CTest aprovadas (360,90 s).
- Após o ajuste final de rolagem do foco, a suíte Qt/QML foi reexecutada: oito casos aprovados, incluindo inicialização/finalização (53,07 s).
- Navegação por Tab/Espaço, seleção de contexto, ausência de controles de partida em Explorar, domínios vazios, breadcrumbs, retorno ao Jogar, preservação de alternativas/dicas/teto/resposta confirmada, conclusão, resultado pendente e recuperação de sessão passaram.
- Qmllint dos arquivos alterados, auditoria estrita do catálogo, verificação Apache e metadados de release passaram. São preservados os 122 organismos e os arquivos das 855 receitas, sem modificação.
- O gate de distribuição continua bloqueado: zero erros técnicos, 120 pendências fotográficas, zero pendências de outros materiais e 275 bloqueios científicos. Não houve ampliação de permissões.

Para reproduzir: `cmake --build build -j 4`, `ctest --test-dir build --output-on-failure`, `python scripts/capture_home.py`. A suíte `GlossataInterfaceTests` grava os estados em `build/tests/previews/navigation/`. O manifesto liga os PNGs arquivados aos binários e fontes usados; as capturas anteriores não são regeneradas por esse procedimento.

A referência estética não é uma captura. Os registros de [baseline](../baseline/README.md) e [2.1.2](../2.1.2/README.md) permanecem intactos. Renderização validada em software/offscreen, escala 1; a janela compacta exige rolagem para conteúdo longo. Não houve ensaio em outras GPUs, escalas de sistema ou leitores de tela. Redução de movimento usa a preferência existente, sem introduzir animações adicionais.

## Direitos

As imagens de tela documentam a interface; não concedem autorização coletiva de redistribuição dos elementos reproduzidos. As fotografias de exploração mantêm autores, fontes, licenças e alterações em [PHOTO_CREDITS.md](../../../PHOTO_CREDITS.md); veja também [EXPLORACAO_E_FOTOGRAFIAS.md](../../../EXPLORACAO_E_FOTOGRAFIAS.md) e [DIREITOS_E_LICENCAS.md](../../../DIREITOS_E_LICENCAS.md). A identidade visual permanece independente do código Apache 2.0. Não foram adicionadas fotografias nem ampliadas permissões.
