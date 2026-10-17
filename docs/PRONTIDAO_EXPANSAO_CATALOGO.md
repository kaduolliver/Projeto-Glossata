# Prontidão para expansão do catálogo

Investigação e implementação em **2026-10-17**, na branch `feat/admissao-catalogo-auditavel`, sobre a `main` incorporada pelo PR #8. Escopo: governança e manutenção/importação offline. Nenhum organismo, foto real, texto científico, pontuação, banco de jogador, layout ou evento real de curadoria foi alterado. Não houve consulta de API de conteúdo, contato com autores, remoção de mídia ou aprovação humana nesta intervenção.

O processo técnico prepara **propostas de admissão**. A integração de um organismo continua exigindo alteração explícita por Pull Request e execução dos controles completos. O relatório não certifica direitos ou identificação científica. O projeto tem um mantenedor; revisão editorial não é revisão independente nem especializada.

## 1. Controles existentes e lacunas tratadas

| Etapa | Controle existente | Lacuna e medida nesta intervenção | Critério verificável |
| --- | --- | --- | --- |
| Importação | `import_inaturalist.py` recusa expansão direta antes de rede/escrita; Research Grade, táxon comunitário e foto específica | Faltava contrato entre candidato e proposta publicada | Novo comando de verificação e preparação, sem escritor do catálogo ativo |
| Provedores | Interfaces, cache imutável, IDs com namespace/dataset, datas, orçamento e modo offline | Uma resposta não demonstra revisão nem direitos de todos os materiais | Dossiê exige snapshots, localizadores do ID/nome e análise de uso por camada |
| Reconciliação | IDs externos, nomes canônicos, aceitos/sinônimos, divergências e homônimos | Cópia manual de um candidato sem marcador poderia escapar do controle de staging | Auditoria identifica IDs ausentes do baseline legado e exige admissão completa |
| Fotografias | Política CC0/CC BY, exceções documentadas, hash, crédito, proveniência, ledger e recuperação | Metadados públicos não equivalem a confirmação humana; licenças antigas incompletas | Mesmos validadores de permissão, evidências e eventos; pendências mantidas |
| Conteúdo educativo | Fontes editáveis, geração e revisão por tópico | Não bastava editar um JSON derivado | Gerador parametrizado para workspace e comparação sem escrita |
| Dicas | Fontes, assistência separada da categoria, avaliação contextual e revisão | Ausência de conteúdo não pode ser preenchida com invenções | Exige fonte/revisão do conteúdo presente; indisponibilidade justificada do ausente |
| Histórico | Ledgers com hash, trava comum, idempotência, recuperação e comparação com Git | Evidências do dossiê também precisam invalidar revisões após mudança | Registro `admission:<id>` no mesmo ledger, vinculado a conteúdo e bytes das provas |
| Fila | IDs estáveis, agrupamento, privacidade, limites de Issues e decisão separada | Novos dossiês devem reutilizar a fila | Registros de admissão no grupo de licenciamento, com referências públicas filtradas |
| Direitos públicos | Diagnóstico por recurso, inventário de pacote e gate | Faltava distinguir capturas, histórico remoto e caches locais | Inventários separados e registro estruturado; nenhuma exclusão automática |

GBIF, Catalogue of Life/ChecklistBank, WoRMS e Wikimedia Commons têm adaptadores reais; NCBI, LPSN, POWO e MycoBank continuam previstos conforme [PROVEDORES_CIENTIFICOS.md](PROVEDORES_CIENTIFICOS.md). As coletas legadas de fotos de quiz continuam dependentes do contrato iNaturalist. O novo verificador reutiliza esse contrato para mídia de quiz; não apresenta o adaptador Commons de fotos de apresentação como importador genérico de fotos de identificação. Uma nova origem de mídia exigirá adaptação e testes de proveniência, sem herdar a licença de um dataset.

## 2. Diagnóstico documental atual

