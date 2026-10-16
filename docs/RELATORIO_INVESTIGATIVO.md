# Relatório investigativo de pontos fracos

Investigação em **16/10/2026**, sobre o commit `041ae413e4200e1f6e2da49ec7b3b03f9b734b71`. Aplicativo **1.7.0**, catálogo **2.4.0**, ambos não lançados. Resultados estruturados: [INVESTIGACAO_EVIDENCIAS.json](INVESTIGACAO_EVIDENCIAS.json).

## Parecer

Os pontos mais urgentes envolvem a identidade dos jogadores, a recuperação de decisões de governança interrompidas e a preservação de partidas ainda não gravadas. Há também um caminho de manutenção com proteção insuficiente contra redirecionamentos e uma configuração remota que permite mudanças na `main` sem proteção adicional de branch.

O projeto já possui controles úteis: transações SQLite, validação central de pontuação, separação dos rankings por regras/conteúdo, filtros de licença por arquivo, histórico de revisões, limites de API e CI. Os cenários abaixo demonstram lacunas específicas nesses controles. A aprovação da suíte existente não cobre automaticamente essas lacunas.

Este relatório não registra correções: seus achados precisam de implementação e validação próprias. Nenhuma decisão humana, licença ou revisão científica foi criada. Não houve alteração de código do aplicativo, de bancos reais, de evidências publicadas ou de configurações do GitHub.

## Método e limites de confiança

- **Reproduzido:** comportamento observado na implementação real, em banco temporário, fixture sintética ou transporte HTTP simulado.
- **Configuração observada:** resultado de consulta somente de leitura à API do GitHub; pode mudar após a coleta.
- **Sustentado pelo código:** caminho identificado por leitura, sem reproduzir uma falha física ou todos os seus efeitos.
- **Limite de garantia:** propriedade que o mecanismo não oferece; não significa necessariamente defeito ou invasão.
- **Pendente de teste:** hipótese que não deve ser apresentada como falha comprovada.

A prioridade considera impacto, exposição e pré-condições. Não é uma classificação CVSS nem certificação jurídica ou científica. Os testes de HTTP não acessaram servidores locais ou externos; somente as consultas administrativas ao GitHub usaram rede. As identidades e autorizações das fixtures existentes são sintéticas e nunca foram incorporadas ao catálogo.

## Matriz de achados

| ID | Prioridade | Estado da evidência | Ponto fraco | Principal consequência |
| --- | --- | --- | --- | --- |
| INV-01 | Alta | Reproduzido em Qt/SQLite temporário | Renomeação sem identidade estável de perfil | Resultados não acompanham o perfil renomeado |
| INV-02 | Alta | Reproduzido com exceção controlada | Permissão gravada em dois arquivos sem recuperação completa | Histórico e proveniência divergem; repetição não repara |
| INV-03 | Alta | Sustentado pelo código | Progresso de partida apenas em memória até encerramento | Perda do progresso ainda não salvo em interrupção |
| INV-04 | Alta para manutenção | Configuração remota observada | `main` sem proteção de branch e sem rulesets | Ausência de barreira adicional contra alterações indevidas por quem tem escrita |
| INV-05 | Média | Reproduzido com dois escritores controlados | CLI direta pode contornar o bloqueio do bridge | Cadeia de revisão torna-se inconsistente |
| INV-06 | Média, condicional | Reproduzido com HTTP simulado | Verificador de links segue redirecionamento para HTTP local | Requisições fora do escopo de fontes públicas na manutenção online |
| INV-07 | Média | Reproduzido em Qt/SQLite temporário | Unicidade de nomes insuficiente para Unicode | Perfis equivalentes por caixa podem ficar separados |
| INV-08 | Alta antes de release | Pendências documentadas | Direitos e associações científicas ainda incompletos | Publicação continua bloqueada; necessidade de saneamento humano |
| INV-09 | Média | Limite demonstrado em fixture | Cadeia de hashes não autentica o responsável | Escrita autorizada ou comprometida pode produzir cadeia internamente coerente |
| INV-10 | Média | Confirmado na documentação | Versões, regras e número de suítes divergentes | Manutenção e instruções podem partir de premissas erradas |

## INV-01 — Resultados deixam de acompanhar o perfil renomeado

