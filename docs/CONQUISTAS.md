# Conquistas — Subetapa 2.3.3

Aplicativo 2.1.0, catálogo 2.4.0 preservado, sem lançamento. Implementação offline sobre a identidade local única da PR #37. [Interface e evidências](interface/VERSOES/2.3.3/README.md).

## Diagnóstico antes da alteração

`GameController::achievements()` calculava seis desafios efêmeros, com contadores da partida e sem IDs/desbloqueios persistidos. `AchievementsView` apresentava seis retângulos não interativos. Não havia histórico de consultas. Preservamos os seis nomes/critérios existentes, agora com IDs estáveis e progresso persistente; “Um ciclo completo” continua sendo o número de respostas necessário para percorrer o conjunto selecionado, não uma afirmação de identificação correta de todos os organismos.

`GameService` confirma respostas e dicas por checkpoint antes de aceitar a ação; resultados são finalizados por `saveConfiguredResult`. SQLite 7 já tinha `game_runs`, `answer_records`, `session_checkpoints`, nomes imutáveis de partida e vínculo local por UUID. `SessionSnapshot` valida estrutura, transições, regras e integridade. A classificação real vem de `Species.taxonomy`; `displayGroupId()` usa reino, supergrupo ou domínio disponível, sem criar reinos artificiais. `Theme`, `UiButton`, `UiIcon`, rolagem/foco do shell e diálogos Qt Quick são reutilizados.

## Separação de responsabilidades

- `application/Achievements`: catálogo de definições e avaliador puro. Critérios usam fatos persistidos e validados; nenhuma decisão de desbloqueio pertence ao QML.
- `persistence/AchievementStorage`: migração, leitura por proprietário, deduplicação entre resultados e checkpoints, visitas e progresso monotônico. Usa a conexão SQLite existente.
- `GameController`: atualiza a projeção depois de ações confirmadas; fornece `achievements`, `achievementsError`, `achievementsChanged` e `achievementUnlocked(QVariantMap)`.
- `AchievementsView`: resumo, filtros, cartões e detalhes. `AchievementBadge`/`AchievementBadgePaths.js` desenham 30 glifos vetoriais com `QtQuick.Shapes`, sem plugin SVG, rede ou imagens de terceiros.

## Persistência e eventos

Migração **7 → 8**, transacional e aditiva: `achievement_progress(player_id, achievement_id, progress, unlocked_at, retrospective)` e `species_visits(player_id, species_id, scientific_name, visited_at)`. Chaves primárias compostas e referências a `player_profiles.id`; gatilhos preservam identidade, visitas e progresso/desbloqueios consolidados. Esquema incompleto é recusado, sem reconstruir sobre dados existentes. Migrações mais antigas continuam pelo fluxo existente; backup/restore aceita esquema 8 e copia os dados integralmente.

Somente o UUID de `local_user_binding` pode registrar consultas ou atualizar conquistas. Renomear não muda esse vínculo. Resultados, checkpoints e perfis anteriores permanecem com os proprietários originais. Não há login, seleção permanente de perfis, servidor ou vinculação remota automática.

A avaliação lê `game_runs` e seus `answer_records` válidos; um `game_id` já consolidado não é contado novamente pelo checkpoint. Checkpoints válidos ainda não consolidados contribuem somente com respostas confirmadas/dicas; inclusive respostas de uma sessão abandonada permanecem fatos reais. Partidas abandonadas ou em finalização pendente não contam como concluídas. IDs de pergunta repetidos/entradas inválidas não aumentam contagem; o serviço e a persistência continuam sendo a primeira validação de eventos.

No modo Livre, encerrar uma partida com ao menos uma resposta é a conclusão definida pelo comportamento existente. No Competitivo, todas as perguntas configuradas devem estar respondidas e o resultado salvo. “Precisão em campo” e “Olhar independente” exigem adicionalmente dez respostas. Uma dica em pergunta não respondida permanece no checkpoint e impede “sem dicas”; não se torna acerto. Regras de pontos, ranking, dicas e recuperação não mudam.

Fichas na enciclopédia registram visita após a view ser criada/atualizada no contexto Explorar. A ficha pós-resposta do quiz também registra consulta. Navegar por grupos, pré-carregar dados ou simplesmente chamar `exploreTaxon` não registra visita. O backend verifica organismo/contexto; a chave UUID/ID do organismo deduplica reaberturas. Guardamos nome científico para não associar uma visita antiga a outro organismo em eventual reutilização de ID.

Progresso é calculado, limitado à meta e conservado pelo máximo persistido. A atualização das 30 linhas e seus desbloqueios ocorre numa transação; sinais são emitidos **após commit**, somente para novas transições ao estado desbloqueado. O payload inclui UUID, ID, título, descrição, categoria, ícone/badge, progresso, meta e data. Recarregar a view, retomar a sessão ou reavaliar registros já reconhecidos não repete sinais. Não há fila de notificações nem toast; isso pertence à 2.3.4. Se houver interrupção entre evento de jogo e avaliação, o checkpoint permite reconhecer o fato na reabertura, silenciosamente; não se promete entrega de notificações após crash.

## Compatibilidade e reconhecimento retroativo

Respostas detalhadas validadas podem reconhecer quantidade, acertos, sequência por sessão, organismos e diversidade científica compatível com IDs/nomes do catálogo instalado. Uma classificação atual não é prova da classificação histórica; nomes divergentes ou taxonomia indisponível são excluídos das metas de grupo. Progresso já consolidado é conservado.

