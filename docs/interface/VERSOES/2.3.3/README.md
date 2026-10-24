# Subetapa 2.3.3 — Conquistas

Os [refinamentos posteriores na mesma PR #38](refinamentos/README.md) corrigem a identidade de organismos/seleção de evidências e acrescentam quatro classes visuais. Os relatórios e capturas abaixo registram a primeira implementação e permanecem preservados.

Aplicativo **2.1.0**, ainda não lançado; catálogo **2.4.0** preservado. Base: `0e007ace1cc64c67d72441ac833c3af979b6a4fd`, integração da PR #37. Branch `feat/conquistas-2-3-3`.

O sistema anterior tinha seis indicadores efêmeros da partida. A implementação oferece **30 conquistas persistentes em seis categorias**, com badges geométricos próprios, metas transparentes, filtros combinados, progresso e detalhes em diálogo Qt Quick. A lista completa, a migração SQLite **7 → 8**, os critérios de reconhecimento retroativo e o procedimento de extensão estão em [CONQUISTAS.md](../../../CONQUISTAS.md).

Os seis desafios anteriores conservam nomes e condições compatíveis. Contagens passam a atravessar partidas; sequência e ciclo continuam com condição por sessão. Dados já consolidados pertencem ao UUID local, independente do nome. Consultas a fichas só contam a partir desta implementação. Eventos inválidos/repetidos não aumentam contadores, e resultados não se duplicam ao recuperar ou finalizar checkpoints. Conclusão competitiva exige todas as perguntas; as metas gerais de perfeição/ausência de dicas exigem dez respostas.

`AchievementBadge` usa `QtQuick.Shapes` e um mapa de 30 glifos distintos. A tentativa inicial com imagens SVG foi substituída porque o ambiente Qt não tinha o plugin leitor de SVG; os protótipos locais foram preservados. Não foi instalada dependência nova. A referência fornecida orientou o acabamento, sem importar imagens/badges de terceiros. Novos módulos/desenhos mantêm pendências explícitas de escopo/licenciamento; não há revisão ou autoria humana presumida.

Filtros de estado e categoria sobrevivem à navegação dentro da execução. A grade usa duas, três ou quatro colunas conforme a largura. Tab/Shift+Tab, Espaço/Enter, Escape, retorno de foco e enquadramento do cartão pelo shell fazem parte da validação. Estados têm rótulos/contagens além de cores. Hover, pressão e progresso respeitam “Transições suaves”; não há animação contínua nem notificação de desbloqueio. O sinal C++ está preparado para a 2.3.4, que não foi iniciada.

## Validação e reprodução

Todos os bancos usados em testes e capturas são temporários. As capturas usam `QQuickWindow::grabWindow`, QML/controlador de produção e eventos de quiz/consulta executados de fato no ambiente isolado; não usam mockups ou um modelo de conquistas fabricado.

```powershell
cmake -S . -B build/achievements-2-3-3 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/achievements-2-3-3 --parallel 4
ctest --test-dir build/achievements-2-3-3 --output-on-failure -j 4
$env:QT_QPA_PLATFORM='offscreen'
$env:QT_QUICK_BACKEND='software'
$env:QT_QUICK_CONTROLS_STYLE='Basic'
$env:QT_SCALE_FACTOR='1.25' # repetir com 1 e 1.5
build/achievements-2-3-3/tests/glossata_ui_tests.exe achievementsFiltersDetailsKeyboardAndCaptures
python scripts/capture_home.py --binary build/achievements-2-3-3/projeto_glossata.exe
```

O teste de interface grava `previews/achievements/<escala>/captures.json` e imagens reais. A execução completa de CTest usa o diretório de build/testes como diretório corrente; os caminhos de previews ficam relativos a ele. Fatores Qt não certificam troca física entre monitores/DPI ou leitores de tela.

### Resultados registrados

| Verificação | Resultado |
| --- | --- |
| CTest completo | **26/26 suítes**, zero falhas |
| Regras/controlador | 66 casos aprovados |
| Interface completa | 31 casos aprovados |
| Identidade/migrações | 38 casos aprovados |
| Recuperação | 21 casos aprovados |
| Conquistas em 100%, 125% e 150% | 3 execuções aprovadas; 45 capturas |
| qmllint, smoke, auditoria do repositório, inventário de código, status e versão | Aprovados |
| Catálogo estrito e integridade histórica | Aprovados; 122 organismos e 1.053 eventos históricos preservados |
| Gate de distribuição | Bloqueado por pendências de permissões/revisões; não houve publicação |

[Relatório estruturado e comandos](verificacoes/resultados.json), [CTest](verificacoes/ctest-final.txt), [regras](verificacoes/core-results.txt), [interface](verificacoes/ui-test-results.txt), [identidade](verificacoes/identity-results.txt) e [recuperação](verificacoes/recovery-results.txt).

Depois da suíte completa, a inspeção visual identificou a data parcialmente cortada no diálogo. A altura passou a considerar cabeçalho, rodapé e paddings reais. Foi acrescentada uma asserção de enquadramento da data; após recompilação, o teste específico e as capturas foram repetidos nas três escalas, assim como lint, smoke e os checks listados. A suíte completa não foi repetida depois desse ajuste restrito ao diálogo; seu relatório é preservado com essa distinção.

Os relatórios iniciais também foram mantidos: expectativa de esquema 7 atualizada para 8, espera pelo carregamento inicial no teste de navegação e tentativa de SVG sem plugin leitor. O inventário de licença exigiu incluir os novos caminhos no índice Git, mantendo-os explicitamente pendentes. Nenhum desses checks constitui aprovação de autoria, licença ou ciência.

### Evidências visuais

Cinco estados por combinação: bloqueadas (`locked`), visão geral (`overview`), desbloqueada com foco (`unlocked-focus`), detalhe (`details`) e progresso parcial (`partial`). Dimensões da tabela são lógicas; os PNGs refletem o fator de escala. O [manifesto](capturas.json) registra SHA-256, dimensões físicas, horário real, binário e entradas de compilação.

| Resolução | 100% | 125% | 150% |
| --- | --- | --- | --- |
| 1000×700 | [Visão geral](capturas/100/overview-1000x700.png) | [Visão geral](capturas/125/overview-1000x700.png) | [Visão geral](capturas/150/overview-1000x700.png) |
| 1240×820 | [Visão geral](capturas/100/overview-1240x820.png) | [Visão geral](capturas/125/overview-1240x820.png) | [Visão geral](capturas/150/overview-1240x820.png) |
| 1920×1080 | [Visão geral](capturas/100/overview-1920x1080.png) | [Visão geral](capturas/125/overview-1920x1080.png) | [Visão geral](capturas/150/overview-1920x1080.png) |

[Detalhe em 1000×700/150%](capturas/150/details-1000x700.png), [progresso parcial](capturas/100/partial-1240x820.png) e [coleção bloqueada](capturas/100/locked-1920x1080.png). A [captura oficial da tela inicial](../../../images/tela-inicial.png) e seu [registro](../../../images/tela-inicial.json) foram renovados pelo script após a compilação final.

### Arquivos e responsabilidades

- `src/application/Achievements.*`: definições, métricas e projeção dos 30 feitos.
- `src/infrastructure/persistence/AchievementStorage.*` e `SqliteGameHistoryRepository.*`: esquema 8, evidências, consultas e transações por UUID.
- `src/presentation/GameController.*`: integração com eventos confirmados e sinal de desbloqueio.
- `resources/qml/views/AchievementsView.qml`, `components/AchievementBadge.qml` e `AchievementBadgePaths.js`: coleção, filtros, cartões, desenhos e detalhes.
- `Main.qml`, `TaxonomyView.qml` e `InfoView.qml`: contexto de navegação, rolagem e registro de consulta real.
- CMake, `resources.qrc`, `scripts/player_backup.py` e testes: compilação, compatibilidade do backup e regressões.
- Versão, changelogs, status gerado, documentação de interface, escopo pendente e capturas: correspondência aplicativo 2.1.0/catálogo 2.4.0 e rastreabilidade.

### Inventário local sem exclusões

Executado após terminar builds/testes e antes de abrir a PR. O [dry-run padrão](verificacoes/cleanup-dry-run.json) e a [varredura de builds antigos](verificacoes/cleanup-old-builds-dry-run.json) respeitam as proteções do script. O [inventário ampliado](verificacoes/inventario-local.json) relaciona caminhos, bytes, motivos e riscos; linhas aninhadas se sobrepõem.

| Caminho | Tamanho na varredura | Motivo e risco |
| --- | --- | --- |
| `build/achievements-2-3-3` | 1.909.719.788 bytes | Build atual e evidências; risco alto, preservar |
| `build/local-ranking-history` | 1.920.037.652 bytes | Build anterior; risco alto, conteúdo não descartável automaticamente |
| `build/single-local-user` | 1.911.598.001 bytes | Build anterior; risco alto, preservação da rastreabilidade |
| `previews/achievements` | 6.299.625 bytes | Originais das 45 capturas versionadas; risco médio |
| `build/achievements-2-3-3/tests/previews/achievements` | 1.436.972 bytes | Capturas anteriores ao ajuste final; risco médio |
| `build/achievements-2-3-3/artefatos-locais` | 1.597.850 bytes | 30 protótipos SVG e quatro relatórios desta tarefa preservados; risco médio para evidências |
| `scripts/__pycache__` | 1.363.308 bytes | Cache recompilável; risco baixo, exige autorização para excluir |
| `tests/__pycache__` | 585.884 bytes | Cache recompilável; risco baixo, exige autorização para excluir |

Os quatro relatórios gerados na raiz e os protótipos SVG não versionados foram movidos para `artefatos-locais`, sem sobrescritas ou exclusões. Nenhum recurso previamente existente/versionado, banco, mídia, pacote ou histórico foi removido. Não foi utilizado `--apply`.

## Limitações deliberadas

- Históricos sem respostas detalhadas não são convertidos em eventos. Livre sem checkpoint finalizado não prova ausência de dica numa pergunta pendente. Datas retroativas indicam reconhecimento.
- Não há metas por dias, conta remota, sincronização, mudança de pontuação, ranking global ou notificações flutuantes.
- A avaliação relê o histórico nas ações relevantes; volumes muito maiores podem exigir otimização futura. Falha de persistência mantém o histórico e impede emitir desbloqueio não confirmado.
- O gate de distribuição continua bloqueado por permissões/revisões existentes e pelo escopo pendente de materiais novos. Testes não concedem aprovação científica/jurídica. Não há release, tag ou merge automático.
