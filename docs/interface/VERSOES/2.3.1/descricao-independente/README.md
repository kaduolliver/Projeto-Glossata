# PR #35 — Morfologia e Descrição educativa independentes

Correção pontual da [PR #35](https://github.com/kaduolliver/Projeto-Glossata/pull/35), sobre `d17c68f443e7908fec278f873c1c042368f71415`. A associação anterior da descrição com Morfologia foi removida; os registros anteriores da ficha em `restauracao-visual/` permanecem históricos. Quiz, dicas, contadores e estabilidade do retrato não foram modificados.

Implementação e teste no commit [`b076cfa`](https://github.com/kaduolliver/Projeto-Glossata/commit/b076cfa). A correção é de apresentação e não implica aprovação científica ou de distribuição.

## Implementação

`InfoView.sections` recupera `profile.morphology` entre Nutrição e Conservação, com o título “Morfologia · aspectos físicos”, ícone `shape`, cor, texto, fontes, evidências e revisão originais. O tópico usa o mesmo delegate dos demais conteúdos científicos da coluna direita. O agregador de referências volta a identificar essa fonte como Morfologia, sem associá-la à descrição inexistente.

`speciesDescriptionRegion` mantém a estrutura visual abaixo da fotografia e dos créditos. Contém somente o título “Descrição educativa” e a mensagem **“Descrição educativa detalhada ainda não disponível.”** Não lê Morfologia, Nutrição, Distribuição, Conservação, outro tópico ou o resumo. Não possui fonte ou revisão de um texto que ainda não existe. As propriedades que faziam a associação foram removidas.

O resumo e os demais tópicos, fontes, avisos científicos e ações permanecem em seus contextos anteriores. Nenhum campo persistente, esquema, conteúdo científico, fotografia, licença, autoria ou registro de curadoria foi introduzido ou modificado. Aplicativo 1.10.0 ainda não lançado; catálogo 2.4.0 preservado. O desenvolvimento de uma descrição independente fica para a **Etapa 3 — Enciclopédia Científica Interativa**, sem antecipar sua implementação nesta PR.

## Validação

`speciesDescriptionStaysIndependentOfMorphology` usa o registro real de *Panthera leo*, QML/controlador de produção, SQLite temporário e Discord desativado. Confere a posição do tópico na coluna direita e entre Nutrição/Conservação, texto e fonte originais, acionamento do link por teclado, revisão, resumo preservado e ausência de texto/fonte científica na região preparada. Também verifica texto morfológico longo e acesso à ação de continuação pela rolagem.

O caso sem Morfologia usa uma fixture somente na entrada local da view: as 122 notas atuais do catálogo têm conteúdo morfológico. O teste remove essa nota da cópia de apresentação, verifica que os demais tópicos continuam visíveis e que a mensagem abaixo da fotografia permanece idêntica; o controlador e o catálogo não são modificados. Essa fixture não é publicada como captura de um registro científico.

**26/26 suítes CTest aprovadas**, em **390,50 s** de tempo real com `-j3`: 62 casos QtTest de domínio, 24 de interface, 32 de identidade e 21 de recuperação, incluindo preparação/limpeza. [Log completo](verificacoes/ctest-results.txt), [interface](verificacoes/ui-test-results.txt) e [resultados rastreáveis](verificacoes/resultados.json). As execuções direcionadas em [100%](verificacoes/description-targeted.txt), [125%](verificacoes/description-dpi-125.txt) e [150%](verificacoes/description-dpi-150.txt) passaram com cinco casos cada, contando preparação/limpeza, incluindo os dois testes existentes de dicas, contadores, recuperação e estabilidade fotográfica.

Build Release, [lint de InfoView](verificacoes/description-qmllint.txt), [smoke do executável](verificacoes/description-smoke.txt), metadados/histórico, status do projeto, licenciamento do código, whitespace e hashes passaram. A prova de escopo nos resultados confirma que somente `InfoView.qml` mudou no código da aplicação: backend, demais telas, componentes de fotografia/dicas, catálogo, dados científicos, licenças, governança e capturas anteriores permanecem idênticos a `d17c68f`.

O [manifesto](capturas.json) registra **18 capturas reais**, com versões, fonte, crédito/proveniência fotográfica, horários, dimensões lógicas/físicas e hashes de arquivos/binários. São duas posições da ficha em 1000×700, 1240×820 e 1920×1080, nas três escalas Qt. Somente o registro real de *Panthera leo* é publicado; as fixtures de texto longo ou ausência de nota não geram evidências científicas artificiais.

| Resolução lógica | Morfologia e estrutura independente | Qt 125% | Qt 150% |
| --- | --- | --- | --- |
| 1000×700 | [Abrir](capturas/morphology-and-description-1000x700.png) | [Abrir](capturas/morphology-and-description-1000x700-125.png) | [Abrir](capturas/morphology-and-description-1000x700-150.png) |
| 1240×820 | [Abrir](capturas/morphology-and-description-1240x820.png) | [Abrir](capturas/morphology-and-description-1240x820-125.png) | [Abrir](capturas/morphology-and-description-1240x820-150.png) |
| 1920×1080 | [Abrir](capturas/morphology-and-description-1920x1080.png) | [Abrir](capturas/morphology-and-description-1920x1080-125.png) | [Abrir](capturas/morphology-and-description-1920x1080-150.png) |

Para reproduzir, compile com `cmake --build build -j3` e execute `ctest --test-dir build --output-on-failure -j3`. Nesta revisão, os binários estão em `build/2.3.1-descricao/`. A partir de `build/tests`, execute `../2.3.1-descricao/glossata_ui_tests.exe speciesDescriptionStaysIndependentOfMorphology restoredQuizHintsCountersAndStablePortrait restoredQuizRecoveryAndUnavailableStates`, com `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, `QT_QUICK_CONTROLS_STYLE=Basic` e `QT_SCALE_FACTOR=1`, `1.25` ou `1.5`. As capturas da ficha ficam em `build/tests/previews/species-description-independent-2.3.1/<escala>/`.

A [auditoria de arquivos](verificacoes/auditoria-arquivos.json) executou o auditor já integrado à `main` em `25a882b`, pela cópia local idêntica, com raiz explícita e `--include-old-builds`, em modo **dry-run**. Nenhum arquivo foi excluído ou movido; as proteções de idade e retenção foram mantidas. A auditoria não altera a PR nem importa mudanças de limpeza.

## Limitações

Capturas Windows 11/Qt 6.11.2 software/offscreen, com fatores Qt; a escala física do Windows não foi alterada. Mouse/teclado são exercitados pelo QtTest, sem ensaio físico de GPU ou leitor de tela. Esta correção não cria descrições nem aprovações; as pendências de distribuição existentes permanecem.
