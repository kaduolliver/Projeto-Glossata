# Salvamento e recuperação de partidas — INV-03

Implementação do desenvolvimento **1.7.0**, ainda não lançado. O histórico SQLite passa ao **esquema 6**, independentemente do catálogo **2.4.0**, preservado. Não há alteração dos pesos das dicas, pontos históricos, IDs permanentes ou decisões de curadoria. Testes/capturas desta intervenção não abrem bancos pessoais.

## Diagnóstico e arquitetura

Antes, `GameService` acumulava respostas/dicas na memória. Ao submeter uma resposta, o serviço preparava a próxima questão, enquanto `GameController` mantinha a anterior no feedback. O SQLite recebia o resultado após o encerramento. Não havia tratamento de fechamento ou recuperação de sessão.

Agora `SessionCheckpointRepository` é uma porta independente de `GameHistoryRepository`, implementada pelo mesmo repositório SQLite. `SessionSnapshot` serializa/valida o contrato; o serviço controla transições e o controlador/QML apresentam recuperação e erros. Não há segundo banco, JSON externo de progresso ou dependência de rede.

| Coluna de `session_checkpoints` | Uso |
| --- | --- |
| `id` | UUID da partida, conservado na finalização idempotente em `game_runs`. |
| `player_id` | FK para a identidade permanente capturada no início. Nome inicial fica no snapshot. |
| `state` | `active`, `finished`, `finalized` ou `abandoned`. |
| `revision` | Inteiro crescente para compare-and-swap entre processos/janelas. |
| `payload` | Snapshot JSON versionado do motor, separado dos rankings. |
| `sha256` | Integridade dos bytes; não autentica o jogador nem impede edição administrativa. |
| `created_at`, `updated_at` | Datas reais das operações, em UTC. |

O payload inclui configuração/filtro/nível/dificuldade, regras/versão competitiva, identidade do conteúdo e versão pedagógica; jogador e nome inicial; enunciados, alternativas e ordens; identidade/hash fotográfico; textos, avaliações e evidências das dicas; respostas, tetos e pontos inteiros; dicas atuais; ordem do ciclo/pool, cursor e estado do gerador pseudoaleatório. Mídias/créditos permanecem no catálogo offline, sem duplicar arquivos no banco de jogadores.

Os snapshots conservam todas as questões geradas, inclusive a próxima questão preparada após uma resposta. Embaralhamento e geração compartilham o motor cujo estado é persistido. A retomada não sorteia novamente alternativas ou questões apresentadas. Métricas/sequências são reconstruídas a partir das respostas, verificando totais sem recalcular pontos com outro peso.

## Gravação e falhas

O início efetivo de uma partida grava o checkpoint inicial. A prévia usada pelos contadores da tela inicial não cria pendências apenas por abrir o programa. Se uma resposta/dica for utilizada nessa prévia, exige checkpoint inicial antes da ação.

Cada resposta/revelação usa `BEGIN IMMEDIATE`, `foreign_keys=ON`, `synchronous=FULL` e espera por lock limitada a dois segundos. O escritor verifica snapshot, último estado e revisão esperada. Não permite apagar respostas, alterar questões anteriores ou retirar dicas reveladas. Cada transição acrescenta no máximo uma resposta.

O commit ocorre **antes** de confirmar a resposta ou mostrar a dica na interface. Em falha, o serviço restaura o estado anterior em memória; o controlador não apresenta feedback nem avança como se a resposta tivesse sido confirmada. A mensagem permite nova tentativa ou retomada do estado persistido. Uma janela antiga não sobrescreve progresso mais recente. Falha de resultado ambíguo é resolvida pela revisão/estado do banco, sem presumir se houve commit.

O leitor verifica hash, formato, inteiros, identidade, alternativas distintas, ordens, provas/tetos e totais pela política central existente. Recuperação exige regras reconhecidas e a mesma identidade do conteúdo/pool carregado. Não regenera avaliações nem renova datas de revisão. Snapshots incompatíveis/corrompidos são conservados e recusados para continuação, com motivo; podem ser abandonados ou copiados para investigação, sem fabricar respostas ou corrigir silenciosamente registros.

Limites técnicos: **32 MiB por snapshot e 10.000 respostas por sessão**. A ação que ultrapassaria um limite é recusada sem perder o checkpoint anterior. Encerre a sessão confirmada antes de iniciar outra. O Livre mantém ciclos sucessivos e encerramento explícito dentro desses limites de armazenamento.

## Recuperação e encerramento

Ao reabrir, a interface lista sessões `active` e `finished`, indicando jogador, respostas confirmadas e estado:

