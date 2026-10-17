# Persistência e recuperação de permissões — INV-02

Correção local em 2026-10-16, no desenvolvimento de **1.7.0 ainda não lançado**. Catálogo **2.4.0**, decisões humanas, evidências, fotografias e dados dos jogadores preservados. Nenhuma permissão real foi criada ou confirmada durante esta intervenção.

## Diagnóstico e escolha arquitetural

O fluxo anterior acrescentava um evento em `resources/compliance/permission_history.jsonl` e depois substituía `resources/data/photo_provenance.json`. Uma falha entre essas operações deixava a projeção antiga. O bridge em `curation_decisions.py` reconhecia o hash da decisão e retornava `already_recorded`, sem recuperar seus efeitos. A escrita por append também podia deixar uma última linha incompleta em uma interrupção.

| Alternativa | Avaliação para o projeto |
| --- | --- |
| Eventos como fonte de verdade e projeção recuperável | Adotada. Os eventos existentes já contêm `before`, `after`, encadeamento e hashes. Permite recuperação sem converter decisões ou introduzir um banco adicional. JSON/JSONL continuam inspecionáveis no Git e embarcáveis offline. |
| Journal transacional separado | Exigiria conciliar um terceiro arquivo, seu estado de commit e sua limpeza com o histórico existente. Não acrescenta informação necessária à recuperação: o evento já contém o estado completo da fotografia. |
| SQLite transacional com exportação | Viável para crescimento futuro, mas criaria uma segunda representação operacional e uma migração de fonte de verdade, exportações e políticas de conflito para todos os consumidores. A necessidade atual é resolvida reutilizando os eventos existentes. |

A atomicidade é **por arquivo**. Não se afirma que JSONL e JSON formam uma transação de sistema de arquivos: um intervalo com projeção atrasada é possível e detectável. O evento válido é o commit duradouro da decisão; a projeção precisa ser recuperada antes de confirmar sucesso ou reutilizar uma decisão.

## Contrato de escrita e repetição

1. Adquirir a trava compartilhada dos escritores de governança. Verificar o histórico inteiro e recuperar eventuais projeções atrasadas, antes de processar outra confirmação.
2. Validar o organismo/fotografia atual, licença, versão, prova localizada, arquivos de evidência, hashes e declaração humana com os validadores existentes. HTTP e estado de Issues continuam sem autorizar conteúdo.
3. Calcular `operation_sha256` a partir do ID da foto, da confirmação vinculada ao conteúdo e, quando existir, do rastreamento da decisão de curadoria. O horário de registro não participa da identidade da operação.
4. Se a operação já existe, verificar a projeção, as evidências e se ela continua sendo a decisão mais recente daquela foto. Uma decisão substituída não pode reverter outra mais recente. O bridge só devolve `already_recorded` após essas verificações.
5. Preparar o histórico anterior **byte a byte**, seguido do novo evento. Escrever em temporário exclusivo no mesmo diretório, executar `flush`/`fsync` e substituir atomicamente o destino. Nenhum evento antigo é reserializado ou removido. A operação de armazenamento substitui o arquivo; o histórico lógico permanece de acréscimo.
6. Reconstituir o estado mais recente daquela foto a partir do evento comprometido. Gravar a projeção pelo mesmo protocolo de temporário, sincronização e substituição. Só então devolver sucesso. Não renovar `reviewed_at`, trocar responsável ou criar outra decisão durante o replay.

Uma interrupção anterior à substituição do histórico deixa a operação ainda não registrada; a repetição a registra uma vez. Depois dessa substituição, a decisão já existe: a recuperação aplica o `after` existente e não acrescenta outro evento. Interromper novamente a recuperação também é seguro para repetição. Temporários de um processo encerrado abruptamente não são interpretados como eventos nem adotados como autorização.

Eventos antigos sem `operation_sha256` são aceitos sem reescrita: sua identidade é derivada do mesmo conjunto de confirmação e rastreamento. A cadeia anterior e os hashes permanecem válidos.

## Recuperação e estados bloqueados

No diretório do projeto, primeiro faça uma inspeção sem escrita:

```powershell
python scripts/content_governance.py recover-permissions
```

O relatório informa `state`, `event_count`, `pending_photo_ids`, `errors` e `applied`. `consistent` retorna código 0 e significa concordância entre eventos e projeção; não é uma aprovação de direitos. As auditorias de evidência/licenciamento continuam necessárias mesmo nesse estado. `recovery_required` e `blocked` retornam código 2. Esse comando não consulta provedores, não atualiza revisões e não cria arquivos de decisão. Em seguida, para aplicar somente eventos já existentes:

