# Escrita serializada dos históricos de governança — INV-05

Correção local preparada em 2026-10-16 sobre a recuperação de permissões do INV-02. Aplicativo **1.7.0** e catálogo **2.4.0** permanecem não lançados. Não há mudança em decisões humanas, evidências, organismos, fotografias, quizzes, interface ou bancos dos jogadores.

## Diagnóstico e alcance

Após INV-02, permissões e importações formais compartilhavam uma trava em `build/curation/governance.lock`. Porém, `content_governance.append_review` e `distribution.record_dependency_review` continuavam lendo o topo e acrescentando eventos sem adquirir essa trava. As CLIs diretas alcançavam esses dois caminhos. Dois escritores podiam construir eventos sobre o mesmo topo, formando uma cadeia inválida; o bridge isolado não prevenia essa concorrência.

A solução centraliza validação de cadeia, comparação do topo e commit em `governance_storage.append_event`. Os três históricos usam a mesma trava persistente, junto aos arquivos protegidos: **`resources/compliance/.governance.lock`**. A remoção de `build/` não remove essa proteção.

| Arquivo/estado | Entradas de escrita identificadas | Garantia atual |
| --- | --- | --- |
| `review_history.jsonl` | `append_review`; CLI `content_governance.py record-review`; bridge `record_decision` do tipo `review` | Validação e escrita sob a trava comum; commit por `append_event`. |
| `permission_history.jsonl` e `photo_provenance.json` | `confirm_permission`; CLI `confirm-permission`; bridge do tipo `permission` | Mesmo escritor comum para o evento; projeção recuperável sob a mesma trava, com idempotência do INV-02. |
| `dependency_reviews.jsonl` | `record_dependency_review`; CLI `distribution.py record-review`; bridge do tipo `dependency` | Mesmo protocolo para validação do inventário, evidências e commit. |
| Projeção de proveniência | `register_photo_provenance` no importador; `permission_recovery --apply`; confirmação | Mesma trava. Importação não pode sobrescrever metadados divergentes; recuperação não cria decisão. |
| Baseline legada e proveniência inicial | `migrate` / `_migrate` | Mesma trava e preservação das decisões existentes; recusa de recriação automática de baseline ausente se já houver eventos de permissão. |
| Evidências e fontes editáveis | `archive_permission`, preparação de mídia, manifestos, classificação e textos | Não acrescentam eventos de decisão por conta própria. São insumos auditados; alterações continuam exigindo o fluxo de governança e manutenção coordenada. Não se declara uma transação global sobre todos os arquivos do catálogo. |
| Créditos, educação gerada, relatórios e cópia para pacote | Geradores, auditores, exportação local | Não acrescentam decisões. Podem detectar transições durante escrita e falhar conservadoramente; executar novamente após a manutenção. |

As funções internas `_append_review`, `_confirm_permission`, `_record_decision`, `_record_dependency_review`, `_migrate` e a recuperação com aplicação verificam a posse da trava; chamá-las diretamente sem a posse é recusado. A trava do bridge cobre também sua leitura do predecessor e a idempotência. Sua presença não é mais a única proteção do histórico.

## Protocolo comum

1. Adquirir o bloqueio exclusivo do sistema operacional antes de validar o conteúdo e ler o predecessor. Reentrância é permitida somente para a mesma thread, processo e diretório canônico; permite que o bridge chame as APIs públicas sem liberar a proteção intermediariamente. Threads e processos diferentes não compartilham a posse.
2. Ler e validar a cadeia completa. Um histórico truncado, divergente ou com hash inválido impede a gravação; não há descarte de linhas nem recomposição automática dos hashes antigos.
3. Passar ao escritor comum o último `event_sha256` observado. O escritor confere esse topo contra sua leitura atual. Ele próprio atribui `previous_sha256` e calcula `event_sha256`; o chamador não pode fornecer esses campos no corpo.
4. Preparar um temporário exclusivo no mesmo diretório, contendo **todos os bytes anteriores** e o novo evento. Sincronizar seu conteúdo com `flush`/`fsync`.
5. **Imediatamente antes de `os.replace`**, verificar novamente a posse/identidade da trava, a validade da cadeia, o último hash e a igualdade de todos os bytes do arquivo original. Mudanças inesperadas recusam o commit, preservando o arquivo encontrado.
6. Substituir atomicamente o ledger e sincronizar o diretório em POSIX. Manter a trava durante os efeitos adicionais da operação, como o replay da projeção de permissões. Liberar ao concluir ou lançar uma exceção.

