# Contexto do Projeto: Projeto Glossata

## Estado atual

<!-- glossata:status:start -->
<!-- Gerado por scripts/project_status.py; não editar manualmente. -->
| Informação atual | Valor derivado | Fonte |
| --- | --- | --- |
| Aplicativo | **1.7.0** — em desenvolvimento; não lançado | [VERSION.txt](VERSION.txt), [CHANGELOG](CHANGELOG.md) |
| Catálogo | **2.4.0** — não lançado | [Manifesto](resources/data/catalog_manifest.json), [histórico](docs/CATALOGO_CHANGELOG.md) |
| Regra padrão de pontuação | `identification-v4-progressive` | [Hint.h](src/domain/Hint.h) |
| Leitura histórica de pontuação | `identification-v3-assistance`, `identification-v2` e `legacy` | [Hint.h](src/domain/Hint.h) |
| Competitivo | 20 questões | [GameConfiguration.h](src/domain/GameConfiguration.h) |
| SQLite e snapshots | histórico 6; catálogo 2; checkpoint 1 | [Fontes e verificação](docs/ESTADO_DO_PROJETO.md) |
| Organismos e imagens do quiz | 122 registros; 120 arquivos distintos | [Catálogo](resources/data/species.json) |
| Domínios | Archaea: 0; Bacteria: 20; Eukaryota: 102 | [Taxonomia](resources/data/taxonomy.json) |
| Testes registrados | **25 suítes CTest**; não representa resultado de execução | [Inventário gerado](docs/PROJECT_STATUS.json), [CMake](tests/CMakeLists.txt) |

Valores derivados das fontes locais. Este bloco não comprova publicação, execução da CI ou aprovação do gate de distribuição.
<!-- glossata:status:end -->

Aplicativo desktop educativo em português, C++17/Qt Quick/QML, SQLite, offline. Use a raiz do clone aberto no editor; cópias em outras pastas não se sincronizam automaticamente. Executável: projeto_glossata.exe; identificador interno Qt glossata_quiz preservado para não mudar bancos dos jogadores. [Fontes e verificação do estado](docs/ESTADO_DO_PROJETO.md).

A exploração principal usa os três domínios do bloco gerado. Eukaryota contém Opisthokonta (Animalia e Fungi), Archaeplastida (Plantae da fonte), SAR (Stramenopiles/Ochrophyta), Amoebozoa (mixomicetos) e Discoba. IDs, nomes, fotografias, créditos e dados dos jogadores são preservados.

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

Livre é contínuo, com alternativas amplas/equilibradas/próximas. Competitivo usa questões de gênero na quantidade indicada no bloco gerado, fotografias de identificação e ranking separado por regras e hash de conteúdo. O pool atual tem 80 organismos competitivos. O misto Livre de gênero tem 100 registros; Plodia interpunctella permanece suspensa por revisão fotográfica.

Domínio, supergrupo, reino, filo/divisão, classe, ordem, família, gênero e espécie são distintos; ausência de rank nunca é preenchida por inferência. Quatro alternativas distintas e revisões adequadas continuam necessárias. Domínio/supergrupo não foram liberados automaticamente; reino também está indisponível porque só há três reinos científicos elegíveis.

Dicas taxonômica, etimológica e morfológica são categorias educativas. A assistência é independente: leve 6 unidades, moderada 3, forte 2. Combinações usam o menor teto, sem cobrança ao abrir/fechar/revisitar. Erro vale zero e pergunta nova começa em 6. A interface anuncia o teto combinado antes da revelação. Política padrão de testes: avaliação pedagógica approved_automated (progressive-pedagogy-v1), sem concessão científica ou jurídica. Três slots por assistência com IDs independentes da categoria, inclusive categorias repetidas justificadas. Lacunas permanecem explícitas. Snapshots e ranking usam a regra indicada no bloco gerado; leitura v2/v3 congelada preservada. Consulte docs/DICAS_E_PONTUACAO.md.

