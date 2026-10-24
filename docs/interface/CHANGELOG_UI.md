# Histórico visual

## PR #39 — modo compacto contextual no Quiz

- No Quiz, um cartão compacto visível, com badge 40×44, margens/intervalos internos menores e metadados em duas linhas; fontes de 12/14 px preservadas e altura intrínseca com quebra de texto.
- Fora do Quiz, até dois cartões completos. A segunda instância retida pausa a leitura, continua na FIFO e volta a aparecer quando houver espaço; o primeiro cartão conserva animação e relógio durante a troca de contexto. [Capturas comparativas, testes e limites](VERSOES/2.3.4/quiz-compacto/README.md).

## PR #39 — notificações no topo direito

- Coluna ancorada 20 px abaixo da borda real do cabeçalho, com 20 px à direita e 10 px entre cartões. A primeira notificação fica acima da segunda; remoções reorganizam a FIFO sem recriar o shell.
- Cartões, largura, entrada/saída, leitura, movimento reduzido e suspensão preservados. Testes ampliados para geometria do cabeçalho, controles, redimensionamento e rolagem. Áudio configurável documentado para Perfil e ajustes, sem código ou arquivos sonoros. [Capturas e validação atuais](VERSOES/2.3.4/topo-direito/README.md).

## Subetapa 2.3.4 — notificações de conquistas

- Camada informativa no shell com duas notificações, FIFO, deduplicação por identidade/conquista e cinco segundos de leitura. Badges/metadados existentes, entrada/saída curtas e respeito a movimento reduzido.
- Suspensão segura durante popups, erros críticos e inatividade; expiração explícita de eventos antigos, foco e comandos do Quiz preservados. [Arquitetura, testes e capturas reais](VERSOES/2.3.4/README.md).

## PR #38 — reorganização dos filtros de Conquistas

- Painel com Estado segmentado e menus de Categoria/Nível; rótulos, alvos de 44 px e distribuição conforme a largura útil, sem preencher desnecessariamente a linha.
- Contador efetivo e limpeza conjunta visível apenas com restrições. Seleção AND e conservação em memória preservadas. Menus nativos Qt Quick Controls com foco/teclado/Escape, limites da janela e transições reduzíveis. [Implementação, testes e comparações](VERSOES/2.3.3/barra-filtros/README.md).

## Refinamentos finais da PR #38 — filtro por nível e Platina

- Três filtros combinados com AND; seleção em memória, contador filtrado e estado vazio preservados. Grupo compacto de botões com teclado, mouse e foco/check acessível.
- Platina com silhueta escalonada, facetas profundas, cristais superiores/laterais e reflexos branco-azulados estáticos. Bronze, Prata, Ouro e símbolos anteriores preservados. [Evidências finais](VERSOES/2.3.3/filtros-nivel-platina/README.md).

## Refinamento da PR #38 — níveis de conquistas

- Bronze, Prata, Ouro e Platina identificam a complexidade do requisito, com molduras e ornamentos próprios, texto de nível e identificação acessível. Os 30 glifos e os critérios são preservados; bloqueio atenua o desenho sem ocultar a classe. Não há raridade estatística ou animação contínua.
- [Correções, classificação e evidências reais](VERSOES/2.3.3/refinamentos/README.md).

## Revisão da PR #37 — usuário local único

- A decisão atual substitui a proposta abaixo: Meu perfil, edição do nome e recordes pessoais por UUID. Instalações com múltiplos perfis antigos exigem uma escolha única; não há gerenciamento permanente de perfis nem histórico de outras identidades na interface.
- [Transição, contratos e verificações](VERSOES/usuario-local-unico/README.md). Aplicativo 2.0.0 ainda não lançado; catálogo 2.4.0 preservado. Capturas anteriores conservadas, sem avanço da Subetapa 2.3.3.

## Melhoria independente — Ranking local e histórico de jogadores

