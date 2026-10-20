# 2.2.1-A — Início fotográfico

Subetapa da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). Base: `599445eb4aef9d7141fbda6dab9957587a58d2c4`, com PR #29 integrado. Esta é uma revisão de interface, não um lançamento do aplicativo. Implementação no commit [`86041114bcb713744ab25d9072b8d14ba2584d84`](https://github.com/kaduolliver/Projeto-Glossata/commit/86041114bcb713744ab25d9072b8d14ba2584d84), entregue no [PR #30](https://github.com/kaduolliver/Projeto-Glossata/pull/30).

## Implementação

O Início apresenta quatro fotografias locais, selecionadas das associações explícitas já verificadas pelo catálogo de mídia. O recorte proporcional respeita o ponto focal existente, com gradiente horizontal e legenda sobre superfície opaca. Setas e miniaturas selecionam as fotos; o indicador informa a posição. Não há avanço automático. Tab/Espaço, setas e Home/End permitem operar por teclado; foco visível e retorno do diálogo de créditos são preservados. A preferência de redução de movimento zera a duração das transições.

Explorar abre a navegação científica; Jogar chama o fluxo existente de início/retomada, inclusive feedback e resultados pendentes. O carrossel não grava estado de sessão. Os três indicadores anteriores permanecem; não há Descoberta do Dia, novo indicador, fotografia importada ou reformulação de outras telas.

`TaxonomyIndex.homeSlides()` só oferece imagens aceitas pelo validador existente e introduções com revisão editorial/especializada utilizável. A legenda é a primeira frase completa da introdução aprovada do grupo, sem reescrever o conteúdo científico. Ausência de evidência, fotografia ou introdução utilizável omite o slide. Mesmo sem slides, os botões de navegação permanecem disponíveis.

Arquivos de produção: `HomeView.qml`, `TaxonomyIndex.h/.cpp`, `GameController.h` e `JsonExplorationMediaCatalog.cpp`. Não foram modificados dados científicos, fotografias, ícones, permissões, revisões, banco de dados ou regras.

## Fotografias, fontes e direitos

| Grupo / representante | Crédito e licença específica | Evidência arquivada | Fonte do trecho educativo existente |
| --- | --- | --- | --- |
| Animalia / *Panthera leo* | [Kevin Pluck — CC BY 2.0](https://commons.wikimedia.org/wiki/File:Lion_waiting_in_Namibia.jpg) | [lion.json](../../../../resources/compliance/exploration/lion.json) | [Jacques et al. (2022)](https://doi.org/10.1007/s12032-022-01740-w) |
| Arthropoda / *Danaus plexippus* | [kcthetc1 — CC0 1.0](https://commons.wikimedia.org/wiki/File:Danaus_plexippus_100029190.jpg) | [monarch.json](../../../../resources/compliance/exploration/monarch.json) | [Klementz et al. (2024)](https://doi.org/10.1093/molbev/msae188) |
| Plantae / *Helianthus annuus* | [S.G.S. — CC0 1.0](https://commons.wikimedia.org/wiki/File:Sunflowers_public_domain_helianthus_annuus_IMG_0815.jpg) | [sunflower.json](../../../../resources/compliance/exploration/sunflower.json) | [Fangel et al. (2012)](https://doi.org/10.3389/fpls.2012.00152) |
| Fungi / *Amanita muscaria* | [Bernard Spragg. NZ from Christchurch, New Zealand — CC0 1.0](https://commons.wikimedia.org/wiki/File:Amanita_muscaria_(1)_(8692325426).jpg) | [amanita.json](../../../../resources/compliance/exploration/amanita.json) | [Challacombe et al. (2019)](https://doi.org/10.1186/s12864-019-6358-x) |

As versões de licença provêm dos snapshots individuais já arquivados; não foram inferidas da licença do site. A interface apresenta autoria, título fornecido, licença com link, fonte específica, modificações, coleta e hash da evidência. Os [créditos completos](../../../PHOTO_CREDITS.md) e o [manifesto de fotografias](../../../../resources/data/exploration_photos.json) continuam sendo as referências de atribuição. As fontes educativas são bibliográficas, não licenças de fotografias.

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

[Diálogo de créditos](capturas/credits-1240x820.png). Versões anteriores preservadas integralmente. Em 1000 × 700, os indicadores inferiores usam a rolagem existente; não ficam inacessíveis. As imagens locais têm até 960 pixels e podem perder nitidez quando ampliadas; nenhuma imagem adicional foi baixada. Validação visual em Windows/Qt software, escala 1; outras GPUs, escalas e leitores de tela não foram ensaiados.

Comandos: `cmake --build build -j 4`, `ctest --test-dir build --output-on-failure`, `qmllint resources/qml/views/HomeView.qml --unqualified disable`, `python scripts/capture_home.py`, `python scripts/audit_catalog.py --strict`, `python scripts/licensing_inventory.py --check-code`, `python scripts/release.py check`, `python scripts/distribution.py check`.

Resultados locais em 20/10/2026:

- Compilação Release concluída. CTest completo: **26/26 suítes aprovadas** (191,89 s). Após os ajustes finais, domínio/aplicação e interface foram reexecutados: **2/2 suítes aprovadas** (68,56 s), com nove casos Qt/QML incluindo inicialização/finalização.
- Após a normalização de finais de linha para o Git, o teste específico do carrossel foi executado novamente: aprovado (7,30 s). Suas 13 capturas finais estão no manifesto; a captura canônica do executável também foi regenerada.
- Verificados seleção circular, miniaturas, teclado, extremos, redução de movimento, diálogo/foco, estado vazio, Explorar/Jogar e retomada sem alteração de foto/alternativas. As regressões existentes cobrem respostas, dicas, resultados pendentes e recuperação.
- Qmllint de todos os arquivos QML, auditoria estrita do catálogo, validação das 109 fotos de exploração, escopo Apache 2.0, integridade de release e higiene do repositório: aprovados.
- A comparação de governança com a base preserva os 1.053 eventos existentes e seus hashes. Os arquivos de dados, fotografias, branding e compliance não têm alterações: 122 organismos e 855 receitas preservados; indicadores continuam em 49 etimologias e 104 morfologias.
- Gate de distribuição continua **bloqueado**: zero erros técnicos, 120 pendências fotográficas, zero pendências de outros materiais e 275 bloqueios científicos. A interface não concede permissões nem resolve esses achados.
