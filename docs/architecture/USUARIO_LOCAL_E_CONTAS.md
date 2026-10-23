# Usuário local e contas futuras

Decisão do mantenedor na revisão da PR #37: uma identidade local por instalação. O vínculo SQLite persistente seleciona um UUID existente ou cria um UUID novo somente quando não há perfil ativo. Não é cadastro, autenticação ou prova de propriedade remota. A [transição e seus limites](../interface/VERSOES/usuario-local-unico/README.md) fazem parte da implementação; os requisitos abaixo são planejamento, sem autorização de implementação nesta PR.

## Requisitos antes de implementar contas

| Área | Requisito futuro |
| --- | --- |
| Cadastro | Definir dados mínimos, confirmação de acesso, recuperação da conta e tratamento de duplicidades. Não presumir que nome local seja único globalmente ou comprove identidade. |
| Autenticação | Selecionar um provedor/fluxo adequado ao aplicativo nativo, validar sessões e autorização de cada operação; definir expiração, revogação e armazenamento seguro dos tokens. Não embutir segredos de servidor no cliente. |
| Identificador remoto | Criar identificador remoto estável e distinto do UUID local; manter vínculos explícitos por instalação. Cópia de um SQLite ou coincidência de nomes não autentica ninguém. |
| Estatísticas por conta | Definir quais partidas pertencem à conta, sua proveniência e política de comparação. Manter configuração, regras, conteúdo, unidade de pontos e critérios de desempate versionados. |
| Sincronização | Exigir adesão explícita, escolher quais dados/identidades vincular e verificar propriedade. Prever idempotência, conflitos, reenvio, operação offline, desvinculação e dispositivos compartilhados. Não enviar automaticamente bancos inteiros ou todos os perfis antigos. |
| Rankings globais | Definir contextos comparáveis, elegibilidade, validação e moderação no serviço. Resultados locais/legados não se tornam competitivos globais automaticamente. Manter a origem visível e separada da validação online. |
| Privacidade e consentimento | Explicar dados enviados, finalidade, visibilidade e retenção; oferecer controles de compartilhamento, exportação e desvinculação. Uma escolha de usuário local não autoriza publicação ou associação a uma conta. As políticas concretas e suas avaliações humanas continuam pendentes. |

## Vinculação de dados locais

O usuário deverá ver quais UUIDs, partidas e resultados serão associados, com suas limitações e origem, e confirmar explicitamente. A autenticação da conta remota deve preceder a verificação de autorização para cada vínculo; conhecer um UUID/nome ou ter acesso a um dispositivo compartilhado não prova a propriedade de todos os históricos nele existentes. As regras para recuperação e resolução de disputas precisam de decisão humana antes de implementação.

Não fundir identidades por nome. Preservar a trilha de proveniência e os proprietários originais; um vínculo remoto futuro não deve reescrever partidas antigas para fabricar propriedade. Projetar a reversibilidade do compartilhamento sem destruir os originais locais. O usuário único atual não implica uma única conta remota para sempre, nem cadastro automático.

## Limites desta PR

Não há conta autenticada, servidor, API, PostgreSQL, sincronização ou ranking global. SQLite continua local; as proteções de integridade não são prova de autoria autenticada, inviolabilidade do dispositivo ou certificação competitiva. O [planejamento histórico da PR #36](ESTATISTICAS_MULTIPLAYER.md) permanece como referência anterior; a decisão atual substitui a hipótese de gerenciamento permanente de vários perfis locais.
