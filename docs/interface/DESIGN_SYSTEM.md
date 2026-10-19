# Design system — revisão 2.1.2

Os tokens ficam em `resources/qml/style/Theme.qml`, singleton QML local registrado por `qmldir` e embarcado no QRC. `UiButton`, `NavItem`, `StatCard` e `TextAction` usam os tokens; as demais views conservam seus estilos. Não há dependência nova ou mudança na organização das telas.

| Papel | Tokens/valores |
| --- | --- |
| Fundo e superfícies | `canvas` #091A22; `surface` #10262D; `surfaceRaised` #18343C; `surfaceHover` #21444A |
| Verde | `forest` #174D38; `primary` #216445; hover #29744E; pressionado #164C35 |
| Perigo | #8F3535; hover #A34040; pressionado #712B2B |
| Texto | principal #F0F5EC; auxiliar #B4C9CA; desabilitado #91A6AA |
| Seleção/foco | dourado #E2C477; foco #F1D58B, borda 2 px; borda normal 1 px |
| Tipografia | Segoe UI; título 30, subtítulo 20, corpo 14, rótulo 13, legenda 12, métrica 28 px; fonte científica em itálico |
| Espaçamentos | 6, 10, 12 e 20 px; padding de navegação 14 px |
| Formas | raio 10 px nos controles e 16 px nos cartões; alturas anteriores preservadas |

Títulos/subtítulos e fonte científica estão preparados para consumo futuro; esta etapa não migra a tipografia de todas as telas. Acentos específicos, perigo, sucesso e assistência conservam significados distintos. A referência conceitual orientou acabamento e paleta, sem importar imagens, ícones, taxonomia ou funcionalidades.

## Estados e acessibilidade implementados

Foco de teclado com borda dourada de 2 px; seleção da navegação também possui indicador lateral. Estados normal, hover, pressionado e desabilitado usam cores próprias. O texto desabilitado mantém opacidade integral. Transições de cor duram 120 ms, sem escala ao pressionar; transição de página permanece 180 ms. A opção existente “Transições suaves” alimenta `Theme.motionEnabled`: ao desligar, Behaviors ficam inativos e durações vão a zero. Não há detecção automática da preferência de movimento do sistema operacional.

O teste Qt `designSystemStatesAndContrast` verifica contraste por luminância sRGB de pelo menos 4,5:1 entre texto/fundos ativos de UiButton nas variantes primária, secundária e perigo (normal/hover/pressionado), 3:1 entre foco e canvas, navegação Tab, ativação Space, bloqueio do clique desabilitado e redução de movimento. Isso não certifica acessibilidade de todas as telas, escalas ou leitores de tela.

Implementação nativa: [Behavior do Qt Quick](https://doc.qt.io/qt-6/qml-qtquick-behavior.html). Veja os [contratos dos componentes](COMPONENTES.md) e a [revisão com capturas](VERSOES/2.1.2/README.md).

## Inventário histórico anterior à revisão

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
