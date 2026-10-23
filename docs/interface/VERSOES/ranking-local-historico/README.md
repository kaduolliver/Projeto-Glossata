# Ranking local e histórico de jogadores

Melhoria independente após a PR #36/Subetapa 2.3.2, baseada em `adbf100`. Aplicativo **1.12.0** (MINOR pela nova consulta e navegação compatíveis), catálogo **2.4.0** inalterado. Sem lançamento ou início da Subetapa 2.3.3.

## Diagnóstico

`PlayerIdentityMigration.cpp` já cria UUIDs permanentes, separa identidades `legacy_unresolved` e registra a proveniência em `identity_migration_notes`. Não presume propriedade por nome. A exclusão de perfil grava `deleted_at`; identidades e resultados são conservados. Gatilhos impedem apagar identidades, trocar UUIDs e transferir partidas ou seus nomes registrados. Nenhuma migração, tabela, índice ou gatilho foi alterado nesta melhoria; o esquema permanece na versão 6.

`SqliteGameHistoryRepository::loadRanking` e `loadTopResults` consultavam recordes de todas as identidades. `GameService::history` e testes existentes utilizam esse contrato, que permanece intacto. `GameController` antes usava a consulta mista para as cinco posições da tela. `ranking_records` conserva um melhor resultado e a última pontuação por UUID/classificação; não é um registro completo de todas as partidas. `game_runs`, `answer_records`, arquivos legados e checkpoints têm outras finalidades e permanecem intactos.

## Consultas atualizadas

`GameHistoryRepository` oferece operações explícitas para a fonte local: `loadActiveRanking(key, limit)` e `loadRecordHistory(key, limit, offset)`. Os padrões vazios preservam a compilação dos consumidores existentes e recusam uma visão não suportada. Nenhuma delas filtra uma lista mista previamente limitada.

O ranking atual faz `JOIN` por UUID e aplica `origin='profile' AND deleted_at IS NULL` no `WHERE`, **antes do `LIMIT`**. Perfis sem recordes não ocupam posição. O controlador solicita cinco jogadores. Ordena por pontuação decrescente, acertos decrescentes, data mais antiga do melhor resultado, nome sem distinção ASCII de caixa, nome exato e UUID, conservando os desempates anteriores.

O histórico consulta os mesmos valores armazenados, sem calcular resultados com regras atuais. Inclui ativos, excluídos e legados, mostrando o nome registrado em `ranking_records.player_name`, datas, pontos, acertos, situação e UUID. Esse nome é o snapshot disponível no recorde (atualizado pelo mecanismo anterior ao registrar uma nova partida), não uma reconstrução do nome do melhor resultado. Renomear um perfil não reescreve esse snapshot. O ranking atual continua usando o nome atual do perfil.

No histórico, `free` consulta a classificação Livre; `competitive` reúne as chaves armazenadas `competitive|…`, inclusive versões antigas de conteúdo/regras. Também é possível consultar uma chave exata pelo repositório. Cada recorde competitivo expõe sua chave original. Os desempates históricos usam o nome registrado e acrescentam a chave depois do UUID para tornar páginas de configurações diferentes determinísticas. Isso não cria um ranking competitivo vigente entre configurações antigas.

As consultas históricas aceitam no máximo 100 linhas por chamada e recusam limite não positivo ou deslocamento negativo. A tela mantém somente uma página de até 20 registros e solicita uma linha adicional para saber se existe próxima página. Anterior/Próxima substituem a página em memória; não acumulam todo o banco. A paginação por deslocamento é local, sem promessa de snapshot entre escritas concorrentes.

## Interface e identidades

Estatísticas conserva painéis no topo, seletor responsivo Taxonomia/Ranking, cores taxonômicas, rolagem compartilhada e o pódio do ranking atual. Acrescenta seletores independentes Ranking atual/Histórico e Livre/Competitivo. O histórico não mostra posições, medalhas ou fundo de campeão. Identifica “Jogador ativo”, “Perfil excluído” e “Legado — identidade não vinculada”.

O contexto é guardado no controlador durante a execução, sem gravar preferências no SQLite. Voltar de outra página conserva contexto, modalidade e página histórica. Alterar modalidade ou atualizar resultados/perfis reinicia a paginação para evitar deslocamentos obsoletos. Alternar apenas Ranking atual/Histórico não consulta nem altera recorde pessoal ou métricas. O histórico detalhado do perfil continua relativo à configuração competitiva atual, mesmo quando o painel de recordes consulta configurações antigas.

`loadPlayerResult` e `playerMetrics` continuam consultando exclusivamente `player_id`; seleção de perfil, estatísticas da partida, migrações e recuperação não mudam. Homônimos, nomes reutilizados e identidades legadas nunca são associados automaticamente. As informações de proveniência (`identity_migration_notes`, snapshots, origem e chaves de classificação), resultados, datas e vínculos originais permanecem no banco.

## Validação e evidências

Os resultados executados, capturas reais e inventário não destrutivo serão registrados neste diretório após os checks. Todos os testes/capturas desta melhoria usam SQLite temporário e o QML/controlador reais. Não há acesso aos bancos dos jogadores.

## Limitações futuras

Histórico é uma consulta de **recordes registrados**, não uma lista completa de partidas. Cada UUID pode ter um recorde por configuração armazenada. A fonte continua explicitamente local e offline, sem conta, servidor, API, PostgreSQL, sincronização, ranking global ou importação de resultados antigos. Uma fonte global futura precisa de seu próprio contrato e políticas; esta melhoria não cria esse serviço nem concede autorização de distribuição.
