# Subetapa 2.3.4 — notificações de conquistas

Base: `9297b00e35d406888fb548284c17052881f8b6d9`, integração da PR #38 na `main`, conferida pela API do GitHub e por `git fetch origin main`. Branch `feat/achievement-notifications-2-3-4`. Aplicativo **2.2.0 ainda não lançado**: MINOR por funcionalidade compatível. Catálogo **2.4.0**, SQLite **8** e critérios/badges da 2.3.3 preservados.

## Integração

A análise inicial conferiu `GameController`, `AchievementStorage`, `AchievementBadge`, metadados dos quatro níveis, `Main`, animações e design system antes de editar a implementação. `AchievementStorage::refreshAchievements` só entrega eventos novos após confirmar a transação; reconhecimentos retroativos não entram nesses eventos. `GameController::refreshAchievements` emite `achievementUnlocked` apenas com esse resultado confirmado. Esse código permanece inalterado.

`Main.qml` instancia uma única `AchievementNotificationLayer`, irmã do layout principal, independente do Loader/rolagem das páginas. `Connections` consome somente `achievementUnlocked`; não observa mudanças da lista para detectar conquistas. Nenhuma página inicia notificações ao ser carregada.

Antes de enfileirar, a camada exige os campos reais `id`, `title`, `description`, `badge`, `visualLevel`, `visualLevelName`, `playerId` e `unlockedAt`, strings não vazias, data válida, `unlocked: true`, `retrospective: false`, material/glifo existentes e identidade local ativa correspondente. Não inventa títulos, níveis ou dados faltantes. O evento é copiado para a fila de apresentação; não grava outra preferência ou tabela no SQLite.

## Fila e ciclo de vida

- FIFO com até **duas** notificações simultâneas. Eventos adicionais aguardam sem substituir os já apresentados. `ListModel` acrescenta/remove as instâncias individualmente; um novo evento não reinicia o tempo da outra notificação.
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

A coluna fica no canto inferior direito, com margem de 20 px, espaçamento de 10 px e largura de até 440 px, limitada pela janela. Títulos e nomes de nível usam quebra de linha; a badge mantém proporções. O layout da página e a rolagem não se movem para acomodar a coluna.

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

## Validação e evidências

Resultados e comparações serão consolidados após a execução final.

## Arquivos

- Criados: `resources/qml/components/AchievementNotification.qml` e `AchievementNotificationLayer.qml`.
- Integração: `resources/qml/Main.qml` e `resources.qrc`.
- Testes: três cenários específicos em `tests/QmlInterfaceTests.cpp`.
- Versão, changelogs, status derivado, design/componentes, escopo pendente e captura oficial: rastreabilidade da versão não lançada.

## Inventário sem limpeza

O inventário dry-run será registrado após terminar builds e testes. Nenhuma exclusão ou movimentação será feita nesta tarefa.

## Limites

Capturas offscreen verificam o quadro renderizado e as escalas Qt; não certificam fluidez física, comportamento de todos os leitores de tela ou troca de monitores. Não há integração Steam/Windows, sons, conta online ou sincronização. Os componentes novos permanecem pendentes de decisão explícita de licença; testes não concedem revisão científica, licença ou autorização de distribuição. Sem merge, tag, release ou avanço a outra subetapa.
