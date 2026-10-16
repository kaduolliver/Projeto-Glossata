# Histórico do aplicativo

Este arquivo registra mudanças do aplicativo com versionamento semântico. A versão do catálogo e o histórico de dados/licenças estão em [CATALOGO_CHANGELOG.md](docs/CATALOGO_CHANGELOG.md). Datas de revisão científica pertencem aos registros de revisão, não à data de lançamento.

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
