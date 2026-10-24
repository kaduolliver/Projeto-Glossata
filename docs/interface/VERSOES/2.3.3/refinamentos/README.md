# Refinamentos da PR #38 — evidências e níveis visuais

Continuação da Subetapa 2.3.3 na branch `feat/conquistas-2-3-3`. Aplicativo **2.1.0 ainda não lançado**, catálogo **2.4.0**, SQLite **8**. As [evidências da primeira implementação](../README.md) continuam preservadas. Não há alteração do catálogo, pontuação, identidade local ou critérios/metas das 30 conquistas.

## Correções técnicas

`Achievements::evaluate` passa a validar existência, ID positivo e nome científico não vazio e idêntico ao catálogo antes de contar um organismo distinto. Respostas inválidas ou duplicadas não aumentam a métrica. Metas de diversidade taxonômica exigem também aprovação da taxonomia; identificar o organismo não exige aprovar sua classificação. IDs reutilizados, nomes divergentes ou ausência de dados não são corrigidos por inferência.

`achievementEvidence` reserva o ID da partida somente depois de aceitar respostas detalhadas sequenciais e válidas, cujas regras, contagem, pontos e acertos conferem com o resultado. Caso esses dados sejam insuficientes, busca um checkpoint do mesmo UUID, validado pelos mecanismos existentes de digest/regras/identidade. Escolhe uma fonte por partida, sem somar agregados e respostas nem duplicar eventos. Checkpoints fornecem apenas respostas confirmadas e dicas, inclusive numa pergunta pendente; o fallback não prova conclusão consolidada. Sessões abandonadas, em andamento ou competitivas incompletas não se tornam partidas concluídas.

Um checkpoint finalizado somente complementa um resultado detalhado com a dica pendente/tamanho do ciclo se suas respostas coincidirem exatamente com as do resultado. Reavaliações são determinísticas e idempotentes. O máximo persistido conserva progresso e desbloqueios anteriores; os dados históricos não são reparados, apagados ou transferidos entre identidades.

## Quatro classes da mesma família

| Nível | Material | Elementos distintos | Quantidade |
| --- | --- | --- | --- |
| I · Inicial | Bronze | Moldura simples e um pequeno losango | 8 |
| II · Intermediário | Prata | Contorno interno e losangos laterais | 12 |
| III · Avançado | Ouro | Louros, três marcas e fitas gravadas | 7 |
| IV · Especial | Platina | Moldura facetada, cristais e acabamento luminoso estático | 3 |

