# PR #32 — créditos e interação dos cartões

Base: `904d13a15dd415a66b8859cc45fe4326fbe1b0bd`, mesma branch `feat/issue-22-explore-2.2.2` e [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32). Esta correção se limita à apresentação dos créditos, às dimensões da ação e ao hover de `ExploreCard`. Aplicativo 1.8.0 ainda não lançado; catálogo 2.4.0. Os [refinamentos anteriores](../rodada-final/README.md), suas capturas e evidências permanecem históricos e intactos.

Código e testes em [beee137](https://github.com/kaduolliver/Projeto-Glossata/commit/beee137a54468f6833c694186176c9d2d6c05f5f). O manifesto das capturas identifica esse commit e os hashes exatos das fontes e dos binários.

## Decisões de apresentação

O diálogo **Fonte, origem e créditos**, imediatamente acessível em cada cartão, reúne autoria literal, título quando documentado, licença e versão declaradas, origem, modificações históricas, fontes e avisos. Seu conteúdo é local e funciona offline; apenas abrir os sites externos requer rede. O formato tem a mesma proeminência para as diferentes autorias, inclusive CC BY 2.0, sem apresentar outros créditos fotográficos comparáveis mais destacados no cartão. A versão da licença não é inferida a partir do URL.

| Registros examinados | Apresentação | Fundamento e limites |
| --- | --- | --- |
| 18 fotos CC BY 2.0 | Crédito completo no diálogo adjacente | A [seção 4(b)](https://creativecommons.org/licenses/by/2.0/legalcode.en) admite atribuição razoável ao meio e exige proeminência equivalente aos créditos comparáveis. Nome, título fornecido, URI, licença e modificações são mantidos no mesmo diálogo. |
| 3 fotos CC BY 3.0 | Mesmo diálogo | A [seção 4(b)](https://creativecommons.org/licenses/by/3.0/legalcode.en) também admite apresentação razoável, preservando crédito e proeminência. |
| 51 fotos CC BY 4.0 | Mesmo diálogo | A [seção 3(a)(2)](https://creativecommons.org/licenses/by/4.0/legalcode.en#s3a) admite remissão contextual à informação exigida. |
| 42 fotos CC0 1.0 | Mesmo diálogo, conservando os créditos existentes | Não se atribui nova autoria ou condição. Os créditos continuam disponíveis. |
| Quatro fotos dos organismos em destaque com versão não comprovada | Crédito fornecido integralmente no diálogo; aviso curto de versão não comprovada permanece no cartão | As evidências individuais registram declaração CC BY e autoria, mas não demonstram a versão. A [FAQ oficial de atribuição](https://creativecommons.org/faq/#how-do-i-properly-attribute-material-offered-under-a-creative-commons-license) admite remissão à informação de crédito. Essa escolha de apresentação não comprova a versão, não concede direitos e não resolve as pendências de permissão/redistribuição. |
| Pedido de atribuição adicional específico, quando presente | Mantém atribuição visível e também o diálogo | O fallback permanece por condição documental específica, não simplesmente por a versão ser anterior à 4.0. |

As 114 fotos do adaptador incluem variantes exclusivas da Home: seu inventário foi examinado, mas esta correção só muda a apresentação dos cartões de Explorar. Nenhuma evidência atual desse conjunto contém texto adicional em `Attribution` ou `Restrictions`; `Credit` continua integral no diálogo. Os quatro destaques conservam os créditos fornecidos, incluindo símbolos de copyright e pseudônimos, sem abreviar ou normalizar autores. As evidências consultadas não registram exigência específica de posicionamento junto à foto. Avisos de versão não comprovada e indisponibilidade de fotografia continuam visíveis. Pedidos específicos são cobertos por fixture somente em memória; nenhuma atribuição ou permissão é inventada no catálogo.

O [inventário das decisões](politica-atribuicao.json) registra IDs, caminhos/hashes das evidências, versões declaradas e a decisão de apresentação por foto. É um registro técnico de interface, sem revisor ou aprovação jurídica/científica nova. Fontes normativas consultadas nesta rodada; a aplicação dos critérios ao meio é a interpretação documentada acima. Registros originais e gates de distribuição permanecem intactos.

## Área clicável e hover

O botão deixa de usar `Layout.fillWidth`. Sua largura corresponde ao texto e ao padding de `TextAction`, limitada pela largura disponível e alinhada à esquerda. Uma linha contenedora absorve cliques no espaço restante sem abrir o diálogo ou navegar. A fotografia e o restante da ação principal continuam abrindo o táxon; o teclado mantém Tab, Space, foco visível e retorno de foco após Escape/Fechar. O bloqueador da linha não impede o Flickable de assumir um gesto de rolagem.

O diagnóstico inicial reproduziu a expansão da área clicável: 346 px para uma largura implícita de 151 px. No ambiente software/offscreen não reproduziu alternância contínua de hover durante a permanência do cursor. A interação anterior dependia de dois `Button` aninhados com hover e animações próprios. Em Explorar, o cartão passa a observar a área inteira com um `HoverHandler` passivo e desativa o hover automático do botão externo; o `TextAction` continua com seus próprios estados e transição do Design System. Assim, a superfície do cartão não depende da arbitragem de hover entre os dois controles. Jogar conserva o comportamento anterior e `TextAction.qml` não é alterado.

## Verificação

**26/26 suítes CTest aprovadas em 416,22 s**, incluindo **61 casos QtTest de domínio e 17 de interface**, contando inicialização/limpeza. [Resultados e hashes](verificacoes/resultados.json), [CTest completo](verificacoes/ctest-results.txt), [domínio](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt) e [smoke real de produção com saída 0](verificacoes/smoke-test.txt).

Logs arquivados em UTF-8/LF; linhas emitidas na codificação Windows foram transcodificadas sem alterar seu conteúdo. O registro conserva também os hashes dos logs originais.

O teste de interação usa os componentes reais e registros locais: domínio, organismo em destaque, grupos cobrindo CC BY 2.0/3.0/4.0 e CC0, além de um pedido de atribuição específico sintético somente em memória. As 42 combinações (sete entradas × três resoluções × movimento normal/reduzido) verificam largura/alinhamento, permanência e pequenos movimentos do cursor, entrada/saída pelas quatro bordas, estabilidade dos sinais de hover/cores e dos pixels do botão após as transições. O organismo em destaque recebe 1,2 s de permanência em cada combinação. Dois pontos vazios da linha ficam inertes; clique no botão, Tab/Space, Escape, retorno de foco e navegação principal por fotografia/teclado permanecem independentes.

O [diagnóstico inicial](verificacoes/credits-baseline.txt) conserva a falha esperada de largura no código anterior e não constitui resultado da aceitação final. Os [testes direcionados após a correção](verificacoes/credits-targeted.txt) passaram (6/6, incluindo inicialização/limpeza), assim como a suíte completa acima. A piscada contínua não foi reproduzida antes da mudança no renderizador de software; os testes de estabilidade após isolar os estados passaram. Outras GPUs, escalas e leitores de tela não foram ensaiados.

Pesquisa ampla, filtros combinados, progressão em lotes, deduplicação, breadcrumbs, retorno com lotes/foco/rolagem, centralização fotográfica, fontes/links, Jogar, sessões e recuperação continuam aprovados. As regressões das PRs #30/#31, incluindo carrossel, timing e descoberta, passaram. Build Release, lint QML, escopo de licenciamento do código, status e metadados passaram. [Auditorias técnicas](verificacoes/technical-audits.json) sem erros e [governança](verificacoes/governance-history.json) com os 1.053 eventos preservados. O [gate de distribuição](verificacoes/distribution-summary.json) conserva os bloqueios anteriores, sem erros técnicos/de dependências.

As **18 novas capturas reais** (seis estados × três resoluções) usam a janela Qt, componentes/controlador de produção, SQLite temporário, Discord desativado e links interceptados nos testes. Ambiente: Windows 11, Qt 6.11.2, software/offscreen, escala 1 e 96 DPI; movimento reduzido somente para registrar imagens estáticas. O [manifesto](capturas.json) inclui dimensões, horários UTC e hashes. Sem montagem ou retoque. Capturas anteriores não foram substituídas. Foram inspecionados o cabeçalho/domínios, a galeria, os descendentes e os diálogos, com fotografias e textos preservados.

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Domínios e controles | [Captura](capturas/overview-1000x700.png) | [Captura](capturas/overview-1240x820.png) | [Captura](capturas/overview-1920x1080.png) |
| Organismos em destaque | [Captura](capturas/gallery-1000x700.png) | [Captura](capturas/gallery-1240x820.png) | [Captura](capturas/gallery-1920x1080.png) |
| Grupos descendentes | [Captura](capturas/descendants-1000x700.png) | [Captura](capturas/descendants-1240x820.png) | [Captura](capturas/descendants-1920x1080.png) |
| Créditos CC BY 4.0 e autoria extensa | [Captura](capturas/contextual-credits-1000x700.png) | [Captura](capturas/contextual-credits-1240x820.png) | [Captura](capturas/contextual-credits-1920x1080.png) |
| Histórico e apresentação atual | [Captura](capturas/credit-history-1000x700.png) | [Captura](capturas/credit-history-1240x820.png) | [Captura](capturas/credit-history-1920x1080.png) |
| Créditos com versão não comprovada | [Captura](capturas/featured-credits-1000x700.png) | [Captura](capturas/featured-credits-1240x820.png) | [Captura](capturas/featured-credits-1920x1080.png) |

```powershell
cmake -S . -B build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:/glossata_game/build/2.2.2-creditos
cmake --build build -j 3
python scripts/capture_home.py --binary build/2.2.2-creditos/projeto_glossata.exe --section discovery
ctest --test-dir build --output-on-failure
$qmlFiles = @(rg --files resources/qml -g '*.qml')
qmllint @qmlFiles --unqualified disable
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/release.py check
```

O diretório de binários preserva a instância anterior aberta no Windows. A captura oficial da Home foi regenerada após recompilar o C++ compartilhado: a imagem permaneceu byte a byte igual, atualizando somente o registro de fonte/binário. O aviso preexistente do ciclo de vida da Home continua [documentado na rodada 1](../rodada-1/verificacoes/preexisting-home-context.txt), fora destas três correções.

## Limites preservados

Sem alterações no conteúdo científico, arquivos fotográficos, metadados, permissões, licenças, critérios de elegibilidade, governança ou bancos pessoais. Pesquisa, filtros, carregamento progressivo, breadcrumbs, restauração de navegação, outras telas e sistemas do jogo permanecem no escopo já implementado. Sem outra PR, merge, release, tag ou avanço para 2.2.3. A criação/revisão de descrições completas continua reservada à Etapa 3.