O [registro por recurso](PRONTIDAO_EXPANSAO_CATALOGO.json) contém autor/crédito declarado, organismo, IDs, URLs, licença e versão comprovada quando disponíveis, datas, hashes, alterações, pendências, prioridade e ação. Os snapshots completos já existentes permanecem em [SANEAMENTO_DIREITOS.json](SANEAMENTO_DIREITOS.json) e em `resources/compliance/permissions/` e `exploration/`. O novo registro referencia os snapshots pelo hash e localizador, sem duplicar as respostas.

| Indicador | Estado verificado |
| --- | --- |
| Organismos ativos | 122, preservados |
| Fotos distintas de quiz | 120, preservadas; dois organismos compartilham mídia |
| Fotos de quiz com pendências documentais | 120; mesmo número do diagnóstico histórico |
| Fotos com metadados públicos arquivados para conferência | 120; zero novas consultas nesta execução |
| Versão da licença explicitada nesses 120 snapshots públicos | 0; o código da licença não demonstra sua versão |
| Divergências de crédito registradas | 7; requerem comparação, sem substituir fotógrafo por observador ou aviso CC0 |
| Achados em outros materiais | 7, envolvendo branding, perfis, dicas e gerador de textos |
| Fotos de apresentação | 109; zero achados documentais de permissão pelo validador atual |
| Revisão científica das apresentações | 109 ainda pendentes; permissão demonstrada não aprova representatividade |
| Bloqueios científicos prioritários do gate | 2, preservados |
| Capturas do README inventariadas | 2, com hashes compatíveis com os registros de captura |
| Mídias apenas no histórico remoto inventariado | 37 caminhos, 39 referências de objetos Git |
| Mídias locais fora do bundle | 109; nenhuma rastreada na árvore Git atual |
| Componentes do pacote local inventariados | 23, avaliados separadamente |

As pendências das fotos incluem versão não comprovada, data original de obtenção desconhecida e confirmação humana ausente. A data real de consulta pública arquivada não foi retroativamente convertida em data do download original. As 120 fotos possuem informações para **conferência inicial pelo mantenedor**; nenhuma ficou automaticamente apta a `confirm_permission` por essa coleta. Não se substituiu uma licença antiga por uma declaração mais recente.

Os sete materiais são `resources/branding/glossata.png`, `glossata_discord.png`, `glossata.ico`, `resources/data/species_profiles.json`, `hint_education.json`, `progressive_hints.json` e `scripts/build_education.py`. O inventário deste último recebeu somente o novo hash após a parametrização do gerador, preservando o hash anterior e o estado pendente. O manifesto **2.4.0 ainda não lançado** foi completado com essa alteração documental, conforme [VERSIONAMENTO.md](VERSIONAMENTO.md); sua identidade de bytes mudou. Nenhum texto do corpus foi reescrito. O aplicativo permanece **1.7.0 em desenvolvimento**, sem tag ou release.

Essas contagens são achados documentais, sem conclusão automática de violação. Tampouco ausência de achado no validador prova regularização. As consultas usadas nos dois casos prioritários ocorreram em **2026-10-17T11:30:18.288714+00:00**; `checked_at` no novo relatório é a auditoria local, não nova conferência remota.

## 3. Prioridade e exposição das mídias

| Prioridade | Recursos | Providência legítima |
| --- | --- | --- |
| P0 | Duas associações com bloqueio científico; captura pública de quiz que reproduz foto ainda pendente | Conferir evidência/uso; decidir complementação, substituição ou suspensão, preservando os registros |
| P1 | 120 fotos de quiz públicas, branding, textos com direitos ainda indefinidos e capturas | Demonstrar licença específica/versão e atribuição; revisar direitos dos textos e do material composto |
| P1 | 37 caminhos no histórico público; obrigações das dependências do pacote | Investigar proveniência/exposição e condições de distribuição; não apagar nem reescrever histórico nesta tarefa |
| P2 | Representatividade das 109 fotos de exploração | Revisão editorial identificada; especialista apenas quando houver participação comprovada |
| P3 | 109 arquivos locais fora do bundle e fixtures sintéticas | Manter separados da distribuição, conferir antes de eventual reaproveitamento |

Os escopos não são mutuamente exclusivos: uma mesma foto pode estar no catálogo, no Git, nos inputs QRC e em uma captura. Cada recurso registra os escopos aplicáveis:

