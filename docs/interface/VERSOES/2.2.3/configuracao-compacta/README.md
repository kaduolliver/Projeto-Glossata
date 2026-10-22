# PR #33 — configuração compacta e fontes contextuais

Revisão pontual sobre `efa28f0888b9c22b82b58837ecac11eb13bbd4cc`, na mesma branch `feat/issue-22-play-2.2.3` e [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33). Aplicativo 1.9.0 ainda não lançado; catálogo 2.4.0 preservado. Sem merge, tag, release ou avanço para 2.3.1.

Implementação e testes em [b5da446](https://github.com/kaduolliver/Projeto-Glossata/commit/b5da446bf13b853f7650fd7ce3653430f17d5325). O mesmo commit de fontes e os mesmos executáveis sustentam a suíte completa, as execuções direcionadas e as capturas finais; seus hashes ficam nos manifestos.

## Composição e fontes

O painel de identificação usa título discreto, margens de 12 pixels lógicos e altura calculada pelo conteúdo. Com pelo menos 820 pixels úteis, seletor, dificuldade atual e início ficam lado a lado; nas janelas compactas, o início ocupa a segunda linha. O conjunto do seletor tem largura máxima de 360 pixels. A quantidade elegível e as alternativas distintas aparecem abaixo, com quebra de linha se necessária. Não há altura fixa nem nova seleção de dificuldade. Modelo de níveis, disponibilidade e chamada de início permanecem intactos.

Um único botão **Fonte, origem e créditos** substitui os dois links separados da configuração. O diálogo local organiza identificação taxonômica, enquadramento científico, referência educativa e proveniência. Mostra nome/nível/ID, origem/ID na origem, referência e URL existentes, responsável/data/tipo/estado/limitações da revisão quando registrados, motivo, escopo, hash e evidências disponíveis. Dados ausentes são informados sem criar autoria, referência, revisão ou aprovação. As informações são locais e consultáveis offline; apenas abrir os links externos depende de conexão.

Os avisos de enquadramento e revisão continuam visíveis na página. A configuração não mostra uma fotografia própria; os cartões descendentes mantêm seus diálogos e atribuições fotográficas independentes. Não há nova licença nem alteração da política de atribuição. Esc e Fechar devolvem o foco ao botão contextual; os links acompanham a rolagem do diálogo sem deslocar a página de fundo.

## Botão Voltar e preservação

A margem superior do botão protege sua moldura. O timer local `positionPlayTop` posiciona a página taxonômica de Jogar no topo após a abertura ou mudança do grupo e cancela eventual inércia anterior, aguardando o layout. Isso também cobre a navegação entre descendentes e breadcrumbs na mesma instância. O ajuste não modifica o estado `playPresentation` nem o mecanismo de restauração: `Main.qml` e `CategoryView.qml` permanecem byte a byte iguais à base desta revisão. O timer não reposiciona Explorar.

Somente `TaxonomyView.qml` e os testes de interface recebem alterações funcionais. Código C++ de produção, componentes compartilhados, dados científicos, fotografias, licenças, governança e outras telas permanecem intactos. A Home oficial não precisa de nova captura, pois suas fontes e aparência não mudam nesta rodada. As [evidências de retorno à seleção](../refinamentos-navegacao/README.md) e da [entrega inicial](../README.md) são preservadas.

## Validação

**26/26 suítes CTest aprovadas**, em **507,66 s** de tempo real: 62 casos QtTest de domínio, 21 de interface, 32 de identidade e 21 de recuperação, incluindo preparação/limpeza. [Log completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt) e [resultados com proveniência](verificacoes/resultados.json). A execução inclui as regressões das PRs #30/#31/#32, pesquisa, filtros, paginação, breadcrumbs, créditos, fotografias, Início, Quiz, pontuação e sessões.

`compactPlayConfigurationAndContextualSources` percorre os três tamanhos de janela, todas as opções de identificação disponíveis em Animalia, dificuldade previamente selecionada, grupos vazio/limitado e a troca para modelos menores. Confere alinhamento, ausência de sobreposição, largura limitada do seletor, contador abaixo dos controles e altura do painel derivada do conteúdo. A guarda do índice do contador evita acessar o nível anterior enquanto o modelo muda de grupo; não interfere na seleção ou elegibilidade.

O teste abre o diálogo por mouse e teclado, confere os dados contra os mapas do controlador e intercepta a ativação real de URLs, sem abrir navegador ou exigir rede. Percorre os links da referência e das evidências, Tab/Backtab, Esc, Fechar e restauração do foco. Exercita transições normais e redução de movimento. Verifica o botão Voltar integralmente visível na abertura, após rolagem e seleção de descendente, e após navegar por breadcrumb. O teste de preservação de contexto anterior continua cobrindo as três abas, rolagem, foco e sessão recuperável; o teste da seleção continua verificando início Livre, Competitivo e misto.

A execução [direcionada](verificacoes/play-compact-targeted.txt) passou com cinco casos, incluindo preparação/limpeza. As execuções adicionais do novo teste em [125%](verificacoes/play-compact-dpi-125.txt) e [150%](verificacoes/play-compact-dpi-150.txt) passaram com três casos cada. Build Release, lint QML, escopo de licenciamento do código, metadados/histórico e [smoke real do executável](verificacoes/smoke-test.txt) passaram.

As [auditorias técnicas](verificacoes/technical-audits.json) preservam 122 organismos, 122 registros fotográficos válidos, 114 fotos de exploração, 533 vínculos e 623 cartões. A comparação com a base conserva os [1.053 eventos de governança](verificacoes/governance-history.json). O [resumo do gate de distribuição](verificacoes/distribution-summary.json) é idêntico ao da revisão anterior, sem erros técnicos/de dependências; as pendências de permissões e revisão permanecem.

## Capturas reais

São **30 novos PNGs**: configuração, diálogo no topo, proveniência, abertura de descendente com Voltar visível, grupo vazio e grupo limitado em cada resolução a 100%, mais configuração/diálogo nas três resoluções a 125% e 150%. Os demais quadros produzidos nas escalas adicionais continuam no diretório de build. O [manifesto](capturas.json) registra fontes, executáveis, dimensões, fator Qt, horário, táxon, rolagem e hashes. Todas as imagens vêm de `QQuickWindow::grabWindow`, com QML e controlador de produção, SQLite temporário e Discord desativado; sem montagem ou retoque.

| Resolução lógica | Configuração | Fontes | Proveniência | Voltar | Sem partida |
| --- | --- | --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/configure-1000x700.png) | [Abrir](capturas/sources-top-1000x700.png) | [Abrir](capturas/sources-provenance-1000x700.png) | [Abrir](capturas/return-top-1000x700.png) | [Vazio](capturas/empty-group-1000x700.png) / [limitado](capturas/limited-group-1000x700.png) |
| 1240×820 | [Abrir](capturas/configure-1240x820.png) | [Abrir](capturas/sources-top-1240x820.png) | [Abrir](capturas/sources-provenance-1240x820.png) | [Abrir](capturas/return-top-1240x820.png) | [Vazio](capturas/empty-group-1240x820.png) / [limitado](capturas/limited-group-1240x820.png) |
| 1920×1080 | [Abrir](capturas/configure-1920x1080.png) | [Abrir](capturas/sources-top-1920x1080.png) | [Abrir](capturas/sources-provenance-1920x1080.png) | [Abrir](capturas/return-top-1920x1080.png) | [Vazio](capturas/empty-group-1920x1080.png) / [limitado](capturas/limited-group-1920x1080.png) |

