# Integridade, rastreabilidade e autoria — INV-09

Revisão local em **2026-10-17**, sobre a base `51e1b7a15d665d2ca4b732f7d911e7ffdd7a66a8`. Há um mantenedor; não foi criado revisor independente, autenticação de especialista ou decisão de curadoria. Nenhum evento anterior foi substituído, reserializado ou recalculado.

## Garantias e raízes de confiança

| Propriedade | Garantia oferecida | Limite |
| --- | --- | --- |
| Integridade interna | `read_chain`, na camada comum `governance_storage.py`, verifica SHA-256 canônico e `previous_sha256` desde o primeiro evento. | Um histórico inteiramente substituído e recalculado pode ser internamente válido. Isso não é uma quebra do SHA-256. |
| Rastreabilidade | Eventos vinculam conteúdo/evidência, decisão anterior, data e responsável declarado; escritores preservam o prefixo sob trava comum. Git permite examinar versões/diffs. | Depende da preservação dos arquivos e referências anteriores. Datas/campos do evento e nome/e-mail de um commit não autenticam pessoas. |
| Identidade declarada | `reviewer`, `actor_type: human`, justificativa e tipo de revisão são declarações exigidas pelos validadores. | Não comprovam quem escreveu, sua formação ou a realização da revisão. Conta GitHub com escrita também não prova o autor de cada evento. |
| Comparação histórica | `audit-history --base SHA_COMPLETO` verifica prefixos idênticos dos registros da base nos arquivos atuais. | O SHA precisa ser escolhido e preservado por meio confiável. Uma base escolhida depois da adulteração pode coincidir com ela. |
| Autenticação verificável | **Não implementada para revisores.** Relatórios dizem `authorship_authenticated: false` e `digital_signatures_verified: false`. | Nenhuma assinatura ou vínculo entre chave/pessoa é inferido de hashes, CI, `actor_type` ou Git. |

Validações de conteúdo, evidência, prazo e permissão continuam necessárias. Integridade não transforma afirmação em fato científico ou autorização jurídica. `approved_expert` exige a declaração/evidência previstas, sem comprovar por si só identidade, competência ou revisão independente.

A trava INV-05 previne concorrência inconsistente entre escritores cooperativos; a recuperação INV-02 reconstitui projeções de eventos existentes. Nenhuma impede um administrador de substituir arquivos por fora do protocolo. Código verificador, seleção da base e administração do repositório permanecem partes da raiz de confiança.

## Comparação e recibo offline

O comando pertence à CLI existente, sem serviço, banco, dependência ou rede adicional:

```powershell
# Consistência interna; funciona também em cópia sem .git.
python scripts/content_governance.py audit-history

# Exemplo histórico real; não é base automática para futuros PRs.
python scripts/content_governance.py audit-history --base 51e1b7a15d665d2ca4b732f7d911e7ffdd7a66a8
```

O recibo `build/governance-history.json` contém presença, quantidade de eventos, último hash, SHA-256 dos bytes e resultado da comparação. Não exporta eventos, nomes, evidências ou coordenadas. Arquivo ausente aparece como ausente/zero eventos, sem aprovação presumida. A coleta usa a mesma trava dos escritores para não misturar etapas de uma operação legítima.

Verifica os três ledgers (`review_history.jsonl`, `permission_history.jsonl`, `dependency_reviews.jsonl`) e preservação exata do `review_baseline.json` existente na base. A baseline é registro legado, não revisão humana. Ledgers novos são permitidos se íntegros; eventos novos não são aprovados pela comparação. Reescrita, reordenação, truncamento, remoção, mudança de formatação do prefixo e alteração da baseline são recusados. Não há correção ou descarte automático.

Exige **ID completo de commit ancestral de HEAD**, não branch/tag móvel. Falha de Git, commit ausente em clone raso, objeto de outro tipo, base não ancestral e cadeia inválida resultam em falha. Replacement refs são desabilitadas. Sem base, informa `not_requested`; no primeiro push com SHA zero, `unavailable_initial_push`: só checagem interna. `consistent` nunca significa autoria autenticada.

Em erro, preserve checkout, recibo e objetos anteriores; confira `git diff`/`git show` e a origem da divergência. Recupere a referência exata de cópia confiável quando necessário. Não escolha base mais recente para eliminar erro, recalcule eventos ou sobrescreva decisões. Correção legítima acrescenta evento pelo fluxo existente; ambiguidade exige reconciliação humana documentada.

## CI, Git e âncoras

`offline-validation` compara com `github.event.pull_request.base.sha` ou `github.event.before`, recebidos pelo ambiente sem interpolação em código shell. O checkout já conserva histórico completo. O recibo integra os artefatos técnicos existentes, sem novas permissões ou commits. A comparação cobre **base e estado atual**, não todas as versões intermediárias: mudança transitória restaurada exige inspeção Git. Issue fechada, PR integrado e check aprovado não concedem aprovação de conteúdo.

A [âncora desta intervenção](ANCORA_GOVERNANCA_INV09.json) preserva o recibo real da base acima. Na coleta os três ledgers estavam ausentes: **nenhuma revisão humana nova foi demonstrada**. A baseline manteve SHA-256 `97b500f9b18539e9b67d631f53fddcc4a272ae499173d5a494719f3e9c6667a6`. É registro histórico datado, não inventário corrente ou autenticação.

Para preservar âncora útil, guarde SHA completo, recibo e objetos Git em backup com controle separado do checkout editável. Clone/bundle offline previamente retido permite reencontrar o commit; confira o SHA exato na restauração. Recibo no mesmo repositório e artefatos Actions com retenção limitada são auxiliares, **não âncoras independentes ou armazenamento inviolável**. Não houve backup externo validado nesta intervenção nem criação de tag/release.