```powershell
python scripts/content_governance.py recover-permissions --apply
python scripts/content_governance.py recover-permissions
python scripts/content_governance.py credits
python scripts/audit_catalog.py --strict
python scripts/distribution.py check
```

Repetir a confirmação original ou sua importação pelo bridge também tenta recuperar a projeção, sob a mesma trava. A recuperação explícita é útil quando o processo foi interrompido e não se deseja registrar uma nova decisão. Créditos/arquivos derivados devem ser regenerados e verificados após uma recuperação que os afete; os hashes do catálogo devem ser atualizados pelo fluxo normal de manutenção quando seus insumos mudarem. Uma recuperação não libera o gate de distribuição.

O replay só avança se o estado da foto corresponde exatamente ao `before` inicial ou a algum `after` conhecido, as transições são coerentes e as provas necessárias ainda passam nos validadores. O plano completo é validado antes de qualquer escrita da projeção. Metadados de outras fotos são preservados.

Exigem avaliação humana: cadeia inválida/truncada, hashes adulterados, transições divergentes, operação duplicada/inválida, confirmação sem evento, estado materializado desconhecido, arquivo da projeção ausente ou ilegível, fotografia removida/alterada e evidência indisponível ou incompatível. Não se presume que uma mudança posterior de fonte revogue uma licença CC válida; o bloqueio impede concluir automaticamente a reconciliação.

Nesses casos, preserve os arquivos conflitantes e os temporários fora do controle de versão, com acesso adequado. Compare-os com o histórico Git e backups verificáveis. Não remova eventos, não refaça hashes para ocultar divergências, não altere responsável/data e não copie uma confirmação para outra fotografia. Uma restauração precisa preservar os eventos e quaisquer decisões posteriores; conflitos sem prova suficiente continuam bloqueados. O comando não oferece uma opção de força. Perda integral da projeção não é reparada automaticamente, pois os eventos não necessariamente contêm a proveniência das fotos nunca confirmadas.

## Travas, consumidores e limites

`governance_storage.py` usa um bloqueio não bloqueante do sistema operacional sobre `build/curation/governance.lock`: `msvcrt.locking` no Windows e `fcntl.flock` em POSIX. O arquivo permanece após o uso; **não o remova para liberar uma operação**. O fechamento do descritor, inclusive por término abrupto do processo, libera a trava. Outro escritor recebe erro e pode tentar novamente depois. Uma antiga `decision.lock` é recusada conservadoramente; confira se o processo antigo terminou antes de remover apenas essa sentinela legada. A trava de sincronização de Issues é independente.

| Fluxo | Comportamento nesta correção |
| --- | --- |
| `content_governance.confirm_permission` e CLI | Mesma trava, validação, identidade idempotente, commit do evento e replay da projeção. |
| `curation_decisions.record_decision` | Mesma trava para importações formais. Permissões recuperadas antes da verificação de binding/repetição; decisões anteriores não são substituídas pela repetição. |
| `import_inaturalist.py` | Registro de proveniência sob a mesma trava; adição idempotente e recusa de sobrescrita divergente, inclusive de confirmações humanas. Importação não concede permissão de distribuição. |
| Migração legada | Mesma trava, preserva proveniência existente e recusa recriação automática da baseline se já há eventos de permissão. |
| `credits` / `generate_credits` | Detectam projeção inconsistente como pendência de direitos. Não executam replay nem ocultam os créditos existentes. |
| `audit_governance`, `audit_catalog`, fila de curadoria e distribuição | Auditoria inclui inconsistência entre histórico e projeção como erro técnico; consumidores existentes recebem o achado e mantêm o bloqueio. |
| `records` / geração educativa / recursos offline | Formato e semântica da proveniência mantidos; CI inspeciona recuperação antes da geração. Atualização de créditos e revisões derivadas continua explícita. |
| C++/QML, quizzes e bancos dos jogadores | Sem mudança. Nenhuma ferramenta de recuperação acessa os bancos dos jogadores. |

Use um sistema de arquivos local com substituição atômica e execute editores/escritores de governança de forma coordenada. A trava é cooperativa e local: não coordena clones em computadores diferentes ou edições manuais que a ignorem. Os leitores de auditoria podem observar uma transição e falhar conservadoramente; execute novamente após o escritor terminar. O custo de cada novo evento é proporcional ao tamanho do ledger, pois seu prefixo completo é copiado. SQLite permanece uma opção futura se esse volume se tornar um impedimento. O problema de concorrência dos escritores científicos legados de INV-05 não é declarado resolvido por esta mudança de permissões.

