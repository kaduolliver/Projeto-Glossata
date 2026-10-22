# Design system — revisão 2.1.2

O Quiz 2.3.1 aplica os tokens existentes: superfície petróleo, seleção verde-floresta, foco dourado, tipografia e espaçamentos. Estados animam apenas cor/borda com `durationFast` e `motionEnabled`, sem deslocar alternativas ou fotografias. Cartões usam texto com quebra e altura compartilhada; fotografia integral tem escala máxima 1 apenas na variante do Quiz. Não há novos tokens, ícones ou arquivos fotográficos. [Registro](VERSOES/2.3.1/README.md).

Na seleção Jogar 2.2.3, os painéis Livre/Competitivo usam `surface`, bordas `green/gold`, tipografia e espaçamentos existentes. Seletores usam `UiButton.compact`, agrupamento exclusivo e foco dourado. Os cartões optam pela composição fotográfica moderna compartilhada; a superfície, o hover e a redução de movimento continuam pelos tokens existentes. Não há novos tokens, ícones, arquivos de fotografia ou regras científicas. [Registro](VERSOES/2.2.3/README.md).

Na rodada de usabilidade da PR #32, somente o botão de créditos ganha fundo `surfaceRaised` e borda `green` no hover; foco mantém `focus` e as transições usam os tokens existentes. Cartões compartilham dimensões intrínsecas por seção e controles ficam na base, sem altura textual fixa. Carregamento progressivo usa `UiButton` compacto. A faixa da barra vertical é comum às páginas e a roda convencional em Início/Explorar recebe deslocamento moderado, preservando precisão nativa e movimento reduzido. [Registro atual](VERSOES/2.2.2/usabilidade/README.md).

Na correção de créditos da PR #32, a ação ocupa o texto e padding existentes, alinhada à esquerda, com espaço restante inerte. Os créditos completos passam ao diálogo contextual também para CC BY 2.0/3.0 e registros legados documentados; o aviso de versão não comprovada e o fallback de atribuição específica permanecem visíveis. Hover do cartão é observado passivamente, independente do botão interno; `durationFast`, `motionEnabled`, foco e o componente `TextAction` são preservados. [Registro atual](VERSOES/2.2.2/correcoes-creditos/README.md). Os parágrafos seguintes registram as composições anteriores.

Nos refinamentos finais da PR #32, os cartões passam a valorizar trechos educativos existentes de até duas linhas. Fonte/origem/créditos usam controle discreto e diálogo contextual; atribuições permanecem visíveis quando a política conservadora não permite compactação contextual. Fotografias centrais, integrais e sem ampliação; classificação com rótulos de rank e rodapé científico abaixo dos descendentes. A galeria ganha Anteriores/Próximos e intervalo visível, preservando `motionEnabled` e funcionamento manual. Sem tokens, ícones, dados científicos ou dependências novas. [Registro atual](VERSOES/2.2.2/rodada-final/README.md).

Na rodada 2 da PR #32, Explorar mantém a paleta e os controles existentes: cartões com foto integral de 146/160 px, molduras arredondadas e gradiente nas margens livres; composição horizontal em grupos largos, título de 18 px e atribuição integral de 11 px. Cabeçalho de 26 px e busca se alinham lado a lado a partir de 930 px de conteúdo. Galeria horizontal mantém mínimo de 280 px por cartão, com quatro destaques simultâneos quando há largura. Fontes/classificação e detalhes fotográficos usam expansores, preservando os avisos e a atribuição visíveis. Foco dourado e transições de cor/rolagem respeitam `motionEnabled`; não há avanço automático, tokens ou dependências novas. [Capturas e limites atuais](VERSOES/2.2.2/rodada-2/README.md). Os parágrafos seguintes registram as composições anteriores.

Na rodada 1 da PR #32, o desenho e os tokens são preservados. As correções estruturais calculam colunas com mínimo de 280 px e espaçamento existente, compactam a trilha científica em Explorar e ajustam imagens proporcionalmente sem recorte. O foco fotográfico posiciona o raster inteiro dentro das margens livres. Paginação, controles de expansão e indicação de atualização reutilizam `TextAction` e os estados existentes; nenhum cabeçalho, galeria, crédito ou página de organismo recebe o redesenho reservado à rodada 2. [Capturas e validação](VERSOES/2.2.2/rodada-1/README.md).

A Subetapa 2.2.2 aplica os tokens existentes a Explorar: cabeçalho editorial e campo de busca, filtros com `TaxonComboBox`, cartões fotográficos e cabeçalho taxonômico. Domínios usam três colunas; destaques usam duas/quatro conforme largura; resultados e descendentes usam duas/três. Créditos e textos longos aumentam a altura dos cartões. O conteúdo mantém a rolagem vertical do shell; busca tem nome acessível e borda de foco; cartões usam dourado somente em `visualFocus`. Hover de cor usa `durationFast` e `motionEnabled`, sem escala, carrossel ou animação obrigatória. A lupa é acrescentada ao `UiIcon` existente, com o mesmo traço; não há novos tokens, cores, fotografias ou categorias científicas. [Registro 2.2.2](VERSOES/2.2.2/README.md).

Na revisão 2.2.1-B, a Descoberta do Dia usa os mesmos tokens de superfície, borda, tipografia e espaçamento; título dourado, nome científico em itálico e foto proporcional sem decoração nova. A área empilha foto/texto em larguras abaixo de 700 px de conteúdo. Os indicadores reutilizam `StatCard`, com descrições opcionais e valores reais. A página conserva a rolagem do shell para acomodar a seção após o carrossel. Não há mudança de paleta, ícones ou tokens.

O refinamento da PR #31 aumenta somente os ícones dos três indicadores inferiores para 28 px, com contêiner de 48 px e centro vertical preservado. Os padrões de `StatCard` continuam em 23/42 px nas outras telas. A fonte educativa mantém os estados visuais de `TextAction` e perde apenas o tooltip. Créditos da descoberta usam `Dialog`, `ScrollView`, `TextAction` e `UiButton` existentes, sem reformular o diálogo do carrossel.

Os tokens ficam em `resources/qml/style/Theme.qml`, singleton QML local registrado por `qmldir` e embarcado no QRC. `UiButton`, `NavItem`, `StatCard` e `TextAction` usam os tokens; as demais views conservam seus estilos. Não há dependência nova ou mudança na organização das telas.

Na Subetapa 2.2.1-A, `HomeView` também utiliza esses tokens. A fotografia ocupa a região principal à direita com gradiente; a legenda aparece diretamente sobre um degradê inferior escuro, transparente acima e nas laterais. O título do grupo usa `subtitleSize` e seminegrito; a descrição usa `bodySize`, seguida do nome científico em itálico e de Créditos. Pontos distinguem seleção (verde) e foco por teclado (dourado). O crossfade de 600 ms e o avanço automático de oito segundos usam `homeCrossfadeDuration` e `homeSlideInterval`, com pausa por interação/visibilidade e desativação pela redução de movimento. Os tokens e contratos compartilhados não foram alterados.

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

`UiButton`, `TextAction`, `NavItem` e os pontos do carrossel usam `visualFocus` nos destaques de foco. O clique mantém o foco funcional, sem borda dourada; Tab e Shift+Tab exibem o indicador. `activeFocusOnTab` e as verificações funcionais de foco permanecem preservados.

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
