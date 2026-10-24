# Painel de filtros de Conquistas — PR #38

Continuação da Subetapa 2.3.3 em `feat/conquistas-2-3-3`, sobre `e5f0cd0e3bcb7649435c1b903817f475beed7720`. Aplicativo **2.1.0 ainda não lançado**, catálogo **2.4.0** e SQLite **8**. Os [registros anteriores](../filtros-nivel-platina/README.md) permanecem preservados.

## Organização e interação

“Filtrar conquistas” reúne três grupos rotulados em uma superfície com borda, raio e espaçamento do tema. Estado conserva Todas/Desbloqueadas/Bloqueadas em segmentos exclusivos. Categoria oferece as sete opções existentes em um `TaxonComboBox`; Nível usa outro menu com os cinco valores `visualLevel` existentes. Controles têm 44 px de altura, indicação de seleção e foco. A largura atende aos textos sem expandir os controles para ocupar toda a linha.

`AchievementsView` usa `GridLayout`, `UiButton`, `TaxonComboBox` e `TextAction`. As adaptações dos menus são locais à view; os componentes compartilhados e `Main.qml` permanecem iguais. Os popups Qt Quick Controls aparecem na camada de sobreposição, com margem de 8 px para a janela, lista rolável, destaque da opção e navegação por teclado/Enter/Escape. Hover e transições de opacidade usam `Theme.motionEnabled` e `durationFast`; não há animação contínua.

O layout considera a largura útil após a barra lateral. Quando os três grupos cabem, ficam lado a lado à esquerda. Quando cabem apenas os dois menus, Estado ocupa a primeira linha; Categoria e Nível ficam na segunda. Abaixo dessa largura, os grupos empilham. As larguras naturais dos grupos determinam os pontos de mudança, sem rolagem horizontal.

A linha inferior mostra a contagem efetiva, incluindo singular e zero. “Limpar filtros” aparece apenas com alguma restrição e restaura os três padrões, devolvendo foco a Todas. A altura reservada da linha evita deslocamentos ao mostrar a ação. O contador global e a barra de progresso continuam derivados da coleção inteira.

O predicado permanece **estado AND categoria AND nível**. `Main.achievementsPresentation` conserva os três valores durante a navegação na execução. Não há preferência nova no SQLite, mudança de critérios, desbloqueio, pontuação, quiz, identidade, catálogo, badges ou dependências.

## Reprodução

```powershell
$env:PATH='C:\msys64\ucrt64\bin;'+$env:PATH
cmake --build build/achievements-refinements --parallel 4
ctest --test-dir build/achievements-refinements --output-on-failure -j 4
$env:QT_QPA_PLATFORM='offscreen'
$env:QT_QUICK_BACKEND='software'
$env:QT_QUICK_CONTROLS_STYLE='Basic'
$env:QT_FONT_DPI='96'
$env:QT_SCALE_FACTOR='1.25' # repetir com 1 e 1.5
build/achievements-refinements/tests/glossata_ui_tests.exe achievementsFiltersDetailsKeyboardAndCaptures
```

Os testes usam bancos/identidades temporários, Discord desativado e QML/controlador de produção. Desbloqueios vêm de respostas e consulta reais no ambiente isolado; o modelo não é substituído para fabricar conquistas.

## Comparações reais

As imagens são quadros do programa obtidos por `QQuickWindow::grabWindow`. O manifesto registra dimensões lógicas/físicas, escala, horário, hashes das fontes e dos binários. A Home e seus componentes não foram alterados; sua captura oficial anterior permanece preservada.

| Resolução lógica | Antes — 100% | Painel atual — 100% | Categoria — 125% | Nível — 150% |
| --- | --- | --- | --- | --- |
| 1000×700 | [Três linhas](../filtros-nivel-platina/capturas/100/overview-1000x700.png) | [Duas linhas](capturas/100/overview-1000x700.png) | [Menu](capturas/125/category-popup-1000x700.png) | [Menu](capturas/150/level-popup-1000x700.png) |
| 1240×820 | [Três linhas](../filtros-nivel-platina/capturas/100/overview-1240x820.png) | [Uma linha](capturas/100/overview-1240x820.png) | [Menu](capturas/125/category-popup-1240x820.png) | [Menu](capturas/150/level-popup-1240x820.png) |
| 1920×1080 | [Três linhas](../filtros-nivel-platina/capturas/100/overview-1920x1080.png) | [Uma linha](capturas/100/overview-1920x1080.png) | [Menu](capturas/125/category-popup-1920x1080.png) | [Menu](capturas/150/level-popup-1920x1080.png) |

Também são registrados filtros combinados, seleção vazia, retorno à tela, limpeza, bloqueadas/desbloqueadas, progresso parcial e detalhes dos quatro materiais. Todos os originais anteriores continuam em suas pastas.

