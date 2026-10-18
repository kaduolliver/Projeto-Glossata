# Projeto Glossata

**Explore a diversidade da vida.**

> **Transparência:** há conteúdo legado sem revisão humana rastreável e permissões de redistribuição ainda incompletas. Testes não certificam conformidade jurídica ou científica. Consulte o [plano e a matriz de ação](docs/CONFORMIDADE.md), os [direitos por camada](docs/DIREITOS_E_LICENCAS.md) e o [canal de correção/remoção](docs/REPORTAR_PROBLEMA.md).

> Uma iniciativa educacional independente, gratuita e sem fins lucrativos, dedicada à divulgação científica e ao aprendizado sobre a biodiversidade.

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt-6-41CD52)
![SQLite](https://img.shields.io/badge/SQLite-local-003B57)
![Plataforma](https://img.shields.io/badge/plataforma-Windows-informational)

## Sobre o projeto

O **Projeto Glossata** nasceu como um jogo educativo de identificação de organismos e está evoluindo para uma **plataforma de educação e exploração da natureza**. Seu propósito é reunir informação científica acessível, recursos visuais e atividades interativas para estimular a curiosidade e tornar o estudo da biodiversidade mais envolvente.

O nome *Glossata* faz referência a um clado de lepidópteros que inclui a maioria das borboletas e mariposas — uma homenagem à origem do projeto, **não uma restrição ao conteúdo**. A iniciativa pretende abordar a diversidade da vida em diferentes grupos biológicos, suas características, relações evolutivas e papéis nos ecossistemas.

A aplicação disponível atualmente é o **Projeto Glossata**, um programa desktop em português, com catálogo local de organismos, exploração taxonômica, fichas educativas, desafios de identificação e acompanhamento do desempenho. A visão de longo prazo é integrar esses recursos a uma **enciclopédia interativa da natureza**.

![Tela inicial real do Projeto Glossata, com navegação lateral, convite para explorar a biodiversidade e cartões de descoberta.](docs/images/tela-inicial.png)

*Captura do aplicativo em execução, com perfil temporário. [Origem da captura](docs/images/tela-inicial.json) · [Como atualizar a captura e preparar lançamentos](docs/VERSIONAMENTO.md).*

![Quiz real do Projeto Glossata, com fotografia de um leão, quatro alternativas de identificação do gênero, pontuação possível e botão de dica.](docs/images/quiz.png)

*Quiz em execução com perfil temporário. Fotografia: Patrick Jackson, conforme o crédito da fonte; licença declarada [CC BY](https://creativecommons.org/licenses/by/4.0/), com versão ainda não comprovada. [Foto original](https://www.inaturalist.org/photos/748172022) · [Origem da captura e atribuição](docs/images/quiz.json). A proveniência legada da foto mantém as [pendências documentadas](docs/RELATORIO_CONFORMIDADE.md).*

### Missão

Democratizar o acesso ao conhecimento sobre a biodiversidade por meio de ferramentas gratuitas, conteúdo com referências verificáveis e experiências interativas de aprendizagem.

### Princípios

- **Acesso ao conhecimento:** proposta educacional gratuita e sem finalidade lucrativa.
- **Rigor e transparência:** identificação das fontes, das limitações e dos estágios de curadoria.
- **Aprendizagem ativa:** uso de exploração, observação, questões e feedback para consolidar conceitos.
- **Respeito à biodiversidade:** incentivo à compreensão da diversidade biológica e de sua conservação.
- **Evolução contínua:** aperfeiçoamento gradual de dados, recursos e acessibilidade.

## Estado atual

<!-- glossata:status:start -->
<!-- Gerado por scripts/project_status.py; não editar manualmente. -->
| Informação atual | Valor derivado | Fonte |
| --- | --- | --- |
| Aplicativo | **1.7.0** — em desenvolvimento; não lançado | [VERSION.txt](VERSION.txt), [CHANGELOG](CHANGELOG.md) |
| Catálogo | **2.4.0** — não lançado | [Manifesto](resources/data/catalog_manifest.json), [histórico](docs/CATALOGO_CHANGELOG.md) |
| Regra padrão de pontuação | `identification-v4-progressive` | [Hint.h](src/domain/Hint.h) |
| Leitura histórica de pontuação | `identification-v3-assistance`, `identification-v2` e `legacy` | [Hint.h](src/domain/Hint.h) |
| Competitivo | 20 questões | [GameConfiguration.h](src/domain/GameConfiguration.h) |
| SQLite e snapshots | histórico 6; catálogo 2; checkpoint 1 | [Fontes e verificação](docs/ESTADO_DO_PROJETO.md) |
| Organismos e imagens do quiz | 122 registros; 120 arquivos distintos | [Catálogo](resources/data/species.json) |
| Domínios | Archaea: 0; Bacteria: 20; Eukaryota: 102 | [Taxonomia](resources/data/taxonomy.json) |
| Testes registrados | **26 suítes CTest**; não representa resultado de execução | [Inventário gerado](docs/PROJECT_STATUS.json), [CMake](tests/CMakeLists.txt) |

Valores derivados das fontes locais. Este bloco não comprova publicação, execução da CI ou aprovação do gate de distribuição.
<!-- glossata:status:end -->

O [estado verificável e suas fontes](docs/ESTADO_DO_PROJETO.md) explicam a geração deste bloco. Consulte também o [procedimento de versionamento](docs/VERSIONAMENTO.md). Registro de testes, versão em arquivo e entrada de changelog não comprovam publicação ou aprovação da distribuição.

A [auditoria de estabilização](docs/AUDITORIA_ESTABILIZACAO.md) registra as verificações integradas de migração, recuperação, backup e governança, as correções reproduzidas e os testes manuais pendentes. A [proteção da `main`](docs/PROTECAO_REPOSITORIO.md) foi confirmada ativa por consulta datada; isso não libera a distribuição dos recursos com pendências.

**Em desenvolvimento.** O produto funcional atual é um aplicativo desktop, **não** uma enciclopédia web completa. Os recursos descritos abaixo correspondem ao estado documentado do repositório em outubro de 2026; a visão futura está separada na seção [Roadmap](#roadmap).

| Recurso | Situação |
| --- | --- |
| Interface desktop com Qt Quick/QML | Implementado |
| Exploração de grupos e níveis taxonômicos | Implementado |
| Quiz com quatro alternativas e feedback | Implementado |
| Modos Livre e Competitivo | Implementado |
| Dicas estratégicas com pontuação proporcional | Implementado |
| Fichas educativas e fontes para organismos do catálogo | Implementado; curadoria em evolução |
| Perfis, histórico local, estatísticas e rankings | Implementado |
| Catálogo e funcionamento offline | Implementado |
| Enciclopédia ampla com pesquisa avançada | Visão futura |
| Trilhas de aprendizagem e atlas interativo | Visão futura |
| Plataforma web e colaboração pública | Possibilidades futuras, sem cronograma definido |

## Biodiversidade e classificação

O catálogo atual utiliza **Bacteria, Archaea e Eukaryota** como portas didáticas para explorar linhagens evolutivas:

As contagens por domínio e o total são derivados do catálogo no bloco [Estado atual](#estado-atual).

Eukaryota oferece **Opisthokonta** (incluindo Animalia e Fungi), **Archaeplastida** (Plantae no sentido da fonte), **SAR** (em Stramenopiles), **Amoebozoa** e **Discoba** (ainda sem representantes). Supergrupo designa um clado de navegação sem rank formal universal; domínio, reino e os demais níveis permanecem distintos. Grupos vazios continuam exploráveis e não geram quizzes.

> **Nota científica:** três domínios são uma organização didática; hipóteses de dois domínios situam os eucariotos dentro de Archaea. A navegação omite ramos intermediários e não representa uma filogenia completa ou definitivamente resolvida. Monera e Protista ficam apenas nas referências históricas e na compatibilidade dos dados. O enquadramento consulta Adl et al. (2019), Burki et al. (2020) e estudos de Asgard de 2025–2026, com divergências e fontes registradas na [documentação científica e de migração](docs/CLASSIFICACAO_FILOGENETICA.md).

Os **122 enquadramentos novos permanecem rascunhos assistidos por IA**, sem simular revisão por especialista. A classificação original do iNaturalist está preservada por organismo e tem revisão separada da síntese filogenética. [Auditoria dos 122 registros](docs/AUDITORIA_FILOGENETICA.json).

## Funcionalidades implementadas

### Rede de provedores científicos

Ferramentas de manutenção integram **GBIF, Catalogue of Life, WoRMS e Wikimedia Commons**, com contratos independentes, cache offline, reconciliação de nomes/sinônimos, prevenção de duplicatas por identidade externa e registro de divergências. A descoberta prioriza linhagens pouco representadas. As consultas produzem candidatos em banco separado, com revisão científica e jurídica pendente; não alteram automaticamente os 122 organismos ou o jogo.

NCBI Taxonomy, LPSN, POWO e MycoBank têm etapas e restrições documentadas para integração futura. A [arquitetura, condições de acesso e procedimentos](docs/PROVEDORES_CIENTIFICOS.md) distinguem provedores implementados de planejados; os [recibos de consultas reais e replay offline](docs/VALIDACAO_PROVEDORES.json) não são aprovação científica ou jurídica.

### Exploração taxonômica

Navegação por domínios, linhagens, grupos e subgrupos, com caminhos clicáveis, fontes, limitações e níveis de identificação disponíveis conforme a cobertura do catálogo. Ranks ausentes não são inferidos. Animalia, Plantae e Fungi têm acesso rápido, preservando seus caminhos em Opisthokonta ou Archaeplastida. Os controles de quiz mantêm a posição anterior à expansão fotográfica; os grupos filhos aparecem depois dos controles. Os cartões integram fotografias reais e micrografias identificadas ao fundo, em 42% da largura e toda a altura, com recorte proporcional e gradiente horizontal. A auditoria cobre os 623 cartões: 534 com associação fotográfica e 89 com pendência justificada, usando 109 imagens offline; grupos sem foto adequada conservam a ilustração. Autoria, origem, licenças por arquivo e limites estão nos [créditos](docs/PHOTO_CREDITS.md) e na [documentação da exploração](docs/EXPLORACAO_E_FOTOGRAFIAS.md). Os níveis de quiz exigem revisão adequada, aprovação fotográfica no nível e quatro alternativas distintas. Domínio, supergrupo e reino estão sem quiz elegível nesta etapa; os demais níveis disponíveis continuam funcionando.

![Tela real da exploração do Projeto Glossata com filtros restaurados e fotografias integradas aos cartões dos três domínios](docs/images/exploracao.png)

[Créditos das fotografias](docs/PHOTO_CREDITS.md) · [Registro da captura real](docs/images/exploracao.json).

### Projeto Glossata

Cada questão apresenta quatro alternativas de identificação no mesmo nível taxonômico, seleção, confirmação e feedback. A fotografia e os dados utilizados na resposta são preservados para a etapa de revisão.

- **Modo Livre:** exploração contínua, com filtros e diferentes políticas de alternativas; a partida termina quando o jogador decide encerrá-la.
- **Modo Competitivo:** quantidade fixa de questões indicada no bloco Estado atual, sem repetição de organismos na mesma partida, com condições padronizadas, regras de elegibilidade e ranking específicas.

### Dicas estratégicas e pontuação

O quiz busca três dicas progressivas — leve, moderada e forte — conforme as fontes, o nível e as alternativas. A categoria é independente da posição:

| Categoria | Assunto |
| --- | ---: |
| Taxonômica | Contexto da classificação |
| Etimológica | Origem do nome |
| Morfológica | Aspectos físicos |

A **assistência daquela dica e questão**, avaliada separadamente da categoria, define o teto:

| Assistência | Unidades | Teto do acerto |
| --- | ---: | ---: |
| Leve | 6 | 1 ponto |
| Moderada | 3 | 0,5 ponto |
| Forte | 2 | 1/3 de ponto |

Abrir o painel de dicas não reduz a pontuação. Quando mais de uma dica é revelada, vale o **menor teto**; a interface anuncia o teto resultante antes de revelar. Uma tradução reconhecível pode oferecer assistência forte, enquanto uma descrição já evidente na foto pode oferecer assistência leve. O cálculo permanece em unidades inteiras e acertos/precisão são independentes dos pontos.

A versão de testes usa **avaliação pedagógica automatizada** (`approved_automated`, política `progressive-pedagogy-v1`), identificada na interface e independente da revisão científica. As 290 propostas anteriores são preservadas como rascunhos. Receitas locais para os 122 organismos reutilizam fontes existentes; o contexto determina quais dicas podem aparecer. Não se inventa uma terceira dica quando falta evidência. Categorias podem repetir com conteúdo distinto, mantendo escolha direta de qualquer dica e o menor teto. Consulte a [política e os limites](docs/DICAS_E_PONTUACAO.md), a [auditoria das avaliações](docs/AUDITORIA_ASSISTENCIA.json) e a [cobertura e lacunas](docs/AUDITORIA_PROGRESSAO_DICAS.json). **Relatar problema nesta dica** prepara um rascunho local; somente uma ação explícita abre o formulário externo. Respostas antigas conservam os pontos; novas partidas competitivas usam a regra indicada no bloco Estado atual, separada das políticas anteriores internamente no mesmo ranking.

### Fichas educativas

Os 122 organismos ativos têm fichas em português com informações como apresentação, características morfológicas, distribuição ou ocorrência documentada, conservação e curiosidades; informações sobre alimentação ou nutrição aparecem quando documentadas. As fichas disponibilizam classificação e referências, com cobertura e revisão ainda em evolução.

### Imagens e proveniência

O catálogo possui 122 registros ativos e 120 arquivos distintos de imagem. Existe validação técnica de arquivos, fonte, créditos, licença, identidade e integridade. **Nem toda imagem é evidência suficiente para identificar uma espécie:** as imagens de apoio dos 20 registros bacterianos são sinalizadas como tal, e a elegibilidade visual varia conforme o nível taxonômico e a revisão.

### Progresso e perfis

- Perfis locais com identidade permanente: renomear mantém a propriedade dos resultados, excluir preserva o histórico e reutilizar um nome cria outro jogador. A [migração para IDs](docs/IDENTIDADE_JOGADORES.md) mantém resultados pré-ID como legado de vínculo pendente, sem atribuí-los automaticamente por coincidência de nome.
- [Nomes com política Unicode versionada](docs/NOMES_UNICODE.md): `João`/`JOÃO` e formas compostas/decompostas não geram novos perfis duplicados. Acentos e grafia de exibição são preservados; colisões antigas continuam separadas por ID, sem fusão automática.
- Histórico de respostas e partidas, métricas de precisão e uso de dicas.
- [Salvamento e recuperação](docs/RECUPERACAO_PARTIDAS.md): checkpoints após respostas/dicas, continuação ou abandono ao reabrir e finalização idempotente. Fechar a janela suspende a partida; sessões competitivas incompletas não atualizam o ranking. Inclui ferramenta de backup e restauração verificados.
- Rankings locais com distinção entre recorde e última partida.
- Conquistas e estatísticas associadas à experiência disponível.
- Persistência em SQLite e possibilidade de usar o aplicativo sem internet após a preparação dos recursos.

### Discord (opcional)

Integração local com o Discord Rich Presence, configurável e desativável, sem exigir conexão para as funções principais do jogo.

## Tecnologias

| Camada | Tecnologias |
| --- | --- |
| Linguagem principal | C++17 |
| Interface | Qt 6 Quick / QML |
| Persistência | SQLite / Qt SQL |
| Compilação | CMake, Ninja, GCC via MSYS2 UCRT64 |
| Importação e auditoria de dados | Python 3.11+ |
| Testes | CTest, Qt Test e testes auxiliares em Python |
| Dados e recursos | JSON, imagens e recursos incorporados ao aplicativo |

### Organização do repositório

```text
Projeto-Glossata/
├── docs/                 # Documentação técnica, científica e de curadoria
├── package/              # Recursos e scripts de distribuição para Windows
├── resources/            # Dados, imagens, QML e elementos visuais
├── scripts/              # Importação, auditoria e empacotamento
├── src/                  # Código-fonte C++
├── tests/                # Testes automatizados
├── CMakeLists.txt        # Configuração da compilação
├── resources.qrc         # Manifesto de recursos Qt
├── PROJECT_CONTEXT.md    # Contexto e decisões técnicas do projeto
└── README.md
```

A arquitetura separa regras de domínio, serviços de aplicação, interfaces de persistência, adaptadores de infraestrutura e apresentação QML. O catálogo e o histórico são mantidos localmente, e o acesso a fontes externas é concentrado nos processos de preparação e atualização dos dados.

## Compilação e testes no Windows

### Pré-requisitos

- Windows;
- **MSYS2 UCRT64** com compilador C++ compatível com C++17;
- **CMake** e **Ninja**;
- **Qt 6** com os componentes Core, Gui, Quick, Sql, Network e Test;
- **Python 3.11+** para os scripts auxiliares.

Garanta que o terminal utilizado tenha acesso aos executáveis e bibliotecas instalados no ambiente UCRT64.

No PowerShell, ajuste o caminho à sua instalação antes dos comandos seguintes:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
```

### Clonar e compilar

```bash
git clone https://github.com/kaduolliver/Projeto-Glossata.git
cd Projeto-Glossata
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
python scripts/project_status.py check --build build
cmake --build build --parallel 4
```

### Executar os testes

```bash
ctest --test-dir build --output-on-failure
python scripts/audit_catalog.py --strict
python scripts/release.py check
```

Para verificar arquivos QML no PowerShell, com `qmllint` disponível no PATH:

```powershell
$qmlFiles = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
qmllint @qmlFiles --unqualified disable
```

Os testes usam bancos temporários e não publicam presença real no Discord. A contagem acima é de suítes registradas; consulte a saída do CTest e os relatórios em `build/tests/` para resultados de uma execução. Para executar o binário compilado no mesmo terminal UCRT64, use `./build/projeto_glossata.exe`; perfis reais só devem ser abertos para uso normal, nunca como fixtures de teste.

### Empacotar para Windows

No PowerShell, após compilar:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

O processo prepara os componentes Qt e as dependências de execução para o pacote local. **O repositório não inclui executáveis e bibliotecas geradas:** um clone novo precisa ser compilado. A validação em uma instalação Windows completamente limpa ainda é um ponto a verificar.

Esse comando é o fluxo de distribuição e falha enquanto houver permissões ou obrigações não demonstradas. Para testar localmente, use `powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Development`; o pacote fica identificado como desenvolvimento local e inclui créditos, evidências e pendências offline. Isso não autoriza redistribuí-lo.

Inicie pelo atalho `package/Projeto Glossata.lnk` ou por `package/run_glossata_game.cmd`. O executável é `package/projeto_glossata.exe`; os perfis e o histórico existentes mantêm o mesmo diretório de dados.

## Catálogo, atualização e curadoria

O [processo de admissão e prontidão para expansão](docs/PRONTIDAO_EXPANSAO_CATALOGO.md) reúne requisitos por organismo, direitos por recurso, revisões editoriais e propostas offline. A preparação é testada com fixtures sintéticas; os bloqueios documentais existentes continuam ativos e nenhuma nova espécie foi admitida nesta etapa.

A aplicação **não depende de consultas à internet durante o jogo**. Os scripts de importação utilizam fontes externas, incluindo o iNaturalist, para preparar dados e recursos locais. A proveniência e os direitos de uso das imagens devem ser respeitados.

Para inspecionar provedores e cobertura sem rede e sem modificar o catálogo:

```bash
python scripts/scientific_catalog.py providers
python scripts/scientific_catalog.py coverage
```

Novas descobertas usam `scientific_catalog.py discover`, offline por padrão e com rede somente mediante `--online`, produzindo candidatos pendentes em `build/scientific-sources/`. Consulte os parâmetros e restrições em [PROVEDORES_CIENTIFICOS.md](docs/PROVEDORES_CIENTIFICOS.md). O importador legado não é o caminho para expandir o catálogo: a expansão direta é recusada antes de rede/escrita. Reimportações compatíveis requerem cache e curadoria; seus agrupamentos históricos e alias `--per-kingdom` não definem grupos naturais na aplicação. Regenerar dados/educação/créditos é manutenção de conteúdo, não pré-requisito de uma compilação normal do clone.

Para executar a auditoria local de catálogo:

```bash
python scripts/audit_catalog.py --strict
```

A curadoria distingue aprovação de identificação visual, fotografia, etimologia, morfologia e informação educativa. Conteúdos não revisados não devem ser apresentados como evidência conclusiva de identificação. Consulte os relatórios em [`docs/`](docs/) para conhecer cobertura, fontes, licenças, limitações e pendências.

Aprovações antigas sem responsável e evidência foram classificadas como legadas. Novo conteúdo, inclusive assistido por IA, entra em rascunho; alterações exigem nova revisão registrada. O relatório de atualização é offline por padrão (`python scripts/review_updates.py`); `--online` gera propostas para análise humana, sem substituir o conteúdo publicado.

O gerenciamento de solicitações reutiliza essas auditorias: `python scripts/curation_requests.py plan` gera uma fila local com IDs, evidências e prioridades. Há registro de validação da integração com Issues e ativação semanal autorizada em 2026-10-16, com programação às segundas às 9h17 de Brasília, limitada a três criações e oito atualizações por rodada. Esse registro não é uma consulta do estado remoto atual. Há quatro formulários públicos; Projects permanece opcional. Fechar uma Issue nunca aprova um recurso. Veja [Curadoria automatizada](docs/CURADORIA_AUTOMATIZADA.md) para evidências, limitações, permissões e decisões humanas.

As cadeias SHA-256 verificam integridade interna, sem autenticar o responsável declarado. A comparação com uma base Git preservada acrescenta rastreabilidade histórica, sem certificar revisão científica. Consulte [garantias, âncoras e proposta de autenticação](docs/AUDITORIA_E_CONFIANCA.md).

`python scripts/review_updates.py --rights-report` prepara triagem por foto/material a partir dos mesmos validadores, offline por padrão. `--online` coleta metadados públicos como propostas; `--package package` avalia separadamente o pacote existente. Não cria permissões ou aprovações e mantém o gate de distribuição. Consulte [saneamento, evidências e decisões humanas](docs/SANEAMENTO_DIREITOS.md).

## Roadmap

A expansão abaixo expressa **objetivos e possibilidades**, e não funcionalidades já entregues nem compromissos de lançamento.

**Curto prazo — qualidade e aprofundamento do aplicativo**

- Ampliar a revisão científica das fichas, fotografias e dicas.
- Melhorar a representatividade dos grupos biológicos e a documentação das fontes.
- Fortalecer testes, acessibilidade, empacotamento e distribuição.
- Evoluir filtros, estatísticas e experiência de aprendizagem.

**Médio prazo — enciclopédia interativa**

- Introduzir busca e navegação dedicadas ao conteúdo enciclopédico.
- Ampliar páginas de organismos e de grupos taxonômicos, com referências e contextualização evolutiva.
- Conectar verbetes, relações taxonômicas e questionários em uma experiência integrada.
- Planejar trilhas de estudo e recursos didáticos estruturados.

**Longo prazo — ecossistema educacional**

- Avaliar uma versão web e acesso multiplataforma.
- Explorar mapas, habitats, distribuição geográfica e visualização da árvore da vida.
- Desenvolver processos de contribuição e revisão científica comunitária.
- Considerar conteúdos para educadores e diferentes níveis de aprendizagem.

## Documentação complementar

- [`docs/ESTADO_DO_PROJETO.md`](docs/ESTADO_DO_PROJETO.md) — fontes de verdade, geração, validação e limites do estado atual.
- [`PROJECT_CONTEXT.md`](PROJECT_CONTEXT.md) — contexto geral, arquitetura, regras e decisões técnicas.
- [`docs/CLASSIFICACAO_FILOGENETICA.md`](docs/CLASSIFICACAO_FILOGENETICA.md) — domínios, linhagens, fontes, divergências e migração.
- [`docs/AUDITORIA_FILOGENETICA.json`](docs/AUDITORIA_FILOGENETICA.json) — enquadramento rastreável dos 122 organismos.
- [`docs/FICHAS_ESPECIES.md`](docs/FICHAS_ESPECIES.md) — corpus das fichas educativas.
- [`docs/CURADORIA_AUTOMATIZADA.md`](docs/CURADORIA_AUTOMATIZADA.md) — fila de curadoria, Issues, formulários, rotina semanal, Projects e decisões humanas rastreáveis.
- [`docs/RECUPERACAO_PERMISSOES.md`](docs/RECUPERACAO_PERMISSOES.md) — persistência idempotente, recuperação de projeções e bloqueio de inconsistências de permissões.
- [`docs/CONCORRENCIA_GOVERNANCA.md`](docs/CONCORRENCIA_GOVERNANCA.md) — escrita serializada por APIs, CLIs e bridge, comparação do topo e testes de concorrência entre processos.
- [`docs/RELATORIO_EVOLUCAO_INTEGRADA.md`](docs/RELATORIO_EVOLUCAO_INTEGRADA.md) — auditoria Git/CI, cobertura fotográfica, avaliação automática das dicas e validação local em 1.6.0.
- [`docs/DICAS_E_PONTUACAO.md`](docs/DICAS_E_PONTUACAO.md) — critérios das dicas e sistema de pontos.
- [`docs/COBERTURA_TAXONOMICA.md`](docs/COBERTURA_TAXONOMICA.md) — cobertura e lacunas do catálogo.
- [`docs/PHOTO_CREDITS.md`](docs/PHOTO_CREDITS.md) — atribuição das imagens.
- [`docs/EVOLUCAO_COMPETITIVA.md`](docs/EVOLUCAO_COMPETITIVA.md) — regras da modalidade competitiva.

## Participação e contato

O projeto é atualmente mantido de forma independente. Ideias, correções e relatos de problemas podem ser apresentados pelas [Issues do repositório](https://github.com/kaduolliver/Projeto-Glossata/issues). Consulte [CONTRIBUTING.md](CONTRIBUTING.md) para Pull Requests, testes, fontes científicas e declaração de origem/direitos. Contribuições de código para incorporação devem ser compatíveis com Apache-2.0; não se exige cessão de copyright nem CLA.

## Direitos e licenças

**O código próprio expressamente enumerado está sob [Apache License 2.0](LICENSE)**, permitindo utilização, modificação e redistribuição, inclusive comercial, conforme seus termos. Criador original: **Carlos Eduardo Lima de Oliveira**. Repositório oficial: [kaduolliver/Projeto-Glossata](https://github.com/kaduolliver/Projeto-Glossata). Consulte [NOTICE](NOTICE) e o [escopo por arquivo e suas exclusões](docs/ESCOPO_LICENCIAMENTO.md).

O Projeto Glossata permanece experimental: a iniciativa oficial é educacional, científica, acessível, gratuita e sem fins lucrativos. Isso não acrescenta restrição não comercial à Apache. Fotografias mantêm licenças individuais; dados externos, textos educativos, documentação, identidade visual e bibliotecas têm tratamento separado. A [política editorial](docs/propostas/LICENCIAMENTO_TEXTOS.md) adota **CC BY 4.0 como padrão para redações educativas próprias elegíveis**, inclusive futuras, permitindo usos comerciais e adaptações mediante atribuição. A escolha não licencia automaticamente o corpus legado ou textos externos, que conservam suas condições; a organização da coletânea continua decisão independente. A lógica de `build_education.py` está enumerada sob Apache; a redação extraída para `group_education.json` mantém sua pendência de direitos. Consulte o [saneamento da Issue #2](docs/SANEAMENTO_ISSUE_2.md).

A licença do código **não autoriza automaticamente redistribuir mídias ou pacotes pendentes**, nem concede direitos de marca. Consulte os [direitos por camada](docs/DIREITOS_E_LICENCAS.md), o [inventário histórico de preparação](docs/INVENTARIO_LICENCIAMENTO.json), os [créditos](docs/PHOTO_CREDITS.md) e o [relatório de implementação](docs/RELATORIO_IMPLEMENTACAO_APACHE_2_0.md). Os gates de distribuição continuam ativos.

Os três ícones atuais compõem a [identidade visual oficial](docs/IDENTIDADE_VISUAL.md): mariposa estilizada e biodiversidade, com direção criativa do mantenedor e geração declarada via ChatGPT. A [política reservada](docs/propostas/USO_IDENTIDADE_VISUAL.md) não concede licença gráfica geral para reutilização por projetos independentes e se limita aos direitos efetivamente disponíveis; não comprova exclusividade ou regularização. O código continua Apache-2.0, e projetos derivados são orientados a utilizar identidade própria, respeitados visualização, fork no GitHub e usos legalmente permitidos. A intenção de uso oficial não libera a distribuição dos ícones enquanto houver pendências documentais.

---

**Projeto Glossata — Explore a diversidade da vida.**
