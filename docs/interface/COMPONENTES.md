# Componentes compartilhados

## Início — 2.2.1-A

`HomeView` reutiliza `UiButton`, `TextAction` e `StatCard`, sem alterar seus contratos. Dois buffers `Image` locais cuidam do carrossel; `FeatheredPhoto` permanece nos cartões. `homePlayButton` continua chamando `app.openPlay()`; `homeExploreButton` chama `app.openExplore()`. `homeCarousel.currentIndex` identifica o slide confirmado ao concluir a transição, mantendo título, descrição, nome e créditos anteriores durante o crossfade. Setas circulam, Home/End selecionam extremos e os indicadores de posição aceitam Tab/Espaço. A preferência existente de movimento controla o crossfade de 600 ms e desativa o avanço automático de oito segundos; hover na legenda/controles, botão pressionado, foco de teclado, créditos e visibilidade pausam o Timer. Hover nas demais regiões da Home permite o avanço. Os controles preservam foco visível e o diálogo devolve o foco ao botão de créditos ao fechar.

Os metadados de mídia expostos incluem título, autor, licença, URL, coleta, caminho e hash da evidência. Nenhum componente QML decide permissões ou registra revisão. O estado vazio não oferece créditos fictícios nem desativa as ações de navegação.

Os indicadores visuais usam somente pontos; a posição e o total permanecem no nome acessível de cada ponto. `homeCreditsButton` conserva a largura implícita de `TextAction`, alinhada à direita da legenda; hover e foco não se estendem ao restante do cartão.

`homePhotoCaption` é um layout transparente: título do grupo, descrição existente e linha de nome científico/Créditos. `homeCaptionShade` desenha somente o degradê inferior com transparência vertical e lateral, repintado em redimensionamentos e mudanças de altura da legenda. A camada permanece independente das imagens e do crossfade; os recortes dos cantos continuam sendo aplicados por cima.

## Integração de navegação — 2.1.3

- `Main.qml`: shell central com seis destinos, cabeçalho por seção e contexto, larguras/margens responsivas de `Theme.qml`. Os `NavItem` existentes recebem identificadores estáveis `navHome`, `navExplore`, `navPlay`, `navStatistics`, `navAchievements` e `navSettings`.
- `CategoryView` e `TaxonomyView`: nova propriedade opcional `browsingOnly`, padrão `false`. Ela oculta ações exclusivas de jogo na exploração; não muda os dados nem os contratos existentes. Os objectNames anteriores são mantidos.
- `Theme.qml`: acrescenta tokens de largura lateral, margem de página, legenda de seção e marca. Sem novos recursos gráficos ou dependências.

## Design system — 2.1.2

| Componente | Contrato preservado | Revisão visual |
| --- | --- | --- |
| `UiButton` | Button; `iconName`, `secondary`, `danger`, `compact`, `text`, sinais e dimensões gerais | Fundos verde/petróleo/perigo; foco dourado; transição de cor; desabilitado explícito. A precedência de secondary sobre danger permanece. |
| `NavItem` | Button; `iconName`, `selected`, `text` e clique | Seleção verde-floresta com detalhe dourado, foco de 2 px, hover e descrição acessível de seleção. |
| `StatCard` | Rectangle; `title`, `value`, `iconName`, `accent`, tamanho implícito 200 × 118 | Superfície, borda, texto e espaçamento centralizados; acentos das telas continuam válidos. |
| `TextAction` | Button; texto, fonte ajustável, clique e `textActionBackground` | Fundo discreto para hover/pressão, texto claro, foco dourado e desabilitado legível. |

Importação local: `import "../style" as Style`. Os valores vêm de `Style.Theme`. Não adicionar regras de jogo ou catálogo ao tema. Preservar propriedades públicas e objectName dos testes. Geometria específica de ícones não precisa compartilhar tokens apenas por coincidir numericamente.

Main.qml conecta a opção existente de movimento ao singleton e à transição de página. Cards de exploração e fotografias não foram reformulados. Ao migrar outros componentes em etapa autorizada, conferir estados, teclado, texto longo e janela mínima e arquivar capturas conforme [o procedimento](README.md).
