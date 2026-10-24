# Filtro por nível e Platina — refinamentos finais da PR #38

Continuação da Subetapa 2.3.3 em `feat/conquistas-2-3-3`. Aplicativo **2.1.0 ainda não lançado**, catálogo **2.4.0** e SQLite **8**. As [evidências anteriores](../refinamentos/README.md) permanecem preservadas. Nenhum código de backend, critério, nível, pontuação ou mecanismo de persistência foi alterado neste refinamento.

## Três filtros combinados

As opções “Todos os níveis”, “Inicial — Bronze”, “Intermediário — Prata”, “Avançado — Ouro” e “Especial — Platina” usam o `visualLevel` já projetado pelo backend. A lista aplica **estado AND categoria AND nível**, atualiza o contador filtrado e mantém o resumo global das 30 conquistas independente da seleção. Combinações sem resultado preservam a mensagem de seleção vazia.

`AchievementsView` inicializa `level: all` no objeto existente `Main.achievementsPresentation` ao abrir pela primeira vez; o objeto guarda `state`, `category` e `level`. O arquivo `Main.qml` não precisa mudar para receber essa chave dinâmica. A seleção permanece ao sair da tela e retornar durante a execução; não é gravada no SQLite. Os três grupos têm espaçamento compacto. Os botões de nível usam `UiButton`, indicação de check, foco visível, nomes acessíveis, mouse, Tab/Shift+Tab e Espaço. A linha de níveis cabe em 1000×700 com o tamanho tipográfico de legenda, sem alterar o tamanho dos alvos de interação.

Cada grupo é autoexclusivo: clicar novamente na opção atual conserva sua marcação visual, em vez de deixar o filtro ativo sem botão marcado.

## Prata e Platina

A Prata conserva sua paleta, contorno interno e losangos laterais. A Platina tem uma silhueta própria com ombros escalonados, cristais superiores, placas facetadas laterais e fecho inferior. Camadas de sombra azul-acinzentada e reflexos branco-azulados dão profundidade estática. A diferença de geometria continua visível quando bloqueada; texto de nível e estado complementa a cor.

