# Política editorial e licenciamento misto dos textos

## Decisão adotada e limites

Em **2026-10-17**, Carlos Eduardo Lima de Oliveira, mantenedor do Projeto Glossata, autorizou expressamente **CC BY 4.0 como licença padrão das redações educativas originais elegíveis**, inclusive das futuras redações enciclopédicas próprias. A finalidade é divulgação e reutilização do conhecimento, permitindo usos comerciais e adaptações mediante atribuição e demais condições da [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode.en), §§2–3. O caráter sem fins lucrativos da iniciativa oficial não acrescenta restrição NC.

A decisão administrativa está em `content_policy.json`, `editorial_text_policy`, vinculada ao registro `eligible-original-text-cc-by-4.0-2026-10-17`. Sua base é a solicitação expressa do mantenedor; não representa assinatura, cessão de colaboradores, revisão especializada ou prova de titularidade exclusiva, inclusive sobre redação assistida por IA.

**Política padrão adotada não equivale a concessão sobre cada material.** A aplicação depende de delimitar redação, origem e direitos efetivamente disponíveis. Nenhum arquivo educativo inteiro nem passagem do corpus legado foi liberado nesta intervenção. Os quatro recursos educativos continuam pendentes. `own_material_licenses.texts` permanece `null`, pois não há concessão indistinta ao corpus; `collection` continua decisão independente. A lista documental `educational_text_rights` do inventário existente começa vazia, sem preencher origens desconhecidas.

Excluem-se do padrão textos externos, reproduções, citações, traduções/adaptações sujeitas a direitos próprios, fotografias, ícones, dados externos, fatos enquanto tais e organização da coletânea. A licença só alcança direitos que o concedente pode efetivamente disponibilizar; não se atribuem ao mantenedor textos de terceiros. Código enumerado permanece Apache-2.0, identidade visual reservada e fotografias com licença individual.

O caminho deste documento foi preservado. Sua versão anterior era uma proposta **não adotada**; o histórico Git conserva esse estado correto à época. A decisão atual escolhe o padrão, sem converter a investigação anterior em comprovação de direitos.

## Incorporação de fontes e licenças distintas

| Origem ou utilização | Tratamento editorial |
| --- | --- |
| Redação própria elegível | Delimitar expressão, autoria/contribuições disponíveis e evidências antes de indicar CC-BY-4.0. Uma URL ou ausência de coincidência textual não prova originalidade. |
| Síntese factual independente | Identificar bibliografia que sustenta os fatos e, separadamente, a origem da redação. Avaliar independência de expressão; trocar palavras não basta. Fatos não recebem reivindicação de exclusividade. |
| Wikipédia | Conferir edição, revisão permanente, rodapé, histórico, avisos de importação e licença específica. Distinguir reprodução, tradução, adaptação e consulta factual. Preservar ShareAlike quando aplicável, sem converter uma adaptação CC BY-SA em CC BY exclusiva. |
| Descrições encontradas no iNaturalist | Verificar fonte textual efetiva, eventualmente Wikipédia, e respectivos autores, licença e avisos. Hospedagem não demonstra titularidade. Direitos sobre descrição, dados da observação, foto e som são separados. |
| Artigos e instituições | Bibliografia confiável pode fundamentar síntese independente; acesso público e atribuição não autorizam reprodução, tradução ou adaptação de expressão protegida. Conferir licença/versão do material específico ou outro fundamento aplicável, sem copiar extensivamente. |
| NC | Registrar e avaliar a condição não comercial frente ao uso e à distribuição pretendidos. Gratuidade oficial não regulariza automaticamente todos os usos. Não apresentar a coletânea inteira como comercialmente reutilizável se houver partes restritas. |
| ND | Distinguir compartilhamento não modificado de tradução/adaptação. Não usar a licença como autorização geral para divulgar adaptações; exigir compatibilidade ou autorização específica demonstrada. |
| Origem inconclusiva | Manter pendente; não aplicar automaticamente o padrão escolhido. Conteúdo de IA também exige origem, fontes e revisão, sem autoria humana exclusiva presumida. |