1. **Catálogo ativo:** organismos e `photo.path`, com associação e proveniência, sem alterar elegibilidade do quiz.
2. **Árvore Git atual:** `git ls-files`; presença no repositório público exige atenção independentemente de o recurso estar no executável.
3. **README:** `docs/images/quiz.png` identifica a foto **748172022**, de *Panthera leo*, crédito Patrick Jackson; ainda faltam evidência da versão, data original e confirmação. `tela-inicial.png` registra inputs e branding; a lista de inputs não prova quais fotos aparecem nos pixels. `exploracao.png` também consta no inventário de outras mídias documentais e exige conferência dos elementos representados. Nenhuma captura foi substituída nesta tarefa.
4. **Executável/pacote:** QRC é prova de input de compilação, não de conteúdo de um `.exe` preexistente. A comparação de arquivos soltos usa hashes; conteúdo embutido é explicitamente `not_verified`. O pacote local tem manifesto desatualizado e pendências de dependências; não foi regenerado ou publicado.
5. **Arquivos locais:** mídias ignoradas fora do bundle não foram apresentadas como arquivos atuais do repositório público.
6. **Histórico:** `git rev-list --objects --remotes=origin`, sobre refs buscadas, identifica caminhos removidos da árvore atual. Ramos exclusivamente locais são excluídos. IDs de objetos Git são identificados como tais, não como SHA-256 de mídia. A análise não enumera todas as versões antigas de caminhos ainda ativos nem prova remoção de caches/forks. Sem refs acessíveis, o resultado é desconhecido, não zero.

Para materiais duvidosos, as opções são obter prova complementar e registrar a análise, propor substituição por recurso documentado ou propor suspensão do uso. Remover da árvore atual não remove o histórico. Qualquer remoção, troca de foto real ou reescrita histórica depende de autorização específica e avaliação do impacto. Nada disso foi executado.

Os hashes dos arquivos removidos e as referências históricas já levantadas estão em [LIMPEZA_REPOSITORIO.md](LIMPEZA_REPOSITORIO.md); as substituições preservadas em `resources/data/catalog_replacements.json` complementam esse inventário. Esses registros devem ser consultados antes de investigar novamente cada mídia histórica, sem presumir autoria ou permissão onde os metadados antigos não existem.

## 4. Os dois bloqueios científicos

| Foto / organismo | Evidência arquivada | Diagnóstico delimitado |
| --- | --- | --- |
| **707161720 — Panthera tigris** (organismo 125) | Observação **386235634**, táxon/comunidade **41967**, foto presente, `is_active: true`, `quality_grade: casual`, `captive: true`, um voto público `wild: false`; CC BY, crédito Craig A. C. Symonds | Identidade e associação declaradas coincidem. Não satisfaz Research Grade. O snapshot sustenta condição de cativeiro e conflito com política de importação, sem demonstrar identificação incorreta |
| **659755043 — Lemur catta** (organismo 128) | Observação **361508293**, táxon/comunidade **43617**, foto presente, `is_active: true`, `quality_grade: casual`, `captive: true`, dois votos públicos `wild: false`; CC BY, crédito biogeek | Mesma distinção: incompatibilidade com o filtro da observação, sem evidência aqui de táxon errado |

Snapshots: `80735cc5e9404ad80df44bf7f61a26cbdadd850a98682a616f9630a3929f1f9e` e `974889b0f7312bfa262eb6c2b04759b2a5dc20558142f459667a28a5819ce770`, respectivamente. O registro JSON fornece SHA-256 dos bytes, origem e localizador no arquivo arquivado. São evidências minimizadas; usuários dos votos e coordenadas não foram publicados.

Os códigos atuais incluem `legacy_observation_not_research_grade` e, na comparação dos snapshots públicos, `current_observation_not_research_grade`. “Casual” isoladamente pode ter causas diferentes; nestes snapshots há também os indicadores de cativeiro descritos. Não se extrapolam para distribuição da espécie, validade de seu nome ou qualidade de todas as observações. Nenhuma revisão visual por especialista foi realizada.

