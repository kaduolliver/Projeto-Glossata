# Lote 2 — comparação das propostas para fichas

**Estado: versão integralmente confirmada e incorporada em 2026-10-18.**

Conferência única do lote, preparada a partir da `main` que integrou o PR #19. Esta apresentação deriva de `educational_regularization.profile_openstax_batch` em [SANEAMENTO_ISSUE_2.json](../SANEAMENTO_ISSUE_2.json). Foram conferidas as fontes arquivadas, ajustadas cinco redações compartilhadas e acrescentadas duas referências específicas para sanar a lacuna sobre micélio. A preparação anterior não criou aprovações; posteriormente o mantenedor confirmou separadamente a versão editorial e os direitos disponíveis, conforme a evidência indicada abaixo.

São **34 ocorrências em 28 organismos**, agrupadas abaixo em nove redações compartilhadas, sem perder os vínculos individuais. A confirmação das 68 redações de grupos não alcança este lote. Os campos legados permanecem `verified: false` e `reviewed_on: null`; a revisão real é `approved_editorial` no ledger existente. A confirmação de CC BY 4.0 abrange somente as 34 redações identificadas, sem licença global.

**Fingerprint das 34 propostas** (SHA-256 do JSON canônico de `proposed_changes`): `e0d6ac843d4da1111698d4c1f478840e231cedb7a6f67949318bcc1ad2e55478`.

**Fonte anterior à promoção, preservada no Git:** `resources/data/species_profiles.json`, SHA-256 `df137441551f36ce54e326c944bd8292221b9e537481a5c84e084602272a8bec`.

**Versão anterior preservada no Git:** fingerprint `a6139ee7e5e468cec7506a282c90ceb5eaa9d895b5d7c5acf1595bd770be5460`; documento no merge do PR #19, `09e8eb759172e32b9554c4c86d8a583a667e4dcc`. A revisão técnica assistida por IA não é revisão por especialista nem prova de autoria humana exclusiva.

## Índice das ocorrências

