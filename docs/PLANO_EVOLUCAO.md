# Plano de evolução e auditoria integrada

Instrução operacional recebida em 16/10/2026. Execução sequencial: concluir, validar e registrar em commit cada etapa antes da seguinte. O plano não substitui o README. Não há autorização para push, release ou reescrita do histórico.

## Linha de base

- Branch `feat/classificacao-filogenetica`, commit `3bcbd4f`; árvore limpa antes da implementação.
- Aplicativo 1.5.0, catálogo 2.2.0. 122 organismos, 120 fotos de quiz; apresentação com 98 fotos, 503 vínculos de grupos, misto adicional e 119 pendências.
- `cmake --build build --parallel 4`: sem alterações necessárias. `ctest --test-dir build --output-on-failure`: **9/9**, 62,41 s, bancos temporários.
- Gate de distribuição: saída 2, zero erros técnicos, 120 fotos com documentação de permissão incompleta, sete créditos divergentes e nove achados de inventário/permissão de outros materiais. Isso não demonstra incompatibilidade das 120 licenças.

## Dependências e ordem

1. Auditoria do Git/CI e proteção do índice. As próximas etapas dependem de testes confiáveis e preservação de evidências.
2. Investigar todas as pendências fotográficas, ampliar somente onde houver fonte/representatividade/permissão demonstráveis e registrar cobertura de cartões especiais.
3. Integrar avaliação pedagógica automatizada explicitamente identificada; conteúdo, progressão, pontuação, persistência e relatos. Não converter isso em revisão científica humana ou permissão de imagens.
4. Validação integrada e pacote local, com versões independentes, sem publicação.

## Progresso

- **Etapa 1 concluída em 6cf7dc6.** Logs remotos confirmaram Git ausente; correção MSYS2, auditor do índice/hook e quatro testes adversariais. 10/10 suítes (51,76 s); catálogo 2.2.1 corrige três hashes de inventário sem conceder permissões.
- **Etapa 2 concluída em b3e99e4.** Investigação das 119 pendências, 11 novas fotos e contabilização explícita do misto. 10/10 suítes (64,43 s); catálogo 2.3.0.
- **Etapa 3 concluída localmente e validada, pronta para commit.** 11/11 suítes (58,90 s); detalhes e pendências abaixo. Validação integrada será executada depois desse commit.

## Retomada

Conferir git status e os commits antes de continuar; não repetir etapas concluídas. O próximo passo é o commit da etapa 3, seguido da validação integrada e do pacote canônico local de desenvolvimento. Testes/capturas usam bancos temporários. Nenhuma ação remota, tag ou release está autorizada. Não declarar GitHub Actions aprovado sem execução remota autorizada.

## Etapa 2 — implementação e pesquisa

Base: etapa 1 em `6cf7dc6`. Adicionadas 11 fotos; cobertura 98 → 109 arquivos, 504 → 534 cartões (80,90% → 85,71%), 119 → 89 pendências, +2.461.743 bytes. O cartão misto agora ocupa uma linha explícita `special:mixed`, totalizando 623 registros de cobertura. Investigadas as 29 espécies inicialmente pendentes e 20 representantes alternativos; resultados/recusas preservados na auditoria. Grupos superiores recebem alternativas vinculadas ao backbone GBIF por evidência arquivada e hash; espécies recusam identidade diferente. Nenhuma alteração de QML, filtros, pontuação, dados dos 122 organismos ou 120 fotos do quiz. Catálogo 2.3.0. Validação concluída: compilação, auditoria estrita, check de versões, qmllint e 10/10 suítes CTest (64,43 s). Gate de distribuição mantém bloqueios legítimos; nenhum dado de jogador foi usado.

Etapa 2 registrada em **b3e99e4**, após 10/10 suítes.

## Etapa 3 — concluída e validada, pronta para commit

Aplicativo 1.6.0, catálogo 2.4.0 (não lançados). 855 receitas para os 122 organismos, avaliação pedagógica approved_automated com prazo configurável, identidade/foto/fontes/política por hash; 290 propostas históricas e revisões científicas preservadas. Três slots independentes da categoria, seleção direta, contexto científico e progressão; exceções e lacunas explícitas. Notas de organismo em níveis superiores recebem escopo individual claro. Quatro conteúdos inadequados não foram relabelados como morfologia. Snapshots v4 preservam texto, categorias, tetos, opções e avaliação; v3/v2 permanecem congelados e não há ALTER TABLE ou regravação de pontos em esquema 3.

Cobertura de referência (seed 42): 64/80 organismos competitivos completos; gênero 80/100 com três dicas; espécie 29/40; família 39/100; ordem 23/100. Classe tem 95/120 com duas e 25 com uma; filo tem 100/121 com uma e 21 lacunas completas, por falta de conteúdo científico utilizável/escopo adequado. Todas as lacunas estão em AUDITORIA_PROGRESSAO_DICAS.json. Isso não promete três dicas em qualquer combinação de opções. As partidas mantêm 20 questões e diversidade por grupo; o Livre não exclui organismos por falta de dicas.

Compilação e **11/11 suítes CTest (58,90 s)** aprovadas, inclusive UI real em software, pontuação/ordens, persistência/reabertura/migrações, modo competitivo e relatos. Auditoria estrita, qmllint, licenças/fotos e metadados aprovados. Captura real da tela inicial renovada (1240×820, banco temporário). Comparação byte a byte confirmou organismos, taxonomia, revisões científicas/baseline, fontes educativas e 120 fotos do quiz preservados. Gate de distribuição: zero erros técnicos, 120 permissões fotográficas incompletas, sete questões de direitos de materiais próprios/legados (incluindo o novo corpus), mídias históricas fora do bundle e bloqueios científicos documentados; nada foi liberado artificialmente.

Próximo passo: registrar commit independente da etapa 3; executar validação integrada e atualizar somente o pacote canônico local de desenvolvimento, usando o script existente e bancos temporários. Sem push, tag ou release.
