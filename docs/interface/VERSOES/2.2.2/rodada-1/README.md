# PR #32 — rodada 1: correções estruturais de Explorar

Base: `f51f26e0a748d0c21089bd87d9b91a2016b3a5f9`. Revisão exclusiva da [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), na mesma branch `feat/issue-22-explore-2.2.2`, vinculada à Issue #22. Não cria PR, release, tag, merge ou avanço para 2.2.3. As [evidências iniciais](../README.md) continuam históricas; aplicativo 1.8.0 ainda não lançado e catálogo 2.4.0 preservados.

## Correções

- **Progressão independente:** no máximo 12 registros iniciais em cada seção; Carregar mais solicita outros 12, limitado ao total. Totais, relevância, filtros, IDs e deduplicação vêm integralmente de `TaxonomyIndex`; QML apenas apresenta prefixos das listas. Consulta/filtro reinicia os dois limites. Ativação por teclado leva ao primeiro cartão novo. Ao retornar de um táxon, consulta/filtros são restaurados e a apresentação recomeça na primeira página.
- **Consulta consistente:** `GameController.exploreQuery` acompanha imediatamente a digitação, com notificação própria. O timer de 220 ms adia a atualização visual e expõe estado pendente; não armazena uma consulta antiga. Filtros, Enter, término da edição e limpeza finalizam a espera. Durante a espera a interface informa Atualizando pesquisa e não apresenta resultados anteriores sob texto novo. A consulta continua válida se a view for destruída antes do timeout.
- **Fotos proporcionais:** os arquivos e metadados originais ficam intactos. Um ponto focal não define os limites de um recorte cientificamente seguro; o raster inteiro é ajustado à moldura. Foco válido é aplicado somente nas margens livres, sem esconder pixels. A fonte da imagem é atribuída quando a moldura intersecta o viewport real e retirada fora dele. O Canvas fotográfico oculto da variante enciclopédica fica sem fonte; Jogar preserva sua fotografia anterior.
- **Trilha compacta:** domínio, dois ancestrais próximos e táxon atual ficam visíveis; um controle expande todos os níveis intermediários com seus IDs originais. O nível atual tem seminegrito e indicação textual/acessível. Tab/Espaço permitem expandir, percorrer e abrir ancestrais; mudar o táxon recolhe a trilha. A classificação e os breadcrumbs de Jogar não são reorganizados.
- **Colunas e textos:** colunas calculadas por largura mínima de 280 px e espaçamento existente nos domínios, resultados, destaques e descendentes. Os quatro destaques continuam distribuídos de forma equilibrada; o último domínio pode ocupar a linha restante. Contadores visíveis e acessíveis usam singular/plural. Não há novo desenho de cartão, galeria, cabeçalho, ficha, créditos ou fontes.

## Validação e rastreabilidade

**26/26 suítes CTest aprovadas**, sem falhas, na execução final de **318,90 s**. QtTest registrou **60 casos de domínio/aplicação e 15 de interface** aprovados, incluindo inicialização/limpeza. [Resultados, binários e hashes](verificacoes/resultados.json), [CTest completo](verificacoes/ctest-results.txt), [domínio](verificacoes/core-results.txt) e [interface](verificacoes/ui-test-results.txt).

- Busca ampla por `a`: 500 grupos e 122 organismos no catálogo atual, com somente 12 cartões iniciais de cada seção; progressão independente a 24, ordem idêntica ao índice, totais preservados e IDs sem duplicação. Bacteria + espécie valida o último lote de oito registros e oculta Carregar mais ao chegar a 20. Alterar consulta ou filtro reinicia os limites.
- Consulta atual, sinais independentes e uma conclusão coalescida após edições rápidas; limpeza e filtros durante espera; saída da view antes do timeout; retorno com texto/filtros consistentes. A apresentação pendente não mostra cartões de consulta anterior. Singular de grupo/organismo, plural e nomes acessíveis verificados.
- Breadcrumbs compactos/expandidos, todos os índices originais, Tab/Espaço, abertura de ancestral oculto, recolhimento da altura e retorno à pesquisa. A navegação por teclado preserva o foco visual no início da página após mudar de táxon.
- Fotografias verificadas com raster integral e razão de aspecto preservada em cartões de 280, 372 e 510 px; foco de teste em 0,1/0,5/0,9 apenas nas margens livres. Nas três resoluções, verifica-se largura mínima, limites da janela e fonte da foto apenas nas molduras que intersectam o viewport; o Canvas oculto fica sem fonte.
- Regressões completas das PRs #30/#31 aprovadas: carrossel, timing/crossfade/pausas, descoberta diária, indicadores, créditos, teclado, Jogar, sessão ativa, histórico, checkpoints, recuperação e resultado pendente. Destruição imediata de Explorar/página taxonômica não deixa callbacks pendentes. O [diagnóstico anterior dos cartões](verificacoes/resolved-card-callbacks-before.txt) é histórico da falha corrigida por timers filhos, não evidência de aceitação.
- Build Release, smoke test real com código de saída 0, lint de todos os QML, escopo Apache, metadados e registro oficial da Home aprovados. [Auditorias técnicas](verificacoes/technical-audits.json) e [comparação da governança](verificacoes/governance-history.json) preservam elegibilidade, mídias e os 1.053 eventos. `TaxonomyIndex`, dados, fotografias, licenças e evidências iniciais estão idênticos à base. O [gate de distribuição](verificacoes/distribution-summary.json) continua bloqueado pelas pendências existentes.

