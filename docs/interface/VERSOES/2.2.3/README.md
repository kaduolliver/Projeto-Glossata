# Subetapa 2.2.3 — Modernização da tela Jogar

Revisão atual da PR #33: [alinhamento do botão Iniciar partida](alinhamento-iniciar/README.md), sobre a [configuração compacta e fontes contextuais](configuracao-compacta/README.md). A [configuração direta e o retorno ao contexto de seleção](refinamentos-navegacao/README.md), assim como as capturas, contratos e resultados abaixo, permanecem preservados como histórico.

Base: `1243a8bd47faa19a4a4b765ea432d5eadaef9a15`, integração da PR #32 na `main`, confirmada pela API antes das alterações. Branch `feat/issue-22-play-2.2.3`, entregue na [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33), vinculada à [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22), sem merge ou lançamento. Referência estética: exclusivamente a região 3 (seleção taxonômica) da imagem fornecida; fotografias, números e classificações da referência não são conteúdo do catálogo.

Aplicativo **1.9.0 não lançado** (MINOR por seleção compatível); catálogo **2.4.0** inalterado. Escopo: apresentação da seleção de Jogar, modos/configuração, navegação e disponibilidade projetada pelos critérios existentes. Explorar recebe somente o alinhamento das ações dos cartões. Quiz, regras científicas, pontuação, sessões, dados e governança são preservados.

