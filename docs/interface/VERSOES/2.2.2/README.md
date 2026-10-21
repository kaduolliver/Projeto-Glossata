# Subetapa 2.2.2 — Modernização da tela Explorar

Revisão atual da PR #32: [rodada 2 — composição fotográfica e retorno aos resultados](rodada-2/README.md), após a [rodada 1 — escalabilidade, sincronização, enquadramento e navegação](rodada-1/README.md). As capturas, os logs e os contratos abaixo registram a entrega inicial e continuam preservados como histórico.

Base: `9d047910ddb6b56051d1830cba4052ca03a4e520`, merge da [PR #31](https://github.com/kaduolliver/Projeto-Glossata/pull/31) na `main`, confirmado pela API do GitHub antes de qualquer alteração de código. A `main` local foi atualizada por fast-forward e a implementação começou na branch `feat/issue-22-explore-2.2.2`. Entregue na [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), vinculada à [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22), sem merge ou lançamento. Busca/seleção e testes C++ em [7ded42a](https://github.com/kaduolliver/Projeto-Glossata/commit/7ded42a); interface, testes, versão e capturas em [e4e8bd4](https://github.com/kaduolliver/Projeto-Glossata/commit/e4e8bd4d34167a4fd831dd47fb2dd594047de20d).

## Composição e escopo

A referência fornecida `Glossata_Referencia_Visual_Etapa2.png` foi inspecionada; somente a seção **2. TELA EXPLORAR (ENCICLOPÉDIA)** orienta esta composição. Fotos em destaque, cabeçalho com busca, hierarquia científica e cartões de organismos são adaptados ao shell e aos tokens existentes. Nenhuma foto, contagem, classificação ou informação científica foi importada da referência.

`CategoryView`, somente em `browsingOnly`, apresenta busca local, domínio, nível, três portas de domínio e até quatro organismos em destaque. Animalia, Plantae e Fungi continuam atalhos dentro de suas linhagens, sem promoção a domínios. Archaea continua navegável com zero organismos. Os destaques usam os mesmos critérios de elegibilidade da Descoberta do Dia: foto de identificação elegível por espécie e resumo com revisão editorial/especializada utilizável e fonte HTTPS. A seleção determinística prefere grupos representados distintos, depois completa até quatro por ID; não representa abundância, prioridade científica ou amostragem aleatória.

Os resultados são separados entre grupos taxonômicos e organismos cadastrados; organismos têm nome científico em itálico, nome comum somente se existente e caminho completo. Contagens significam registros únicos do catálogo sob o nó, não diversidade total, número de fotografias ou alternativas jogáveis. A busca não inicia partida, não abre fichas novas da Etapa 3 e não reescreve descrições. Dados incompletos não recebem destinos inferidos. Não há rede obrigatória, API externa, download ou nova dependência.

`TaxonomyView`, em Explorar, acrescenta cabeçalho com fotografia proporcional sem recorte, nome, nível, contagem e aviso de escopo; breadcrumbs e cartões descendentes mantêm a hierarquia real. Foto, autoria, licença declarada, origem e limitações têm apresentação visível e créditos expansíveis. Introduções, fontes, ressalvas filogenéticas e estado de revisão existente são preservados; para organismos, o resumo utilizável já existente é apresentado com sua fonte, sem criar uma ficha nova. Voltar conserva pesquisa/filtros no controlador, sem persistir preferências no banco. Jogar usa a variante anterior dos cartões e seus controles anteriores; Quiz, conquistas, estatísticas e diálogos gerais de créditos ficam fora da reformulação.

## Contrato de pesquisa e integridade

- `SpeciesCatalog.loadCurrentSpecies/loadCurrentTaxons`: por padrão, equivalem às leituras existentes. No adaptador SQLite, são o snapshot JSON instalado, capturado antes da leitura das linhas históricas. As leituras antigas continuam disponíveis para os jogos/migrações; nenhuma linha, perfil ou sessão é apagada. O índice científico usa apenas a projeção atual.
- `TaxonomyIndex.search(query, domainId, rank)`: filtra por interseção de domínio e rank existentes; pesquisa todos os termos em nomes, nome comum disponível, linhagem e rótulo do nível. Ignora caixa, espaços repetidos e acentos, sem modificar a grafia original. Relevância: nome científico exato, prefixo, ocorrência no nome, nome comum, caminho/nível; empates por nome e IDs estáveis. Resultados retornam `groups`, `organisms`, `total`, `groupCount` e `organismCount`.
- Deduplicação por ID positivo de organismo e ID de táxon. Nós de espécie vinculados ao organismo aparecem somente na seção de organismos. Homônimos com IDs/caminhos distintos continuam separados. Nós históricos do SQLite não entram na navegação; um grupo atual vazio continua pesquisável. Um organismo sem nó de espécie correspondente não recebe atalho inventado.
- `GameController.exploreQuery/exploreDomain/exploreRank` guardam apenas estado de consulta em memória; `exploreResults`, `exploreRanks` e `featuredOrganisms` expõem a projeção. QML apresenta dados e solicita navegação; não decide elegibilidade, classificação ou pontuação. A digitação tem espera de 220 ms para evitar reconstrução de centenas de cartões por tecla; Enter e saída do campo aplicam imediatamente.
- `ExploreCard.encyclopedic` é opcional, `false` por padrão. A variante nova mostra foto, rank, contagem/caminho e créditos curtos, sem tooltip longo. O contrato e a variante de Jogar permanecem disponíveis.

Aplicativo **1.8.0**, MINOR por funcionalidades compatíveis; catálogo **2.4.0** preservado. A identificação documental **2.2.2** é a subetapa da interface, independente dessas versões. Não há lançamento, tag, merge ou nova aprovação de curadoria/direitos. Os gates existentes continuam aplicáveis.

## Validação e capturas

Release recompilada; **26/26 suítes CTest aprovadas**, sem falhas, em **249,59 s**. QtTest registrou **59 casos aprovados** em domínio/aplicação e **13 na interface**, incluindo inicialização/limpeza das suítes. Os [resultados e hashes](verificacoes/resultados.json) vinculam [CTest completo](verificacoes/ctest-results.txt), [domínio](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt), identidade e recuperação aos binários utilizados.

- Busca por nomes científicos/comuns, caixa, acentos compostos/decompostos e espaços; combinação com domínio e rank; ausência de resultados; grupos atuais vazios; homônimos válidos com IDs distintos; repetição de registros/IDs; organismo sem caminho científico válido; dados sem nome comum/foto; navegação por resultados, ancestrais e descendentes; resumo científico idêntico à fonte existente.
- Teclado real para digitar, selecionar filtros e abrir resultados; foco visual, rolagem automática, retorno com filtros restaurados e cartões com textos longos a 242, 370 e 510 px. Conteúdo científico é texto simples. Nenhuma partida é criada ao explorar; uma sessão ativa conserva foto, alternativas, dicas, métricas, jogador, grupo e checkpoint, retornando por Retomar partida.
- Regressões completas das PRs #30 e #31: seleção/timing/crossfade/pausas e créditos do carrossel; Descoberta do Dia, indicadores e diálogo contextual; navegação, histórico, backup, migrações, sessão recuperável e resultado pendente. A variante de Jogar e suas proporções fotográficas anteriores continuam testadas.
- Lint de todos os QML, escopo Apache, estado derivado do projeto, auditorias técnicas de catálogo/mídias/repositório e comparação da governança com a base aprovados. Dados, imagens, manifesto e os **1.053 eventos de revisão** permanecem idênticos. Metadados/captura oficial e smoke test do executável real aprovados; nenhum perfil pessoal foi utilizado.
- O [diagnóstico de distribuição](verificacoes/distribution-summary.json) continua bloqueado por pendências científicas/de direitos já existentes, sem erros técnicos ou de dependências nessa consulta. Não é aprovação de lançamento e não foi contornado.

As **27 capturas** abaixo vieram de `scientificExploreSearchFiltersNavigationAndCaptures`, com janela Qt real, QML e controlador de produção; nenhuma imagem foi montada ou retocada. O [manifesto](capturas.json) registra horários UTC reais, dimensões, estado de pesquisa/navegação, hashes dos PNGs, fontes e binários. Logs são arquivados em UTF-8/LF, com regra específica em `.gitattributes`, para manter os hashes em checkouts Windows. Créditos individuais continuam nas fontes locais de fotografias e nos avisos visíveis; o manifesto identifica essas fontes sem ampliar permissões.

| Estado real | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Explorar, domínios e busca | [Captura](capturas/explore-1000x700.png) | [Captura](capturas/explore-1240x820.png) | [Captura](capturas/explore-1920x1080.png) |
| Organismos em destaque, com foco/rolagem | [Captura](capturas/featured-1000x700.png) | [Captura](capturas/featured-1240x820.png) | [Captura](capturas/featured-1920x1080.png) |
| Pesquisa Panthera, grupos e organismos | [Captura](capturas/search-1000x700.png) | [Captura](capturas/search-1240x820.png) | [Captura](capturas/search-1920x1080.png) |
| Panthera + Eukaryota + espécie | [Captura](capturas/filtered-1000x700.png) | [Captura](capturas/filtered-1240x820.png) | [Captura](capturas/filtered-1920x1080.png) |
| Nó científico do resultado | [Captura](capturas/taxon-1000x700.png) | [Captura](capturas/taxon-1240x820.png) | [Captura](capturas/taxon-1920x1080.png) |
| Créditos locais expandidos | [Captura](capturas/photo-credits-1000x700.png) | [Captura](capturas/photo-credits-1240x820.png) | [Captura](capturas/photo-credits-1920x1080.png) |
| Grupos descendentes e avisos de Eukaryota | [Captura](capturas/descendants-1000x700.png) | [Captura](capturas/descendants-1240x820.png) | [Captura](capturas/descendants-1920x1080.png) |
| Busca sem resultados | [Captura](capturas/empty-search-1000x700.png) | [Captura](capturas/empty-search-1240x820.png) | [Captura](capturas/empty-search-1920x1080.png) |
| Archaea, sem organismos cadastrados | [Captura](capturas/empty-group-1000x700.png) | [Captura](capturas/empty-group-1240x820.png) | [Captura](capturas/empty-group-1920x1080.png) |

Condições observadas: Windows 11, Qt 6.11.2, software/offscreen, escala 1, 96 DPI, SQLite temporário e Discord desativado; movimento reduzido somente para quadros estáticos. A redução de movimento e o timing normal também são testados nas regressões existentes. Outras GPUs, escalas e leitores de tela não foram ensaiados. A [imagem oficial da Home](../../../images/tela-inicial.png) e seu registro foram regenerados pelo executável de produção usando `scripts/capture_home.py --section discovery`, porque os fontes do controlador/índice pertencem à rastreabilidade dessa captura; a Home mantém seu desenho anterior. Capturas e testes não certificam ciência, direitos ou acessibilidade integral.

```powershell
cmake -S . -B build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.2.2-verified
cmake --build build -j 4
python scripts/capture_home.py --binary build/2.2.2-verified/projeto_glossata.exe --section discovery
ctest --test-dir build --output-on-failure
$qmlFiles = @(rg --files resources/qml -g '*.qml')
qmllint @qmlFiles --unqualified disable
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/audit_catalog.py --strict
python scripts/exploration_media.py
python scripts/content_governance.py audit-history --base 9d047910ddb6b56051d1830cba4052ca03a4e520
python scripts/release.py check
```

CTest configura software/offscreen nos testes pertinentes; as capturas são gravadas em `build/tests/previews/explore-2.2.2/` antes do arquivamento, com contexto real em `capturas.json`. O smoke test usa `build/2.2.2-verified/projeto_glossata.exe --smoke-test`, com Qt no PATH. Nenhum desses comandos publica release ou cria tag.