Exploração: acesso rápido abre os nós científicos Animalia, Plantae e Fungi, com breadcrumbs completos. JsonExplorationMediaCatalog implementa o port ExplorationMediaCatalog, injetado em TaxonomyIndex. O manifesto exploration_photos.json associa explicitamente 533 grupos e o misto a 109 fotos verificáveis; 89 grupos conservam ilustração e motivo documentado. Nenhuma foto de quiz muda. FeatheredPhoto.qml preenche 42% da largura e toda a altura, com PreserveAspectCrop equivalente, ponto focal, gradiente horizontal e cantos arredondados. Controles de jogo precedem os grupos filhos; acesso rápido vem depois de PlayModeSelector, preservando a posição dos filtros. Evidências e créditos estão integrados à governança; consulte docs/EXPLORACAO_E_FOTOGRAFIAS.md.

## Persistência e integridade

QStandardPaths::AppDataLocation mantém glossata_history.sqlite e glossata_catalog.sqlite. Nunca alterar bancos pessoais em testes/capturas. O esquema SQLite atual consta do bloco gerado (identidade permanente, Unicode e checkpoints): game_runs referencia o ID permanente e preserva o nome inicial; answer_records conserva pontos/snapshots. session_checkpoints salva motor/questões/ordens/dicas após cada ação antes de confirmar a interface; retomada/abandono explícitos, CAS por revisão e finalização/ranking na mesma transação. Fechar a janela suspende, não finaliza. Migração transacional/idempotente preserva arquivos lógicos e vínculos legados sem inferência por nome. Regra atual indicada no bloco gerado, leitura histórica v3/v2/legacy sem recálculo. Consulte docs/RECUPERACAO_PARTIDAS.md, docs/IDENTIDADE_JOGADORES.md e docs/NOMES_UNICODE.md.

O banco do catálogo tem esquema independente do histórico: DDL e carga transacionais, hash do bundle, índices, chaves estrangeiras e cache. educational_kingdom no SQLite conserva associação histórica. Nós antigos não são apagados; as raízes atuais são domínios. IDs edu: são aceitos somente por compatibilidade, sem aparecer como grupos naturais no menu.

Perfis têm UUID permanente, nome editável aparado, limite de 32 unidades UTF-16 e prevenção de novos conflitos pela política central nfc-simple-casefold-v1 (NFC/folding simples/NFC). A etapa Unicode audita chaves e colisões e verifica política/versão Unicode na reabertura. Colisões Unicode antigas recebem IDs separados, sem fusão. Um perfil ativo mínimo é preservado na interface. A sessão captura ID e nome ao iniciar; seleção, renomeação ou exclusão posterior não transferem resultados. Exclusão é lógica e nomes reutilizados geram IDs novos. Ranking conserva melhor/última partida e sinaliza registros legados ou de perfis excluídos. As seis conquistas mantêm as regras existentes.

## Fontes e direitos

docs/CONFORMIDADE.md, docs/DIREITOS_E_LICENCAS.md e docs/RELATORIO_CONFORMIDADE.md documentam pendências. review_baseline.json é registro legado que deve ser preservado, sem inviolabilidade física ou comprovação de revisão humana. Eventos são declarações de responsáveis; hashes e actor_type: human não autenticam autoria. A autoridade local de revisão continua nos ledgers/governance. audit-history --base SHA_COMPLETO compara os três ledgers/baseline com commit Git ancestral explicitamente selecionado; recibos mínimos não substituem uma âncora retida fora do checkout. Consulte docs/AUDITORIA_E_CONFIANCA.md. Não inventar revisores, evidências, autores, datas ou licenças; uma URL não aprova conteúdo.

photo_provenance.json e snapshots mínimos preservam crédito literal e evidência sem coordenadas/perfis de observadores. Padrão de novas fotos: CC0/CC BY; NC exige política expressa e autorização de escopo compatível. Bloquear licenças ausentes/desconhecidas/fora da política antes do download. Fonte bibliográfica não concede direito de cópia.