**Evidência:** [renamePlayerProfile](../src/infrastructure/persistence/SqliteGameHistoryRepository.cpp#L405) altera somente `player_profiles.name`; [consulta de resultados](../src/infrastructure/persistence/SqliteGameHistoryRepository.cpp#L338) e [métricas históricas](../src/infrastructure/persistence/SqliteGameHistoryRepository.cpp#L366) usam o nome armazenado nas outras tabelas.

**Reprodução:** a sonda C++ vinculada à implementação de produção criou “Ada”, salvou um resultado de seis unidades e um acerto e renomeou o perfil para “Lia”. A operação retornou sucesso. A consulta por “Lia” retornou zero resultados; por “Ada”, um. Os pontos continuam no banco, associados ao nome antigo.

**Impacto:** o jogador pode interpretar isso como perda de resultados. Novas partidas ficam sob o novo nome e podem fragmentar o histórico. A retenção de resultados após excluir um perfil é intencional; reutilizar nomes requer uma política explícita para não confundir pessoas diferentes.

**Correção recomendada:** ID permanente de jogador, nome de exibição separado, associação histórica transacional e preservação dos nomes da época como snapshots. A partida deve capturar o ID inicial. Decidir como tratar resultados antigos e nomes reutilizados, sem fundir perfis automaticamente.

**Critério de conclusão:** renomear e reabrir mantém os mesmos resultados/métricas por perfil; excluir, recriar ou trocar de perfil durante a partida não transfere dados indevidamente. Migração idempotente, transacional e testada com cópias temporárias.

## INV-02 — Interrupção da confirmação de permissão não se recupera na repetição

**Evidência:** [confirm_permission](../scripts/content_governance.py#L602) acrescenta o evento em `permission_history.jsonl` e depois salva `photo_provenance.json` ([linhas finais da operação](../scripts/content_governance.py#L633)). O [bridge de decisões](../scripts/curation_decisions.py#L103) retorna `already_recorded` quando encontra o evento, antes de reconciliar a proveniência.

**Reprodução:** usando a fixture de permissão já existente nos testes, foi injetado `OSError` somente na gravação da proveniência. O histórico recebeu um evento; a proveniência permaneceu igual. Repetir a decisão retornou `already_recorded`, mas os cinco achados de permissão da fixture continuaram presentes.

**Pré-condição:** interrupção ou falha entre as duas gravações, como indisponibilidade de armazenamento. A reprodução não simula todos os efeitos de queda de energia nem testa garantias de `fsync`.

**Impacto:** inconsistência entre o evento e o estado materializado, bloqueio operacional e recibo de repetição que não informa a falta de recuperação. **Não foi observada liberação indevida de direitos:** o validador continuou apontando a documentação ausente.

**Correção recomendada:** adotar evento como fonte de verdade com projeção recuperável, ou protocolo de journal/commit que possa reconciliar as duas escritas. Uma transação SQLite para o estado de manutenção também é uma opção, preservando os ledgers exportados. O retorno idempotente precisa verificar os efeitos materiais.

**Critério de conclusão:** falhas injetadas antes/depois de cada escrita, seguidas de reabertura e repetição, produzem um único evento e proveniência consistente; nenhum hash anterior ou responsável real é sobrescrito.

## INV-03 — Partida em andamento depende da memória do processo

**Evidência:** [submitAnswer](../src/application/GameService.cpp#L151) acrescenta respostas à sessão em memória; [saveCurrentResult](../src/application/GameService.cpp#L242) só persiste depois do encerramento. Não foi localizado tratamento `onClosing` em `Main.qml` nem fluxo de recuperação de rascunho ou backup/restauração.

**Exposição:** uso normal do programa. Uma interrupção antes do salvamento pode perder respostas e dicas já usadas na partida corrente. A retentativa existente protege falhas de gravação do resultado final, mas não registra continuamente a sessão.

**Correção recomendada:** checkpoint atômico após resposta e revelação de dica, separado do ranking, e tratamento do fechamento normal. Incluir opções, jogador estável, teto aplicado, snapshots, regras e identidade da sessão. Definir a política de retomada competitiva antes de permitir recuperação; não oferecer nova tentativa gratuita da mesma questão. Usar backup consistente do SQLite com restauração verificada.

**Critério de conclusão:** encerramento forçado/reabertura em pontos definidos recupera estado coerente, sem duplicar resultado, recalcular pontos antigos ou tratar partida incompleta como concluída.

## INV-04 — Proteções remotas da branch principal ausentes

**Evidência de consulta:** a API de `branches/main` retornou `protected: false`; a listagem de rulesets retornou vazia. Os metadados e a hora real estão no arquivo de evidências. Nenhuma proteção foi ativada nesta investigação.

**Limite de interpretação:** isso não permite escrita pública irrestrita. As permissões normais do GitHub continuam válidas. O risco exige uma conta/integração com escrita, ou uma operação indevida dessa conta.

**Impacto:** código, políticas, evidências e workflows dependem mais diretamente da conta com escrita, sem proteção adicional contra push indevido, reescrita ou remoção de histórico.

**Correção recomendada:** configurar regras para restringir force push/exclusão e exigir o check técnico `offline-validation`; revisar acessos de contas e integrações. Proteger caminhos de governança/workflows com revisão compatível com a equipe real. Se houver um único mantenedor, não declarar revisão independente inexistente. A obrigatoriedade do gate de distribuição deve continuar no lançamento: exigir sua aprovação para qualquer commit de desenvolvimento enquanto persistem pendências impediria também seu saneamento.

**Critério de conclusão:** configuração consultável e documentada, testes dos bloqueios com escopo autorizado e permissões mínimas preservadas. Alterar regras exige autorização específica; este relatório não a pressupõe.

## INV-05 — Escritores diretos concorrentes podem quebrar a cadeia de revisão

**Evidência:** [append_review](../scripts/content_governance.py#L586) lê o último evento e depois faz append sem bloqueio próprio. O [bridge](../scripts/curation_decisions.py#L60) possui `decision.lock`, mas a CLI `content_governance.py record-review` chama o escritor diretamente.

**Reprodução:** duas chamadas diretas, sincronizadas após ler o mesmo topo, concluíram a gravação com o mesmo `previous_sha256`. A leitura posterior rejeitou a cadeia.

**Exposição:** duas operações de manutenção simultâneas pela API/CLI direta. Não foi demonstrada corrupção pelo uso sequencial normal do bridge.

**Impacto:** auditorias deixam de aceitar o histórico até reparação consciente. O detector funciona e falha de forma restritiva; o problema é permitir a produção de um histórico inconsistente.

**Correção recomendada:** centralizar bloqueio e comparação do topo esperado no escritor comum, compartilhado por todos os caminhos. Considerar também recuperação de lock abandonado e processos em checkouts diferentes; um lock em `build/` protege somente o contexto em que é compartilhado.

**Critério de conclusão:** operações concorrentes são serializadas ou uma delas é recusada antes da escrita; todos os eventos preservados formam cadeia válida. Nenhum reparo automático deve apagar decisão humana divergente.

## INV-06 — Redirecionamento pode sair das fontes públicas na manutenção online

**Evidência:** [PublicClient.get](../scripts/review_updates.py#L21) valida a URL inicial e bloqueia IP privado literal, mas usa `urllib.request.urlopen` com redirecionamento padrão ([linha 35](../scripts/review_updates.py#L35)). Não valida cada destino nem resolve nomes para conferir os endereços resultantes.

**Reprodução:** handlers HTTP/HTTPS inteiramente em memória simularam uma fonte pública HTTPS que redireciona para HTTP de loopback. O cliente seguiu o destino e aceitou o resultado. Nenhum serviço local foi acessado e nenhum conteúdo real foi obtido.

**Pré-condições:** manutenção explicitamente online e uma referência ou servidor que conduza ao destino indesejado. O jogo offline normal não usa esse caminho. Não foi demonstrada exfiltração, execução de código ou invasão remota do aplicativo. Resolução para IP privado e rebinding de DNS são riscos adicionais ainda não reproduzidos.

**Correção recomendada:** recusar redirecionamentos ou validar cada salto, manter HTTPS, limitar portas/destinos e verificar endereços resolvidos. Reutilizar o transporte restritivo dos provedores onde adequado; a diversidade de fontes bibliográficas exige política própria documentada.

**Critério de conclusão:** testes bloqueiam downgrade para HTTP, loopback, redes privadas, redirecionamentos encadeados e destinos fora da política, sem abrir conexões durante os testes. Consultas continuam gerando somente propostas.

## INV-07 — Perfis com diferenças de caixa em caracteres acentuados coexistem

**Evidência:** [chave de nomes](../src/infrastructure/persistence/SqliteGameHistoryRepository.cpp#L128) usa `COLLATE NOCASE`. A implementação real aceitou “João” e “JOÃO” como dois perfis.

**Impacto:** duplicidade percebida, fragmentação de resultados e comportamento diferente entre nomes ASCII e acentuados. Não envolve execução de SQL por texto do jogador: as operações inspecionadas usam parâmetros vinculados.

**Correção recomendada:** normalização Unicode e case folding explícitos na chave canônica, mantendo o nome original para exibição e ID estável para identidade. Definir também a equivalência entre caracteres compostos/decompostos. Casos preexistentes exigem resolução consciente, sem fusão silenciosa.

**Critério de conclusão:** unicidade funciona para caixa e formas Unicode conforme política documentada; perfis já existentes e seus resultados permanecem rastreáveis.

## INV-08 — Documentação de direitos e revisão científica ainda impede lançamento

**Evidência:** diagnóstico existente conserva **120** fotos com comprovação incompleta, **sete** achados de outros materiais e **dois** bloqueios de origem científica para as fotos `707161720` e `659755043`. Esses números não significam 120 licenças comprovadamente incompatíveis. Não houve nova consulta de licença por fotografia nesta investigação.

O gate de pacote não remove arquivos do GitHub. A publicação de mídias no repositório e a publicação de executáveis são superfícies distintas a revisar. Os 37 arquivos antigos removidos continuam no histórico anterior; as 109 mídias locais fora do bundle estão ignoradas e não devem ser confundidas com arquivos atuais publicados.

**Correção recomendada:** priorizar comprovação por arquivo, divergências de crédito e as duas associações científicas; decidir explicitamente sobre materiais próprios. Avaliar separadamente as obrigações das dependências do pacote com `--package`. Qualquer decisão sobre direitos ou retirada de recursos ativos exige evidência e manutenção da funcionalidade prevista.

**Critério de conclusão:** permissões, atribuições e revisões atendem aos validadores e à avaliação humana apropriada; o gate permanece obrigatório. Testes não substituem análise jurídica ou científica.

## INV-09 — Consistência por hash não equivale a autoria autenticada

**Evidência:** [read_chain](../scripts/content_governance.py#L258) verifica encadeamento e SHA-256; responsável e tipo de ator são campos declarados. Em uma fixture, alterar os eventos e recomputar a cadeia resultou novamente em leitura aceita.

**Interpretação:** isso não quebra SHA-256 nem demonstra ataque por usuário sem escrita. Mostra que quem controla todos os bytes e pode substituir o histórico também pode produzir hashes coerentes. O mecanismo identifica alterações sem recomputação e suporta rastreabilidade dentro de uma raiz confiável; sozinho, não prova quem revisou.

**Correção recomendada:** explicitar a raiz de confiança, restringir escrita, preservar âncoras anteriores e considerar assinaturas verificáveis quando houver necessidade de autenticar autoria. Git e suas proteções precisam participar desse modelo. Um documento com `actor_type: human` não comprova identidade ou qualificação independentemente.

**Critério de conclusão:** documentação e interface distinguem declaração, integridade e autenticação; eventuais assinaturas têm identidade e política de verificação definidas. Não exigir ou simular um segundo curador inexistente.

## INV-10 — Documentação atual contradiz as fontes de verdade

**Evidência:** [README](../README.md#L45) informa 1.5.0/2.3.0; `VERSION.txt` e o manifesto indicam 1.7.0/2.4.0. [PROJECT_CONTEXT](../PROJECT_CONTEXT.md#L37) ainda descreve v3 como regra atual e [lista nove suítes](../PROJECT_CONTEXT.md#L77), embora o código use v4 e o CTest registre 15.

**Impacto:** comandos, retomadas e decisões podem partir de estado antigo. Relatórios de etapas concluídas são históricos legítimos; o erro está em informações apresentadas como atuais.

**Correção recomendada:** bloco de estado derivado de fontes únicas e validação na CI, preservando relatórios históricos identificados como tais.

**Critério de conclusão:** estado atual concorda com versões, regras e suítes; documentação distingue preparação e lançamento.

## Áreas que ainda precisam de investigação específica

| Área | Evidência atual | Próxima verificação |
| --- | --- | --- |
| Windows limpo, GPU e DPI | Testes principais usam software/offscreen; janela mínima 1000 × 700 e textos com pixels fixos | Instalação sem MSYS2, renderização de hardware, teclado/leitor de tela e escalas de 125%, 150% e 200% |
| Crescimento do histórico | `playerMetrics` carrega respostas de cada partida; sem medição de volume nesta revisão | Benchmark com milhares de partidas sintéticas, tempo de abertura e uso de memória antes de otimizar |
| Reprodutibilidade do ambiente | Workflow atualiza MSYS2 e não fixa versões dos pacotes Qt/GCC; build registra hashes de entradas e binário | Registrar toolchain e faixa suportada; verificar compilação em ambiente limpo com versões documentadas |
| Privacidade e segredos | Auditoria usa lista de campos/padrões; importador legado mantém cache da resposta; saídas novas selecionam metadados mínimos | Revisar retenção local, metadados e cenários de vazamento; não tratar detecção por padrões como prova universal de ausência |
| Avaliação pedagógica | Classificação automática explícita, com restrições e lacunas documentadas | Amostragem por especialista/usuários, especialmente associação linguística e combinação de dicas; não converter heurística em revisão humana |

Essas linhas são pendências de verificação, não afirmações de vulnerabilidade comprovada. Não houve varredura de dependências por CVE, análise de binários de terceiros, teste de invasão remoto ou medição de desempenho de grande escala.

## Controles que funcionaram durante a investigação

- As cadeias inconsistentes foram rejeitadas pelo leitor; o teste concorrente não produziu uma aprovação aceita silenciosamente.
- A confirmação interrompida continuou bloqueada pelos validadores de permissão.
- Partida competitiva incompleta é armazenada como incompleta e não atualiza o ranking, conforme a lógica inspecionada.
- Os novos clientes de provedores possuem limites de resposta, hosts e ritmo de requisições; a lacuna de redirecionamento descrita é do cliente de manutenção investigado.
- A automação usa permissões separadas, limites de criação/atualização, ações fixadas por SHA e execução semanal controlada. Não foi encontrado uso de texto de Issue como comando de aprovação científica.
- A CI remota do commit inspecionado aprovou `offline-validation`. O job `distribution-permissions` falhou no gate documentado: [execução 38958447401](https://github.com/kaduolliver/Projeto-Glossata/actions/runs/38958447401). A falha global não deve ser confundida com falha de compilação.

## Validação executada e artefatos

1. Reexecução da sonda C++/Qt/SQLite de perfis em `QTemporaryDir`; código do repositório permaneceu inalterado desde sua compilação na análise anterior.
2. Falha controlada entre evento e proveniência, seguida de repetição real do bridge.
3. Dois escritores diretos sincronizados para exercitar a janela de concorrência.
4. Recomputação de uma cadeia sintética para testar o limite de autenticação.
5. Redirecionamento com handlers HTTP/HTTPS em memória e zero conexões do teste.
6. CTest selecionado: `ctest --test-dir build -R 'Glossata(Governance|CurationDecision|CurationGitHub|RepositoryAudit)Tests' --output-on-failure`: **4/4 suítes passaram**, 2,54 s.
7. Consultas somente de leitura à branch, aos rulesets e aos jobs da CI, com resultados mínimos e sem credenciais no relatório.

As reproduções adicionais ficam localmente em `build/investigation/`, fora do Git; seus resultados sanitizados estão no JSON versionado. A sonda de perfis está em `build/project-review/`. Para repetir os cenários em outro checkout, usar as fixtures `GovernanceTests` e `DecisionTests`, direcionar a raiz a diretórios temporários e aplicar os pontos de falha descritos nos achados. Não usar catálogos ou decisões reais nesses testes.

Não houve nova compilação do aplicativo nem validação visual nesta investigação. As quatro suítes existentes passaram ao mesmo tempo em que as reproduções adicionais demonstraram lacunas não cobertas por elas. A próxima implementação deve transformar cada reprodução corrigida em regressão permanente.

## Plano de tratamento

| Ordem | Entrega | Responsabilidade | Aceite principal |
| --- | --- | --- | --- |
| 1 | IDs estáveis e nomes Unicode, migração de perfis | Engenharia; mantenedor decide conflitos históricos | Resultados acompanham identidade sem fusão indevida |
| 2 | Recuperação de permissão e escritor comum com lock | Engenharia; curadoria confere eventos divergentes | Interrupção/repetição/concorrência preservam um estado coerente |
| 3 | Restringir transporte de manutenção | Engenharia | Nenhum salto permitido para destinos inseguros/privados |
| 4 | Checkpoints, backup e política de retomada | Engenharia e mantenedor | Recuperação sem duplicidade ou vantagem competitiva |
| 5 | Proteções da branch e raiz de confiança | Mantenedor, com autorização para configuração remota | Regras verificáveis, acesso restrito e declarações precisas |
| 6 | Saneamento de direitos/ciência e documentação atual | Curadoria/mantenedor; engenharia apoia automação | Evidências e decisões reais; gate de lançamento satisfeito |

As frentes de direitos e ciência podem avançar em paralelo às correções técnicas. Este relatório e seus testes não autorizam push, publicação, contato com terceiros ou alteração de aprovações.