[Classificação individual e justificativa das 30 conquistas](../../../../CONQUISTAS.md#níveis-visuais-de-complexidade). `AchievementLevel` pertence à definição C++; `visualLevel`/`visualLevelName` são projetados para QML e eventos. Não há migração para guardar metadados estáticos. Os 30 glifos vetoriais anteriores são mantidos integralmente; o nível não depende do nome, posição ou estado de desbloqueio, não mede perguntas do Quiz e não representa raridade estatística.

Cartões e detalhes exibem o nível textual e o disponibilizam aos recursos de acessibilidade. O bloqueio reduz a opacidade separadamente da moldura e do símbolo, mantendo a geometria da classe legível. O foco, os filtros e o retorno do diálogo seguem os componentes existentes. Nenhuma animação contínua foi introduzida; a resposta à pressão e as transições seguem redução de movimento.

## Reprodução

O build anterior estava em uso na aplicação aberta. A validação usa **`build/achievements-refinements`** e mantém esse processo em execução.

```powershell
$env:PATH='C:\msys64\ucrt64\bin;'+$env:PATH
cmake -S . -B build/achievements-refinements -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/achievements-refinements --parallel 4
ctest --test-dir build/achievements-refinements --output-on-failure -j 4
$env:QT_QPA_PLATFORM='offscreen'
$env:QT_QUICK_BACKEND='software'
$env:QT_QUICK_CONTROLS_STYLE='Basic'
$env:QT_FONT_DPI='96'
$env:QT_SCALE_FACTOR='1.25' # repetir com 1 e 1.5
build/achievements-refinements/tests/glossata_ui_tests.exe achievementsFiltersDetailsKeyboardAndCaptures
```

Todos os bancos e identidades nos testes/capturas são temporários; Discord é desativado. As imagens vêm de `QQuickWindow::grabWindow`, QML e controlador reais. A coleção desbloqueada é produzida por dez respostas corretas numa partida livre e pela conclusão integral competitiva sem dicas, seguidas de uma consulta real. Não se altera o modelo da view para fabricar desbloqueios.

## Capturas reais

O [manifesto de capturas](capturas.json) registra método, fontes, binários, SHA-256, horários reais, dimensões lógicas/físicas e banco temporário. São 23 imagens por escala (69 no total): quinze estados da coleção nas três resoluções e oito detalhes que mostram as quatro classes bloqueadas/desbloqueadas em 1240×820.

| Classe | Bloqueada em 100% | Desbloqueada em 100% | Desbloqueada em 150% |
| --- | --- | --- | --- |
| Inicial / Bronze | [Primeiro voo](capturas/100/level-locked-initial-1240x820.png) | [Primeiro voo](capturas/100/level-unlocked-initial-1240x820.png) | [Primeiro voo](capturas/150/level-unlocked-initial-1240x820.png) |
| Intermediário / Prata | [Conhecedor](capturas/100/level-locked-intermediate-1240x820.png) | [Conhecedor](capturas/100/level-unlocked-intermediate-1240x820.png) | [Conhecedor](capturas/150/level-unlocked-intermediate-1240x820.png) |
| Avançado / Ouro | [Precisão em campo](capturas/100/level-locked-advanced-1240x820.png) | [Precisão em campo](capturas/100/level-unlocked-advanced-1240x820.png) | [Precisão em campo](capturas/150/level-unlocked-advanced-1240x820.png) |
| Especial / Platina | [Desafio perfeito](capturas/100/level-locked-special-1240x820.png) | [Desafio perfeito](capturas/100/level-unlocked-special-1240x820.png) | [Desafio perfeito](capturas/150/level-unlocked-special-1240x820.png) |

| Resolução lógica | 100% | 125% | 150% |
| --- | --- | --- | --- |
| 1000×700 | [Coleção](capturas/100/overview-1000x700.png) | [Coleção](capturas/125/overview-1000x700.png) | [Coleção](capturas/150/overview-1000x700.png) |
| 1240×820 | [Coleção](capturas/100/overview-1240x820.png) | [Coleção](capturas/125/overview-1240x820.png) | [Coleção](capturas/150/overview-1240x820.png) |
| 1920×1080 | [Coleção](capturas/100/overview-1920x1080.png) | [Coleção](capturas/125/overview-1920x1080.png) | [Coleção](capturas/150/overview-1920x1080.png) |

As capturas anteriores não foram substituídas. A tela inicial não sofreu alteração visual neste refinamento; sua captura oficial anterior permanece com as mesmas entradas da Home, validadas por `release.py check`.

## Verificações finais

| Verificação | Resultado |
| --- | --- |
| CTest completo, após os ajustes finais | **26/26 suítes**, zero falhas, 428,31 s |
| Regras/controlador | **86 casos** aprovados, incluindo 20 novos cenários |
| Interface completa | **31 casos** aprovados |
| Identidade/migrações | **38 casos** aprovados |
| Recuperação | **21 casos** aprovados |
| Interface específica | Aprovada em 100%, 125% e 150%; 69 imagens |
| qmllint, smoke, auditoria do repositório/código | Aprovados |
| Status gerado, metadados/histórico de versão | Aprovados |
| Catálogo estrito e integridade histórica | Aprovados; 122 organismos e 1.053 eventos preservados |
| Integridade SQLite | Backup verifica `integrity_check`/`foreign_key_check`; testes de identidade/migração verificam vínculos |
| Gate de distribuição | Bloqueado por pendências de permissões/revisões; sem novos erros técnicos/dependências |

[Resultados estruturados e comandos](verificacoes/resultados.json), [CTest](verificacoes/ctest-final.txt), [regras](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt), [identidade](verificacoes/identity-results.txt), [recuperação](verificacoes/recovery-results.txt).

Os testes de regressão cobrem organismo válido/ausente, ID reutilizado, nomes divergentes/ausentes, duplicação e taxonomia sem aprovação; resultado detalhado, ausente, inválido ou com total divergente; checkpoint inválido/abandonado/ativo, recuperação, competitivo incompleto e outro proprietário. Reavaliam duas vezes e reabrem o banco, verificando igualdade da projeção e preservação integral das linhas históricas. Os metadados visuais são iguais antes/depois do desbloqueio e nas reaberturas.

Os relatórios iniciais foram mantidos. O fixture de histórico inválido tentou primeiro um valor impedido pelo CHECK SQLite; passou a representar um snapshot incompleto, sem contornar a restrição. O primeiro teste visual manteve uma referência ao diálogo após recriar a página; foi corrigido para buscar o novo objeto. A execução completa acima ocorreu **após** essas correções dos testes.

## Inventário sem exclusões

Após terminar builds/testes, executados o [dry-run padrão](verificacoes/cleanup-dry-run.json), o [dry-run de builds antigos](verificacoes/cleanup-old-builds-dry-run.json) e a [varredura ampliada](verificacoes/inventario-local.json). A PR #38 permaneceu aberta: somente inventário, sem limpeza ou `--apply`.

| Caminho | Tamanho na varredura | Motivo e risco |
| --- | --- | --- |
| `build/achievements-refinements` | 1.908.758.204 bytes | Build validado e relatórios recentes; risco alto, preservar |
| `build/achievements-2-3-3` | 1.910.145.166 bytes | Executável anterior em uso e artefatos de compilação; risco alto, preservar |
| `previews/achievements-refinements` | 9.617.985 bytes | Originais das 69 capturas; risco médio para rastreabilidade |
| `build/achievements-refinements/tests/previews/achievements-refinements` | 2.242.517 bytes | Capturas adicionais da suíte completa; risco médio, preservar |
| `scripts/__pycache__` | 1.363.308 bytes | Cache recompilável; risco baixo, candidato apenas a futura limpeza autorizada |
| `tests/__pycache__` | 585.884 bytes | Cache recompilável; risco baixo, candidato apenas a futura limpeza autorizada |

Nenhum arquivo foi excluído ou movido neste refinamento. Bancos, recursos, pacote, capturas e histórico foram preservados. Linhas aninhadas do inventário se sobrepõem; seus tamanhos não devem ser somados.

## Arquivos alterados

Os arquivos de implementação alterados são `src/application/Achievements.cpp/.h`, `src/infrastructure/persistence/AchievementStorage.cpp`, `resources/qml/components/AchievementBadge.qml`, `AchievementBadgePaths.js` e `resources/qml/views/AchievementsView.qml`. Regressões ficam em `tests/AchievementCases.h`, `GameServiceTests.cpp` e `QmlInterfaceTests.cpp`; documentação, changelog, status derivado e `.gitattributes` completam a rastreabilidade.

## Limitações

Não se revogam desbloqueios anteriores, inclusive se a identidade de um registro não puder ser confirmada na avaliação atual. Um fallback não reconstrói a conclusão de um resultado insuficiente. O avaliador continua relendo o histórico, com o mesmo limite de desempenho já documentado. Evidências gráficas offscreen não certificam leitores de tela ou troca física de monitores. Metais e níveis são escolhas de apresentação, sem calibração científica/estatística da dificuldade.

O gate de distribuição conserva as pendências existentes. Desenhos/módulos mantêm seu escopo de licenciamento pendente; testes não concedem aprovações científicas ou jurídicas. Não houve limpeza destrutiva, merge, release, tag ou início da Subetapa 2.3.4.
