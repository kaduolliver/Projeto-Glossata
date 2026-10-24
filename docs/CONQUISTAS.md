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

A avaliação prioriza `game_runs` com `answer_records` detalhados válidos, sequenciais e compatíveis com contagem, pontos, acertos e versão de regras do resultado. Somente então o `game_id` é reservado para deduplicação. Se o resultado for agregado, incompleto ou inconsistente, o checkpoint correspondente ainda pode fornecer respostas confirmadas/dicas, com digest, regras e UUID validados pelo mecanismo existente. Não se somam as duas fontes. Um checkpoint abandonado, ativo, em finalização ou usado como fallback de resultado insuficiente não prova conclusão consolidada. Sem nenhuma fonte válida, não há novo progresso; o progresso persistido anteriormente é preservado. IDs de pergunta repetidos/entradas inválidas não aumentam contagem.

No modo Livre, encerrar uma partida com ao menos uma resposta é a conclusão definida pelo comportamento existente. No Competitivo, todas as perguntas configuradas devem estar respondidas e o resultado salvo. “Precisão em campo” e “Olhar independente” exigem adicionalmente dez respostas. Uma dica em pergunta não respondida permanece no checkpoint e impede “sem dicas”; não se torna acerto. Regras de pontos, ranking, dicas e recuperação não mudam.

Fichas na enciclopédia registram visita após a view ser criada/atualizada no contexto Explorar. A ficha pós-resposta do quiz também registra consulta. Navegar por grupos, pré-carregar dados ou simplesmente chamar `exploreTaxon` não registra visita. O backend verifica organismo/contexto; a chave UUID/ID do organismo deduplica reaberturas. Guardamos nome científico para não associar uma visita antiga a outro organismo em eventual reutilização de ID.

Progresso é calculado, limitado à meta e conservado pelo máximo persistido. A atualização das 30 linhas e seus desbloqueios ocorre numa transação; sinais são emitidos **após commit**, somente para novas transições ao estado desbloqueado. O payload inclui UUID, ID, título, descrição, categoria, ícone/badge, progresso, meta e data. Recarregar a view, retomar a sessão ou reavaliar registros já reconhecidos não repete sinais. Não há fila de notificações nem toast; isso pertence à 2.3.4. Se houver interrupção entre evento de jogo e avaliação, o checkpoint permite reconhecer o fato na reabertura, silenciosamente; não se promete entrega de notificações após crash.

## Compatibilidade e reconhecimento retroativo

Respostas detalhadas validadas podem reconhecer quantidade, acertos e sequência por sessão. Para `species_10`, o organismo deve existir no catálogo instalado com ID positivo e nome científico não vazio e idêntico ao registro. IDs inexistentes/reutilizados, nomes divergentes ou ausentes não entram na contagem de organismos distintos. A identidade de um organismo não depende da aprovação da sua classificação; as metas de grupos exigem adicionalmente `taxonomyApproved` e um grupo científico disponível. Uma classificação atual não prova a classificação histórica. Progresso já consolidado é conservado, inclusive após a correção: não se revogam desbloqueios nem se reescrevem respostas/pontos.

Partidas completas com respostas detalhadas podem reconhecer conclusão/perfeição. “Sem dicas” em Livre exige checkpoint finalizado que também prove ausência de dica na pergunta pendente; histórico sem essa evidência não concede o feito. Competitivo completo não possui pergunta pendente. “Um ciclo completo” exige o tamanho original do conjunto no checkpoint. Totais antigos sem respostas detalhadas não provam eventos individuais e não são somados a estes, evitando duplicação.

Consultas começam nesta implementação. Não se infere visita pelo quiz, acerto, ranking ou existência de ficha no catálogo. Não há conquistas por dias: não existe registro por evento/dia suficiente para garantir continuidade. A data retroativa é a **data do reconhecimento**, marcada por `retrospective`, nunca uma data original inventada. A reavaliação na inicialização/transição local é silenciosa e idempotente. Dados pessoais reais não são usados em testes/capturas.

## Extensão e limites

Para acrescentar uma conquista: crie uma definição com ID permanente e `AchievementLevel` explícito em `Achievements::definitions`, reutilize uma métrica ou acrescente um fato verificável ao avaliador/porta; forneça um glifo distinto no mapa de desenhos; cubra limiar, duplicação, falhas e reabertura. Não recicle IDs nem altere metas retroativamente sem decisão/migração. UI, filtros, totais e categorias são derivados das definições. Não é necessário editar seletores independentes.

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

## Níveis visuais de complexidade

Cada definição recebe um `AchievementLevel` explícito, projetado como `visualLevel`/`visualLevelName`. O nível é estático, independe do bloqueio e não altera a métrica ou a meta. Não mede dificuldade das perguntas nem raridade estatística. A classificação combina volume, diversidade e condições dentro de uma mesma partida; é uma decisão de apresentação, sem alegar calibração empírica.

Bronze usa moldura simples; Prata acrescenta contorno e pequenos losangos; Ouro acrescenta louros e três marcas; Platina usa contorno facetado e cristais geométricos com acabamento luminoso estático. Todas mantêm a família hexagonal e os 30 símbolos vetoriais anteriores. Texto, geometria e nomes acessíveis complementam a cor. SQLite permanece no esquema 8: metadados de apresentação não exigem migração.