As **27 capturas reais** foram geradas por `progressiveExploreNavigationAndCaptures`, com QML/controlador de produção em janela Qt, SQLite temporário e Discord desativado. O [manifesto](capturas.json) registra dimensões, horários UTC reais, consulta/filtros, quantidades apresentadas e hashes dos PNGs/fontes/binários. Não houve montagem ou retoque. Condições: Windows 11, Qt 6.11.2, software/offscreen, escala 1 e 96 DPI; movimento desligado para capturas estáticas. Outras GPUs, escalas e leitores de tela não foram ensaiados.

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Domínios com colunas adaptativas | [Captura](capturas/domains-1000x700.png) | [Captura](capturas/domains-1240x820.png) | [Captura](capturas/domains-1920x1080.png) |
| Destaques com fotos inteiras | [Captura](capturas/fitted-featured-1000x700.png) | [Captura](capturas/fitted-featured-1240x820.png) | [Captura](capturas/fitted-featured-1920x1080.png) |
| Busca ampla: 12 por seção | [Captura](capturas/broad-first-page-1000x700.png) | [Captura](capturas/broad-first-page-1240x820.png) | [Captura](capturas/broad-first-page-1920x1080.png) |
| Mais grupos, foco no primeiro novo | [Captura](capturas/more-groups-1000x700.png) | [Captura](capturas/more-groups-1240x820.png) | [Captura](capturas/more-groups-1920x1080.png) |
| Filtros combinados, último lote | [Captura](capturas/filtered-last-page-1000x700.png) | [Captura](capturas/filtered-last-page-1240x820.png) | [Captura](capturas/filtered-last-page-1920x1080.png) |
| Breadcrumb compacto | [Captura](capturas/compact-breadcrumbs-1000x700.png) | [Captura](capturas/compact-breadcrumbs-1240x820.png) | [Captura](capturas/compact-breadcrumbs-1920x1080.png) |
| Todos os níveis expandidos | [Captura](capturas/expanded-breadcrumbs-1000x700.png) | [Captura](capturas/expanded-breadcrumbs-1240x820.png) | [Captura](capturas/expanded-breadcrumbs-1920x1080.png) |
| Retorno com consulta/filtros | [Captura](capturas/return-to-results-1000x700.png) | [Captura](capturas/return-to-results-1240x820.png) | [Captura](capturas/return-to-results-1920x1080.png) |
| Sem resultados | [Captura](capturas/empty-search-1000x700.png) | [Captura](capturas/empty-search-1240x820.png) | [Captura](capturas/empty-search-1920x1080.png) |

A captura oficial da Home foi reexecutada com `scripts/capture_home.py --binary build/2.2.2-verified/projeto_glossata.exe --section discovery`, atualizando [seu registro](../../../../images/tela-inicial.json). O PNG e seu caminho no README permanecem preservados: a Home não teve mudança visual nesta rodada. Logs usam UTF-8/LF para manter hashes em checkouts Windows. Sincronização/testes C++ e registro da Home em [00f2280](https://github.com/kaduolliver/Projeto-Glossata/commit/00f2280990531fab7311e0afe3a420313016a84a); interface, testes, capturas e evidências em [96a25b0](https://github.com/kaduolliver/Projeto-Glossata/commit/96a25b05f4b0c31b29b3366bb8654b263ac9e9a6).

```powershell
cmake --build build -j 4
python scripts/capture_home.py --binary build/2.2.2-verified/projeto_glossata.exe --section discovery
ctest --test-dir build --output-on-failure
$qmlFiles = @(rg --files resources/qml -g '*.qml')
qmllint @qmlFiles --unqualified disable
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/release.py check
```

## Limites

A progressão limita cartões solicitados para apresentação; a pesquisa C++ continua avaliando o índice local completo. Carregar mais repetidamente mantém os cartões já solicitados; esta rodada não adiciona virtualização completa. O cache interno do Qt pode conservar imagens anteriormente decodificadas, mesmo com a fonte retirada dos cartões fora da área visível. A validação técnica existente das mídias continua verificando integridade/elegibilidade independentemente da decodificação para exibição.

Um ensaio adicional que criou e removeu a Home antes de aguardar um quadro detectou avisos de contexto inválido em `HomeView.qml:49`, no `Qt.callLater(preloadNext)` preexistente. Essa tela está idêntica à base; o achado fica separado do cancelamento corrigido dos cartões de Explorar. O teste de estresse da rodada força criação/destruição imediata entre Explorar e a página taxonômica. O [registro do achado da Home](verificacoes/preexisting-home-context.txt) é diagnóstico fora do escopo, sem tratá-lo como falha nova da pesquisa nem conceder aprovação de correção dessa tela. A navegação normal e as regressões existentes da Home passaram na execução final.

Pontos de foco não concedem aprovação de recorte. Não há mudança de catálogo, critérios de revisão/eligibilidade, fotografias, licenças, ciência, sessões, dicas, pontuação ou governança. A versão 1.8.0 permanece em desenvolvimento, sem novo lançamento. Testes/capturas não constituem certificação científica, jurídica ou de acessibilidade em plataformas não ensaiadas. O redesenho estético continua reservado à rodada 2.
