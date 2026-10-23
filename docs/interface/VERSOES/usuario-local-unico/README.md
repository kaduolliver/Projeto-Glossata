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

Implementação em [17d8492](https://github.com/kaduolliver/Projeto-Glossata/commit/17d8492) (vínculo local, migração, backups e testes) e [88b3813](https://github.com/kaduolliver/Projeto-Glossata/commit/88b3813) (controlador, interface, versão e captura oficial). A PR continua sendo [#37](https://github.com/kaduolliver/Projeto-Glossata/pull/37).

`ctest --test-dir build/single-local-user --output-on-failure`: **26/26 suítes aprovadas**, em **563,92 segundos**. QtTest registra 62 casos de regras/controlador, 36 de identidade, 21 de recuperação e 30 de interface, incluindo inicialização/finalização. Seis testes de Estatísticas/transição também passaram separadamente nos fatores Qt 100%, 125% e 150%. Todos os bancos são temporários; não há teste ou captura sobre dados pessoais reais.

| Requisito | Verificação |
| --- | --- |
| Primeira execução, UUID estável, reabertura e renomeação | `localUserFirstOpenAndStableBinding`, `gameKeepsStartingPlayer` |
| Instalação SQLite 6 com um perfil, UUID/resultados preservados e esquema 7 | `localUserSchema6PreservesSingleProfile`, migrações anteriores e backups |
| Vários perfis, escolha explícita única, rejeição de legado e disputa entre instâncias | `localUserExplicitChoiceAndGuards`, `localUserTransitionKeepsOwnersAndSessions` |
| Proteção do vínculo contra update/delete/replace e perfis inelegíveis | Gatilhos verificados em SQLite temporário; banco incompleto recusado na reabertura |
| Sessão própria recuperável; sessão de terceiro invisível, não recuperável e não abandonável | `localUserTransitionKeepsOwnersAndSessions`, suíte de recuperação; comparação do payload alheio antes/depois |
| Recordes e métricas por UUID; nenhuma herança da pontuação maior do outro perfil | Testes de transição e `singleLocalUserSettingsRecordsAndCaptures` |
| Modalidades/configurações separadas, conteúdo competitivo completo e ausência de métricas inventadas | `personalCompetitiveConfigurationStaysSeparate`, `statisticsContextsAndConfirmedMetrics` |
| Meu perfil, ausência de criação/seleção/exclusão permanente, teclado, foco, painéis e rolagem | Testes Qt/QML em nove combinações de resolução/fator |
| Quiz, dicas, taxonomia, exploração, conquistas e demais fluxos existentes | Suíte completa de regras/interface, testes de dicas e recuperação |

A primeira execução encontrou uma fixture de recuperação que usava o catálogo JSON, enquanto o controlador usa SQLite, e uma expectativa antiga de esquema 6. A fixture passou a usar o mesmo catálogo da produção e a expectativa passou a 7; as regras de integridade/recuperação não foram relaxadas. Os logs `initial-*` preservam essa execução, ao lado dos testes finais aprovados e do teste direcionado após a correção.

Lint de todo o QML, smoke test isolado do executável, auditorias do repositório/catálogo/licenças, metadados e `release.py check`/`check-history` passaram. A comparação de governança preserva os 1.053 eventos da base. [Resultados e hashes](verificacoes/resultados.json), [CTest](verificacoes/ctest.txt), [identidades](verificacoes/identity-results.txt) e [recuperação](verificacoes/recovery-results.txt) registram os limites técnicos dessas verificações; não concedem aprovação científica/jurídica ou autorização de distribuição.

## Capturas reais

O [manifesto](capturas.json) contém **36 capturas reais**, dimensões, horários e hashes de fontes/binários/PNGs. Usa `QQuickWindow::grabWindow` sobre QML e controlador de produção, com registros sintéticos em SQLite temporário e Discord desativado. Não houve mockup ou substituição por mapa de apresentação. Ambiente Windows 11, Qt 6.11.2, backend software/offscreen e `QT_SCALE_FACTOR` nos fatores abaixo; não é certificação de mudanças físicas entre monitores/DPI.

| Resolução lógica | 100% | 125% | 150% |
| --- | --- | --- | --- |
| 1000 × 700 | [Recordes](capturas/100/personal-free-1000x700.png) | [Recordes](capturas/125/personal-free-1000x700.png) | [Recordes](capturas/150/personal-free-1000x700.png) |
| 1240 × 820 | [Recordes](capturas/100/personal-free-1240x820.png) | [Recordes](capturas/125/personal-free-1240x820.png) | [Recordes](capturas/150/personal-free-1240x820.png) |
| 1920 × 1080 | [Recordes](capturas/100/personal-free-1920x1080.png) | [Recordes](capturas/125/personal-free-1920x1080.png) | [Recordes](capturas/150/personal-free-1920x1080.png) |

Também há imagens da [escolha pontual sem pré-seleção](capturas/100/transition-1000x700.png), de [Meu perfil](capturas/125/my-profile-1240x820.png) e do [Competitivo sem recorde elegível](capturas/150/personal-competitive-empty-1240x820.png). A captura oficial da Home e seu registro foram renovados por `scripts/capture_home.py --binary build/single-local-user/projeto_glossata.exe`, no mesmo commit das alterações visuais. [O caminho oficial](../../../images/tela-inicial.png) permanece no README.

Os binários e manifestos brutos de build continuam locais. `verificacoes/resultados.json` identifica por hash o manifesto `build/build-manifest.json`, sem versionar um artefato gerado proibido pela auditoria. As capturas e resultados da direção anterior da PR #37 foram preservados integralmente; seu README recebeu apenas um aviso de substituição.

## Inventário não destrutivo

Após os testes/builds, foram executados `python scripts/cleanup_build.py` e `python scripts/cleanup_build.py --include-old-builds`, **somente em dry-run**, conforme solicitado. Ambos retornaram zero candidatos elegíveis por recência/retenção. A PR permanece aberta: não houve `--apply` nem limpeza/exclusão de arquivos.

| Caminho/agrupamento | Tamanho aproximado | Motivo e risco; ação |
| --- | --- | --- |
| `build/single-local-user/` | 1,78 GiB | Compilação, capturas, scripts locais e logs atuais; alto risco para a rastreabilidade; preservar. |
| `build/local-ranking-history/` | 1,79 GiB | Binários e evidências da direção anterior da PR; alto risco; preservar. |
| `build/tests/previews/` | 225,3 MiB | Capturas históricas; alto risco; preservar. |
| `build/__pycache__/` | 172 KiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `scripts/__pycache__/` | 1,30 MiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `tests/__pycache__/` | 565 KiB | Cache regenerável; candidato potencial de baixo risco, sem autorização de exclusão; preservar. |
| `build/*.{py,txt,log,png,tmp}` | 38,9 MiB | 283 auxiliares, relatórios e capturas avulsos, possivelmente únicos; alto risco; preservar. |

O [inventário completo](verificacoes/auditoria-arquivos.json) informa bytes, caminhos, motivos, riscos, planos e arquivos não versionados. Não encontrou duplicações adicionais no escopo das novas capturas; as cópias dos 36 PNGs para a documentação são intencionais. Os quatro grupos identificados no [inventário anterior](../ranking-local-historico/verificacoes/auditoria-arquivos.json) continuam preservados. Igualdade de bytes não autoriza exclusão. `resources/`, `package/`, SQLite/dados pessoais, mídias, governança, manifestos, documentação e evidências anteriores não foram usados como alvo de limpeza.

## Evolução futura

Os [requisitos de contas, autenticação e sincronização](../../../architecture/USUARIO_LOCAL_E_CONTAS.md) distinguem UUID local e usuário remoto. Vincular dados locais a uma conta exigirá decisão explícita e verificações de propriedade; nomes coincidentes ou UUIDs copiados não bastam. Esta revisão mantém funcionamento offline e não autoriza publicação de resultados ou dados pessoais.
