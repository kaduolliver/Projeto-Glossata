# Histórico do aplicativo

Este arquivo registra mudanças do aplicativo com versionamento semântico. A versão do catálogo e o histórico de dados/licenças estão em [CATALOGO_CHANGELOG.md](docs/CATALOGO_CHANGELOG.md). Datas de revisão científica pertencem aos registros de revisão, não à data de lançamento.

## [1.9.0] — Não lançado; seleção taxonômica para jogar

- PR #33: dificuldade junto ao seletor de identificação, com intervalo de 20 pixels lógicos nas janelas amplas e quebra para a linha seguinte nas compactas; início à direita e altura compacta preservados. [Capturas e escalas](docs/interface/VERSOES/2.2.3/alinhamento-dificuldade/README.md).
- PR #33: botão Iniciar partida alinhado à margem interna direita nas janelas amplas, com dificuldade central e largura compacta preservada; composição responsiva e mensagem de elegibilidade mantidas. [Capturas](docs/interface/VERSOES/2.2.3/alinhamento-iniciar/README.md).
- PR #33, ajuste pontual: painel de identificação compacto e adaptativo, fontes do grupo reunidas em diálogo contextual offline e botão Voltar integralmente visível na abertura e navegação taxonômica. Estado da seleção e regras preservados. [Capturas e validação](docs/interface/VERSOES/2.2.3/configuracao-compacta/README.md).
- PR #33, refinamento compatível: configuração direta do modo Livre na página do grupo, sem repetir os painéis globais; dificuldade preservada. Retorno à aba, rolagem e foco de teclado da seleção em memória, reaproveitando a restauração do Explorar. [Capturas e validação](docs/interface/VERSOES/2.2.3/refinamentos-navegacao/README.md).
- Subetapa 2.2.3: seleção fotográfica responsiva com Reinos, Domínios e Grupos avançados, IDs existentes e disponibilidade derivada dos níveis já validados pelo backend. Livre, dificuldade, jogo misto e Competitivo com apresentação própria; regras e Quiz preservados. Alinhamento localizado dos créditos e ações do Explorar. [Registro visual](docs/interface/VERSOES/2.2.3/README.md). MINOR por funcionalidade compatível; catálogo 2.4.0 inalterado.

## [1.8.0] — Não lançado; exploração enciclopédica

- PR #32, usabilidade na versão ainda não lançada: hover dos créditos distinguível do cartão, alturas intrínsecas uniformes por seção, justificativas de foto indisponível no diálogo, botões compactos para carregar mais e faixa comum junto à barra vertical. Roda convencional de 110 pixels por incremento em Início/Explorar, preservando deltas contínuos, galeria, foco e redução de movimento. [Capturas e validação](docs/interface/VERSOES/2.2.2/usabilidade/README.md).
- PR #32, correção compatível na versão ainda não lançada: créditos completos no diálogo contextual também para CC BY 2.0/3.0 e registros legados documentados; aviso de versão não comprovada e pedidos específicos preservados. Botão limitado ao texto/padding, espaços vazios da linha inertes e hover do cartão independente do controle de créditos. [Política, capturas e validação históricas](docs/interface/VERSOES/2.2.2/correcoes-creditos/README.md).
- PR #32, refinamentos finais: cartões reutilizam resumos existentes elegíveis em duas linhas, com diálogo contextual independente para fonte/origem/créditos e atribuições visíveis nos casos conservadores. Fotos centralizadas sem recorte/ampliação, classificação resumida com ranks documentados, rodapé científico abaixo dos descendentes e galeria com indicação de posição. Histórico fotográfico separado do comportamento atual. [Verificação e capturas históricas](docs/interface/VERSOES/2.2.2/rodada-final/README.md).
- PR #32, rodada 2: cartões enciclopédicos compactos, galeria fotográfica navegável, cabeçalho com busca integrada e limpeza apenas do texto, páginas de grupos/organismos com classificação e fontes expansíveis. Retorno à pesquisa conserva lotes carregados, foco e rolagem. Fotos integrais, atribuições, ciência, Jogar e governança preservados. [Capturas e contratos](docs/interface/VERSOES/2.2.2/rodada-2/README.md).
- PR #32, rodada 1: resultados progressivos em lotes de até 12 por seção, fotos enciclopédicas proporcionais com foco nas margens livres e carga condicionada à área visível, breadcrumbs compactos/expansíveis e colunas por largura mínima. Consulta sincronizada imediatamente, atualização visual com debounce de 220 ms e contadores com singular/plural. Correções incorporadas à mesma versão ainda não lançada; catálogo, Jogar e governança preservados. [Registro da rodada](docs/interface/VERSOES/2.2.2/rodada-1/README.md).
- Subetapa 2.2.2 da Issue #22: Explorar com cartões fotográficos, busca científica local por nomes e linhagens, filtros combináveis de domínio e nível, resultados separados por grupos/organismos e acesso ao nó científico por ID estável. Organismos em destaque reutilizam a elegibilidade da Descoberta do Dia.
- Navegação científica baseada no catálogo instalado, sem duplicações por nós históricos do SQLite, mantendo esses registros e os bancos dos jogadores. Breadcrumbs, créditos e avisos científicos preservados; pesquisa e filtros sobrevivem ao retorno dos resultados. Jogar conserva seus controles e regras.
- MINOR pela funcionalidade compatível de busca/filtros. Catálogo 2.4.0 e seus bytes/licenças preservados. Sem lançamento, tag ou autorização de distribuição. Evidências em [2.2.2](docs/interface/VERSOES/2.2.2/README.md).

