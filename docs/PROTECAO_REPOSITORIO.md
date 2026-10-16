# Proteção do repositório para manutenção individual

## Situação verificada e escopo

O INV-04 foi reavaliado em **16/10/2026**, sem assumir que o diagnóstico anterior permanecia válido. A [consulta registrada](VERIFICACAO_INV04.json) foi somente de leitura, usando o GitHub CLI autenticado. O repositório é público e sua branch padrão é `main`.

| Item remoto observado | Resultado |
| --- | --- |
| Branch `main` | `protected: false` |
| Rulesets e regras efetivas para `main` | Listas vazias |
| Check `offline-validation` | `success`, emitido pelo GitHub Actions, app ID `15368` |
| Check `distribution-permissions` | `failure`, gate de direitos/curadoria já documentado |
| Permissão padrão dos workflows | `read` |
| Permissão para Actions aprovar PRs | `false` |
| Ações permitidas globalmente | `all`; exigência global de SHA desativada |

Esse estado é um snapshot da hora da consulta, não uma garantia permanente. Não houve criação, edição ou exclusão de ruleset, mudança de permissão, push, PR remoto ou release nesta implementação.

| Entrega | Estado |
| --- | --- |
| Payload de proteção, CODEOWNERS, template de PR, ajustes da CI e testes | Implementados localmente, dependem de integração autorizada |
| Proteção aplicada e validada no servidor | **Pendente de autorização/ativação** |
| Teste real de rejeição de force push/exclusão na `main` | **Não executado**; não se testa destruição na branch real |
| Revisão científica ou permissão de uso de recursos | Não alterada |

## Política preparada

[`.github/rulesets/main-solo.json`](../.github/rulesets/main-solo.json) é um payload para criar um **branch ruleset** no GitHub. O campo `enforcement: active` descreve o estado pretendido após uma ativação autorizada; a existência do arquivo no Git não configura o servidor.

| Regra proposta | Motivo e efeito |
| --- | --- |
| Somente `refs/heads/main`, sem exclusões | As branches de trabalho continuam disponíveis |
| `deletion` | Bloqueia a exclusão da branch principal |
| `non_fast_forward` | Bloqueia force push e reescrita por esse caminho |
| `pull_request`, **zero aprovações obrigatórias** | Mantém diff e registro de integração sem exigir outra pessoa |
| Aprovação de CODEOWNERS e do último push desativadas | O mantenedor não depende de um segundo revisor inexistente |
| Conversas resolvidas | O próprio mantenedor pode resolver comentários antes de integrar |
| Apenas `offline-validation`, integração `15368` | Exige a validação técnica do GitHub Actions observada; não exige o gate de distribuição |
| Branch atualizada antes da integração | O check precisa cobrir a base atual; se `main` avançar, atualizar a branch de trabalho e executar novamente |
| Nenhum ator de bypass permanente | O administrador segue a rotina normal de PR/checks; uma emergência exige mudança administrativa explícita e registrada |

PRs sem aprovações obrigatórias são suportados por rulesets: o objetivo aqui é registro e validação técnica. O mantenedor pode integrar o próprio PR quando o check passar. Não há obrigatoriedade de assinatura, histórico linear, merge queue, deployments, equipes ou revisores adicionais. Consulte as [regras disponíveis](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/available-rules-for-rulesets).

Se a CI quebrar, a correção continua possível em uma branch e PR que restabeleça o check. Indisponibilidade do GitHub/runner pode atrasar integrações; não se introduz um bypass silencioso como solução permanente. A permissão administrativa de editar uma regra continua sendo uma raiz de confiança.

## Checks técnicos e gates de distribuição

| Workflow/job | Finalidade | Obrigatório para integrar desenvolvimento? |
| --- | --- | --- |
| `quality.yml` / `offline-validation` | Build, CTest, QML, reprodução de derivados, integridade, versões e proposta de proteção | **Sim, após ativação do ruleset** |
| `quality.yml` / `distribution-permissions` | Expor bloqueios de direitos, curadoria e distribuição | **Não**; continua falhando quando as evidências não bastam |
| `curation.yml` / `audit` | Preparar fila e simular acesso ao GitHub em leitura | Não; tem agenda e acionamento próprios |
| `curation.yml` / `synchronize` | Sincronizar Issues dentro dos limites autorizados | Não; pode ser corretamente ignorado em simulação |

O nome obrigatório é exatamente `offline-validation`, não o nome do workflow nem sua conclusão global. O workflow de qualidade pode continuar vermelho porque o gate de distribuição falha, enquanto o check técnico está aprovado. O ruleset não transforma essa aprovação em licença ou autorização para publicar.

