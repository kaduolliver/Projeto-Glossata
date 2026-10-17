## Mudança

Descreva o problema e o resultado esperado.

Informe a origem/autoria do código e os termos de trechos externos. Submissões de código para incorporação devem ser compatíveis com Apache-2.0 conforme CONTRIBUTING.md; não há cessão/CLA. Dados, textos e mídias mantêm condições separadas. Atualize o escopo enumerado quando incluir arquivos de código.

## Verificações

- Registre os comandos executados e seus resultados.
- Confira o check técnico `offline-validation` no commit atual do PR.
- Pendências de `distribution-permissions` devem permanecer explícitas; sua falha não autoriza lançar um pacote.

## Governança e histórico

Se houver mudanças em workflows, políticas, revisões, permissões, catálogo ou versões:

- Identifique os arquivos e a justificativa; confira o diff completo, incluindo exclusões.
- Preserve evidências, hashes anteriores e responsáveis reais; use o fluxo de governança para decisões de conteúdo.
- Compare o histórico com o SHA completo da base: `python scripts/content_governance.py audit-history --base SHA_COMPLETO`. Confira o recibo, sem tratar hashes, campos de autor ou CI como autenticação do revisor.
- Diferencie alteração administrativa de revisão científica e autorização de direitos.
- Registre limitações, decisões pendentes e recuperação quando pertinente.

O projeto possui um mantenedor. A conferência do próprio trabalho é autoverificação, sem aprovação independente presumida. Consulte [o procedimento de proteção](https://github.com/kaduolliver/Projeto-Glossata/blob/main/docs/PROTECAO_REPOSITORIO.md), após sua integração.
