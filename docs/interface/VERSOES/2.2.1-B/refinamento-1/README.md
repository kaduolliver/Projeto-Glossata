# Refinamentos da PR #31

Base exata: `023942f10905cba377611c01f96ff9d286da1804`. Escopo exclusivo da Subetapa 2.2.1-B: rótulo e descrição do indicador de identificação por espécie, créditos contextuais da Descoberta do Dia, remoção do tooltip da fonte educativa e ícones maiores somente nos três indicadores da Home.

Telas afetadas: Início e seu novo diálogo contextual. `StatCard` recebe dimensões opcionais com os padrões anteriores. O diálogo e o funcionamento do carrossel, os critérios e a contagem do catálogo, as demais telas, os dados científicos e a governança permanecem preservados.

Implementação e capturas: commit pendente de criação, após as verificações abaixo. As capturas e os resultados iniciais da 2.2.1-B permanecem históricos na pasta superior. A entrega atualiza somente o [PR #31](https://github.com/kaduolliver/Projeto-Glossata/pull/31), sem merge ou lançamento.

## Quatro ajustes

1. **Identificação por espécie:** novo título e descrição explicitam que são organismos com foto elegível no nível espécie, excluindo o total de fotografias. O valor continua dinâmico em `identificationPhotos`; filtros, seleção diária e serviços C++ não mudam. Nesta execução, permanecem 122 organismos, dois domínios e 20 organismos elegíveis à identificação por espécie.
2. **Foto e créditos:** abre um diálogo da fotografia exibida na descoberta, com nome científico, autoria, licença declarada, versão separada, origem, modificações e links para foto/observação/termos. Os metadados acompanham a descoberta se a data mudar com o diálogo aberto. Versão ausente é “Não comprovada”, mesmo havendo URL de termos. Avisos de direitos e identificação permanecem explícitos; o rodapé oferece Todos os créditos e Fechar. Escape e Fechar devolvem foco ao botão de origem. O diálogo do carrossel é preservado byte a byte.
3. **Fonte educativa sem tooltip:** texto, URL acessível, clique, Tab/Espaço, hover e foco visual permanecem. Só as duas propriedades de tooltip foram removidas.
4. **Ícones maiores na Home:** `iconSize: 28` e `iconContainerSize: 48` nos três indicadores. Padrões de `StatCard` continuam em 23/42 px nas outras telas. Contêineres e ícones são centralizados; cores, fontes e espaçamentos existentes permanecem.

## Verificação em 20/10/2026

- Release recompilada; **3/3 suítes pertinentes aprovadas** em 103,28 s: domínio/aplicação, interface e metadados de release. [Domínio](verificacoes/core-results.txt): 56 casos aprovados. [Qt/QML](verificacoes/ui-test-results.txt): 11 casos aprovados, sem avisos, incluindo regressões completas da PR #30 e navegação/sessões.
- Nas três resoluções, foram verificados título/valor/descrição e nome acessível dos indicadores, dimensões reais e centros dos ícones, fonte visível sem tooltip por hover e teclado, abertura/fechamento/foco do diálogo, metadados e pendências, mudança de data com o diálogo aberto e acesso aos créditos gerais.
- As ativações reais dos links científicos, da fotografia, da observação e da licença foram interceptadas por `QDesktopServices::setUrlHandler` apenas no teste, conferindo as URLs sem abrir navegador ou acessar a rede. O aplicativo conserva `Qt.openUrlExternally` por ação explícita.
- Após o ajuste exclusivo do modo de captura do executável, a suíte de release foi repetida e passou (0,50 s); também passaram o smoke test real com banco temporário, a captura padrão e a captura rolada. A opção isolada sem modo de captura retorna 6 antes de abrir banco.
- Qmllint de todos os QML, escopo Apache, metadados do projeto e integridade de release/captura aprovados. Fontes de elegibilidade/seleção, dados, mídias e compliance idênticos à base. Sem novas aprovações, permissões ou alterações do catálogo. [Hashes dos logs e resultados](verificacoes/resultados.json).

## Capturas reais

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Home no topo, carrossel preservado | [Captura](capturas/home-1000x700.png) | [Captura](capturas/home-1240x820.png) | [Captura](capturas/home-1920x1080.png) |
| Descoberta e indicadores após rolagem | [Captura](capturas/discovery-1000x700.png) | [Captura](capturas/discovery-1240x820.png) | [Captura](capturas/discovery-1920x1080.png) |
| Créditos contextuais da descoberta | [Captura](capturas/credits-1000x700.png) | [Captura](capturas/credits-1240x820.png) | [Captura](capturas/credits-1920x1080.png) |

Esses nove quadros vêm da janela real de `dailyDiscoveryNavigationIndicatorsAndCaptures`, com QML/controlador de produção, SQLite temporário, Discord desativado e movimento desligado apenas para capturas estáticas. Data injetada é restaurada à data local real antes de capturar. [Manifesto](capturas.json): horários reais, dimensões, fontes/testes/binários, hashes e contexto observado; nenhum pixel foi montado ou retocado.

A captura inicial em repouso conserva a aparência anterior, pois os ajustes estão abaixo da área inicialmente visível. Foram arquivados também dois quadros independentes do **executável de produção**, ambos gerados por `scripts/capture_home.py`: [topo](capturas/home-production-1240x820.png), com [registro](captura-producao-inicial.json), e [Home rolada](capturas/home-production-discovery-1240x820.png), com [registro](captura-producao-descoberta.json). O segundo é a nova [imagem oficial do README](../../../../images/tela-inicial.png), com [manifesto próprio](../../../../images/tela-inicial.json). A imagem oficial não foi substituída por quadro do QtTest.

O modo opcional `--section discovery` solicita rolagem ao contêiner real da Home no próprio executável e aguarda o quadro antes de `grabWindow`. É restrito à captura isolada, sem efeito na execução normal, no carrossel ou nas partidas. O padrão continua capturando o topo. A opção e seu enquadramento ficam registrados no manifesto; os fontes do procedimento também entram nos hashes da captura.

```powershell
cmake --build build -j 4
ctest --test-dir build -R '^(GlossataTests|GlossataInterfaceTests|GlossataReleaseTests)$' --output-on-failure
python scripts/capture_home.py --binary build/2.2.1-B-verified/projeto_glossata.exe --section discovery
python scripts/release.py check
```

Condições: Windows 11, Qt 6.11.2, software/offscreen, escala 1 e 96 DPI. Outras GPUs, escalas e leitores de tela não foram ensaiados. Testes técnicos não certificam ciência, acessibilidade integral ou direitos de distribuição. Os bloqueios de distribuição existentes continuam preservados.
