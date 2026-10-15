# Contexto do Projeto: Projeto Glossata

## Objetivo e estado atual

Aplicativo desktop educativo em português para explorar Animalia, Plantae, Fungi, Protista e Monera no modelo didático tradicional dos cinco reinos. Cada pergunta tem quatro alternativas do mesmo nível taxonômico, seleção e confirmação, painel de três dicas e ficha da espécie. O modo Livre continua até o jogador encerrá-lo; o Competitivo padronizado tem 20 questões.

O catálogo ativo possui 122 espécies, com 42 Animalia e 20 em cada um dos outros reinos. Após autorização do usuário, 26 organismos sem cobertura visual foram substituídos; os dados anteriores foram arquivados. Monera mantém os mesmos 20 organismos, com imagens de apoio para identificação por pistas. Consulte [docs/SUBSTITUICOES_CATALOGO.md](docs/SUBSTITUICOES_CATALOGO.md) para o estado atual e [docs/PHOTO_CREDITS.md](docs/PHOTO_CREDITS.md) para créditos.

## Workspace e ambiente

- Nome exibido: Projeto Glossata. Executável: projeto_glossata.exe. O identificador interno Qt continua glossata_quiz para preservar o diretório existente dos bancos de perfis e histórico.

- Raiz canônica: C:\glossata_game. Não editar a cópia antiga do Desktop, cujo caminho com acentos e & já provocou problemas de moc/autogen.
- Git: main integra a expansão dos cinco reinos; feat/refinamento-identificacao contém esta etapa de refinamento. origin aponta para https://github.com/kaduolliver/Projeto-Glossata.
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
- EducationalContent lê education.json: introduções por ID, 122 fichas e dicas/curiosidades por espécie ou grupo. Confere ID e nome científico para aplicar a ficha. Rejeita pistas que contenham alternativas, gênero ou nomes da linhagem; não usa fatos legados não revisados. species_profiles.json é o corpus editável, incorporado offline por scripts/build_education.py; consulte docs/FICHAS_ESPECIES.md.
- ExploreCard distingue cadastro de cobertura visual; SpeciesPortrait exibe fotos e créditos sem substituição decorativa no quiz. TextAction e TaxonComboBox compartilham os estados arredondados, escuros e com foco acessível; EndGameDialog confirma o encerramento.
- resources/data contém species.json, taxonomy.json, catalog_replacements.json, photo_reviews.json, photo_audit.json, education.json, import_report.json e discord.json. Há 120 arquivos de imagem para os 122 registros ativos; candidatos não selecionados não entram no executável. role=supporting e supporting_for vinculam uma imagem de apoio a uma questão de Monera, preservando o táxon original do registro fotográfico.
- scripts/import_inaturalist.py importa, verifica licenças individuais, deduplica e mantém cache local.

## Fluxos e regras

Home → Jogar → Todas as espécies ou reino → grupos/subgrupos → nível de identificação → quiz → feedback → ficha → próxima pergunta ou resultado → retorno à exploração.

Grupos são gerados pelos dados e podem ter profundidades e ranks diferentes. O caminho clicável retorna aos ancestrais. Grupos pequenos e taxonomias parciais continuam navegáveis; quizzes sem quatro respostas distintas são desabilitados. Níveis possíveis: espécie, gênero, família, ordem, classe, filo/divisão e reino científico.

Todas as alternativas pertencem ao grupo escolhido e ao nível da pergunta. O modo misto intercala reinos proporcionalmente ao tamanho dos grupos e embaralha internamente. Espécies não repetem dentro de um ciclo nem imediatamente entre ciclos. Resposta inválida não altera a rodada. O botão Dica abre três categorias: taxonômica (6 unidades), etimológica (3) e morfológica (2). Uma categoria disponível é marcada uma vez e pode ser consultada novamente; combinações mantêm o menor teto. Abrir o painel não custa pontos. Acertos, erros e precisão são contados separadamente. Consulte docs/DICAS_E_PONTUACAO.md para cobertura, fontes e migração.

Responder prepara a próxima pergunta no serviço; o controller mantém o snapshot da pergunta, foto, dica e alternativas até nextQuestion e bloqueia confirmações duplicadas. Voltar abre uma confirmação modal. Continuar preserva a seleção; confirmar registra uma vez e retorna ao grupo anterior ou aos reinos. Falha de gravação leva ao resultado pendente para nova tentativa.

Todos os 122 registros ativos possuem imagem tecnicamente válida: 102 têm fotos para identificação visual e os 20 de Monera usam imagens de apoio com aviso explícito, conforme solicitação do usuário. Monera possui descrições biológicas e pistas em linguagem simples. O modo misto inicia em gênero, com 100 registros dos cinco reinos; Plodia interpunctella está suspensa das perguntas por revisão fotográfica. Há 16 introduções de grupos e 122 fichas com apresentação, distribuição/ocorrências documentadas, morfologia, conservação, curiosidade e dica; 107 incluem alimentação/nutrição. Há 22 categorias globais da IUCN registradas no iNaturalist e 100 fichas sem categoria global confirmada. Fontes e classificação completa podem ser expandidas na ficha. Níveis sem quatro alternativas distintas continuam bloqueados. Registros substituídos recebem novos IDs e os antigos ficam arquivados em catalog_replacements.json.

