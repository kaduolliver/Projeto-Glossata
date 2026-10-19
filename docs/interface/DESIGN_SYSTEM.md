# Inventário do design system atual

Base: `b201e20c2d83fda6d790bc6a96217c0fdc75307e`. Este documento descreve valores existentes; não cria tokens nem altera QML.

| Papel atual | Valores exemplares | Origem |
| --- | --- | --- |
| Fundo da janela e barra lateral | `#091A22`, `#10242D` | `Main.qml` |
| Superfícies e controles | `#132B35`, `#11262F`, `#203943` | paleta de `Main.qml` e componentes |
| Texto principal | `#E6F1E9`, `#F5FAF7` | paleta e cabeçalho |
| Texto auxiliar | `#9AB2B8`, `#91ACB4`, `#718F96` | cabeçalho, exploração e filtros |
| Ação primária/seleção | `#218653`, gradiente `#29C878` → `#0DA662` | paleta, `UiButton` |
| Ênfase semântica | verde para acerto, vermelho para erro/perigo, âmbar para atenção, azul/roxo para métricas | `FeedbackView`, `ResultsView`, `StatisticsView`, `HintPanel` |

Tipografia base: `Segoe UI`, 14 px em `Main.qml`, com títulos maiores por tela, pesos `DemiBold`, rótulos menores e espaçamento de letras em divisórias. A aplicação usa Qt Quick Controls `Basic`. `UiButton` oferece primário, secundário e perigo, altura 50 px ou 40 px no modo compacto, raio 12 px e nome acessível pelo texto. `NavItem` tem altura 46 px, raio 9 px, indicador verde de seleção e estado de foco. Cartões, estatísticas, fotos com gradiente e seletores possuem arquivos próprios, mas não compartilham um arquivo único de tokens.

Os layouts usam `RowLayout`, `ColumnLayout`, `GridLayout`, `Flow` e uma área principal `Flickable`. `CategoryView` alterna de três para duas colunas quando a largura da view cai abaixo de 850 px; `TaxonomyView` usa duas colunas abaixo desse limite. A janela impõe tamanho mínimo de 1000 × 700. `Main.qml` controla uma transição de opacidade de 180 ms que o usuário pode desativar em Ajustes.

## Centralização futura, sujeita a revisão visual

1. Inventariar usos reais de cada cor, dimensão, fonte e estado de foco, inclusive contrastes e significado semântico. Consolidar somente os valores equivalentes em um módulo QML de tokens; preservar diferenças intencionais.
2. Fazer componentes compartilhados consumir os tokens antes de migrar views uma a uma. Verificar texto, foco, seleção, estados indisponíveis e variantes de botão com captura real e testes de teclado.
3. Definir critérios de adaptação para janela mínima, escala de exibição e rolagem; validar em Qt software e em ambiente gráfico interativo antes de declarar consistência visual.

Nenhuma paleta nova ou resultado de auditoria de contraste é afirmado aqui. A cor de uma dica indica assistência definida pelo conteúdo; não altera pontuação.