Alternativas para decisão do mantenedor: preparar outra foto da mesma espécie com observação elegível e direitos demonstrados; ou analisar uma política explicitamente delimitada para fotos de cativeiro/apoio, com justificativa pedagógica, associação e níveis de identificação revisados. Eventual exceção exige mudança explícita de política e testes: o novo fluxo **não libera** observação Casual por existir revisão editorial. Se as evidências continuarem insuficientes, pode-se propor suspensão. O saneamento científico e o de licenças permanecem independentes; ambos os bloqueios continuam ativos.

## 5. Fluxo de admissão consolidado

### Estados e responsabilidades

Não foi criada outra máquina de aprovação. **Candidato** é a localização no `CandidateStore`, cujos campos científico/jurídico permanecem `pending`; **rascunho** corresponde a `draft` no sistema de governança. **Pendente de revisão** usa `pending`, `needs_review` ou `expired`, conforme a causa. `in_review` continua disponível nos registros editoriais legados, sem criar aprovação.

**Aprovado editorialmente** é `approved_editorial`, com decisão real do mantenedor; **aprovado por especialista** é `approved_expert`, reservado a participação e qualificação documentadas. O validador exige as declarações e evidências, mas não autentica a identidade nem certifica a qualificação. **Rejeitado/suspenso** usa `rejected` e justificativa no ledger, cujo rótulo existente é “Conteúdo suspenso”. O efeito depende do recurso e da política de publicação; não se inventa novo estado para cada tela.

Um único mantenedor pode revisar editorialmente conteúdo experimental. Não se exige um segundo revisor inexistente, e o mantenedor não é apresentado como especialista ou revisão independente sem evidência. IA e metadados de API entram como rascunho; uma URL ou teste bem-sucedido não é aprovação.

### Contrato e sequência

1. Descobrir/reconciliar pelo fluxo atual, arquivando cache e revisão do candidato. Identidade inclui provedor, ID e dataset/release quando aplicável.
2. Resolver sinônimos para um registro aceito antes da admissão. Comparar nomes, IDs externos, possíveis homônimos e divergências; nomes semelhantes não autorizam fusão. IDs ativos e IDs de substituições não são reciclados.
3. Preparar um workspace em `build/scientific-sources/drafts/<nome>/` com os arquivos padrão. O dossiê fica em `resources/compliance/admissions/<id>.json`. O template não preenche conteúdo, concessões, revisores ou datas de revisão.
4. Preservar o candidato completo, seu hash e ID determinístico; arquivar as evidências do provedor em `resources/compliance/sources/<hash>.json`. Informar localizadores JSON Pointer de ID e nome no payload, hash dos bytes e proveniência da consulta. O hash da resposta HTTP original não é recalculado a partir do payload minimizado; o cache registra ambos, com escopos distintos.
5. Reconciliar a linhagem com a política filogenética adotada, preservando fonte, níveis ausentes e divergências. A projeção estrutural não aprova a classificação.
6. Selecionar foto adequada, preservar bytes/hash/crédito e verificar a licença **daquela mídia** antes do download pelo importador já existente. O verificador de admissão não baixa imagens. Conferir Research Grade, comunidade, ID, nome, foto e táxon ativos. Não inferir identidade por reconhecimento automático.
7. Registrar a coleta minimizada e a confirmação legítima pelo fluxo de permissão existente. O ledger deve sustentar a projeção; copiar um campo “confirmado” sem evento não funciona. Licença/versionamento, evidência, modificações e crédito continuam obrigatórios; CC0 mantém crédito editorial.
8. Documentar separadamente `source_use.taxonomy_data` e `source_use.educational_text`: licença/concessão ou fundamento documental aplicável, citação, justificativa de compatibilidade, escopos `offline_catalog` e `public_repository`, prova local com hash, URL e localizador. Uma licença dos dados não é licença da foto nem do texto consultado. Licenças/cessões dos materiais próprios continuam decisões do mantenedor; este processo não escolhe uma por ele.
9. Preparar `species_profiles.json`, `hint_education.json` e `curation_reviews.json`; gerar `education.json` pelo gerador existente. Exigem-se informações educativas com fontes; nutrição é opcional quando inaplicável. Conservação desconhecida deve ser indicada honestamente. Dicas/etimologias ausentes recebem motivo em `unavailable_content`; o conteúdo presente exige revisão de sua fonte e, quando disponível antes da resposta, de sua assistência contextual.
10. Registrar revisões reais por tópico/afirmação e do dossiê `admission:<id>` no **mesmo** `review_history.jsonl`. Nova admissão não recebe dispensa por aprovação legada. Revisão vencida, rejeitada, pendente ou com hash alterado bloqueia a preparação. Alteração das provas ou do conteúdo invalida a revisão do dossiê.
11. Regenerar derivados no workspace e executar `admission-check`. A comparação com geração em memória detecta edição isolada do derivado. Executar também os validadores técnicos aplicáveis; a preparação não substitui compilação, auditoria completa ou gate de distribuição.
12. Executar `admission-prepare`. Exporta uma proposta imutável por hash em `build/scientific-sources/admissions/`, preserva a referência à base e exige que os ledgers sejam extensões dos atuais. Repetir gera a mesma proposta, sem duplicar eventos. Uma base alterada exige reconciliação explícita.
13. Integrar **o delta** por PR: preservar organismos existentes, evidências, prefixos dos ledgers, direitos por recurso e dados de jogadores; atualizar histórico/manifesto/versão do catálogo conforme SemVer; regenerar derivados e créditos e executar CI. O workspace de um candidato **não é substituição integral do catálogo**. A auditoria de admissão também roda sobre novos IDs na árvore ativa. A publicação de executável/catálogo continua condicionada ao gate e à autorização.

