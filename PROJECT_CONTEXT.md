# Contexto do Projeto: Glossata Quiz

## Objetivo e estado atual

Aplicativo desktop educativo em português para explorar Animalia, Plantae, Fungi, Protista e Monera no modelo didático tradicional dos cinco reinos. Cada pergunta tem quatro alternativas do mesmo nível taxonômico, seleção e confirmação, uma dica opcional e ficha da espécie. A partida continua até o jogador encerrá-la.

O catálogo ativo possui 122 espécies, com 42 Animalia e 20 em cada um dos outros reinos. Após autorização do usuário, 26 organismos sem cobertura visual foram substituídos; os dados anteriores foram arquivados. Monera mantém os mesmos 20 organismos, com imagens de apoio para identificação por pistas. Consulte [docs/SUBSTITUICOES_CATALOGO.md](docs/SUBSTITUICOES_CATALOGO.md) para o estado atual e [docs/PHOTO_CREDITS.md](docs/PHOTO_CREDITS.md) para créditos.

## Workspace e ambiente

- Raiz canônica: C:\glossata_game. Não editar a cópia antiga do Desktop, cujo caminho com acentos e & já provocou problemas de moc/autogen.
- Git: main integra a expansão dos cinco reinos; feat/refinamento-identificacao contém esta etapa de refinamento. origin aponta para https://github.com/kaduolliver/Das-Glossata-Projekt.
- Código, recursos, testes, documentação e configurações compartilhadas são versionados. Build, pacote gerado, capturas, cache e bancos pessoais são ignorados.
- C++17, Qt 6 Quick/QML, Qt Sql/QSQLITE, CMake/Ninja, MSYS2 UCRT64.
- Ambiente validado: Qt 6.11.2 e GCC 16.2.0. Importação/testes auxiliares usam Python 3.11+ e sua biblioteca padrão.
- IntelliSense usa build/compile_commands.json.

## Arquitetura

- Domínio: Species preserva os campos originais e acrescenta linhagem, reino didático, origem e Photo. Question vincula a fotografia e a referência da dica; GameSession guarda pontuação e estado. TaxonomicPerformance agrega respostas por IDs de reino e filo/divisão.
- QuizSelection centraliza elegibilidade e distratores. GameService filtra descendentes/família, embaralha, intercala reinos, controla dicas e salva resultados.
- TaxonomyIndex mantém árvore, contagens, caminhos e níveis disponíveis em memória. Navegar não relê arquivos nem acessa a API.
- As portas SpeciesCatalog, GameHistoryRepository e PlayerProfileRepository mantêm a separação entre aplicação e infraestrutura.
- JsonSpeciesCatalog lê e valida recursos Qt. SqliteSpeciesCatalog prepara o catálogo normalizado, aplica migrações aditivas e carrega o cache.
- SqliteGameHistoryRepository mantém ranking/perfis, com conexões exclusivas e fechamento correto.
- GameController expõe o estado ao QML como gameController e preserva a pergunta respondida durante a revisão.
- Main.qml mantém sidebar, Loader, rolagem e transições. views contém Home, categorias, TaxonomyView, quiz, feedback, informações, resultado, estatísticas, conquistas e ajustes.
- PhotoResolver centraliza associação, fonte verificada, licença, autoria, SHA-256, existência, decodificação e resolução mínima de 300 pixels em ambas as dimensões. Aprovação por nível fica em photo_reviews.json e é vinculada ao ID da foto; metadados e aprovação são persistidos em photo_json no catálogo SQLite.
- EducationalContent lê education.json: introduções por ID e dicas/curiosidades validadas por espécie ou grupo. Rejeita pistas que contenham alternativas, gênero ou nomes da linhagem; não usa fatos legados não revisados.
- ExploreCard distingue cadastro de cobertura visual; SpeciesPortrait exibe fotos e créditos sem substituição decorativa no quiz. TextAction e TaxonComboBox compartilham os estados arredondados, escuros e com foco acessível; EndGameDialog confirma o encerramento.
- resources/data contém species.json, taxonomy.json, catalog_replacements.json, photo_reviews.json, photo_audit.json, education.json, import_report.json e discord.json. Há 120 arquivos de imagem para os 122 registros ativos; candidatos não selecionados não entram no executável. role=supporting e supporting_for vinculam uma imagem de apoio a uma questão de Monera, preservando o táxon original do registro fotográfico.
- scripts/import_inaturalist.py importa, verifica licenças individuais, deduplica e mantém cache local.

## Fluxos e regras

Home → Jogar → Todas as espécies ou reino → grupos/subgrupos → nível de identificação → quiz → feedback → ficha → próxima pergunta ou resultado → retorno à exploração.

Grupos são gerados pelos dados e podem ter profundidades e ranks diferentes. O caminho clicável retorna aos ancestrais. Grupos pequenos e taxonomias parciais continuam navegáveis; quizzes sem quatro respostas distintas são desabilitados. Níveis possíveis: espécie, gênero, família, ordem, classe, filo/divisão e reino científico.

Todas as alternativas pertencem ao grupo escolhido e ao nível da pergunta. O modo misto intercala reinos proporcionalmente ao tamanho dos grupos e embaralha internamente. Espécies não repetem dentro de um ciclo nem imediatamente entre ciclos. Resposta inválida não altera a rodada. A dica pode ser usada uma vez por pergunta.

Responder prepara a próxima pergunta no serviço; o controller mantém o snapshot da pergunta, foto, dica e alternativas até nextQuestion e bloqueia confirmações duplicadas. Voltar abre uma confirmação modal. Continuar preserva a seleção; confirmar registra uma vez e retorna ao grupo anterior ou aos reinos. Falha de gravação leva ao resultado pendente para nova tentativa.