- **Continuar sessão:** recupera o mesmo UUID/jogador, alternativas, pontos e dicas. Se a última resposta já foi confirmada, continua na próxima questão salva; não oferece nova tentativa da anterior. O feedback anterior não reabre automaticamente. Uma sessão pronta para finalização abre resultados e tenta concluir a gravação.
- **Abandonar:** pede confirmação. Atualiza somente estado/revisão, conserva o payload e não acrescenta resultado ao ranking. Fechar o diálogo/janela não abandona a sessão.

Novas partidas ficam bloqueadas enquanto houver pendências a resolver. Uma troca explícita de partida em andamento conserva o checkpoint anterior, que passa à lista de pendências. Seleção/renomeação/exclusão de perfil não transfere propriedade; nome reutilizado continua com outro ID. Uma sessão iniciada antes da exclusão lógica do perfil pode terminar sob o ID original, conforme INV-01.

`Main.qml.onClosing` chama `prepareToClose`: a partida ativa é suspensa, sem finalizar ou publicar ranking. Falha bloqueia o fechamento, oferecendo nova tentativa ou fechamento explícito com o último checkpoint confirmado. `aboutToQuit` também tenta preservar o estado em outras saídas normais; encerramento forçado não depende desse handler.

## Integridade do ranking

| Estado/ação | Efeito |
| --- | --- |
| Checkpoint ativo ou pronto para finalizar | Nenhuma escrita no ranking. |
| Última resposta competitiva | Grava primeiro `finished`, com as 20 respostas e tetos originais. |
| Finalização | Partida, respostas, ranking/resumo e passagem a `finalized` na **mesma transação**. |
| Falha na finalização | Rollback dos resultados/ranking; checkpoint `finished` continua recuperável. |
| Retentativa após commit | Mesmo UUID e comparação integral com o resultado anterior; sem duplicar partida/reaplicar ranking. |
| Encerramento competitivo antecipado | Mantém o contrato anterior: resultado incompleto, sem ranking competitivo. Essa decisão explícita encerra a possibilidade de continuação. |
| Encerramento explícito no Livre | Finaliza respostas confirmadas, como antes; fechar a janela apenas suspende. |
| Abandono | Conserva checkpoint, sem resultado final/ranking. |

`validAnswerScoring` e as validações competitivas existentes permanecem ativos. A finalização confere respostas, proprietário, configuração e totais contra o snapshot `finished` antes do commit. Não se usa upsert de ranking para representar progresso parcial.

## Migração e compatibilidade

v5→v6 é aditiva, transacional e idempotente: cria tabela, índice e triggers que conservam ID/proprietário e impedem exclusão física. Não altera resultados, perfis, chaves Unicode, notas ou arquivos lógicos anteriores. Bancos mais antigos percorrem as etapas de identidade/nomes na mesma transação. Falha reverte versão e DDL. Binários antigos recusam o esquema 6; não há downgrade automático.

Resultados históricos sem checkpoint continuam legíveis e a API anterior de gravação por ID permanece disponível. Se houver checkpoint com o UUID, a finalização exige estado e conteúdo correspondentes. Não é possível recuperar partidas perdidas antes desta implementação.

Na [estabilização de 17/10/2026](AUDITORIA_ESTABILIZACAO.md), a abertura do esquema 6 passou a conferir tabelas/colunas, guards de identidade/nomes/checkpoints e referências necessárias, também antes do commit de migração. Estruturas incompletas são recusadas, sem reconstrução automática. Tabelas antigas de respostas sem FK declarada continuam compatíveis: seus vínculos são conferidos explicitamente, sem reescrever linhas ou alterar pontos. Isso é um contrato mínimo de compatibilidade e integridade, não autenticação do SQLite ou prova contra um administrador que substitua schema e dados.

## Backup e restauração verificados

