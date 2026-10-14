# Contexto do Projeto: Glossata Quiz

## Objetivo

Aplicativo desktop educativo em português para aprender sobre Glossata (borboletas e mariposas). Cada rodada mostra uma curiosidade, pode revelar uma dica adicional e oferece quatro nomes científicos para escolha por botão. Não há digitação da resposta. A partida continua até o jogador encerrá-la; a pontuação e o ranking são locais.

## Workspace e ambiente

- Versionamento local com Git, branch `main`. O commit inicial contém código, recursos, testes, documentação, configurações compartilhadas do editor e os arquivos fonte do launcher/`qt.conf`. Build, DLLs/executáveis empacotados, capturas geradas e bancos locais são ignorados. Ainda não há remoto configurado.
- Projeto canônico: `C:\glossata_game`.
- Abrir esta pasta como raiz do workspace no VS Code. Há uma cópia antiga em outra pasta do Desktop cujo caminho contém acentos, espaços e `&`; evitar compilar a cópia antiga porque o Qt autogen/moc teve problemas nesse caminho.
- Linguagem: C++17.
- GUI: Qt 6 Quick/QML.
- Persistência: SQLite via Qt Sql/QSQLITE.
- Build: CMake + Ninja, MSYS2 UCRT64.
- Ambiente validado nesta máquina: GCC 16.2.0, Qt 6.11.2.
- `.vscode/settings.json` faz o IntelliSense usar `${workspaceFolder}/build/compile_commands.json`; o CMake exporta esse arquivo.

## Arquitetura

- `src/domain/Species.h`: espécie com id, nome científico, curiosidade, dica, família, região e campo de imagem.
- `src/domain/Question.h`: prompt, curiosidade, dica extra, resposta correta, opções e metadados da espécie usada na pergunta.
- `src/domain/GameSession.h`: pontuação, total de respostas, número da pergunta e estado final.
- `src/application/GameService.*`: fluxo da partida, embaralhamento do catálogo, alternativas, dicas, pontuação e coordenação do salvamento.
- `src/application/ports/SpeciesCatalog.h`: abstração para carregar espécies.
- `src/application/ports/GameHistoryRepository.h`: abstração do ranking.
- `src/application/ports/PlayerProfileRepository.h`: operações de perfil.
- `src/infrastructure/content/JsonSpeciesCatalog.*`: lê o catálogo JSON embarcado no recurso Qt.
- `src/infrastructure/persistence/SqliteGameHistoryRepository.*`: ranking e perfis em SQLite; prepara diretórios, cria tabelas, nomeia conexões separadamente e fecha/remove a conexão no destrutor.
- `src/presentation/GameController.*`: QObject exposto ao QML como `gameController`.
- `resources/qml/Main.qml`: janela principal, menu lateral e navegação entre as telas.
- `resources/qml/views/`: início, categorias, pergunta, revisão da resposta, ficha da espécie, resultado, estatísticas, conquistas e perfis/ajustes.
- `resources/qml/components/`: botões, ícones, cartões de estatísticas, menu e ilustração decorativa em Canvas.
- `resources/data/species.json`: catálogo embarcado por `resources.qrc`.
- `tests/GameServiceTests.cpp`: testes automatizados de regras e persistência.
- `tests/QmlInterfaceTests.cpp`: teste do fluxo de navegação e confirmação de respostas com renderização das telas em dois tamanhos de janela.
- `src/infrastructure/discord/DiscordRichPresence.*`: presença opcional via RPC/IPC local, usando Qt Network/QLocalSocket; sem token de bot ou Client Secret.
- `resources/branding/`: PNG e ICO temporários derivados da arte fornecida pelo usuário. O ICO está embarcado como recurso nativo do executável Windows e como recurso Qt.
- Identidade atual importada de `C:\Users\Kadu\Desktop\glossata_icon_pack`: `glossata_logo.png` → `resources/branding/glossata.png`, `glossata_icon.ico` → `glossata.ico`, `glossata_discord.png` → `glossata_discord.png`. Os PNGs têm 1254×1254; o ICO inclui sete tamanhos. O recurso nativo Windows tem dependência explícita do ICO para recompilar quando a arte muda.
- `docs/DISCORD_SETUP.md`: instruções para cadastrar a imagem da atividade e testar a integração.

