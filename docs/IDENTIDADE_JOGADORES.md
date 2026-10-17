# Identidade permanente de jogadores — INV-01

Correção do desenvolvimento **1.7.0**, ainda não lançado. A etapa de identidade passa de `PRAGMA user_version=3` para **4**, seguida pela [política Unicode do INV-07, esquema 5](NOMES_UNICODE.md); isso é independente da versão **2.4.0 do catálogo científico**, preservada. A implementação não altera regras, pontuações, fotografias, créditos, permissões ou conquistas. Testes e capturas usam exclusivamente bancos temporários.

## Diagnóstico e política

Antes, `player_profiles.name` era a chave do perfil, enquanto `game_runs`, `history` e `ranking_records` repetiam `player_name`. Renomear atualizava somente a primeira tabela. Consultas pessoais e métricas procuravam o nome novo, deixando de encontrar resultados anteriores. Excluir e recriar um nome podia juntar resultados de pessoas diferentes. A partida preservava somente uma cópia do nome.

Agora o domínio distingue `PlayerIdentity { id, displayName }` e `PlayerProfile`, com estado ativo, excluído ou legado. O ID é um UUID criado uma vez, persistido como texto e protegido contra atualização no SQLite. O nome é um atributo editável; não é chave de resultado. Renomear não atualiza a propriedade das partidas nem seus snapshots.

| Operação | Comportamento |
| --- | --- |
| Criar perfil | Gera ID novo; não busca resultados antigos com o mesmo nome. |
| Renomear | Mantém ID, resultados, métricas e chaves de ranking. A exibição atual usa o novo nome. |
| Iniciar partida | Copia ID e nome de exibição para `GameSession`. A identidade não depende mais do perfil selecionado ao terminar. |
| Trocar seleção | Afeta a próxima partida. A partida atual conserva a identidade capturada. |
| Renomear durante partida | Mantém o ID; o nome no snapshot continua sendo aquele do início. |
| Excluir perfil | Marca `deleted_at`; a identidade e os resultados permanecem. O perfil sai da seleção e seus resultados continuam identificados como perfil excluído. |
| Encerrar partida de perfil excluído | Salva sob o ID originalmente capturado, sem recriar o perfil nem transferir a partida. |
| Reutilizar um nome livre | Cria outro ID e outro histórico pessoal. Não herda resultados de perfil excluído ou renomeado. |
| Falha de gravação e retentativa | Reutiliza ID do jogador, ID da partida e snapshot originais. O mesmo ID de partida com proprietário diferente é recusado. |
| Reiniciar aplicativo | Os IDs vêm do banco; não são recriados. A seleção inicial continua seguindo a ordenação de perfis existente, sem mudar a propriedade dos dados. |

Os novos conflitos de nomes usam chave NFC + `QString::toCaseFolded()` + NFC, aparando espaços externos. O INV-07 centraliza e versiona essa política de folding simples, audita as chaves e documenta suas [equivalências e limitações](NOMES_UNICODE.md). Colisões Unicode já existentes não são unidas nem renomeadas automaticamente: recebem IDs distintos, ficam registradas nas notas da migração e podem ser selecionadas por ID. Novas criações/renomeações conflitantes são recusadas por triggers. A interface mantém o limite de 32 unidades UTF-16 já aplicado aos nomes.

## Esquema SQLite 4

| Tabela | Mudança |
| --- | --- |
| `player_profiles` | `id TEXT PRIMARY KEY NOT NULL`, `name`, `name_key`, `origin`, `deleted_at`, `legacy_name_key`. `origin` distingue `profile` de `legacy_unresolved`. Triggers impedem alteração do ID e exclusão física; estados legados não são perfis jogáveis. |
| `game_runs` | Acrescenta `player_id REFERENCES player_profiles(id)`. Migração preenche todas as partidas existentes. Inserção sem ID é recusada por trigger; troca de proprietário e alteração de `player_name` também são recusadas. A coluna antiga de nome vira snapshot do início de cada partida. |
| `ranking_records` | Chave passa a `(ranking_key, player_id)`. Melhores/últimos pontos, acertos e datas preservados; nome da última partida permanece como snapshot. |
| `history` | Resumo da última partida por `player_id`, com FK e unicidade por identidade. Não é usado para atribuir proprietários pelo nome. |
| `answer_records` | Sem alteração de pontuação ou snapshot: continua relacionado ao ID da partida, que agora tem proprietário permanente. |
| `legacy_history_results` | Mantém IDs originais, nomes, unidades, acertos e datas; recebe `player_id` de agrupamento legado não resolvido. |
| `identity_migration_notes` | ID envolvido, tabela/chave de origem e motivo da decisão mecânica de migração. Não simula revisão humana. |
| `legacy_player_profiles_v3`, `legacy_history_v3`, `legacy_ranking_records_v3` | Arquivos lógicos das tabelas anteriores dentro do próprio banco. Preservam registros e valores da etapa compatível com v3; não são apagados após a cópia. |

