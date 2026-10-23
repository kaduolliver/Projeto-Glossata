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

Implementação em dois commits: [consultas e testes de identidade](https://github.com/kaduolliver/Projeto-Glossata/commit/6fa02e4) e [controlador, interface, testes e versão](https://github.com/kaduolliver/Projeto-Glossata/commit/495e9de). Todos os testes/capturas desta melhoria usam SQLite temporário e o QML/controlador reais. Não há acesso aos bancos dos jogadores.

`ctest --test-dir build/local-ranking-history --output-on-failure`: **26/26 suítes aprovadas**, em 774,15 segundos. Os resultados incluem 62 casos de regras/controlador, 35 de identidade, 21 de recuperação e 30 de interface (contagens do QtTest incluem inicialização/finalização). Os três testes de navegação do histórico, painéis/rolagem e cores/pódio também passaram separadamente em 125% e 150%. Lint de todo o QML, smoke test do executável com banco temporário, auditorias do repositório/catálogo, metadados e `release.py check`/`check-history` aprovados. A comparação de governança conserva os 1.053 eventos da base.

| Cenário | Evidência automatizada |
| --- | --- |
| Mais de cinco ativos; ativo sem partidas; excluído/legado acima dos ativos; homônimos; nome reutilizado; menos de cinco; nenhum elegível | `PlayerIdentityTests::activeRankingFiltersBeforeLimitAndPreservesHistory` e `activeRankingWithNoEligiblePlayers` |
| Paginação, limites, nome registrado, datas, pontos, acertos e desempates por nome/UUID | `PlayerIdentityTests::historicalPagesAndStoredTieBreaks` |
| Livre/Competitivo e contexto independentes; configuração antiga; recorde e métricas por UUID; seleção, sessão e reabertura | `QmlInterfaceTests::localRankingHistoryNavigationAndCaptures`, testes de identidade e regressões de controlador |
| Recuperação, encerramento após exclusão e reutilização de nome, respostas/checkpoints imutáveis | `GlossataSessionRecoveryTests`, regressões existentes e novo teste de navegação |
| Pódio, teclado, foco, painéis no topo, seletor compacto e rolagem | `statisticsStableColorsAndPodium`, `statisticsTopPanelsAndSharedScrolling` e regressão completa de interface |

O teste de filtragem inclui seis ativos com resultados e dois registros inelegíveis com pontuação maior: filtrar somente depois de um `LIMIT 5` não produziria as cinco linhas esperadas. Compara ainda os dados de todas as tabelas antes/depois das consultas e após reabrir. Os testes antigos de controlador passam a procurar os resultados excluídos no histórico, mantendo a verificação do UUID, snapshot e métricas originais. As falhas da primeira execução e sua causa ficam nos logs `initial-*`, junto aos resultados finais; não foram apagadas para ocultar resultados.

Os [resultados e hashes](verificacoes/resultados.json), [log completo do CTest](verificacoes/ctest.txt) e [manifesto de capturas](capturas.json) vinculam fontes, binários e evidências. São **36 PNGs reais**, com dados sintéticos declarados, em nove combinações de dimensões lógicas/fator Qt. Não houve imagem simulada nem troca do backend por mapa de apresentação nos quadros novos. Alguns enquadramentos acompanham o foco de teclado e a rolagem, deixando cabeçalhos/linhas fora da área visível; o conteúdo continua acessível pela rolagem da página.

| Resolução lógica | 100% | 125% | 150% |
| --- | --- | --- | --- |
| 1000 × 700 | [Histórico](capturas/100/history-first-1000x700.png) | [Histórico](capturas/125/history-first-1000x700.png) | [Histórico](capturas/150/history-first-1000x700.png) |
| 1240 × 820 | [Histórico](capturas/100/history-first-1240x820.png) | [Histórico](capturas/125/history-first-1240x820.png) | [Histórico](capturas/150/history-first-1240x820.png) |
| 1920 × 1080 | [Histórico](capturas/100/history-first-1920x1080.png) | [Histórico](capturas/125/history-first-1920x1080.png) | [Histórico](capturas/150/history-first-1920x1080.png) |

Também há quadros de [ranking atual com pódio](capturas/100/ranking-current-1920x1080.png), [classificação competitiva antiga](capturas/150/history-competitive-archived-1240x820.png) e [controles da segunda página](capturas/125/history-next-page-1000x700.png). Backend Qt software/offscreen no Windows 11, Qt 6.11.2, com `QT_SCALE_FACTOR=1`, `1.25` e `1.5`; isso não certifica mudanças físicas entre monitores/DPI do sistema nem acessibilidade universal.

`scripts/capture_home.py --binary build/local-ranking-history/projeto_glossata.exe` renovou a captura oficial real em [docs/images/tela-inicial.png](../../../images/tela-inicial.png) e seu registro. O PNG tem os mesmos pixels anteriores; o registro agora vincula o binário 1.12.0 e os novos hashes do controlador. Não se alteraram pixels artificialmente para gerar diferença no Git. O caminho do README permanece intacto.

## Inventário não destrutivo

O manifesto bruto gerado permanece em `build/build-manifest.json`, com cópia preservada em `build/local-ranking-history/archived-build-manifest.json`; seu hash está em `verificacoes/resultados.json`. A auditoria impediu versionar esse artefato de build, sem alterar suas regras. Os logs versionados desta melhoria usam LF conforme o padrão de `.gitattributes`, preservando os hashes em checkouts Windows/Linux.

Executados **depois dos testes/builds e antes da abertura da PR**: `python scripts/cleanup_build.py` e `python scripts/cleanup_build.py --include-old-builds`, ambos em `dry-run`. Os dois planos retornaram **zero candidatos elegíveis**, por recência/retenção. Nenhum `--apply` ou comando de exclusão foi executado.

O [inventário complementar](verificacoes/auditoria-arquivos.json) registra caminhos, bytes, motivos, riscos e quatro grupos de PNGs locais com bytes iguais. As cópias do build para esta documentação são intencionais. Hash igual não é autorização de exclusão.

| Caminho/agrupamento | Tamanho aproximado | Motivo e risco; ação |
| --- | --- | --- |
| `build/local-ranking-history/` | 1,79 GiB | Build atual, scripts locais, capturas e logs; risco alto para a rastreabilidade; preservar. |
| `build/tests/previews/` | 225,3 MiB | Capturas de etapas anteriores; risco alto; preservar. |
| `build/__pycache__/` | 172 KiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `scripts/__pycache__/` | 1,30 MiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `tests/__pycache__/` | 565 KiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `build/*.{py,txt,log,png,tmp}` | 38,9 MiB | 283 auxiliares/relatórios/capturas históricos, possivelmente únicos; risco alto; preservar. |

Os [planos padrão](verificacoes/cleanup-default.json) e [estendido](verificacoes/cleanup-extended.json) detalham os builds anteriores e as proteções aplicadas. `resources/`, `package/`, SQLite/dados pessoais, governança, manifestos, documentação e evidências anteriores permanecem preservados. Nenhuma exclusão de arquivo versionado faz parte desta melhoria.

## Limitações futuras

Histórico é uma consulta de **recordes registrados**, não uma lista completa de partidas. Cada UUID pode ter um recorde por configuração armazenada. A fonte continua explicitamente local e offline, sem conta, servidor, API, PostgreSQL, sincronização, ranking global ou importação de resultados antigos. Uma fonte global futura precisa de seu próprio contrato e políticas; esta melhoria não cria esse serviço nem concede autorização de distribuição.
