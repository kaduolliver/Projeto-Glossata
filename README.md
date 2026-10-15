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

### Missão

Democratizar o acesso ao conhecimento sobre a biodiversidade por meio de ferramentas gratuitas, conteúdo com referências verificáveis e experiências interativas de aprendizagem.

### Princípios

- **Acesso ao conhecimento:** proposta educacional gratuita e sem finalidade lucrativa.
- **Rigor e transparência:** identificação das fontes, das limitações e dos estágios de curadoria.
- **Aprendizagem ativa:** uso de exploração, observação, questões e feedback para consolidar conceitos.
- **Respeito à biodiversidade:** incentivo à compreensão da diversidade biológica e de sua conservação.
- **Evolução contínua:** aperfeiçoamento gradual de dados, recursos e acessibilidade.

## Estado atual

Aplicativo **1.1.0 em desenvolvimento** · Catálogo **1.0.0, baseline ainda não lançada**. Consulte o [CHANGELOG](CHANGELOG.md), o [histórico científico e de licenças](docs/CATALOGO_CHANGELOG.md) e o [procedimento de versionamento](docs/VERSIONAMENTO.md). Os bloqueios de distribuição impedem uma tag/release nesta etapa; nenhuma versão publicada é declarada por estes números.

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

O catálogo atual utiliza os **cinco reinos do modelo didático tradicional** como portas de entrada para o aprendizado:

| Reino didático | Organismos ativos no catálogo |
| --- | ---: |
| Animalia | 42 |
| Plantae | 20 |
| Fungi | 20 |
| Protista | 20 |
| Monera | 20 |
| **Total** | **122** |

> **Nota científica:** a classificação tradicional dos cinco reinos é usada como organização pedagógica, não como representação definitiva da filogenia moderna. Especialmente *Monera* e *Protista* não constituem grupos naturais equivalentes aos demais nas classificações contemporâneas. O projeto mantém separadas a organização didática e a taxonomia científica registrada nas fontes, e pretende ampliar gradualmente a apresentação de relações evolutivas e sistemas atuais de classificação.

## Funcionalidades implementadas

### Exploração taxonômica

Navegação por reinos, grupos e subgrupos, com caminhos clicáveis, textos introdutórios e níveis de identificação disponíveis conforme a cobertura do catálogo. Os níveis de quiz são oferecidos apenas quando há alternativas suficientes para uma questão válida.

### Projeto Glossata

Cada questão apresenta quatro alternativas de identificação no mesmo nível taxonômico, seleção, confirmação e feedback. A fotografia e os dados utilizados na resposta são preservados para a etapa de revisão.

- **Modo Livre:** exploração contínua, com filtros e diferentes políticas de alternativas; a partida termina quando o jogador decide encerrá-la.
- **Modo Competitivo:** 20 questões em condições padronizadas, sem repetição de organismos na mesma partida, com regras de elegibilidade e ranking específicas.

### Dicas estratégicas e pontuação

O quiz oferece até três categorias de dicas, conforme a disponibilidade e a validação do conteúdo:

| Categoria | Pontuação máxima de um acerto |
| --- | ---: |
| Sem dica | 1 ponto |
| Taxonômica | 1 ponto |
| Etimológica | 0,5 ponto |
| Morfológica | 1/3 de ponto |

Abrir o painel de dicas não reduz a pontuação. Quando mais de uma dica é utilizada, vale o **menor teto** entre as categorias reveladas. Internamente, a pontuação é representada por unidades inteiras (6, 3 ou 2), para evitar erros de arredondamento na persistência e na comparação dos resultados. A disponibilidade das dicas depende da curadoria e das condições da questão.

### Fichas educativas

Os 122 organismos ativos têm fichas em português com informações como apresentação, características morfológicas, distribuição ou ocorrência documentada, conservação e curiosidades; informações sobre alimentação ou nutrição aparecem quando documentadas. As fichas disponibilizam classificação e referências, com cobertura e revisão ainda em evolução.

### Imagens e proveniência

O catálogo possui 122 registros ativos e 120 arquivos distintos de imagem. Existe validação técnica de arquivos, fonte, créditos, licença, identidade e integridade. **Nem toda imagem é evidência suficiente para identificar uma espécie:** as imagens de apoio de Monera são sinalizadas como tal, e a elegibilidade visual varia conforme o nível taxonômico e a revisão.

### Progresso e perfis

