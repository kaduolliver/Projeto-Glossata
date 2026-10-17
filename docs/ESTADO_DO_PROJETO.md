# Estado atual verificável — INV-10

Revisão local em **2026-10-17**, partindo de `801e85d`. A correção modifica documentação, verificações e CI; não altera versões para ajustar o texto, nem muda comportamento C++/QML, pontuação, catálogo, licenças ou bancos pessoais. Nenhuma consulta remota, push, tag ou release foi realizada.

## Fontes de verdade e projeções

| Informação | Autoridade | Verificação |
| --- | --- | --- |
| Versão do aplicativo | `VERSION.txt` | CMake lê a versão; `main.cpp` utiliza o macro da versão do projeto. A validação confere esse vínculo e a versão do CMake configurado. |
| Estágio do aplicativo | Entrada correspondente no início de `CHANGELOG.md` | “Não lançado” significa desenvolvimento. Uma entrada datada significa preparação; não comprova publicação. |
| Versão, estado declarado e identidade do catálogo | `resources/data/catalog_manifest.json` | SemVer, estado e SHA-256 do conjunto de entradas; compatibilidade com a entrada atual de `docs/CATALOGO_CHANGELOG.md`. O auditor/release check existente verifica também os arquivos do catálogo. |
| Regras atuais e leituras anteriores | Constantes em `src/domain/Hint.h` | Extração das declarações e conferência de que `GameConfiguration` usa `CurrentScoringRules` como padrão. |
| Quantidade de questões competitivas | `GameConfiguration::CompetitiveQuestionCount` | Extração do valor, sem criar outro número de configuração. |
| Esquemas SQLite | Escritas de `PRAGMA user_version` e guardas de suporte nos repositórios | Ambos precisam concordar. O esquema do snapshot de checkpoint é independente e vem de `SessionCheckpoint.h`. |
| Suítes registradas | Declarações CMake, conferidas contra `ctest --show-only=json-v1` | Lista nominal completa; divergência de quantidade ou nomes é erro, inclusive em build desatualizado. |
| Organismos, arquivos de imagem e domínios | `species.json` e `taxonomy.json` | Contagens derivadas dos registros e referências, sem inferir revisões científicas. |

`docs/PROJECT_STATUS.json` é **gerado**, contendo valores e hashes das entradas. Os blocos delimitados por `glossata:status:start/end` em README e PROJECT_CONTEXT são projeções desse mesmo processo. Não são autoridades independentes nem precisam receber edição manual de versões/contagens. O estado corrente deve ser consultado nesses blocos, sem repetir os números neste documento.

`build/build-manifest.json` e o manifesto do pacote descrevem uma compilação específica; capturas e relatórios de teste também pertencem às respectivas execuções. Não são substitutos dos arquivos-fonte atuais. Contar suítes registradas não prova que foram executadas, passaram ou cobrem todas as funcionalidades. Não se infere publicação remota a partir de um número, changelog, tag local ou snapshot gerado.

## Diagnóstico e correções

| Divergência encontrada | Tratamento |
| --- | --- |
| README indicava aplicativo 1.5.0 e catálogo 2.3.0, enquanto fontes declaravam 1.7.0/2.4.0. | Substituição do estado escrito manualmente pelo bloco gerado, sem mudar os arquivos de versão. |
| PROJECT_CONTEXT ainda dizia “nove suítes”; o build anterior tinha 22. | Inventário derivado e conferido nominalmente com CTest; a própria suíte de metadados acrescenta um registro. Isso não é uma correção artificial da contagem. |
| Regras, esquemas e versões repetidos como fatos atuais em diferentes trechos. | Referências ao bloco gerado e aos arquivos responsáveis, preservando a leitura histórica das regras e migrações. A regra atual já era v4 no código e não foi alterada. |
| Relatórios e contexto misturavam etapas anteriores com estado corrente. | Notas explícitas de registro histórico, marcadores no contexto e links para o estado atual. Versões, resultados e evidências originais permanecem. |
| Changelog dizia genericamente que não havia mudanças em QML/bancos na entrega, após correções que os alteraram. | A afirmação foi limitada à etapa inicial de curadoria, à qual pertencia; o cabeçalho agrega o escopo final em desenvolvimento. |
| README sugeria expansão pelo importador legado; contexto incluía geração de conteúdo na compilação normal. | Exemplos de inspeção com provedores/cobertura, distinção entre candidatos e catálogo, bloqueio da expansão direta e build do clone sem regenerar conteúdo automaticamente. |
| Caminho local do mantenedor parecia obrigatório para colaboradores. | Instrução para usar a raiz do próprio clone, com exemplo explícito de PATH UCRT64 e execução do binário. |
| Registro de ativação semanal podia ser interpretado como configuração remota atual. | Mantida a evidência datada e removida a presunção de consulta em tempo real. Nenhuma configuração remota foi alterada ou revalidada nesta tarefa. |

Os relatórios investigativo, de evolução e de proteção mantêm suas datas/contagens originais. Os resultados datados dos documentos de identidade, Unicode e recuperação continuam descrevendo aquelas intervenções. Não atualizar retroativamente “22/22” para o inventário de hoje nem transformar versões em preparação em lançamentos publicados.

