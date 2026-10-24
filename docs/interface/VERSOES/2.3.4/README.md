# Subetapa 2.3.4 — notificações de conquistas

**Atualização final da PR #39:** [modo compacto contextual no Quiz, capturas e verificações atuais](quiz-compacto/README.md). O [reposicionamento superior direito](topo-direito/README.md) e a implementação inicial permanecem documentados como histórico. A revisão conserva a versão 2.2.0 ainda não lançada e o requisito de efeitos sonoros configuráveis para a modernização futura de Perfil e ajustes, sem implementar áudio.

Base: `9297b00e35d406888fb548284c17052881f8b6d9`, integração da PR #38 na `main`, conferida pela API do GitHub e por `git fetch origin main`. Branch `feat/achievement-notifications-2-3-4`. Aplicativo **2.2.0 ainda não lançado**: MINOR por funcionalidade compatível. Catálogo **2.4.0**, SQLite **8** e critérios/badges da 2.3.3 preservados.

## Integração

A análise inicial conferiu `GameController`, `AchievementStorage`, `AchievementBadge`, metadados dos quatro níveis, `Main`, animações e design system antes de editar a implementação. `AchievementStorage::refreshAchievements` só entrega eventos novos após confirmar a transação; reconhecimentos retroativos não entram nesses eventos. `GameController::refreshAchievements` emite `achievementUnlocked` apenas com esse resultado confirmado. Esse código permanece inalterado.

`Main.qml` instancia uma única `AchievementNotificationLayer`, irmã do layout principal, independente do Loader/rolagem das páginas. `Connections` consome somente `achievementUnlocked`; não observa mudanças da lista para detectar conquistas. Nenhuma página inicia notificações ao ser carregada.

Antes de enfileirar, a camada exige os campos reais `id`, `title`, `description`, `badge`, `visualLevel`, `visualLevelName`, `playerId` e `unlockedAt`, strings não vazias, data válida, `unlocked: true`, `retrospective: false`, material/glifo existentes e identidade local ativa correspondente. Não inventa títulos, níveis ou dados faltantes. O evento é copiado para a fila de apresentação; não grava outra preferência ou tabela no SQLite.

## Fila e ciclo de vida

- FIFO com até **duas** instâncias, sendo no máximo **uma visível no Quiz**, em modo compacto. A segunda instância existente permanece retida com leitura pausada; eventos adicionais aguardam na mesma fila. Fora do Quiz, até duas aparecem no desenho completo. `ListModel` acrescenta/remove as instâncias individualmente; a troca de contexto preserva eventos, tempo restante e a primeira instância.
- Deduplicação por **UUID local + ID da conquista**, durante a sessão da interface. A data é validada e conservada; uma variação de data não permite repetir a mesma conquista para o mesmo UUID.
- Cinco segundos de leitura após a entrada. Dispensa manual ou expiração conclui a saída antes de liberar a instância e apresentar a próxima. Dispensar altera somente a apresentação.
- Popups visíveis suspendem a apresentação, inclusive menus e diálogos comuns. Isso é deliberadamente conservador para proteger diálogos críticos de identidade/salvamento. A camada fica abaixo do overlay, sem criar Popup, janela nativa ou dimmer próprio. Erros de identidade, checkpoint, conquistas ou resultado pendente também suspendem a apresentação.
- Minimização, ocultação ou perda de atividade oculta as notificações e pausa a leitura. A aplicação nunca solicita ativação ou primeiro plano por causa de uma conquista. Ao retornar, a leitura restante continua; não há novo deslizamento em cartões já apresentados.
- Durante suspensão, eventos com **60 segundos desde o recebimento** expiram explicitamente, tanto ativos como pendentes. A checagem também ocorre ao retomar, pois timers Qt podem ficar suspensos pelo sistema. Durante apresentação normal a fila não descarta eventos por idade. Eventos recentes recebidos durante inatividade continuam elegíveis; eventos antigos não reaparecem depois de ausência longa.
- Invalidade, duplicação, expiração e troca de identidade/backend emitem `notificationDiscarded(id, reason)`, com contador e último motivo disponíveis para diagnóstico. Conclusões emitem `notificationCompleted`; apresentação emite `notificationPresented`. Essas informações não alteram dados persistentes nem criam ruído no fluxo do usuário.
- Uma troca de identidade/backend limpa a apresentação; UUIDs diferentes não compartilham eventos. Reiniciar o programa ou recriar o shell não reconstrói notificações antigas. A persistência de conquistas segue como autoridade.