Implementação em [d522335](https://github.com/kaduolliver/Projeto-Glossata/commit/d522335ea923bb75e37507ffd39247ae4cbb7a09), alinhamento/padding em [b9d9fd6](https://github.com/kaduolliver/Projeto-Glossata/commit/b9d9fd6afa84526f6a70b053fad8db935e65f4ea) e nome acessível do jogo misto em [bd1811c](https://github.com/kaduolliver/Projeto-Glossata/commit/bd1811c). O [manifesto](capturas.json) identifica o código final das capturas e os hashes dos arquivos/executáveis. A proveniência da execução completa e das execuções direcionadas fica separada em [resultados.json](verificacoes/resultados.json).

## Organização e contratos

- **Reinos:** os atalhos científicos existentes para Animalia, Plantae e Fungi; não é uma lista de reinos inventada a partir da referência.
- **Domínios:** Archaea, Bacteria e Eukaryota, com seus IDs originais, inclusive os grupos vazios.
- **Grupos avançados:** os descendentes imediatos existentes de Eukaryota — Amoebozoa, Archaeplastida, Discoba, Opisthokonta e SAR. Cada cartão permite seguir pela árvore original. Não há um reino artificial “Protista” nem mistura desses clados com domínios.

`TaxonomyIndex.playEntry` acrescenta somente `playLevels`, reutilizando `identificationLevels`; `playGroups` e `playChildren` organizam mapas já existentes. O controlador os expõe sem lógica científica em QML. `describe`, pesquisa, critérios de aprovação e geração de questões não foram reescritos. Testes comparam IDs, ordem, contagens e os demais campos com as projeções anteriores. A navegação usa a projeção atual do catálogo, preservando os registros históricos no banco.

Cartões mostram **organismos no catálogo** e **níveis de identificação disponíveis** separadamente. Selecionar um cartão abre a configuração de identificação; não inicia a partida. O painel informa os organismos elegíveis e as respostas distintas no nível escolhido, conforme o backend. A dificuldade pode restringir a composição final das alternativas; essa validação continua no início da partida. Grupos vazios ou com poucos registros permanecem acessíveis e exibem estado sem partida, com início desabilitado. O jogo misto conserva identificação de gêneros, sua contagem elegível e início explícito.

Livre e Competitivo têm painéis próprios. A dificuldade é configuração do Livre. Competitivo deixa explícitos seu conjunto geral, identificação de gêneros, alternativas equilibradas e quantidade de questões fornecida pelo controlador; não assume o grupo selecionado. As ações usam os mesmos `beginTaxonGame` e `beginCompetitiveGame`, sem modificar Main, GameService, Quiz, regras, ranking ou persistência.

`ExploreCard.playSelection` reutiliza moldura, fotografia integral/centralizada, escala máxima 1, carga por viewport e diálogo contextual já existentes. Usa o mesmo hover passivo, foco dourado, transições e redução de movimento; créditos específicos obrigatórios continuam visíveis e integrais. As grades compartilham a maior altura intrínseca por seção e usam largura mínima de 280 pixels lógicos. As abas são `UiButton.compact` em grupo exclusivo. Os títulos completos têm quebra de linha e as ações ficam alinhadas na base.

Em Explorar, o controle de créditos preserva padding `spaceMd`; a ação principal recebe a mesma margem esquerda e uma margem superior `spaceXs`. Os dois textos começam na mesma coordenada horizontal, conservando botão compacto, espaçamento interno, espaço vazio inerte, foco, hover e cliques independentes. Não se altera a política de atribuição, fotografias, seleção, pesquisa, filtros, paginação ou estado da enciclopédia. [Histórico anterior preservado](../2.2.2/README.md).

## Validação e evidências

**26/26 suítes CTest aprovadas**, incluindo 62 casos QtTest de domínio, 19 de interface, 32 de identidade e 21 de recuperação, contando inicialização/limpeza. Execução completa sobre o código de [b9d9fd6](https://github.com/kaduolliver/Projeto-Glossata/commit/b9d9fd6), após o ajuste final do padding; tempo total registrado pelo CTest: **456,09 s**. A última alteração acrescenta somente um nome acessível explícito ao jogo misto, com teste correspondente; sua validação direcionada e capturas finais são registradas separadamente, sem atribuir a ela a execução anterior da suíte completa. [Log completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt), [resultados e proveniência de cada execução](verificacoes/resultados.json).

Após o último ajuste, `playSelectionNavigationAndCaptures` passou novamente em escala [100%](verificacoes/play-final-targeted.txt), [125%](verificacoes/play-dpi-125.txt) e [150%](verificacoes/play-dpi-150.txt), cada execução com três casos aprovados, incluindo preparação/limpeza. Verifica nome acessível, exclusividade das abas, IDs, igualdade de alturas, fotos proporcionais/centradas, créditos independentes, teclado/foco/hover, grupos vazios e limitados, disponibilidade por nível, dificuldade, início Livre/Competitivo/misto, retorno à partida e alinhamento dos textos no Explorar. As regressões da execução completa cobrem PRs #30/#31/#32, pesquisa, filtros, deduplicação, paginação, breadcrumbs, restauração, três dicas/pontuação, recuperação/checkpoints e histórico.

Build Release, lint QML, escopo de licenciamento do código, status do projeto, metadados/histórico e [smoke real com saída 0](verificacoes/smoke-test.txt) passaram. As [auditorias técnicas](verificacoes/technical-audits.json) preservam 122 organismos, 122 registros fotográficos válidos, 114 fotos de exploração, 533 vínculos e 623 cartões. A comparação dos [1.053 eventos de governança](verificacoes/governance-history.json) com a base confirma integridade. O [gate de distribuição](verificacoes/distribution-summary.json) mantém os bloqueios anteriores, sem erros técnicos ou de dependências; verificações técnicas não aprovam direitos ou ciência.

As capturas usam a interface e controlador reais em QtTest, SQLite temporário e Discord desativado. Os PNGs não recebem montagem, retoque ou simulação. O manifesto registra resolução lógica/física, fator Qt, horário, tela/táxon e hashes das fontes, imagens e executáveis. São arquivados **36 quadros**: oito estados em cada uma das três resoluções a 100%, mais modos/reinos nas três resoluções a 125% e 150%.

| Resolução lógica | Modos | Reinos | Domínios | Avançados | Configuração | Sem partida | Ajuste Explorar |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/modes-1000x700.png) | [Abrir](capturas/kingdoms-1000x700.png) | [Abrir](capturas/domains-1000x700.png) | [Abrir](capturas/advanced-1000x700.png) | [Abrir](capturas/configure-1000x700.png) | [Vazio](capturas/empty-group-1000x700.png) / [limitado](capturas/limited-group-1000x700.png) | [Abrir](capturas/explore-alignment-1000x700.png) |
| 1240×820 | [Abrir](capturas/modes-1240x820.png) | [Abrir](capturas/kingdoms-1240x820.png) | [Abrir](capturas/domains-1240x820.png) | [Abrir](capturas/advanced-1240x820.png) | [Abrir](capturas/configure-1240x820.png) | [Vazio](capturas/empty-group-1240x820.png) / [limitado](capturas/limited-group-1240x820.png) | [Abrir](capturas/explore-alignment-1240x820.png) |
| 1920×1080 | [Abrir](capturas/modes-1920x1080.png) | [Abrir](capturas/kingdoms-1920x1080.png) | [Abrir](capturas/domains-1920x1080.png) | [Abrir](capturas/advanced-1920x1080.png) | [Abrir](capturas/configure-1920x1080.png) | [Vazio](capturas/empty-group-1920x1080.png) / [limitado](capturas/limited-group-1920x1080.png) | [Abrir](capturas/explore-alignment-1920x1080.png) |

| Resolução lógica | Qt 125% | Qt 150% |
| --- | --- | --- |
| 1000×700 | [Modos](capturas/modes-1000x700-125.png) / [reinos](capturas/kingdoms-1000x700-125.png) | [Modos](capturas/modes-1000x700-150.png) / [reinos](capturas/kingdoms-1000x700-150.png) |
| 1240×820 | [Modos](capturas/modes-1240x820-125.png) / [reinos](capturas/kingdoms-1240x820-125.png) | [Modos](capturas/modes-1240x820-150.png) / [reinos](capturas/kingdoms-1240x820-150.png) |
| 1920×1080 | [Modos](capturas/modes-1920x1080-125.png) / [reinos](capturas/kingdoms-1920x1080-125.png) | [Modos](capturas/modes-1920x1080-150.png) / [reinos](capturas/kingdoms-1920x1080-150.png) |

Para repetir: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, `cmake --build build -j3` e, após a compilação, `ctest --test-dir build --output-on-failure`. O teste `playSelectionNavigationAndCaptures` gera oito estados por resolução em `build/tests/previews/play-2.2.3/<escala>/`. Usa `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic`; pode ser executado separadamente com `QT_SCALE_FACTOR=1.25` ou `1.5`. Nesta evidência, os quadros ampliados arquivados priorizam modos/reinos; o teste funcional completo percorre todos os estados nas três escalas. Preserve este registro ao gerar novas evidências.

A [Home oficial](../../../images/tela-inicial.png) foi novamente capturada por `scripts/capture_home.py --binary build/2.2.3-verified/projeto_glossata.exe --section discovery`. O PNG e o [registro](../../../images/tela-inicial.json) acompanham o ajuste final, com versão, binário e fontes reais. A seleção diária é conteúdo dinâmico do programa, sem congelamento ou alteração artificial de pixels. `HomeView.qml`, `Main.qml` e os dados permanecem intactos.

## Limitações e integridade

Windows 11, Qt 6.11.2, renderização software/offscreen; fatores Qt 1, 1,25 e 1,5 mantêm as larguras lógicas e ampliam o raster físico. Não alteramos a escala do Windows nem ensaiamos outras GPUs ou leitores de tela. Mouse e teclado foram exercitados por eventos Qt; testes não substituem validação universal de acessibilidade.

O gate de distribuição conserva suas pendências anteriores; não há nova aprovação ou permissão. O diagnóstico histórico da Home com frame zero permanece fora deste escopo. A criação e revisão de descrições científicas completas continua reservada à Etapa 3. Esta entrega não modifica dados científicos, arquivos fotográficos, licenças ou eventos de governança; não cria merge, tag, release nem antecipa a modernização do Quiz 2.3.1.