O sufixo `_v3` representa a etapa de esquema anterior à identidade, inclusive quando o banco chegou de v0/v1/v2. A conversão preexistente de `score` em pontos para `score_units` multiplica por seis somente quando a coluna ainda é `score`; a pontuação exibida permanece equivalente. Não há novo cálculo de pesos, acertos, recordes ou regras competitivas nesta correção.

`game_runs.player_id` é acrescentado como coluna que permite NULL para atender às regras de `ALTER TABLE ... REFERENCES`; o preenchimento é validado e as inserções futuras exigem valor por trigger. As relações de ranking/resumo declaram `NOT NULL` diretamente. `PRAGMA foreign_keys=ON` e `foreign_key_check` protegem as relações, e há índice em `(player_id, ranking_key)` para métricas.

## Migração sem presumir pessoas

As versões antigas não registraram IDs, renomeações nem exclusões suficientes para provar quem era o dono de um nome. **Mesmo um nome exatamente igual ao de um perfil atual não é evidência de propriedade.** A migração não associa automaticamente esses resultados ao ID de nenhum perfil ativo.

1. Abrir a transação SQLite que também engloba as atualizações anteriores de esquema necessárias.
2. Arquivar as tabelas anteriores, mantendo todas as linhas. Criar um ID para cada perfil existente, sem ligá-lo a resultados por coincidência de nome.
3. Criar identificadores `legacy_unresolved` para os agrupamentos que o esquema antigo fazia por `COLLATE NOCASE`. São identificadores de conjuntos de dados, **não uma afirmação de que cada conjunto corresponde a uma única pessoa**. Não se aplica a nova normalização Unicode para juntar esses conjuntos.
4. Copiar os rankings existentes integralmente, com os mesmos valores e datas. Associar partidas e resumos antigos somente aos respectivos conjuntos legados. Conservar todas as linhas de `history` no arquivo lógico; o resumo novo seleciona a última por conjunto, sem descartar o arquivo de origem.
5. Registrar motivos e origens, instalar constraints/triggers, verificar FKs e atualizar `user_version=4` dentro da mesma transação. Qualquer falha desfaz DDL, IDs criados, cópias e vínculos.
6. Em novas aberturas v4, verificar FKs e reutilizar o esquema, sem repetir migração, gerar IDs ou acrescentar notas. Esquemas futuros desconhecidos são recusados.

Isso **preserva o ranking antigo**, mas as métricas pessoais do perfil atual podem começar sem partidas atribuídas: os resultados anteriores continuam nos conjuntos legados, visíveis no ranking e identificados como **“legado, vínculo pendente”**. A tela de estatísticas explica essa separação; não se trata de pontuação apagada ou ranking reiniciado. Resultados de partidas novas passam a acompanhar o perfil normalmente, inclusive após renomeação.

Uma associação retroativa requer evidência de identidade e procedimento específico autorizado, com testes e trilha de decisão. Não foi implementado um botão de reivindicação por nome nem uma fusão automática. Não edite `player_id`, apague linhas antigas ou remova triggers para fazer métricas coincidirem. Partidas anteriores eventualmente misturadas pelo mesmo nome não podem ser separadas com segurança a partir de dados que o banco nunca guardou.

Para inspeção local em ferramenta SQLite **somente leitura**, sem publicar nomes/bancos:

```sql
PRAGMA user_version;
PRAGMA foreign_key_check;
SELECT id, name, origin, deleted_at FROM player_profiles;
SELECT player_id, source_table, source_key, reason FROM identity_migration_notes;
SELECT id, player_id, player_name, score_units, correct_answers FROM game_runs;
```

Antes de atualizar uma instalação real, feche o aplicativo e preserve uma cópia do banco e de eventuais arquivos WAL/SHM conforme seu estado de armazenamento. O programa antigo recusa `user_version=4`: não há downgrade automático. Não restaure uma cópia anterior sobre partidas novas sem reconciliação. Nesta intervenção não foram abertos nem migrados bancos pessoais; a migração real ocorrerá quando a nova compilação abrir o banco dessa instalação.

