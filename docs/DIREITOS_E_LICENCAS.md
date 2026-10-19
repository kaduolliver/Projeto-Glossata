# Direitos, materiais próprios e terceiros

Em **2026-10-17**, o mantenedor autorizou **Apache-2.0 para o código próprio expressamente enumerado**, com Carlos Eduardo Lima de Oliveira identificado por ele como criador original. Consulte [LICENSE](../LICENSE), [NOTICE](../NOTICE) e [escopo e exclusões](ESCOPO_LICENCIAMENTO.md). A licença permite uso comercial conforme seus termos; o caráter sem fins lucrativos descreve a iniciativa oficial. Não há licença global sobre todos os materiais do repositório.

A [preparação de licenciamento](PLANO_LICENCIAMENTO.md) preserva o diagnóstico anterior à escolha e a análise das versões exatas do pacote. O [registro de decisão](propostas/LICENCIAMENTO_CODIGO.md) identifica a mudança administrativa, sem converter aprovações antigas ou criar assinatura/revisão jurídica. [CONTRIBUTING.md](../CONTRIBUTING.md) orienta submissões compatíveis com Apache-2.0 sem cessão/CLA. Publicidade, gratuidade e referência bibliográfica continuam insuficientes para presumir direitos externos.

| Camada | Registro | Situação e providência |
| --- | --- | --- |
| Código próprio enumerado | `ESCOPO_LICENCIAMENTO.json`, LICENSE/NOTICE e política | Apache-2.0 adotada; lógica do gerador incluída após extração da redação; limites da investigação de origem documentados |
| Textos próprios | Corpus e introduções, incluindo assistência de IA | CC BY 4.0 limitada à expressão educativa documentada por passagem; grupos, fichas e dicas atuais cobertos pelos lotes expressamente confirmados, sem licença global de fatos, fontes ou materiais |
| Organização da coletânea | Seleção/estrutura do catálogo | Definir direitos sobre a organização, respeitando fatos e contribuições individuais |
| Dados de terceiros | Identificadores, classificações e registros das fontes | Conferir termos/direitos aplicáveis sem inferir licença das fotos |
| Fotografias | `photo_provenance.json` e snapshots por foto | Completar evidência de licença, versão, crédito e data |
| Ícones/identidade | `asset_rights.json` | Uso/distribuição documentados somente nas versões oficiais, com escopo, evidência e hashes; sem licença gráfica geral |
| Bibliotecas, fontes e outros recursos | `package/compliance/dependencies.json` | Revisar por componente e versão; cumprir avisos e obrigações |

Decisões de licença e opções ainda pendentes:

| Material próprio | Opções a considerar | Registro necessário |
| --- | --- | --- |
| Código | Apache-2.0 escolhida para o escopo próprio delimitado | Concessão expressa; compatibilidade/obrigações do pacote e contribuições externas permanecem análises separadas |
| Texto educativo original | CC BY 4.0 escolhida como padrão, inclusive para futuras redações próprias | Origem, direitos efetivamente disponíveis e exclusões por passagem; padrão não licencia automaticamente arquivos inteiros |
| Organização/dados próprios | CC0, CC BY ou termos próprios quando cabíveis | Delimitar quais direitos pertencem ao mantenedor e quais dados vêm de terceiros |

`content_policy.json` e `asset_rights.json` registram `code: "Apache-2.0"`. Em **2026-10-17**, o mantenedor também adotou a [política editorial de licenciamento misto](propostas/LICENCIAMENTO_TEXTOS.md): CC BY 4.0 para redações próprias elegíveis, com usos comerciais e adaptações mediante atribuição, limitada aos direitos disponíveis. Essa decisão está separada em `editorial_text_policy`; `own_material_licenses.texts` permanece `null` porque não há concessão indistinta ao corpus. `collection` continua pendente. Textos externos conservam condições próprias, incluindo SA/NC/ND quando aplicáveis; bibliografia não comprova origem autoral. Nenhuma passagem legada foi declarada regularizada nesta decisão.

A redação antes incorporada em `scripts/build_education.py` foi extraída para `resources/data/group_education.json`; essa versão legada e suas pendências permanecem no Git. Em 2026-10-18, as 68 ocorrências foram substituídas pelo lote novo expressamente aprovado editorialmente, com concessão CC BY 4.0 limitada aos direitos disponíveis. Crédito: Projeto Glossata — redação assistida por IA; revisão editorial de Carlos Eduardo Lima de Oliveira. Artigos científicos e fatos não são relicenciados. A lógica Python separada continua enumerada sob Apache-2.0. A licença do código não engloba fotos nem substitui avisos/obrigações de terceiros. Nome/logotipo/ícones têm tratamento próprio, sem declaração de registro de marca ou concessão automática. Após a autorização oficial dos ícones e a confirmação concreta deste lote, os sete achados originais de outros materiais passam a três textuais remanescentes. Nenhuma passagem recebeu licença automaticamente. Consulte [SANEAMENTO_ISSUE_2.md](SANEAMENTO_ISSUE_2.md).

A identidade visual atual foi preservada e sua [política reservada](propostas/USO_IDENTIDADE_VISUAL.md) foi escolhida expressamente pelo mantenedor em 2026-10-17. Não há concessão gráfica geral para reutilização ou sublicenciamento por projetos independentes; a reserva limita-se aos direitos efetivamente disponíveis. A autorização adicional de 2026-10-18 permite somente o uso/distribuição oficial dos três hashes documentados (`documented_official_use`), sem comprovar titularidade exclusiva ou direitos de terceiros. Consulte a [documentação independente](IDENTIDADE_VISUAL.md) e a [situação individual dos recursos](POLITICA_BRANDING_IMPLEMENTACAO.md). Apache-2.0, visualização/fork no GitHub e usos legalmente permitidos permanecem respeitados. A Issue #2 continua aberta e o gate do pacote permanece bloqueado pelas demais pendências.

