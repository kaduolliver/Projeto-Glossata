# PR #32 — rodada 2: Explorar fotográfico

Base: `ce613adcde4d8af30bf4d136c65974b65d928f5d`, na mesma branch `feat/issue-22-explore-2.2.2` e [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), vinculada à Issue #22. As [evidências iniciais](../README.md) e da [rodada 1](../rodada-1/README.md) permanecem históricas. Aplicativo 1.8.0 ainda não lançado; catálogo 2.4.0 intacto. Commit final da interface: pendente até a gravação das verificações.

## Composição e contratos

Apenas a região **2. TELA EXPLORAR (ENCICLOPÉDIA)** da referência fornecida orienta esta rodada: fotografia, nomes científicos, pesquisa integrada e galeria. Nenhum conteúdo científico, fotografia, categoria ou contagem foi copiado da imagem. A comparação usa o catálogo real, com três domínios, incluindo Archaea sem organismos cadastrados. Não há mudança nas telas Início, Jogar, Quiz, Estatísticas ou Conquistas.

- `ExploreCard.encyclopedic` usa molduras fotográficas de 146 px para grupos e 160 px para organismos, cantos arredondados, gradiente discreto nos espaços livres e conteúdo compacto. Grupos com largura a partir de 450 px dispõem foto e texto lado a lado. Nome científico integral, nome comum disponível, contagem ou dois ancestrais próximos substituem o caminho completo nos cartões. O caminho integral continua na descrição acessível e na página taxonômica. Autoria integral e licença permanecem visíveis, sem abreviação ou elipse; textos extensos podem aumentar a altura. A variante padrão de Jogar conserva seu visual e sua fotografia.
- O raster inteiro permanece proporcional, com foco válido aplicado somente às margens livres. Não há recorte, distorção, cópia decorativa da fotografia ou alteração de arquivos/metadados. A moldura usa cores do tema para preencher espaços; não presume que o foco autorize cortes. A imagem só recebe fonte se intersectar tanto a galeria horizontal quanto a rolagem vertical da página. Timers filhos continuam canceláveis ao destruir a view; o cache Qt pode reter imagens já decodificadas.
- `CategoryView` coloca título e busca lado a lado a partir de 930 px de conteúdo, empilhando-os abaixo disso. Filtros mantêm modelos/IDs existentes e largura limitada. `clearExploreQuery` limpa somente o texto; `clearExploreSearch` limpa texto e filtros. Debounce de 220 ms, resultados pendentes, relevância, deduplicação, totais e lotes independentes de 12 preservam os contratos da rodada 1.
- A galeria contém somente os quatro destaques elegíveis existentes, sem seleção nova. Em largura suficiente, todos aparecem juntos; nas janelas compactas, mantém cartões de pelo menos 280 px e permite arrastar ou usar Anteriores/Próximos. Tab e setas esquerda/direita tornam o cartão focado visível. Não há avanço automático. Transições de cor e rolagem respeitam `Theme.motionEnabled`.
- `Main.explorePresentation` armazena apenas estado efêmero de apresentação: chave de texto/domínio/rank, limites das duas seções, posição vertical, posição da galeria e controle de retorno. `CategoryView` restaura esse estado após recriar os delegates, limitado à área disponível. Abertura de resultado e retorno preservam texto, filtros, quantidade carregada, foco e rolagem. Nova combinação de consulta/filtros reinicia os lotes. Nada é gravado no banco e nenhum contrato de `GameController` ou `TaxonomyIndex` muda.
- `TaxonomyView`, apenas em Explorar, diferencia grupo e organismo por identificação e destaque fotográfico; grupo mostra contagem local e organismo destaca nome comum e resumo existente. Introdução educativa/resumo, classificação e fontes têm seções próprias. Breadcrumbs compactos/expansíveis, IDs, avisos de identificação, revisão, enquadramento e limitações permanecem acessíveis. Atribuição fotográfica fica visível; detalhes expansíveis mantêm título, modificações, origem e licença. A URL original aceita os campos existentes `photoUrl` ou `sourceUrl`, conforme o tipo de mídia. Classificação integral e fontes científicas podem ser expandidas sem abrir serviços externos obrigatórios. Não são fichas completas da Etapa 3 nem novas redações científicas.

## Validação e capturas

**26/26 suítes CTest aprovadas em 323,56 s**, incluindo **60 casos QtTest de domínio/aplicação e 16 de interface**, com inicialização/limpeza incluídas. [Resultados e hashes](verificacoes/resultados.json), [CTest completo](verificacoes/ctest-results.txt), [domínio](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt) e [smoke test real com saída 0](verificacoes/smoke-test.txt). Build Release, lint de todos os QML, escopo de licenciamento do código, metadados de versão/captura e consistência de status aprovados. [Auditorias técnicas](verificacoes/technical-audits.json) sem erros; [comparação de governança](verificacoes/governance-history.json) preserva os 1.053 eventos. O [gate de distribuição](verificacoes/distribution-summary.json) continua bloqueado pelas pendências anteriores, sem erros técnicos/de dependências e sem concessão de direitos ou revisão científica.