## Arquitetura e validação

`PlayerIdentityMigration.cpp` concentra a mudança de esquema e é chamado na transação do repositório. Os ports de histórico recebem `PlayerIdentity`; não existe sobrecarga de gravação por nome. `GameService.saveCurrentResult()` usa somente a identidade da sessão e não aceita outro jogador no encerramento. Uma sessão sem identidade pode ser usada em testes de perguntas, mas sua tentativa de persistência é recusada. O controlador captura um perfil ativo no início e consulta métricas/recordes por ID. QML seleciona perfis pelo valor `playerId`, mantendo o texto do nome.

Os métodos de conveniência por nome remanescentes são limites de seleção/edição explícita de perfis ativos. Não consultam resultados, não buscam identidades legadas e não participam da gravação das partidas. Comparação exata resolve seleções antigas; uma busca normalizada ambígua é recusada.

Testes dedicados em `PlayerIdentityTests.cpp` cobrem criação/UUID, renomeação, reabertura, exclusão lógica, recriação/reutilização de nomes, retentativa com proprietário incorreto, invariância do snapshot, nomes Unicode, migração v3, preservação de respostas/fotografia, idempotência, rollback de migração interrompida e casos sem evidência de propriedade. `GameServiceTests.cpp` cobre também a identidade capturada no início, troca/renomeação/exclusão/recriação durante a partida e recusa de salvar sem identidade. Os cenários anteriores v0/v1/v2, pontuação, ranking competitivo e interface permanecem na regressão.

```powershell
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
python scripts/capture_home.py
python scripts/audit_catalog.py --strict
python scripts/release.py check
```

Resultados locais em 2026-10-16 (horário de Brasília):

| Verificação | Resultado |
| --- | --- |
| Compilação C++17/Qt 6 Release | Concluída com a API de gravação exclusivamente por identidade; todas as chamadas de produção e fixtures adaptadas. |
| Regressão CTest | **20/20 suítes aprovadas em 102,49 s**. |
| Identidade dedicada | Dez cenários, mais inicialização/finalização: **12 verificações QtTest aprovadas**. Migração v3, rollback, idempotência, ambiguidades e preservação de snapshots/valores testados com bancos temporários. |
| Domínio/controlador | **50 verificações QtTest aprovadas**, incluindo novos cenários de troca, renomeação, exclusão e recriação durante a partida, e recusa de persistência sem identidade inicial. Migrações anteriores e políticas de pontuação continuam testadas. |
| Interface | Suíte QML aprovada; `qmllint` dos dois views alterados sem achados. |
| Captura real | Executada por `capture_home.py`, conferida visualmente, em 1240 × 820, banco temporário e Discord desabilitado. PNG idêntico ao anterior porque a aparência inicial não mudou; registro de origem atualizado com o binário e os fontes atuais. |
| Catálogo e governança | Auditoria estrita aprovada para os 122 organismos; insumos/hashes científicos e de licenciamento idênticos. Inspeção de permissões consistente, sem aplicar recuperação. |
| Versionamento e índice | Metadados, histórico desde `62b4f53` e auditoria do índice aprovados. Aplicativo 1.7.0 e catálogo 2.4.0 ainda não lançados; sem tag, push ou release. |
| Pacote local | Atualizado com `package.ps1 -Development`; `validate_package.ps1` iniciou o executável com PATH isolado, sem dependências carregadas do MSYS2 externo e usando banco temporário. Marcador de desenvolvimento/impedimento de redistribuição mantido. |

O empacotador avisou sobre traduções do framework Qt ausentes e componentes opcionais de Direct3D 12 não encontrados. O teste do pacote e a captura usam renderização software; não representam teste de instalação limpa do Windows ou de todas as GPUs. As mensagens próprias do Glossata permanecem em português. Os scripts revisados rodaram com `-ExecutionPolicy Bypass` somente nos processos temporários de empacotamento, sem alteração persistente da política do sistema.

Nenhum teste abre ou escreve em bancos reais. A correção não implementa recuperação de partidas ainda em memória (INV-03) nem certificação de integridade contra edição administrativa do SQLite. O catálogo e os bloqueios de distribuição permanecem independentes desta migração.

Referências de implementação: documentação oficial de [ALTER TABLE e suas restrições](https://www.sqlite.org/lang_altertable.html) e [transações SQLite](https://www.sqlite.org/transactional.html). As garantias de migração são verificadas também pelos testes de rollback sobre a implementação Qt/SQLite usada pelo projeto.