Partidas completas com respostas detalhadas podem reconhecer conclusão/perfeição. “Sem dicas” em Livre exige checkpoint finalizado que também prove ausência de dica na pergunta pendente; histórico sem essa evidência não concede o feito. Competitivo completo não possui pergunta pendente. “Um ciclo completo” exige o tamanho original do conjunto no checkpoint. Totais antigos sem respostas detalhadas não provam eventos individuais e não são somados a estes, evitando duplicação.

Consultas começam nesta implementação. Não se infere visita pelo quiz, acerto, ranking ou existência de ficha no catálogo. Não há conquistas por dias: não existe registro por evento/dia suficiente para garantir continuidade. A data retroativa é a **data do reconhecimento**, marcada por `retrospective`, nunca uma data original inventada. A reavaliação na inicialização/transição local é silenciosa e idempotente. Dados pessoais reais não são usados em testes/capturas.

## Extensão e limites

Para acrescentar uma conquista: crie uma definição com ID permanente em `Achievements::definitions`, reutilize uma métrica ou acrescente um fato verificável ao avaliador/porta; forneça um glifo distinto no mapa de desenhos; cubra limiar, duplicação, falhas e reabertura. Não recicle IDs nem altere metas retroativamente sem decisão/migração. UI, filtros, totais e categorias são derivados das definições. Não é necessário editar seletores independentes.

A avaliação relê o histórico local nas ações relevantes. É uma solução simples para o volume atual; históricos muito extensos podem justificar uma projeção incremental futura com os mesmos IDs e transações. Não há certificação antitrapaça: um SQLite local não prova autoria autenticada. Não há revisão científica nova, transferência de dados entre identidades ou liberação de distribuição.

Os novos módulos são explicitamente pendentes em `ESCOPO_LICENCIAMENTO.json`. Desenhos/QML/JS novos foram produzidos nesta tarefa com assistência de IA, sem referência gráfica externa importada; não há autoria humana, cessão ou licença específica presumida. A inclusão no escopo de licença depende de decisão humana separada, sem alterar decisões históricas. Protótipos SVG locais preservados não integram o aplicativo. Testes técnicos não resolvem essas pendências.

## Catálogo de conquistas

As metas abaixo são acumuladas por UUID, salvo quando o próprio critério exige uma mesma partida.

| ID | Nome | Categoria | Critério |
| --- | --- | --- | --- |
| `first_answer` | Primeiro voo | Primeiros passos | Responda sua primeira pergunta. |
| `first_correct` | Primeira descoberta | Primeiros passos | Confirme sua primeira resposta correta. |
| `first_game` | Uma jornada | Primeiros passos | Conclua uma partida com respostas registradas. |
| `first_competitive` | Novo desafio | Primeiros passos | Conclua todas as perguntas de uma partida competitiva. |
| `first_hint` | Uma pista | Primeiros passos | Utilize uma dica disponível no quiz. |
| `correct_5` | Olhar atento | Desempenho | Acerte cinco perguntas. |
| `correct_10` | Naturalista iniciante | Desempenho | Acerte dez perguntas. |
| `correct_20` | Conhecedor | Desempenho | Acerte vinte perguntas. |
| `correct_50` | Naturalista | Desempenho | Acerte cinquenta perguntas. |
| `correct_100` | Especialista | Desempenho | Acerte cem perguntas. |
| `streak_3` | Em sequência | Desempenho | Faça três acertos seguidos na mesma partida. |
| `perfect_game` | Precisão em campo | Desempenho | Conclua uma partida com pelo menos dez respostas e nenhum erro. |
| `no_hint_game` | Olhar independente | Desempenho | Conclua uma partida com pelo menos dez respostas, sem revelar nenhuma dica. |
| `group_10` | Uma linhagem | Conhecimento taxonômico | Acerte dez perguntas sobre organismos do mesmo reino, supergrupo ou domínio do catálogo. |
| `group_50` | Raízes do conhecimento | Conhecimento taxonômico | Acerte cinquenta perguntas sobre organismos do mesmo grupo científico do catálogo. |
| `groups_3` | Diversidade da vida | Conhecimento taxonômico | Acerte perguntas sobre organismos de três grupos científicos distintos. |
| `species_10` | Dez descobertas | Conhecimento taxonômico | Acerte perguntas envolvendo dez organismos distintos. |
| `cycle` | Um ciclo completo | Conhecimento taxonômico | Responda tantas perguntas quanto o total de organismos selecionados em uma partida. |
| `visit_1` | Primeira ficha | Exploração | Consulte uma ficha de organismo na enciclopédia. |
| `visit_10` | Explorador de espécies | Exploração | Consulte fichas de dez organismos diferentes. |
| `visit_25` | Caderno de campo | Exploração | Consulte fichas de vinte e cinco organismos diferentes. |
| `visit_groups_3` | Além de uma linhagem | Exploração | Consulte organismos de três grupos científicos distintos. |
| `answered_10` | Explorador | Dedicação | Responda dez perguntas. |
| `answered_100` | Prática constante | Dedicação | Responda cem perguntas. |
| `answered_250` | Longa jornada | Dedicação | Responda duzentas e cinquenta perguntas. |
| `games_10` | Dez jornadas | Dedicação | Conclua dez partidas com respostas registradas. |
| `games_25` | Dedicação à ciência | Dedicação | Conclua vinte e cinco partidas com respostas registradas. |
| `competitive_perfect` | Desafio perfeito | Desafios especiais | Conclua uma partida competitiva sem erros. |
| `competitive_no_hint` | Autonomia competitiva | Desafios especiais | Conclua uma partida competitiva sem revelar nenhuma dica. |
| `correct_500` | Horizonte do conhecimento | Desafios especiais | Acerte quinhentas perguntas. |