## Funcionalidades atuais

### Jogo

- A interface abre na tela inicial e usa tema escuro com acentos verdes, inspirado na referência visual fornecida. O estilo de controles Qt é `Basic`, definido antes de iniciar o aplicativo.
- O jogador escolhe o catálogo completo ou uma família com ao menos quatro espécies distintas. Atualmente há **Todas as espécies (22)**, **Nymphalidae (7)** e **Saturniidae (5)**. Perguntas e alternativas respeitam o grupo selecionado.
- Selecionar uma alternativa não responde à pergunta: é necessário pressionar **Confirmar**. Os atalhos 1–4 selecionam opções; Enter confirma quando a tela de pergunta tem foco.
- Após confirmar, uma tela de acerto/erro exibe a resposta correta e a curiosidade. **Ver mais informações** abre a ficha com nome científico, família, distribuição e dica. **Próxima pergunta** libera a rodada seguinte. O controller impede respostas extras enquanto essa revisão está pendente.
- **Pausar** e a navegação lateral preservam a partida. O botão **Jogar** retoma a revisão pendente ou a pergunta atual.
- Começa na pergunta 1.
- Funciona continuamente até o jogador pressionar **Encerrar partida**.
- Cada espécie aparece uma vez por ciclo; ao terminar o catálogo, ele é reembaralhado e evita repetir imediatamente a última espécie do ciclo anterior.
- A cada pergunta, gera quatro nomes científicos distintos, não vazios e incluindo a resposta correta. O catálogo precisa conter ao menos quatro espécies válidas.
- A resposta correta incrementa a pontuação. Respostas não fazem o jogo terminar automaticamente.
- Depois de encerrar a partida, novas respostas não são aceitas; a tela permite iniciar outra partida.
- A pontuação final é salva uma única vez quando a partida é encerrada. Nenhum resultado é salvo se nenhuma pergunta foi respondida.
- O nome do jogador é capturado no início da partida e exibido na tela. Selecionar, renomear ou excluir perfis não altera essa cópia; a seleção atual vale para a próxima partida.
- Falhas de gravação no SQLite são comunicadas por um aviso global com o botão **Tentar salvar**. Uma nova partida fica bloqueada enquanto o resultado está pendente. Fechar o aplicativo perde o resultado pendente; não há fila persistente de recuperação.

### Estatísticas e conquistas

- Estatísticas mostram perguntas respondidas, acertos, aproveitamento e desempenho por família da **partida atual ou recém-encerrada**. Os cartões não representam dados acumulados de todas as partidas.
- Há seis conquistas por partida: primeira resposta, cinco acertos, dez respostas, três acertos seguidos, vinte acertos e conclusão de um ciclo do grupo selecionado.
- Estatísticas e conquistas reiniciam quando uma nova partida começa; não são persistidas. O ranking continua persistido e aparece junto das estatísticas.
- A tela **Perfis e ajustes** mantém criação, seleção, renomeação e exclusão de perfis. Permite também desligar as transições entre telas para a sessão atual.

### Ícone temporário e Discord