- `photographicExploreNavigationAndCaptures` valida cabeçalho empilhado/alinhado nas três resoluções, galeria por mouse e teclado/setas, cartão focado visível, atribuição integral, classificação resumida, diferenças entre grupo/organismo, foto taxonômica inteira, expansores e abertura das URLs exatas de fotografia, licença e referência científica.
- Retorno do organismo de índice 13 conserva os **24 grupos e 24 organismos** já carregados, texto, destino por ID, foco e posição vertical com tolerância de 1 px. As verificações anteriores continuam cobrindo filtros preservados no retorno. Nova consulta/filtro reinicia os lotes; limpar só o texto conserva domínio/rank e os controles; Limpar tudo restaura o estado inicial.
- Regressões da rodada 1 aprovadas: busca ampla, ordem/relevância, deduplicação, filtros combinados, último lote, textos extensos/ausência de fotos, raster integral com foco nas margens, interseção dos dois viewports, debounce/edição rápida, breadcrumbs compactos/expandidos e destruição imediata de views com timers pendentes.
- Regressões das PRs #30/#31 e de Jogar aprovadas: carrossel/transições/pausas/redução de movimento, descoberta diária, indicadores e créditos, navegação separada, sessão ativa, perguntas/dicas, checkpoint, recuperação, histórico e resultado pendente. Dados, mídias e controladores científicos são idênticos à base; nenhum banco pessoal foi usado.

As **30 capturas reais** abaixo correspondem a dez estados nas três resoluções. O [manifesto](capturas.json) registra contexto, horário UTC, dimensões, rolagem, lotes e hashes dos PNGs, fontes e binários. Foram obtidas com `QQuickWindow::grabWindow`, QML/controlador de produção, SQLite temporário e Discord desativado. Links são interceptados pelos testes, sem navegador ou rede. Não há montagem, retoque ou imagens simuladas. Comparadas à referência, as capturas aproximam a hierarquia fotográfica, a galeria e a pesquisa integrada; conservam a paleta do projeto e atribuições integrais, mesmo quando exigem mais linhas.

Condições: Windows 11, Qt 6.11.2, software/offscreen, escala 1 e 96 DPI. Capturas estáticas usam redução de movimento. Outras GPUs, escalas e leitores de tela não foram ensaiados.

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Cabeçalho e domínios | [Captura](capturas/overview-1000x700.png) | [Captura](capturas/overview-1240x820.png) | [Captura](capturas/overview-1920x1080.png) |
| Galeria fotográfica | [Captura](capturas/gallery-1000x700.png) | [Captura](capturas/gallery-1240x820.png) | [Captura](capturas/gallery-1920x1080.png) |
| Organismo individual | [Captura](capturas/organism-1000x700.png) | [Captura](capturas/organism-1240x820.png) | [Captura](capturas/organism-1920x1080.png) |
| Proveniência da fotografia | [Captura](capturas/photo-provenance-1000x700.png) | [Captura](capturas/photo-provenance-1240x820.png) | [Captura](capturas/photo-provenance-1920x1080.png) |
| Classificação e fontes | [Captura](capturas/scientific-context-1000x700.png) | [Captura](capturas/scientific-context-1240x820.png) | [Captura](capturas/scientific-context-1920x1080.png) |
| Breadcrumbs expandidos | [Captura](capturas/breadcrumbs-expanded-1000x700.png) | [Captura](capturas/breadcrumbs-expanded-1240x820.png) | [Captura](capturas/breadcrumbs-expanded-1920x1080.png) |
| Grupo taxonômico | [Captura](capturas/group-1000x700.png) | [Captura](capturas/group-1240x820.png) | [Captura](capturas/group-1920x1080.png) |
| Descendentes | [Captura](capturas/descendants-1000x700.png) | [Captura](capturas/descendants-1240x820.png) | [Captura](capturas/descendants-1920x1080.png) |
| Retorno com 24 resultados por seção | [Captura](capturas/restored-results-1000x700.png) | [Captura](capturas/restored-results-1240x820.png) | [Captura](capturas/restored-results-1920x1080.png) |
| Grupo sem organismos | [Captura](capturas/empty-group-1000x700.png) | [Captura](capturas/empty-group-1240x820.png) | [Captura](capturas/empty-group-1920x1080.png) |

```powershell
cmake -S . -B build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.2.2-round-2
cmake --build build -j 4
python scripts/capture_home.py --binary build/2.2.2-round-2/projeto_glossata.exe --section discovery
ctest --test-dir build --output-on-failure
$qmlFiles = @(rg --files resources/qml -g '*.qml')
qmllint @qmlFiles --unqualified disable
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/release.py check
```

O diretório de binários foi separado porque a instância anterior estava aberta no Windows. Ela não foi interrompida. Os registros contêm os hashes dos executáveis novos efetivamente testados.

## Limites preservados

O aviso preexistente de ciclo de vida em `HomeView.qml:49`, encontrado no estresse sem aguardar o primeiro quadro, permanece fora do escopo, conforme pedido. O [diagnóstico histórico separado](../rodada-1/verificacoes/preexisting-home-context.txt) não é evidência de correção. `HomeView.qml` permanece idêntico à base; a propriedade de memória em `Main.qml` não muda sua composição. A captura oficial foi reexecutada com `scripts/capture_home.py --binary build/2.2.2-round-2/projeto_glossata.exe --section discovery`, com SQLite temporário, atualizando [imagem](../../../../images/tela-inicial.png) e [registro](../../../../images/tela-inicial.json) no mesmo commit visual. Seu conteúdo diário reflete a data real da execução; o caminho no README é preservado.

Progressão de apresentação não é virtualização completa. A rolagem é restaurada durante a vida da janela e ajustada ao tamanho disponível; fechar o programa descarta esse estado. Autoria extensa permanece integral e pode ocupar mais linhas que na referência; não se presume autorização para abreviar atribuições. Fotografias verticais conservam espaços laterais para preservar características científicas.

Dados científicos, critérios de elegibilidade, mídias, licenças e eventos de governança não são alterados. O gate de distribuição continua sujeito às pendências existentes, sem aprovações novas. Nenhuma outra PR, merge, tag, release, correção da Home ou avanço para 2.2.3.
