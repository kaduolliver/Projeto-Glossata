# Auditoria de estabilização técnica

Auditoria de **17/10/2026**, iniciada sobre `main` no commit `5d739e7817754a9b7da8e982088a3ff224cf559f`, em branch própria `fix/auditoria-estabilizacao`. Escopo: integração das correções INV-01 a INV-10; sem funcionalidades novas, expansão do catálogo, mudança de pontuação, revisão científica ou autorização de direitos. Os estados dos relatórios históricos foram confrontados com código, testes e consulta remota atual.

## 1. Estado inicial

- Árvore de trabalho limpa. O PR #7 havia integrado os commits investigativos, incluindo o ajuste de portabilidade do teste Git no Windows.
- Aplicativo **1.7.0** e catálogo **2.4.0**, ambos ainda não lançados; regras `identification-v4-progressive`; histórico SQLite 6, catálogo SQLite 2 e snapshot 1. As correções compatíveis desta auditoria integram o desenvolvimento existente, sem tag ou release.
- **122 organismos e 120 fotografias distintas do quiz**. Manifesto, dados, fotografias, evidências e históricos de curadoria preservados. A comparação Git dos insumos científicos permaneceu vazia.
- CMake Release e compilação inicial aprovados; **25/25 suítes CTest**, 152,68 s, antes das alterações.
- [Consulta GitHub](VERIFICACAO_ESTABILIZACAO_GITHUB.json) somente GET, às **13:36:55 UTC**: `main.protected=true`, ruleset **25303072 ativo**, sem bypass. Check técnico da base aprovado e gate de distribuição reprovado.

Nenhum banco pessoal real foi aberto, copiado ou migrado. Todos os bancos de teste usam diretórios temporários; os processos filhos também recebem caminhos temporários. Não houve contato com titulares, coleta de novas mídias ou alteração de permissões administrativas.

## 2. Componentes investigados

| Área | Implementação e cruzamento investigados | Conclusão técnica |
| --- | --- | --- |
| Identidade, Unicode e legado (INV-01/07) | `SqliteGameHistoryRepository`, `PlayerIdentityMigration`, `PlayerNameMigration`, `PlayerNamePolicy`, perfis do controlador | UUIDs novos gerados por `QUuid`; nome/snapshot editáveis separados do proprietário; colisões antigas conservadas, sem associação por nome. |
| Migração SQLite | Construção do repositório, versões antigas, DDL transacional, chaves e notas de migração | Fixtures anteriores à identidade e versões 3, 4 e 5 percorrem migração para 6; rollback e repetição preservam resultados, IDs e snapshots. Nova verificação recusa estruturas incompletas. |
| Sessões e pontuação (INV-03) | `GameService`, `GameController`, `Main.qml`, `SessionSnapshot`, `SessionCheckpointStorage` | Checkpoint precede confirmação da ação; retomada mantém alternativas, respostas, dicas, tetos e dono; finalização transacional/idempotente. |
| Rankings e métricas | Respostas versionadas, `saveConfiguredResult`, configurações e consultas por ID | Partida incompleta não entra no competitivo; nome reutilizado não herda resultados; acertos e pontos permanecem independentes. |
| Backup | `player_backup.py`, testes Python e restauração de sessão pelo Qt/SQLite | API de backup inclui WAL confirmado; publicação exclusiva, manifesto/hash e destino separado. Lacunas de validação corrigidas. |
| Governança (INV-02/05/09) | `governance_storage`, `confirm_permission`, recovery, bridge e auditoria de histórico | Escritores usam a mesma trava reentrante por thread/processo; eventos/projeção recuperáveis e idempotentes; comparação com base Git conserva histórico. Hashes não autenticam revisores. |
| Transporte (INV-06) | `PublicClient`, política de destinos, resolvedor, TLS, redirecionamentos e consumidores | HTTPS/443, hosts explícitos, DNS público fixado e validação por salto; limites preservados. Regressão inteiramente simulada, sem conexões reais de teste. |
| Direitos e fontes (INV-08) | Relatório offline, proveniência/créditos e gate existente | Sem novas aprovações ou mudanças científicas. Pendências permanecem como pendências, sem conclusão jurídica fictícia. |
| Documentação/CI (INV-04/10) | Metadados gerados, workflows, ruleset e permissões | Snapshot remoto atualizado; documentos antigos mantidos como históricos; fonte de versões continua verificável. |