## [1.7.0] — Não lançado; curadoria e integridade do projeto

- Refinamentos da PR #31: indicador renomeado para Identificação por espécie, com significado explícito e elegibilidade inalterada; créditos contextuais da fotografia da Descoberta do Dia, acesso aos créditos gerais e avisos preservados; fonte educativa sem tooltip; ícones de 28 px em contêineres de 48 px somente nos indicadores da Home.

- Subetapa 2.2.1-B (#22): Descoberta do Dia offline e determinística pela data local, com fotografia de identificação, resumo editorial/especializado existente, fonte e acesso ao nó da espécie em Explorar. Três indicadores passam a contar organismos únicos, domínios representados e organismos com fotografia de identificação utilizável no aplicativo. Carrossel, quiz, persistência e governança preservados; validação e capturas em `docs/interface/VERSOES/2.2.1-B/`.

- PR #30: hover da barra pausa somente nas áreas dos botões/indicadores, permitindo retomada nos espaçadores e intervalos vazios entre os pontos.

- PR #30: legenda e créditos confirmam o novo slide somente ao concluir o crossfade de 600 ms; hover pausa apenas na legenda e nos controles, preservando teclado, créditos, janela inativa e retomada com oito segundos completos. Refinamentos futuros registrados sem implementação.

- PR #30: legenda científica do carrossel diretamente sobre a fotografia, com degradê inferior suave, título do grupo em destaque, descrição e linha discreta de nome científico/Créditos; capturas reais e testes de contraste/layout atualizados.

- PR #30: destaque de foco dos botões compartilhados e pontos do carrossel usa `visualFocus`; cliques mantêm o funcionamento sem borda amarela, preservada em Tab/Shift+Tab.

- PR #30: corrigida a seleção dos quatro slides em catálogos SQLite com grupos históricos homônimos; indicadores apenas por pontos e botão Créditos compacto à direita, com novas capturas reais.

- PR #30: substituída somente a foto do leão na Home por outra de Panthera leo em 1920 × 1247, com identificação e licença CC BY 2.0 próprias; quatro fotos ativas passam a atender FHD, preservando os cartões e os fallbacks.

- Refinamento do PR #30: variantes fotográficas maiores exclusivas da Home, indicadores sem miniaturas, avanço de oito segundos com pausas e crossfade de 600 ms, créditos compactos e novas capturas reais. Fotos dos cartões/quiz e regras preservadas.

- Subetapa 2.2.1-A (#22): Início com quatro fotografias locais, carrossel manual acessível, créditos e fontes por slide, ações Explorar/Jogar e redução de movimento. Nenhuma nova mídia ou alteração de catálogo.

- Subetapa 2.1.3 (#22): navegação lateral com Explorar separado de Jogar, cabeçalho contextual e shell responsivo; sessões e retornos preservados, com testes de navegação e capturas reais.

- Subetapa 2.1.2 (#22): tokens Qt/QML centralizados, quatro componentes compartilhados com estilos consistentes, foco de teclado e redução de movimento; capturas reais e teste de contraste/estados. Telas, catálogo e regras preservados.

- PR #23: referências ativas das duas fotos declaram CC BY sem versão comprovada; URLs inferidos removidos, evidências e decisão editorial expressa preservadas. Uso local exige declaração por arquivo verificável; distribuição continua bloqueada.
- Etapa 1 (#4/#22): duas fotos de origem Casual substituídas por Research Grade, com associação editorial expressa, créditos e proveniência por arquivo; 855 receitas e direitos textuais preservados. Versões de licença ainda pendentes, sem liberação de distribuição.
- PR #21: dicas podem usar a revisão válida da afirmação individual sem exigir aprovação de seu agregado independente. Mantidos os bloqueios de notas sem revisão, vencidas, rejeitadas ou incompatíveis, a progressão leve/moderada/forte e a pontuação. Indicadores científicos passam a consultar a governança atual, sem depender de flags legadas.
- Issue #2: incorporado o lote consolidado após confirmações editorial e de direitos disponíveis: 887 substituições, duas notas mantidas e 264 cópias derivadas. Cobertura textual atual completa por passagem nos três recursos, com 971 novos registros editoriais e os 76 anteriores preservados. Organismos, fotos, regras e dados dos jogadores intactos; distribuição geral permanece bloqueada.
- Gravação de governança tolera recusas transitórias reconhecidas do Windows com retentativas limitadas, mantendo atomicidade, trava e verificação do topo em cada tentativa; erros persistentes continuam sendo recusados.

- Lote técnico da Issue #2 aplicado após as duas confirmações expressas do mantenedor: 15 redações / 1.147 ocorrências, com CC BY 4.0 somente nos direitos disponíveis e validação dos vínculos exatos. Derivados regenerados; ciência, graus, pontos e 76 eventos anteriores preservados. Restam 1.153 ocorrências documentais em três arquivos; permissões integrais e distribuição continuam bloqueadas.

- Issue #2: resumo de prontidão por recurso integrado à investigação existente, com contagem de vínculos pendentes e testes contra cobertura parcial. Preparadas 15 redações técnicas para conferência (1.147 ocorrências), sem aplicação ou concessão automática; diagnóstico da Issue atualizado de sete para três recursos pendentes, preservando acompanhamento humano e bloqueios de distribuição.

- Lote 2 incorporado após confirmação editorial e CC BY 4.0 delimitada: 34 notas/28 organismos, 35 eventos editoriais idempotentes e 805 outras passagens das fichas ainda pendentes. Projeções regeneradas pelo fluxo existente, com aparência de `110` atualizada sem reclassificar assistência ou alterar fotos/pontos; concessões parciais verificadas contra a proposta exata. Distribuição integral continua bloqueada.

- Lote 2 das fichas: revisão assistida por IA das nove propostas, com ressalvas transferidas para metadados quando seguro e referências específicas para micélio. Conferência de 34 campos/28 organismos vinculada a fingerprint e teste de preservação do escopo; aplicação e direitos dependem de duas confirmações concretas. Catálogo ativo e decisões anteriores preservados.

- Continuidade da Issue #2: comparativo das 34 propostas de fichas preparado por organismo/campo, com fontes, licenças, limites e impacto. Propostas e registros de revisão preservados; sequência dos próximos lotes documentada, sem aplicar conteúdo ou ampliar permissões.

- Lote de grupos aplicado após confirmação editorial expressa da versão exata e concessão CC BY 4.0 limitada aos direitos disponíveis: 68 ocorrências documentadas e 38 eventos editoriais idempotentes. Gerador invalida metadados legados após mudança de afirmação/fonte; concessão fica vinculada ao arquivo aprovado. Receitas, graus de assistência, organismo 82 e demais permissões preservados; fichas propostas continuam rascunhos e distribuição permanece bloqueada.

- Regularização textual: lote de 68 redações novas de grupos e proposta delimitada de 34 notas de fichas preparados para conferência, fontes/licenças localizadas e validação do gerador em workspace temporário. Conteúdo ativo e direitos permanecem intactos até decisões reais dos lotes; API bibliográfica do EMBL-EBI incluída explicitamente na política de manutenção, preservando os guardas do transporte.

- Issue #2: vínculos documentais das 2.402 ocorrências educativas agrupados no inventário existente, preservando origem inconclusiva e direitos externos. Gate reforçado contra liberação parcial de arquivos mistos. Após aprovação expressa do mantenedor, registradas as duas notas e o agregado do organismo 82 como revisão editorial, restaurando essas notas sem alterar receitas ou pontos. Três ícones com uso/distribuição documentados exclusivamente nas versões oficiais após autorização específica; sem licença gráfica geral ou concessão textual. Quatro pendências textuais e demais bloqueios do pacote preservados.

- Política editorial de licenciamento misto formalizada por decisão do mantenedor: CC BY 4.0 como padrão para redações próprias elegíveis, sem concessão automática sobre o corpus. Inventário existente recebe contrato documental por ocorrência e validações de vínculo/evidência, com alertas SA/NC/ND; textos, ícones e fotografias mantêm pendências. Sem alteração de ciência, interface ou versão do aplicativo.

- Revisão do PR #16: declaração literal de geração por texto delimitada à origem declarada da arte, sem inferir ausência de imagens nas etapas de edição, preparação da variante Discord ou conversão para ICO. Etapas desconhecidas preservadas como tais; não há mudança em imagens, ciência, aprovações ou permissões.

- Saneamento concentrado da Issue #2: inventário de passagens e vínculos de origem incorporado ao relatório de direitos existente; projeção de dicas conferida sem criar aprovações. Dois trechos sobre a nutrição de Pseudoboletus parasiticus corrigidos como rascunhos fundamentados, sem alterar receitas/pontos. Nova declaração real de geração dos ícones somente por texto preservada separadamente. Proposta de licença educativa preparada, sem ativação; sete pendências de direitos mantidas. Veja [SANEAMENTO_ISSUE_2.md](docs/SANEAMENTO_ISSUE_2.md).

- Política visual reservada escolhida expressamente pelo mantenedor: sem concessão gráfica geral para projetos independentes, limitada aos direitos disponíveis e respeitando Apache-2.0, visualização/fork e usos legalmente permitidos. Registro administrativo vinculado aos três recursos, sem alterar imagens, permissões ou liberar distribuição. Fluxo de substituição em forks e pendências individuais documentados em [POLITICA_BRANDING_IMPLEMENTACAO.md](docs/POLITICA_BRANDING_IMPLEMENTACAO.md).

- Identidade visual: decisão administrativa de preservar os três ícones atuais, com documentação independente de finalidades, origem declarada e proposta gráfica não ativada. Bytes dos ícones, licenças, permissões e histórico preservados; nenhuma aprovação de distribuição. Consulte [IDENTIDADE_VISUAL.md](docs/IDENTIDADE_VISUAL.md).

- Correções finais do PR #12: referência de Euglenida confirmada na editora como BMC Biology (2020), DOI 10.1186/s12915-020-0754-1; rótulos e hashes derivados atualizados, sem alterar descrições ou regras. Documentada a licença CC BY-NC-SA 4.0 do Biology 2e web atual e a falta de identificação da revisão histórica utilizada; direitos educativos continuam pendentes, sem presumir infração ou originalidade integral.
- Issue #2: redação do gerador extraída sem alteração para `group_education.json`, com validação offline, isolamento de workspace, comparação das saídas e rastreabilidade no inventário. Lógica Python separada incluída explicitamente no escopo Apache-2.0; textos e três ícones continuam pendentes. Origem dos ícones declarada pelo mantenedor como criação via ChatGPT, sem presumir direitos sobre referências ou licença gráfica. Sete achados preservados; relatório em [SANEAMENTO_ISSUE_2.md](docs/SANEAMENTO_ISSUE_2.md).
- Apache-2.0 adotada por autorização expressa para 155 arquivos de código próprio enumerados, com LICENSE oficial integral, NOTICE reconhecendo Carlos Eduardo Lima de Oliveira como criador original e verificação offline na CI. Gerador misto, textos/coletânea, mídias, marca e dependências excluídos da concessão automática; permissões e revisões anteriores preservadas. Política embarcada e identidade do catálogo 2.4.0 ainda não lançado atualizadas, sem mudar versão do aplicativo, conteúdo científico ou liberar distribuição.
- Preparação de licenciamento no PR #10, anterior à decisão acima: inventário documental reproduzível por arquivo/componente, comparação MIT/Apache-2.0/GPLv3, proposta de escopo não ativada e orientações de contribuição. Inspeção do pacote Qt 6.11.2 e demais dependências identifica aviso ICU fora do diretório coletado; sem exportação, escolha de licença, `LICENSE`, alterações do catálogo ou aprovações naquela etapa. Plano em [PLANO_LICENCIAMENTO.md](docs/PLANO_LICENCIAMENTO.md).
- Admissão auditável: proposta offline de novos organismos com identidade externa, deduplicação, evidências arquivadas, direitos por camada e revisão editorial no ledger existente. Cópias manuais de novos IDs sem os requisitos são recusadas pela auditoria. Relatório de prontidão diferencia catálogo, Git atual/histórico público, capturas e pacote; reaproveita snapshots sem novas consultas. Nenhuma espécie, fotografia, aprovação ou decisão real adicionada; distribuição permanece bloqueada.
- Estabilização integrada: abertura/migração do SQLite recusa estruturas incompletas e confere vínculos legados sem reescrever resultados. Backup valida a estrutura do manifesto e recusa destinos com sidecars SQLite. Regressões de rollback, interrupções e snapshots inconsistentes ampliadas; ruleset ativo 25303072 confirmado somente por leitura. Catálogo, pontuação, interface e decisões de curadoria preservados; relatório em [AUDITORIA_ESTABILIZACAO.md](docs/AUDITORIA_ESTABILIZACAO.md).
- INV-08: investigação estruturada de direitos/associações por foto/material, coleta pública limitada e minimizada em propostas, comparação com diagnóstico histórico e distinção entre catálogo, Git/checkout e pacote. Reutiliza proveniência, fila e gate existentes; dependências avaliadas separadamente. Sem concessões, remoções, alteração de autoria/taxonomia, decisões automáticas ou lançamento.
- INV-09: garantias de integridade/rastreabilidade distintas de autoria declarada/autenticada; comparação offline dos três ledgers/baseline com commit-base completo, recibo mínimo e verificação técnica na CI. Fichas explicitam responsável declarado; regressões de rehash, truncamento, referências Git e preservação de registros. Sem assinatura/credencial criada, mudança de evento, aprovação, versão ou configuração remota.
- INV-10: estado atual gerado de versões/manifestos/regras/esquemas e inventário CTest, com blocos consistentes no README/PROJECT_CONTEXT, validação contra o build configurado, testes adversariais e gate técnico na CI/preparação de tag. Relatórios históricos identificados; instruções de build/execução e candidatos corrigidas. Nenhuma versão, regra, dado científico ou publicação alterada por esta correção documental.
- INV-03: checkpoints SQLite v6 transacionais após respostas/dicas, com questões/alternativas/tetos e identidade permanente; recuperação com continuação/abandono e fechamento seguro. Revisões impedem sobrescrita por janela antiga; ranking e finalização são atômicos e idempotentes. Backup SQLite com WAL, manifesto e restauração em arquivo novo; testes com interrupção abrupta. Captura real inicial atualizada; catálogo 2.4.0 preservado, sem lançamento.
- INV-07: política central versionada de nomes NFC/case folding simples/NFC, com grafia preservada e mesma regra em criação e renomeação. Migração SQLite v5 audita/recalcula chaves, preserva IDs/resultados e registra colisões antigas sem fusão; reabertura verifica política, versão Unicode e integridade das chaves. Testes multialfabeto, acentos, formas canônicas, conflitos, rollback e reinicialização. Sem lançamento ou alteração do catálogo.
- INV-01: identidade permanente de jogadores, propriedade de partidas/métricas por ID, snapshot do nome inicial e exclusão lógica. Migração SQLite v4 transacional/idempotente arquiva tabelas anteriores, conserva rankings/pontos e mantém vínculos antigos não demonstráveis como legado pendente. Seleção QML por ID, novos conflitos Unicode recusados sem unir perfis antigos; catálogo 2.4.0 e curadoria preservados. Sem lançamento.
- INV-05: escritor comum para revisões científicas, permissões e dependências, com trava persistente fora de `build/`, posse por thread/processo, espera limitada e verificação do topo imediatamente antes da gravação atômica. APIs, CLIs e bridge compartilham a proteção; idempotência/recuperação do INV-02 e hashes históricos preservados. Trava operacional excluída do catálogo e pacote e recusada no índice; testes com processos concorrentes e falhas abruptas. Sem lançamento ou alteração de curadoria.
- INV-02: permissões com eventos como fonte de verdade, projeção recuperável, identidade idempotente e escrita atômica sincronizada. Bridge verifica/reconcilia antes de `already_recorded`; importador e migração preservam decisões existentes sob a mesma trava. Falhas injetadas e reinicialização em fixtures temporárias; inconsistências ambíguas bloqueadas. Catálogo 2.4.0 e registros reais preservados; sem lançamento.
- INV-06: transporte HTTPS de manutenção com hosts/porta explícitos, redirecionamentos validados, DNS público fixado na conexão, TLS verificado e limites de tempo/dados/saltos. Testes sem Internet e propostas de recusa sem alterar conteúdo, permissões ou curadoria; catálogo 2.4.0 preservado.
- INV-04: proposta local de proteção da `main` para mantenedor único, com PR sem aprovação externa, bloqueio de force push/exclusão e somente `offline-validation` obrigatório. Inspetor somente GET, responsáveis por caminhos sensíveis, testes e checkout sem credenciais persistidas. Ativação remota pendente de autorização; regras de distribuição e catálogo inalterados.
- Limpeza de 37 fotografias legadas fora do bundle (4,9 MB), com hashes, motivos e referências históricas preservados em [LIMPEZA_REPOSITORIO.md](docs/LIMPEZA_REPOSITORIO.md). Recursos ativos, dados dos jogadores e catálogo 2.4.0 inalterados; bloqueios de distribuição mantidos.
- Publicação semanal protegida pela variável `GLOSSATA_CURATION_WEEKLY_SYNC`, desabilitada por padrão. Auditorias e simulação autenticada com permissões de leitura continuam semanais; primeira publicação manual e ativação exigem autorizações separadas.
- Tabelas dos relatórios e corpos de Issues mantêm linhas contíguas para renderização correta no GitHub.
- Validação em checkout limpo: teste visual cria seu diretório de capturas; cliente GitHub só inicializa transporte/TLS após validar a requisição e o orçamento, sem abrir rede nos testes offline. Falhas HTTP fecham a resposta e não são repetidas.
- Ativação remota autorizada em 2026-10-16: simulações reais sem escrita, seis Issues únicas em duas rodadas limitadas e atualização controlada com preservação de anotação técnica. Chave semanal habilitada após autorização separada; resultados, execuções e limitações documentados. Sem Project, tag, release ou liberação de distribuição.
- Fila pública mínima derivada dos auditores existentes, com IDs/hashes estáveis, agrupamento, prioridade, referências verificadas e limitação de publicação.
- Integração GitHub Issues idempotente, preservação de notas/etiquetas humanas, paginação completa e limites de chamadas/criação/atualização. Issue encerrada ou etiqueta de aprovação não altera a governança.
- Formulários científicos, fotográficos, educacionais e de direitos; execução semanal/manual e integração opcional com GitHub Projects v2, em campo administrativo independente.
- Modelos locais para pedidos individuais de autorização e decisões humanas; aprovações explícitas encaminhadas aos históricos existentes com hash, responsável, justificativa e controle da decisão anterior. Nenhum contato ou consentimento é automatizado.
- Na etapa inicial de curadoria, anterior às correções INV acima: catálogo **2.4.0** preservado byte a byte, inclusive revisões e permissões, sem mudança em quizzes, QML, layout ou bancos dos jogadores naquela etapa. MINOR por ferramentas de manutenção compatíveis; sem tag, push ou release.

## [1.6.0] — Não lançado; evolução integrada

- Avaliação pedagógica automatizada explícita para a versão de testes, separada das revisões científicas e permissões. Receitas offline, datas reais, prazo configurável, fontes e hashes; alterações invalidam a avaliação sem renovação silenciosa.
- Dicas com IDs independentes da categoria, três posições progressivas de 6/3/2 unidades, seleção direta, categorias repetidas justificadas e lacunas explícitas. O Competitivo prioriza perguntas completas por grupo, conserva 20 questões e a variedade; o Livre mantém os organismos e níveis válidos.
- Snapshots `identification-v4-progressive` incluem texto, categoria, assistência, fonte, evidência automatizada, alternativas e conteúdo. Política v3 congelada para histórico; nenhum recálculo de pontos. Ranking interno separa versões no mesmo programa.
- Relato de dica com rascunho local e abertura externa somente por iniciativa do jogador; sem respostas/dicas ocultas, perfis ou bancos no relato.
- Auditoria Git/CI e ampliação fotográfica em commits anteriores desta evolução: Git no MSYS2, auditor de índice e 109 imagens de apresentação, 534/623 cartões (85,71%), 89 pendências. Fotografias e identidades do quiz preservadas.
- Catálogo **2.4.0**, sucessor de 2.3.0; funcionalidade compatível (MINOR). Sem tag, push ou release; gate de distribuição conserva pendências legítimas.

## [1.5.0] — Não lançado; refinamento fotográfico

- Fotografias de fundo com 42% da largura e altura integral, recorte proporcional, ponto focal editável, gradiente horizontal e cantos arredondados; sem novos efeitos de GPU ou consultas durante o jogo.
- Controles de alternativas e modo competitivo restaurados à posição anterior à expansão fotográfica, mantendo atalhos e navegação.
- Descoberta paginada, busca CC0, preparação com licença por arquivo antes do download e auditoria de cobertura completa. Fotografias incompatíveis, ilustrações, crédito não demonstrável e metadados sensíveis são recusados.
- Catálogo **2.2.0**: 98 fotos de apresentação, 503 associações explícitas a grupos mais o cartão misto; 119 pendências justificadas. Todos os 623 cartões têm foto ou motivo documentado. Os 122 organismos e 120 fotos de quiz preservam seus bytes e identificadores; nenhuma revisão humana, elegibilidade, pontuação ou conquista é alterada.
- Testes de cobertura, identidade, direitos, recursos offline, pontos focais, renderização e posição dos controles ampliados. Classificações visuais permanecem rascunhos de IA ligados à identificação da fonte; pendências legadas de distribuição continuam.
- MINOR compatível pelas ferramentas de preparação e ampliação da apresentação; sem tag, push ou release pública.

## [1.4.0] — Não lançado; em desenvolvimento

- Infraestrutura modular de manutenção com adaptadores reais GBIF, Catalogue of Life/ChecklistBank, WoRMS e Wikimedia Commons; plano explícito para NCBI Taxonomy, LPSN, POWO e MycoBank. Sem novas dependências do executável ou consultas durante o jogo.
- Candidatos em SQLite separado, IDs externos qualificados por provedor/release, snapshots por hash, replay offline e consultas limitadas. Reconciliação conservadora de sinônimos, homônimos e divergências; nenhuma fusão por votação ou publicação automática.
- Descoberta por ancestral verificado prioriza Archaea e Mollusca; ausência de rota para Discoba é registrada. Mídias são apenas propostas de metadados com licença por arquivo, sem downloads ou aprovações automáticas.
- Expansão direta pelo importador legado bloqueada antes da rede/escrita; reimportação dos registros existentes continua compatível. Auxiliares de pesquisa/links respeitam a restrição da LPSN contra HTML automatizado. Gate detecta candidatos indevidamente embarcados.
- Nona suíte CTest com 26 testes offline; verificações reais limitadas dos quatro provedores e replay documentados. Aplicativo recebe MINOR por ferramentas compatíveis. Catálogo **2.1.0** reutilizado sem alteração de organismos, fotos, revisões, pontuação, perfis ou conquistas.
- Sem tag, push ou publicação; bloqueios legados de distribuição e decisões humanas documentadas permanecem.

## [1.3.0] — Não lançado; em desenvolvimento

- Acesso rápido a Animalia, Plantae e Fungi sem duplicar nós ou alterar suas posições filogenéticas. Os grupos filhos passam a preceder os controles de quiz na exploração; breadcrumbs e filtros conservam os identificadores científicos.
- Fotografias reais nos cartões, incluindo micrografias identificadas para Bacteria e Archaea. Componente reutilizável com transparência progressiva nas bordas e centro opaco, compatível com o renderizador software do Qt. Ilustrações preservadas onde faltam fotos adequadas.
- Catálogo **2.1.0**: oito imagens de apresentação, 276.666 bytes, CC0/CC BY com evidências por arquivo, créditos automáticos e inventário offline. Nenhuma das 120 fotografias de quiz ou dos 122 organismos foi substituída.
- Auditoria e testes cobrem licenças, identidade, recusa antes do download, transparência real, atalhos, filtros e acesso ao catálogo inteiro. Capturas reais atualizadas; bancos temporários nos testes. Dados dos jogadores e regras de pontuação preservados.
- Incrementos MINOR compatíveis do aplicativo e do catálogo. Sem tag, publicação ou push; bloqueios legados de redistribuição e pendências científicas permanecem documentados.

## [1.2.0] — Não lançado; em desenvolvimento

- Navegação pelos domínios Bacteria, Archaea e Eukaryota e por linhagens evolutivas, com fontes, divergências e limitações. Novos enquadramentos permanecem rascunhos até revisão humana.
- Modelo distingue domínio, supergrupo sem rank formal, clado e os ranks do backbone original. Monera e Protista ficam restritos à compatibilidade e às referências históricas.
- Catálogo **2.0.0**, com projeção rastreável de 122 organismos e preservação das fotografias, créditos, identidades, dados dos jogadores e pontuação. Migração aditiva/transacional do catálogo SQLite v2; histórico de partidas permanece em seu esquema e regras anteriores.
- Nenhum rank novo liberado automaticamente para quiz; revisões e quatro alternativas distintas continuam obrigatórias. Novos hashes de conteúdo separam os rankings competitivos antigos.
- Versão MINOR do aplicativo por funcionalidade compatível; o esquema do catálogo recebe MAJOR pela nova organização. Sem tag ou release enquanto persistirem os bloqueios documentados.

## [1.1.0] — Não lançado; histórico da preparação anterior

Base anterior: CMake `1.0`, equivalente a `1.0.0`; não havia tags de lançamento nem histórico formal. Esta entrada prepara uma versão MINOR, pois acrescenta funcionalidades compatíveis. Não representa uma release publicada.

### Adições

- Captura real do quiz no README, com perfil temporário, atribuição da fotografia e registro da origem da captura.
- Governança por tópico: evidências, responsável, revisão editorial ou por especialista, expiração configurável, histórico e invalidação após mudanças.
- Créditos offline e informações de revisão nas fichas; caminho para comunicar erro ou problema de atribuição.
- Importação com política de licença por fotografia e propostas de atualização separadas do conteúdo publicado.
- Versões independentes do aplicativo e catálogo; manifestos com hashes, controles de lançamento e captura real da tela inicial no README.

### Alterações

- Rótulos de assistência mais legíveis no painel de dicas: leve em verde, moderada em amarelo e forte em coral, com maior destaque no texto.
- Dicas recebem assistência independente da categoria, com teto combinado anunciado antes da revelação. Regras `identification-v3-assistance`, snapshots verificáveis e migração SQLite v3 preservam pontos antigos e separam rankings competitivos.
- Auditoria de 290 avaliações: 132 propostas de reclassificação, quatro inadequações como morfologia prévia e nenhuma nova aprovação humana. O Livre sinaliza propostas; o Competitivo bloqueia dicas sem classificação confiável.

- Nome apresentado: **Projeto Glossata**, inclusive janela, executável, launcher e workspace. Identificador interno e localização dos bancos dos jogadores preservados.
- Aprovações antigas sem prova foram registradas como legado. Novos textos assistidos por IA permanecem rascunhos até revisão humana.
- Empacotamento público bloqueia recursos sem permissão demonstrável; pacote de desenvolvimento é exclusivo para testes locais.

### Catálogo e direitos

- Acompanha o catálogo **1.0.0**, ainda não lançado, identificado em `resources/data/catalog_manifest.json` e no manifesto de compilação.
- 122 organismos e 120 fotografias distintas preservados; não houve inclusão, remoção ou renomeação de táxons nesta etapa de governança.
- 120 snapshots de permissão e 1.660 registros de revisão legada/pendente; nenhuma nova aprovação humana ou licença individual concedida.
- Não foi escolhida licença global para os materiais próprios. As pendências de fotos, textos, arte e dependências estão em [RELATORIO_CONFORMIDADE.md](docs/RELATORIO_CONFORMIDADE.md).

### Verificações e limites

Build, CTest, lint QML e auditoria técnica são requisitos de integridade. Metadados de versão e captura também são conferidos. **O gate obrigatório de distribuição continua bloqueado** por permissões e achados científicos prioritários; não criar tag nem publicar release enquanto falhar. Testes não constituem certificação jurídica ou científica.
