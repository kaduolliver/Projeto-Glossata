# Correção pontual da persistência — PR #40

Correção compatível sobre `1d390721af009543e04719c16b75450aac172354`, na mesma branch `feat/profile-settings-2-4-1` e na PR #40. Integra o aplicativo **2.3.0 ainda não lançado**, sem novo lançamento, release ou tag. Catálogo 2.4.0, SQLite 8 e os registros da entrega inicial permanecem preservados.

## Causa e correção

O carregamento substituía valores semanticamente inválidos por padrões, mas não registrava uma correção pendente. Como `dirty_` só era marcado por setters, `flush()` retornava antecipadamente e o botão não gravava as correções sem outra alteração.

`recoveryPending_` agora registra a recuperação separadamente de alterações do usuário. `flush()` continua sendo o caminho automático do timer/fechamento; `retrySave()` é a confirmação explícita usada pelo botão. `canRetryPersistence` informa se essa ação pode operar com segurança. O estado é centralizado em C++, sem cópia divergente em QML.

| Situação | Comportamento |
| --- | --- |
| Arquivo válido ou ausente | Carrega normalmente, sem gravação inicial; alterações reais mantêm agrupamento em 250 ms. Nenhuma mudança significa nenhuma escrita. |
| Valores semanticamente inválidos | Somente as opções inválidas usam padrões em memória. O aviso explica a situação e o botão grava as correções, mesmo sem setters. Valores válidos e chaves desconhecidas são preservados. |
| Falha de gravação | Valores continuam válidos na sessão e o botão permite repetir a operação após resolver o impedimento. O estado de recuperação continua pendente se necessário. |
| INI estruturalmente malformado | O original é preservado, a proteção de formato continua retida e o botão fica oculto. A mensagem orienta corrigir o arquivo externamente e reabrir o Glossata. |

Enquanto houver recuperação pendente, timer, fechamento e alterações em outras opções não a confirmam silenciosamente. Alterações da sessão aguardam o botão; se o aplicativo for fechado sem essa confirmação, o arquivo original permanece como estava. Após sucesso, os estados pendentes e o aviso são limpos. Uma nova tentativa sem mudanças não regrava o arquivo.

Os padrões continuam sendo movimento ativado, sons automáticos desativados e volume 35%. A ordem, superfícies e controles das cinco seções permanecem iguais; somente a mensagem e a disponibilidade da ação de erro foram corrigidas.

## Arquivos e limites

- `src/presentation/AppPreferences.h` e `.cpp`: estado de recuperação, confirmação explícita e proteção estrutural.
- `resources/qml/views/SettingsView.qml`: disponibilidade do botão e chamada de `retrySave()`.
- `tests/PreferencesAudioTests.cpp`: valores isolados/múltiplos inválidos, recuperação sem setters, fechamento, falha/nova tentativa, chaves futuras e idempotência por bytes e horário de modificação.
- `tests/QmlInterfaceTests.cpp`: estados válido, inválido, malformado e falha de escrita, acionando o botão real por teclado.
- `CHANGELOG.md`, registro da subetapa e metadados derivados: documentação da correção na versão ainda não lançada.

Não há alterações em identidade/UUID, SQLite, GameController, ranking, estatísticas, Quiz, conquistas, notificações, reprodução de áudio, animações, Discord, catálogo ou dependências. Todos os testes usam diretórios temporários e a saída gravadora de áudio existente. As evidências anteriores permanecem intactas.

## Validação

A [suíte completa CTest](../verificacoes/persistencia-ctest-completo.txt) aprovou **28/28 grupos**, sem falhas, em **681,47 s**. [Resumo, comandos, hashes e escopo preservado](../verificacoes/persistencia-resultados.json).

| Verificação | Resultado |
| --- | --- |
| [Preferências e áudio](../verificacoes/persistencia-unit.txt) | 22 casos aprovados, zero falhas/ignorados |
| [Interface afetada](../verificacoes/persistencia-ui-afetada.txt) | 10 casos aprovados, incluindo os seis estados de persistência, perfil e áudio |
| [Interface completa](../verificacoes/persistencia-ui-completa.txt) | 45 casos aprovados, zero falhas/ignorados |
| QML, metadados e escopo de código | Lint completo, project status, release check e inventário de código aprovados |
| [Governança](../verificacoes/persistencia-governanca.json) | 1.053 eventos e baseline preservados frente a `1d39072` |

As verificações abrangem arquivo válido sem escrita, volume/booleanos isolados ou múltiplos inválidos, padrões seguros, recuperação sem setters, acionamento do botão por teclado, chaves futuras, falha de gravação/nova tentativa, fechamento/reabertura e idempotência por bytes e horário de modificação. Os testes também comprovam que timer e fechamento não confirmam uma recuperação pendente silenciosamente.

[Manifesto das capturas reais](capturas.json), com fontes, executáveis, horários e SHA-256. São quadros do QML/controlador de produção no harness Qt isolado, em 1240×820 e escala 100%; nenhuma imagem foi simulada. Os nove quadros locais dos cenários foram preservados, com quatro selecionados para documentação:

| Estado | Captura |
| --- | --- |
| Valores recuperáveis, com botão | [Antes de salvar](capturas/invalid-volume-before.png) |
| Recuperação explícita concluída, aviso removido | [Depois do botão](capturas/invalid-volume-saved.png) |
| INI malformado, mensagem de intervenção e sem botão | [Arquivo preservado](capturas/malformed-before.png) |
| Falha de escrita, com nova tentativa disponível | [Falha de gravação](capturas/write-failure-before.png) |

Os arquivos de implementação alterados estão relacionados acima; a [lista completa da correção](../verificacoes/persistencia-arquivos-modificados.txt) inclui documentação e evidências. A Home, sua captura oficial e as evidências anteriores permanecem intactas.

## Inventário e limitações

Após encerrar builds e testes, foi feito apenas o [inventário em dry-run](../verificacoes/persistencia-cleanup-dry-run.json), sem exclusões durante a PR aberta. O script encontrou zero candidatos elegíveis. O [registro local](../verificacoes/persistencia-inventario.json) mantém caminho, tamanho, motivo e risco dos artefatos preservados.

| Caminho | Bytes | Motivo | Risco |
| --- | ---: | --- | --- |
| `build/profile-settings-2-4-1` | 323675715 | Aplicativo e testes recompilados, usados na rastreabilidade | alto |
| `previews/profile-settings-persistence-fix` | 672008 | Originais das capturas afetadas; quatro também versionadas | médio |
| `build/tests/previews/profile-settings-persistence-fix` | 671875 | Capturas dos mesmos cenários durante a suíte completa | médio |
| `scripts/__pycache__` | 1363308 | Cache Python regenerável | baixo |

Nenhuma limpeza foi aplicada. O INI malformado exige intervenção externa e reabertura; não há reconstrução automática nem monitoramento de edições externas. Enquanto houver recuperação semântica pendente, outras alterações de preferências da sessão também aguardam o botão para persistir. As pendências de distribuição/licenciamento já registradas na PR #40 continuam vigentes; o gate permanece bloqueado com código 2. Não houve nova dependência, aprovação científica/jurídica, merge, release ou tag.