A inspeção não encontrou necessidade de outro mecanismo de governança, dependência nova ou refatoração do núcleo. Não foram adicionados testes que repetissem indiscriminadamente os existentes: as novas regressões se concentram nas lacunas reproduzidas e na validação semântica de snapshots.

## 3. Problemas reproduzidos e correções

| Caso | Reprodução antes da correção | Correção e resultado esperado |
| --- | --- | --- |
| EST-01 — esquema atual incompleto | Banco temporário com `user_version=6` continuava disponível após remover trigger de nome, trigger de identidade do checkpoint, tabela de checkpoints ou renomear coluna de respostas. Quatro casos falharam no teste que exigia recusa. | Verificar tabelas/colunas, objetos de proteção e referências necessários na abertura e antes do commit da migração. Recusar sem reconstrução, exclusão ou alteração de versão/dados. |
| EST-02 — manifesto inválido | Manifestos JSON `[]`, `null`, número ou string provocavam `AttributeError`; versões booleanas/fracionárias podiam ser aceitas por igualdade numérica do Python. | Exigir objeto, inteiros suportados e digest SHA-256 válido antes de restaurar; erro controlado e nenhum destino publicado. |
| EST-03 — destino com sidecars SQLite | Um destino sem arquivo principal, mas com `-wal`, `-shm` ou `-journal`, não era recusado antecipadamente. Houve publicação antes do erro e aceitação de `-shm` preexistente. | Considerar todo o conjunto de nomes do destino, incluindo links, e conferir novamente antes da publicação. Preservar sidecars, não publicar sobre eles e não remover arquivos existentes. |

As reproduções usam danos deliberados em fixtures, não demonstram corrupção de um banco real nem perda efetiva de dados pessoais.

Durante a integração da nova verificação, a suíte completa detectou exigência excessiva de FK declarada em uma tabela de respostas legada e a extração documental dependente da forma da guarda de versão. Ambos foram corrigidos antes da entrega: vínculos legados são conferidos por consulta explícita, sem reconstruir a tabela; a guarda superior permanece explícita para o extrator de metadados. O teste legado passou a verificar lista não vazia antes de acessar o primeiro resultado, evitando crash secundário e mantendo as asserções sobre pontos. A existência de respostas órfãs também é recusada quando o schema antigo não declara FK.

## 4. Preservação e limites das correções

Não se altera o número do esquema, a identidade do catálogo ou as regras dos jogos para mascarar divergências. A verificação de esquema é um contrato mínimo dos objetos necessários, não comparação criptográfica de todo SQL nem proteção contra administrador hostil. Estruturas incompletas/ambíguas ficam bloqueadas para investigação humana. Nenhum reparo automático é inferido.

O histórico pré-ID permanece em identidades legadas de propriedade não demonstrada. Colisões Unicode existentes não são unidas. Um jogador excluído logicamente continua dono de partidas anteriores; outro perfil com o mesmo nome tem outro ID. UUID identifica o registro local, sem autenticar uma pessoa.

Na recuperação, resposta confirmada permanece no prefixo imutável e o próximo item é a questão já persistida. Dicas/tetos permanecem no snapshot; a retomada não renova avaliação nem recalcula pontuação antiga. Uma sessão `finished` só entra no ranking na finalização atômica, e o UUID impede duplicação. Snapshot incompatível pode ser abandonado explicitamente, conservando o registro.

Banco e manifesto de backup continuam sendo dois arquivos, sem promessa de transação entre ambos. Interrupção depois da publicação exclusiva do banco pode deixar cópia sem manifesto válido, recusada para restauração; crie outro backup com novo nome. Hard links são necessários; formatos incompatíveis falham em vez de sobrescrever arquivos. Escritores externos não cooperativos ainda podem disputar sidecars após a conferência; o utilitário não promete exclusão mútua sobre uma pasta controlada por outro processo.

## 5. Testes adicionados

- `PlayerIdentityTests.cpp`: seis cenários de recusa de schema/dados incompatíveis, duas reaberturas por cenário e preservação de IDs/resultados. Inclui ausência de objetos críticos, violação referencial e esquema futuro. Novo teste de migração v5 incompleta confirma rollback do DDL/versão, repetição e recuperação da fixture após correção explícita.
- `GameServiceTests.cpp`: migração legada conserva ranking e unidades; detecção de resposta órfã mesmo sem FK declarada; diagnóstico seguro se a consulta não produzir linhas.
- `SessionRecoveryTests.cpp`: quatro snapshots com hash correspondente, mas total, gerador, hash fotográfico ou regras incompatíveis; recusa após reabertura, registro preservado, ranking vazio e abandono explícito. Não se confunde hash válido com estado válido.
- `test_player_backup.py`: passou de cinco para **11 testes**. Novos cenários cobrem tipos de manifesto, sidecars preexistentes/tardios, interrupção depois de publicar o banco, interrupção da cópia de restauração e exclusão de dados WAL não confirmados. Continuam cobertos WAL confirmado, corrupção/hash, destino existente, FK e esquema futuro.