Há 120 fotos distintas do quiz, com pendências de versão/data de licença e confirmação humana, sete divergências de crédito, direitos da arte/textos e obrigações de dependências. Duas fotos de origem Casual exigem revisão prioritária. 109 imagens de apresentação têm licenças públicas por arquivo evidenciadas na coleta; isso não aprova cientificamente seu uso nem resolve os recursos legados. Nenhuma nova aprovação de especialista foi criada. O gate de distribuição permanece obrigatório.

## Provedores científicos e expansão

Saneamento INV-08: review_updates.py --rights-report gera relatório offline por foto/material em build/rights-investigation.json, reutilizando release_gate, credits, evidências e IDs da fila. --online coleta somente metadados públicos de fotos específicas em lotes limitados, com transporte seguro/cache minimizado em build/update-proposals; nunca altera dados publicados. --package package avalia o inventário existente e sua integridade separadamente. Casos científicos ambíguos continuam pendentes; ver docs/SANEAMENTO_DIREITOS.md.

scripts/scientific_sources define contratos, adaptadores, transporte limitado/cache, reconciliação e SQLite exclusivo de candidatos. scientific_catalog.py opera offline por padrão; --online habilita consultas explícitas. GBIF, CoL/ChecklistBank, WoRMS e Commons estão integrados; NCBI, LPSN, POWO e MycoBank têm planos/restrições em provider_plan.json. GBIF usa checklist fixo e CoL usa release 316165 (2026-08-26 XR), não um latest implícito. A descoberta por ancestral confirmado atende Archaea/Mollusca; Discoba ainda não tem rota integrada.

Candidatos, cache e relatórios ficam apenas em build/scientific-sources. Estados científico/jurídico são pending; não há comando de promoção, publicação ou atribuição de ID do jogo. As fontes não são votos independentes nem classificações automáticas do aplicativo. Media query/arquivo específico consulta só metadados com política CC0/CC BY por arquivo. Histórico da permissão e revisões são preservados. audit.staging_errors integra o gate contra vazamento de candidatos para o bundle.

Importador legado preserva reimportação sem expansão; nova expansão direta é recusada antes de rede/escrita. Não contornar esse bloqueio: futuras incorporações exigem revisão humana e mudança explícita no catálogo e seus arquivos editáveis. research_etymology e review_updates não fazem GET automatizado do HTML LPSN, conforme os termos consultados. Ver docs/PROVEDORES_CIENTIFICOS.md e docs/VALIDACAO_PROVEDORES.json. verify_scientific_providers.py valida contratos/cache sem incorporar conteúdo.

## Discord e empacotamento

DiscordRichPresence usa IPC local, sem bot token/Client Secret; ausência do Discord não bloqueia o jogo. Configuração embarcada em resources/data/discord.json e override em package/discord.json. Application ID 1559712085250801685; asset glossata_icon_v2. Mensagem: Explorando os domínios da vida; Uma descoberta por vez.

Ícones PNG/ICO e arte foram fornecidos pelo usuário. Launcher: package/Projeto Glossata.lnk ou package/run_glossata_game.cmd. O pacote local inclui DLLs, QML, plugins, qt.conf e apenas o driver SQL QSQLITE. -Development é exclusivo para testes locais; nunca publicar como release liberada.

## Build, testes e retomada

Ambiente: MSYS2 UCRT64, Qt 6, CMake/Ninja, Python 3.11+. No PowerShell, adicionar o diretório UCRT64/bin da sua instalação ao PATH e executar:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    python scripts/project_status.py check --build build
    cmake --build build --parallel 4
    ctest --test-dir build --output-on-failure
    python scripts/audit_catalog.py --strict
    python scripts/release.py check

