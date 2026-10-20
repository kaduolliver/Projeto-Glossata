# 2.2.1-A — Início fotográfico

Subetapa da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). Base: `599445eb4aef9d7141fbda6dab9957587a58d2c4`, com PR #29 integrado. Esta é uma revisão de interface, não um lançamento do aplicativo. Implementação inicial no commit [`86041114bcb713744ab25d9072b8d14ba2584d84`](https://github.com/kaduolliver/Projeto-Glossata/commit/86041114bcb713744ab25d9072b8d14ba2584d84), entregue no [PR #30](https://github.com/kaduolliver/Projeto-Glossata/pull/30).

## Implementação

O Início apresenta quatro fotografias locais, selecionadas das associações explícitas já verificadas pelo catálogo de mídia. O recorte proporcional respeita o ponto focal existente, com gradiente horizontal e legenda diretamente sobre um degradê inferior suave. Setas e pequenos pontos selecionam as fotos; não há faixa de miniaturas. O avanço automático ocorre a cada oito segundos, com crossfade de 600 ms e retorno ao primeiro slide. A navegação manual reinicia o intervalo. Hover na legenda/barra de controles, botão pressionado, foco de teclado no carrossel, créditos abertos, janela inativa/oculta/minimizada e Home invisível suspendem o avanço; ao retomar começa um intervalo completo. Hover nas demais regiões da Home não pausa. Tab/Espaço, setas e Home/End permitem operar por teclado; foco visível e retorno do diálogo de créditos são preservados. A preferência de redução de movimento zera a duração das transições e desativa o avanço automático. `Theme.homeSlideInterval` e `Theme.homeCrossfadeDuration` centralizam os tempos. A legenda e os créditos passam ao próximo organismo somente quando o crossfade termina.

Explorar abre a navegação científica; Jogar chama o fluxo existente de início/retomada, inclusive feedback e resultados pendentes. O carrossel não grava estado de sessão. Os três indicadores anteriores permanecem; não há Descoberta do Dia, novo indicador, nova espécie ou reformulação de outras telas.

`TaxonomyIndex.homeSlides()` só oferece imagens aceitas pelo validador existente e introduções com revisão editorial/especializada utilizável. A legenda é a primeira frase completa da introdução aprovada do grupo, sem reescrever o conteúdo científico. Ausência de evidência, fotografia ou introdução utilizável omite o slide. Mesmo sem slides, os botões de navegação permanecem disponíveis.

A seleção usa os identificadores estáveis dos quatro grupos, preservando os homônimos históricos no SQLite. A posição é apresentada apenas por pontos; seus nomes acessíveis informam posição e total. Créditos fica à direita da legenda, com largura natural do texto e padding do componente, inclusive no hover e no foco.

Arquivos de produção do refinamento: `HomeView.qml`, `Theme.qml`, `TaxonomyIndex.cpp` e `JsonExplorationMediaCatalog.cpp`. Automação: `exploration_media.py` e `exploration_discovery.py`; recursos QRC, créditos, inventário de direitos e manifesto do catálogo regenerados. Dados científicos, imagens dos cartões/quiz, ícones, decisões humanas, banco e regras permanecem preservados.

## Fotografias, fontes e direitos