Desempenho taxonômico agrega por reino didático e ID de filo/divisão da fonte, independentemente do nível perguntado. Um filo ausente ou ambíguo usa Classificação não disponível. O desempenho taxonômico e as seis conquistas permanecem relativos à partida atual/última partida. Estatísticas adicionais do histórico são derivadas das respostas persistidas por perfil e modalidade.

## Dados locais e migrações

Dois bancos ficam em QStandardPaths::AppDataLocation, fora do pacote:

- glossata_history.sqlite mantém history/player_profiles no mesmo caminho; score foi migrado transacionalmente para score_units, multiplicando valores antigos por seis uma única vez. game_runs/answer_records guardam as novas partidas e suas respostas, categorias reveladas, máximos e pontos, com UUID e salvamento transacional.
- glossata_catalog.sqlite mantém taxons, espécies, origem, créditos e hash do conteúdo embarcado.

O catálogo usa chaves estrangeiras, índices e consultas recursivas. Carga/atualização de conteúdo são transacionais, com upsert e pais antes dos filhos. Colunas são acrescentadas sem apagar tabelas ou registros anteriores. A coluna active marca somente substituições explícitas como inativas, verificando ID/nome anterior e existência do substituto. Consultas e contagens usam registros ativos; dados locais não relacionados permanecem intactos. Reabertura carrega o SQLite uma vez para memória. Se a preparação falhar, o adaptador conserva o conteúdo embarcado como fallback.

Perfis têm nomes aparados, limite de 32 caracteres e unicidade COLLATE NOCASE. É possível criar, selecionar, renomear e excluir; o último perfil não pode ser excluído. Instalação vazia cria Jogador 1. A partida captura o nome ao iniciar; mudanças de perfil durante o jogo não transferem seu resultado.

O ranking mostra a melhor pontuação por nome e modalidade; a última partida fica separada. O Competitivo usa uma chave com limite, grupo, nível, dificuldade, regras e SHA-256 do conteúdo. PRAGMA user_version=2 migra o histórico transacionalmente, preservando linhas legadas e reconstruindo somente partidas com respostas disponíveis. Falha ao salvar mantém resultado pendente e permite repetir sem duplicação. Consulte docs/EVOLUCAO_COMPETITIVA.md.

## Discord e ícones

DiscordRichPresence usa RPC/IPC local com Qt Network/QLocalSocket, sem token de bot ou Client Secret. Configuração em resources/data/discord.json; package/discord.json pode substituir a embarcada. Application ID: 1559712085250801685; asset: glossata_icon_v2.

A mensagem permanece Explorando os cinco reinos biológicos e Uma descoberta por vez, sem nomes ou pontuações. Ausência de Discord não bloqueia o jogo; a presença pode ser desativada em ajustes.

resources/branding contém PNG, ICO multirresolução e arte para Discord fornecidos pelo usuário. ICO é recurso nativo do executável e ícone global da janela. Launcher recomendado: package/Projeto Glossata.lnk; alternativa: package/run_glossata_game.cmd. O pacote usa ICO com nome derivado de hash e atualiza o cache do Windows. Assets do Discord são administrados separadamente no Developer Portal.

## Build, testes e pacote

No PowerShell, a partir da raiz:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel 4
    ctest --test-dir build --output-on-failure
    $qmlFiles = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
    qmllint @qmlFiles --unqualified disable
    powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Development

unqualified é desativado para context properties intencionalmente injetadas pelo C++. O pacote inclui Qt DLLs, QML, plugins, dependências MSYS2, qt.conf, configuração Discord e launcher. Mantém somente QSQLITE entre drivers SQL.

Quatro suítes CTest:

1. GlossataTests: regressões do jogo, perfis/ranking, filtros, níveis, grupos pequenos, mistura, fotos offline, licenças e migração/reabertura do catálogo.
2. GlossataInterfaceTests: cliques e confirmação, cinco reinos, subgrupos, caminho de volta, retorno ao grupo e capturas em 1240×820 e 1000×700; rejeita avisos QML.
3. GlossataDiscordTests: IPC isolado, handshake, frames fragmentados, PONG, confirmação/erro e ausência de Discord.
4. GlossataImportTests: quatorze testes Python sem rede para preservação de identidades e textos históricos, licenças, mapeamento, árvore, hashes, reimportação idempotente, revisão vinculada à foto/fonte, falha segura sem cache e geração educacional reproduzível com validação de identidade/fontes/escopo e rejeição de classificação como etimologia.

Testes usam bancos temporários e não publicam presença real. Capturas ficam em build/tests/previews; relatórios em build/tests/core-results.txt, ui-test-results.txt e catalog-audit.json. Consulte docs/REFINAMENTO_IDENTIFICACAO.md para resultados, limitações científicas e registros pendentes desta etapa.

