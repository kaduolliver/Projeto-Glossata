# Saneamento documental da Issue #2

Auditoria de **2026-10-17**, sobre `e2df447cbae2b332de51391e3f34b190b0d74e91` (PR #11 integrado). [Issue #2](https://github.com/kaduolliver/Projeto-Glossata/issues/2), solicitação estável `curation-v1-4f2ce3678bf9c200687be506`. O [registro estruturado](SANEAMENTO_ISSUE_2.json) contém hashes, histórico Git por material, contagens e comparação do gerador. Este relatório é uma investigação documental assistida por IA; não constitui revisão independente, certificação científica ou parecer jurídico.

## Resultado e contagens

**Nenhum dos sete achados de direitos foi aprovado ou encerrado.** A separação entre código e redação do gerador foi concluída e verificada. Sua lógica Python própria passou a integrar explicitamente o escopo Apache-2.0 autorizado; o achado de direitos da redação foi preservado no recurso extraído. O problema técnico de escopo do arquivo misto foi resolvido, sem declarar resolvido o direito sobre seus textos.

A auditoria-base foi reexecutada em uma árvore temporária obtida por `git archive` do commit acima, sem bancos pessoais, consultas de conteúdo ou alteração do checkout original. A auditoria posterior usa o checkout desta intervenção.

| Diagnóstico do gate de fontes | Antes | Depois |
| --- | ---: | ---: |
| Erros técnicos de governança | 0 | 0 |
| Fotografias do quiz com documentação de permissão pendente | 120 | 120 |
| Outros materiais com permissão pendente | 7 | 7 |
| Bloqueios científicos prioritários | 2 | 2 |
| Confirmações humanas criadas nesta intervenção | — | 0 |

O gate continua **bloqueando distribuição**. Os números não demonstram infrações autorais: identificam documentação e decisões faltantes. As pendências de fotos de exploração e dependências continuam análises próprias. A Issue permanece aberta; o PR usa referência sem comando de fechamento. Não houve merge automático, release, tag, alteração das proteções ou publicação de pacote.

## Matriz dos sete materiais

| Material e problema original | Evidências encontradas e direitos demonstrados | Correção realizada ou proposta | Estado final, pendências e ação humana |
| --- | --- | --- | --- |
| `resources/branding/glossata.png` — permissão ausente | Arquivo desde `a9dc4ed`; README da pasta documenta cópia do pacote `glossata_icon_pack`. Mantenedor declarou nesta intervenção: “Criei através do ChatGPT”. Isso documenta ferramenta declarada, sem demonstrar direitos de referências, exclusividade ou licença gráfica | Declaração ligada aos bytes por SHA-256 no inventário; arquivo preservado | `pending_permission`. Informar data/modalidade de criação, referências utilizadas e respectivos direitos; conferir termos aplicáveis e escolher escopo de uso da identidade |
| `resources/branding/glossata_discord.png` — permissão ausente | Mesma introdução no Git/pacote e declaração. Variante documentada para Rich Presence; não há registro individual da geração/edição | Mesmo vínculo documental, sem presumir que o tratamento da imagem principal autoriza toda variante | `pending_permission`. Confirmar relação com o original, alterações e materiais incorporados; manifestação expressa sobre uso/redistribuição |
| `resources/branding/glossata.ico` — permissão ausente | Mesmo commit/pacote. README registra tamanhos de ícone Windows; existência de conversor no repositório não prova que foi utilizado na criação destes bytes | Preservado sem conversão; origem declarada vinculada ao hash próprio | `pending_permission`. Confirmar derivação e direito sobre a arte de origem; conversão de formato não sana direitos faltantes |
| `resources/data/species_profiles.json` — originalidade/direitos | Introduzido em `011b1da`, corpus legado assistido por IA; 122 fichas e 839 objetos de texto com URL. Flags/datas antigas não comprovam revisão humana. Não há concessão documentada de licença da redação | Auditoria estrutural e referências amostrais registradas; nenhuma redação alterada | `pending_originality_and_rights_review`. Revisar paráfrases, eventuais reproduções e fontes por tópico; registrar revisão editorial real; decidir licença apenas das partes próprias após delimitação |
| `resources/data/hint_education.json` — originalidade/direitos | Histórico desde `f4b8705`, com refinamentos em `035d4d0`/`5a119b3`. 122 identidades e 64 textos com URL; justificativas de assistência declaram propostas de IA, sem revisão humana presumida | Mantidos todos os textos, graus, referências, fotos e receitas; lacunas documentais registradas | `pending_originality_and_rights_review`. Conferir etimologias e redação por referência; avaliação pedagógica/editorial distinta de direitos e de revisão especializada |
| `resources/data/progressive_hints.json` — originalidade/direitos | Gerado por `review_hints.py`, introduzido em `d7c523e`, com atualização administrativa em `4d04756`; 122 identidades, vínculo fotográfico e entradas/fontes de origem. `approved_automated` é avaliação pedagógica automática, não autorização autoral | Preservados os bytes, data real da avaliação, referências e pesos. Não foi executada uma nova avaliação para renovar datas | `pending_originality_and_rights_review`. Revisar direitos das redações de origem e das justificativas; derivação local não licencia o corpus automaticamente |
| `scripts/build_education.py` — código misturado com redação pendente | Histórico desde `0ba0eee`; inspeção do arquivo/histórico não identificou avisos ou fragmentos de código externo. Isso não comprova autoria exclusiva ou cessões desconhecidas. A concessão do código próprio já foi expressamente autorizada pelo mantenedor | Extraídas 19 definições e 15 pares de notas para `resources/data/group_education.json`; conferida equivalência; lógica Python incluída explicitamente no escopo Apache-2.0, com SPDX. Caminho/hash/estado anteriores preservados em `inventory_history` | **Separação técnica concluída; direito sobre a redação pendente.** O sétimo registro agora aponta para `group_education.json`, com a mesma permissão negativa e estado. Mantenedor deve revisar originalidade, fontes e licença dos textos; nenhum texto foi relicenciado |

O Git documenta introdução/evolução dos arquivos e declara autores de commits; não demonstra sozinho identidade jurídica, autoria exclusiva ou ausência de componentes externos. O JSON do relatório preserva os commits completos. O ID do grupo automático permanece o mesmo; a chave individual da redação extraída passa de `asset:` + digest do caminho Python para o digest do novo caminho JSON. Trata-se de mudança de localização registrada, não de desaparecimento ou aprovação de um caso. Os outros seis caminhos permanecem iguais.

## Identidade visual: alcance da declaração

A [declaração de origem](../resources/compliance/branding/origin-declaration.json) transcreve somente a resposta efetiva do mantenedor e registra os três hashes. Data de criação, modelo/modalidade, imagens de entrada, registro de geração e termos aceitos naquela ocasião **não foram informados**. A data do registro não foi utilizada como data de criação. A declaração não é consentimento de terceiros nem escolha de licença da marca.

Os [termos atuais da OpenAI](https://openai.com/policies/row-terms-of-use/), consultados em 2026-10-17 e exibidos com vigência de 2026-01-01, separam entrada, saída e materiais de terceiros: atribuem ao usuário eventuais direitos da OpenAI sobre a saída, dentro dos limites legais, e exigem direitos adequados sobre as entradas. Também indicam que saídas podem não ser únicas. As seções consultadas estão no registro; não se presumiu que essa modalidade/versão governou a criação dos arquivos. Esse enquadramento contratual não comprova proteção autoral exclusiva, ausência de referências protegidas ou concessão de direitos de marca.

Procedimento recomendado: o mantenedor deve preservar privadamente o registro original de geração, identificar as entradas e seus direitos, documentar como as variantes derivaram da arte e fornecer uma declaração pública minimizada que vincule os arquivos, o escopo autorizado e as condições aplicáveis. Não publicar conversas completas, dados da conta ou imagens de terceiros desnecessárias. Se referências não puderem ser esclarecidas, avaliar documentação adicional ou uma substituição futura expressamente autorizada. Nenhuma substituição foi feita aqui. Não se afirma registro no INPI, marca registrada, endosso da OpenAI ou exclusividade.

## Redação educativa e evidências científicas

A inspeção integral estrutural dos três JSONs distingue os fatos da forma de redação. A presença de fatos, bibliografia ou assistência de IA não prova originalidade da expressão textual. Não foram encontrados campos que documentem cessão/licença individual da redação nesses três recursos. O corpus permanece classificado como legado assistido por IA. Não há base para declarar todas as frases originais, cópias, citações autorizadas ou paráfrases juridicamente regulares.

Nas fichas existem **839 objetos de texto e 636 redações exatas distintas**; nas dicas, **64 e 63**, respectivamente. Repetição interna pode representar texto de grupo ou cautela compartilhada e não comprova cópia externa. Os 903 objetos têm URL, mas não campos separados `locator`/`section`/`page`/`source_locator`. Parte dos URLs já identifica registros ou seções: a falta desses campos não equivale automaticamente à ausência de evidência. É necessário localizar cada afirmação e verificar se a fonte sustenta o escopo declarado. As receitas progressivas incluem localizadores próprios; isso não sana, por transferência, a redação das fontes.

A conferência amostral encontrou os seguintes encaminhamentos, **sem alterar textos ou fontes para fazer auditorias passarem**:

| Escopo | Consulta/evidência localizada | Resultado documental e encaminhamento |
| --- | --- | --- |
| *Papilio machaon*, alimentação e distribuição | [Animal Diversity Web — conta da espécie](https://animaldiversity.org/accounts/Papilio_machaon/), seções **Food Habits** e **Geographic Range**, consultada em 2026-10-17 | A fonte descreve alimentação larval/adulta e distribuição; oferece apoio para conferência editorial dessas notas. Não é uma avaliação de todos os tópicos nem autorização para reproduzir sua redação |
| Introduções e notas associadas ao OpenStax | [Biology 2e — Preface](https://openstax.org/books/biology-2e/pages/preface), **About OpenStax resources / Customization**, consultado em 2026-10-17 | A página atual indica **CC BY-NC-SA 4.0** para o livro. Não presumir autorização irrestrita ou Apache para reprodução/adaptação textual. Verificar versão efetivamente utilizada e se há adaptação protegida; fatos científicos e redação reproduzida devem ser analisados separadamente. Não se presume revogação de eventual licença anterior válida |
| Saturniidae | [Artigo PMC5723595](https://pmc.ncbi.nlm.nih.gov/articles/PMC5723595/), **Introduction**, trecho sobre alimentação dos adultos, consultado por resultado indexado da própria PMC em 2026-10-17 | Resultado indexado sustenta que adultos geralmente não se alimentam; a abertura direta exibiu proteção de navegador. A evidência é parcial, não uma leitura integral ou aprovação do grupo/corpus |
| Euglenida: referência da introdução extraída | [Registro PMC7052976](https://pmc.ncbi.nlm.nih.gov/articles/PMC7052976/), título indexado **Evolution of metabolic capabilities and molecular features of diplonemids, kinetoplastids, and euglenids**, consulta indexada em 2026-10-17 | O título cobre o assunto geral, mas o rótulo local “Open Biology” exige conferência bibliográfica. A abertura direta exibiu proteção de navegador. Preparar confirmação de periódico/DOI/seção antes de corrigir o metadado; não declarar revisão do conteúdo com base só no título |

Não foram copiados parágrafos das fontes. Não foi realizada comparação exaustiva com toda a literatura, detecção confiável de plágio ou revisão científica especializada. Não se registra um direito como resolvido por tradução ou por ausência de correspondência literal. Termos históricos Monera/Protista e receitas existentes foram preservados integralmente nesta refatoração; a navegação filogenética não foi modificada.

## Separação técnica e rastreabilidade

`group_education.json` é a fonte editável única dos antigos `DEFINITIONS` e `PLAIN_NOTES`. Arrays conservam a ordem, inclusive definições que não geram um nó na árvore atual: são 19 definições de origem e 14 introduções ativas. O carregador valida esquema, tipos, duplicatas, rank, referência HTTPS e pares de notas. Um workspace sem recurso é recusado; não recebe silenciosamente redação do catálogo ativo. Os testes de admissão utilizam um recurso sintético vazio.

O SHA-256 canônico dos valores extraídos, medido antes da alteração, é `6a41c03329d295cf1f7966fe110cf3cef0844bbf71cab41f7ba8dffc2e3a5e5e`. A geração em memória antes/depois é idêntica em todos os campos salvo `generated_source_sha256`, que ganhou o hash da nova fonte. O hash canônico de todos os demais campos é `388472e49b4bfc3717a678671e01f18017c91139c1d17430814562e30a416929`. Não houve mudança de revisão, conteúdo científico, receitas, assistência, pesos ou data de avaliação. O recurso-fonte não precisa de download nem ser consultado pelo executável: o programa continua lendo a projeção embarcada `education.json`.

`asset_rights.json` mantém os mesmos estados e campos de permissão. O registro da redação conserva seu histórico de inventário e acrescenta o caminho/hash do gerador anterior. A declaração de ferramenta dos ícones tem hash próprio e não ocupa os campos de autorização. `content_policy.json`, seus campos `texts`/`collection`, baseline, proveniência fotográfica e ledgers não foram alterados. Não foram criados eventos ou responsáveis humanos fictícios. O manifesto completa o snapshot 2.4.0 ainda não lançado; o aplicativo permanece 1.7.0 em desenvolvimento, sem alteração de regras.

## Decisões humanas e fluxo existente

1. Para os ícones, completar evidências de criação/entradas e decidir autorização de uso dos elementos gráficos. A identificação do criador do projeto já está documentada em NOTICE e não substitui essa decisão.
2. Para cada conjunto de redação, classificar o que é original, paráfrase, citação ou material externo, localizar fontes e esclarecer reprodução/adaptação. Havendo reprodução protegida, conferir a licença específica e condições antes de propor uso; não aplicar CC BY 4.0 automaticamente.
3. Registrar uma revisão editorial real por tópico no ledger existente quando realizada. Usar `review_kind: editorial` e evidências localizadas; não declarar revisão por especialista sem comprovação. Direitos e aprovação científica são dimensões independentes.
4. Para direitos de outros materiais, preparar evidência minimizada identificando recurso/hash, titular ou declarante fundamentado, origem, direitos disponíveis e escopo autorizado. O inventário `asset_rights.json` já suporta `holder`, `evidence_path`, `evidence_sha256` e `distribution_allowed`; alterações legítimas devem ser revisadas em PR e preservar o histórico. **A CLI `confirm-permission` é específica de fotos do catálogo**; não usar IDs fictícios para simular uma autorização de texto ou branding. O bridge atual oferece decisões de revisão, fotografia e dependência, não um aprovador genérico de materiais próprios.
5. Reexecutar auditorias, regenerar as projeções aplicáveis e atualizar manifesto/histórico após uma decisão legítima. A fila `curation_requests.py plan` reutiliza o mesmo grupo da Issue #2. Fechar Issue, alterar etiqueta, inserir LICENSE ou passar testes não aprova um recurso.

Os documentos anteriores de licenciamento permanecem históricos, com remissão datada à extração. A enumeração Apache atual tem 156 arquivos; nenhum dos três JSONs educativos, do novo recurso de redação ou dos ícones integra essa concessão de código. O caminho de relato permanece [REPORTAR_PROBLEMA.md](REPORTAR_PROBLEMA.md).

## Verificações executadas

- Importação/geração: **16 testes aprovados**, incluindo equivalência do corpus extraído, reprodução offline, isolamento de workspace e rejeição de dados inválidos.
- Admissão: **18 testes aprovados** com fixtures sintéticas; sem promoção de espécie real.
- Investigação de direitos: **35 testes aprovados**, incluindo texto Apache oficial, escopo explícito, ausência de concessão de mídia e bloqueio de material misto pendente.
- Auditoria estrita: **122 organismos**, 122 registros fotográficos válidos, 120 fotos distintas do quiz, pool competitivo 80, sem erros técnicos; fotos/etimologia/revisão mantêm suas limitações já documentadas.
- Comparação dos arquivos: **341 mídias existentes em `resources/` com hashes idênticos**; fichas/dicas/progressão, espécies, proveniência, créditos, política, baseline, LICENSE, NOTICE e VERSION preservados byte a byte. O relatório JSON enumera as verificações.
- Auditoria dos três ledgers e baseline contra o commit-base completo: **prefixos preservados**, sem eventos adicionados. `licensing_inventory.py --check-code`: **sem erros**.
- Compilação Release executada; **26/26 suítes CTest aprovadas** em 170,31 segundos, incluindo interface, recuperação de partidas, identidade permanente, Unicode, concorrência e recuperação da governança. As verificações complementares e a execução remota de `offline-validation` são registradas no PR.
- `distribution.py check`: **bloqueio esperado**, com 120/7/2 pendências conforme a tabela. Não foi produzido pacote de distribuição.

A compilação, a integridade e os testes automatizados não certificam originalidade, autorização jurídica ou correção científica integral. As sete decisões de direitos permanecem responsabilidade do mantenedor, com eventual apoio qualificado quando necessário.
