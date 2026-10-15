# Histórico do catálogo científico e das licenças

A versão do catálogo é independente de `VERSION.txt` (aplicativo). Cada snapshot registra os hashes dos dados, revisões, políticas, evidências e mídias em `resources/data/catalog_manifest.json`. O manifesto é embarcado no executável, copiado para o pacote e referenciado pelo manifesto de compilação. Git preserva os snapshots anteriores; nenhuma revisão humana recebe uma nova data pela atualização da versão.

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