## Validação

| Verificação | Resultado |
| --- | --- |
| CTest completo, implementação final | **26/26**, zero falhas, 462.79 s |
| Regras/controlador | **86 casos** aprovados |
| Interface completa | **31 casos** aprovados |
| Identidade/migrações | **38 casos** aprovados |
| Recuperação | **21 casos** aprovados |
| Filtros específicos | Aprovados em **100%, 125% e 150%**, nas três resoluções |
| Capturas atuais | **159 imagens**, 53 por escala |
| qmllint, smoke, auditoria de repositório/código | Aprovados |
| Status, versão e histórico | Aprovados |
| Catálogo estrito e integridade histórica | Aprovados; 122 organismos e 1.053 eventos preservados |
| Preservação de escopo | Backend, catálogo, governança, SQLite, níveis, badges e componentes compartilhados iguais à base deste ajuste |
| Distribuição | Bloqueada por pendências existentes, sem erros técnicos ou de dependências |

[Resultados estruturados](verificacoes/resultados.json), [CTest completo](verificacoes/filterbar-ctest.txt), [regras](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt), [identidade](verificacoes/identity-results.txt), [recuperação](verificacoes/recovery-results.txt) e [resumo do gate](verificacoes/distribution-summary.json). A [auditoria do conjunto staged](verificacoes/filterbar-final-staged-audit.txt) e o [gate do histórico deste ajuste](verificacoes/filterbar-final-history.txt) também passaram. Os [registros das capturas](capturas.json) vinculam fontes, binário do programa, binário/fonte do teste e commit da implementação.

O teste específico percorre os três estados, sete categorias e cinco níveis; compara os IDs realmente exibidos com as definições do controlador e verifica contagem, singular, zero e AND. Testa Escape sem confirmar, destaque inicial/navegação por setas, Enter, abertura/seleção por clique, Tab entre os menus, retorno de foco e limites dos popups. Confere limpeza dos três padrões, marcação exclusiva, conservação em memória ao navegar, resumo/barra global independentes, detalhes e redução de movimento. A grade é verificada em duas colunas de controles em 1000×700 e três nas duas larguras maiores.

As execuções específicas finais estão registradas em [100%](verificacoes/filterbar-ui-1.txt), [125%](verificacoes/filterbar-ui-1.25.txt) e [150%](verificacoes/filterbar-ui-1.5.txt). Os três relatórios iniciais foram preservados: a preparação da verificação de limpeza e a interferência do ponteiro sobre o destaque de hover ao reabrir menus foram corrigidas na automação. O resultado completo acima corresponde à versão final compilada, após esses ajustes.

Arquivos funcionais alterados: `resources/qml/views/AchievementsView.qml` e `tests/QmlInterfaceTests.cpp`. Os demais arquivos desta continuação documentam a mudança, o status derivado e as evidências. A captura oficial da Home não foi renovada porque nem ela nem seus componentes foram alterados.

## Inventário sem limpeza

Executado somente após terminar builds e testes: [dry-run padrão](verificacoes/cleanup-dry-run.json), [builds antigos em dry-run](verificacoes/cleanup-old-builds-dry-run.json) e [inventário ampliado](verificacoes/inventario-local.json). A PR #38 permanece aberta: não houve limpeza, exclusão, movimentação ou `--apply`.

| Caminho | Tamanho na varredura | Motivo e risco |
| --- | --- | --- |
| `build/achievements-refinements` | 1.918.381.084 bytes | Build compilado e relatórios recentes/anteriores; risco alto, preservar |
| `build/achievements-2-3-3` | 1.910.145.166 bytes | Build compilado e relatórios recentes/anteriores; risco alto, preservar |
| `previews/achievements-filterbar` | 20.846.290 bytes | Capturas locais e originais das evidências; risco médio para rastreabilidade, preservar |
| `build/achievements-refinements/tests/previews/achievements-filterbar` | 4.768.294 bytes | Capturas locais e originais das evidências; risco médio para rastreabilidade, preservar |
| `scripts/__pycache__` | 1.363.308 bytes | Cache Python recompilável; risco baixo, somente candidato a futura limpeza autorizada |
| `tests/__pycache__` | 585.884 bytes | Cache Python recompilável; risco baixo, somente candidato a futura limpeza autorizada |

Entradas aninhadas se sobrepõem; seus tamanhos não devem ser somados. Semelhança e duplicação de capturas não autorizam exclusão. Recursos, dados de jogadores, builds recentes, documentação e evidências anteriores foram preservados.

## Limites

Capturas offscreen com escalas Qt não certificam leitores de tela ou troca física de DPI entre monitores. Testes não concedem revisão científica ou licença: as pendências de distribuição e de escopo dos desenhos da etapa anterior permanecem. Sem merge, tag, release ou início da 2.3.4.