## Componente e interação

`AchievementNotification.qml` reutiliza `AchievementBadge` integralmente. A superfície, tipografia, espaçamentos e raios usam `Theme`; a borda usa o material existente. Ouro ganha acento metálico e Platina uma borda mais definida e reflexo azul-claro curto durante a entrada. Não há som, recurso externo, dependência nova, partícula ou animação contínua.

Entrada de **280 ms**, curva `OutCubic`, deslocamento de **26 px** e opacidade. Saída de **200 ms**, curva `InCubic`. Desligar `Theme.motionEnabled` conclui as transições imediatamente, remove deslocamento/reflexo e conserva o tempo de leitura. Interromper a entrada por suspensão não destrói o cartão; a saída já solicitada termina sem iniciar outra enquanto a apresentação estiver bloqueada.

A coluna fica no canto superior direito, com margem de 20 px a partir da borda inferior real do cabeçalho, 20 px à direita, espaçamento de 10 px e largura de até 440 px, limitada pela janela. Títulos e nomes de nível usam quebra de linha; a badge mantém proporções. O layout da página e a rolagem não se movem para acomodar a coluna. [Geometria e evidências atualizadas](topo-direito/README.md).

Não há foco automático, interceptação de teclas, clique no corpo ou navegação para Conquistas. Só o botão de dispensa recebe interação. O clique nele não solicita foco; Tab permite alcançá-lo explicitamente. Se o usuário tiver levado o foco para uma notificação, sua conclusão devolve o foco ao último controle disponível da atividade, sem interferir em um popup ou janela inativa. Textos e descrição têm nomes acessíveis, complementando material e cor.

