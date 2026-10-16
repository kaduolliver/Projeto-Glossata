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

- **Etapa 1 concluída localmente, pronta para commit independente.** Logs remotos confirmaram `git: command not found` (job 116714052713 do run 38820019422). Git adicionado às dependências MSYS2; preflight explícito. Auditor de blobs do índice, testes com repositórios temporários e hook local preparados. Nenhum artefato indevido ou padrão conhecido de segredo encontrado no índice inicial; nenhuma exclusão realizada.
- Etapas 2 e 3 **não iniciadas**. Validação da etapa 1: 10/10 suítes CTest (51,76 s), quatro testes adversariais do índice, checks de versão e distribuição (bloqueio legítimo preservado). Três hashes de inventário corrigidos com histórico, sem conceder permissões; catálogo 2.2.1.

## Retomada

Conferir `git status`, este registro e `docs/AUDITORIA_REPOSITORIO.md`. Após registrar o commit da etapa 1, iniciar a etapa 2 por docs/AUDITORIA_FOTOGRAFIAS.json e as 119 pendências; investigar representantes alternativos nos grupos superiores e registrar o misto explicitamente. Preservar o banco dos jogadores; testes e capturas usam bancos temporários. Registrar comandos, resultados e impedimentos reais, sem declarar GitHub Actions aprovado antes de uma execução remota autorizada.
