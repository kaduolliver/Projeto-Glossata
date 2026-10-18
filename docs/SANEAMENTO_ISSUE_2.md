# Saneamento documental da Issue #2

## Continuidade: diagnóstico atual e redação técnica — 2026-10-18

**A Issue #2 ainda não pode ser encerrada legitimamente.** A base desta etapa é o merge do PR #20, `ba57dfe037915b81c86f03c7ecd9782374c6fb0e`. As aprovações concretas anteriores foram preservadas; elas não alcançam outras redações por semelhança, arquivo ou assunto. Os registros abaixo desta seção documentam etapas anteriores e não substituem este diagnóstico atual.

### O que foi realizado

- Reexecutada a investigação offline, reutilizando as evidências arquivadas: nenhuma nova consulta de conteúdo externo, download, contato ou atribuição de autoria.
- Atualizado **somente o bloco automático da [Issue #2](https://github.com/kaduolliver/Projeto-Glossata/issues/2)**: de sete achados desatualizados para **três recursos pendentes**, com evidência e hash da `main` publicada. Preservados o identificador estável, o acompanhamento humano e o estado aberto. Nenhuma outra Issue foi sincronizada.
- Acrescentado ao relatório existente `rights_investigation.py` o resumo `other_materials_resolution`, que enumera os vínculos ainda pendentes e diferencia a prontidão para conferência do encerramento da Issue. Não altera permissões, estados de revisão nem o gate.
- Preparadas [15 redações técnicas compartilhadas](propostas/REDACAO_TECNICA_ISSUE_2.md), cobrindo **1.147 ocorrências**: 290 justificativas de origem, 855 derivadas e duas notas da avaliação automática. A proposta remove a necessidade de reproduzir etimologias dentro das justificativas, mantendo as etimologias originais intactas. É **rascunho não aplicado**, sem licença ou aprovação nova registrada.
- A comparação em workspace temporário verifica os ponteiros e hashes anteriores, as quatro substituições literais do gerador e a equivalência de todas as afirmações e tópicos científicos. Receitas diferem somente na justificativa; graus, restrições, categorias, pontos, textos biológicos e vínculos fotográficos são preservados. A conferência dessa redação não aprova assistência ou ciência.

Fingerprint da proposta: `87f0ab41d22b1e707dbd39723d2ab6cf2dc617606e6eb7fbb096ac5cbaaa1d1e`. A lista completa está no [JSON de conferência](propostas/REDACAO_TECNICA_ISSUE_2.json). A licença proposta é CC BY 4.0 somente nos direitos efetivamente disponíveis nessa expressão textual assistida por IA; Apache-2.0 do código, direitos científicos, fotos e identidade visual conservam seu escopo independente.

### Situação dos materiais

| Material originalmente apontado | Estado atual e limite | Pendência nesta etapa |
| --- | --- | --- |
| `glossata.png` | Uso/distribuição oficial documentados por decisão anterior; identidade reservada | Sem nova concessão; não se declara exclusividade gráfica |
| `glossata_discord.png` | Mesmo escopo oficial delimitado | Sem nova concessão |
| `glossata.ico` | Mesmo escopo oficial delimitado | Sem nova concessão |
| `build_education.py` | Lógica separada da redação e expressamente no escopo Apache-2.0 | O recurso extraído `group_education.json` recebeu a concessão delimitada anterior para 68 ocorrências; nenhuma herança para outros textos |
| `species_profiles.json` | 34 notas documentadas; arquivo inteiro não liberado | **805 ocorrências**: 122 introduções, 122 distribuições, 88 nutrições, 121 morfologias, 108 curiosidades, 122 dicas e 122 conservações |
| `hint_education.json` | Sem concessão global | **374 ocorrências**: 290 justificativas, 64 etimologias e 20 prefixos |
| `progressive_hints.json` | Projeção gerada, sem concessão global | **1.121 ocorrências**: 855 justificativas, 264 cópias biológicas e duas notas da avaliação |

São **2.300 ocorrências inconclusivas**, agrupadas em **788 hashes de redação**. Não são 2.300 obras independentes, nem prova de infração. Os vínculos individuais estão no inventário já existente, `resources/compliance/asset_rights.json → educational_text_rights`; o [relatório JSON](SANEAMENTO_ISSUE_2.json), seção `remaining_text_regularization`, agrupa a pendência por recurso, tópico e referência declarada. Regeneração: `python scripts/review_updates.py --rights-report`.

Foram inspecionados os compositores locais ignorados de fichas, etimologias e assistência. Seus hashes foram registrados como observação técnica; não foram publicados seus textos integrais. Comentários que chamam uma redação de original e campos legados `verified` não comprovam direitos, data histórica de criação ou revisão humana. Também não basta uma referência ao OpenStax para demonstrar cópia ou independência da redação. Permanecem inconclusivos os trechos cuja derivação autoral não foi demonstrada.

### Caminho para concluir a Issue

1. Conferir o lote técnico exato e confirmar separadamente a concessão dos direitos disponíveis. Após isso, aplicar apenas os ponteiros vinculados, regenerar e testar, sem aprovar graus de assistência. **Mesmo essa aplicação deixaria 1.153 ocorrências pendentes.**
2. Tratar as fichas, etimologias e prefixos restantes por lotes fundamentados: demonstrar a origem e o direito existente ou preparar nova síntese documentada, sem traduções/cópias ocultadas como redação própria. A revisão editorial do mantenedor deve ser registrada para a versão concreta, distinta de revisão por especialista.
3. Propagar evidências às cópias geradas, mantendo cada vínculo; concessões parciais não liberam o arquivo misto. Exigir cobertura completa e decisão efetiva antes de alterar permissões do recurso inteiro.
4. Reexecutar o gate e conferir a `main` integrada. Somente com zero achados de materiais e evidência completa haverá base para decidir o encerramento desta Issue; fotografias, curadoria científica e dependências continuam independentes.

A autorização geral para resolver o problema não foi convertida em declaração fictícia de autoria, revisão ou licença sobre textos legados de origem desconhecida. As duas conferências do lote concreto foram solicitadas; enquanto não houver respostas, ele permanece proposta.

### Verificação e preservação

Não houve alteração em recursos ativos, ícones, fotografias, créditos, 122 organismos, pool competitivo de 80, pontuação, perfis ou dados dos jogadores. Os **76 eventos editoriais** e os ledgers de permissões continuam idênticos à base; nenhuma aprovação de especialista, grau de assistência ou permissão foi criada. App e catálogo continuam nas versões em desenvolvimento existentes, sem tag, release ou pacote.

Configure e compilação Release/Ninja aprovados; **26/26 suítes CTest passaram em 185,38 segundos**, incluindo os **55 testes de investigação de direitos** e **22 de importação/geração**. As regressões novas recusam cobertura parcial mesmo com redações iguais, metadados ausentes e tentativa de usar prontidão de um material para liberar o pacote; verificam também os 1.147 vínculos da proposta, sua derivação e a preservação científica.

Passaram ainda: verificação Apache-2.0, auditoria estrita, recuperação de permissões, regras locais do repositório, inventário do repositório, exploração, cobertura científica, assistência, histórico de governança frente à base publicada, metadados/histórico de lançamento e `qmllint`. Dicas progressivas, educação, créditos e filogenia foram regenerados pelo fluxo existente, com bytes dos recursos ativos **idênticos à base**. A investigação e as propostas de atualização executaram sem rede de conteúdo e sem decisões. A proteção remota da branch não foi alterada; o check local não foi apresentado como nova inspeção remota.

Esses resultados correspondem à validação **local**. Nenhum sucesso remoto desta branch foi presumido, nem houve validação visual manual ou de pacote.

O comando `python scripts/distribution.py check` manteve retorno **2**, com **zero erros técnicos, 120 pendências fotográficas, três textuais e 31 bloqueios científicos de alta prioridade**. Estes últimos incluem as duas fotos prioritárias e os 29 agregados modificados no lote anterior, fora da aprovação concedida. Não houve nova validação de pacote ou renderização visual; testes não certificam direitos ou ciência.


## Lote 2 confirmado e incorporado — 2026-10-18

O mantenedor confirmou **“Aprovo editorialmente esta versão completa”** e, separadamente, **“Confirmo a aplicação delimitada a este lote”**, para o fingerprint `e0d6ac843d4da1111698d4c1f478840e231cedb7a6f67949318bcc1ad2e55478`. A confirmação concreta está na [evidência imutável](../resources/compliance/sources/2e9ce124b2056fdb0683dbd55b15dd2391f6e42bb0ffb1dd712ecb572510c5ce.json), SHA-256 `f0651e9f4f618f2e9a36a3d04bc8d300ab00b21e61f2dc4c91e5fdf6bc5f324d`, com perguntas, respostas, 34 notas, vínculos, limites e data civil real. A identidade é declarada, sem assinatura digital ou autenticação independente; não se inventou horário da resposta ou revisão especializada.

Foram substituídas **34 notas em 28 organismos**, correspondentes às nove redações da [conferência aprovada](propostas/REVISAO_FICHAS_LOTE_2.md). O arquivo resultante tem SHA-256 `a3e485307481475e8d320dacde35f437fc35a59b74eb9b54870a7fd652b66055`. A comparação com a proposta confirma igualdade dos objetos aprovados e ausência de outras alterações nas fichas. Os bytes anteriores e a versão apresentada no commit `63599f2` permanecem no Git. As oito referências utilizadas conservam autores, DOI, licença específica CC BY 4.0, localizadores e evidências; a nona evidência histórica substituída não foi apagada. Não foi incorporada imagem ou reprodução textual dos artigos.

O bridge de decisões e a trava comum registraram **35 eventos editoriais**: 34 notas e o conteúdo científico idêntico no alias morfológico de `110`. O ledger passou de 41 para 76 eventos, preservando integralmente o prefixo anterior. Repetir cada envelope retornou `already_recorded`. Os campos legados das novas notas ficam `verified: false` / `reviewed_on: null`; a responsabilidade e data reais estão em `governance`, como `approved_editorial`. As três decisões de `82` e as 38 de grupos continuam intactas.

A aprovação **não abrange tópicos agregados** que incluem outras passagens. Os agregados educativos dos 28 organismos e o agregado morfológico de `110` refletem `needs_review`; isso não oculta notas individuais não alteradas nem as 35 agora aprovadas. As demais informações das fichas, dicas válidas e a jogabilidade permanecem preservadas. Aprovação editorial não significa revisão por especialista, comprovação da fotografia ou aprovação da dificuldade da dica.

**Direitos:** 34 ocorrências em `educational_text_rights` recebem `independent_factual_synthesis`, `documented` e CC BY 4.0 somente sobre direitos disponíveis nessas redações assistidas por IA. Crédito: **Projeto Glossata — redação assistida por IA; revisão editorial de Carlos Eduardo Lima de Oliveira**. Preservar identificação do lote, [origem](https://github.com/kaduolliver/Projeto-Glossata), [licença](https://creativecommons.org/licenses/by/4.0/), avisos, fontes e indicação de alterações. Não se reivindica autoria humana exclusiva, exclusividade sobre fatos ou direitos sobre os artigos.

As **805 outras ocorrências de `species_profiles.json` continuam inconclusivas**. A permissão do arquivo inteiro permanece `distribution_allowed: false`, assim como a de `progressive_hints.json`; não houve licença global de textos/coletânea. O validador existente foi ampliado para conferir a concessão parcial contra os ponteiros, hashes e a proposta imutável confirmada, recusando reutilização em outro arquivo/texto, ampliação da licença ou confirmação incompleta. A verificação é documental, não autenticação humana ou certificação jurídica. A política e as licenças de fotos, código e branding continuam independentes.

**Organismo 110:** a observação pertence à cultura DSM 107014 (= SAG 52.96), sem associação presumida à fotografia de apoio `160680326`. A ressalva educativa foi mantida. O fluxo existente regenerou a aparência e a receita correspondente; somente texto, fonte e localizador acompanham a origem. Os 121 outros conjuntos de receitas permanecem idênticas; categorias, graus, níveis, valores de pontos e vínculos fotográficos foram preservados. A assistência continua sem aprovação humana e a identificação visual não foi liberada. Duas ocorrências da receita composta tiveram seus hashes atualizados, mantendo direitos inconclusivos do conjunto; não receberam concessão independente por derivação.

| Indicador | Antes do lote 2 | Após incorporação |
| --- | --- | --- |
| Notas deste lote incorporadas | 0 | 34, para 28 organismos |
| Eventos editoriais totais | 41 | 76; 35 novos, nenhum de especialista |
| Ocorrências de fichas com concessão delimitada | 0 | 34; arquivo inteiro ainda bloqueado |
| Ocorrências das fichas com origem inconclusiva | 839 | 805 |
| Ocorrências inconclusivas em todo o inventário | 2.334 / 795 hashes | 2.300 / 788 hashes |
| Achados textuais da Issue #2 | 3 | 3: fichas, dicas de origem e receitas compostas |

O inventário permanece com 2.402 ocorrências, agora agrupadas em 865 hashes; diversidade de redações não é contagem de recursos autorizados. A redução de 34 pendências por ocorrência não resolve o achado do arquivo inteiro. As seções abaixo preservam a preparação e o primeiro lote como registros anteriores. A Issue #2 permanece aberta. Próximo lote recomendado: outras passagens de fichas, priorizando os agregados alterados e mantendo a aprovação de `82`; investigar origem por ocorrência, sem ampliar concessões anteriores.

### Validação da incorporação

- Configure e compilação Release/Ninja aprovados; **26/26 suítes CTest passaram em 218,67 segundos**, incluindo pontuação, persistência, recuperação de sessões/perfis e governança. Não se declara validação visual manual ou de pacote.
- **21 testes de importação/geração** e **53 de investigação de direitos** passaram. As regressões históricas usam o estado anterior à confirmação em workspace temporário; as novas cobrem promoção exata, revisão somente das notas autorizadas, preservação das demais, invalidação pontual após mudança, concessão parcial e tentativas de ampliar ponteiro/arquivo/licença. Nenhum teste altera registros reais.
- Auditoria estrita: **122 organismos e pool competitivo de 80**. Apache, governança/histórico, manifestos/estado, proteção proposta do repositório, investigação documental, exploração, cobertura, assistência, filogenia e `qmllint` passaram. Os geradores mantêm bytes idênticos na repetição; não renovam uma avaliação válida. As mídias, os créditos, a política, o lote de grupos, o organismo 82 e as regras do jogo foram comparados à base e preservados.
- Curadoria: `plan` passou de **15 solicitações / 3.309 achados para 15 / 3.274**, preservando os identificadores das solicitações. `sync` sem `--apply`: simulação offline, zero alterações remotas; Issue #2 aberta. A redução é de 35 revisões rastreáveis, sem reduzir artificialmente achados do arquivo inteiro.
- Gate de distribuição: retorno **2**, **zero erros técnicos**, **120 pendências fotográficas**, **três textuais** e **31 bloqueios científicos**. São os dois bloqueios fotográficos preexistentes mais **29 agregados alterados** (28 tópicos educativos e a morfologia de 110) que ficaram explicitamente fora da confirmação. Não se usa a aprovação de uma nota para aprovar um agregado com outras passagens. O próximo lote deve tratar esses agregados com análise e decisão próprias; não foi relaxado o gate.

Os logs detalhados desta rodada ficam localmente em `build/lote2-validation-*.log`, com comandos/resultados no JSON deste relatório; a CI repetirá os controles após a publicação da branch. Resultados remotos serão registrados quando efetivamente observados. Não houve release, tag, pacote, merge, contato externo ou mudança nas proteções da main.

## Preparação do lote 2 — registro anterior às confirmações, 2026-10-18

O PR #19 foi confirmado como integrado: merge `09e8eb759172e32b9554c4c86d8a583a667e4dcc`, em 2026-10-18. A `main` local foi atualizada por avanço direto e a revisão ocorre em `fix/issue-2-species-profiles-lote-2`, com workspace inicialmente limpo. O lote de grupos aprovado e os registros anteriores foram preservados. As seções seguintes registram etapas anteriores e não descrevem necessariamente o estado remoto atual.

A [conferência única das fichas](propostas/REVISAO_FICHAS_LOTE_2.md) apresenta nove redações para 34 campos de 28 organismos, com comparação, fontes, licenças específicas, limitações e vínculos individuais. **Fingerprint final da proposta: `e0d6ac843d4da1111698d4c1f478840e231cedb7a6f67949318bcc1ad2e55478`.** A versão anterior, fingerprint `a6139ee7e5e468cec7506a282c90ceb5eaa9d895b5d7c5acf1595bd770be5460`, permanece no Git; a revisão estruturada está em `profile_openstax_batch.proposal_revision` no JSON existente.

Foram mantidas quatro redações e ajustadas cinco, alterando nove das 34 ocorrências propostas. As ressalvas administrativas de predação, comparação de plantas, parceiros de líquen e comprovação fotográfica das cianobactérias foram deslocadas para os metadados e a conferência. A ressalva de *Gomphosphaeria* permanece no texto por ser necessária para distinguir cultura estudada e imagem de apoio. Nenhum resultado experimental de *Haania orlovi* ou *Cladonia grayi* foi atribuído às espécies do catálogo.

A revisão encontrou uma lacuna concreta na referência anterior sobre micélio: Challacombe et al. (2019) não sustentava claramente o trecho sobre estruturas reprodutivas. A nova síntese distingue hifas/micélio e corpos de frutificação, usando [Islam et al. (2017)](https://doi.org/10.1038/s41598-017-13295-2) e [Berger et al. (2022)](https://doi.org/10.3390/microorganisms10071379). Foram arquivados somente metadados bibliográficos e licença específica CC BY 4.0, com datas reais e hashes; figuras e texto integral não foram incorporados. As sete evidências anteriores continuam preservadas; oito referências sustentam a versão final. As referências alternativas que não apresentaram licença aberta comprovada nos metadados não foram baixadas nem usadas na proposta. A referência bibliográfica anterior ao OpenStax não foi tratada como prova de infração.

**Estado atual: proposta, aguardando duas confirmações distintas do mantenedor.** A primeira trata da revisão editorial das notas exatas e do mesmo conteúdo científico no alias morfológico de `110`; a segunda trata de CC BY 4.0 somente nos direitos efetivamente disponíveis nessas redações assistidas por IA. Nenhuma aprovação é herdada do lote de grupos, do conteúdo legado ou da existência de referências. Tópicos agregados que incluem outras passagens e graus de assistência ficam fora da aprovação solicitada.

`species_profiles.json` continua com SHA-256 `df137441551f36ce54e326c944bd8292221b9e537481a5c84e084602272a8bec`; não foram alterados conteúdo ativo, permissões ou ledgers. Permanecem 122 organismos, pool competitivo de 80 e as decisões do organismo 82. As demais passagens das fichas continuam inconclusivas; não se declara o arquivo regularizado. Após respostas concretas, somente o subconjunto aprovado poderá ser incorporado, regenerado e validado antes de publicar a branch e abrir o PR. A Issue #2 permanece aberta; nenhum pacote, tag ou release será criado.

### Verificações desta preparação

- Configure e compilação Release/Ninja aprovados; **26/26 suítes CTest passaram em 196,21 segundos**, incluindo pontuação, persistência, recuperação, governança e admissão. Não se presume validação remota ou visual manual.
- **20 testes de importação/geração** e **52 de investigação de direitos** passaram. A nova regressão vincula proposta, documento de conferência, fonte, fingerprint e escopo aos 34 ponteiros originais. Na simulação temporária, 35 notas derivadas mudam; 121 receitas ficam idênticas e a aparência de `110` preserva graus/categorias/níveis/foto. Regenerar não aprova cientificamente as novas notas.
- Auditoria estrita: 122 organismos e pool competitivo de 80. Apache, integridade de manifestos/estado, histórico frente ao merge do PR #19, repositório, exploração, cobertura, assistência e `qmllint` aprovados. Educação, créditos e filogenia reproduzíveis; dados ativos e os 41 eventos editoriais permanecem idênticos à base.
- `curation_requests.py plan` e `sync` **sem `--apply`**: 15 solicitações / 3.309 achados, como antes; `remote_checked: false`, zero criações/atualizações. Os indicadores não foram reduzidos por existirem propostas.
- Gate de distribuição: retorno **2**, zero erros técnicos, 120 pendências fotográficas, três textuais e dois bloqueios científicos. Os bloqueios continuam ativos. As 34 notas ainda não receberam revisão humana nem licença; a incorporação e o PR dependem das duas confirmações solicitadas na etapa 5 do pedido.

Estes resultados verificam a preparação com o catálogo ativo preservado. Depois da confirmação e aplicação efetivas será necessário regenerar os recursos e executar novamente as verificações pertinentes, inclusive CTest completo. Não se usa esta rodada como aprovação de um lote ainda não aplicado.

## Consolidação para Pull Request — 2026-10-18

A branch `fix/issue-2-educational-text-rights` preserva o merge do PR #18 (`ed7e5c44e1acea80674b3c3bf402071d0836454c`) como base. Na conferência remota desta etapa, `main` continua nesse commit e protegida; não existia outro PR para a branch. A solicitação de continuidade autoriza publicar a branch e abrir o PR após a validação, sem merge ou encerramento da Issue. Nenhuma redação aprovada, evento editorial ou evidência de confirmação foi recriada.

Foi preparada a [comparação das 34 propostas para fichas](propostas/REVISAO_FICHAS_LOTE_2.md), com índice por organismo/campo, redação atual/proposta, bibliografia, localizador, licença verificada, evidência, limitações e impacto previsto. É uma apresentação do registro existente, sem nova investigação externa ou alteração das propostas. O fingerprint das 34 mudanças é `a6139ee7e5e468cec7506a282c90ceb5eaa9d895b5d7c5acf1595bd770be5460`; a fonte ativa `species_profiles.json` permanece intacta. Os nove textos compartilhados conservam os 34 vínculos individuais. Não se registrou nova aprovação ou concessão.

A conferência não encontrou nova inconsistência estrutural. Permanecem os cuidados já documentados: distinguir grupo e espécie, não transferir resultados de *Cladonia grayi* ou *Haania orlovi*, não chamar *Euphorbia* de planta sem flores e não identificar *Gomphosphaeria* pela imagem de apoio. Isso não equivale a revisão científica humana. A sequência dos lotes 2–5 está preparada no comparativo, condicionada à integração do primeiro PR e às decisões concretas aplicáveis.

**Nova validação local para publicação:** configure/compilação Release aprovados; **26/26 suítes CTest passaram em 154,66 segundos**. Gerador, direitos, governança, dicas progressivas e demais regressões foram executados pelo CTest. Auditoria estrita, Apache, manifesto/estado, histórico frente ao PR #18, repositório, exploração, cobertura, assistência e `qmllint` passaram; os derivados são reproduzíveis. O gate mantém retorno **2**, com zero erros técnicos, 120 pendências fotográficas, três textuais e dois bloqueios científicos prioritários. Os testes não certificam direitos ou ciência; não foi validado um novo pacote.

Os resultados remotos devem ser verificados nos checks do PR publicado; a entrega e seu corpo registram as URLs e conclusões efetivamente observadas. Esta anotação documenta verificações locais e não presume sucesso remoto. Os registros abaixo preservam as etapas anteriores à publicação.

## Lote de grupos confirmado e aplicado — 2026-10-18

O mantenedor respondeu **“Aprovo editorialmente essa versão”** e, separadamente, **“Confirmo a aplicação delimitada ao lote”**. As perguntas completas, respostas reais, data civil, 68 vínculos e limites estão na [evidência imutável da confirmação](../resources/compliance/sources/4ee212a1aabf07d279a3732180fe819413b8daad932a8f8721c3acd141b2997c.json). A identidade é declarada, sem assinatura digital ou autenticação independente. Não se inventou o instante das respostas nem uma revisão por especialista.

O arquivo ativo `resources/data/group_education.json` foi substituído **pelos bytes exatos aprovados**, SHA-256 `1d62e0bc4e834d6548338f2787667e328704930ef0027ed588e5b2bbc4e6d23f`. São 68 ocorrências para 19 definições e 15 conjuntos complementares, com referências e licenças individuais preservadas. As redações legadas, seus hashes e os registros preparatórios abaixo permanecem no Git e no histórico do inventário.

Foram registrados **38 eventos editoriais** para as notas efetivamente geradas pelo aplicativo, pelo bridge existente e sob a trava comum. Cada evento está vinculado ao conteúdo atual, escopo de grupo, evidências científicas e confirmação concreta. A repetição dos mesmos envelopes retornou `already_recorded`; o ledger passou de três para 41 eventos, preservando o prefixo anterior. As três decisões do organismo 82 não foram alteradas. A aprovação de conteúdo não aprova graus de assistência.

O gerador agora conserva os campos legados `verified` e `reviewed_on` somente enquanto a afirmação, a fonte e o escopo forem iguais. Nas novas notas esses campos ficam `false`/`null`; a revisão real está exclusivamente em `governance`, como `approved_editorial`, responsável Carlos Eduardo Lima de Oliveira e data **2026-10-18**. Uma alteração posterior invalida a nota modificada e não suspende as demais notas aprovadas. O teste histórico de extração continua usando os valores originais da base Git, sem substituir seu hash esperado pela nova redação.

A concessão **CC BY 4.0 abrange apenas os direitos efetivamente disponíveis nas 68 redações novas**. Crédito do lote: **Projeto Glossata — redação assistida por IA; revisão editorial de Carlos Eduardo Lima de Oliveira**. Preservar identificação do lote, [origem do projeto](https://github.com/kaduolliver/Projeto-Glossata), [licença](https://creativecommons.org/licenses/by/4.0/), avisos e indicação de alterações. Autores, títulos, URLs, licenças/versões e localizadores das fontes científicas continuam em `group_batch.source_evidence` e nos snapshots referenciados. Não se atribui ao mantenedor a autoria dos artigos, dos fatos científicos ou autoria humana exclusiva dessas redações.

O contrato `educational_text_rights` foi atualizado para as 68 ocorrências ativas, com origem, processo, evidência, crédito e concessão delimitados. A permissão do recurso é `eligible_group_wording_only`, vinculada ao mesmo arquivo e à confirmação. O gate recusa a reutilização dessa prova para outros bytes, escopos ou recursos, mesmo que se atualize apenas o hash do inventário. `own_material_licenses.texts` continua `null`: não existe licença global sobre o corpus. Fotografias, identidade visual, coletânea, artigos externos e código conservam seus direitos separados.

A nova assinatura do ledger exige regenerar a avaliação automática das dicas e a projeção educativa. Isso foi feito pelo fluxo existente, com **os 122 conjuntos de receitas idênticos**, incluindo redações, fontes, graus, alternativas taxonômicas e vínculos fotográficos. Mudaram somente as entradas/metadados automáticos necessários; não houve nova aprovação científica das dicas. Os 122 organismos e o pool competitivo de 80 permanecem preservados.

| Indicador | Preparação anterior | Após a confirmação deste lote |
| --- | --- | --- |
| Ocorrências novas promovidas | 0 | 68 de grupos |
| Eventos editoriais novos nesta fase | 0 | 38; nenhum de especialista |
| Concessões textuais delimitadas | 0 | 1 recurso / 68 ocorrências |
| Ocorrências ativas com origem ainda inconclusiva | 2.402 | 2.334, agrupadas em 795 hashes |
| Achados textuais da Issue #2 | 4 | 3: fichas, dicas de origem e dicas derivadas |
| Fila local de curadoria | 15 solicitações / 3.348 achados | 15 solicitações / 3.309 achados |

As **34 propostas de fichas para 28 organismos continuam como rascunhos**, fora desta confirmação. Não foram aplicadas, licenciadas ou aprovadas editorialmente por extensão. Permanecem também as demais passagens das fichas, etimologias/justificativas e seus derivados. O achado de `group_education.json` foi resolvido somente no escopo delimitado e verificável acima; a Issue #2 permanece aberta e a regularização integral não foi declarada concluída. A sincronização segue simulada, sem escrita remota.

### Validação da promoção confirmada, anterior à publicação

- Configure e compilação Release aprovados; **26/26 suítes CTest passaram**, em **133,44 segundos**, na rodada final.
- **19 testes de importação/geração** e **52 testes de investigação de direitos** passaram. Cobrem a versão exata, invalidação pontual após mudança, preservação do organismo 82 e recusa de concessão emprestada, escopo ampliado ou evidência malformada; usam cópias temporárias e fixtures sintéticas.
- A primeira rodada CTest encontrou uma expectativa legada em `introductionsAndMissingContent`, que exigia `verified: true`. O teste passou a exigir a revisão editorial real, responsável, data, hash e utilização permitida, mantendo os campos legados vazios. Foram preservadas as exigências de qualidade/tamanho e cobertura das introduções. Após recompilar, a suíte completa foi repetida com sucesso. A regressão histórica de extração usa o Git da base registrada; a CI já faz checkout com histórico completo.
- Auditoria estrita: **122 organismos, pool competitivo 80**, sem erros. Receitas dos 122 organismos idênticas à base; fotos, créditos, fontes das fichas/dicas e prefixos históricos preservados. Apache, recuperação de permissões, auditoria do repositório, exploração, cobertura, assistência, manifesto/estado, auditoria histórica, histórico de captura e `qmllint` aprovados localmente.
- `distribution.py check` continua com retorno **2**: **zero erros técnicos, 120 pendências fotográficas, três textuais e dois bloqueios científicos prioritários**. Não foi produzido ou validado um novo pacote; obrigações de dependências continuam independentes.
- Plano e sincronização simulada: **15 solicitações / 3.309 achados**, `apply: false`, `remote_checked: false`, nenhuma Issue criada/atualizada ou erro. Não houve push, merge, release, tag ou encerramento da Issue. Testes e declarações não constituem certificação jurídica, revisão especializada ou autenticação de autoria.

## Registro preparatório anterior à confirmação — 2026-10-18

O registro abaixo descreve a etapa de propostas antes das respostas reais. Seus números e pendências devem ser lidos naquele contexto; o estado atual está na seção acima.

PR #18 confirmado como integrado em `ed7e5c44e1acea80674b3c3bf402071d0836454c`. A branch `fix/issue-2-educational-text-rights` parte dessa base limpa. Os registros autênticos dos ícones e do organismo 82 permanecem intactos; esta etapa não solicita nova escolha de identidade ou de licença padrão.

As **68 ocorrências de grupos** foram examinadas e receberam propostas de substituição em [propostas/group_education.json](propostas/group_education.json): 19 definições e 15 conjuntos de notas, preservando as lacunas vazias anteriores. São redações novas assistidas por IA, com referências localizadas e processo declarado; não se afirma autoria humana exclusiva ou originalidade universal por ausência de correspondências. O SHA-256 da proposta é `1d62e0bc4e834d6548338f2787667e328704930ef0027ed588e5b2bbc4e6d23f`. O JSON deste relatório, em `educational_regularization.group_batch`, contém os vínculos por ponteiro/hash, evidências e limitações.

**A proposta ainda não substitui o arquivo ativo.** Sua revisão editorial humana e a confirmação da aplicação do padrão CC BY 4.0 aos direitos efetivamente disponíveis neste lote permanecem pendentes. Isso não reabre a escolha da licença. O objetivo é conferir a versão concreta antes de registrar qualquer decisão ou promover a nova redação. Os quatro recursos textuais continuam bloqueados; nenhum achado foi encerrado nesta etapa.

| Escopo examinado | Fonte científica e localizador utilizados |
| --- | --- |
| Animalia | Jacques et al. (2022), introdução/multicelularidade; Adl et al. (2019), Holozoa/Metazoa |
| Plantae e Tracheophyta | Fangel et al. (2012), parede celular; Woudenberg et al. (2022), introdução e tabela 1, tecidos condutores |
| Fungi, Ascomycota e Basidiomycota | Challacombe et al. (2019), enzimas e hifas; Adl et al. (2019), diagnósticos de Fungi/Dikarya |
| Monera e Protista, somente como história | Fisler et al. (2020), árvores e sistemas históricos; Adl et al. (2019), linhagens eucarióticas atuais |
| Arthropoda e Insecta | Klementz et al. (2024), apêndices articulados; Jürgens et al. (2024), cabeça/tórax/abdome; Schumann e Triphan (2020), cutícula e muda |
| Chordata | D'Aniello et al. (2023), caracteres compartilhados; imagens do artigo excluídas |
| Amoebozoa, Myxogastria e Mycetozoa | Adl et al. (2019), diagnósticos; Rikkinen et al. (2019), ciclo de vida e implicações evolutivas; nomes da fonte do catálogo preservados |
| Lepidoptera, Papilionidae e Saturniidae | Seah e Saranathan (2023), escamas; Ballesteros Mejia et al. (2020), lagartas e adultos. Hall, UF/IFAS IN219, fornece somente o fato de existir representante sem caudas; sua redação e imagens não são reproduzidas ou adaptadas |
| Euglenida | Butenko et al. (2020), Background, película e diversidade nutricional |
| Chiton | Irisarri et al. (2020), Background, placas e pé; vínculo do gênero preservado no catálogo |

Foram conferidos metadados e trechos pertinentes de **15 publicações abertas**, sem afirmar leitura integral: 14 com CC BY 4.0 e uma com CC BY 3.0. A versão 3.0 de Fangel et al. não foi substituída por 4.0; a licença proposta alcança somente os direitos disponíveis sobre as novas redações, preservando direitos/atribuição das fontes. Em Schumann e Triphan, o XML declara CC BY sem versão: a versão 4.0 foi conferida no link de copyright **do artigo específico**, sem presumir a licença geral de todo o portal. Há uma referência factual complementar, com licença não inferida. Fontes científicas e origem da expressão são registros separados.

As evidências selecionadas em `resources/compliance/sources/` registram autores fornecidos pela fonte, DOI, URI da licença, consulta efetiva, localizador e hash da resposta XML quando obtida. São registros de investigação assistida, sem assinatura humana ou certificação. Os artigos completos ficam apenas no cache local ignorado pelo Git; nenhuma imagem dos artigos foi incorporada. A política HTTP recebeu somente o host explícito da API bibliográfica do EMBL-EBI; HTTPS/443, validação DNS/IP, limites e demais proteções continuam ativos. Não houve novo provedor de catálogo ou consulta durante a execução normal do jogo.

Não foram baixados capítulos do OpenStax para esta redação. O diagnóstico anterior sobre Biology 2e e os URLs legados permanecem no histórico e na base ativa até uma substituição aprovada. Referenciar o livro não foi tratado como prova de infração; mudar a bibliografia, isoladamente, não regulariza uma redação antiga.

Validação do lote em workspace temporário, pelo gerador real: 68 vínculos conferidos; **38 notas geradas seriam alteradas**, todas corretamente passam a `needs_review`, sem herdar revisão legada. A auditoria estrita da cópia com a proposta mantém **122 organismos e pool competitivo 80**, sem erros técnicos. Os 122 perfis, 122 conjuntos de receitas, ligações taxonômicas e avaliação pedagógica permanecem idênticos. A proposta não altera o organismo 82, fotografias, ícones, pontos ou dados dos jogadores.

A regressão reproduzível integra a suíte existente: `python tests/test_import_inaturalist.py`, **18 testes aprovados**, incluindo os casos que conferem cada ponteiro/hash/evidência e testam a projeção isolada dos dois lotes. Os **21 testes do transporte simulado** passaram, incluindo o host bibliográfico com recusa de DNS privado, HTTP e porta não permitida. Na rodada final, configure/compilação Release e **26/26 suítes CTest** passaram em **146,37 segundos**, já com as novas regressões. Apache/escopo, auditoria estrita do catálogo ativo e histórico frente à base do PR #18 estão consistentes; os três eventos anteriores permanecem byte a byte intactos. Os verificadores offline relacionados da CI e `qmllint` foram executados localmente; não houve execução remota presumida.

Triagem estrutural dos próximos lotes: `species_profiles.json` contém **34 contextos bibliográficos do OpenStax** (19 de nutrição, 14 de curiosidade e um de morfologia). `hint_education.json` não possui URLs diretos desse livro. Isso não estabelece autoria ou infração nem comprova inexistência de dependência indireta. Fontes de 122 fichas e 122 registros de dicas foram mapeadas para os próximos lotes, sem afirmar que todos os seus conteúdos já foram revistos cientificamente ou reescritos.

### Segundo lote preparado, sem promoção

Após a validação técnica isolada do procedimento de grupos, foram preparadas **34 notas novas para 28 fichas**, limitadas aos contextos diretos acima. Os objetos propostos estão em `educational_regularization.profile_openstax_batch.proposed_changes` no [relatório estruturado](SANEAMENTO_ISSUE_2.json), cada um com ponteiro, identidade, hashes anterior/novo, referência e localizador. Nenhum objeto foi aplicado a `species_profiles.json`. As datas de revisão propostas estão vazias e `verified` é falso; a redação nova não recebe a aprovação da nota anterior.

| Conteúdo proposto | Ocorrências | Fundamentação e limites |
| --- | --- | --- |
| Nutrição de plantas fotossintetizantes | 16 | Johnson (2016), DOI `10.1042/EBC20160016`, visão geral/fixação de carbono. Explicação do grupo; não atribui raízes a musgos ou hepáticas |
| Muda de insetos | 7 | Schumann e Triphan (2020), introdução/cutícula e ecdise. Não presume crescimento ou muda continuada dos adultos |
| Diversidade reprodutiva de plantas | 3 | Woudenberg et al. (2022), tabela 1 e grupos de plantas terrestres. A comparação com plantas sem flores não transforma *Euphorbia* em planta sem flores |
| Nutrição e curiosidade do líquen | 2 | Armaleo et al. (2019), DOI `10.1186/s12864-019-5629-x`, Background. Distingue associação, células e genomas; o estudo de *Cladonia grayi* não identifica os parceiros de *C. imperialis* |
| Micélio e estruturas reprodutivas de fungos | 3 | Challacombe et al. (2019), Background/Discussion. Escopo geral explicitado; sem alteração do organismo 82 |
| Nutrição de louva-a-deus | 1 | Bäumler et al. (2026), DOI `10.1186/s12983-026-00610-9`, Background. Usa o contexto de predação; não atribui a estratégia particular de *Haania orlovi* a *Ovalimantis* |
| Nutrição de cianobactérias | 1 | Johnson (2016), fotossíntese oxigênica. O grupo é o escopo; a fotografia não demonstra o metabolismo |
| Morfologia de *Gomphosphaeria aponina* | 1 | Marter et al. (2021), DOI `10.3390/genes12030389`, seção 3.1, cultura DSM 107014. Mantém a limitação da imagem de apoio e a necessidade de microscopia |

As quatro publicações adicionais têm **CC BY 4.0 verificada no XML do artigo específico**; suas evidências selecionadas integram o inventário já usado pelo primeiro lote. Ao todo são 19 publicações abertas com licença verificada (18 CC BY 4.0 e uma CC BY 3.0), mais a referência factual complementar de Hall. Não houve reprodução de figuras, ingestão de capítulos do OpenStax ou declaração de autoria humana exclusiva. Referências e licenças das fontes não constituem, por si, concessão de direitos sobre as propostas.

A nova regressão offline confere os 34 vínculos e aplica as propostas somente em diretório temporário. **35 notas geradas mudariam**, incluindo o alias morfológico de `110`; todas ficam `needs_review`. A avaliação pedagógica antiga é invalidada, e a regeneração automática explícita não aprova cientificamente as notas. As receitas dos outros 121 organismos permanecem idênticas; em `110`, os graus, categorias, níveis, vínculos fotográficos e justificativas são preservados. Apenas a redação, fonte e localizador da receita de aparência acompanham a nota de origem. O organismo 82 mantém suas notas e decisões. **18 testes da suíte de importação passaram**, incluindo os dois lotes.

### Balanço desta fase

Foram examinadas para substituição **102 ocorrências**: 68 de grupos e 34 de fichas, com propostas novas e evidência factual localizada. **Zero substituições foram promovidas ao catálogo ativo e zero novas concessões ou aprovações humanas foram registradas.** A documentação do processo dos rascunhos é mais completa, mas sua revisão editorial e a confirmação da elegibilidade concreta dos direitos continuam pendentes. O corpus ativo conserva suas 2.402 ocorrências/863 hashes inconclusivos quanto à origem; os quatro achados textuais da Issue #2 permanecem abertos. As demais passagens das fichas, as etimologias/justificativas das dicas e suas projeções ainda precisam dos lotes seguintes. Não se declara regularização integral.

O próximo passo é a conferência editorial do lote concreto de grupos pelo mantenedor. Sua aprovação, se ocorrer, será vinculada à versão apresentada e registrada no fluxo existente; o inventário `educational_text_rights` somente será atualizado para os hashes ativos quando houver promoção. A expansão às fichas e dicas usa o mesmo processo; nenhuma aprovação do lote de grupos se estende automaticamente aos demais recursos. A sincronização local continua em simulação, sem criação ou encerramento de Issues. A rotina real deve ser conferida depois de uma eventual integração na `main`.

## Conferência concentrada de 2026-10-18 após PR #17

Base `49e4e3dcfe8c6b18237a5f61e816bdc39e154d21`; PR #17 confirmado como integrado. Branch `fix/issue-2-final-compliance`. O registro estruturado existente recebeu `final_compliance`, preservando as etapas anteriores. A escolha de CC BY 4.0 e a política visual reservada não foram reabertas.

| Material | Estado anterior | Correção realizada | Evidência | Estado atual |
| --- | --- | --- | --- | --- |
| `group_education.json` | Origem/direitos pendentes | Conferidas as 19 definições e os 15 conjuntos de notas; as 68 redações não vazias são equivalentes às constantes históricas extraídas do gerador. Registrados todos os vínculos, sem atribuir autoria por Git | Snapshot documental referenciado em `final_compliance.prose_evidence`, histórico público e contrato por ocorrência | `inconclusive`; origem autoral e eventual independência da expressão ainda exigem confirmação. Nenhuma licença externa foi convertida |
| `hint_education.json` | Origem/direitos pendentes | 374 ocorrências vinculadas ao histórico e à bibliografia existente, sem alterar graus ou etimologias | Mesmo snapshot e inventário existente, com contexto individual preservado | Direitos pendentes; não houve busca exaustiva de similaridade ou prova de originalidade |
| `species_profiles.json` | Origem/direitos pendentes | 839 ocorrências dos 122 organismos vinculadas; três decisões editoriais do organismo 82 registradas após aprovação expressa | Histórico, fonte, confirmação real e `final_compliance.editorial_review_82.recorded_decisions` | Direitos pendentes; as duas notas e o agregado estão `approved_editorial`, sem revisão especializada |
| `progressive_hints.json` | Direitos das origens pendentes | Conferida equivalência exata com o gerador atual e preservadas ligações às fontes, sem licença independente inferida | Projeção reproduzível e snapshot documental | Pendências das redações de origem preservadas; avaliação automática regenerada somente após o novo ledger editorial |
| `glossata.png` | Permissão pendente | Termos oficiais consultados, contexto confirmado e autorização específica registrada para os bytes atuais | Declarações anteriores, consulta e [autorização limitada](../resources/compliance/sources/c17e8fcc2b053085aa17ea30db3b173cb56fc3917846f10f544d814ff517e29f.json) | `documented_official_use`; distribuição permitida exclusivamente nas versões oficiais |
| `glossata_discord.png` | Permissão pendente | Uso e variante identificados; ausência de componentes externos confirmada, sem inferir ferramenta ou sequência histórica | Mesma autorização vinculada ao hash específico e [conclusão individual](IDENTIDADE_VISUAL.md) | `documented_official_use`; sem licença gráfica geral |
| `glossata.ico` | Permissão pendente | Relação atual com o PNG preservada e uso oficial autorizado para este hash | Correspondência dos sete frames e autorização sobre a arte-base e os arquivos finais | `documented_official_use`; uso oficial limitado, sem exclusividade afirmada |

**Medição objetiva:** 2.402 ocorrências, agrupadas em 863 hashes de redação, receberam exame estrutural/documental e vínculo ao histórico; não se afirma leitura científica integral, comparação com toda a literatura ou revisão humana desses 863 textos. Quatro registros compartilhados conservam cada ponteiro/hash, sem milhares de declarações de autoria redundantes. A derivação local das 68 redações de grupos e a projeção das dicas são verificáveis, mas isso não comprova origem autoral. **Zero passagens receberam licença nova; as 2.402 ocorrências/863 hashes permanecem inconclusivos quanto à origem autoral. Dos sete achados de outros materiais, três foram resolvidos no escopo documentado de uso oficial dos ícones e quatro textuais permanecem.** Isso não encerra a Issue ou libera o pacote. Não se presumiu reprodução indevida onde não há prova, nem se reescreveram textos apenas para melhorar contagens.

O gate foi reforçado para recursos textuais que aleguem uma permissão: exige cobertura de todos os componentes atuais, evidência íntegra, condições/atribuição aplicáveis e ausência de pendências, além da autorização explícita do recurso. Preserva múltiplos titulares sem inventar um titular único. O inventário por passagem continua sem poderes de aprovação. Entre os 116 recursos inventariados, somente as permissões dos três ícones mudaram, após autorização específica; os demais estados foram preservados. A resposta adicional do mantenedor sobre os textos expressa crença em redação assistida por IA com Biology 2e como referência, mas não confirma ausência de cópia/tradução/adaptação; por isso a origem autoral continua inconclusiva e nenhuma passagem recebeu CC BY automaticamente.

O mantenedor confirmou **“Autorizar o registro limitado ao uso oficial”**, após conferir a proposta concreta de `distribution_allowed: true` dos três ícones. O registro separa essa autorização dos fatos de geração e dos termos consultados, vincula os três hashes e limita a permissão a `official_project_distribution`. O validador exige esses vínculos e recusa ampliação do escopo, ausência da confirmação ou troca de bytes. Os registros anteriores de origem e política permanecem intactos. A recusa inicial da revisão automática e a autorização adicional posterior estão preservadas no JSON; a proposta foi aplicada somente após essa resposta. Não há licença gráfica geral, certificação jurídica, exclusividade autoral, registro de marca ou autorização de terceiros presumida.

### Revisão editorial do organismo 82 registrada

O registro JSON mostra redações anteriores/atuais, DOI, localizador, limitações, recomendação e hashes atuais das duas notas e do agregado. A seção pertinente de Sánchez-García et al. (2020), DOI `10.1073/pnas.1922539117`, foi conferida pelo texto indexado da fonte primária: **Results and Discussion / Many gains, and few losses, of ECM symbiosis**, discussão de *Pseudoboletus*, referências 36–42. O acesso direto à PMC exige validação de navegador; não se afirma leitura integral do artigo ou suplementos.

As duas frases atuais mantêm a incerteza adequada. A curiosidade é uma inferência educativa prudente, não citação do estudo. A fonte indica CC BY-NC-ND 4.0; citar fatos não licencia tradução/adaptação de sua expressão protegida sob CC BY. Não foi copiado texto do artigo para o catálogo.

O mantenedor respondeu expressamente: **“Aprovo editorialmente as duas notas e o tópico agregado nessa versão”**. A confirmação foi vinculada aos três hashes e registrada pelo bridge existente em `review_history.jsonl`, com responsável Carlos Eduardo Lima de Oliveira, estado `approved_editorial`, tipo editorial e data efetiva **2026-10-18**. Usou-se a data civil da resposta, sem inventar seu instante exato. A repetição exata de cada decisão retornou `already_recorded` e preservou três eventos, sem duplicação. Os rascunhos anteriores permanecem no JSON como histórico; `recorded_decisions` aponta os eventos efetivos.

As duas notas voltam a ficar disponíveis; os outros campos, dicas e regras foram preservados. Não houve revisão por especialista ou concessão de direitos textuais. O hash do ledger invalida a avaliação automática das dicas, tornando necessária sua regeneração e a projeção educativa pelo fluxo existente. Comparações confirmaram os 122 conjuntos de receitas idênticos; foram atualizados somente os metadados automáticos, os três estados editoriais e os hashes derivados correspondentes. O hash anterior do recurso foi preservado no inventário.

### Curadoria e providências humanas

A fila local foi reexecutada e a sincronização simulada **sem `--apply`**: `apply: false`, `remote_checked: false`, nenhuma Issue criada/atualizada e nenhum erro. A simulação não verificou a situação remota da Issue após um futuro merge. A origem incerta dos textos foi mantida conforme a ressalva expressa do mantenedor. A aprovação editorial específica e a autorização limitada dos ícones já foram registradas. Não se escolhe outra licença nem se exige equipe externa. A Issue #2 deve continuar aberta; depois de eventual merge, a rotina de `main` deve ser conferida novamente, preservando os quatro achados textuais restantes. O mantenedor ainda precisa esclarecer origens textuais quando houver evidências ou autorizar redações independentes para casos concretos; bibliografia, geração automática e ausência de correspondências não bastam para conceder direitos.

| Medição local | Antes desta intervenção | Após as confirmações efetivas |
| --- | --- | --- |
| Achados de outros materiais, Issue #2 | 7 | 4 textuais; 3 permissões gráficas limitadas documentadas |
| Pendências documentais de fotos do quiz | 120 | 120, inalteradas |
| Registros científicos prioritários | 5 | 2 associações fotográficas; os 3 registros editoriais de `82` foram decididos |
| Fila de curadoria | 15 solicitações / 3.354 achados | 15 solicitações / 3.348 achados |
| Erros técnicos do gate do repositório | 0 | 0; distribuição continua bloqueada |

### Validação local desta intervenção

| Verificação | Resultado efetivamente observado |
| --- | --- |
| Configure e compilação Release com CMake/Ninja | Aprovados |
| CTest completo após atualização do inventário e dos recursos | **26/26 suítes passaram**, em 173,64 segundos |
| Regressões de investigação de direitos e decisões | 51 e 12 testes passaram, respectivamente; fixtures sintéticas e diretórios temporários |
| `audit_catalog.py --strict`, avaliação de dicas e projeção educativa | Aprovados; 122 organismos, pool competitivo 80; receitas dos 122 organismos idênticas à base |
| Apache/escopo, manifesto e estado atual | `licensing_inventory.py --check-code`, `release.py check`, `check-history` e `project_status.py check --build build` aprovados |
| Governança e recuperação | `recover-permissions` consistente; `audit-history --base 49e4e3dcfe8c6b18237a5f61e816bdc39e154d21` confirma prefixos anteriores preservados e exatamente três novos eventos editoriais |
| Dados, créditos, mídias e QML | Fontes científicas editáveis, política, proveniência, fotos, ícones, LICENSE/NOTICE e créditos preservados; geradores reproduzíveis; qmllint aprovado |
| Auditorias auxiliares da CI | Repositório, proposta de proteção, exploração, cobertura, assistência e propostas de manutenção offline executadas sem erro técnico; não houve ativação de configuração remota |
| `distribution.py check` | Retorno **2**, bloqueio legítimo: 120 fotos, 4 materiais textuais e 2 registros científicos prioritários; zero erros técnicos |

Não foi produzido ou auditado um novo pacote; obrigações de dependências distribuídas, incluindo a pendência histórica do ICU, não foram declaradas resolvidas. Estes resultados não são certificação jurídica ou científica. A validação remota do commit será consultada no Pull Request; este relatório registra os testes locais, sem presumir execução remota futura. Não houve merge, tag, release, pacote público ou encerramento da Issue.

## Registro histórico — política editorial após PR #16

O PR #16 foi integrado à `main` em `b7c7a493e9ee44b2790caa20b7ba1125865f763a`. Na intervenção seguinte, o mantenedor decidiu expressamente **CC BY 4.0 como padrão para redações educativas próprias elegíveis**, inclusive futuras redações enciclopédicas, com usos comerciais e adaptações mediante atribuição. A [política consolidada](propostas/LICENCIAMENTO_TEXTOS.md) distingue essa escolha de uma concessão válida sobre cada material e estabelece condições para fontes externas, proveniência, atribuição e futuras contribuições.

A decisão administrativa foi concluída; não se declarou que os quatro recursos educativos são integralmente próprios ou regularizados. `editorial_text_policy` registra o padrão separadamente; `own_material_licenses.texts` e `collection` permanecem `null`. O inventário de passagens é reaproveitado, com contrato documental opcional em `asset_rights.json`; a lista começa vazia, sem atribuir origens desconhecidas ou registrar aprovações. **Zero achados encerrados: quatro de textos e três de ícones continuam pendentes.** As 120 pendências fotográficas e os cinco registros científicos prioritários também permanecem.

A documentação da antiga proposta passa a refletir a escolha efetiva, sem alterar o registro histórico abaixo. Continuam necessárias a conferência de direitos/origem e delimitação de partes elegíveis, a conclusão fundamentada para distribuição dos ícones e a revisão editorial real dos dois trechos do organismo 82. Não houve alteração de ciência, mídia, interface, licença externa ou gate.

A política integra os hashes de entrada da avaliação automática de dicas; sua alteração exigiu regeneração pelo fluxo existente, preservando integralmente os 122 conjuntos de receitas e suas redações, fontes, graus e pontos. Somente o hash da política e os metadados de avaliação automática foram atualizados nos derivados; o hash anterior do recurso permanece em `inventory_history`, e a avaliação anterior no Git. Não houve nova revisão humana. A lista documental de direitos por passagem permanece vazia; o relatório de direitos existente passa a expor o padrão administrativo e a validação dos futuros registros, sem conceder permissões.

Validação local desta política: compilação aprovada e **26/26 suítes CTest passaram em 176,77 segundos**. A suíte de investigação de direitos contém 46 testes, incluindo sete novos casos de regressão. Apache/escopo, política editorial, auditoria estrita (122 organismos e pool competitivo 80), manifestos, preservação do histórico frente à base, recuperação de permissões, créditos reproduzíveis e qmllint passaram. O relatório mantém 2.402 ocorrências e 863 passagens únicas, zero registros de direitos criados e zero aprovações concedidas. `distribution.py check` continua retornando **2**, com zero erros técnicos, 120 pendências fotográficas, sete de outros materiais e cinco registros científicos prioritários. Não houve pacote, release, tag, merge ou encerramento da Issue #2; esses testes não certificam direitos nem revisão científica.

## Registro da etapa do PR #16 — saneamento concentrado após PR #15

Base: `f3dc5806868b9e9afe78a661dcdb516b9f551d33`, com PR #15 confirmado como integrado. Branch `fix/issue-2-rights-resolution`. O JSON existente recebeu a seção `integral_resolution`, sem substituir os registros anteriores. **Zero dos sete achados de direitos foi encerrado; sete permanecem pendentes.** Não foi demonstrada autorização suficiente para ampliar permissões. Foram corrigidos dois trechos científicos e removida uma lacuna de declaração de origem, sem apresentar essas correções como regularização jurídica.

| Material | Estado anterior | Correção | Evidência | Estado atual |
| --- | --- | --- | --- | --- |
| `glossata.png` | Permissão pendente; entradas de geração não declaradas | Mantenedor informou “Apenas descrição de texto”; resposta preservada sobre geração da arte, sem afirmar ausência de edições posteriores | `resources/compliance/branding/text-only-declaration.json`, declaração anterior e política reservada | Permissão pendente; não há licença gráfica geral ou autorização externa criada |
| `glossata_discord.png` | Permissão pendente | Declaração de geração vinculada ao recurso, sem inferir como a variante final foi preparada; bytes preservados | Registros de origem vinculados ao hash específico; sequência de edição ou nova geração desconhecida | Permissão pendente; uso oficial pretendido separado da liberação de distribuição |
| `glossata.ico` | Permissão pendente | Registro complementar e integridade preservados; conversão não exige titularidade exclusiva presumida | Hash específico e declaração do mantenedor | Permissão pendente; não se exige registro de marca ou prova impossível de exclusividade |
| `group_education.json` | Redação/direitos pendentes | Inventariadas as 19 definições e os 15 conjuntos complementares, inclusive textos não projetados; nenhuma reescrita sem problema demonstrado | 68 ocorrências textuais, vinculadas às referências de origem no inventário | Pendência de direitos preservada; licença de redação própria ainda não escolhida |
| `species_profiles.json` | Redação/direitos pendentes; 122 fichas | Nutrição e curiosidade de `82`, Pseudoboletus parasiticus, corrigidas como rascunhos com fonte localizada; demais textos preservados | 839 ocorrências; comparação dos dois trechos antes/depois no JSON e estudo primário indicado abaixo | Direitos pendentes; dois trechos e tópico agregado aguardam revisão editorial, sem responsável humano inventado |
| `hint_education.json` | Redação/direitos pendentes | Inventariadas etimologias, prefixos e justificativas sem modificar graus, textos ou identidades | 374 ocorrências; URLs e localizações de cada origem preservadas | Direitos pendentes; arquivo idêntico à base |
| `progressive_hints.json` | Redação/direitos pendentes | Derivação conferida pelo gerador; reavaliação automática necessária após mudar o hash da ficha, com avaliação anterior arquivada | 1.121 ocorrências; todos os objetos de receitas/espécies idênticos antes/depois | Herda pendências das redações de origem; nenhuma aprovação humana ou licença independente criada |

### Resultado da revisão textual e científica

O relatório de direitos existente agora inclui `educational_texts`: 2.402 ocorrências agrupadas por SHA-256 em 863 passagens únicas. Cada repetição conserva arquivo, ponteiro e fonte próprios; textos iguais com referências diferentes não têm as evidências confundidas. O inventário não infere originalidade da ausência de coincidências nem transforma URL, flags legadas ou assistência de IA em permissão. A comparação da projeção de dicas não grava avaliações ou renova datas.

Para evitar duplicação documental, o snapshot versionado guarda contagens, hashes de entrada e digest do inventário completo. Gere os detalhes offline com `python scripts/review_updates.py --rights-report`; os ponteiros por passagem ficam em `build/rights-investigation.json`, campo `educational_texts`. Não foi criado outro sistema ou uma série de relatórios de direitos.

A cobertura estrutural é completa para os campos de redação dos quatro arquivos; houve leitura editorial assistida por IA e conferência externa dirigida, **não validação científica integral de todas as afirmações dos 122 organismos nem comparação exaustiva com a literatura**. Fatos científicos são distinguidos da expressão textual e dos termos de suas fontes. Redação legada assistida por IA permanece de origem autoral individual inconclusiva; citações bibliográficas não são tratadas como transcrições. Nenhuma reprodução protegida incompatível foi concretamente demonstrada nas fontes conferidas; isso não prova originalidade integral ou exclui adaptações. Passagens geradas continuam ligadas às redações anteriores. As condições atuais de OpenStax, LPSN e demais fontes não são automaticamente transferidas para todo o texto do Glossata.

**Correção concreta:** os textos de `82/nutrition` e `82/curiosity` apresentavam parasitismo como fato estabelecido. Sánchez-García e colaboradores, [PNAS (2020), DOI 10.1073/pnas.1922539117](https://pubmed.ncbi.nlm.nih.gov/33257574/), discutem incerteza sobre a relação em **Results and Discussion / Many gains, and few losses, of ECM symbiosis**. O trecho foi consultado pela indexação da [fonte primária na PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC7768725/); a abertura direta apresentou proteção de navegador e o PDF da editora não abriu nesta consulta. Não se alega leitura integral. O [repositório institucional dos autores](https://commons.clarku.edu/faculty_biology/170/) confirma a referência e informa CC BY-NC-ND 4.0; nenhum artigo, imagem, tradução ou parágrafo foi incorporado. As novas frases comunicam a incerteza factual em redação própria assistida por IA, **ainda sem aprovação editorial humana ou licença de texto ativada**.

Os dois trechos deixam de herdar a aprovação legada: ficam indisponíveis individualmente até revisão; a curiosidade geral pode usar o fallback já existente. A ficha, as outras informações e o quiz continuam disponíveis. Como somente a entrada `species_profiles.json` mudou, foi necessária uma nova avaliação **pedagógica automática**, não científica: receitas, graus, opções possíveis, política e regras não mudaram. O snapshot anterior permanece no JSON e no Git. Nenhum teste de hash histórico foi reescrito para ocultar a transição.

As conferências dirigidas também sustentam fatos de morfologia/alimentação de Papilio machaon em **Physical Description / Food Habits** da ADW; morfologia de Nereocystis/Egregia nas respectivas entradas SeaNet; etimologia de Arthrospira jenneri em **Etymology**, registro LPSN 40754; e traços de grupos nas seções institucionais já citadas. Localizadores e limitações estão no bloco `source_checks` do JSON. Não se transferiu categoria regional de conservação para avaliação global; as seis redações distintas de conservação e seus vínculos foram preservados.

### Decisões finais necessárias, sem exigências artificiais

1. **Textos próprios:** avaliar a [proposta objetiva CC BY 4.0](propostas/LICENCIAMENTO_TEXTOS.md), delimitar as partes efetivamente próprias e eventuais exclusões. A licença não foi ativada: `texts` e `collection` continuam `null`. Origem inconclusiva ou adaptação externa deve ser esclarecida no escopo correspondente, sem atribuir todo o corpus ao mantenedor ou exigir uma equipe externa inexistente.
2. **Ícones:** a criação via ChatGPT, direção criativa, resposta “Apenas descrição de texto” sobre geração e intenção de uso oficial já estão declaradas; não é necessário repetir essas respostas. Não foram informadas todas as etapas de preparação dos arquivos finais: não se presume ausência de imagens nas edições e conversões posteriores. Falta documentar uma conclusão fundamentada sobre os direitos efetivamente disponíveis para a expressão gráfica resultante e o escopo de distribuição oficial. Havendo questão concreta de direito externo, esclarecê-la; não se exige comprovar a inexistência de qualquer direito no mundo, registro de marca, exclusividade autoral de IA ou publicação de prompts/conversas privadas. A política reservada não concede direitos externos.
3. **Duas correções científicas:** realizar revisão editorial real dos trechos e do tópico `topic:82:education` pelo mecanismo existente, com evidência localizada. Não registrar revisão por especialista. A CLI de confirmação de fotos não se aplica a textos ou branding.

### Validação e bloqueios atuais

Compilação Release aprovada; **26/26 suítes CTest passaram em 136,56 segundos na conferência final**, incluindo gerador educativo, dicas progressivas, investigação de direitos (39 testes), governança, histórico, perfis, backups e recuperação de partidas. Auditoria estrita: 122 organismos e pool competitivo 80 preservados. Apache-2.0, manifestos/status, recuperação de permissões em modo de conferência e comparação dos ledgers com a base passaram. A reprodução dos créditos permaneceu idêntica; `qmllint` e auditoria do índice sem erros. Testes novos verificam deduplicação sem perder fontes, cobertura de conteúdo não projetado, detecção de derivado adulterado e ausência de renovação/aprovação durante a investigação. O teste C++ confere que os dois rascunhos não herdam a aprovação antiga; nenhuma referência histórica foi sobrescrita.

O gate retorna **2**, com zero erros técnicos: permanecem os sete materiais, 120 registros fotográficos com documentação pendente e os dois bloqueios fotográficos anteriores. Há também três registros `needs_review` do organismo 82 decorrentes da correção: duas notas e o tópico agregado. São três registros de uma mesma revisão, não três novos organismos ou novas licenças. Nenhum campo de permissão, licença ativa ou ledger foi alterado para reduzir contagens. Fechar a Issue ou escolher uma licença não aprova esses recursos automaticamente. Não houve pacote, release, tag, merge automático ou encerramento da Issue.

### Conferência do PR #16 antes do merge

A resposta humana “Apenas descrição de texto” foi preservada literalmente. O registro complementar agora separa a declaração sobre geração da arte das etapas posteriores: o README identifica a variante Discord, mas não documenta sua sequência de criação; os sete frames atuais do ICO correspondem ao PNG principal redimensionado por Lanczos em conferência somente leitura, sem provar a ferramenta ou ordem histórica. Não se afirma que cada arquivo final tenha sido gerado independentemente por texto ou sem processamento de uma imagem anterior. Os três hashes gráficos permanecem idênticos.

Conferência editorial do organismo 82: somente `species:82:nutrition` e `species:82:curiosity` diferem nas notas em relação à base do PR. Ambas conservam DOI 10.1073/pnas.1922539117, URL da PMC e localizador de seção; estão em `needs_review`, com `usable: false`, sem revisor ou data de aprovação. O tópico agregado `topic:82:education` também exige revisão, mas não é usado para ocultar os demais campos: resumo, distribuição, morfologia, conservação, nomes e dicas válidas permanecem disponíveis. A curiosidade geral usa o fallback existente quando necessário. Não houve alteração de conteúdo nesta conferência.

A revisão editorial humana pendente deve conferir a sustentação e clareza das duas novas frases frente à seção indicada, registrar a limitação de acesso à fonte e decidir cada nota e o tópico agregado pelo fluxo existente, vinculando a decisão aos respectivos hashes atuais. A conferência técnica não constitui essa aprovação.

Após o esclarecimento documental, a recompilação passou, assim como as cinco suítes locais pertinentes: `GlossataTests`, `GlossataGovernanceTests`, `GlossataReleaseTests`, `GlossataProjectStatusTests` e `GlossataRightsInvestigationTests` (31,45 segundos no total). As verificações Apache, manifesto e preservação do histórico também passaram.

No head anterior `79586a51f0e020541f98c0a8d39405b0ddda2bd8`, as duas execuções remotas de `offline-validation` terminaram com sucesso. O diagnóstico remoto de `distribution-permissions` confirmou zero erros técnicos, 120 pendências fotográficas, sete de outros materiais e cinco registros científicos prioritários (dois de fotos e três da revisão do organismo 82); o bloqueio foi mantido. A [execução do PR](https://github.com/kaduolliver/Projeto-Glossata/actions/runs/39081486112) preserva a evidência dessa validação anterior; novos checks acompanham o commit de esclarecimento.

## Registro histórico das etapas anteriores

Auditoria de **2026-10-17**, sobre `e2df447cbae2b332de51391e3f34b190b0d74e91` (PR #11 integrado). [Issue #2](https://github.com/kaduolliver/Projeto-Glossata/issues/2), solicitação estável `curation-v1-4f2ce3678bf9c200687be506`. O [registro estruturado](SANEAMENTO_ISSUE_2.json) contém hashes, histórico Git por material, contagens e comparação do gerador. Este relatório é uma investigação documental assistida por IA; não constitui revisão independente, certificação científica ou parecer jurídico.

## Correções finais do PR #12 — 2026-10-17

A correção bibliográfica foi preparada sobre `3da8ad72825eb6027ee3c38938cce6100fbe6157`, preservando os dois commits anteriores. A [página oficial da editora](https://link.springer.com/article/10.1186/s12915-020-0754-1), seções **About this article / Cite this article** e cabeçalho, confirma **BMC Biology**, volume 18, artigo 23, publicação em **2 de março de 2020**, DOI **10.1186/s12915-020-0754-1**. O título é **Evolution of metabolic capabilities and molecular features of diplonemids, kinetoplastids, and euglenids**. O [PubMed informado](https://pubmed.ncbi.nlm.nih.gov/32122335/) exibiu proteção de navegador nesta consulta; a confirmação foi feita na editora, não atribuída a uma leitura do PubMed que não ocorreu.

O rótulo incorreto foi corrigido em `group_education.json` e na bibliografia de `REFINAMENTO_IDENTIFICACAO.md`. O URL PMC foi preservado. A pesquisa de ocorrências não encontrou outro registro derivado ativo dessa referência. Euglenida não tem nó ativo correspondente no catálogo atual: regenerar `education.json` altera somente o hash de `group_education.json` em `generated_source_sha256`. Todos os demais campos da projeção continuam idênticos aos do head anterior, incluindo descrições, dicas, revisões e avaliação pedagógica. A descrição científica de Euglenida não foi alterada.

O hash anterior do recurso foi preservado em `asset_rights.json / inventory_history`; o novo hash foi calculado dos bytes reais, sem mudar estado ou campos de permissão. O manifesto foi regenerado pelo comando de catálogo existente, mantendo 2.4.0 como **não lançado**. O teste de extração conserva seu digest histórico: confere explicitamente o novo rótulo e restaura somente o rótulo antigo em memória para comparar os demais valores com a extração original. Não se substituiu um hash de referência para aceitar mudanças arbitrárias.

### OpenStax: edição referenciada e limites da comprovação

Os URLs do corpus identificam **Biology 2e**, segunda edição, e suas seções. A [página oficial de uma seção](https://openstax.org/books/biology-2e/pages/1-introduction), em **Citation information**, identifica a publicação de 28 de março de 2018; o [prefácio atual](https://openstax.org/books/biology-2e/pages/preface), **About OpenStax resources / Customization**, declara **CC BY-NC-SA 4.0**. Essa é a licença confirmada para o conteúdo web atual consultado em 2026-10-17, não uma afirmação de que toda versão anterior da mesma edição teve sempre essa licença.

Há uma lacuna concreta de proveniência: os registros locais apontam para URLs mutáveis, sem snapshot/hash ou identificador da revisão OpenStax efetivamente consultada ao redigir cada nota. Assim, não foi possível confirmar retroativamente qual revisão/licença acompanhou aquela consulta. O ano de publicação da edição não identifica a revisão utilizada. A escolha legítima de uma licença anterior deve ser sustentada por evidência da versão obtida; a consulta atual não presume revogação de uma concessão CC anterior válida.

Uma **citação bibliográfica** aponta a fonte e não demonstra reprodução da expressão textual. **Fatos científicos** devem ser distinguidos da forma de redação. **Reprodução textual** e **adaptação de expressão protegida**, quando identificadas, exigem análise das condições aplicáveis à versão correspondente. Referenciar OpenStax não torna automaticamente o texto do Glossata uma adaptação, uma infração ou uma redação comprovadamente original. Não foi demonstrada reprodução incompatível concreta nesta conferência, nem originalidade integral; a revisão de redação e direitos permanece pendente. Não foi aplicada licença CC aos textos do Glossata nem alterada a licença Apache do código. Nenhuma licença foi inferida somente da gratuidade ou do propósito educacional.

As verificações da auditoria inicial abaixo permanecem históricas. Os resultados desta correção são registrados no bloco `final_pr12_correction` do JSON e no PR existente; nenhuma revisão humana, científica especializada ou jurídica foi criada.

**Validação desta correção:** compilação Release concluída; **26/26 suítes CTest aprovadas em 183,14 segundos** e **16 testes do gerador/importação aprovados**. Auditoria estrita, verificação Apache-2.0, manifesto/status, recuperação de permissões em modo de conferência, comparação dos ledgers com o head anterior e auditoria de governança passaram sem erros técnicos. Avaliação pedagógica existente continua válida, sem renovar sua data. Exploração e filogenia também passaram. `distribution.py check` retornou **2**, bloqueio esperado: **120** pendências fotográficas, **sete** de outros materiais e **dois** bloqueios científicos. Nenhum gate foi alterado ou contornado. O resultado remoto do novo commit é acompanhado no PR #12.

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
| Euglenida: referência da introdução extraída | [Registro PMC7052976](https://pmc.ncbi.nlm.nih.gov/articles/PMC7052976/), consulta indexada inicial em 2026-10-17, complementada pela [editora](https://link.springer.com/article/10.1186/s12915-020-0754-1) nas correções finais | A abertura inicial da PMC exibiu proteção de navegador. A editora confirmou BMC Biology (2020), DOI 10.1186/s12915-020-0754-1; o rótulo bibliográfico foi corrigido nas duas ocorrências locais. A conferência bibliográfica não constitui revisão da descrição científica |

Não foram copiados parágrafos das fontes. Não foi realizada comparação exaustiva com toda a literatura, detecção confiável de plágio ou revisão científica especializada. Não se registra um direito como resolvido por tradução ou por ausência de correspondência literal. Termos históricos Monera/Protista e receitas existentes foram preservados integralmente nesta refatoração; a navegação filogenética não foi modificada.

## Separação técnica e rastreabilidade

`group_education.json` é a fonte editável única dos antigos `DEFINITIONS` e `PLAIN_NOTES`. Arrays conservam a ordem, inclusive definições que não geram um nó na árvore atual: são 19 definições de origem e 14 introduções ativas. O carregador valida esquema, tipos, duplicatas, rank, referência HTTPS e pares de notas. Um workspace sem recurso é recusado; não recebe silenciosamente redação do catálogo ativo. Os testes de admissão utilizam um recurso sintético vazio.

O SHA-256 canônico histórico dos valores na extração inicial é `6a41c03329d295cf1f7966fe110cf3cef0844bbf71cab41f7ba8dffc2e3a5e5e`; a correção posterior do rótulo bibliográfico é explicitamente isolada no teste, sem modificar esse registro histórico. Na extração, a geração antes/depois foi idêntica em todos os campos salvo `generated_source_sha256`, que ganhou o hash da nova fonte. O hash canônico de todos os demais campos continua `388472e49b4bfc3717a678671e01f18017c91139c1d17430814562e30a416929`, inclusive após a correção bibliográfica. Não houve mudança de revisão, conteúdo científico, receitas, assistência, pesos ou data de avaliação. O recurso-fonte não precisa de download nem ser consultado pelo executável: o programa continua lendo a projeção embarcada `education.json`.

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