| ID | Organismo | Campo | Comparação | Ponteiro na ficha |
| --- | --- | --- | --- | --- |
| 133 | *Ginkgo biloba* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/133/nutrition` |
| 53 | *Viburnum opulus* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/53/nutrition` |
| 61 | *Verbascum thapsus* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/61/nutrition` |
| 44 | *Dictyanthus pavonii* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/44/nutrition` |
| 47 | *Polypodium glycyrrhiza* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/47/nutrition` |
| 43 | *Kindbergia oregana* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/43/nutrition` |
| 48 | *Marah macrocarpa* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/48/nutrition` |
| 49 | *Mastigostyla macbridei* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/49/nutrition` |
| 51 | *Mimetes chrysanthus* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/51/nutrition` |
| 54 | *Xanthosoma robustum* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/54/nutrition` |
| 57 | *Pleurophyllum speciosum* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/57/nutrition` |
| 50 | *Tulipa regelii* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/50/nutrition` |
| 55 | *Pleurothallis maitamae* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/55/nutrition` |
| 23 | *Titanomis sisyrota* | Curiosidade | [Proposta 2](#proposta-2) | `/23/curiosity` |
| 26 | *Geminaria canalis* | Curiosidade | [Proposta 2](#proposta-2) | `/26/curiosity` |
| 30 | *Xenocnema spinipes* | Curiosidade | [Proposta 2](#proposta-2) | `/30/curiosity` |
| 31 | *Sphaerocoris annulus* | Curiosidade | [Proposta 2](#proposta-2) | `/31/curiosity` |
| 34 | *Dicarnosis erythrocephala* | Curiosidade | [Proposta 2](#proposta-2) | `/34/curiosity` |
| 35 | *Pachypodistes angulata* | Curiosidade | [Proposta 2](#proposta-2) | `/35/curiosity` |
| 41 | *Ovalimantis maculata* | Curiosidade | [Proposta 2](#proposta-2) | `/41/curiosity` |
| 41 | *Ovalimantis maculata* | Alimentação/nutrição | [Proposta 3](#proposta-3) | `/41/nutrition` |
| 52 | *Asterella drummondii* | Curiosidade | [Proposta 4](#proposta-4) | `/52/curiosity` |
| 52 | *Asterella drummondii* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/52/nutrition` |
| 60 | *Huperzia lucidula* | Curiosidade | [Proposta 4](#proposta-4) | `/60/curiosity` |
| 60 | *Huperzia lucidula* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/60/nutrition` |
| 62 | *Euphorbia duckei* | Curiosidade | [Proposta 4](#proposta-4) | `/62/curiosity` |
| 62 | *Euphorbia duckei* | Alimentação/nutrição | [Proposta 1](#proposta-1) | `/62/nutrition` |
| 65 | *Cladonia imperialis* | Curiosidade | [Proposta 5](#proposta-5) | `/65/curiosity` |
| 65 | *Cladonia imperialis* | Alimentação/nutrição | [Proposta 6](#proposta-6) | `/65/nutrition` |
| 70 | *Ganoderma brownii* | Curiosidade | [Proposta 7](#proposta-7) | `/70/curiosity` |
| 71 | *Marasmius haematocephalus* | Curiosidade | [Proposta 7](#proposta-7) | `/71/curiosity` |
| 81 | *Entoloma trachyosporum* | Curiosidade | [Proposta 7](#proposta-7) | `/81/curiosity` |
| 110 | *Gomphosphaeria aponina* | Alimentação/nutrição | [Proposta 8](#proposta-8) | `/110/nutrition` |
| 110 | *Gomphosphaeria aponina* | Morfologia | [Proposta 9](#proposta-9) | `/110/morphology` |

## Comparação das redações

### Proposta 1

**Nutrição de plantas fotossintetizantes**

**Organismos:** *Ginkgo biloba* (`133`), *Viburnum opulus* (`53`), *Verbascum thapsus* (`61`), *Dictyanthus pavonii* (`44`), *Polypodium glycyrrhiza* (`47`), *Kindbergia oregana* (`43`), *Marah macrocarpa* (`48`), *Mastigostyla macbridei* (`49`), *Mimetes chrysanthus* (`51`), *Xanthosoma robustum* (`54`), *Pleurophyllum speciosum* (`57`), *Tulipa regelii* (`50`), *Pleurothallis maitamae* (`55`), *Asterella drummondii* (`52`), *Huperzia lucidula* (`60`), *Euphorbia duckei* (`62`).

**Campo:** Alimentação/nutrição. **Escopo proposto:** Plantas fotossintetizantes.

**Redação anterior à promoção:** Produz açúcares por fotossíntese e absorve água e sais minerais do ambiente.

**Redação aprovada e incorporada:** A energia da luz sustenta a produção de compostos orgânicos a partir do gás carbônico. Água e nutrientes minerais também são necessários; não são substituídos pela luz.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/25-1-early-plant-life). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Johnson MP. (2016) — Photosynthesis.](https://doi.org/10.1042/ebc20160016).

**Localizador:** Introduction / photosynthetic energy conversion; Overview of photosynthesis; carbon fixation.

**Alteração nesta revisão:** A síntese distingue energia luminosa, fonte de carbono, água e nutrientes; não atribui raízes a musgos ou hepáticas.

**Conferência editorial necessária:** Escopo de plantas fotossintetizantes; não afirma presença de raízes em musgos/hepáticas nem descreve uma dieta exclusiva da espécie.

**Fundamento conferido:** Johnson, Introduction: conversão de água e CO2 em compostos orgânicos; The light reactions / The major pigments: complexos proteicos, magnésio da clorofila e cofatores minerais. Explicação bioquímica de grupo, sem inferir absorção exclusiva pela raiz.

**Limitações preservadas nos metadados:** Restrita às 16 plantas fotossintetizantes relacionadas; não se estende à planta parasita Hydnora. Não descreve todas as fontes de carbono possíveis de cada espécie nem uma observação da fotografia.

**Evidência já arquivada:**

- [CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/5e1e3142021152579254d2eab48550b6d09ba4d78c48afef23fd8f3fa96bb495.json), SHA-256 `7516098a9094cfc796bab73c878ba7fa942f52dde69ad069e67a867b11efb9e0`. Consulta efetiva: `2026-10-18T04:43:37.559645+00:00`.

**Vínculos da proposta:** objeto `36544f4934820f82b10f3472cd5aaabbd4659a75aba7477737fa7b2abd7c14db`; texto `3f53a430c94b4260d63bec62902402a444e93458b002a76ead120760b631637b`. Não são assinatura nem aprovação.

### Proposta 2

**Muda de insetos**

**Organismos:** *Titanomis sisyrota* (`23`), *Geminaria canalis* (`26`), *Xenocnema spinipes* (`30`), *Sphaerocoris annulus* (`31`), *Dicarnosis erythrocephala* (`34`), *Pachypodistes angulata* (`35`), *Ovalimantis maculata* (`41`).

**Campo:** Curiosidade. **Escopo proposto:** Insetos.

**Redação anterior à promoção:** O corpo tem um revestimento externo que precisa ser trocado durante o crescimento.

**Redação aprovada e incorporada:** Na fase de crescimento, o inseto renova periodicamente a cutícula, o revestimento externo do corpo. Essa troca é chamada muda ou ecdise.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/28-6-superphylum-ecdysozoa-arthropods). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Schumann I et al. (2020) — The PEDtracker: An Automatic Staging Approach for <i>Drosophila melanogaster</i> Larvae.](https://doi.org/10.3389/fnbeh.2020.612313).

**Localizador:** Introduction / larval cuticle and ecdysis; general insect scope, not a species-specific experiment.

**Alteração nesta revisão:** A fase de crescimento delimita corretamente muda/ecdise e evita atribuir muda contínua a todos os adultos.

**Conferência editorial necessária:** Contexto geral de cutícula e ecdise durante o crescimento. Não atribui muda continuada a todos os insetos adultos.

**Fundamento conferido:** Schumann e Triphan, Introduction, primeiro parágrafo: cutícula e renovação ao longo do desenvolvimento pós-embrionário; resultados experimentais de Drosophila não transferidos.

**Limitações preservadas nos metadados:** Não transfere cronologia, hormônios ou medidas experimentais de Drosophila às sete espécies.

**Evidência já arquivada:**

- [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/46d5c405344ea48434f5e0d58763f5522cb3353a587a7881816a11bd0fba1de1.json), SHA-256 `525baa38b13b08fddd65a1cebd54419fc241ef9b88e5cfd92787353cf90907ad`. Consulta efetiva: `2026-10-18T04:20:58.557402+00:00`.

**Vínculos da proposta:** objeto `28f4ca0baf93462c0b484b43aa0e9f7cd51c9a9c824f205185fe8e4d52580ba2`; texto `d300db13d7eaf269b4f7378c1700958531cdcddbb780fb78d0a5ae87d646d546`. Não são assinatura nem aprovação.

### Proposta 3

**Alimentação de louva-a-deus**

**Organismos:** *Ovalimantis maculata* (`41`).

**Campo:** Alimentação/nutrição. **Escopo proposto:** Louva-a-deus.

**Redação anterior à promoção:** Captura insetos e outros pequenos animais.

**Redação aprovada e incorporada:** Louva-a-deus são predadores de outros artrópodes. As pernas dianteiras participam da captura e da retenção da presa.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/28-6-superphylum-ecdysozoa-arthropods). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Bäumler F et al. (2026) — A striking difference: biomechanics of the impaling hunting strategy of a moss mantis.](https://doi.org/10.1186/s12983-026-00610-9).

**Localizador:** Background / mantodean predators and prehensile forelegs; species-specific impaling strategy excluded.

**Alteração nesta revisão:** Retirada do texto do jogador a ressalva sobre a espécie experimental; preservada aqui e na conferência.

**Conferência editorial necessária:** A fonte estuda Haania orlovi. Usar somente o contexto geral de predação e pernas raptoriais; não transferir sua estratégia de empalamento ou inferir dieta exclusiva de Ovalimantis.

**Fundamento conferido:** Bäumler et al., Background, segundo parágrafo: Mantodea como predadores de artrópodes e uso de pernas dianteiras raptoriais.

**Limitações preservadas nos metadados:** O experimento trata de Haania orlovi, não Ovalimantis maculata. Não atribui empalamento, velocidades, mecanismo elástico ou dieta exclusiva de Haania a Ovalimantis.

**Evidência já arquivada:**

- [CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/21ffa70424dd36b0d1803b1870cce6aada1e8260ee41ed0f2da118086f7dfe7b.json), SHA-256 `8d38a801390ae92664c812933d0a525ff0999c186eb3609eb28ee352fbf43cf4`. Consulta efetiva: `2026-10-18T04:39:14.726809+00:00`.

**Vínculos da proposta:** objeto `e1d42bcda824721707f8e8efa48f2c5b14d5ac6d67517da59d55aede0fdd7c85`; texto `805d664ecbbb232a847d7c5091cba9bf9ebee156439cd38864311b2873bc9c07`. Não são assinatura nem aprovação.

### Proposta 4

**Diversidade reprodutiva das plantas**

**Organismos:** *Asterella drummondii* (`52`), *Huperzia lucidula* (`60`), *Euphorbia duckei* (`62`).

**Campo:** Curiosidade. **Escopo proposto:** Plantas.

**Redação anterior à promoção:** Nem todas as plantas produzem flores; vários grupos se reproduzem por esporos.

**Redação aprovada e incorporada:** A reprodução das plantas não se resume às flores: musgos, hepáticas e samambaias formam esporos.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/25-1-early-plant-life). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Woudenberg S et al. (2022) — Deep origin and gradual evolution of transporting tissues: Perspectives from across the land plants.](https://doi.org/10.1093/plphys/kiac304).

**Localizador:** Introduction; Table 1; Figure 2 / mosses, liverworts, ferns and seed plants; images excluded.

**Alteração nesta revisão:** Mantida uma comparação explícita entre plantas; retirada a explicação administrativa sobre exclusividade.

**Conferência editorial necessária:** Comparação entre linhagens, não diagnóstico da espécie. Euphorbia não é apresentada como planta sem flores.

**Fundamento conferido:** Woudenberg et al., Table 1: bryophytes/free-sporing, liverwort sporangia, moss sporangia, seed-free ferns e angiosperms/flowering plants.

**Limitações preservadas nos metadados:** É uma comparação de grupos, não diagnóstico das três espécies. Euphorbia duckei não é classificada como planta sem flores. Não afirma que apenas plantas sem flores produzam esporos.

**Evidência já arquivada:**

- [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/4f391b456952374c2b8724a6c1a168941dcc6a8b88b5d9b05f754145f2a4864a.json), SHA-256 `af13bf0be3f2031430cc7bfdc518a910a2fba0aedc3a1ce1f61d5c6b33735f48`. Consulta efetiva: `2026-10-18T04:19:24.761213+00:00`.

**Vínculos da proposta:** objeto `510cc9414f1564807002fb6fd3230f3d19651cf390eeb0a50099524b782b376a`; texto `8add7bd37ed52d83c0b730675bab851c2888dd42b2ccbf07c4b01bffe4b1ac97`. Não são assinatura nem aprovação.

### Proposta 5

**Parceiros do líquen**

**Organismos:** *Cladonia imperialis* (`65`).

**Campo:** Curiosidade. **Escopo proposto:** Líquens; comparação com estudo de outra espécie.

**Redação anterior à promoção:** A parte visível de muitos fungos é apenas uma parte do organismo, que também cresce no substrato.

**Redação aprovada e incorporada:** Os parceiros de um líquen mantêm suas células e genomas distintos, mesmo formando uma estrutura integrada.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/24-1-characteristics-of-fungi). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Armaleo D et al. (2019) — The lichen symbiosis re-viewed through the genomes of Cladonia grayi and its algal partner Asterochloris glomerata.](https://doi.org/10.1186/s12864-019-5629-x).

**Localizador:** Background / mycobiont and photobiont; study on Cladonia grayi, not Cladonia imperialis.

**Alteração nesta revisão:** Preservada a independência celular/genética como fato geral; a identificação da espécie experimental fica nos metadados.

**Conferência editorial necessária:** A fonte trata de Cladonia grayi. Não identifica o fotobionte de Cladonia imperialis nem transforma a associação em uma única célula ou genoma.

**Fundamento conferido:** Armaleo et al., Background, segundo parágrafo: parceiros mantêm independência genética/celular, sem fusão de genomas.

**Limitações preservadas nos metadados:** O artigo estuda Cladonia grayi e Asterochloris glomerata, não identifica parceiros de Cladonia imperialis. Não reivindica identidade de fotobionte, fusão de genomas ou direitos sobre a fotografia do artigo.

**Evidência já arquivada:**

- [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/fc47932ea1637452ba9a3a8e780310e2d9922f66a6d77bbe06035195a8f07342.json), SHA-256 `17111371f43431bd39f9694d1a63a78ebdf343af31c0860e119f19afe58330e1`. Consulta efetiva: `2026-10-18T04:39:11.747062+00:00`.

**Vínculos da proposta:** objeto `f901c7e5f741f76c919c72c15ad935f38922339dfc1873242ade8ccaba3acc20`; texto `e6b9875ae2d6d936718ab69196abfc1dc268b3d3745ffb304a3c13898396c278`. Não são assinatura nem aprovação.

### Proposta 6

**Nutrição do líquen**

**Organismos:** *Cladonia imperialis* (`65`).

**Campo:** Alimentação/nutrição. **Escopo proposto:** Líquens.

**Redação anterior à promoção:** O parceiro fotossintetizante produz açúcares; o fungo participa da estrutura da associação.

**Redação aprovada e incorporada:** No líquen, o parceiro fotossintetizante fornece compostos de carbono à associação. O fungo participa da organização do corpo visível, chamado talo; são organismos associados, não um único tecido vegetal.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/24-3-ecology-of-fungi). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Armaleo D et al. (2019) — The lichen symbiosis re-viewed through the genomes of Cladonia grayi and its algal partner Asterochloris glomerata.](https://doi.org/10.1186/s12864-019-5629-x).

**Localizador:** Background / lichen symbionts; primary symbiotic interfaces and carbon transfer; general lichen scope.

**Alteração nesta revisão:** Explicação de associação e transferência de carbono adequada ao escopo; sem atribuir um fotobionte nominal.

**Conferência editorial necessária:** Contexto geral da associação liquênica; não atribui automaticamente um fotobionte específico ao organismo do catálogo.

**Fundamento conferido:** Armaleo et al., Background; Inferences from differential transcription about nutritional fluxes at the symbiotic interface; Figure 1 caption apenas como localizador factual de talo/tecido fúngico, imagem não incorporada.

**Limitações preservadas nos metadados:** Não transfere parceiros específicos ou previsões genômicas de Cladonia grayi para Cladonia imperialis.

**Evidência já arquivada:**

- [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/fc47932ea1637452ba9a3a8e780310e2d9922f66a6d77bbe06035195a8f07342.json), SHA-256 `17111371f43431bd39f9694d1a63a78ebdf343af31c0860e119f19afe58330e1`. Consulta efetiva: `2026-10-18T04:39:11.747062+00:00`.

**Vínculos da proposta:** objeto `5565131410b3b7bc1165d60b1331cbe8563778d869bb3ffbaec4fb5124b7c785`; texto `627a7262814dec15174c396298c970164213411d1986685efc1135f551c6c4f7`. Não são assinatura nem aprovação.

### Proposta 7

**Micélio e estruturas visíveis**

**Organismos:** *Ganoderma brownii* (`70`), *Marasmius haematocephalus* (`71`), *Entoloma trachyosporum* (`81`).

**Campo:** Curiosidade. **Escopo proposto:** Fungos filamentosos que formam cogumelos.

**Redação anterior à promoção:** A parte visível de muitos fungos é apenas uma parte do organismo, que também cresce no substrato.

**Redação aprovada e incorporada:** Em muitos fungos, filamentos chamados hifas se ramificam e formam uma rede, o micélio. Nos fungos que produzem cogumelos, as estruturas reprodutivas visíveis não representam todo o organismo: o micélio também cresce no substrato.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/24-1-characteristics-of-fungi). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Berger RG et al. (2022) — Mycelium vs. Fruiting Bodies of Edible Fungi—A Comparison of Metabolites.](https://doi.org/10.3390/microorganisms10071379).

**Localizador:** 2. Chemical Composition / mycelia and fruiting bodies; complemento: Islam et al. (2017), DOI 10.1038/s41598-017-13295-2, Introduction / hyphae and mycelial network.

**Alteração nesta revisão:** A referência anterior não sustentava claramente corpos de frutificação. A nova síntese distingue estruturas reprodutivas e micélio vegetativo, sem chamar todo cogumelo de parte indiferenciada da rede.

**Conferência editorial necessária:** As fontes explicam rede de hifas e distinção entre micélio e corpos de frutificação. Não estudam as três espécies do lote nem comprovam sua comestibilidade, ecologia particular ou composição química. A nota não afirma que toda estrutura visível seja reprodutiva.

**Fundamento conferido:** Islam et al. (2017), Introduction: hifas ramificadas e rede micelial; Berger et al. (2022), 2. Chemical Composition: distinção de morfologia, desenvolvimento e localização de micélio e corpos de frutificação.

**Limitações preservadas nos metadados:** Síntese geral de fungos filamentosos, não estudo das três espécies do catálogo. Não transfere composição química, comestibilidade, propriedades medicinais ou resultados mecânicos dos organismos estudados. Challacombe et al. (2019) é preservado como evidência histórica da versão anterior, sem sustentar o trecho reprodutivo revisado. Nenhuma figura dos artigos foi incorporada.

**Evidência já arquivada:**

- [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/e9718603ce7828b220217216878e3dba5c61a48ce0023b084838ede5583500d0.json), SHA-256 `354685cd9b248370c79f2367df8bedb3bcb924ed788b1c991e1ee66c8a4009bd`. Consulta efetiva: `2026-10-18T12:18:16.537571+00:00`.

- [CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/f3be6e0ac7c150341c9f995eff21a808e59aaee1a2ecaac8b96f2539e67b35ee.json), SHA-256 `23e713ee682d52bbf17a212fb02bf07813900a73509e62a93696643d3b71c69f`. Consulta efetiva: `2026-10-18T12:19:18.967081+00:00`.

**Vínculos da proposta:** objeto `b0f26cbaf31431b484c0f2c4ee59f37635b116420dd8aa0ee77383e7fe7adb5a`; texto `2213ee569e97bc671ab75a1813a9f1fb1b1c07efe418a57f88504953b3376ea7`. Não são assinatura nem aprovação.

### Proposta 8

**Fotossíntese de cianobactérias**

**Organismos:** *Gomphosphaeria aponina* (`110`).

**Campo:** Alimentação/nutrição. **Escopo proposto:** Cianobactérias.

**Redação anterior à promoção:** Faz fotossíntese.

**Redação aprovada e incorporada:** A fotossíntese permite às cianobactérias usar luz para incorporar carbono em compostos orgânicos, liberando oxigênio.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/22-1-prokaryotic-diversity). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Johnson MP. (2016) — Photosynthesis.](https://doi.org/10.1042/ebc20160016).

**Localizador:** Introduction / oxygenic photosynthesis in plants, algae and cyanobacteria; carbon fixation.

**Alteração nesta revisão:** Retirada a explicação sobre o processo de comprovação fotográfica, mantendo o metabolismo como afirmação de grupo.

**Conferência editorial necessária:** O metabolismo é explicado pelo escopo científico do grupo, não demonstrado pela fotografia de apoio.

**Fundamento conferido:** Johnson, Introduction: fotossíntese oxigênica, carbono orgânico e cianobactérias.

**Limitações preservadas nos metadados:** A fotografia de apoio não comprova metabolismo nem identificação da espécie.

**Evidência já arquivada:**

- [CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/5e1e3142021152579254d2eab48550b6d09ba4d78c48afef23fd8f3fa96bb495.json), SHA-256 `7516098a9094cfc796bab73c878ba7fa942f52dde69ad069e67a867b11efb9e0`. Consulta efetiva: `2026-10-18T04:43:37.559645+00:00`.

**Vínculos da proposta:** objeto `8007f8d3d6fe5c884052d234b08911530dfe44ff066dfef39c9b0e75271bdd85`; texto `125cd1f269d3c4b9289ecf94947b31087feafda1990746f3c11e10eab48ab3b0`. Não são assinatura nem aprovação.

### Proposta 9

**Morfologia observada em cultura de Gomphosphaeria**

**Organismos:** *Gomphosphaeria aponina* (`110`).

**Campo:** Morfologia. **Escopo proposto:** Gomphosphaeria aponina, cultura DSM 107014; imagem apenas de apoio.

**Redação anterior à promoção:** Células agrupadas em colônias, protegidas por uma camada gelatinosa. A imagem do jogo é apenas apoio e não confirma essas características.

**Redação aprovada e incorporada:** Em cultura estudada de Gomphosphaeria aponina, as células aparecem ligadas por estruturas mucilaginosas, formando colônias. Essa organização requer observação microscópica; a imagem de apoio do jogo não confirma a identidade nem esses detalhes.

**Referência anterior:** [OpenStax · Biology 2e](https://openstax.org/books/biology-2e/pages/22-1-prokaryotic-diversity). Sua presença não demonstra cópia ou infração.

**Referência proposta:** [Marter P et al. (2021) — Filling the Gaps in the Cyanobacterial Tree of Life-Metagenome Analysis of <i>Stigonema ocellatum</i> DSM 106950, <i>Chlorogloea purpurea</i> SAG 13.99 and <i>Gomphosphaeria aponina</i> DSM 107014.](https://doi.org/10.3390/genes12030389).

**Localizador:** 3.1 Light and Scanning Electron Microscopy / G. aponina DSM 107014 (= SAG 52.96); Figure 1 excluded.

**Alteração nesta revisão:** Mantida a ressalva no texto porque retirar a distinção entre cultura observada e imagem de apoio permitiria uma leitura enganosa.

**Conferência editorial necessária:** Observação da cultura DSM 107014 (= SAG 52.96), não confirmação da fotografia do catálogo nem caráter diagnóstico entre as alternativas do quiz.

**Fundamento conferido:** Marter et al., 3.1, primeiro parágrafo: células de DSM 107014 (= SAG 52.96) conectadas por hastes mucilaginosas e formando colônias.

**Limitações preservadas nos metadados:** A cultura DSM 107014 não é apresentada como origem da fotografia de apoio 160680326. Não confirma identidade visual ou caracteres diagnósticos nas alternativas. A aprovação editorial do texto não aprova automaticamente o grau moderado da dica.

**Evidência já arquivada:**

- [CC-BY-4.0](http://creativecommons.org/licenses/by/4.0/) da publicação específica; [snapshot selecionado](../../resources/compliance/sources/b159e807c97831c17f6dff5db0bc764f1ef56502776589510642ddb3a10733b6.json), SHA-256 `24a02c98a2474b1be25bddef527d5195145dd5e3886545da9d5c71a1e715bf53`. Consulta efetiva: `2026-10-18T04:39:55.856506+00:00`.

**Vínculos da proposta:** objeto `9127f91c26f4934e0739a5b511d21b4b0ea929fdb4ad875b3cfc3b9b712a2efc`; texto `e04ff18ce72d3c14eef9942e30968d4f0159a87f50c328dbd63f6e3c763d8d49`. Não são assinatura nem aprovação.

## Impactos e limites já verificados

As sete evidências anteriores foram preservadas; oito referências sustentam a versão final, pois a referência anterior sobre fungos foi substituída por duas específicas, também com CC BY 4.0 verificada. Autoria, DOI, localizador, data real da consulta e hash são preservados por artigo. A licença de um artigo não licencia automaticamente uma revista inteira, suas imagens de terceiros ou estas novas redações. Não foi incorporada figura ou reprodução textual dos artigos; são sínteses factuais assistidas por IA. A consulta não certifica originalidade integral nem substitui a conferência editorial.

Na simulação temporária já existente, as 34 alterações produzem 35 notas derivadas diferentes, incluindo o alias morfológico de `110`; todas exigem nova revisão. A avaliação automática das dicas é invalidada e deve ser regenerada explicitamente. As receitas de 121 organismos permanecem idênticas; na receita de aparência de `110`, acompanham a origem somente texto, fonte e localizador, preservando categorias, graus, níveis, fotografia e justificativa. A aprovação científica dessa nota não aprovará automaticamente seu peso pedagógico.

O organismo 82 está fora das propostas. Não há mudança de distribuição, conservação, identidade, fotografia ou licença fotográfica neste lote. A imagem de `110` continua apenas apoio; o bloqueio da identificação visual não é removido. A incorporação preserva 122 organismos e pool competitivo de 80; as demais informações de cada ficha continuam independentes destas notas.

As ressalvas sobre espécies experimentais e prova fotográfica permanecem nos metadados e nesta conferência. A ressalva de Gomphosphaeria também permanece na redação do jogador por ser essencial. Quatro redações adequadas foram preservadas; cinco foram ajustadas em nove das 34 ocorrências. Não foi ingerido capítulo do OpenStax para redigir esta revisão. A troca de referência não prova infração da redação anterior.

A aprovação solicitada alcança somente estas 34 notas e o mesmo conteúdo científico no alias morfológico derivado de 110. Não aprova tópicos agregados que incluam outras passagens, outros textos, identificação da fotografia, dificuldade da dica, permissões fotográficas ou o arquivo inteiro. Os tópicos agregados alterados devem refletir a necessidade de revisão sem suspender notas individuais não alteradas. A decisão de direitos é independente e não transforma a CC BY 4.0 em licença global do catálogo.

## Duas confirmações recebidas

1. **Editorial:** aprovar as nove redações finais nos 34 campos listados, incluindo o mesmo conteúdo científico no alias morfológico de 110, vinculados ao fingerprint acima. É revisão editorial do mantenedor, sem alegação de revisão por especialista ou aprovação do grau de assistência.

2. **Direitos:** confirmar CC BY 4.0 somente nos direitos efetivamente disponíveis sobre essas 34 redações assistidas por IA, preservando crédito do projeto, identificação do lote, fontes e avisos; sem autoria humana exclusiva, relicenciamento dos artigos ou liberação das demais passagens e materiais.

O mantenedor respondeu **“Aprovo editorialmente esta versão completa”** e, separadamente, **“Confirmo a aplicação delimitada a este lote”**. As perguntas e respostas reais, o fingerprint, todas as notas e os vínculos estão na [evidência da confirmação](../../resources/compliance/sources/2e9ce124b2056fdb0683dbd55b15dd2391f6e42bb0ffb1dd712ecb572510c5ce.json), SHA-256 `f0651e9f4f618f2e9a36a3d04bc8d300ab00b21e61f2dc4c91e5fdf6bc5f324d`. Foram incorporadas as 34 notas e registrados 35 eventos editoriais, incluindo o alias científico de 110. Não foram aprovados tópicos agregados ou graus de assistência. O restante do arquivo continua pendente; sua distribuição integral permanece bloqueada.

## Continuidade após o PR #19

O lote de grupos já foi integrado em `09e8eb759172e32b9554c4c86d8a583a667e4dcc` e permanece intacto. As confirmações deste lote foram registradas sem ampliar o seu escopo; os lotes seguintes continuam dependentes de evidências e decisões próprias.

| Lote | Conteúdo | Próximo critério antes da promoção |
| --- | --- | --- |
| 2 | Estas 34 notas de fichas | Incorporadas após as duas confirmações; as 805 passagens restantes da origem continuam inconclusivas e o arquivo inteiro não foi liberado. |
| 3 | Demais passagens de `species_profiles.json` | Investigar origem por ocorrência, reutilizar evidências suficientes e só propor substituições fundamentadas; manter incertezas de distribuição/conservação e a revisão já realizada de `82`. |
| 4 | Etimologias, explicações e justificativas de `hint_education.json` | Conferir fontes nomenclaturais e expressão textual separadamente; preservar categorias e graus, reavaliando contexto apenas quando mudanças reais exigirem. |
| 5 | Conferência final de `progressive_hints.json` | Regenerar exclusivamente pelo fluxo existente e verificar a cobertura de direitos das origens, sem licença independente inferida sobre derivados. |

A avaliação automática e a projeção educativa precisam ser regeneradas em cada lote efetivamente alterado; o lote 5 é a conferência final, não uma autorização para manter derivados desatualizados até lá. Cada promoção exige auditoria estrita, governança, integridade, regressões e gate de distribuição, que conserva pendências independentes.