Os [Termos de Uso da Wikimedia, §7](https://foundation.wikimedia.org/wiki/Policy:Terms_of_Use#7._Licensing_of_Content) descrevem CC BY-SA 4.0, alternativas/exceções e atribuição por artigo/histórico ou outros meios admitidos. Examine a licença real da versão utilizada. A reunião de obras independentes não deve ser confundida com adaptação; ShareAlike não é aplicado automaticamente a toda a futura enciclopédia. Consulte também [CC BY-SA 4.0, §§1–3](https://creativecommons.org/licenses/by-sa/4.0/legalcode.en).

O iNaturalist distingue [licenças de dados, imagens e sons](https://help.inaturalist.org/en/support/solutions/articles/151000175695); essas licenças não estabelecem, por si, direitos sobre uma descrição taxonômica externa. Consulte os [termos da plataforma](https://www.inaturalist.org/pages/terms) e a fonte efetiva da passagem. As condições [NC](https://creativecommons.org/licenses/by-nc/4.0/) e [ND](https://creativecommons.org/licenses/by-nd/4.0/legalcode.en) permanecem específicas; esta decisão não modifica a política fotográfica CC0/CC BY nem suas exceções.

As referências oficiais acima foram consultadas na preparação desta política. Não são novas evidências de autoria ou permissões do corpus, nem novas consultas científicas sobre organismos.

## Contrato mínimo por passagem

Reutilize `educational_texts` de `python scripts/review_updates.py --rights-report`. Esse inventário já agrupa redações exatas por SHA-256 e conserva **cada ocorrência** por arquivo e ponteiro JSON. Não copie os textos para outro banco nem renove datas de revisão ao gerar relatórios.

Registros documentais opcionais ficam em `resources/compliance/asset_rights.json`, lista `educational_text_rights`. A lista não é ledger de decisões ou via alternativa de autorização. O contrato vale para os quatro recursos já inventariados; novas fontes da futura enciclopédia exigirão ampliar explicitamente a cobertura do mesmo inventário antes de registrar suas passagens.

| Campo | Contrato e reaproveitamento |
| --- | --- |
| `file`, `pointer`, `text_sha256` | Vínculo obrigatório à ocorrência atual. `pointer` segue JSON Pointer; `text_sha256` é SHA-256 dos bytes UTF-8 da redação, conforme o inventário. Mesmo texto em outra localização não herda direitos. Alteração exige nova conferência e preservação do registro anterior no Git. |
| `origin_kind` | `original`, `external_reproduction`, `adaptation`, `translation`, `independent_factual_synthesis` ou `inconclusive`. É classificação de origem, não aprovação editorial. |
| `declared_origin` | Descrição factual da origem conhecida, assistência de IA e contribuições, quando documentadas. Use `null` para informação desconhecida. |
| `source_text` | Objeto opcional para **origem autoral**, distinto de bibliografia. Quando disponíveis: `url`, `title`, `author_credit`, `license` (SPDX quando conhecido), `license_version`, `license_url`, `source_revision`, `consulted_at`, `locator`. Registre a data real e uma revisão permanente quando houver. Não reutilize a URL científica como origem textual por inferência. |
| `transformations` | Lista contendo `translation` e/ou `adaptation`, quando aplicáveis. Lista vazia não prova ausência histórica; explicar incerteza em `declared_origin`. |
| `rights_available`, `declared_license` | Escopo dos direitos demonstrados/declarados e licença da passagem, sem preencher pela política padrão. Não significam confirmação jurídica ou autorização para o pacote. Licença desconhecida permanece `null`. |
| `evidence` | Referências locais existentes `{path, sha256, locator}`; hash dos bytes da evidência, não assinatura/autenticação. Não publicar documentos privados ou dados pessoais desnecessários. |
| `investigation_state` | `pending_human_review`, `documented`, `inconclusive`, `rejected` ou `suspended`, descrevendo a investigação documental. `documented` não significa revisão editorial/científica aprovada. Não altera os estados canônicos dos recursos. |
| `editorial_review_key` | Referência à chave de revisão existente, quando aplicável; por exemplo, uma nota ou tópico já coberto pela governança. Responsável, estado, data e justificativa continuam no mecanismo existente, vinculados ao hash. Não criar aprovação dentro deste registro. |
| `attribution` | Objeto com `author_credit`, `title`, `original_url`, `license_url`, `notices`, `modifications`, conforme fornecidos/aplicáveis. Traduções/adaptações também ficam indicadas em `transformations`. Não inventar dados ausentes. |

Quando houver contexto documental compartilhado demonstrado, um registro pode usar `occurrences: [{file, pointer, text_sha256}, ...]` em lugar dos três campos de vínculo na raiz. Não misture os formatos no mesmo registro. Cada ocorrência continua explícita; duplicatas ou vínculos desatualizados são recusados. O agrupamento de investigações inconclusivas não afirma autoria ou licença comum. Bibliografia e contexto de cada ocorrência permanecem no inventário original, sem duplicação de redações ou inferência por hash compartilhado.

Para um recurso educativo que alegue permissão de distribuição, o gate verifica também cobertura de todas as ocorrências atuais, ausência de pendências autorais, atribuição aplicável e integridade dos metadados/evidências. Uma única passagem documentada não libera o arquivo inteiro. O registro de permissão do recurso e sua evidência continuam necessários; a cobertura não cria essa autorização. O campo existente de titular pode ser substituído por `permission.holders`, lista de titulares efetivamente identificados, quando houver mais de um; não preencher um titular artificial para representar materiais mistos. Créditos não fornecidos pela fonte podem ser explicitamente indicados por `attribution.credit_not_supplied_by_source: true`, com evidência, sem inventar nomes. Isso não dispensa as demais obrigações aplicáveis.

Identidade do organismo/grupo e tipo de conteúdo são obtidos da localização, papel e vínculos do inventário e de suas fontes editáveis; bibliografia e localizador científico já estão nas ocorrências. Não duplicar esses dados como outra autoridade. Para textos derivados, conservar `derived_from` e investigar os direitos da origem, sem concessão independente inferida sobre a saída.

O leitor valida vínculos, duplicatas, formato básico e hashes de evidências. Aponta origem inconclusiva, fonte textual/evidência insuficiente e condições SA/NC/ND reconhecidas nos identificadores declarados. **Não é um analisador jurídico universal:** não verifica identidade/autenticidade de autores, todos os termos de uma licença ou independência semântica; licenças desconhecidas e situações especiais exigem análise humana. Campos de aprovação/permissão não fazem parte deste contrato.

A escolha administrativa altera um arquivo já incluído nos hashes de entrada da avaliação automática de dicas. Por isso foi necessário regenerar essa avaliação e sua projeção em `education.json`, pelo fluxo existente. Todas as receitas e seus textos, graus, identidades e referências foram comparados com a base e preservados; apenas o hash de entrada e os metadados da avaliação automática mudaram. A avaliação anterior permanece no Git e o hash anterior do recurso no inventário. Isso não renova uma revisão científica humana nem autoriza direitos.

`scripts/licensing_inventory.py --check-code` mantém todas as verificações Apache e acrescenta a coerência da política e a integridade estrutural desses registros, na CI existente. O relatório de direitos mostra os apontamentos; não modifica os recursos, as revisões, os créditos ou o gate. Uma lista vazia ou um registro sem apontamentos **não concede direitos nem libera distribuição**. A regularização de um recurso requer decisão legítima no fluxo existente, evidência adequada e eventual delimitação explícita de suas partes; esta intervenção não cria um comando de aprovação de textos.

## Atribuição e contribuições futuras

Preservar, quando exigidos e disponíveis: autores/partes indicadas para atribuição, título ou identificação da obra, origem específica, licença/versão e link, avisos aplicáveis e indicação das modificações e traduções/adaptações. Respeitar as condições da licença efetiva sem exigir dados que ela não exige ou a fonte não fornece. Não substituir autores externos pelo criador do Glossata nem sugerir endosso da Wikipédia, iNaturalist ou instituições.

Cada contribuição deve declarar origem e transformações, fornecer evidência dos direitos necessários e separar revisão editorial da científica. O padrão CC BY 4.0 pode ser aplicado às contribuições próprias elegíveis com termos de submissão compatíveis; não há cessão presumida, CLA automático ou atribuição exclusiva ao mantenedor. Conteúdo ainda não revisado permanece no estado existente de rascunho/pendência, independente de sua categoria autoral. Não se exige equipe externa para revisão editorial real do mantenedor; revisão por especialista só é indicada quando comprovada.

## Diretrizes para a futura enciclopédia

Somente planejamento: artigos poderão ter visão geral, taxonomia, morfologia/anatomia, alimentação, comportamento, reprodução, distribuição/habitat, ecologia, conservação, relações evolutivas, curiosidades e referências. Cada seção poderá reunir fontes e condições distintas, delimitadas por passagem/seção, com créditos, fontes científicas, licenças e histórico de alterações consultáveis offline.

A futura interface deverá diferenciar essas camadas e não apresentar uma licença única para conteúdo misto. Ainda não existem novas telas, navegação enciclopédica, artigos extensos, pesquisa avançada ou integrações nesta intervenção. A ampliação do modelo de apresentação fica para a etapa posterior de personalização/refinamento.

## Issue #2 e próximos passos

A **escolha da licença padrão está concluída**. A conferência dos direitos do corpus e a concessão sobre partes delimitadas permanecem pendentes: quatro recursos educativos e três ícones continuam nos sete achados da Issue #2. Fotografias, dependências, coletânea e revisão dos dois trechos do organismo 82 conservam seus tratamentos separados. Nenhuma passagem foi declarada regularizada por esta política.

Próxima ação do mantenedor: examinar ocorrências/evidências já inventariadas, registrar a origem e os direitos demonstráveis sem generalizar por hash compartilhado, delimitar exclusões e decidir efetivamente os casos aplicáveis. Não usar `confirm-permission`, específico de fotografias, para aprovar textos. Fechar uma Issue não confirma direitos. A reprodução ou distribuição continua bloqueada quando faltarem requisitos obrigatórios.