| Grupo / representante | Crédito e licença específica | Evidência arquivada | Fonte do trecho educativo existente |
| --- | --- | --- | --- |
| Animalia / *Panthera leo* | [Ron Knight from Seaford, East Sussex, United Kingdom — CC BY 2.0](https://commons.wikimedia.org/wiki/File:Lion_(Panthera_leo)_(8603270337).jpg) | [home-lion-fhd.json](../../../../resources/compliance/exploration/home-lion-fhd.json) | [Jacques et al. (2022)](https://doi.org/10.1007/s12032-022-01740-w) |
| Arthropoda / *Danaus plexippus* | [kcthetc1 — CC0 1.0](https://commons.wikimedia.org/wiki/File:Danaus_plexippus_100029190.jpg) | [monarch.json](../../../../resources/compliance/exploration/monarch.json) | [Klementz et al. (2024)](https://doi.org/10.1093/molbev/msae188) |
| Plantae / *Helianthus annuus* | [S.G.S. — CC0 1.0](https://commons.wikimedia.org/wiki/File:Sunflowers_public_domain_helianthus_annuus_IMG_0815.jpg) | [sunflower.json](../../../../resources/compliance/exploration/sunflower.json) | [Fangel et al. (2012)](https://doi.org/10.3389/fpls.2012.00152) |
| Fungi / *Amanita muscaria* | [Bernard Spragg. NZ from Christchurch, New Zealand — CC0 1.0](https://commons.wikimedia.org/wiki/File:Amanita_muscaria_(1)_(8692325426).jpg) | [amanita.json](../../../../resources/compliance/exploration/amanita.json) | [Challacombe et al. (2019)](https://doi.org/10.1186/s12864-019-6358-x) |

As versões de licença foram reconferidas nos metadados específicos dos quatro arquivos, com novos snapshots arquivados para as variantes; não foram inferidas da licença do site. A legenda exibe nome científico, trecho educativo e o botão Créditos. O diálogo conserva autoria, título fornecido, licença com link, fonte específica, modificações, coleta e hash da evidência. Os [créditos completos](../../../PHOTO_CREDITS.md) e o [manifesto de fotografias](../../../../resources/data/exploration_photos.json) continuam sendo as referências de atribuição. As fontes educativas são bibliográficas, não licenças de fotografias.

As fotos mantêm identificação fornecida pela fonte; esta implementação não declara nova revisão humana, validação especializada ou endosso. As legendas descrevem grupos, não certificam todas as características do indivíduo fotografado. Nenhuma autorização foi ampliada.

Nas capturas, há redimensionamento, recorte e gradiente feitos durante a renderização. Os arquivos fotográficos originais permanecem intactos. Capturar a interface não confirma autorização para redistribuir todos os elementos nela presentes. O código Apache 2.0 não relicencia fotografias, textos nem identidade visual. Os bloqueios do pacote continuam independentes.

## Capturas e reprodução

As capturas reais e seus hashes são relacionados em `capturas.json`, com fontes, binário, horário e método. A captura oficial do README usa `python scripts/capture_home.py`. As demais são produzidas pela janela Qt real no teste `homeCarouselPhotographsAndNavigation`, com banco temporário e Discord desabilitado; não são imagens de conceito.

| Slide | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Leão | [Captura](capturas/slide-0-1000x700.png) | [Captura](capturas/slide-0-1240x820.png) | [Captura](capturas/slide-0-1920x1080.png) |
| Monarca | [Captura](capturas/slide-1-1000x700.png) | [Captura](capturas/slide-1-1240x820.png) | [Captura](capturas/slide-1-1920x1080.png) |
| Girassol | [Captura](capturas/slide-2-1000x700.png) | [Captura](capturas/slide-2-1240x820.png) | [Captura](capturas/slide-2-1920x1080.png) |
| Cogumelo | [Captura](capturas/slide-3-1000x700.png) | [Captura](capturas/slide-3-1240x820.png) | [Captura](capturas/slide-3-1920x1080.png) |

[Diálogo de créditos](capturas/credits-1240x820.png). Versões anteriores preservadas integralmente. Em 1000 × 700, os indicadores inferiores usam a rolagem existente; não ficam inacessíveis. As variantes maiores são exclusivas da Home; a tabela abaixo registra as dimensões efetivas. A antiga foto do leão era limitada a 1280 × 960; foi substituída na Home por outra foto de Panthera leo, em 1920 × 1247, sem ampliação artificial. Validação visual em Windows/Qt software, escala 1; outras GPUs, escalas e leitores de tela não foram ensaiados.

Comandos: `cmake --build build -j 4`, `ctest --test-dir build --output-on-failure`, `qmllint resources/qml/views/HomeView.qml --unqualified disable`, `python scripts/capture_home.py`, `python scripts/audit_catalog.py --strict`, `python scripts/licensing_inventory.py --check-code`, `python scripts/release.py check`, `python scripts/distribution.py check`.

Resultados da implementação inicial em 20/10/2026, antes deste refinamento (registro histórico; não representam a nova execução):

- Compilação Release concluída. CTest completo: **26/26 suítes aprovadas** (191,89 s). Após os ajustes finais, domínio/aplicação e interface foram reexecutados: **2/2 suítes aprovadas** (68,56 s), com nove casos Qt/QML incluindo inicialização/finalização.
- Após a normalização de finais de linha para o Git, o teste específico do carrossel foi executado novamente: aprovado (7,30 s). Suas 13 capturas finais estão no manifesto; a captura canônica do executável também foi regenerada.
- Verificados seleção circular, miniaturas, teclado, extremos, redução de movimento, diálogo/foco, estado vazio, Explorar/Jogar e retomada sem alteração de foto/alternativas. As regressões existentes cobrem respostas, dicas, resultados pendentes e recuperação.
- Qmllint de todos os arquivos QML, auditoria estrita do catálogo, validação das 109 fotos de exploração, escopo Apache 2.0, integridade de release e higiene do repositório: aprovados.
- A comparação de governança com a base preserva os 1.053 eventos existentes e seus hashes. Os arquivos de dados, fotografias, branding e compliance não têm alterações: 122 organismos e 855 receitas preservados; indicadores continuam em 49 etimologias e 104 morfologias.
- Gate de distribuição continua **bloqueado**: zero erros técnicos, 120 pendências fotográficas, zero pendências de outros materiais e 275 bloqueios científicos. A interface não concede permissões nem resolve esses achados.


## Refinamento do PR #30

Código, recursos e capturas finais no commit [`c3d70f06709a460d23d53212846992fcf3658a79`](https://github.com/kaduolliver/Projeto-Glossata/commit/c3d70f06709a460d23d53212846992fcf3658a79).

As imagens anteriores dos cartões e suas evidências foram preservadas byte a byte. `home_variants` associa explicitamente cada foto-base à variante do mesmo arquivo-fonte. O validador exige identificação, autoria, licença/versão, página e SHA-1 original coincidentes, além de hash local, evidência e resolução nativa comprovada. Variante inválida não substitui a foto-base. A descoberta de cartões ignora recursos com `usage: home`.

| Organismo | Resolução real da Home | Licença comprovada | Evidência da nova coleta |
| --- | --- | --- | --- |
| *Panthera leo* | 1920 × 1247 | CC BY 2.0 | [home-lion-fhd.json](../../../../resources/compliance/exploration/home-lion-fhd.json) |
| *Danaus plexippus* | 2048 × 2021 | CC0 1.0 | [home-monarch.json](../../../../resources/compliance/exploration/home-monarch.json) |
| *Helianthus annuus* | 3024 × 3024 | CC0 1.0 | [home-sunflower.json](../../../../resources/compliance/exploration/home-sunflower.json) |
| *Amanita muscaria* | 3000 × 2129 | CC0 1.0 | [home-amanita.json](../../../../resources/compliance/exploration/home-amanita.json) |

A seleção ativa soma 7.920.358 bytes: três originais e a imagem do leão reduzida pela própria fonte de 5184 × 3367 para 1920 × 1247. Nenhum detalhe foi criado por ampliação. O arquivo anterior de 1280 × 960 permanece disponível como fallback rastreável. Todos passaram pela checagem existente de metadados sensíveis. Foram aproveitadas as mesmas identificações e referências, sem nova aprovação humana. A concessão pública por arquivo é registrada pelo mecanismo já existente; não elimina outros bloqueios. O catálogo 2.4.0 permanece não lançado, com identidade de conteúdo regenerada; aplicativo 1.7.0 inalterado.

Dois buffers nativos `Image` mantêm a foto atual enquanto carregam a próxima. O recorte usa a proporção e o ponto focal, com um gradiente independente das imagens. Canvas é utilizado apenas para os recortes dos cantos; os cartões continuam usando seu componente anterior sem alterações. A renderização por Canvas das fotografias foi substituída após reproduzir invalidação de textura em um buffer pré-carregado invisível no renderizador software. O crossfade só começa após o carregamento do buffer de entrada; uma falha mantém a fotografia anterior e libera a navegação. A decodificação é limitada a 1920 pixels por lado, sem ampliar arquivos menores, e o cache dos buffers é desativado; não se mantêm as quatro imagens decodificadas pelo carrossel. O limite de 3072 pixels/5 MB por arquivo aplica-se somente às variantes da Home; os cartões conservam o orçamento anterior de 1024 pixels. Links externos continuam restritos a ações explícitas do usuário.

Validação do refinamento anterior à troca FHD do leão em 20/10/2026 (registro histórico):

- Compilação Release concluída; **26/26 suítes CTest aprovadas** (196,79 s). Suíte Qt/QML: **10 casos aprovados**, sem avisos, incluindo inicialização/finalização (57,39 s).
- O teste aguarda um intervalo real de oito segundos, verifica os dois buffers simultaneamente durante o crossfade de 600 ms, o reinício após ação manual, hover, foco de teclado, créditos, janela inativa/oculta, redução de movimento e saída da Home. Intervalos acelerados são usados somente nas verificações posteriores de pausa/reinício. Testado também retorno ao primeiro slide.
- As 12 capturas de slides são verificadas por amostragem de pixels reais para rejeitar um quadro vazio, além da prontidão de carregamento. Há 13 capturas finais: quatro slides nas três resoluções solicitadas e diálogo de créditos. Captura oficial regenerada pelo executável real; manifesto atualizado com PNGs, fontes, binários, horários e dados de cada mídia.
- **21 testes de mídia aprovados**: licença, autoria, evidências, resolução nativa, rejeição de ampliação oferecida pelo servidor, vínculo ao mesmo arquivo-fonte e isolamento dos cartões. Auditoria atual: 114 entradas de mídia, incluindo a nova substituta do leão; apenas quatro são selecionadas na Home. Não há organismos adicionados.
- Qmllint de todos os QML, auditoria estrita do catálogo, Apache 2.0, integridade de manifestos e higiene do repositório aprovados. Permanecem 122 organismos, 855 receitas, 49 etimologias e 104 morfologias verificadas.
- Comparação com `5eb1fc94be8f7570fe188c85101ce9e16128aa7b`: os 1.053 eventos, todos os 109 registros/arquivos anteriores e os vínculos dos cartões são preservados. Nenhuma alteração de decisões humanas, textos científicos, ícones, regras ou persistência.
- Distribuição **bloqueada** por 120 pendências fotográficas e 275 bloqueios científicos; zero erros técnicos e zero pendências de outros materiais. Não foi publicado pacote ou release.

O histórico inicial permanece no Git, e as versões baseline/2.1.2/2.1.3 não são modificadas. Renderização avaliada em Qt software/offscreen, escala 1; não há medição de FPS ou garantia sobre outras GPUs/escalas/leitores de tela.


## Complemento FHD solicitado pelo mantenedor

Substituição e capturas no commit [`7f11c1a9c0c72fb0557512bfb35c3e4a6c37ede8`](https://github.com/kaduolliver/Projeto-Glossata/commit/7f11c1a9c0c72fb0557512bfb35c3e4a6c37ede8).

Entre as quatro fotografias ativas, somente o leão estava abaixo de 1920 × 1080. Foi preparada outra imagem da mesma espécie, [Panthera leo em Etosha, por Ron Knight](https://commons.wikimedia.org/wiki/File:Lion_(Panthera_leo)_(8603270337).jpg), com CC BY 2.0 comprovada no arquivo específico. A fonte identifica a espécie no título e na categoria `Panthera leo of Etosha National Park`, arquivada pelo coletor; isso não constitui revisão por especialista. A imagem passou pela inspeção visual técnica e pela checagem de metadados sensíveis.

`home_replacements` registra esta seleção separadamente de `home_variants`: não é apresentada como outro tamanho da fotografia de Kevin Pluck. A nova imagem exige evidência própria, identidade coincidente com o organismo 126 e categoria identificadora da fonte, além de pelo menos 1920 × 1080 pixels. A permissão e a identificação da foto antiga não são transferidas. O validador continua exigindo o mesmo arquivo-fonte para variantes de resolução. Em caso de falha, mantém o fallback validado; não libera uma imagem sem licença.

As fotos dos cartões, os vínculos taxonômicos, os organismos, as receitas, os textos e os históricos permanecem intactos. As outras três imagens ativas já superavam FHD e foram mantidas. Crédito, snapshot, hash, QRC, inventário, catálogo e capturas são atualizados pelos mecanismos existentes. Validação final deste complemento: Release compilada; 26/26 suítes CTest aprovadas (195,06 s), dez casos Qt/QML aprovados (57,49 s) e 22 testes de mídia aprovados. A suíte exige pelo menos 1920 × 1080 para as quatro imagens ativas e verifica a rejeição de outra espécie ou evidência identificadora ausente. As 13 capturas reais e o registro canônico do README foram regenerados. Auditorias de catálogo, Apache, governança, mídia, metadados e integridade aprovadas. O gate permanece bloqueado pelas mesmas 120 pendências fotográficas e 275 científicas, com zero erros técnicos.

## Correção de contagem e controles do PR #30

Correções, testes e capturas no commit [`c823bd775a3cfb141cd221859d668af89eca07c9`](https://github.com/kaduolliver/Projeto-Glossata/commit/c823bd775a3cfb141cd221859d668af89eca07c9).

Em 20/10/2026, uma consulta somente leitura ao catálogo local confirmou os registros históricos `edu:Animalia`, `edu:Plantae` e `edu:Fungi`, além dos grupos científicos atuais com os mesmos nomes. A seleção por nome em um `QHash` podia encontrar primeiro um registro antigo, sem introdução/foto utilizável, e interromper a busca. Isso explica a contagem variável entre o catálogo recém-criado das capturas e instalações existentes. A seleção agora usa os quatro IDs científicos explícitos; as exigências de fotografia e revisão continuam iguais. Nenhum registro antigo é removido ou migrado.

Removido o rótulo numérico de posição; permanecem quatro pontos e a informação equivalente para tecnologias assistivas. `homeCreditsButton` deixa de preencher a largura da legenda, usando a largura implícita de `TextAction` e alinhamento à direita. O diálogo e suas atribuições permanecem intactos.

O teste de regressão insere os três homônimos em um SQLite temporário e verifica, em três reaberturas, quatro slides idênticos aos do catálogo novo, preservando os registros históricos e os 122 organismos. Os testes QML verificam quatro pontos, ausência do contador e largura/alinhamento de Créditos em 1000 × 700, 1240 × 820 e 1920 × 1080. As 13 capturas e a imagem oficial do README foram regeneradas com o aplicativo real. Como o executável habitual estava aberto, a compilação Release usou a saída isolada `build/pr30-verified`, sem encerrar o jogo nem alterar dados dos jogadores.

Validação desta correção: **26/26 suítes CTest aprovadas** (217,01 s); domínio/aplicação com 53 casos e interface Qt/QML com dez casos aprovados. Qmllint, auditoria estrita do catálogo, 114 registros de mídia, escopo Apache, manifestos e hashes das capturas aprovados. Os 1.053 eventos de governança foram preservados byte a byte, assim como dados científicos, fotografias e evidências. A distribuição continua bloqueada por 120 pendências fotográficas e 275 bloqueios científicos, com zero erros técnicos e zero pendências de outros materiais. Não foi publicado pacote nem release.

## Foco por mouse e teclado

Implementação, testes e capturas no commit [`faeeae89527de29dfa28952d090588d0f8ed0d8d`](https://github.com/kaduolliver/Projeto-Glossata/commit/faeeae89527de29dfa28952d090588d0f8ed0d8d).

As verificações de destaque visual nos três botões compartilhados e nos pontos do carrossel usam `visualFocus`. As verificações funcionais de foco, pausa do carrossel e navegação permanecem intactas. Tab e Shift+Tab preservam o dourado; clicar não o ativa. Nenhuma cor ou outro estado visual foi alterado.

Regressões verificam clique e funcionamento dos botões, ausência da borda pelo mouse e retorno da borda por Tab/Shift+Tab. [Mouse](capturas/mouse-focus-1240x820.png) e [teclado](capturas/keyboard-focus-1240x820.png) têm capturas reais adicionais. A imagem oficial registra a interação com Créditos no mesmo `Main.qml`/controlador, executado pelo QtTest com SQLite temporário; o manifesto identifica esse binário e a fonte do quadro. A captura inicial pelo executável de produção também foi refeita com `scripts/capture_home.py`: o estado em repouso conserva a mesma aparência.

Release recompilada; domínio/aplicação aprovado com 53 casos e suíte Qt/QML final aprovada com dez casos (77,51 s), sem avisos. Verificados estados de foco dos quatro controles, ações, navegação, carrossel e sessões. Qmllint, auditoria estrita do catálogo, escopo Apache, manifestos e histórico preservados. O gate de distribuição continua bloqueado por pendências existentes, com zero erros técnicos. Nenhum catálogo, fotografia, regra ou registro de decisão foi alterado.

## Legenda integrada à fotografia

Implementação, testes e capturas no commit [`6cb4cdc076023a3730d5b5f2d3adfe6b1b4d590d`](https://github.com/kaduolliver/Projeto-Glossata/commit/6cb4cdc076023a3730d5b5f2d3adfe6b1b4d590d).

A superfície retangular foi removida. No canto inferior direito, o nome do grupo existente passa a ser o título educativo, em seminegrito de 20 px; a descrição mantém exatamente o trecho científico aprovado, em 14 px. Nome científico em itálico e Créditos compartilham a linha inferior. O diálogo de autoria, licenciamento e fontes permanece intacto.

Uma camada Canvas independente desenha o degradê inferior difuso, com queda vertical e lateral, sem modificar fotografias ou os buffers do crossfade. A pintura ocorre em alterações de tamanho, inclusive reflow da legenda; não há animação ou processamento das fotos nessa camada. Os recortes dos cantos permanecem preservados. A largura proporcional e as margens mantêm a legenda separada dos botões Explorar/Jogar.

Os testes exercitam os quatro slides nas três resoluções, conferindo ordem dos textos, itálico, seminegrito, ausência de cortes e sobreposições, transparência do degradê e contraste sobre um pixel fotográfico branco. Esta verificação de contraste usa o degradê realmente renderizado: pelo menos 3:1 para o título e 4,5:1 para descrição, nome e Créditos. As capturas reais atualizadas incluem os estados de mouse e teclado. A captura oficial desta revisão volta ao estado inicial do executável de produção, por `scripts/capture_home.py`; o manifesto identifica separadamente as capturas produzidas pelo QtTest. Não houve alterações de catálogo, fotografias, licenças ou funcionalidades externas à legenda.

Validação final: compilação Release concluída; dez casos Qt/QML aprovados (78,09 s), sem avisos, incluindo carrossel, créditos, teclado e regressões de navegação/sessões. Qmllint de todos os QML, auditoria estrita do catálogo, escopo Apache, integridade dos manifestos/capturas e preservação dos 1.053 eventos de governança aprovados. As 15 capturas e a imagem oficial foram regeneradas; quadros das três resoluções foram inspecionados visualmente. O gate de distribuição permanece bloqueado pelas pendências existentes, com zero erros técnicos. Outras GPUs, escalas e leitores de tela não foram ensaiados nesta intervenção.

## Sincronização e pausa contextual

`beginTransition()` inicia apenas a mistura das fotografias. `finishTransition()` confirma o buffer e o índice da nova foto, atualizando juntos título, descrição, nome científico e créditos. A legenda anterior permanece durante os 600 ms, sem antecipar dados da próxima fotografia. Redução de movimento confirma imediatamente o destino, como anteriormente. Estilos, posições e textos permanecem iguais.

O handler de hover global foi removido: a legenda e a barra de controles têm handlers próprios. Teclado, controles pressionados, diálogo aberto, janela inativa/oculta e Home invisível conservam suas proteções; a retomada reinicia oito segundos completos. Hover sobre a fotografia, apresentação ou ações Explorar/Jogar não impede o avanço automático.

As capturas [durante a transição](capturas/transition-in-progress-1240x820.png) e [ao concluí-la](capturas/transition-complete-1240x820.png) registram a correspondência entre foto e legenda. A captura oficial mostra o estado real após interação por mouse com Créditos, com quadro e binário identificados no JSON; a captura padrão do executável também foi refeita. Os três refinamentos futuros foram apenas registrados no [planejamento](../../README.md#planejamento-das-próximas-subetapas--issue-22).

Validação final: Release recompilada; dez casos Qt/QML aprovados (90,01 s), sem avisos. Verificados metadados antes/durante/depois da mistura, dois intervalos reais de oito segundos (incluindo a retomada), hover genérico sem pausa, hover na legenda/controles com pausa, reinício manual, teclado, créditos, janela inativa, redução de movimento e regressões de navegação/sessões. Qmllint, catálogo estrito, Apache, manifestos, hashes das capturas e preservação dos 1.053 eventos aprovados. As 17 capturas têm os métodos de movimento diferenciados no manifesto. O gate de distribuição permanece bloqueado por pendências existentes, com zero erros técnicos.