O inventário e a quantidade de suítes são gerados acima e conferidos contra o CTest configurado. Registro não significa execução aprovada. Os testes usam bancos temporários e não publicam presença real. Capturas ficam em build/tests/previews; relatórios em build/tests. Validar também qmllint e scripts/validate_package.ps1; não declarar teste em Windows limpo sem realizá-lo. Regeneração de filogenia/educação/créditos pertence à manutenção de conteúdo, sem ser necessária para compilar o clone.

Atualizar o pacote local após recompilar. Em alterações relevantes da tela inicial, gerar captura real com scripts/capture_home.py e incluir docs/images/tela-inicial.png e seu registro no mesmo commit. O README também inclui docs/images/quiz.png, produzido pela interface de teste real, com proveniência em quiz.json. Nunca gerar mockup para representar funcionalidades.

Commits pequenos após verificações, sem push/release sem autorização específica. SemVer do aplicativo é independente do catálogo. Valores atuais vêm das fontes no bloco gerado; atualize-o por `python scripts/project_status.py write --build build` após alterar legitimamente essas fontes e reconfigurar o CMake. Tag vX.Y.Z somente pelo gate scripts/release.py tag; não contornar bloqueios nem confundir testes com certificação.

## Registros de etapas anteriores

A auditoria integrada de estabilização está em docs/AUDITORIA_ESTABILIZACAO.md: compatibilidade dos esquemas antigos, recusa de SQLite incompleto, validação de backup/sidecars e snapshots semanticamente inconsistentes. A consulta datada em docs/VERIFICACAO_ESTABILIZACAO_GITHUB.json confirmou ruleset 25303072 ativo, sem alteração remota pela auditoria. Dados pessoais e decisões reais de curadoria não foram usados ou modificados.

<!-- glossata:historical:start -->
Os registros abaixo descrevem etapas e consultas datadas; não afirmam versões lançadas, contagem atual de testes ou configuração remota atual. A configuração do GitHub não foi reconsultada nesta revisão documental.

Aplicativo 1.5.0 e catálogo 2.2.0 ampliaram ferramentas e cobertura fotográfica de apresentação (MINOR compatível), sem modificar os 122 organismos ou 120 fotos do quiz. Aplicativo 1.4.0 acrescentou provedores científicos (MINOR) sem alterar o catálogo 2.1.0; aplicativo 1.3.0 e catálogo 2.1.0 acrescentaram navegação/mídias (MINOR); o catálogo 2.0.0 marcou a mudança MAJOR de organização/esquema.

Evolução integrada 1.6.0: commits 6cf7dc6 (Git/CI), b3e99e4 (109 fotos de apresentação, 534/623 cartões, 89 pendências), d7c523e (dicas progressivas), a16530e (compatibilidade dos atalhos) e ef70d7c (relatório). Catálogo 2.4.0; validação em docs/RELATORIO_EVOLUCAO_INTEGRADA.md. Publicação do histórico na main autorizada e realizada com a ativação da curadoria em 2026-10-16; sem tags/releases.

Curadoria automatizada (1.7.0): scripts/curation_requests.py reutiliza os auditores; fila mínima em build/curation, Issues limitadas/idempotentes, Project opcional e não criado. Publicação e ativação semanal autorizadas em 2026-10-16; variável remota GLOSSATA_CURATION_WEEKLY_SYNC=true, cron 17 12 * * 1 (segunda 9h17 Brasília). Rodadas reais 38953314116 e 38953556113: seis IDs únicos, uma atualização controlada; simulação final 38954178774 sem escrita. CI técnico 38952589782 aprovado após correção 4c1442c; gate de distribuição bloqueado. Somente decisões humanas explícitas usam os ledgers; Issue fechada/etiqueta não aprova recursos. Catálogo 2.4.0 preservado. Evidências/limitações em docs/CURADORIA_AUTOMATIZADA.md; sem tag/release.
<!-- glossata:historical:end -->
