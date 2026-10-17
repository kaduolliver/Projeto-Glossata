# Histórico do aplicativo

Este arquivo registra mudanças do aplicativo com versionamento semântico. A versão do catálogo e o histórico de dados/licenças estão em [CATALOGO_CHANGELOG.md](docs/CATALOGO_CHANGELOG.md). Datas de revisão científica pertencem aos registros de revisão, não à data de lançamento.

## [1.7.0] — Não lançado; curadoria e integridade do projeto

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