Todos os 122 registros ativos possuem imagem tecnicamente válida: 102 têm fotos para identificação visual e os 20 de Monera usam imagens de apoio com aviso explícito, conforme solicitação do usuário. O quiz de Monera utiliza classificação registrada como pista; descrições biológicas específicas serão ampliadas na próxima etapa. O modo misto inicia em gênero, com 101 registros dos cinco reinos. Há 16 introduções para táxons ainda ativos e duas revisões educativas específicas de espécie. Níveis sem quatro alternativas distintas continuam bloqueados. Registros substituídos recebem novos IDs e os antigos ficam arquivados em catalog_replacements.json.

Desempenho taxonômico agrega por reino didático e ID de filo/divisão da fonte, independentemente do nível perguntado. Um filo ausente ou ambíguo usa Classificação não disponível. Estatísticas e seis conquistas permanecem relativas à partida atual/última partida, sem progresso cumulativo persistido.

## Dados locais e migrações

Dois bancos ficam em QStandardPaths::AppDataLocation, fora do pacote:

- glossata_history.sqlite mantém history e player_profiles no caminho e schema anteriores.
- glossata_catalog.sqlite mantém taxons, espécies, origem, créditos e hash do conteúdo embarcado.

O catálogo usa chaves estrangeiras, índices e consultas recursivas. Carga/atualização de conteúdo são transacionais, com upsert e pais antes dos filhos. Colunas são acrescentadas sem apagar tabelas ou registros anteriores. A coluna active marca somente substituições explícitas como inativas, verificando ID/nome anterior e existência do substituto. Consultas e contagens usam registros ativos; dados locais não relacionados permanecem intactos. Reabertura carrega o SQLite uma vez para memória. Se a preparação falhar, o adaptador conserva o conteúdo embarcado como fallback.

Perfis têm nomes aparados, limite de 32 caracteres e unicidade COLLATE NOCASE. É possível criar, selecionar, renomear e excluir; o último perfil não pode ser excluído. Instalação vazia cria Jogador 1. A partida captura o nome ao iniciar; mudanças de perfil durante o jogo não transferem seu resultado.

O ranking mantém o resultado mais recente por nome, ignorando caixa, mesmo com pontuação menor. Não acumula pontos. Bancos antigos são migrados por transação, consolidando duplicatas pelo maior played_at e depois id. A UI mostra os cinco melhores. Falha ao salvar mantém resultado pendente, permite repetir e bloqueia nova partida até sucesso.

## Discord e ícones

DiscordRichPresence usa RPC/IPC local com Qt Network/QLocalSocket, sem token de bot ou Client Secret. Configuração em resources/data/discord.json; package/discord.json pode substituir a embarcada. Application ID: 1559712085250801685; asset: glossata_icon_v2.

A mensagem permanece Explorando os cinco reinos biológicos e Uma descoberta por vez, sem nomes ou pontuações. Ausência de Discord não bloqueia o jogo; a presença pode ser desativada em ajustes.

resources/branding contém PNG, ICO multirresolução e arte para Discord fornecidos pelo usuário. ICO é recurso nativo do executável e ícone global da janela. Launcher recomendado: package/Glossata Quiz.lnk; alternativa: package/run_glossata_game.cmd. O pacote usa ICO com nome derivado de hash e atualiza o cache do Windows. Assets do Discord são administrados separadamente no Developer Portal.

## Build, testes e pacote

No PowerShell, a partir da raiz:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel 4
    ctest --test-dir build --output-on-failure
    $qmlFiles = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
    qmllint @qmlFiles --unqualified disable
    powershell -ExecutionPolicy Bypass -File scripts/package.ps1

unqualified é desativado para context properties intencionalmente injetadas pelo C++. O pacote inclui Qt DLLs, QML, plugins, dependências MSYS2, qt.conf, configuração Discord e launcher. Mantém somente QSQLITE entre drivers SQL.

Quatro suítes CTest:

1. GlossataTests: regressões do jogo, perfis/ranking, filtros, níveis, grupos pequenos, mistura, fotos offline, licenças e migração/reabertura do catálogo.
2. GlossataInterfaceTests: cliques e confirmação, cinco reinos, subgrupos, caminho de volta, retorno ao grupo e capturas em 1240×820 e 1000×700; rejeita avisos QML.
3. GlossataDiscordTests: IPC isolado, handshake, frames fragmentados, PONG, confirmação/erro e ausência de Discord.
4. GlossataImportTests: nove testes Python sem rede para preservação de identidades e textos históricos, licenças, mapeamento, árvore, hashes, reimportação idempotente, revisão vinculada à foto/fonte e falha segura sem cache.

Testes usam bancos temporários e não publicam presença real. Capturas ficam em build/tests/previews; relatórios em build/tests/core-results.txt, ui-test-results.txt e catalog-audit.json. Consulte docs/REFINAMENTO_IDENTIFICACAO.md para resultados, limitações científicas e registros pendentes desta etapa.

## Próximas prioridades e retomada

Revisar cientificamente fatos/dicas anteriores e novos registros, acrescentar curiosidades com fontes, ampliar representatividade e avaliar progresso persistido por perfil se solicitado. A amostra inicial de Protista contém Protozoa; Monera contém Bacteria, embora o mapeamento também suporte Chromista e Archaea. Falta validar o pacote em um Windows sem Qt/MSYS2 instalado.

Preservar C++17, Qt Quick, SQLite, portas/adaptadores, IDs originais, dados dos jogadores e testes. Trabalhar apenas em C:\glossata_game. Ao alterar regras/catálogo, testar comportamento; ao mudar QML, conferir lint/capturas; ao recompilar o executável, atualizar o pacote. A API é usada na importação, nunca para jogar ou navegar.