`AchievementBadge.qml` desenha as camadas adicionais somente para `special`; `AchievementBadgePaths.js` altera apenas o material especial. Bronze, Prata, Ouro e os 30 desenhos centrais permanecem iguais. Não há neon, partículas, animação contínua ou dependências externas. As transições existentes obedecem redução de movimento.

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
python scripts/capture_home.py --binary build/achievements-refinements/projeto_glossata.exe
```

Testes/capturas usam identidades e bancos temporários, Discord desativado, QML e controlador reais. Os exemplos desbloqueados resultam de dez respostas corretas no Livre e uma conclusão competitiva integral sem dicas, além de uma consulta real. Nenhum modelo da interface é alterado para simular progresso.

## Comparações reais

As comparações abaixo mostram as badges no tamanho dos cartões. O [manifesto](capturas.json) registra dimensões lógicas/físicas, escala, horários reais, binários e SHA-256. Há exemplos em 1000×700, 1240×820 e 1920×1080, nas escalas 100%, 125% e 150%, incluindo filtros combinados, seleção vazia e retorno à tela.

| Material | Bloqueada — 1000×700/100% | Desbloqueada — 1000×700/100% |
| --- | --- | --- |
| Prata | [Cartões](capturas/100/filtered-locked-intermediate-1000x700.png) | [Cartões](capturas/100/filtered-unlocked-intermediate-1000x700.png) |
| Platina | [Cartões](capturas/100/filtered-locked-special-1000x700.png) | [Cartões](capturas/100/filtered-unlocked-special-1000x700.png) |

| Material | Detalhe bloqueado — 1240×820/150% | Detalhe desbloqueado — 1240×820/150% |
| --- | --- | --- |
| Prata | [Conhecedor](capturas/150/level-locked-intermediate-1240x820.png) | [Conhecedor](capturas/150/level-unlocked-intermediate-1240x820.png) |
| Platina | [Desafio perfeito](capturas/150/level-locked-special-1240x820.png) | [Desafio perfeito](capturas/150/level-unlocked-special-1240x820.png) |

[Três filtros combinados](capturas/100/combined-filters-1000x700.png), [seleção vazia](capturas/100/empty-filters-1000x700.png) e [seleção restaurada ao retornar](capturas/100/restored-filters-1000x700.png). A [captura oficial da Home](../../../../images/tela-inicial.png) foi renovada pelo programa após recompilar, e o [registro](../../../../images/tela-inicial.json) vincula o novo binário. Nenhum pixel foi modificado fora do programa; `Main.qml` permaneceu igual à base deste refinamento.

## Validação final

| Verificação | Resultado |
| --- | --- |
| CTest completo após os ajustes finais | **26/26**, zero falhas, 475,54 s |
| Regras/controlador | **86 casos** aprovados |
| Interface completa | **31 casos** aprovados |
| Identidade/migrações | **38 casos** aprovados |
| Recuperação | **21 casos** aprovados |
| Interface específica | Aprovada em 100%, 125% e 150% |
| Capturas definitivas | **132 imagens**, 44 por escala, nas três resoluções |
| qmllint, smoke e auditoria de repositório/código | Aprovados |
| Status e versão/histórico | Aprovados |
| Catálogo estrito e integridade histórica | Aprovados; 122 organismos e 1.053 eventos preservados |
| Preservação de escopo | Backend, definições/níveis, 30 símbolos e materiais Bronze/Prata/Ouro iguais à base `10aceb8` |
| Distribuição | Bloqueada por permissões/revisões; sem novos erros técnicos/dependências |

[Resultados estruturados e comandos](verificacoes/resultados.json), [CTest final](verificacoes/levels-final-ctest.txt), [regras](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt), [identidade](verificacoes/identity-results.txt) e [recuperação](verificacoes/recovery-results.txt).

O teste específico seleciona cada nível, verifica os IDs/contadores filtrados, combina os três critérios, alterna bloqueio/desbloqueio, confirma seleção vazia e conserva o resumo global. Em cada resolução, verifica retorno da navegação, marcação dos três filtros, teclado, mouse e ausência de corte nos botões. Mantém os testes de movimento reduzido, detalhes e foco. As comparações usam a renderização real dos cartões, sem ampliar ou redesenhar badges fora do programa.

Os relatórios iniciais continuam guardados. Eles identificaram o clique repetido que apagava a indicação selecionada e uma comparação de referências JavaScript no teste, substituída pela comparação dos valores do estado. Uma execução anterior de CTest foi interrompida para mover a inicialização de `level` para `AchievementsView`, preservando `Main.qml`. O relatório final acima corresponde à recompilação e à execução completa **após** essa organização, não à execução interrompida.

## Inventário sem limpeza

Executado somente após terminar builds/testes: [dry-run padrão](verificacoes/cleanup-dry-run.json), [builds antigos em dry-run](verificacoes/cleanup-old-builds-dry-run.json) e [inventário ampliado](verificacoes/inventario-local.json). A PR #38 continua aberta; não houve limpeza ou `--apply`.

| Caminho | Tamanho na varredura | Motivo e risco |
| --- | --- | --- |
| `build/achievements-refinements` | 1.913.892.326 bytes | Build validado e relatórios recentes; risco alto, preservar |
| `build/achievements-2-3-3` | 1.910.145.166 bytes | Build anterior e evidências; risco alto, preservar |
| `previews/achievements-levels-final` | 17.801.793 bytes | Originais das 132 capturas versionadas; risco médio para rastreabilidade |
| `build/achievements-refinements/tests/previews/achievements-levels-final` | 4.082.765 bytes | Capturas da suíte completa; risco médio, preservar |
| `scripts/__pycache__` | 1.363.308 bytes | Cache recompilável; risco baixo, apenas candidato a futura limpeza autorizada |
| `tests/__pycache__` | 585.884 bytes | Cache recompilável; risco baixo, apenas candidato a futura limpeza autorizada |

Nenhum arquivo foi excluído ou movido. Linhas aninhadas do inventário se sobrepõem; seus tamanhos não devem ser somados. Todas as evidências anteriores continuam versionadas nos diretórios próprios.

## Arquivos alterados

- `resources/qml/views/AchievementsView.qml`: inicialização e seleção em memória, predicado AND, botões compactos e mensagem vazia.
- `resources/qml/components/AchievementBadge.qml` e `AchievementBadgePaths.js`: camadas/estrutura exclusivas da Platina.
- `tests/QmlInterfaceTests.cpp`: opções, combinações, contadores, seleção vazia, navegação, teclado/mouse, escalas e capturas.
- Changelog, documentação de conquistas/componentes/design, status gerado, captura oficial e registro: rastreabilidade da mesma versão ainda não lançada.

## Limites

Capturas offscreen nas escalas Qt não certificam leitores de tela ou troca física entre monitores. Os níveis continuam escolhas estáticas de apresentação; não representam raridade estatística. O gate de distribuição conserva as pendências existentes de permissões/revisões e não é contornado por testes. O escopo de licenciamento dos desenhos continua pendente. Sem merge, tag, release ou notificações da 2.3.4.