Não adicionar `continue-on-error`, remover validadores ou produzir um check fictício para satisfazer a proteção. Não usar filtros de caminhos ou condição no job obrigatório que eliminem a validação de mudanças somente documentais. Não reutilizar o mesmo nome de check em outro workflow. A CI continua usando `push`, `pull_request` e acionamento manual; não foi introduzido `pull_request_target`.

O lançamento permanece sujeito ao procedimento de [VERSIONAMENTO.md](VERSIONAMENTO.md), ao gate completo de distribuição e à autorização de publicação. Nenhuma tag ou release é criada por esta proposta.

## Permissões dos workflows

| Contexto | Permissões necessárias | Situação local |
| --- | --- | --- |
| Qualidade: ambos os jobs | `contents: read` | Mantida; checkout agora usa `persist-credentials: false` nos dois jobs |
| Curadoria: `audit` | `contents: read`, `issues: read` | Preservada |
| Curadoria: `synchronize` | `contents: read`, `issues: write` | Preservada somente no job já autorizado para Issues |
| Project opcional | Credencial específica, apenas nos passos que a utilizam | Não criada, ampliada ou ativada |

Nenhum workflow recebe `contents: write`, aprovação de PR, escrita em checks/statuses ou configuração administrativa. A geração de artefatos continua com as permissões existentes. O nome do check técnico foi explicitado e foram definidos limites de execução; a publicação semanal de curadoria e seus controles não mudam.

