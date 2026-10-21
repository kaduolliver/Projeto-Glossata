# PR #32 — refinamentos finais de Explorar

Base: `0be191949ab194b90027441b43e8196c01ad07ae`, na mesma branch e [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), vinculada à Issue #22. Aplicativo 1.8.0 ainda não lançado; catálogo 2.4.0. [Entrega inicial](../README.md), [rodada 1](../rodada-1/README.md) e [rodada 2](../rodada-2/README.md) permanecem históricas. Commit final pendente das verificações.

## Alterações

`TaxonomyIndex.describe` acrescenta projeções compatíveis, sem mudar pesquisa, relevância, filtros, contagens ou IDs. `educationalNote` reutiliza integralmente o resumo/introdução retornado por `EducationalContent`, que verifica a usabilidade da revisão registrada. Exige revisão editorial/especializada, texto, título da fonte e URL HTTPS válida. Não usa a descrição genérica como substituta. QML limita a exibição a duas linhas com elipse; a página conserva o conteúdo integral e a fonte. `classificationSummary` prioriza família/gênero existentes, com rótulo de rank; se ambos faltam, usa até dois ancestrais próximos documentados. Não infere nós ausentes nem funde homônimos.

Cada cartão enciclopédico tem **Fonte, origem e créditos**, com ação independente de abrir o táxon. O diálogo é criado sob demanda, mantém autoria, licença/versão, título, origem, modificações históricas, fonte científica e avisos; links usam os URLs existentes. Escape/Fechar devolvem foco ao controle; Tab mantém os links visíveis na rolagem do diálogo. Sem rede obrigatória ou novos registros de curadoria. A variante de Jogar permanece visualmente igual.

O adaptador de mídia expõe metadados existentes e uma indicação conservadora de apresentação contextual para CC0 1.0 e CC BY 4.0 já verificadas, sem exigência adicional em `Attribution` da evidência. O controle permanece ao lado da fotografia e leva diretamente aos créditos completos. Para versões antigas, desconhecidas ou condições específicas, mantém autoria/licença/atribuição indicada visíveis no cartão, além do diálogo. Campos de evidência, permissões e governança não são editados; essa projeção não concede direitos nem libera distribuição.