- A arte fornecida pelo usuário aparece no cabeçalho da interface, no ícone da janela e no executável. O recurso nativo é gerado por `src/app/windows/app.rc.in`; o executável Windows agora usa o subsistema GUI, evitando uma janela de console ao iniciar.
- O launcher com ícone próprio é `package/Glossata Quiz.lnk`, gerado pelo empacotamento. Seu `IconLocation` aponta para `package/branding/glossata-<hash>.ico`; o nome baseado no conteúdo muda ao trocar a arte, e o script notifica o Explorer sobre o atalho atualizado. Cópias antigas/fixadas do atalho precisam ser substituídas. O launcher `.cmd` continua disponível; arquivos `.cmd` usam o ícone da associação de arquivos do Windows.
- `scripts/set-icon.ps1 -InputImage 'caminho da imagem'` gera PNG 1024×1024 e ICO com sete tamanhos, preservando proporção e transparência. Recompilar e reempacotar depois de trocar a arte.
- `resources/data/discord.json` contém `enabled`, `applicationId` e `largeImage`. O jogo prioriza `discord.json` ao lado do executável e usa o recurso embarcado como fallback. O empacotamento copia a configuração de origem.
- Configuração atual: Application ID `1559712085250801685`, asset **glossata_icon_v2**. O usuário confirmou que enviou a imagem nova com a chave anterior `glossata_icon`, mas o Discord ainda exibia a imagem antiga. Para separar as versões, a configuração agora usa uma chave nova: cadastrar `glossata_discord.png` em **Rich Presence → Art Assets** como `glossata_icon_v2`. O App Icon usa `glossata.png` em **General Information**. O envio no Portal é feito pelo usuário; a aparência final da nova chave depende desse cadastro.
- A presença publica somente nome do jogo, o texto **Explorando os cinco reinos biológicos**, o estado **Uma descoberta por vez** e tempo desde a abertura. A mensagem foi solicitada pelo usuário; o catálogo atual continua dedicado a Glossata. Pode ser desligada em **Perfis e ajustes** ou por `enabled: false`. Ao fechar, envia atividade nula. Sem Discord disponível, tenta novamente após 15 segundos; o jogo continua independente da conexão.
- `glossata_quiz.exe --discord-test` testa a conexão sem abrir a janela do jogo, grava `discord-test.log` ao lado do executável e termina em até 12 segundos. Código 0 indica confirmação de publicação pelo Discord.
- Teste real nesta máquina confirmou publicação e remoção da atividade fora do sandbox. O sandbox bloqueou o canal local com acesso negado; isso não era falha no código. O cadastro/exibição visual do asset ainda depende do upload no Portal.

### Dica extra

- Cada espécie contém uma curiosidade principal e uma dica adicional.
- **Revelar dica** pode ser usado uma vez por pergunta.
- A dica fica escondida antes da ação e o estado é reiniciado quando uma nova espécie aparece.

### Catálogo

- Há 22 registros no momento. Os nomes são:
  `Papilio machaon`, `Vanessa atalanta`, `Acherontia atropos`, `Brahmaea wallichii`, `Lymantria dispar`, `Danaus plexippus`, `Aglais io`, `Vanessa cardui`, `Bombyx mori`, `Attacus atlas`, `Actias luna`, `Hyalophora cecropia`, `Automeris io`, `Antheraea polyphemus`, `Heliconius melpomene`, `Papilio ulysses`, `Morpho peleides`, `Pieris brassicae`, `Galleria mellonella`, `Plodia interpunctella`, `Manduca sexta`, `Caligo memnon`.
- O schema por registro é `id`, `name`, `fact`, `hint`, `family`, `region`, `image`.
- A aplicação só carrega registros com nome, fato e dica não vazios.
- Nome, fato e dica são aparados; campos contendo apenas espaços são rejeitados. Arquivos ausentes ou JSON inválido geram diagnóstico. O serviço também valida esses campos e remove nomes duplicados ignorando caixa.
- Todas as propriedades `image` estão vazias. A UI usa uma ilustração genérica criada em QML/Canvas e marcada **Ilustração decorativa**; ela não é uma representação da espécie perguntada. Fotos identificadas ainda não foram adicionadas. Não incluir fotos sem identificar espécie, licença e atribuição.
- Os fatos foram escritos para o catálogo, mas ainda precisam de revisão com fontes taxonômicas/naturalistas confiáveis antes de uma release pública.

### Perfis

- Perfis de jogador são persistidos localmente na tabela `player_profiles` do SQLite.
- É possível listar/selecionar, criar, renomear e excluir perfis pela janela **Perfis**.
- Nomes são aparados, limite de 32 caracteres, e a chave SQLite é `COLLATE NOCASE`, evitando duplicatas sem diferença de caixa.
- A interface impede excluir o último perfil; uma instalação sem perfis cria `Jogador 1`.
- Atualmente personalização significa nome do perfil; não há avatar, preferências ou estatísticas individuais.
- Resultados antigos guardam uma cópia do nome utilizado na partida; renomear o perfil não reescreve entradas históricas.