## Uso e manutenção

Após configurar o clone, com CMake/CTest/Python no PATH:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
python scripts/project_status.py check --build build
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Se uma alteração legítima modificar uma fonte de versão, regra, esquema, dados ou testes, revise primeiro a fonte e seu histórico; reconfigure CMake e regenere somente as projeções:

```powershell
python scripts/project_status.py write --build build
python scripts/project_status.py check --build build --report build/project-status-check.json
```

Revise o diff de `docs/PROJECT_STATUS.json`, README e PROJECT_CONTEXT e inclua-os no mesmo commit da fonte alterada. Não altere o software para fazer coincidir uma versão antiga escrita na documentação. Não execute `release.py catalog` sem mudança de catálogo somente para corrigir texto. `write` não modifica fontes, versões, dados científicos ou datas de revisão.

`check` sem `--build` valida apenas as fontes e projeções do checkout, útil antes da configuração e nos testes isolados. Com `--build`, verifica que o diretório configurado pertence ao clone, a versão do projeto coincide e a lista real de testes corresponde ao inventário. Não executa os testes. Metadados faltantes, marcadores duplicados, formato não suportado ou fonte ambígua produzem erro; não são preenchidos com valores presumidos.

O leitor CMake aceita os registros nominais estáticos usados pelo projeto. Novos registros dinâmicos, alterações na sintaxe das fontes ou migrações em outro formato exigem adaptar o leitor e seus testes. A conferência com o CTest evita aceitar silenciosamente uma aproximação do inventário. O verificador também recusa algumas contradições explícitas de versões/regras e contagens manuais nos dois documentos atuais; **não interpreta toda a prosa nem comprova implementação de funcionalidades**. Conteúdo científico e funcionalidades planejadas continuam exigindo revisão apropriada.

## CI, lançamento e limites

O job técnico `offline-validation` executa `project_status.py check --build build` após configurar o CMake e antes de compilar/testar. O relatório local fica em `build/project-status-check.json` e entra nos artefatos técnicos. CTest inclui `GlossataProjectStatusTests`, com mutações exclusivamente em cópias temporárias. `release.py tag` também exige a validação configurada antes dos demais checks de build; isso acrescenta verificação, sem reduzir qualquer gate anterior.

`distribution-permissions` permanece separado e obrigatório para distribuição. Este verificador retorna explicitamente `tests_executed_by_this_command=false`, `publication_verified=false` e `distribution_gate_evaluated=false`; consistência de texto não concede direitos, aprovação científica ou liberação do pacote. Executar somente `check`/`write` não publica nada. Somente o fluxo de lançamento existente, com autorização e verificações reais, pode preparar tag; push e release continuam sendo ações separadas.

Os testes cobrem divergências de versões/manifestos/logs, regras padrão, vínculo da versão do executável, esquemas SQLite, contagens/listas de testes, build de outro clone, registros dinâmicos/duplicados, blocos adulterados, histórico preservado, idempotência e diferença entre entrada datada e publicação. Os resultados efetivamente executados nesta intervenção são registrados abaixo, separados dos metadados derivados.

## Validação desta intervenção

Verificações locais em **2026-10-17**, nesta intervenção:

| Verificação executada | Resultado |
| --- | --- |
| Configuração CMake/Ninja em Release e `cmake --build build --parallel 4` | Concluídas sem erro. |
| `project_status.py check --build build --report build/project-status-check.json` | Consistente, incluindo versão configurada e lista nominal do CTest. |
| `python tests/test_project_status.py` | 10 testes aprovados; mutações em cópias temporárias. |
| `python tests/test_release.py` | 13 testes aprovados, incluindo o novo requisito na preparação de tag. Nenhuma tag foi criada. |
| `ctest --test-dir build --output-on-failure` | **23/23 suítes aprovadas**, em 119,31 s; resultado desta execução local, sem afirmar execução remota da CI. |
| `qmllint` em todos os QML, com `--unqualified disable` | Sem diagnósticos ou erro. |
| `python scripts/audit_catalog.py --strict` | 122 registros válidos segundo a auditoria técnica. |
| `python scripts/repository_protection.py check` e `python scripts/release.py check` | Aprovados localmente; proposta de proteção e integridade de metadados/captura, sem alterar configuração remota. |
| `python scripts/release.py check-history --base 801e85d` | Sem violação da regra de captura no commit visual. |
| Diff contra `801e85d` em `VERSION.txt`, `src/`, `resources/data/`, `resources/compliance/` e `docs/PHOTO_CREDITS.md` | Sem alterações. Versões, regras, catálogo, evidências e runtime preservados. |

Não houve mudança visual ou nova captura nesta tarefa. Os testes não utilizaram bancos pessoais. Não foi executado o gate de distribuição para declarar sua aprovação; as pendências documentadas de direitos e curadoria permanecem. A correção não publicou pacote, tag, release ou commits remotos, e não revalidou configurações do GitHub.