`canonical`, `digest` e `read_chain` agora são definidos na camada comum e continuam disponíveis pelos imports de `content_governance`, preservando os consumidores. O cálculo SHA-256 é o mesmo; eventos anteriores não são reserializados. Cada evento novo custa cópia e validação proporcionais ao tamanho do ledger, escolha compatível com o volume atual e com o Git.

Uma segunda decisão do bridge sobre o mesmo tópico ainda precisa conhecer a decisão anterior. Se duas propostas partiram do mesmo estado, uma pode ser recusada após a primeira gravação, exigindo comparação humana. A ferramenta não atualiza automaticamente a expectativa do mantenedor para aprovar a segunda proposta.

## Locks, falhas e operação

O bloqueio usa `msvcrt.locking` no Windows e `fcntl.flock` em POSIX. A posse é do descritor aberto; o arquivo permanece no disco depois da liberação. **Não apague, mova ou substitua `.governance.lock` para desbloquear a manutenção.** Seu conteúdo não representa uma aprovação, não contém dados pessoais e não constitui sentinela de processo ativo. A identidade do arquivo também é conferida antes de gravar.

Por padrão, a aquisição não espera: conflito gera `FileExistsError` antes da escrita. Código de manutenção pode usar `governance_lock(root, timeout=...)`, entre 0 e 30 segundos, com relógio monotônico e espera limitada. O prazo rege a aquisição, não cancela à força uma decisão que já está sendo gravada. Não há roubo de trava por idade, exclusão automática nem espera infinita. O chamador pode tentar novamente após o escritor terminar; conflitos de decisão exigem comparação, não retentativa automática com expectativa alterada.

Exceções liberam a posse em `finally`. Encerrar o processo também fecha seus descritores e libera o bloqueio; o arquivo pode continuar existindo sem impedir a próxima operação. Interrupção antes da substituição deixa o histórico antigo; depois dela, o evento completo permanece. Para permissões, a projeção atrasada segue o [procedimento do INV-02](RECUPERACAO_PERMISSOES.md). Não se acrescenta evento compensatório nem se apaga decisão para restaurar uma cadeia.

**Atualização de instalações antigas:** finalize os processos de manutenção antes de trocar os scripts. A trava antiga em `build/curation/governance.lock` não é o novo ponto de coordenação; não há garantia de interoperabilidade com uma versão antiga já carregada em memória. Uma antiga `build/curation/decision.lock` é recusada conservadoramente: verifique seu processo proprietário antes de remover somente essa sentinela legada. A trava `sync.lock` da publicação de Issues continua independente e não concede aprovação de conteúdo.

O arquivo operacional é ignorado pelo Git, rejeitado pelo auditor se for adicionado à força e explicitamente excluído do manifesto e da cópia de evidências para o pacote. Não se exclui outro arquivo de licença ou histórico desses controles. O caminho canônico faz aliases do mesmo diretório de históricos usarem a mesma trava. Clones com arquivos separados não compartilham uma trava global: uma eventual mesclagem de históricos divergentes precisa de avaliação humana; não se apagam eventos para fazer o merge passar.

Os bloqueios são cooperativos, destinados aos escritores do projeto em sistema de arquivos local. Não impedem um administrador de editar arquivos por fora, não substituem controle de acesso ou assinatura e não constituem protocolo distribuído de rede/nuvem. A comparação final detecta interferências observadas antes do commit; não autentica autores nem elimina corridas de um escritor que ignore deliberadamente o protocolo. Não foram simuladas falhas físicas de energia ou armazenamento. Não há alteração do gate de distribuição.

