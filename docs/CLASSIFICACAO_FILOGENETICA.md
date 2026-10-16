# Classificação filogenética e migração

Aplicativo **1.2.0** e catálogo **2.0.0**, em desenvolvimento. O sistema `phylogeny-v1` organiza a exploração offline por **Bacteria, Archaea e Eukaryota**. Não cria espécies, fotos ou aprovações para preencher grupos vazios.

## Critério científico e limites

Os três domínios são portas didáticas baseadas na [proposta de Woese, Kandler e Wheelis (1990)](https://doi.org/10.1073/pnas.87.12.4576). Essa apresentação não afirma que os três sejam ramos equivalentes de uma árvore resolvida. Análises de Asgard sustentam uma origem dos eucariotos dentro de Archaea; trabalhos de [2025](https://doi.org/10.1038/s41586-025-08955-7) e [2026](https://doi.org/10.1038/s41586-026-10128-z) diferem quanto à posição dos parentes arqueanos mais próximos. As hipóteses e a correção indicada na página do estudo de 2025 ficam registradas nas fontes.

A organização eucariótica consulta [Adl et al. (2019)](https://doi.org/10.1111/jeu.12691), especialmente Systematics, Figure 1 e as entradas dos clados, e [Burki et al. (2020)](https://doi.org/10.1016/j.tree.2019.08.008). O esquema de Adl não atribui ranks formais a todos os clados. Portanto, **supergrupo é um rótulo de navegação**, distinto de reino ou filo. A interface também admite clados sem rank formal.

Esta é uma árvore de navegação com ramos omitidos. Animalia e Fungi aparecem em Opisthokonta; Plantae, no sentido do backbone original do iNaturalist, aparece em Archaeplastida. Isso não torna Plantae sinônimo universal de Archaeplastida, nem afirma parentesco imediato entre os nós visíveis. Não se apresenta esta seleção de linhagens como uma cobertura completa de Eukaryota.

## Cobertura atual

| Domínio | Linhagem | Organismos |
| --- | --- | ---: |
| Bacteria | Bactérias do catálogo | 20 |
| Archaea | Sem representantes cadastrados | 0 |
| Eukaryota | Opisthokonta: Animalia e Fungi | 62 |
| Eukaryota | Archaeplastida: Plantae da fonte | 20 |
| Eukaryota | Amoebozoa: mixomicetos | 10 |
| Eukaryota | SAR: Stramenopiles / algas pardas | 10 |
| Eukaryota | Discoba: sem representantes | 0 |
| **Total** | | **122** |

Monera e Protista são referências históricas, preservadas apenas para proveniência e compatibilidade. Protozoa e Chromista também saem da árvore ativa: os registros antes assim classificados são posicionados a partir dos grupos inferiores documentados. Não se redistribui qualquer organismo apenas porque era chamado de “protista”. Linhagens desconhecidas ou correspondências conflitantes exigem ampliar a política com fontes antes da importação.

Bacteria deixa de ser um reino na árvore ativa. Bactérias, mixomicetos e algas pardas não recebem reinos substitutos inventados. Os demais nomes e ranks inferiores permanecem conforme a classificação original da fonte, inclusive possíveis diferenças de nomenclatura bacteriana ou entre sistemas de classificação. Não foi executada uma atualização automática para GTDB/LPSN, nem afirmada equivalência universal entre os ranks de diferentes fontes.

## Dados, fontes e revisão

- `resources/data/phylogeny.json`: fontes, localizadores, regras explícitas de enquadramento, nós, limitações e hipóteses alternativas. A data registrada é de consulta/síntese, sem simular revisão humana.
- `species.json`: `taxonomy` contém o caminho completo da navegação; `source_taxonomy` conserva cada nó original, com rank, pai e identificador da fonte. `legacy_educational_kingdom` conserva a associação histórica; `domain` e `classification` documentam o enquadramento atual e as divergências.
- `taxonomy.json`: somente os nós da organização atual. IDs de organismos e dos táxons inferiores são preservados; nós novos têm IDs explícitos `domain:` ou `clade:`. Não se geram IDs falsos do iNaturalist para esses nós.
- `resources/compliance/taxonomy_migration_v1.json`: comparação dos 122 IDs, nomes, caminhos originais e metadados integrais das fotografias antes da migração.
- [Auditoria completa](AUDITORIA_FILOGENETICA.json): um registro por organismo, fontes, grupo, estado e hash verificável.

A revisão `taxonomy` continua vinculada **à classificação original da fonte**; seus hashes e evidências não foram renovados. A nova síntese tem o tópico independente `phylogeny`: **122 rascunhos assistidos por IA, sem revisão humana declarada**. Alterações nos caminhos, regras ou evidências mudam seus hashes e invalidam aprovações futuras. As fichas apresentam separadamente o estado da fonte e o do enquadramento, suas referências e limitações.

## Migração e compatibilidade

`python scripts/phylogeny.py --apply` é uma projeção offline idempotente dos dados editáveis. O gerador `build_education.py` incorpora o resultado e aplica a governança existente. As identidades e todos os metadados de foto são comparados com o registro anterior; nenhum arquivo de imagem é modificado. O importador trabalha sobre as linhagens originais e reaplica a projeção, impedindo que sua saída volte a criar a navegação dos cinco reinos. As cotas antigas do importador continuam identificadas como agrupamentos históricos de consulta à API.

O catálogo SQLite usa a migração **v2**, com DDL e carga em uma mesma transação, inserção de pais antes dos filhos e upsert. Em caso de falha ocorre rollback, e o adaptador conserva o catálogo embarcado como fallback. Os registros e nós antigos permanecem fisicamente disponíveis no banco; as raízes de navegação são exclusivamente os domínios. Registros locais não relacionados e substituições anteriores são preservados.

Antes da primeira migração de um banco existente, `catalog_migration_snapshots` conserva uma cópia JSON de todas as linhas originais de táxons e organismos, incluindo os pais anteriores. O snapshot é transacional e não é substituído na reabertura. Um banco com versão futura desconhecida não é rebaixado; o aplicativo usa o fallback embarcado.

O identificador interno do aplicativo, os caminhos dos bancos e o esquema **v3 do histórico de partidas** permanecem estáveis. Não há recálculo de pontos, troca de IDs de organismos nem reinterpretação das respostas antigas. Seletores `edu:` continuam aceitos somente como compatibilidade histórica; não são oferecidos como táxons naturais na interface. Conquistas mantêm suas regras.

Rankings competitivos usam o hash de conteúdo existente, que inclui a nova classificação, as fontes e os estados de revisão. Partidas antigas mantêm a chave original. No Livre, os resultados históricos continuam acessíveis na mesma modalidade.

## Quizzes e validação

Domínio, supergrupo, reino, filo/divisão, classe, ordem, família, gênero e espécie são tratados separadamente. Um rank ausente não é inferido. São necessárias quatro alternativas distintas, revisão adequada e aprovação da fotografia naquele nível. Os novos domínios/supergrupos também exigem revisão de `phylogeny`; nenhuma aprovação fotográfica foi estendida automaticamente.

Nesta etapa, quizzes de domínio e supergrupo estão indisponíveis. Não há quatro reinos científicos elegíveis; as perguntas de reino também ficam indisponíveis, sem reutilizar Monera/Protista como distratores. Os níveis inferiores elegíveis e os modos Livre/Competitivo continuam funcionando. Imagens de apoio bacterianas mantêm os avisos e restrições anteriores.

```powershell
python scripts/phylogeny.py
python scripts/audit_catalog.py --strict
python tests/test_phylogeny.py
ctest --test-dir build --output-on-failure
```

A validação cobre estrutura, correspondência explícita com fontes, ciclos, lacunas, preservação, idempotência, revisão independente, migração, navegação e snapshots de pontuação. Não equivale a revisão científica independente ou liberação jurídica. Permanecem as pendências anteriores de direitos e curadoria em [RELATORIO_CONFORMIDADE.md](RELATORIO_CONFORMIDADE.md). Nenhuma tag, release ou push é feito por esta implementação.