| ID | Conquista | Nível visual | Justificativa pelo requisito |
| --- | --- | --- | --- |
| `first_answer` | Primeiro voo | I · Inicial · Bronze | Primeira resposta, sem condição de acerto. |
| `first_correct` | Primeira descoberta | I · Inicial · Bronze | Primeiro acerto confirmado. |
| `first_game` | Uma jornada | I · Inicial · Bronze | Primeira conclusão livre, sem desempenho exigido. |
| `first_competitive` | Novo desafio | II · Intermediário · Prata | Conclusão integral de um modo com regras fixas. |
| `first_hint` | Uma pista | I · Inicial · Bronze | Primeira interação com dicas. |
| `correct_5` | Olhar atento | I · Inicial · Bronze | Marco inicial de acertos. |
| `correct_10` | Naturalista iniciante | I · Inicial · Bronze | Marco inicial de aprendizagem. |
| `correct_20` | Conhecedor | II · Intermediário · Prata | Acertos acumulados de maior extensão. |
| `correct_50` | Naturalista | II · Intermediário · Prata | Repetição consistente de acertos. |
| `correct_100` | Especialista | III · Avançado · Ouro | Marco amplo de conhecimento. |
| `streak_3` | Em sequência | II · Intermediário · Prata | Sequência de acertos na mesma partida. |
| `perfect_game` | Precisão em campo | III · Avançado · Ouro | Precisão integral em pelo menos dez respostas da mesma partida. |
| `no_hint_game` | Olhar independente | III · Avançado · Ouro | Autonomia durante uma partida de pelo menos dez respostas. |
| `group_10` | Uma linhagem | II · Intermediário · Prata | Concentração de acertos em um grupo científico. |
| `group_50` | Raízes do conhecimento | III · Avançado · Ouro | Conhecimento prolongado de um mesmo grupo. |
| `groups_3` | Diversidade da vida | II · Intermediário · Prata | Diversidade científica em três grupos. |
| `species_10` | Dez descobertas | II · Intermediário · Prata | Reconhecimento de dez organismos distintos. |
| `cycle` | Um ciclo completo | II · Intermediário · Prata | Percurso de todo o conjunto selecionado numa sessão. |
| `visit_1` | Primeira ficha | I · Inicial · Bronze | Primeira consulta científica. |
| `visit_10` | Explorador de espécies | II · Intermediário · Prata | Exploração de dez fichas distintas. |
| `visit_25` | Caderno de campo | III · Avançado · Ouro | Pesquisa ampliada em vinte e cinco organismos. |
| `visit_groups_3` | Além de uma linhagem | II · Intermediário · Prata | Exploração abrangendo três grupos científicos. |
| `answered_10` | Explorador | I · Inicial · Bronze | Participação inicial, independente de precisão. |
| `answered_100` | Prática constante | II · Intermediário · Prata | Prática acumulada regular. |
| `answered_250` | Longa jornada | III · Avançado · Ouro | Prática acumulada extensa. |
| `games_10` | Dez jornadas | II · Intermediário · Prata | Continuidade por várias conclusões. |
| `games_25` | Dedicação à ciência | III · Avançado · Ouro | Continuidade ampla por vinte e cinco conclusões. |
| `competitive_perfect` | Desafio perfeito | IV · Especial · Platina | Precisão integral no conjunto competitivo. |
| `competitive_no_hint` | Autonomia competitiva | IV · Especial · Platina | Autonomia integral no conjunto competitivo. |
| `correct_500` | Horizonte do conhecimento | IV · Especial · Platina | Marco mais elevado de acertos da coleção. |

[Validação e capturas das quatro classes](interface/VERSOES/2.3.3/refinamentos/README.md). A versão 2.1.0 ainda não lançada é completada na mesma PR; catálogo 2.4.0 preservado.

## Filtro por nível e acabamento final da Platina

As opções “Todos os níveis”, “Inicial — Bronze”, “Intermediário — Prata”, “Avançado — Ouro” e “Especial — Platina” usam os valores `visualLevel` existentes. O predicado combina estado **e** categoria **e** nível; o contador de seleção acompanha o resultado e o resumo global continua derivado das 30 definições. Uma combinação vazia mantém a mensagem de seleção vazia.

`achievementsPresentation` guarda `state`, `category` e `level` durante a execução; o valor `all` representa ausência de restrição de nível. Não se grava preferência no SQLite. O painel “Filtrar conquistas” usa Estado segmentado (`UiButton`) e menus de Categoria/Nível (`TaxonComboBox`), com rótulos, foco visível e seleção ativa. Os grupos ficam lado a lado quando cabem, em duas linhas ou empilhados conforme a largura útil. A linha inferior mostra a contagem efetiva e “Limpar filtros” somente quando necessário; a limpeza restaura os três padrões de uma vez e o resumo/progresso global permanece independente. Menus aceitam teclado/Enter/Escape e são posicionados na camada de popup, com margens para a janela. Transições respeitam movimento reduzido. [Capturas comparativas e testes](interface/VERSOES/2.3.3/barra-filtros/README.md). A classificação e os requisitos não são alterados.

A Platina final conserva o símbolo central e ganha uma silhueta com ombros escalonados, cristais no topo e placas laterais facetadas. Camadas vetoriais de sombra/reflexo branco-azulado dão profundidade estática e distinguem sua geometria da Prata, inclusive bloqueada. Bronze, Prata e Ouro mantêm suas geometrias/paletas anteriores. Não há dependência gráfica nova ou animação contínua. [Capturas e verificações finais](interface/VERSOES/2.3.3/filtros-nivel-platina/README.md).
