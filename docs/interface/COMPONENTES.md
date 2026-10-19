# Componentes compartilhados — 2.1.2

| Componente | Contrato preservado | Revisão visual |
| --- | --- | --- |
| `UiButton` | Button; `iconName`, `secondary`, `danger`, `compact`, `text`, sinais e dimensões gerais | Fundos verde/petróleo/perigo; foco dourado; transição de cor; desabilitado explícito. A precedência de secondary sobre danger permanece. |
| `NavItem` | Button; `iconName`, `selected`, `text` e clique | Seleção verde-floresta com detalhe dourado, foco de 2 px, hover e descrição acessível de seleção. |
| `StatCard` | Rectangle; `title`, `value`, `iconName`, `accent`, tamanho implícito 200 × 118 | Superfície, borda, texto e espaçamento centralizados; acentos das telas continuam válidos. |
| `TextAction` | Button; texto, fonte ajustável, clique e `textActionBackground` | Fundo discreto para hover/pressão, texto claro, foco dourado e desabilitado legível. |

Importação local: `import "../style" as Style`. Os valores vêm de `Style.Theme`. Não adicionar regras de jogo ou catálogo ao tema. Preservar propriedades públicas e objectName dos testes. Geometria específica de ícones não precisa compartilhar tokens apenas por coincidir numericamente.

Main.qml conecta a opção existente de movimento ao singleton e à transição de página. Cards de exploração e fotografias não foram reformulados. Ao migrar outros componentes em etapa autorizada, conferir estados, teclado, texto longo e janela mínima e arquivar capturas conforme [o procedimento](README.md).
