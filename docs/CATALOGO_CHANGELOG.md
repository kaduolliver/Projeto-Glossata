# Histórico do catálogo científico e das licenças

A versão do catálogo é independente de `VERSION.txt` (aplicativo). Cada snapshot registra os hashes dos dados, revisões, políticas, evidências e mídias em `resources/data/catalog_manifest.json`. O manifesto é embarcado no executável, copiado para o pacote e referenciado pelo manifesto de compilação. Git preserva os snapshots anteriores; nenhuma revisão humana recebe uma nova data pela atualização da versão.

## [2.1.0] — Não lançado; fotografias de exploração

Aplicativo previsto: **1.3.0**; reutilizado pelo **1.4.0** sem mudança dos dados/revisões/mídias publicados. O fluxo de múltiplos provedores do 1.4.0 produz somente candidatos fora do bundle, sem incrementar a versão ou alterar o hash deste catálogo. Acréscimo MINOR compatível de recursos de apresentação e proveniência no 2.1.0; sem publicação.

- Oito miniaturas reais CC0/CC BY por arquivo, com identificador/revisão do Commons, autor indicado pela fonte, licença/versão/URL demonstradas, coleta real, hash, dimensões e modificações. Manifesto separado `exploration_photos.json`; evidências em `resources/compliance/exploration/`; inventário e créditos automáticos atualizados.
- Representantes: leão, monarca, girassol, Amanita, Macrocystis, Physarum e micrografias de E. coli e Sulfolobus sp. A20 (identificação histórica da fonte, com nota sobre Saccharolobus). A micrografia bacteriana é colorizada pela fonte. São imagens de apresentação; não adicionam registros nem aprovam identificação no quiz.
- Os 122 organismos, 120 fotos do quiz, nomes/identificadores, classificação, referências, revisões e evidências anteriores ficam preservados. Nenhuma nova aprovação humana ou de especialista. Escopos de associação assistidos por IA continuam explicitados como rascunhos.
- Discoba e ramos sem imagem apropriada mantêm ilustrações. O novo inventário não resolve nem reduz os bloqueios de licenciamento/curadoria dos recursos legados. Consultas de links geram propostas, sem alterar evidências publicadas.
- O manifesto passa a identificar também essas imagens, metadados e evidências. As imagens de apresentação não integram o fingerprint competitivo: espécies, taxonomia, conteúdo educativo e regras não mudaram neste lote. Rankings e resultados antigos são preservados, sem recálculo.

## [2.0.0] — Não lançado; projeção filogenética

Aplicativo previsto: **1.2.0**. Mudança MAJOR do esquema de organização; sem publicação de release.

- Os 122 IDs e nomes de organismos, as 120 fotografias e todos os metadados de crédito/licença são preservados. O registro de integridade em `resources/compliance/taxonomy_migration_v1.json` permite comparar a migração.
- Raízes Bacteria (20), Archaea (0) e Eukaryota (102); Opisthokonta (62), Archaeplastida (20), SAR (10), Amoebozoa (10) e Discoba (0). Não há novos organismos nem ranks inventados para preencher lacunas.
- `source_taxonomy` preserva integralmente a classificação original por organismo. `legacy_educational_kingdom` preserva apenas a associação histórica. A árvore ativa retira Monera, Protista, Protozoa e Chromista; Bacteria passa a domínio. Animalia e Fungi ficam em Opisthokonta; Plantae, no sentido do backbone original, em Archaeplastida.
- Política/fonte em `phylogeny.json`; divergências por registro e 122 revisões independentes de filogenia em rascunho. Revisões antigas, evidências de foto e datas não foram renovadas. Fontes consultadas não são aprovação por especialista.
- Nenhuma licença modificada ou concedida. Permanecem os bloqueios de redistribuição e curadoria anteriores. O hash de conteúdo competitivo muda; partidas e pontos históricos não são recalculados.

## [1.0.0] — Não lançado; baseline inicial versionada

Aplicativo previsto: **1.1.0**. Os números identificam uma baseline técnica, sem afirmar que o conteúdo foi cientificamente aprovado ou liberado para redistribuição.

- **Assistência pedagógica:** 290 propostas explícitas, independentes da revisão científica: 122 taxonômicas, 64 etimológicas e 104 morfológicas. Há 132 mudanças propostas de teto e quatro inadequações prévias registradas em `AUDITORIA_ASSISTENCIA.json`. Todas são rascunhos de origem IA; nenhuma aprovação humana, fonte, foto ou etimologia foi inventada. Dicas sem revisão confiável ficam indisponíveis no Competitivo.
- **Histórico:** mudanças de assistência integram o hash do conteúdo competitivo. Revisões científicas e permissões anteriores permanecem preservadas; avaliação pedagógica não equivale a aprovação científica ou concessão de licença.

- **Taxonomia e fichas:** 122 organismos, distribuição didática de cinco reinos e taxonomia da fonte preservadas. Nenhum táxon adicionado, removido ou renomeado neste lote. Estados efetivos de revisão agora distinguem legado, rascunho, pendente e revisão humana rastreável.
- **Revisões:** migração preserva 1.568 registros legados, 91 pendentes e um em análise. Zero novas aprovações humanas; 1.660 registros no total. Datas de arquivamento de evidências não foram tratadas como datas históricas de coleta/revisão.
- **Fotos e créditos:** 120 fotos distintas; crédito literal e evidência mínima preservados, com sete divergências sinalizadas. Autor não foi inferido a partir do observador. Nenhuma nova concessão de licença.
- **Política:** padrão de novas fotos CC0/CC BY; licenças ausentes, desconhecidas ou fora da política bloqueadas antes do download. Exceções exigem autorização documentada e escopo compatível; NC exige política expressa. Não há exceção ativada nesta baseline.
- **Pendências graves:** origem Casual para as fotos 659755043 (*Panthera tigris*) e 707161720 (*Lemur catta*), pendentes de análise. Versão/data da licença e confirmação humana das 120 fotos não estão demonstradas. Consulte [o relatório de conformidade](RELATORIO_CONFORMIDADE.md) para os demais bloqueios.

Para um catálogo já lançado, correções compatíveis de conteúdo/atribuição usam PATCH; acréscimos compatíveis de conteúdo ou campos opcionais usam MINOR; alterações incompatíveis de esquema, identificadores ou interpretação usam MAJOR. Toda mudança de bytes publicados, evidências ou decisões de revisão exige novo snapshot e entrada no histórico. O incremento não aprova o conteúdo. Identificadores competitivos existentes continuam baseados no hash das regras/dados e preservam os resultados antigos.