O arquivo é sincronizado antes da substituição; em POSIX o diretório também é sincronizado depois. O Windows não usa essa operação POSIX de diretório. As garantias dependem do sistema de arquivos, SO e armazenamento: testes de processo interrompido não reproduzem falha física, perda de energia, corrupção do volume ou sincronização em nuvem. A recuperação não é backup, assinatura digital nem certificação de titularidade. Cadeias de hashes detectam alterações, mas não autenticam o autor contra alguém capaz de reescrever toda a cadeia.

Referências de implementação: documentação oficial de [`os.fsync` e `os.replace`](https://docs.python.org/3.11/library/os.html#os.fsync), [`msvcrt.locking`](https://docs.python.org/3.11/library/msvcrt.html#msvcrt.locking) e [`fcntl.flock`](https://docs.python.org/3.11/library/fcntl.html#fcntl.flock). Somente recursos disponíveis no Python 3.11+ são usados; sem dependência nova.

## Verificações e evidência de execução

`tests/test_permission_recovery.py` utiliza exclusivamente diretórios temporários e as fixtures sintéticas existentes. Cobre falhas antes/depois de escrever o temporário, sincronizar, substituir e concluir a escrita dos dois arquivos; inclui encerramento por `os._exit` em subprocessos e recuperação em outro processo. Verifica repetição direta e pelo bridge, falhas de I/O, recuperação interrompida, compatibilidade com eventos antigos, preservação byte a byte do prefixo do histórico, decisões posteriores, metadados não envolvidos, evidências/bytes alterados, cadeia truncada/corrompida, estado desconhecido e exclusão mútua.

A suíte é registrada no CTest como `GlossataPermissionRecoveryTests`. `offline-validation` executa também `recover-permissions` **sem `--apply`**, antes de gerar conteúdo: CI reporta divergências, nunca aprova ou repara silenciosamente decisões. `distribution-permissions` continua separado e com suas pendências legítimas.

Resultados executados localmente no Windows em 2026-10-16:

| Verificação | Resultado |
| --- | --- |
| Configuração CMake e build incremental Release | Concluídos; nenhuma alteração em C++/QML. |
| CTest completo, após ajuste da fixture legada de diretório | **18/18 suítes aprovadas, 103,55 s**. A primeira execução detectou apenas a preparação de teste que pressupunha a ausência de `build/curation`; a asserção de exclusão mútua foi preservada. |
| Nova suíte de recuperação | **14 testes**, incluindo 10 pontos de interrupção por exceção, 10 por saída abrupta em subprocesso, cinco interrupções da própria recuperação e duas falhas de I/O, além dos cenários de inconsistência/legado. Aprovada também dentro do CTest. |
| `recover-permissions`, sem aplicar | `consistent`, zero eventos, nenhuma pendência de materialização e `applied: false`. |
| Auditoria estrita do catálogo | Aprovada: 122 organismos e 122 registros fotográficos (120 arquivos distintos), sem erros técnicos de governança. |
| Créditos e manifesto | Regeneração idêntica dos 120 créditos de fotos do quiz; insumos do manifesto e recursos/`src` preservados. |
| Fila de curadoria | Gerada localmente, sem publicar Issues: 15 solicitações agrupadas e 3.354 achados. |
| Versionamento, histórico e proteção proposta | `release.py check`, `check-history --base 7954f08` e `repository_protection.py check` aprovados; nenhuma regra remota alterada. |
| Workflow | `actionlint` aprovou `.github/workflows/quality.yml`; teste adicional incluído no check técnico. |
| Distribuição | **Permanece bloqueada**: 120 pendências de licenciamento fotográfico e sete de outros materiais, sem erro técnico novo. Nenhum gate flexibilizado. |

Na inspeção real: 120 entradas de proveniência, histórico de permissões vazio e nenhuma confirmação humana materializada. Portanto, não havia decisão real a recuperar ou aprovar. Catálogo e evidências reais não foram usados como alvos de falhas injetadas. Os testes validaram a implementação Windows; a ramificação POSIX não foi executada neste ambiente. Não se realizou ensaio de perda de energia, teste em volume de rede ou alteração de configuração remota. Nenhuma revisão científica, identidade de revisor ou decisão de licenciamento foi modificada. Aplicativo 1.7.0 e catálogo 2.4.0 permanecem não lançados; sem tag, push ou release.