| Resolução lógica | Qt 125% | Qt 150% |
| --- | --- | --- |
| 1000×700 | [Configuração](capturas/configure-1000x700-125.png) / [fontes](capturas/sources-top-1000x700-125.png) | [Configuração](capturas/configure-1000x700-150.png) / [fontes](capturas/sources-top-1000x700-150.png) |
| 1240×820 | [Configuração](capturas/configure-1240x820-125.png) / [fontes](capturas/sources-top-1240x820-125.png) | [Configuração](capturas/configure-1240x820-150.png) / [fontes](capturas/sources-top-1240x820-150.png) |
| 1920×1080 | [Configuração](capturas/configure-1920x1080-125.png) / [fontes](capturas/sources-top-1920x1080-125.png) | [Configuração](capturas/configure-1920x1080-150.png) / [fontes](capturas/sources-top-1920x1080-150.png) |

Para repetir, configure/compile o projeto com Qt e execute `ctest --test-dir build --output-on-failure`. Nesta revisão, os executáveis ficam em `build/2.2.3-compacto/`. Para o teste direcionado, a partir de `build/tests`, execute `../2.2.3-compacto/glossata_ui_tests.exe compactPlayConfigurationAndContextualSources`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. As imagens são geradas em `build/tests/previews/play-compact-2.2.3/<escala>/`.

## Limitações

Windows 11/Qt 6.11.2 software/offscreen, eventos de mouse/teclado pelo QtTest. Os fatores Qt ampliam o raster e conservam a resolução lógica; não alteramos a escala física do Windows nem ensaiamos outras GPUs, touchpad físico ou leitores de tela. Informações locais continuam disponíveis sem internet; seguir URLs externas precisa de conexão. Testes técnicos não aprovam conteúdo científico ou permissões. Permanecem o gate de distribuição com pendências anteriores e o diagnóstico histórico da Home com frame zero.
