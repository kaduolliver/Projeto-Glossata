# Usuário local único — revisão da PR #37

Esta revisão substitui a direção inicial da PR #37 por decisão expressa do mantenedor: uma identidade local por instalação, sem gerenciamento permanente de múltiplos perfis. A [proposta anterior](../ranking-local-historico/README.md), seus commits, capturas e verificações permanecem como histórico da revisão, sem descrever a interface atual.

Aplicativo **2.0.0**, ainda não lançado, com catálogo **2.4.0** inalterado. MAJOR pela retirada incompatível da alternância/gestão de múltiplos perfis. A numeração das subetapas de interface é independente: esta revisão não inicia a Subetapa 2.3.3. Nenhuma conta online, API ou sincronização foi implementada.

## Identidade e transição

O esquema SQLite passa de 6 para **7** apenas para acrescentar `local_user_binding`: uma linha por instalação, com UUID escolhido, momento real da escolha e motivo (`new_installation`, `single_existing` ou `explicit_transition`). A migração é transacional, preserva as tabelas e os gatilhos anteriores e recusa bancos atuais incompletos. Não recalcula nem reatribui resultados, nomes registrados, partidas, respostas ou checkpoints. `PlayerIdentityMigration.cpp` e sua política conservadora de legados não mudam.

Na primeira abertura sem perfis ativos, cria uma identidade nova; legados e perfis excluídos não lhe transferem resultados. Com um perfil ativo, conserva seu UUID e seus dados. Com vários ativos, não escolhe por nome, ordem ou pontuação: bloqueia novas partidas e oferece um fluxo pontual, sem pré-seleção, mostrando nome e UUID para distinguir homônimos. A escolha exige a ação “Definir usuário local” e é persistida antes de habilitar o uso normal. Os demais perfis e seus registros continuam no banco.

Gatilhos adicionais impedem atualizar/excluir/substituir o vínculo, escolher uma identidade inelegível ou desativar a identidade vinculada. Duas instâncias não podem selecionar UUIDs diferentes silenciosamente: a escolha usa `BEGIN IMMEDIATE` e a segunda observa o vínculo vencedor. A aplicação não reabre o seletor após a transição, mesmo que existam outros perfis antigos. Banco incompatível ou identidade indisponível produz erro explícito e bloqueia o jogo, sem recriar UUIDs por aproximação.

Renomear altera somente o nome de exibição conforme a política de nomes já existente. O UUID e os nomes capturados pelas partidas continuam intactos. Conflitos com nomes antigos ainda ativos continuam sujeitos às proteções anteriores; não se apagam/desativam perfis para contornar o conflito.

## Sessões, métricas e recordes

O controlador inicia partidas apenas com a identidade vinculada. Recuperação e abandono exigem o mesmo UUID, inclusive quando chamados diretamente, e a lista de sessões recuperáveis esconde sessões de outras identidades. Estas permanecem armazenadas, com seus proprietários e payloads originais. Os mecanismos de checkpoint do serviço/repositório permanecem disponíveis e não são reescritos pela interface.

Estatísticas conserva os painéis, a seleção responsiva, as cores taxonômicas, os indicadores, as dicas e a rolagem. O painel antes destinado ao ranking passa a “Recordes pessoais”, com melhor pontuação disponível e última pontuação do UUID local. Livre continua indicando grupos/durações variáveis; Competitivo usa a chave exata da configuração vigente, sem comparar regras/conteúdos incompatíveis. O histórico estatístico detalhado também consulta apenas esse UUID e a configuração selecionada. Não se inventam métricas ausentes.

Os seletores Ranking atual/Histórico, a paginação e as consultas introduzidas exclusivamente para aquela proposta foram retirados do código atual. Os contratos anteriores de ranking misto, registros históricos e identidade permanecem no repositório para compatibilidade e evolução futura. O método C++ `ranking()` do controlador mantém uma visão de compatibilidade de no máximo um recorde pessoal; não é exposto ao QML como ranking de identidades.

## Meu perfil

A seção “Meu perfil” permite alterar o nome e informa uso local/offline. Não há seletor, criação ou exclusão de perfis na experiência normal, nem campos de e-mail/senha ou login fictício. “Perfil e ajustes” e “Usuário local” substituem os rótulos anteriores. Animações, Discord, créditos, fontes e transparência permanecem disponíveis.

## Validação

As verificações executadas e as capturas reais desta revisão serão registradas neste diretório após a compilação e os testes, usando exclusivamente bancos temporários. As evidências anteriores continuam preservadas nos diretórios originais.

## Evolução futura

Os [requisitos de contas, autenticação e sincronização](../../../architecture/USUARIO_LOCAL_E_CONTAS.md) distinguem UUID local e usuário remoto. Vincular dados locais a uma conta exigirá decisão explícita e verificações de propriedade; nomes coincidentes ou UUIDs copiados não bastam. Esta revisão mantém funcionamento offline e não autoriza publicação de resultados ou dados pessoais.