### Ranking e dados locais

- Uma tabela `history` armazena nome do jogador no momento da partida, pontuação, acertos e horário UTC.
- O ranking mantém uma entrada por nome (`COLLATE NOCASE`). Ao encerrar outra partida com o mesmo nome, pontuação, acertos e data são substituídos pelo resultado mais recente, mesmo quando a pontuação diminui; os pontos não são acumulados.
- Bancos anteriores são migrados automaticamente em uma transação: entre entradas do mesmo nome, permanece a partida com maior `played_at`; em empate, o maior `id`. Um índice único impede novas duplicatas. A migração é identificada pela presença do índice `history_player_name_unique`.
- A interface carrega os cinco melhores resultados, ordenados pela pontuação e data.
- O banco `glossata_history.sqlite` fica na localização de dados de aplicativo definida por `QStandardPaths::AppDataLocation`, não dentro do pacote.
- Ao expandir histórico/perfis, manter migração/compatibilidade de schema em mente.

## Build, teste e execução

No PowerShell, a partir de `C:\glossata_game`:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
$qmlPaths = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
qmllint @qmlPaths --unqualified disable
```

`--unqualified disable` desativa somente o aviso do `qmllint` para `gameController`, que é intencionalmente injetado pelo C++ como context property. O QML lint não apontou avisos de sintaxe/layout/delegates com essa opção.

Executável empacotado:

- `package/glossata_quiz.exe`
- Launcher recomendado com ícone: `package/Glossata Quiz.lnk`; alternativa: `package/run_glossata_game.cmd`.
- O pacote inclui Qt DLLs/plugins/QML e `qt.conf`.
- Para atualizar o pacote após compilar: `powershell -ExecutionPolicy Bypass -File scripts/package.ps1`. O script roda `windeployqt6` e copia também as dependências MSYS2 não Qt identificadas nos imports dos binários, preservando o launcher e `qt.conf` existentes.
- O pacote inclui somente o driver SQL usado pelo jogo (`QSQLITE`); drivers de Firebird, MariaDB, ODBC e PostgreSQL são excluídos.
- `windeployqt6` mostrou avisos sobre catálogos de traduções ausentes do pacote MSYS2 e `dxcompiler.dll`/`dxil.dll`; o aviso do compilador D3D12 é irrelevante se esses recursos não forem usados. Verificar distribuição numa máquina sem o ambiente MSYS2 antes de declarar o instalador standalone definitivo.

## Testes atuais

O executável `glossata_tests` contém dezoito testes de comportamento (mais inicialização/limpeza Qt, total observado: 20 PASS):

1. Catálogo tem pelo menos 20 espécies, cada uma com nome, fato e dica.
2. Nova partida começa na pergunta 1.
3. Dica é revelada uma vez e reseta na rodada seguinte.
4. Um ciclo tem quatro escolhas válidas e distintas por pergunta; as espécies não repetem no mesmo ciclo; partida só termina ao chamar `finishGame()`.
5. Resultado encerrado é salvo uma única vez em banco temporário.
6. Perfis persistem; nomes duplicados ignorando caixa são rejeitados; renomeação e exclusão funcionam.
7. Catálogos insuficientes, duplicados ou com campos obrigatórios vazios não iniciam uma partida; catálogo ausente também é tratado.
8. O carregador rejeita arquivo ausente, JSON inválido, raiz sem array e registros com dica contendo somente espaços.
9. Resposta inválida mantém a rodada; alternativa incorreta avança sem pontuar.
10. Partida sem respostas não chama o salvamento.
11. Falha ao salvar pode ser repetida, sem duplicar o resultado após sucesso.
12. Troca, criação, renomeação e exclusão de perfis preservam o nome capturado no início da partida.
13. Falha real de inserção SQLite mantém o resultado pendente no controller, bloqueia nova partida e permite recuperação sem duplicação.
14. Nova partida substitui o resultado do mesmo nome, inclusive com pontuação menor e diferenças de caixa; outros jogadores são preservados e o resultado persiste após reabrir o banco.
15. Migração de banco antigo consolida nomes repetidos pela data mais recente, usando o id para desempatar, e permite atualizações posteriores.
16. Falha real de atualização SQLite preserva a pontuação e data anteriores; nova tentativa substitui o resultado sem duplicá-lo.
17. Perguntas, alternativas e ciclos de uma categoria usam somente a família selecionada; categoria inexistente não inicia uma partida.
18. O controller mantém os metadados da resposta na revisão, bloqueia confirmação duplicada e atualiza estatísticas/conquistas da partida.

O executável `glossata_ui_tests` testa seleção e confirmação por clique, revisão correta/incorreta, navegação e renderização das nove telas em 1240×820 e 1000×700, além de estatísticas com todas as famílias e ranking completo. O teste rejeita avisos QML em execução. Usa `QT_QPA_PLATFORM=offscreen` e `QT_QUICK_BACKEND=software`; as capturas ficam em `build/tests/previews/` e o relatório em `build/tests/ui-test-results.txt` quando executado pelo CTest.

`glossata_discord_tests` contém três testes de comportamento: handshake e envio do asset com frames fragmentados, resposta PONG, confirmação/erro e remoção de atividade; ausência de Discord sem bloquear eventos; Application ID inválido sem conexão. Servidores locais exclusivos evitam publicar presença real durante os testes. O teste de interface também carrega o ICO embarcado.

Última verificação: build Release concluída; `ctest` passou (`3/3`); lint de todos os arquivos QML passou. Os testes usam bancos temporários e canais IPC isolados. As capturas foram inspecionadas; resta conferir visualmente o asset depois de cadastrado no Discord e testar o pacote em um Windows sem MSYS2.

## Decisões e histórico de problemas

- PostgreSQL foi descartado: o requisito é ranking local e SQLite não precisa de servidor.
- Qt Quick/QML foi escolhido para interface, com regras em C++.
- O caminho original do Desktop tinha acentos e `&`; em MSYS2/Qt isso provocou falhas do moc. `C:\glossata_game` é o caminho de trabalho validado.
- O IntelliSense antes não encontrava headers como `QString`/`QStandardPaths`. Foi resolvido ao gerar `compile_commands.json` e configurar `.vscode/settings.json`.
- O pacote já havia fechado ao abrir por falta dos módulos QML; foi regenerado com `windeployqt6`, `qt.conf` e launcher.
- Uma alternativa vazia e o contador começando em 0 foram corrigidos na lógica do jogo e cobertos por testes.
- O desligamento do banco SQLite foi corrigido para evitar arquivos de teste bloqueados.
- Foram removidos os arquivos vazios em `src/presentetion/`, o `DatabaseInitializer.cpp` vazio e as telas/componentes QML antigos que não eram usados pelo `Main.qml`.
- Novas telas e componentes em `resources/qml/views/` e `components/` foram implementados para o refinamento visual; são usados pelo `Main.qml` e embarcados no recurso Qt.
- A porta de histórico retorna sucesso/falha ao salvar. O serviço só marca o resultado como salvo após sucesso da gravação. A tabela mantém suas colunas originais; o ranking agora usa um índice único por nome e gravação com atualização em caso de conflito.

## Próximas prioridades

1. Revisar as 22 curiosidades e dicas contra fontes confiáveis e registrar fontes/atribuições.
2. Adicionar fotos de espécies com licença e atribuição claras, exibir foto na rodada e documentar crédito por arquivo.
3. Se necessário, melhorar migrações e apresentação do ranking (estatísticas/filtros); ranking global local já existe.
4. Considerar avatar e outras opções de personalização apenas se forem desejadas; perfis nomeáveis já existem.
5. Fazer um teste visual manual do pacote e testar execução em um Windows sem Qt/MSYS2 instalado.

## Instrução de retomada para outro chat

Usar `C:\glossata_game` como raiz e conferir o conteúdo atual dos arquivos antes de editar. Preservar C++17, Qt 6/QML, SQLite, arquitetura por portas e adaptadores e os testes existentes. Não editar a cópia antiga do Desktop. Ao alterar regras/catálogo, adicionar teste de comportamento, compilar, rodar CTest e QML lint; ao mudar o executável, reempacotar e manter o launcher.