Git acrescenta versões recuperáveis; hashes de objetos não autenticam campos de autor. Assinaturas são mecanismo adicional com formatos/verificação próprios: [documentação do Git](https://git-scm.com/docs/gitformat-signature). Preservar eventos antigos não autentica retroativamente quem os declarou.

## Permissões e manutenção individual

A proposta INV-04 bloqueia force push/exclusão de `main`, exige `offline-validation` e PR com **zero aprovações obrigatórias**, sem bypass permanente. CODEOWNERS dá visibilidade ao mantenedor, sem exigir segundo revisor. Workflows mantêm `contents: read`; escrita em Issues é exclusiva do job de sincronização já autorizado. A comparação não precisa de segredo/chave privada ou escrita. `distribution-permissions` permanece separado da integração técnica.

Invariantes locais foram verificadas; a **ativação remota não foi consultada nem alterada** no INV-09. O registro INV-04 é histórico. Rulesets protegem conforme a configuração efetivamente aplicada: [documentação GitHub](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/about-rulesets). Administradores podem editar regras/verificadores; não se afirma resistência a quem controla o repositório e todas as cópias. Consulte [PROTECAO_REPOSITORIO.md](PROTECAO_REPOSITORIO.md).

## Assinaturas: avaliação e proposta futura

Não se exige assinatura agora: o objetivo é transparência das decisões declaradas de um mantenedor. Sem vínculo confiável da chave, recuperação e política de verificação, a obrigatoriedade acrescentaria manutenção/bloqueio sem resolver confiança científica. Nenhuma chave/credencial foi criada ou configurada.

Se surgirem papéis distintos ou necessidade de autenticar entregas, a primeira opção é **commits assinados pelo mantenedor**, com ferramentas existentes, sem mudar eventos. Proposta não ativada:

1. Vincular chave pública real/fingerprint ao mantenedor por canal separado do PR que a introduz; registrar finalidade e início da confiança. Uma chave não concede confiança a si própria no mesmo PR.
2. Manter chave privada fora do repositório/CI, protegida por senha/dispositivo quando disponível, com backup e procedimento de perda. Não criar identidades fictícias ou publicar material privado.
3. Assinar com `git commit -S`; verificar offline o commit exato com `git verify-commit SHA_COMPLETO` e chaveiro confiável previamente estabelecido. Chave desconhecida, assinatura ausente/ruim e política indisponível permanecem não verificadas, sem autenticar decisões antigas.
4. Respeitar o escopo: assinatura comprova a chave que assinou o commit contendo os registros; não prova que cada `reviewer` é signatário nem valida revisão especializada. Autenticação individual exigiria assinatura por responsável, vínculo aos hashes/escopo e política de delegação adicional, não implementados.
5. Rotação/revogação preserva chaves públicas/histórico, exige confirmação separada e intervalos de confiança. Comprometimento exige análise; datas autodeclaradas não provam assinatura anterior à perda. Não apagar/reclassificar automaticamente decisões.

O selo Verified corresponde às regras de verificação da plataforma, sem certificar mérito científico ou independência: [escopo oficial](https://docs.github.com/en/authentication/managing-commit-signature-verification/about-commit-signature-verification). Obrigatoriedade futura depende de decisão do mantenedor e teste de recuperação. O fluxo atual de tags/releases não mudou.

## Validação desta intervenção

Verificações locais em **2026-10-17**, sem CI remota ou alterações de configurações GitHub:

| Verificação | Resultado |
| --- | --- |
| CMake/Ninja Release e compilação incremental | Concluídos sem erros. |
| CTest completo | **24/24 suítes aprovadas**, em 120,33 s. |
| Nova suíte `GlossataGovernanceHistoryTests` | 11 testes: append nos três ledgers, rehash completo, adulteração simples, truncamento/remoção, baseline alterada, bases inválidas/não ancestrais, replacement refs, cópia sem Git, recibo mínimo/idempotente e ausência de autenticação presumida. |
| Governança e proposta de proteção | 23 testes de governança e sete de proteção aprovados; identidade humana declarada continua sem autenticação. Proposta local válida, sem consulta/ativação remota. |
| Interface Qt/QML | Suíte aprovada com bancos temporários; conferência do responsável declarado e aviso de autoria não autenticada. Captura real `build/tests/previews/information-sources.png` inspecionada. Tela inicial não foi alterada. |
| qmllint em todos os QML | Sem diagnósticos ou erro. |
| Auditoria estrita do catálogo e auditoria de governança | 122 organismos; zero erros técnicos de governança. `authorship_authenticated=false`; distribuição continua bloqueada pelas pendências existentes. |
| Comparação com `51e1b7a15d665d2ca4b732f7d911e7ffdd7a66a8` | Prefixos/baseline preservados; recibo histórico arquivado sem nomes/eventos. |
| `project_status.py check --build build`, `release.py check` e `check-history` contra a base | Aprovados; contagem atual regenerada, sem alterar versão ou conceder distribuição. |
| Diff de `VERSION.txt`, dados, evidências em `resources/compliance/` e créditos contra a base | Sem alterações. Pontuação, decisões, hashes históricos, catálogo e licenças preservados. |

Testes usam exclusivamente repositórios/eventos/identidades de fixture, sem decisões, bancos pessoais ou credenciais reais. Há limitação deliberada: quem controla o histórico inteiro, verificadores e todas as referências ainda pode construir outra história coerente. Retenção independente da base e eventual autenticação futura dependem dos procedimentos do mantenedor. Não houve push, tag, release, assinatura, backup externo validado ou aprovação de distribuição nesta tarefa.