Referências primárias: [overlay e popups Qt](https://doc.qt.io/qt-6/qml-qtquick-controls-overlay.html) e [metadados acessíveis Qt Quick](https://doc.qt.io/qt-6/qml-qtquick-accessible.html). A implementação usa a API existente do projeto, sem depender de anúncios acessíveis introduzidos em versões mais recentes do Qt.

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
build/achievements-refinements/tests/glossata_ui_tests.exe achievementNotificationQueueAndLifecycle achievementNotificationQuizFocusAndPersistence achievementNotificationVisualsAndCaptures
python scripts/capture_home.py --binary build/achievements-refinements/projeto_glossata.exe
```

Os bancos/identidades são temporários e o Discord está desativado. Capturas usam o controlador/QML reais, respostas competitivas efetivamente confirmadas e desbloqueios persistidos; não substituem o modelo nem fabricam badges. Eventos negativos do teste reutilizam um evento observado para verificar duplicação, campos ausentes, retroatividade e identidade incorreta; não são usados nas evidências visuais. O teste de expiração avança somente o argumento da verificação da fila, sem mudar relógio do banco ou do sistema.

## Validação e evidências da implementação inicial

| Verificação | Resultado |
| --- | --- |
| CTest completo após os ajustes finais | **26/26**, zero falhas, 486.39 s |
| Regras/controlador | **86 casos** aprovados |
| Interface completa | **34 casos** aprovados, incluindo três novos de notificações |
| Identidade/migrações | **38 casos** aprovados |
| Recuperação | **21 casos** aprovados |
| Cenários específicos de notificações | Aprovados em **100%, 125% e 150%** |
| Capturas reais | **126 imagens**, 42 por escala, nas três resoluções |
| qmllint, smoke, auditoria de repositório/código, status e versão | Aprovados |
| Histórico/catálogo estrito | Aprovados; 122 organismos e 1.053 eventos de governança preservados |
| Distribuição | Pendências existentes de permissões/revisões; zero erros técnicos/dependências |

[Resultados e comandos](verificacoes/resultados.json), [CTest completo](verificacoes/notifications-ctest.txt), [interface](verificacoes/ui-test-results.txt), [regras](verificacoes/core-results.txt), [identidade](verificacoes/identity-results.txt), [recuperação](verificacoes/recovery-results.txt) e [resumo do gate de distribuição](verificacoes/distribution-summary.json). A [auditoria staged final](verificacoes/notifications-final-staged-audit.txt) e o [gate de histórico/captura](verificacoes/notifications-final-history.txt) também passaram.

O cenário de fila confirma desbloqueio real/simultâneo, ordem FIFO, máximo de duas instâncias, evento recebido durante saída, dispensa manual, cinco segundos de leitura, suspensão por modal/minimização, expiração explícita, conservação do banco e ausência de reapresentação ao navegar/reiniciar. Recusa duplicata, campo ausente, retroatividade, identidade incorreta, data e material inválidos. O cenário do Quiz provoca rollback de conquistas por trigger em banco temporário e verifica zero eventos/notificações; após recuperação, compara seleção, foco, pontos, dicas, rodada, métricas, foto e checkpoint antes/depois da notificação. Teclas numéricas/Enter continuam selecionando e confirmando respostas; dispensa por teclado devolve foco à atividade.

Os três testes específicos passaram em [100%](verificacoes/notifications-ui-1.txt), [125%](verificacoes/notifications-ui-1.25.txt) e [150%](verificacoes/notifications-ui-1.5.txt). O teste visual percorre 1000×700, 1240×820 e 1920×1080, verifica limites/não sobreposição, nomes sem corte e os quatro materiais. As oito telas solicitadas aparecem nas evidências: Quiz antes de concluir a partida, demais telas com notificações da conclusão real. O resumo/fluxo das páginas não é substituído para produzir imagens.

Os relatórios iniciais também foram preservados. Identificaram a precondição de selecionar o organismo antes de registrar uma consulta e cliques anteriores à conclusão do layout no teste. A automação final aguarda a geometria real e confirma o início da saída manual. Os resultados completos acima correspondem à recompilação e execução após esses ajustes.

### Capturas

| Material | 1000×700 / 100% | 1240×820 / 125% | 1920×1080 / 150% |
| --- | --- | --- | --- |
| Bronze | [Inicial](capturas/100/level-initial-1000x700.png) | [Inicial](capturas/125/level-initial-1240x820.png) | [Inicial](capturas/150/level-initial-1920x1080.png) |
| Prata | [Intermediário](capturas/100/level-intermediate-1000x700.png) | [Intermediário](capturas/125/level-intermediate-1240x820.png) | [Intermediário](capturas/150/level-intermediate-1920x1080.png) |
| Ouro | [Avançado](capturas/100/level-advanced-1000x700.png) | [Avançado](capturas/125/level-advanced-1240x820.png) | [Avançado](capturas/150/level-advanced-1920x1080.png) |
| Platina | [Especial](capturas/100/level-special-1000x700.png) | [Especial](capturas/125/level-special-1240x820.png) | [Especial](capturas/150/level-special-1920x1080.png) |

[Duas sobre o Quiz compacto](capturas/100/two-on-quiz-1000x700.png), [quadro durante entrada](capturas/125/entering-1240x820.png) e [suspensão por modal](capturas/150/modal-suspended-1920x1080.png). Exemplos nas demais telas: [Início](capturas/100/page-home-1240x820.png), [Explorar](capturas/100/page-explore-1240x820.png), [Jogar](capturas/100/page-categories-1240x820.png), [Resultados](capturas/100/page-results-1240x820.png), [Estatísticas](capturas/100/page-statistics-1240x820.png), [Conquistas](capturas/100/page-achievements-1240x820.png) e [Perfil e ajustes](capturas/100/page-settings-1240x820.png).

O [manifesto inicial](capturas.json) registra datas reais, dimensões lógicas/físicas, escalas, hashes das imagens/fontes/binários e o commit da implementação inicial. A captura oficial dessa implementação permanece no [histórico do commit `f6d8f80`](https://github.com/kaduolliver/Projeto-Glossata/blob/f6d8f809e742f0308ea41eef996e84fd2e7f2c26/docs/images/tela-inicial.png). A [captura oficial atual da Home](../../../images/tela-inicial.png) e seu [registro](../../../images/tela-inicial.json) acompanham o [refinamento compacto](quiz-compacto/README.md), no enquadramento inicial da Home. A rodada de reposicionamento permanece no [commit `6cd7e7a`](https://github.com/kaduolliver/Projeto-Glossata/blob/6cd7e7a8216b10754a1618373b84f57af44ab72e/docs/images/tela-inicial.png). Todas as rodadas usam `scripts/capture_home.py`, perfil temporário e o commit da respectiva alteração visual. As notificações são demonstradas nas capturas de teste; nenhuma foi fabricada na captura oficial. Todas as evidências anteriores da 2.3.3 permanecem preservadas.

## Arquivos

- Criados: `resources/qml/components/AchievementNotification.qml` e `AchievementNotificationLayer.qml`.
- Integração: `resources/qml/Main.qml` e `resources.qrc`.
- Testes: três cenários específicos em `tests/QmlInterfaceTests.cpp`.
- Metadados: `VERSION.txt`, `CHANGELOG.md`, `docs/CATALOGO_CHANGELOG.md`, `docs/ESCOPO_LICENCIAMENTO.json`, `docs/PROJECT_STATUS.json` e os blocos derivados em `README.md`/`PROJECT_CONTEXT.md`.
- Documentação: `docs/CONQUISTAS.md`, `docs/interface/CHANGELOG_UI.md`, `COMPONENTES.md`, `DESIGN_SYSTEM.md` e este registro com capturas/relatórios. `.gitattributes` conserva LF nos novos relatórios textuais.
- Captura oficial: `docs/images/tela-inicial.png` e `docs/images/tela-inicial.json`, no mesmo commit do shell.

## Inventário sem limpeza

Inventário executado somente após encerrar builds/testes: [dry-run padrão](verificacoes/cleanup-dry-run.json), [builds antigos em dry-run](verificacoes/cleanup-old-builds-dry-run.json) e [inventário ampliado](verificacoes/inventario-local.json). Nenhum arquivo foi excluído ou movido; não foi usado `--apply`.

| Caminho | Tamanho na varredura | Motivo e risco |
| --- | --- | --- |
| `build/achievements-refinements` | 1.928.879.080 bytes | Build e evidências recentes/anteriores; risco alto, preservar |
| `build/achievements-2-3-3` | 1.910.145.166 bytes | Build e evidências recentes/anteriores; risco alto, preservar |
| `previews/achievement-notifications` | 36.916.834 bytes | Capturas locais, originais das evidências; risco médio de rastreabilidade, preservar |
| `build/achievements-refinements/tests/previews/achievement-notifications` | 8.314.213 bytes | Capturas locais, originais das evidências; risco médio de rastreabilidade, preservar |
| `scripts/__pycache__` | 1.363.308 bytes | Cache recompilável; risco baixo, candidato apenas a limpeza futura autorizada |
| `tests/__pycache__` | 585.884 bytes | Cache recompilável; risco baixo, candidato apenas a limpeza futura autorizada |

Entradas aninhadas se sobrepõem; os tamanhos não devem ser somados. Hashes iguais/duplicação de capturas não autorizam exclusão. Recursos, dados pessoais, SQLite, builds recentes, evidências e histórico foram preservados.

## Limites

Capturas offscreen verificam o quadro renderizado e as escalas Qt; não certificam fluidez física, comportamento de todos os leitores de tela ou troca de monitores. Não há integração Steam/Windows, sons, conta online ou sincronização. Os componentes novos permanecem pendentes de decisão explícita de licença; testes não concedem revisão científica, licença ou autorização de distribuição. Sem merge, tag, release ou avanço a outra subetapa.