Checkpoints protegem contra interrupção do processo; não substituem cópia externa em caso de perda/corrupção do disco. `scripts/player_backup.py` usa a [API de backup SQLite](https://www.sqlite.org/backup.html), via [sqlite3.Connection.backup](https://docs.python.org/3/library/sqlite3.html#sqlite3.Connection.backup), incluindo dados confirmados em WAL. Não copia apenas o arquivo principal de um banco ativo. `synchronous=FULL` solicita durabilidade ao SQLite/sistema, sujeita às [garantias do armazenamento](https://www.sqlite.org/pragma.html#pragma_synchronous), sem garantir proteção contra qualquer falha física.

Exemplos manuais com destinos novos, privados e fora do repositório:

```powershell
python scripts/player_backup.py create --source "CAMINHO_DO_BANCO.sqlite" --output "BACKUP_NOVO.sqlite"
python scripts/player_backup.py verify --source "BACKUP_NOVO.sqlite"
python scripts/player_backup.py restore --source "BACKUP_NOVO.sqlite" --output "RESTAURACAO_NOVA.sqlite"
```

O utilitário valida `integrity_check`, FKs e esquema, e grava manifesto adjacente com SHA-256/data real, sem nomes ou linhas dos jogadores. Banco e manifesto devem permanecer privados. Usa arquivo temporário e publicação exclusiva, sem sobrescrever destinos existentes. Requer sistema de arquivos com hard links; em ambiente incompatível, recusa a publicação. Os dois arquivos não formam uma transação única: cópia sem manifesto após interrupção é recusada para restauração até haver novo backup válido.

Restauração confere manifesto/bytes copiados antes de publicar; recusa arquivo alterado ou backup com WAL ativo. Cria **outro arquivo**, sem substituir banco em uso. Teste essa cópia com a versão compatível antes de qualquer troca administrativa, com jogo fechado e instalação atual preservada. O aplicativo valida também snapshots e política Unicode; o utilitário verifica integridade física, sem essa avaliação lógica. Não mescle backups por nomes nem restaure dados antigos sobre partidas novas sem reconciliação. Não há backup periódico automático ou envio a terceiros.

A estabilização exige manifesto JSON como objeto, versões inteiras suportadas e SHA-256 no formato previsto. Um destino não é considerado novo se houver banco, manifesto, link ou `-wal`, `-shm`, `-journal` associado; a verificação é repetida antes da publicação. Esses arquivos não são apagados nem reutilizados. Um backup com journal/WAL não vazio também é recusado para restauração. A publicação exclusiva do banco continua protegida contra corrida pelo mesmo nome, mas não há transação do sistema de arquivos abrangendo banco, manifesto e escritores externos não cooperativos. Interrupção após publicar o banco pode deixar um par incompleto: preserve-o para investigação e crie outro backup com novo nome. O [procedimento manual em ambiente privado](AUDITORIA_ESTABILIZACAO.md#teste-manual-de-uma-cópia-do-banco-real-no-windows) não foi executado sobre dados pessoais nesta auditoria.

## Verificação e limites

Resultados locais em 2026-10-16 (Brasília), Qt 6.11.2 / C++17 Release:

| Verificação | Resultado |
| --- | --- |
| Compilação e CTest | **22/22 suítes**, 109,61 s. |
| Regressão após reforço final do escritor | Domínio/controlador, recuperação e backup: **3/3 suítes**, 29,38 s. |
| Recuperação dedicada | **17 verificações QtTest**: dicas/respostas, ordens/gerador, 19→20 respostas competitivas, finalização idempotente, falhas de gravação, rollback do ranking, abandono, conexões concorrentes, conteúdo/payload incompatíveis, migração, perfil excluído/nome reutilizado e restauração. |
| Interrupção real de processo | Filhos terminam com `std::_Exit` após dica/resposta/última questão e com transação aberta, sem executar destrutores. |
| Interface | **6 verificações QtTest**, incluindo banner, continuação por clique e confirmação de abandono. `qmllint` dos QML alterados sem achados. |
| Backup | **5 testes Python**, incluindo WAL, destino existente, manifesto ausente, hash inválido, FKs e esquema futuro. |
| Catálogo | Auditoria estrita dos 122 organismos aprovada; dados/fotos/licenças preservados. |
| Captura inicial | Programa real em 1240×820, banco temporário e Discord desativado; PNG/registro atualizados no caminho estável do README. |
| Pacote local | Atualizado com `package.ps1 -Development`; `validate_package.ps1` iniciou o binário com PATH isolado e banco temporário. Marcador de desenvolvimento mantido. |

Não foram testados corte físico de energia, falhas de hardware ou edição hostil do SQLite. SHA-256 não converte um jogo offline administrado pelo usuário em sistema antifraude autenticado. As capturas foram conferidas no renderizador software, sem alegação sobre todas as GPUs. Nenhum teste concede permissão ou revisão científica/jurídica.

O empacotador manteve avisos sobre traduções Qt ausentes e componentes opcionais de Direct3D 12. O teste aprovado não equivale a instalação limpa do Windows ou validação de todas as GPUs. Empacotamento usou `-ExecutionPolicy Bypass` somente em processos filhos, sem modificar a política persistente do sistema. Não houve push, tag, release ou liberação do gate de distribuição.