Exemplo de comandos, sem promover organismos nesta tarefa:

```powershell
python scripts/scientific_catalog.py admission-template --candidate-id "candidate:<uuid>" --draft "build/scientific-sources/drafts/<nome>"
# Completar os arquivos e o dossiê com evidências verdadeiras. Preservar o
# baseline legado e os prefixos dos ledgers; não recriar uma baseline de aprovação.
python scripts/content_governance.py confirm-permission --workspace "build/scientific-sources/drafts/<nome>" --decision "<decisao-real.json>"
python scripts/content_governance.py record-review --workspace "build/scientific-sources/drafts/<nome>" --key "admission:<id>" --decision "<revisao-real.json>"
# Registrar também tópicos/afirmações/assistências; regenerar os derivados.
python scripts/scientific_catalog.py admission-check --draft "build/scientific-sources/drafts/<nome>" --species-id "<id>"
python scripts/scientific_catalog.py admission-prepare --draft "build/scientific-sources/drafts/<nome>" --species-id "<id>"
```

`--workspace` só aceita um diretório dedicado sob a raiz de drafts; não permite apontar esses comandos para outro projeto ou banco pessoal. A API `build_education.build(root, persist=False)` permite validar sem escrita; `persist=True` escreve **apenas** o derivado no workspace escolhido. O comportamento padrão do gerador existente foi preservado. Workspaces e propostas são locais/ignorados: arquivá-los antes de limpar `build`, sem publicar provas confidenciais.

## 6. Automação e limites

Automatizados: formatos, integridade, ligação de identidade, localizadores, duplicidades possíveis, estrutura filogenética, geração consistente, presença e rastreabilidade de decisões, estados vencidos, prefixos de ledgers, relatório e proposta. Reutilizam-se trava, recuperação, fontes, política de fotos e fila existentes. Nenhum novo serviço remoto foi introduzido; nenhuma Issue foi criada ou atualizada nesta intervenção.

Não automatizados: consentimento, interpretação conclusiva de concessões, autenticidade do responsável, revisão científica/pedagógica humana, reconhecimento visual definitivo, resolução de homônimos, escolha de licença própria, contato com terceiros ou autorização para publicação. Dados e textos requerem análise distinta de fotos; uma justificativa preenchida não é certificação jurídica. Os mecanismos de integridade continuam com os limites documentados em [AUDITORIA_E_CONFIANCA.md](AUDITORIA_E_CONFIANCA.md).

