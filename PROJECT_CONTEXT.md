# Contexto do Projeto: Projeto Glossata

## Estado atual

Aplicativo desktop educativo em português, C++17/Qt Quick/QML, SQLite, offline. Versão do aplicativo **1.3.0**; catálogo **2.1.0**, ambos ainda não lançados. Workspace canônico: C:\glossata_game. Não editar a cópia antiga do Desktop. Executável: projeto_glossata.exe; identificador interno Qt glossata_quiz preservado para não mudar os bancos dos jogadores.

A exploração principal usa **Bacteria (20), Archaea (0) e Eukaryota (102)**. Eukaryota contém Opisthokonta (62: Animalia e Fungi), Archaeplastida (20: Plantae da fonte), SAR (10: Stramenopiles/Ochrophyta), Amoebozoa (10: mixomicetos) e Discoba (0). Os 122 IDs, nomes, fotografias, créditos e dados dos jogadores são preservados.

Três domínios são portas didáticas, com as hipóteses alternativas de dois domínios documentadas. Supergrupos são clados de navegação sem rank formal universal. A árvore omite ramos intermediários; não afirma uma filogenia completa nem parentesco imediato entre todos os nós visíveis. Monera/Protista são referências históricas, não grupos naturais ativos. Consulte docs/CLASSIFICACAO_FILOGENETICA.md e docs/AUDITORIA_FILOGENETICA.json.

## Arquitetura e conteúdo

- Domínio: Species distingue domínio, supergrupo, reino e ranks inferiores em sua linhagem. educationalKingdom existe somente para compatibilidade histórica; a aplicação usa domain()/displayGroup(). QuizSelection centraliza elegibilidade; TaxonomicPerformance agrega por IDs das linhagens atuais e filo/divisão. Acertos, precisão e pontuação permanecem independentes.
- TaxonomyIndex mantém árvore, contagens, caminhos e níveis em memória. Suas raízes vêm dos nós de domínio; a navegação inclui grupos vazios, com quiz desabilitado. SpeciesCatalog e GameHistoryRepository preservam a separação entre aplicação e infraestrutura.
- GameService filtra descendentes, intercala linhagens, controla o ciclo e salva resultados. QuestionGenerator separa contexto, proximidade e distratores; nenhuma consulta de rede ocorre durante partidas.
- JsonSpeciesCatalog lê os recursos embarcados. SqliteSpeciesCatalog migra o catálogo de forma aditiva/transacional, com pais antes de filhos e upsert; em falha usa o conteúdo embarcado como fallback. Mantém registros locais e nós históricos.
- GameController expõe estado ao QML e preserva o snapshot da pergunta respondida até nextQuestion. Main.qml mantém sidebar, Loader, rolagem e transições. Não duplica regras de pontuação no QML.
- resources/data/phylogeny.json contém a política explícita, fontes, localizadores, divergências e hipóteses alternativas. scripts/phylogeny.py faz projeção e auditoria idempotentes. Não cria ranks, táxons ou aprovações para preencher lacunas.
- species.json mantém taxonomy atual e source_taxonomy original integral por organismo; legacy_educational_kingdom é histórico. classification identifica o sistema, fontes, divergências e limitações. taxonomy.json contém os nós atuais.
- EducationalContent lê education.json gerado por scripts/build_education.py a partir de species_profiles.json, hint_education.json, revisões e fontes. O tópico taxonomy conserva a revisão da classificação original; phylogeny tem revisão independente. Os 122 enquadramentos novos são rascunhos assistidos por IA, sem revisão humana declarada.
- PhotoResolver confere identidade, licença, fonte, crédito, hash, decodificação e resolução mínima de 300 pixels. photo_reviews.json vincula revisão ao ID da foto; photo_json conserva metadados no SQLite. Imagens de apoio bacterianas mantêm avisos e restrições. Não ampliar ranks fotográficos automaticamente.

## Fluxos e regras

Início → Jogar → Todos os organismos ou domínio → linhagens/grupos → nível elegível → quiz → feedback → ficha → próxima pergunta ou resultado. Breadcrumbs voltam aos ancestrais; encerrar requer confirmação. Falha de gravação conserva resultado pendente para nova tentativa.

Livre é contínuo, com alternativas amplas/equilibradas/próximas. Competitivo tem 20 questões de gênero, fotografias de identificação e ranking separado por regras e hash de conteúdo. O pool atual tem 80 organismos competitivos. O misto Livre de gênero tem 100 registros; Plodia interpunctella permanece suspensa por revisão fotográfica.

Domínio, supergrupo, reino, filo/divisão, classe, ordem, família, gênero e espécie são distintos; ausência de rank nunca é preenchida por inferência. Quatro alternativas distintas e revisões adequadas continuam necessárias. Domínio/supergrupo não foram liberados automaticamente; reino também está indisponível porque só há três reinos científicos elegíveis.

Dicas taxonômica, etimológica e morfológica são categorias educativas. A assistência é independente: leve 6 unidades, moderada 3, forte 2. Combinações usam o menor teto, sem cobrança ao abrir/fechar/revisitar. Erro vale zero e pergunta nova começa em 6. A interface anuncia o teto combinado antes da revelação. Propostas pendentes ficam sinalizadas no Livre e indisponíveis no Competitivo. Consulte docs/DICAS_E_PONTUACAO.md.