## Validação

```powershell
python tests/test_governance_concurrency.py
python tests/test_permission_recovery.py
python tests/test_content_governance.py
python tests/test_curation_decisions.py
ctest --test-dir build --output-on-failure
python scripts/content_governance.py recover-permissions
python scripts/audit_catalog.py --strict
python scripts/release.py check
python scripts/audit_repository.py --staged
```

A suíte `GlossataGovernanceConcurrencyTests`, registrada no CTest e portanto no check `offline-validation`, usa somente catálogos temporários. Copia os scripts para esses diretórios para executar os entrypoints reais de CLI com `ROOT` isolado. Não faz consultas externas nem registra permissão no catálogo real.

| Cenário | Evidência automatizada |
| --- | --- |
| Quatro processos com revisões distintas | Todos os eventos aceitos preservados, prefixo anterior idêntico, predecessores únicos e cadeia completa verificável. |
| CLI científica e bridge simultâneos | Ambos integram a mesma cadeia, usando tópicos distintos. |
| API, CLI e bridge de dependências | Três componentes sintéticos, todos preservados e encadeados. |
| API, CLI e bridge de permissões | Mesma operação vinculada gera um evento e projeção consistente; idempotência do INV-02 mantida. |
| Escritor pausado depois de consultar o topo | Doze chamadas concorrentes, cobrindo CLIs, APIs, bridge, migração, importação e recuperação, recusadas sem acrescentar evento. |
| Remoção completa de `build/` durante escrita | A trava persistente permanece; segunda CLI é recusada e a escrita posterior funciona. |
| Timeout e morte de processo | Espera expira no prazo; terminar abruptamente o proprietário permite nova operação. Espera limitada também adquire a posse após liberação normal. |
| Falha antes/depois do commit | Processos de revisão científica e dependências encerrados antes da substituição e após ela; eventos já gravados preservados e novas escritas válidas. INV-02 mantém seus testes de recuperação entre evento e projeção. |
| Threads e reentrância | Quatro threads mantêm a cadeia; chamadas aninhadas do bridge reutilizam somente a própria posse. |
| Duas propostas do mesmo tópico | Uma aceita; a outra exige comparação com a decisão registrada, sem substituí-la. |
| Topo antigo e interferência após sincronizar temporário | Commit recusado; prefixo, evento encontrado e decisão anterior preservados. |
| Cadeia corrompida, posse ausente e timeout inválido | Escrita recusada sem reparação destrutiva. |
| Manifesto, exportação e Git | Trava operacional excluída; evidência real da fixture preservada; adição forçada ao índice recusada. |

Resultados executados localmente no Windows em 2026-10-16:

- Configuração CMake e build incremental Release concluídos; **19/19 suítes CTest aprovadas em 121,26 s**.
- Nova suíte: **16 testes**, incluindo processos simultâneos e CLIs reais em cópias temporárias; 10,30 s no CTest. Suíte do INV-02: **14 testes aprovados**, 16,90 s. Governança (23), bridge (11), versionamento (13) e auditor do Git (5) também aprovados na regressão.
- Auditoria estrita aprovada para os 122 organismos; zero erros técnicos de governança. Créditos regenerados sem diferença, manifesto com os mesmos hashes e recursos/`src`/`package` preservados byte a byte no Git.
- Inspeção real de permissões: `consistent`, zero eventos, nenhuma pendência de materialização e `applied: false`. Nenhum registro real foi alvo dos testes ou de recuperação.
- `release.py check`, histórico desde `075dbd1`, proposta de proteção e sintaxe do workflow (`actionlint`) aprovados. Nova suíte incluída no CTest executado pelo check técnico existente.
- Distribuição continua bloqueada por 120 pendências fotográficas e sete de outros materiais, sem flexibilização dos gates. Nenhum push, release, tag, configuração remota ou decisão científica/licenciamento alterado.

A ramificação POSIX não foi executada neste ambiente. Os resultados técnicos não constituem aprovação científica, comprovação de titularidade ou certificação jurídica.
