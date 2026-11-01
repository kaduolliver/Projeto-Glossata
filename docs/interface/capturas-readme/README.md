# Correção das capturas do README

Base: `d4224c8`, integração da PR #40. Correção de documentação, captura e validação; aplicativo **2.3.0 ainda não lançado**, catálogo **2.4.0**, sem mudança de versão ou novo lançamento. QML, funcionalidades, fontes C++, SQLite, fotografias, atribuições, licenças e registros de revisão permanecem com os mesmos bytes.

## Causa e solução

`scripts/capture_home.py` aceitava `--section top` e `--section discovery`, mas ambos gravavam em `docs/images/tela-inicial.png` e `tela-inicial.json`. O registro anterior declarava explicitamente a rolagem para a Descoberta, enquanto a legenda do README descrevia o carrossel completo. O check anterior passava porque verificava origem, hashes e fontes, sem confrontar seção/enquadramento com o destino.

O mapa de destinos em `scripts/release.py` passa a ser compartilhado pelo capturador e pela validação. O padrão continua `top`; a opção `discovery` grava exclusivamente `docs/images/descoberta-do-dia.png` e seu JSON. Cada registro declara seção e enquadramento; ambos são conferidos, além de dimensões, hashes, origem e fontes atuais. Imagens idênticas para as duas seções são rejeitadas. A regra de PNG e JSON do topo no mesmo commit visual continua obrigatória.

O README apresenta Início, Explorar, Jogar e Descoberta do Dia nessa ordem. Explorar e Quiz reutilizam capturas históricas reais com seus manifestos originais; a imagem de Explorar aparece uma vez. As atribuições e pendências continuam disponíveis. A validação existente também confere os links locais e a integridade das capturas exibidas, aceitando os formatos dos manifestos históricos sem reescrevê-los. O workflow já executa o release check e a suíte de regressão correspondente.

## Evidências

- [Topo atual](../../images/tela-inicial.png) e [registro](../../images/tela-inicial.json).
- [Descoberta atual](../../images/descoberta-do-dia.png) e [registro](../../images/descoberta-do-dia.json).
- [Execuções reais e comparação dos pares protegidos](capturas-verificadas.json): topo, Descoberta e novo topo, com hashes antes/depois nos dois sentidos.
- [Imagem anterior](anterior-home.png) e [JSON anterior](anterior-home.json): cópias integrais de `docs/images/tela-inicial.*` na base `d4224c8`, mantendo inclusive a data original de captura. São evidências da inconsistência anterior, sem pretensão de recaptura recente.
- [Procedimento de atualização e integridade](../../VERSIONAMENTO.md).

O CMake foi configurado com `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`. A primeira tentativa de captura foi corretamente recusada: o executável padrão `build/projeto_glossata.exe` não correspondia ao manifesto. O timestamp de `src/app/main.cpp` foi atualizado sem alterar seus bytes para forçar recompilação pelo CMake. O alvo de produção desta configuração grava em `build/profile-settings-2-4-1/projeto_glossata.exe`; esse executável e o manifesto regenerado foram usados nas capturas. Não se relaxou o controle de integridade.

As duas imagens são conteúdo integral da janela real por `QQuickWindow::grabWindow`, em **1240×820**, Qt offscreen/software, escala 1 e 96 DPI. O isolamento temporário e a desativação do Discord são os existentes no programa; nenhum banco pessoal é usado. Não houve edição dos pixels, mockup, IA ou ampliação da resolução.

Os testes de regressão utilizam PNGs mínimos e executável substituto somente dentro de diretórios temporários de teste; esses fixtures nunca são publicados como captura do aplicativo. Cobrem as opções da CLI, o padrão, preservação do outro par, falhas/truncamento, binário desatualizado, destinos trocados, hashes, dimensões, registros ausentes/inválidos, links e evidências históricas.

## Verificações e higiene

**21 testes de regressão aprovados; 4/4 suítes CTest relacionadas aprovadas, sem falhas, em 15,56 s.** Incluem release, auditoria do repositório, metadados do projeto e investigação de direitos. [Log CTest](ctest.txt) · [Resultados e inventário](verificacoes.json). Não foi executada a suíte completa nesta correção de documentação/ferramentas.

Release check, links locais do README, project status, escopo de código, auditoria do repositório e `git diff --check` passaram. `docs/PROJECT_STATUS.json` foi regenerado somente para refletir o hash do changelog. Os fontes e dados protegidos foram comparados com a base; as cópias históricas também foram comparadas byte a byte com os blobs originais. A inspeção visual confirmou carrossel e ações no topo e cartão/indicadores na Descoberta.

O gate `python scripts/distribution.py check` retornou **2**, com distribuição bloqueada pelas pendências de direitos/revisão; o relatório não apresentou erros técnicos. Nenhuma permissão ou revisão foi alterada para contornar esse gate.

Inventários padrão e ampliado (`--include-old-builds`), ambos **dry-run**, encontraram **zero candidatos elegíveis** e preservaram 24 e 42 conjuntos, respectivamente. A varredura de previews também identificou 53 grupos com PNGs de bytes iguais; caminhos e tamanhos estão no registro, sem interpretar duplicidade como autorização de exclusão. Exemplos de artefatos preservados:

| Caminho | Bytes | Motivo | Risco de remover |
| --- | ---: | --- | --- |
| `scripts/__pycache__/` | 1.397.853 | Cache Python | Exige regeneração; manter sem autorização |
| `tests/__pycache__/` | 585.884 | Cache de testes | Exige regeneração; manter sem autorização |
| `build/profile-settings-2-4-1/` | 323.918.990 | Build usado nas capturas | Perda de rastreabilidade do executável |
| `build/tests/previews/daily-discovery/` | 5.338.784 | Previews recentes protegidos pelo script | Perda de evidências locais de testes |

Logs locais desta tarefa em `build/capturas-readme-*` também estão inventariados. Nenhum arquivo foi excluído e nenhum `--apply` foi executado. Imagens históricas versionadas, recursos, pacotes e dados de jogadores foram preservados.

## Limitações

Inspeção pelo agente das imagens reais não substitui revisão manual do mantenedor, testes de outras GPUs/fontes ou validação científica/jurídica. Os metadados e testes não concedem direitos sobre as fotografias. Os avisos de pendências continuam no README e nos [créditos](../../PHOTO_CREDITS.md). Não há autorização de distribuição, tag, release ou merge nesta correção.