Fundamento consultado para a apresentação contextual: [CC BY 4.0, seção 3(a)(2)](https://creativecommons.org/licenses/by/4.0/legalcode.en#s3a), que admite atender à atribuição por referência adequada ao contexto. A [CC BY 2.0, seção 4(b)](https://creativecommons.org/licenses/by/2.0/legalcode.en) conserva exigências próprias de atribuição; a interface aplica a política conservadora acima, sem presumir equivalência de versões. O exame técnico das evidências atuais não encontrou `Attribution` adicional nas fotografias 4.0. Não é nova aprovação jurídica ou científica.

`TaxonomyView`, apenas em Explorar, ordena identificação/fotografia, introdução/resumo, descendentes/estado informativo e rodapé expansível de classificação/fontes/proveniência. Breadcrumbs completos/compactos e avisos permanecem. Créditos fotográficos junto à imagem são preservados; detalhes complementares ficam no rodapé. A organização de Jogar não muda.

Fotografias dos cartões e páginas de Explorar ficam centralizadas nos dois eixos, com raster integral, proporção e arquivos intactos. Metadados de foco não deslocam a imagem; escala máxima de 1 evita ampliar além dos pixels armazenados. Histórico de modificações é apresentado literalmente sob título próprio, separado da explicação do comportamento atual. Textos antigos que mencionam recorte/foco continuam como evidência histórica, sem afirmar que essa seja a apresentação atual de Explorar.

A galeria mantém seleção científica e funcionamento manual. Anteriores/Próximos, intervalo visível e indicação de cartões fora da área tornam a navegação explícita. Tab/setas, foco e redução de movimento permanecem; não há avanço automático. Consulta, filtros, debounce de 220 ms, lotes progressivos e restauração de foco/rolagem continuam preservados.

## Verificação e capturas

**26/26 suítes CTest aprovadas em 316,21 s**, incluindo **61 casos QtTest de domínio/aplicação e 16 de interface**, com inicialização/limpeza incluídas. [Resultados e hashes](verificacoes/resultados.json), [CTest completo](verificacoes/ctest-results.txt), [domínio](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt) e [smoke test real com saída 0](verificacoes/smoke-test.txt). Build Release, lint QML, escopo de licenciamento do código, status e metadados aprovados. [Auditorias técnicas](verificacoes/technical-audits.json) sem erros e [governança](verificacoes/governance-history.json) com os 1.053 eventos preservados. O [gate de distribuição](verificacoes/distribution-summary.json) mantém os bloqueios anteriores, sem erros técnicos/de dependências e sem novas aprovações.

- O teste novo de domínio verifica a identidade integral das notas com resumos/introduções existentes, revisão editorial/especializada, fonte rastreável e ausência de fallback inventado. Fixtures somente em memória cobrem notas sem fonte/título/texto, rejeitadas, pendentes ou vencidas. `Papilio machaon` mostra família e gênero corretos; homônimos sintéticos mantêm seus ranks e uma família ausente não é inferida.
- O teste de interface valida resumos em duas linhas e ausência de resumo em Archaea, atribuição contextual 4.0 e atribuição próxima para versões não comprovadas, clique independente do cartão, abertura de fontes/licenças exatas, Escape/Fechar/reabertura, retorno de foco e links acessíveis por teclado na rolagem interna. O diálogo é liberado após fechar por timer filho do cartão; não fica associado a contexto destruído.
- As três resoluções validam fotos centradas em ambos os eixos, raster integral, proporção e escala máxima 1. Pontos focais de fixture em 0,1/0,5/0,9 não deslocam a imagem; `Panthera leo` e `Danaus plexippus` são verificados explicitamente nos cartões e páginas. Rodapé fica abaixo dos descendentes ou estado informativo; metadados históricos permanecem literais e distintos da apresentação atual.
- Pesquisa ampla, filtros combinados, ordem/deduplicação, lotes/último lote, debounce, limpeza, breadcrumbs, retorno com 24 grupos/24 organismos e rolagem/foco continuam aprovados. Regressões completas de carrossel, descoberta/indicadores/créditos, Jogar, sessões, dicas, checkpoints, recuperação, histórico e resultado pendente passaram. Não há alteração em `GameController`, `HomeView`, conteúdo científico, arquivos fotográficos, licenças ou governança.

As **36 capturas reais** (12 estados × três resoluções) usam QML/controladores de produção, SQLite temporário, Discord desativado e links interceptados nos testes. O [manifesto](capturas.json) registra contexto, dimensões, horários UTC e hashes dos PNGs, fontes e binários. Nenhum banco pessoal ou conteúdo científico novo é usado nas capturas. Condições: Windows 11, Qt 6.11.2, software/offscreen, escala 1 e 96 DPI, movimento reduzido para imagens estáticas.

Os estados abaixo são capturas reais da janela Qt, sem composição/retocagem. Cada conjunto usa o catálogo instalado; resumos sintéticos existem somente em testes unitários de rejeição/elegibilidade, sem aparecer nas capturas.

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Domínios e controles | [Captura](capturas/overview-1000x700.png) | [Captura](capturas/overview-1240x820.png) | [Captura](capturas/overview-1920x1080.png) |
| Galeria educativa | [Captura](capturas/gallery-1000x700.png) | [Captura](capturas/gallery-1240x820.png) | [Captura](capturas/gallery-1920x1080.png) |
| Créditos contextuais | [Captura](capturas/contextual-credits-1000x700.png) | [Captura](capturas/contextual-credits-1240x820.png) | [Captura](capturas/contextual-credits-1920x1080.png) |
| Histórico e apresentação atual | [Captura](capturas/credit-history-1000x700.png) | [Captura](capturas/credit-history-1240x820.png) | [Captura](capturas/credit-history-1920x1080.png) |
| Grupo e introdução | [Captura](capturas/group-1000x700.png) | [Captura](capturas/group-1240x820.png) | [Captura](capturas/group-1920x1080.png) |
| Grupos descendentes | [Captura](capturas/descendants-1000x700.png) | [Captura](capturas/descendants-1240x820.png) | [Captura](capturas/descendants-1920x1080.png) |
| Pesquisa restaurada | [Captura](capturas/restored-results-1000x700.png) | [Captura](capturas/restored-results-1240x820.png) | [Captura](capturas/restored-results-1920x1080.png) |
| Grupo sem organismos | [Captura](capturas/empty-group-1000x700.png) | [Captura](capturas/empty-group-1240x820.png) | [Captura](capturas/empty-group-1920x1080.png) |
| Panthera leo centralizado | [Captura](capturas/lion-1000x700.png) | [Captura](capturas/lion-1240x820.png) | [Captura](capturas/lion-1920x1080.png) |
| Rodapé de Panthera leo | [Captura](capturas/lion-footer-1000x700.png) | [Captura](capturas/lion-footer-1240x820.png) | [Captura](capturas/lion-footer-1920x1080.png) |
| Danaus plexippus centralizado | [Captura](capturas/monarch-1000x700.png) | [Captura](capturas/monarch-1240x820.png) | [Captura](capturas/monarch-1920x1080.png) |
| Rodapé de Danaus plexippus | [Captura](capturas/monarch-footer-1000x700.png) | [Captura](capturas/monarch-footer-1240x820.png) | [Captura](capturas/monarch-footer-1920x1080.png) |

```powershell
cmake -S . -B build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.2.2-final
cmake --build build -j 3
python scripts/capture_home.py --binary build/2.2.2-final/projeto_glossata.exe --section discovery
ctest --test-dir build --output-on-failure
$qmlFiles = @(rg --files resources/qml -g '*.qml')
qmllint @qmlFiles --unqualified disable
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/release.py check
```

O binário final fica em diretório próprio para não interromper a instância da rodada anterior aberta no Windows. A captura oficial da Home e [seu registro](../../../../images/tela-inicial.json) foram regenerados após recompilar o C++ compartilhado, com SQLite temporário; a composição de `HomeView.qml` e seu caminho no README não mudam.

## Pendências e planejamento

A **Etapa 3 — Enciclopédia Científica Interativa** deverá criar e revisar descrições educativas completas de grupos e organismos, com fontes e revisão rastreáveis pelo fluxo de curadoria. Esta PR apenas reutiliza textos já elegíveis; grupos sem texto permanecem sem descrição no cartão. Não há produção científica em massa nem antecipação de fichas completas.

O aviso preexistente de ciclo de vida da Home continua fora do escopo e [documentado separadamente](../rodada-1/verificacoes/preexisting-home-context.txt). Progressão não é virtualização completa; o cache Qt pode conservar imagens já decodificadas. Outras GPUs, escalas e leitores de tela não são presumidos validados. Pendências de licenciamento/distribuição permanecem vigentes. Sem outra PR, merge, tag, release ou avanço para 2.2.3.