Atualização do lote 2 em **2026-10-18**: após duas confirmações expressas do mantenedor, 34 novas notas de fichas/28 organismos recebem revisão editorial e CC BY 4.0 apenas sobre direitos disponíveis na versão identificada pelo fingerprint. Outras 805 ocorrências de `species_profiles.json` permanecem inconclusivas; a permissão do arquivo inteiro continua falsa, assim como a das receitas compostas. O inventário por passagem e a [evidência do lote](SANEAMENTO_ISSUE_2.md) delimitam o alcance. Fontes externas, fatos, fotografias e branding não são relicenciados. A regularização do grupo anterior e suas permissões foram preservadas.

## Resolução textual consolidada — 2026-10-18

Após as duas confirmações reais do mantenedor, o lote consolidado incorporou 887
substituições e 264 cópias derivadas, preservando duas notas já aprovadas e 1.249
ocorrências anteriormente documentadas. Os três recursos textuais remanescentes
da Issue #2 têm cobertura de direitos por passagem, limitada à expressão educativa
e aos direitos efetivamente disponíveis. Fontes, fatos e dados externos não são
relicenciados; não há autoria humana exclusiva ou revisão especializada presumida.
O [relatório e a evidência](SANEAMENTO_ISSUE_2.md) registram o escopo, os testes e
a preservação dos 76 eventos anteriores. As contagens de 805 passagens e três
pendências nos registros de lotes anteriores acima descrevem aquelas etapas.
O gate geral continua bloqueado por pendências de outros recursos. A integração
e o encerramento da Issue dependem de ação do mantenedor.

## Política das fotografias

O padrão técnico aceita somente **CC0 e CC BY**, por fotografia. Créditos de CC0 são preservados por decisão editorial; não se sugere endosso do titular ou do iNaturalist. O nome do fotógrafo é o crédito fornecido pela fonte da imagem, que pode diferir do observador. Título só é registrado quando fornecido. O link específico da fotografia não é substituído por um link genérico do site.

CC BY exige observar as condições de atribuição, indicação de alterações e demais termos aplicáveis. Alterações posteriores do site devem ser analisadas preservando a evidência do grant obtido, sem concluir automaticamente que uma licença válida foi revogada. [CC BY 4.0, texto legal](https://creativecommons.org/licenses/by/4.0/legalcode.en).

CC0 não elimina automaticamente direitos de personalidade, privacidade, marcas ou outros direitos que possam afetar um uso. [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/).

CC BY-NC está desativada. Gratuidade do projeto não é, isoladamente, uma análise de todos os usos comerciais possíveis. Uma exceção exige política escrita, distribuição não comercial controlada e permissão específica documentada. [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/).

SA e ND não são licenças sem condições: compartilhamento pela mesma licença e limitações sobre adaptações exigem análise própria. Permanecem fora da política automática. [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), [CC BY-ND 4.0](https://creativecommons.org/licenses/by-nd/4.0/).

Uma autorização alternativa deve identificar titular, foto, origem, evidência íntegra, data, responsável, justificativa, escopos `download`/`distribution`, restrições e, para distribuir, os bytes autorizados. Não generalizar autorização de uma observação para todas as suas fotos. Não interpretar silêncio, URL existente ou licença genérica como autorização irrestrita.

Os campos de licença de observação, dados e imagens no iNaturalist são distintos; é necessário conferir a mídia individual. [Orientação do iNaturalist sobre uso das mídias](https://help.inaturalist.org/en/support/solutions/articles/151000169918), [funcionamento das licenças](https://help.inaturalist.org/en/support/solutions/articles/151000173511).

## Fontes, privacidade e dependências

Research Grade é um filtro comunitário da fonte, não uma revisão especializada do conteúdo do aplicativo. A relação entre táxon da observação e táxon comunitário importa para essa classificação. [Qualidade dos dados](https://help.inaturalist.org/en/support/solutions/articles/151000169936), [táxons comunitário e da observação](https://help.inaturalist.org/en/support/solutions/articles/151000173076).

A aplicação continua sem consultas de conteúdo durante o jogo. Manutenção usa chamadas espaçadas e limitadas, sem tentar obter dados ocultados. Não publicar coordenadas ou informações pessoais desnecessárias em evidências e relatos. [Práticas de API](https://www.inaturalist.org/pages/api+recommended+practices), [limites do provedor](https://www.inaturalist.org/pages/developers), [proteção de locais sensíveis](https://help.inaturalist.org/en/support/solutions/articles/151000233080-how-does-inaturalist-protect-the-locations-of-sensitive-species-).

A distribuição do Qt e de seus componentes exige revisar as condições de cada módulo e versão. Linkagem dinâmica não resolve sozinha avisos, disponibilidade do código correspondente, possibilidade de substituição e outras obrigações aplicáveis. Inventariar dependências e copiar textos de licença são etapas técnicas; ainda é necessário decidir e cumprir o meio de fornecimento de código e demais condições. [Orientação oficial Qt sobre GPL/LGPL](https://www.qt.io/development/open-source-lgpl-obligations), [licenciamento por componente](https://doc.qt.io/qt-6/licensing.html).

Nenhum arquivo da fonte Segoe UI é empacotado atualmente; o aplicativo solicita uma fonte do sistema. Se fontes forem incluídas futuramente, precisam entrar no inventário de recursos e no gate de distribuição.