- Perfis locais de jogadores, com criação, renomeação, seleção e exclusão sob regras de integridade.
- Histórico de respostas e partidas, métricas de precisão e uso de dicas.
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

### Clonar e compilar

```bash
git clone https://github.com/kaduolliver/Projeto-Glossata.git
cd Projeto-Glossata
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

### Executar os testes

```bash
ctest --test-dir build --output-on-failure
```

Para verificar arquivos QML no PowerShell, com `qmllint` disponível no PATH:

```powershell
$qmlFiles = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
qmllint @qmlFiles --unqualified disable
```

### Empacotar para Windows

No PowerShell, após compilar:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

O processo prepara os componentes Qt e as dependências de execução para o pacote local. **O repositório não inclui executáveis e bibliotecas geradas:** um clone novo precisa ser compilado. A validação em uma instalação Windows completamente limpa ainda é um ponto a verificar.

Esse comando é o fluxo de distribuição e falha enquanto houver permissões ou obrigações não demonstradas. Para testar localmente, use `powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Development`; o pacote fica identificado como desenvolvimento local e inclui créditos, evidências e pendências offline. Isso não autoriza redistribuí-lo.

Inicie pelo atalho `package/Projeto Glossata.lnk` ou por `package/run_glossata_game.cmd`. O executável é `package/projeto_glossata.exe`; os perfis e o histórico existentes mantêm o mesmo diretório de dados.

## Catálogo, atualização e curadoria

A aplicação **não depende de consultas à internet durante o jogo**. Os scripts de importação utilizam fontes externas, incluindo o iNaturalist, para preparar dados e recursos locais. A proveniência e os direitos de uso das imagens devem ser respeitados.

Exemplos de importação, na raiz do projeto:

```bash
python scripts/import_inaturalist.py --per-kingdom 20 --max-pages 5
python scripts/import_inaturalist.py --offline --per-kingdom 20
```

A execução offline da importação exige respostas e imagens previamente armazenadas no cache. O parâmetro `--per-kingdom` define a meta total de registros importados por reino, não necessariamente a quantidade a acrescentar.

Para executar a auditoria local de catálogo:

```bash
python scripts/audit_catalog.py --strict
```

A curadoria distingue aprovação de identificação visual, fotografia, etimologia, morfologia e informação educativa. Conteúdos não revisados não devem ser apresentados como evidência conclusiva de identificação. Consulte os relatórios em [`docs/`](docs/) para conhecer cobertura, fontes, licenças, limitações e pendências.

Aprovações antigas sem responsável e evidência foram classificadas como legadas. Novo conteúdo, inclusive assistido por IA, entra em rascunho; alterações exigem nova revisão registrada. O relatório de atualização é offline por padrão (`python scripts/review_updates.py`); `--online` gera propostas para análise humana, sem substituir o conteúdo publicado.

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

- [`PROJECT_CONTEXT.md`](PROJECT_CONTEXT.md) — contexto geral, arquitetura, regras e decisões técnicas.
- [`docs/FICHAS_ESPECIES.md`](docs/FICHAS_ESPECIES.md) — corpus das fichas educativas.
- [`docs/DICAS_E_PONTUACAO.md`](docs/DICAS_E_PONTUACAO.md) — critérios das dicas e sistema de pontos.
- [`docs/COBERTURA_TAXONOMICA.md`](docs/COBERTURA_TAXONOMICA.md) — cobertura e lacunas do catálogo.
- [`docs/PHOTO_CREDITS.md`](docs/PHOTO_CREDITS.md) — atribuição das imagens.
- [`docs/EVOLUCAO_COMPETITIVA.md`](docs/EVOLUCAO_COMPETITIVA.md) — regras da modalidade competitiva.

## Participação e contato

O projeto é atualmente mantido de forma independente. Ideias, correções e relatos de problemas podem ser apresentados pelas [Issues do repositório](https://github.com/kaduolliver/Projeto-Glossata/issues). Contribuições de código ou conteúdo devem ser alinhadas previamente com a manutenção, especialmente quando envolverem taxonomia, fontes e licenciamento de imagens.

## Direitos e licenças

**Gratuidade e ausência de finalidade lucrativa não significam, por si só, licença de código aberto.** Consulte o repositório para verificar se existe uma licença de software expressa antes de reutilizar ou redistribuir o código. Fotografias e dados de terceiros podem possuir licenças e exigências de atribuição próprias; veja os créditos e registros de proveniência.

---

**Projeto Glossata — Explore a diversidade da vida.**