Exploração: acesso rápido abre os nós científicos Animalia, Plantae e Fungi, com breadcrumbs completos. JsonExplorationMediaCatalog implementa o port ExplorationMediaCatalog, injetado em TaxonomyIndex. O manifesto exploration_photos.json associa IDs explícitos a oito imagens reais verificáveis, sem alterar fotos de quiz. FeatheredPhoto.qml mantém a região dos cartões, aplica transparência progressiva e preserva ilustrações sem foto adequada. Evidências e créditos estão integrados à governança; consulte docs/EXPLORACAO_E_FOTOGRAFIAS.md.

## Persistência e integridade

QStandardPaths::AppDataLocation mantém glossata_history.sqlite e glossata_catalog.sqlite. Nunca alterar bancos pessoais em testes/capturas. O esquema do histórico continua v3: game_runs/answer_records preservam regras, pontos inteiros, alternativas e snapshots verificáveis de assistência. Regras atuais: identification-v3-assistance; v2/legacy são interpretadas conforme o contrato original, sem recálculo retroativo.

O catálogo passa à migração v2: DDL e carga transacionais, hash do bundle, índices, chaves estrangeiras e cache. educational_kingdom no SQLite conserva associação histórica. Nós antigos não são apagados; as raízes atuais são domínios. IDs edu: são aceitos somente por compatibilidade, sem aparecer como grupos naturais no menu.

Perfis têm nomes aparados, limite de 32 caracteres e unicidade COLLATE NOCASE. Um perfil mínimo é preservado. A partida captura o jogador ao iniciar; trocar o perfil não transfere resultados. Ranking mostra melhor resultado e última partida separadamente. As seis conquistas mantêm as regras existentes.

## Fontes e direitos

docs/CONFORMIDADE.md, docs/DIREITOS_E_LICENCAS.md e docs/RELATORIO_CONFORMIDADE.md documentam pendências. review_baseline.json é imutável e não comprova revisão humana. Histórico humano encadeado e governance são a autoridade de revisão. Não inventar revisores, evidências, autores, datas ou licenças; uma URL não aprova conteúdo.

photo_provenance.json e snapshots mínimos preservam crédito literal e evidência sem coordenadas/perfis de observadores. Padrão de novas fotos: CC0/CC BY; NC exige política expressa e autorização de escopo compatível. Bloquear licenças ausentes/desconhecidas/fora da política antes do download. Fonte bibliográfica não concede direito de cópia.

Há 120 fotos distintas do quiz, com pendências de versão/data de licença e confirmação humana, sete divergências de crédito, direitos da arte/textos e obrigações de dependências. Duas fotos de origem Casual exigem revisão prioritária. Oito imagens de apresentação adicionais têm licenças públicas por arquivo evidenciadas na coleta; isso não aprova cientificamente seu uso nem resolve os recursos legados. Nenhuma nova aprovação de especialista foi criada. O gate de distribuição permanece obrigatório.

## Discord e empacotamento

DiscordRichPresence usa IPC local, sem bot token/Client Secret; ausência do Discord não bloqueia o jogo. Configuração embarcada em resources/data/discord.json e override em package/discord.json. Application ID 1559712085250801685; asset glossata_icon_v2. Mensagem: Explorando os domínios da vida; Uma descoberta por vez.

Ícones PNG/ICO e arte foram fornecidos pelo usuário. Launcher: package/Projeto Glossata.lnk ou package/run_glossata_game.cmd. O pacote local inclui DLLs, QML, plugins, qt.conf e apenas o driver SQL QSQLITE. -Development é exclusivo para testes locais; nunca publicar como release liberada.

## Build, testes e retomada

Ambiente: MSYS2 UCRT64, Qt 6, CMake/Ninja, Python 3.11+. No PowerShell, adicionar C:\msys64\ucrt64\bin ao PATH e executar:

    python scripts/phylogeny.py
    python scripts/build_education.py
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel 4
    ctest --test-dir build --output-on-failure
    python scripts/audit_catalog.py --strict
    python scripts/release.py check

São oito suítes: domínio/persistência, interface real, Discord isolado, importação, governança, versionamento, projeção filogenética e fotografias de exploração. Usam bancos temporários e não publicam presença real. Capturas ficam em build/tests/previews; relatórios em build/tests. Validar também qmllint e scripts/validate_package.ps1; não declarar teste em Windows limpo sem realizá-lo.

Atualizar o pacote local após recompilar. Em alterações relevantes da tela inicial, gerar captura real com scripts/capture_home.py e incluir docs/images/tela-inicial.png e seu registro no mesmo commit. O README também inclui docs/images/quiz.png, produzido pela interface de teste real, com proveniência em quiz.json. Nunca gerar mockup para representar funcionalidades.

Commits pequenos após verificações, sem push/release sem autorização específica. SemVer do aplicativo é independente do catálogo. Aplicativo 1.3.0 e catálogo 2.1.0 acrescentam navegação e mídias de apresentação compatíveis (MINOR); o catálogo 2.0.0 anterior marcou a mudança MAJOR de organização/esquema. Tag vX.Y.Z somente pelo gate scripts/release.py tag; não contornar bloqueios nem confundir testes com certificação.