- Ranking atual exibe até cinco perfis ativos, com filtro SQL anterior ao limite e ouro/prata/bronze preservados. Histórico apresenta páginas de recordes registrados, nomes originais, situação e UUID, sem pódio. Seletores de contexto e modalidade independentes, com estado em memória e teclado preservados.
- [Diagnóstico, contratos e verificações](VERSOES/ranking-local-historico/README.md). Aplicativo 1.12.0, catálogo 2.4.0 preservado; posterior à PR #36, sem início da 2.3.3 ou lançamento.

## PR #36 — cores e dicas em andamento

- Barras e percentuais taxonômicos recebem cores por ID estável; subgrupos usam variações discretas. Reordenação não muda a associação. Ranking mantém layout e recebe ouro, prata e bronze em bordas/números; foco visível permanece prioritário.
- Dicas na partida incluem a pergunta ainda sem resposta, com categorias por ID e uma contribuição por questão. Confirmação, recuperação, navegação e encerramento recompõem a projeção sem alterar métricas oficiais ou histórico. [Contratos, capturas e verificações](VERSOES/2.3.2/cores-e-dicas/README.md).

## PR #36 — painéis no topo, seleção compacta e rolagem

- Preserva o visual aprovado; taxonomia à esquerda/ranking à direita nas amplas, logo abaixo do cabeçalho. Nas compactas, seletores exclusivos conservam contexto/foco e tornam o ranking acessível sem percorrer a árvore.
- Estatísticas reutiliza o mecanismo de roda de Início/Explorar, com 110 pixels, redução de movimento, caminho nativo de alta precisão e cancelamento pelo arrasto da barra. Nenhum sistema de rolagem paralelo ou contrato C++ novo.
- [Revisão, 72 capturas atuais e validação](VERSOES/2.3.2/refinamentos-topo/README.md). Código/testes/planejamento no commit [`e2678ef`](https://github.com/kaduolliver/Projeto-Glossata/commit/e2678efc8c422f7d8a42dce4f90b013833ef6796); 26 suítes aprovadas na implementação final, nas resoluções/escalas previstas. [Planejamento multiplayer](../architecture/ESTATISTICAS_MULTIPLAYER.md) apenas documental.
- Atualiza exclusivamente a [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36), mantendo abaixo os registros iniciais aprovados; sem merge, release ou avanço de subetapa.

## 2.3.2 — Estatísticas

- [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36), vinculada à Issue #22, sobre a integração da PR #35 (`c6db713`). Código/testes no commit [`c435304`](https://github.com/kaduolliver/Projeto-Glossata/commit/c435304a7243c1bc6396fa4e4d97297fe83426a9).
- Mantém a composição existente e moderniza cartões, dicas, taxonomia e ranking; acrescenta erros confirmados e explicita partida atual, histórico parcial por perfil/modalidade e limites do ranking. Layout adaptável, nomes completos e foco nas posições; backend, dados e regras intactos.
- [Diagnóstico, diferenças, 99 capturas reais e validação](VERSOES/2.3.2/README.md); quatro quadros anteriores preservados. 26 suítes aprovadas, três resoluções e fatores Qt 100%/125%/150%; auditoria de arquivos somente em simulação. Aplicativo 1.11.0 em desenvolvimento, catálogo 2.4.0 preservado; sem merge, release ou avanço de subetapa.

## PR #35 — Descrição educativa independente

- Morfologia retorna à coluna direita entre Nutrição e Conservação, com o ícone, conteúdo, fontes e indicadores originais.
- A região abaixo da fotografia mostra apenas a mensagem de descrição indisponível, sem dados ou fontes emprestados de outros tópicos.
- [Correção, testes e capturas atuais da ficha](VERSOES/2.3.1/descricao-independente/README.md); Quiz, dicas, contadores e estabilidade fotográfica não foram modificados.

## PR #35 — restauração visual e quatro ajustes pontuais

- Restaura a interface anterior, títulos de assistência nas dicas, acertos/erros junto à pontuação e estrutura para descrição futura abaixo da foto na ficha.
- Rolagem independente e colunas limitadas mantêm foto e controles estáveis com zero a três dicas; preserva regras, créditos e acessibilidade.
- [Entrega atual e capturas](VERSOES/2.3.1/restauracao-visual/README.md). A proposta abaixo foi rejeitada pelo mantenedor e não constitui a versão final aprovada.
- Implementação e testes no commit [`4ef3ed0`](https://github.com/kaduolliver/Projeto-Glossata/commit/4ef3ed0); 26 suítes e 48 capturas atuais, preservando evidências anteriores como histórico.

## 2.3.1 — Proposta visual rejeitada (histórico)

- Cabeçalho e progresso por modo, fotografia ampliada na composição, alternativas uniformes com nomes extensos e estados explícitos, ações e pontuação discretas.
- Dicas com impacto antes de revelar, consulta compacta na pergunta e textos/fontes/revisão/relato no painel. Seleção e regras preservadas.
- [PR #35](https://github.com/kaduolliver/Projeto-Glossata/pull/35), implementação e testes no commit [`9ccbd60`](https://github.com/kaduolliver/Projeto-Glossata/commit/9ccbd60), sobre a integração da PR #33 na `main`, vinculada à Issue #22.
- [Capturas reais e validação](VERSOES/2.3.1/README.md); evidências anteriores intactas.

## PR #33 — dificuldade junto ao seletor

- Texto da dificuldade a 20 pixels lógicos do seletor nas janelas amplas; abaixo dele nas compactas, sem centralização artificial. Iniciar partida permanece compacto à direita, com altura do painel e contador preservados.
- [Capturas e testes em 100%, 125% e 150%](VERSOES/2.2.3/alinhamento-dificuldade/README.md), sem alterar funcionalidades ou regras.

## PR #33 — alinhamento do início da partida

- Coluna central flexível e botão Iniciar partida à margem interna direita nas janelas amplas, sem esticar o botão; composição compacta e contador abaixo preservados.
- [Capturas e testes nas três resoluções](VERSOES/2.2.3/alinhamento-iniciar/README.md). Sem mudança de regras, dificuldade, disponibilidade ou navegação.

## PR #33 — configuração compacta e fontes contextuais

- Controles do Livre em uma linha nas janelas amplas e duas nas compactas, campo com largura limitada e altura do painel derivada do conteúdo.
- Um único botão de fontes na configuração, com referências, URLs, origem e revisão no diálogo local; avisos científicos permanecem na página.
- Abertura e mudança de grupo posicionadas no topo para expor integralmente o botão Voltar, sem alterar o contexto salvo da seleção.
- [Capturas e testes desta revisão](VERSOES/2.2.3/configuracao-compacta/README.md); evidências anteriores preservadas.

## PR #33 — refinamento da configuração e retorno

- Painéis Livre/Competitivo somente na tela principal; grupo com resumo, configuração direta, dificuldade preservada e início separado antes dos descendentes.
- Aba, rolagem e origem do foco mantidas em memória, compartilhando o mecanismo de restauração da enciclopédia. Retorno por mouse conserva a rolagem sem impor foco de teclado.
- [Capturas e validação desta rodada](VERSOES/2.2.3/refinamentos-navegacao/README.md), na mesma [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33). Evidências anteriores preservadas.

## 2.2.3 — Modernização da tela Jogar — PR #33

- Seleção fotográfica com Reinos em destaque, Domínios e Grupos avançados, preservando IDs e todos os caminhos taxonômicos. Cartões com alturas compartilhadas, fotografias integrais, foco e créditos contextuais existentes.
- Livre e Competitivo em painéis próprios, dificuldade do Livre, configuração de identificação e jogo misto preservados. Contagem catalogada separada da disponibilidade por nível, com estado sem partida para grupos vazios/limitados. Nenhuma mudança no Quiz ou nas regras de partidas.
- Correção localizada do alinhamento dos textos de créditos e ação principal em Explorar. [Capturas, contratos e verificações](VERSOES/2.2.3/README.md), entregues na [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33), vinculada à Issue #22. Evidências anteriores preservadas.

## PR #32 — uniformidade, faixa da barra e rolagem

- Hover próprio dos créditos, cartões de altura compartilhada por seção, ações inferiores alinhadas e justificativa extensa de foto ausente no diálogo. Atribuições obrigatórias permanecem integrais, sem teto fixo para o texto.
- Carregamento com `UiButton` compacto; Loader de páginas respeita faixa constante junto à barra. Roda convencional percorre 110 pixels por incremento em Início/Explorar, com aceitação exclusiva, redução de movimento e interrupção ao mudar foco/página ou interagir com a barra. Eventos contínuos e galeria horizontal continuam nativos.
- [Capturas, escalas e validação](VERSOES/2.2.2/usabilidade/README.md), com implementação/teste/recaptura da Home em [e897712](https://github.com/kaduolliver/Projeto-Glossata/commit/e897712a6eba59b32d220feab92ea6b3cb201791): 26 suítes aprovadas e 30 capturas nas três resoluções, incluindo fatores Qt 125%/150%. As evidências anteriores permanecem históricas.

## PR #32 — correções de créditos e interação

- Atribuição contextual também para CC BY 2.0/3.0 e créditos legados documentados, com aviso visível de versão não comprovada e fallback para pedidos específicos. Política e evidências individuais registradas sem alterar licenças/permissões.
- Ação de créditos com largura do texto e padding, alinhada à esquerda; espaços vazios da linha não abrem créditos nem navegam. Hover do cartão observado de forma passiva, independente do hover/foco do botão de créditos, com as transições e a redução de movimento existentes.
- [Registro, capturas e validação desta correção](VERSOES/2.2.2/correcoes-creditos/README.md). Refinamentos e evidências anteriores preservados.

## PR #32 — refinamentos finais da Subetapa 2.2.2

- Cartões educativos com trechos de notas existentes elegíveis e Fonte, origem e créditos independente da abertura do táxon. Diálogo contextual completo, com atribuição junto à imagem nos casos conservadores; histórico de modificações separado da apresentação atual, sem apagar evidências.
- Fotos centralizadas sem recorte/ampliação; classificação resumida com níveis documentados; rodapé de classificação/fontes/proveniência após descendentes/estado vazio. Galeria manual com indicação de posição/cartões adicionais. Pesquisa e estado de navegação preservados.
- [Registro final e capturas](VERSOES/2.2.2/rodada-final/README.md), na mesma [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32): projeções em [88a9421](https://github.com/kaduolliver/Projeto-Glossata/commit/88a94210bf2a1a7aad44587489059fdc5d3d1e07) e interface/evidências em [4014bfa](https://github.com/kaduolliver/Projeto-Glossata/commit/4014bfa9263819cd8a47c2bdc7a8eb4416c5d739). Rodadas anteriores históricas. Criação/revisão de descrições completas registrada para a Etapa 3.

## PR #32 — rodada 2 da Subetapa 2.2.2

- Cartões enciclopédicos compactos, fotografias integrais com molduras discretas e grupos largos em composição horizontal. Galeria de quatro destaques existentes com controles/setas e foco acessíveis, cabeçalho com pesquisa integrada, filtros compactos e limpeza só do texto.
- Grupos e organismos têm identificação própria, resumo/introdução existentes em destaque e classificação/fontes/proveniência em seções distintas. Atribuições e avisos científicos preservados. Retorno à pesquisa recupera lotes, foco e rolagem; novas consultas reiniciam a paginação. Contratos científicos e telas fora de Explorar permanecem intactos.
- [Registro e capturas desta rodada](VERSOES/2.2.2/rodada-2/README.md), na mesma [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), em [c333578](https://github.com/kaduolliver/Projeto-Glossata/commit/c333578835ee97df0b5d559fcca6d749c3518293). Evidências anteriores preservadas; aviso preexistente da Home continua documentado separadamente.

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
# Subetapa 2.3.3 — Conquistas

Coleção persistente de 30 conquistas, resumo dinâmico, filtros combinados, 30 badges vetoriais nativos, progresso por cartão e detalhes em diálogo integrado. Estados textuais, foco por teclado, rolagem compartilhada e redução de movimento. [Arquitetura e critérios](../CONQUISTAS.md); [verificações e capturas](VERSOES/2.3.3/README.md). Sem notificações da etapa 2.3.4.
