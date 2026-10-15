# Fichas das espécies

> Registro hist?rico: marca??es antigas de verifica??o/aprova??o n?o demonstram revis?o humana rastre?vel. Consulte a [governan?a atual, matriz e pend?ncias](CONFORMIDADE.md). Cobertura t?cnica n?o ? certifica??o cient?fica ou jur?dica.

Conteúdo atualizado em 14/10/2026 para os 122 organismos ativos. A ficha aberta por **Ver mais informações** mantém a paleta, fotografia, créditos, ícones e botões do jogo, com rolagem para textos maiores.

O nome popular em português aparece primeiro quando disponível, seguido do nome científico e de uma apresentação curta. Quando não há nome popular estabelecido na revisão, o título descreve o grupo, como **Mariposa**, **Orquídea** ou **Cianobactéria**; isso não cria um nome popular para a espécie.

A ficha reúne distribuição/habitat, alimentação ou nutrição quando documentada, morfologia, conservação, curiosidade e pista. A classificação científica completa e as referências podem ser expandidas. Os textos são apresentados como texto simples, sem interpretação de HTML.

## Cobertura e limites dos dados

| Conteúdo | Registros |
|---|---:|
| Apresentação, distribuição/ocorrência e morfologia | 122 |
| Curiosidade e pista associadas a cada organismo | 122 |
| Alimentação/nutrição documentada | 107 |
| Categoria global da IUCN registrada no iNaturalist | 22 |
| Conservação sem categoria global confirmada | 100 |

Distribuição natural, local de isolamento e observação pontual são informações diferentes. As fichas de organismos pouco documentados indicam a localidade confirmada e esclarecem que ela não delimita toda a distribuição. Exemplares em zoológicos não estabelecem a distribuição silvestre. O sapo-dourado tem distribuição **histórica**, pois está extinto.

Descrições de aparência baseadas na fotografia têm esse escopo registrado. São pistas de observação, não uma garantia de identificação no nível de espécie. Curiosidades gerais conservam o grupo ao qual se referem. Os 20 organismos de Monera recebem descrições biológicas, sem usar fotos de hospedeiros ou de espécies relacionadas como prova de forma celular ou distribuição.

Na conservação, foram consultados os registros públicos dos 122 táxons na API do iNaturalist. Uma categoria só foi usada quando o registro pertence exatamente à espécie, tem autoridade **IUCN Red List** e não possui região associada. Avaliações regionais, de subespécies e de ancestrais não foram transferidas. A ficha informa a procedência e a data da consulta, além de oferecer o link da avaliação citada. Essa data não é a data da avaliação original.

Ausência de categoria confirmada não equivale a **Pouco preocupante**, **Não avaliada** ou ausência de ameaças. Por exemplo, a avaliação da subespécie migratória da monarca não foi aplicada automaticamente à espécie inteira. O catálogo não atualiza essas categorias pela rede durante uma partida.

## Fontes e manutenção

O corpus editável fica em [species_profiles.json](../resources/data/species_profiles.json), separado dos textos históricos de species.json. Cada tópico possui texto, referência, escopo e data de consulta. As fontes incluem Animal Diversity Web, Kew, NCSU, museus, MarLIN, coleções de microrganismos e artigos científicos, além dos registros de ocorrência indicados nas próprias fichas.

Execute `python scripts/build_education.py` para incorporar as fichas a education.json. A geração ocorre offline e valida IDs, nomes científicos, campos obrigatórios, fontes e escopo da conservação antes de gravar. Campos de alimentação sem informação confirmada são omitidos.

EducationalContent também confere **ID e nome científico** antes de aplicar a ficha, evitando transferir textos para uma futura substituição de organismo. O quiz utiliza as pistas revisadas e conserva o filtro que rejeita nomes de alternativas ou da linhagem. O conteúdo introdutório permanece disponível. O quiz agora seleciona categorias de dicas por nível e alternativas, conforme [Dicas e pontuação](DICAS_E_PONTUACAO.md); nem todas as categorias estão disponíveis em todos os níveis.

## Validação

- Testes do conteúdo: cobertura dos 122 organismos, fontes por tópico, identidade, conservação global, distinção entre extinção e ausência de categoria, nutrição dos grupos não animais e aviso de apoio em Monera.
- Testes de interface: nome/apresentação, tópicos, expansão das fontes, retorno ao quiz, ficha longa do leão e controles inferiores em 1240 × 820 e 1000 × 700.
- Quatro suítes CTest e lint QML. Capturas em build/tests/previews, incluindo species-lion-information.png e compact-species-lion-sources.png.

Esta revisão organiza as evidências consultadas; os registros pouco documentados mantêm os limites descritos na própria ficha.