## 6. Verificações executadas

Esta seção é um registro da validação local, não atualização automática de resultados futuros da CI.

| Verificação | Resultado |
| --- | --- |
| Build Release C++17 / Qt 6.11.2 / GCC 16.2.0 | Recompilação do aplicativo e executáveis de teste concluída. |
| Inicialização do binário recompilado | `projeto_glossata.exe --smoke-test`, saída 0, usando banco temporário próprio e Discord desativado; renderizador software/offscreen. |
| CTest inicial | 25/25, 152,68 s, antes das correções. |
| CTest final | **25/25 suítes aprovadas**, 145,22 s, após os ajustes de compatibilidade; sem testes ignorados pelo CTest. |
| Backup Python | 11/11 em Python 3.11 e 3.14; somente bancos temporários. |
| Sintaxe Python | `python -m compileall -q scripts tests` aprovado. |
| QML | Todos os arquivos em `resources/qml` passaram em `qmllint --unqualified disable`; testes de interface usam renderização real software/offscreen. Sem alegação de inspeção manual de todas as telas/GPUs. |
| Catálogo e fotografias | `audit_catalog.py --strict`, `exploration_media.py` e `phylogeny.py` em modo de auditoria aprovados. 122 organismos; 109 fotos de exploração, 623 cartões com foto ou pendência documentada. |
| Dicas | `review_hints.py` e `audit_hint_assistance.py` aprovados; avaliações automáticas continuam separadas de revisão humana. |
| Derivados | Educação gerada em memória com escrita interceptada, créditos JSON/documento e relatório filogenético comparados aos arquivos existentes: correspondentes. Nenhum insumo científico gravado. |
| Atualizações/direitos | `review_updates.py` e `--rights-report` executados offline, gerando somente propostas/relatórios em `build/`. |
| Governança | Projeção de permissões consistente em consulta sem aplicação; suítes de concorrência, falhas injetadas, recuperação de eventos e histórico integram o CTest completo. |
| Repositório/versões/captura | Auditoria de arquivos, metadados e histórico de captura conferidos antes da publicação da branch. Sem alteração visual, não foi necessária nova captura inicial. |
| CI da base | [Execução 39032867571](https://github.com/kaduolliver/Projeto-Glossata/actions/runs/39032867571): `offline-validation=success`; distribuição bloqueada. A CI desta entrega é consultável no PR da branch de auditoria, sem atribuir a ela o resultado da base. |

A revisão automática de aprovação recusou a execução local de `phylogeny.py --apply`, que poderia gravar conteúdo científico. Foi usada a auditoria sem aplicação e a comparação dos derivados em memória. Não foi contornado o bloqueio nem renovada uma revisão para satisfazer testes. A CI existente permanece intacta, incluindo suas verificações de reprodução de derivados em checkout descartável.

O gate local `distribution.py check` retornou **2**, como esperado: **zero erros técnicos**, **120 pendências fotográficas**, **sete achados de outros materiais** e **dois bloqueios científicos de distribuição**. O relatório também conserva outros estados científicos/pedagógicos e mídias locais fora do bundle. Esses números são achados documentais, não prova automática de infração. Não houve empacotamento/publicação; as obrigações de dependências de um futuro pacote exigem seu gate separado. O pacote local preexistente não foi atualizado nem apresentado como versão validada desta entrega.

## 7. Riscos residuais

- Sem ensaio de corte físico de energia, falha de disco ou todos os antivírus/sistemas de arquivos. `synchronous=FULL` e `fsync` dependem das garantias do armazenamento.
- Backup interrompido pode deixar arquivos incompletos; não são removidos ou sobrescritos automaticamente. Manifesto/hash detecta divergência, sem autenticar autoria.
- Versão Unicode do Qt diferente exige migração explícita; não se promete compatibilidade silenciosa entre todas as futuras versões de Qt.
- Bancos irregulares, decisões ambíguas e snapshots incompatíveis permanecem conservados e bloqueados. Não há reconstrução adivinhada ou fusão por nome.
- Jogo offline sob controle administrativo local não é sistema antifraude autenticado. Git/ruleset e hashes são complementares; um único mantenedor pode alterar verificadores e regras por ação administrativa autorizada.
- Pendências científicas e de direitos impedem distribuição. Aprovação técnica não as resolve e não certifica revisão independente ou autoria.

## 8. Testes manuais necessários

### Teste manual de uma cópia do banco real no Windows

**Preparado, não executado nesta auditoria.** O mantenedor deve controlar os caminhos e conservar toda cópia fora do Git, OneDrive e serviços externos. Não enviar banco, manifesto com dados adicionais ou relatório pessoal em Issue/PR.

1. Fechar o Glossata e conservar a instalação atual. Escolher uma pasta privada fora do repositório e nomes de saída que ainda não existam. Não copiar só o `.sqlite` de um processo aberto, ignorando o WAL.
2. Com caminhos explicitamente escolhidos pelo mantenedor, criar e verificar um backup consistente:

   ```powershell
   python scripts/player_backup.py create --source "CAMINHO_REAL_ESCOLHIDO.sqlite" --output "PASTA_PRIVADA\backup-novo.sqlite"
   python scripts/player_backup.py verify --source "PASTA_PRIVADA\backup-novo.sqlite"
   python scripts/player_backup.py restore --source "PASTA_PRIVADA\backup-novo.sqlite" --output "PASTA_PRIVADA\restauracao-nova.sqlite"
   ```

3. Guardar banco original e backup intactos. Usar **VM, Windows Sandbox sem rede ou conta Windows descartável**, sem acesso de escrita à instalação/perfil original. Disponibilizar apenas uma cópia do runtime de desenvolvimento recém-compilado e a restauração privada. Desabilitar Discord na configuração dessa cópia. Não confundir o pacote antigo com o binário recompilado da auditoria.
4. O executável normal não oferece `--database`; `--smoke-test` usa um banco novo e não testa a cópia. No ambiente descartável, confirmar o `AppDataLocation` da aplicação `glossata_quiz` e restaurar a cópia para o nome de banco esperado **somente se o destino e sidecars ainda não existirem**. Não supor que mudar `APPDATA` basta para isolar as pastas conhecidas do Windows. Não substituir o banco de uma conta existente para fazer o teste.
5. Abrir o jogo nesse ambiente isolado. Conferir contagens de perfis, IDs, resultados e pontos antes/depois da migração; observar aviso de vínculos legados/colisões. Renomear um perfil e verificar que seus resultados continuam associados; criar outro nome sem herdar histórico. Não editar IDs ou remover triggers.
6. Na cópia, iniciar partida, revelar dica, responder e interromper o processo. Reabrir e conferir mesma sessão/dicas/teto, próximo item correto e ausência de ranking parcial. Concluir sessão recuperada, reabrir e confirmar apenas uma finalização. Exercitar abandono e nome reutilizado, sem transferir resultados.
7. Registrar apenas conclusão técnica e versão/hash do binário em anotação privada. Se houver incompatibilidade, preservar os arquivos e registrar o bloqueio sem reparo manual improvisado. Descartar o ambiente de teste depois, conservando original/backup conforme a política pessoal. Não promover a cópia testada para produção automaticamente.

Também permanecem manuais: instalação limpa do runtime atual, navegação por teclado/leitor de tela, diferentes escalas/GPU e ensaio destrutivo de regras apenas em repositório descartável autorizado. Não tentamos excluir ou forçar histórico da `main` real.

## 9. Critérios de estabilidade

Para considerar o estado auditado tecnicamente estável: compilação aprovada, todas as suítes e auditorias técnicas aprovadas, `offline-validation` no commit do PR com a base atual, preservação demonstrada dos dados e nenhum defeito crítico conhecido de migração/recuperação/resultados sem tratamento seguro. Banco real, Windows limpo e GPUs não exercitados devem continuar explicitamente pendentes, sem alegação de validação universal.

Esta auditoria não concede aprovação de distribuição. A integração segue o ruleset, sem push direto para `main`, bypass, novo revisor fictício, alteração de permissões, tag ou release. O relatório descreve um escopo verificável de estabilidade técnica, sem afirmar regularização jurídica ou científica integral.

Os critérios locais foram atendidos: no estado final não permaneceu defeito crítico conhecido e reproduzido nos cenários de migração, recuperação e resultados exercitados. A conclusão é limitada à cobertura documentada; a evidência remota do commit submetido deve ser conferida no check `offline-validation` do PR. O teste da cópia do banco real e os demais ensaios manuais acima continuam pendentes.