O guard de novos IDs usa o baseline legado preservado e comparado pela CI contra o commit-base, sem inserir novas aprovações nele. Não protege contra um administrador que deliberadamente substitua todas as autoridades e contorne os controles; Git, regras de PR e conferência humana permanecem complementares. As 122 entradas legadas mantêm a política anterior, sem serem automaticamente reprovadas pela nova regra de admissão e sem receberem aprovação retrospectiva.

## 7. Verificações e critérios de expansão

Os resultados efetivamente executados estão registrados na seção de validação ao final desta revisão. Os testes novos usam exclusivamente arquivos, organismos, concessões e declarações **sintéticos**, em diretórios temporários. Não exercitam `confirm_permission` sobre registros reais.

O catálogo poderá receber uma proposta quando identidade/duplicidades, fontes e evidências, licença específica da foto, direitos de dados/textos, revisão editorial rastreável, derivados e validação técnica estiverem completos para o novo recurso. Revisão especializada adicional depende da gravidade/incerteza e de participação real, sem exigir uma equipe inexistente para toda manutenção experimental. Casos ambíguos permanecem pendentes. O gate global de distribuição continua bloqueado enquanto houver pendências aplicáveis, mesmo que a proposta de um organismo seja tecnicamente íntegra.

Decisões humanas restantes: comprovar versões/licenças e atribuições das 120 fotos, revisar os sete materiais, escolher direitos dos materiais próprios, avaliar os dois casos de cativeiro, revisar representatividade das apresentações, conferir mídias das capturas/histórico e resolver obrigações das dependências. Substituição/suspensão de recurso ativo, exclusão e reescrita histórica não estão autorizadas por este relatório.

## 8. Validação executada

| Verificação local | Resultado |
| --- | --- |
| Configuração CMake e build incremental Release/Qt | Concluídos sem erros |
| CTest completo | **26/26 suítes aprovadas**, 171,44 s |
| Novas regressões de admissão, após o refinamento da exportação e dos caminhos | **18 testes aprovados** |
| Investigação/proveniência e inventário, após os últimos ajustes | **20 testes aprovados** |
| Fila de curadoria, incluindo admissão e privacidade | **7 testes aprovados** |
| `audit_catalog.py --strict` | 122 registros preservados e zero erros técnicos |
| `phylogeny.py` sem `--apply` | 122 organismos, zero erros estruturais; fontes sem escrita |
| Geração educativa em memória, créditos e relatório de assistência | Idênticos aos derivados existentes; sem reescrever conteúdo |
| `exploration_media.py` e qmllint | Zero erros; 109 fotos, 534/623 cartões fotografados, 89 pendências documentadas |
| Metadados de versões e `release.py check` | Consistentes; não executado comando de tag/publicação |
| Recuperação de permissões e comparação dos ledgers/baseline com `1f71f95e4c927622632fcea987560e72d8630479` | Estado consistente, prefixos preservados; zero eventos criados |
| Diagnóstico de direitos com pacote local | Bloqueado: 120 fotos, sete achados de outros materiais e dois bloqueios científicos; pacote tem 78 achados de dependências e um erro técnico de manifesto desatualizado |
| `distribution.py check` | Recusa esperada, saída 1; nenhum gate removido |

As fixtures cobrem duplicidade por ID/nome, ausência/divergência de evidência taxonômica, mídia sem licença ou fora da política, crédito ausente, identidade divergente, estado incompleto/pendente/rejeitado, tentativa indevida e preparação legítima de promoção, edição somente do derivado, escopo público insuficiente, idempotência, preservação da base/ledgers, falha de exportação, mudança concorrente de arquivos e exclusão de artefatos privados. A proposta legítima passa pelo guard de novos IDs em catálogo sintético e continua marcada `candidates_only`, sem entrar no catálogo real.

Não foi realizada inspeção visual manual nem validação científica especializada. As suítes QML existentes executaram os componentes reais em ambiente de teste; não houve mudança de interface nem nova captura necessária. Não foram feitas novas consultas bibliográficas/de licenciamento à rede: as conclusões de origem permanecem limitadas aos snapshots datados. A CI da Pull Request é uma execução separada, a ser consultada pelos checks do GitHub; o resultado local não foi apresentado como resultado remoto.
