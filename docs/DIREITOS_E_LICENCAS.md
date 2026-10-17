# Direitos, materiais próprios e terceiros

Nenhuma licença global foi aplicada nesta etapa. Repositório público, gratuidade, finalidade educativa e referência bibliográfica não bastam para presumir permissão de cópia de um material. A decisão sobre direitos dos materiais próprios pertence ao mantenedor, após confirmar sua autoria/titularidade.

O código público tem intenção de evolução open source, sem uma licença própria escolhida. A [preparação de licenciamento](PLANO_LICENCIAMENTO.md) compara opções, delimita as camadas e documenta as versões exatas do pacote inspecionado; a [proposta de decisão](propostas/LICENCIAMENTO_CODIGO.md) não está ativada. [CONTRIBUTING.md](../CONTRIBUTING.md) orienta origem, direitos e revisão das contribuições sem cessão ou anuência a licença futura. Nenhum desses documentos substitui os registros individuais de permissão.

| Camada | Registro | Decisão pendente |
| --- | --- | --- |
| Código próprio | `src/`, scripts e QML de autoria confirmada | Escolher licença e confirmar contribuições/titulares |
| Textos próprios | Corpus e introduções, incluindo assistência de IA | Revisar originalidade e evidências; escolher licença apenas do texto próprio |
| Organização da coletânea | Seleção/estrutura do catálogo | Definir direitos sobre a organização, respeitando fatos e contribuições individuais |
| Dados de terceiros | Identificadores, classificações e registros das fontes | Conferir termos/direitos aplicáveis sem inferir licença das fotos |
| Fotografias | `photo_provenance.json` e snapshots por foto | Completar evidência de licença, versão, crédito e data |
| Ícones/identidade | `asset_rights.json` | Documentar titular e autorização para distribuição |
| Bibliotecas, fontes e outros recursos | `package/compliance/dependencies.json` | Revisar por componente e versão; cumprir avisos e obrigações |

Opções para o mantenedor avaliar, sem escolha automática:

| Material próprio | Opções a considerar | Registro necessário |
| --- | --- | --- |
| Código | MIT, Apache-2.0, GPL-3.0 ou termos próprios adequados | Escolha expressa, titulares, abrangência e compatibilidade com dependências |
| Texto educativo original | CC BY 4.0, CC BY-SA 4.0 ou termos próprios | Autoria, escopo, condições desejadas e exclusões de terceiros |
| Organização/dados próprios | CC0, CC BY ou termos próprios quando cabíveis | Delimitar quais direitos pertencem ao mantenedor e quais dados vêm de terceiros |

Os nomes acima são opções de discussão, não recomendação jurídica nem autorização já concedida. `content_policy.json` mantém as escolhas próprias como `null`. Uma eventual licença do código não deve englobar fotos ou avisos incompatíveis. Termos próprios não podem retirar direitos concedidos por licenças de terceiros.

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
