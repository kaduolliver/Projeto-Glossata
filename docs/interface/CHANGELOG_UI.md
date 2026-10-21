# Histórico visual

## PR #32 — rodada 1 da Subetapa 2.2.2

- Pesquisa apresentada em lotes de 12 por seção, com totais integrais e Carregar mais; consulta sincronizada e debounce sem restauração de textos anteriores. Fotos enciclopédicas proporcionais sem recorte, carregadas conforme a área visível; colunas adaptativas com mínimo de 280 px; breadcrumbs compactos com todos os níveis expansíveis e singular/plural nos contadores.
- Alterações restritas à correção estrutural da [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), sem redesenhar cartões, galeria, cabeçalho, créditos ou fichas. [Registro, capturas e validação da rodada](VERSOES/2.2.2/rodada-1/README.md), com sincronização em [00f2280](https://github.com/kaduolliver/Projeto-Glossata/commit/00f2280990531fab7311e0afe3a420313016a84a) e interface/evidências em [96a25b0](https://github.com/kaduolliver/Projeto-Glossata/commit/96a25b05f4b0c31b29b3366bb8654b263ac9e9a6). Evidências anteriores preservadas; limitação preexistente da Home separada dos testes finais aprovados.

## 2.2.2 — Modernização da tela Explorar

- Busca científica offline, filtros por domínio/nível, resultados distintos para grupos e organismos e destaques com elegibilidade existente. Cartões fotográficos com nomes científicos, autoria/licença e contagens do catálogo; cabeçalho taxonômico, breadcrumbs, avisos e créditos expansíveis. Somente o contexto Explorar recebe a modernização; a seleção de Jogar mantém sua variante e regras.
- Aplicativo 1.8.0, catálogo 2.4.0 preservado. Base após merge confirmado da PR #31. [Implementação, contratos, validação e capturas reais](VERSOES/2.2.2/README.md), com interface/capturas no commit [e4e8bd4](https://github.com/kaduolliver/Projeto-Glossata/commit/e4e8bd4d34167a4fd831dd47fb2dd594047de20d), entregue na [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), vinculada à Issue #22; sem merge ou lançamento.

## 2.2.1-B — Descoberta do Dia e Indicadores Dinâmicos — 2026-10-20

- Refinamento da PR #31: “Identificação por espécie” explicita o alcance do indicador; Foto e créditos abre um diálogo da descoberta, com licença/versão/origem/modificações, avisos e acesso geral. Tooltip da fonte removido, mantendo interação e acessibilidade; ícones dos indicadores em 28/48 px, com padrões anteriores nas outras telas. [Registro e capturas desta revisão](VERSOES/2.2.1-B/refinamento-1/README.md), implementada em [`c6adcb902afb1b75855db1f46fedbc7894f5186b`](https://github.com/kaduolliver/Projeto-Glossata/commit/c6adcb902afb1b75855db1f46fedbc7894f5186b).

- Seção abaixo do carrossel com foto proporcional, nome científico, nome comum disponível, resumo existente, fonte e acesso ao nó da espécie no contexto Explorar. Estado vazio preserva as ações da Home.
- Cartões passam a mostrar organismos únicos, domínios realmente representados e organismos com foto de identificação utilizável no app. `StatCard` oferece descrição opcional e nome acessível; usos anteriores mantêm a aparência.
- Data civil local e ordem estável no C++; verificação periódica a cada 30 segundos e ao reativar o aplicativo. Sem rede obrigatória, novos dados, revisão presumida ou escrita em bancos de jogadores.
- Carrossel da PR #30 e seu diálogo de créditos preservados. [Decisões, verificação e capturas reais](VERSOES/2.2.1-B/README.md), no commit `24d1faaedefa077ec9492cba6dd01071ff6095f9` e no [PR #31](https://github.com/kaduolliver/Projeto-Glossata/pull/31), aberto para revisão e sem merge.

## 2.2.1-A — Início fotográfico — 2026-10-20

- PR #30: `controlsHover` considera apenas a área efetiva dos botões e indicadores; espaços vazios, inclusive entre os pontos, não pausam o avanço. Layout e estilos inalterados.

- PR #30: legenda sincronizada à conclusão do crossfade; hover restrito à legenda e à barra de controles, sem pausa sobre a fotografia ou outras regiões da Home. Sem mudança de estilo. Testes de transição, intervalo real e retomadas ampliados; propostas futuras registradas no [planejamento](README.md#planejamento-das-próximas-subetapas--issue-22).

- PR #30: removida a superfície sólida da legenda. Título do grupo seminegrito e descrição aparecem sobre a fotografia; nome científico em itálico e Créditos compartilham a linha inferior. Degradê estático desaparece acima e nas laterais, preservando imagens, transições e cantos. Capturas nas três resoluções atualizadas.

- PR #30: foco visual em `UiButton`, `TextAction`, `NavItem` e pontos do carrossel passa a usar `visualFocus`. Destaque dourado preservado por teclado, sem aparecer apenas pelo clique; seleção, hover, cores e navegação inalterados. Capturas reais de mouse e teclado registradas.

- Correção no PR #30: quatro slides selecionados pelos identificadores estáveis, inclusive em catálogos locais com grupos históricos homônimos; removida a contagem numérica e alinhado Créditos à direita, com área de foco limitada à largura do botão. Capturas reais atualizadas.

- Complemento FHD no PR #30: foto do leão substituída apenas na Home por outra da mesma espécie em 1920 × 1247; demais slides preservados. Crédito e proveniência independentes, capturas reais atualizadas e fallback anterior mantido.

- Refinamento no PR #30: quatro variantes nativas maiores somente na Home; miniaturas removidas em favor de pontos. Avanço automático de oito segundos, crossfade de 600 ms, pausas por interação/visibilidade e redução de movimento. Legenda compacta com Créditos; informações de atribuição preservadas no diálogo. Capturas e manifesto atualizados na mesma revisão.

- Quatro fotografias existentes, preenchimento proporcional, gradiente, miniaturas, setas e posição; sem avanço automático.
- Explorar abre a exploração científica; Jogar preserva início, retomada e resultados pendentes. Tema e ícones oficiais preservados.
- Créditos, fonte educativa e evidência acessíveis por slide; nenhum estado de governança alterado.
- [Decisões, capturas reais e validação](VERSOES/2.2.1-A/README.md).

## 2.1.3 — Navegação e estrutura — 2026-10-20

- Ajuste no mesmo PR: restaurada a composição anterior do logotipo, com nome em uma linha e assinatura “APRENDA. EXPLORE.”; capturas reais atualizadas, mantendo as demais alterações.

- Barra lateral com seis destinos e tokens do tema; cabeçalho identifica seção e contexto, inclusive Créditos.
- Explorar oferece navegação científica sem iniciar partida; Jogar mantém modos, filtros e conclusão. Breadcrumbs e retornos respeitam o contexto.
- Alternância de seções preserva sessão, dicas, resposta confirmada e resultado pendente. Regressões exercitadas com SQLite temporário.
- [Registro da revisão e capturas reais](VERSOES/2.1.3/README.md), nas três resoluções solicitadas. Versões anteriores preservadas. Implementação em [`9ee73bbf83310d80171daff1e70b01a366eeb62d`](https://github.com/kaduolliver/Projeto-Glossata/commit/9ee73bbf83310d80171daff1e70b01a366eeb62d), [PR #29](https://github.com/kaduolliver/Projeto-Glossata/pull/29).

## 2.1.2 — Design system — 2026-10-19

Implementado no commit [`679f7f744c17e5c9f2f28d52ab32105cf0ed8b51`](https://github.com/kaduolliver/Projeto-Glossata/commit/679f7f744c17e5c9f2f28d52ab32105cf0ed8b51), no [PR #28](https://github.com/kaduolliver/Projeto-Glossata/pull/28).

- Tokens locais Qt/QML e modernização de UiButton, NavItem, StatCard e TextAction: verde-floresta, superfícies em azul-petróleo, seleção/foco dourados, contraste e estados interativos.
- Redução de movimento conectada ao ajuste existente; contratos públicos, navegação e regras preservados.
- [Decisões, capturas e validação](VERSOES/2.1.2/README.md). Tela inicial e registro atualizados no mesmo commit visual. Baseline intacto.

## Baseline anterior à reformulação — 2026-10-19

- Código da interface documentado no commit [`b201e20c2d83fda6d790bc6a96217c0fdc75307e`](https://github.com/kaduolliver/Projeto-Glossata/commit/b201e20c2d83fda6d790bc6a96217c0fdc75307e), após a integração do PR #23.
- Linha de base: [telas, estados e 14 capturas reais](VERSOES/baseline/README.md), com [manifesto de proveniência](VERSOES/baseline/capturas.json).
- Escopo: registro documental da Subetapa 2.1.1 da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). Nenhuma mudança visual ou funcional foi aplicada.
- Documentação entregue no [PR #27](https://github.com/kaduolliver/Projeto-Glossata/pull/27), iniciada no commit `31fd668`; o commit de base acima permite recuperar a interface anterior.

Para cada alteração futura, acrescentar data real, commit e PR, telas afetadas, diferença observável, capturas e método de validação. Manter os registros anteriores sem substituir suas imagens.