O padrão remoto já é leitura e Actions não pode aprovar PRs. A política global permite quaisquer ações, mas as ações usadas pelos workflows atuais estão fixadas por SHA e isso é conferido nos testes. Restringir a lista global é uma decisão adicional: exige autorização e avaliação das dependências de ações compostas. A disponibilidade dessa configuração varia com a plataforma/plano. Referência: [permissões do GITHUB_TOKEN](https://docs.github.com/en/actions/tutorials/authenticate-with-github_token).

## Arquivos sensíveis e um único mantenedor

O [CODEOWNERS](../.github/CODEOWNERS) declara `@kaduolliver` para `.github/`, scripts, testes, dados, evidências, versões e documentação, inclusive o próprio arquivo de responsáveis. Isso fornece visibilidade e responsabilidade para contribuições; não impede sozinho a edição nem certifica uma revisão.

O [template de PR](../.github/PULL_REQUEST_TEMPLATE.md) orienta a conferir alterações/exclusões, evidências, hashes anteriores e testes. A autoverificação do mantenedor não deve ser descrita como revisão independente ou aprovação científica. Uma mudança de conteúdo continua passando pelos ledgers e mecanismos existentes; um PR integrado não concede direitos ou aprova uma espécie.

A regra preparada mantém `require_code_owner_review: false`: exigir aprovação do próprio autor poderia inviabilizar a manutenção individual. Não se usam contas auxiliares ou bot para fingir essa aprovação. O check técnico e seus testes reduzem regressões; o mesmo mantenedor que pode editar o código também pode editar verificações e regras, limitação que permanece explícita. [Funcionamento do CODEOWNERS](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-code-owners).

## Validação local e inspeção somente de leitura

Na raiz do projeto:

```powershell
python scripts/repository_protection.py check
python tests/test_repository_protection.py
python scripts/repository_protection.py inspect
```

`check` valida o contrato específico do payload local e não acessa a rede. Os testes também verificam invariantes dos workflows. `inspect` usa somente `gh api --method GET` e grava `build/repository-protection-inspection.json`, com data real, hash da proposta, regras efetivas, bypasses quando visíveis, permissões e checks. Não existe comando `apply` no script.

Falhas de autenticação, privilégios, API ou paginação não são tratadas como proteção ausente. O inspetor aborta em vez de afirmar uma visão completa com listas de 100 ou mais itens. `bypass_actors: null` significa campo ausente/não visível, não ausência de bypass. Em branches protegidas, a proteção clássica deve ser conferida separadamente em **Settings → Branches**; regras clássicas e rulesets podem coexistir. A saída de inspeção não é um teste de imposição pelo servidor.

### Verificações desta implementação

- Configuração CMake Release e build incremental concluídos sem erros.
- CTest: **16/16 suítes passaram**, em 76,65 s, com bancos temporários; inclui sete casos novos de proposta, bloqueio de exigências inadequadas ao mantenedor, permissões, inspeção sem escrita e tratamento de metadados incompletos.
- `repository_protection.py check`, auditoria estrita do catálogo e `release.py check` passaram. Os 122 organismos e o manifesto 2.4.0 permanecem íntegros.
- O inspetor foi exercitado de verdade com GETs, sem modificar o servidor; a consulta atual está no JSON de evidências. Os testes de falhas/permissões usam mocks, não tentativas de escrita remota.
- Os testes de workflows conferem invariantes do formato usado; não são um parser YAML geral nem uma execução do GitHub Actions. `actionlint` não estava disponível neste ambiente. A CI desta implementação ainda não foi executada remotamente porque não houve publicação da branch.
- Não houve mudança visual, de pontuação, banco pessoal ou curadoria. Aplicativo 1.7.0 e catálogo 2.4.0 continuam não lançados; não há nova versão ou tag por uma alteração administrativa ainda em preparação.

## Procedimento de integração e ativação — pendente

1. Revisar os commits locais e os resultados dos testes. Publicar a branch de trabalho e abrir PR **somente quando essas ações forem autorizadas**; não fazer push diretamente para `main`. O relatório investigativo anterior também está em um commit local ainda não enviado.
2. Integrar por PR após o check técnico passar. Conferir que `offline-validation` é emitido pelo GitHub Actions no commit atual; a integração `15368` foi observada nesta coleta e deve ser revista se a origem do check mudar.
3. Obter autorização explícita para a configuração remota. Em **Settings → Rules → Rulesets**, importar o JSON preparado ou criar as mesmas regras manualmente. Conferir **branch**, alvo `main`, **Active**, bypass vazio, zero aprovações e somente o check técnico. Importar JSON altera o servidor e exige essa autorização.
4. Se o ruleset ainda não existir e houver autorização específica, a API aceita o payload abaixo. Antes de criar, consultar os existentes e evitar regras duplicadas. Alterações de um ruleset existente usam seu ID real e revisão do diff; o script fornecido não realiza nenhuma delas.

```powershell
# SOMENTE após autorização explícita; este comando NÃO foi executado.
gh api --method POST repos/kaduolliver/Projeto-Glossata/rulesets --input .github/rulesets/main-solo.json
```

5. Reexecutar `inspect`, conferir condições, regras efetivas e bypasses na interface/API. Registrar ID/URL, responsável, data real e correspondência com o payload. Enquanto isso não acontecer, o status é **pendente**, mesmo que todos os testes locais passem.
6. Validar operacionalmente com um PR legítimo: verificar bloqueio enquanto o check está pendente e integração possível com `offline-validation` aprovado e zero aprovações. Confirmar que a falha de distribuição não bloqueia essa integração. Esse exercício remoto também depende de autorização.
7. Não tentar force push ou exclusão na `main` real como teste. Se for necessário exercício destrutivo, usar repositório ou branch descartável, regra equivalente, escopo autorizado e conta sem bypass. A leitura da configuração real e o ensaio descartável devem ser registrados como evidências distintas.

### Limitações e alternativas da plataforma

Branch rulesets estão disponíveis em repositórios públicos no GitHub Free, conforme a [documentação de criação](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/creating-rulesets-for-a-repository). Este repositório foi observado como público; mudar sua visibilidade/plano exige reavaliar disponibilidade e imposição. `evaluate` não foi usado: sua disponibilidade é diferente de `active`/`disabled`.

A [API de rulesets](https://docs.github.com/en/rest/repos/rules#create-a-repository-ruleset) requer administração em escrita para ativação, mas as consultas não justificam elevar o token dos workflows. Metadados incompletos ou erros não demonstram que uma regra deixou de existir.

Se a interface/plano não permitir a política proposta, registrar a limitação e preparar proteção clássica: impedir force push e exclusão, exigir o check técnico e incluir administradores quando suportado. Não improvisar uma exigência de aprovação externa nem adicionar `distribution-permissions`. Essa alternativa deve ser revisada e autorizada separadamente; não foi aplicada ou testada aqui. Uma regra clássica não é necessariamente equivalente em comportamento de bypass e combinação de regras.

Durante falha de runner/serviço, o desenvolvimento local e branches de trabalho continuam possíveis. Uma exceção administrativa emergencial deve ter autorização, motivo, período e restauração registrados; não se adiciona bypass permanente para contornar checks. GitHub Actions app ID identifica o emissor, mas não impede um workflow autorizado de mudar o conteúdo do check. A segurança continua dependendo do controle da conta administradora, das integrações e da conferência dos diffs.