## Próximas prioridades e retomada

Revisar cientificamente fatos/dicas anteriores e novos registros, acrescentar curiosidades com fontes, ampliar representatividade e avaliar progresso persistido por perfil se solicitado. Protista contém dez Protozoa e dez Chromista; Monera contém vinte Bacteria e nenhuma Archaea. Falta validar o pacote em um Windows sem Qt/MSYS2 instalado.

Preservar C++17, Qt Quick, SQLite, portas/adaptadores, IDs originais, dados dos jogadores e testes. Trabalhar apenas em C:\glossata_game. Ao alterar regras/catálogo, testar comportamento; ao mudar QML, conferir lint/capturas; ao recompilar o executável, atualizar o pacote. A API é usada na importação, nunca para jogar ou navegar.

## Dicas estratégicas e revisão após a resposta

A pontuação usa unidades inteiras de 64 bits: 6 por ponto, 3 por meio ponto e 2 por um terço. A interface mostra duas casas decimais. O ranking mantém recorde e última partida separadamente e ordena por unidades. Cada resposta preserva organismo, foto, nível, alternativas escolhida/correta, acerto, categorias, teto e unidades obtidas. O domínio calcula a combinação; QML apresenta os valores. A revisão educativa é opcional e não altera pontos.

As dicas taxonômicas citam ancestrais verificados mais amplos e deixam ao menos três alternativas compatíveis. Etimologia tem referências para 49 organismos; 104 têm morfologia curada, vinculada à fotografia. Etimologia/morfologia estão revisadas para espécie/gênero, com disponibilidade dependente do conteúdo, elegibilidade e questão. Corpus em resources/data/hint_education.json, incorporado offline pelo gerador; nenhuma consulta de rede ocorre nas partidas. Detalhes, pendências e testes em docs/DICAS_E_PONTUACAO.md.

## Evolução de engenharia e competição

QuestionGenerator separa a geração contextual do ciclo de GameService. GameConfiguration define modo, limite, filtros, dificuldade e versões; SessionMetrics compartilha a acumulação entre respostas atuais e persistidas. QuizSelection usa uma passagem linear e TaxonomyIndex guarda níveis em cache. ReviewResolver aplica a mesma revisão na carga JSON e SQLite; reviews opcionais pendentes não bloqueiam fotografias aprovadas.

Livre oferece alternativas mais amplas, equilibradas ou próximas. Competitivo usa 20 questões de gênero, alternativas equilibradas e fotografias de identificação, com 80 organismos elegíveis; as 20 imagens de apoio de Monera ficam no Livre. A última resposta pode ser revisada antes do resultado final; partidas interrompidas são guardadas e ficam fora do ranking competitivo. Nenhuma espécie foi removida.

Corpus, revisões e diagnóstico: resources/data/curation_reviews.json, resources/data/etymology_research.json e scripts/audit_catalog.py. Há 49 etimologias ao menos parciais, 104 registros morfológicos e 16 introduções. O teste scripts/validate_package.ps1 utiliza bancos temporários e PATH isolado; ainda falta um Windows limpo sem Qt/MSYS2. Relatórios em docs/EVOLUCAO_COMPETITIVA.md e docs/COBERTURA_TAXONOMICA.md.

## Governança científica e direitos

Consulte docs/CONFORMIDADE.md e docs/DIREITOS_E_LICENCAS.md. A baseline resources/compliance/review_baseline.json congela marcações históricas, sem demonstrar revisão humana. Não recriar a baseline para aprovar conteúdo. Datas antigas/flags verified/approved são legadas; a autoridade de revisão é o histórico humano encadeado e governance gerado em education.json. Novo conteúdo fica draft; mudanças de conteúdo/fonte/foto invalidam a revisão. Prazos são configuráveis em content_policy.json e conferidos também offline.

photo_provenance.json e snapshots mínimos preservam evidência por foto, sem perfis do observador nem coordenadas. Crédito do fotógrafo não deve ser inferido do observador. credits.json, PHOTO_CREDITS.md e tela de créditos têm origem comum. Fotos continuam CC0/CC BY por padrão; NC exige política e grant expressos. Consultas de manutenção geram somente propostas em build/update-proposals, nunca renovam revisão nem substituem silenciosamente dados publicados.

package.ps1 bloqueia distribuição por padrão; -Development é apenas para testes locais e exporta avisos/evidências. Há pendências de versão/data da licença de 120 fotos, sete créditos divergentes, direitos da arte/textos e obrigações de dependências. Duas fotos legadas de origem Casual exigem revisão científica prioritária. Preservar perfis/histórico e os modos locais, sem chamar o legado de validado por especialista. Nenhuma licença dos materiais próprios foi escolhida. A quinta suíte GlossataGovernanceTests cobre regressões de evidência, direitos, privacidade, revisão e invalidação. Testes não são certificação de regularidade.
