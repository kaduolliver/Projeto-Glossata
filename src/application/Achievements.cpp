#include "Achievements.h"
#include <algorithm>

const QVector<AchievementDefinition> &Achievements::definitions()
{
    static const QVector<AchievementDefinition> rows{
        {"first_answer", "Primeiro voo", "Responda sua primeira pergunta.", "Primeiros passos", "answered", "leaf", 1, AchievementLevel::Initial},
        {"first_correct", "Primeira descoberta", "Confirme sua primeira resposta correta.", "Primeiros passos", "correct", "seed", 1, AchievementLevel::Initial},
        {"first_game", "Uma jornada", "Conclua uma partida com respostas registradas.", "Primeiros passos", "games", "flag", 1, AchievementLevel::Initial},
        {"first_competitive", "Novo desafio", "Conclua todas as perguntas de uma partida competitiva.", "Primeiros passos", "competitive", "compass", 1, AchievementLevel::Intermediate},
        {"first_hint", "Uma pista", "Utilize uma dica disponível no quiz.", "Primeiros passos", "hints", "lamp", 1, AchievementLevel::Initial},
        {"correct_5", "Olhar atento", "Acerte cinco perguntas.", "Desempenho", "correct", "eye", 5, AchievementLevel::Initial},
        {"correct_10", "Naturalista iniciante", "Acerte dez perguntas.", "Desempenho", "correct", "sprout", 10, AchievementLevel::Initial},
        {"correct_20", "Conhecedor", "Acerte vinte perguntas.", "Desempenho", "correct", "book", 20, AchievementLevel::Intermediate},
        {"correct_50", "Naturalista", "Acerte cinquenta perguntas.", "Desempenho", "correct", "laurel", 50, AchievementLevel::Intermediate},
        {"correct_100", "Especialista", "Acerte cem perguntas.", "Desempenho", "correct", "medal", 100, AchievementLevel::Advanced},
        {"streak_3", "Em sequência", "Faça três acertos seguidos na mesma partida.", "Desempenho", "streak", "spark", 3, AchievementLevel::Intermediate},
        {"perfect_game", "Precisão em campo", "Conclua uma partida com pelo menos dez respostas e nenhum erro.", "Desempenho", "perfect", "target", 1, AchievementLevel::Advanced},
        {"no_hint_game", "Olhar independente", "Conclua uma partida com pelo menos dez respostas, sem revelar nenhuma dica.", "Desempenho", "no_hint", "wing", 1, AchievementLevel::Advanced},
        {"group_10", "Uma linhagem", "Acerte dez perguntas sobre organismos do mesmo reino, supergrupo ou domínio do catálogo.", "Conhecimento taxonômico", "group_correct", "branch", 10, AchievementLevel::Intermediate},
        {"group_50", "Raízes do conhecimento", "Acerte cinquenta perguntas sobre organismos do mesmo grupo científico do catálogo.", "Conhecimento taxonômico", "group_correct", "tree", 50, AchievementLevel::Advanced},
        {"groups_3", "Diversidade da vida", "Acerte perguntas sobre organismos de três grupos científicos distintos.", "Conhecimento taxonômico", "groups", "network", 3, AchievementLevel::Intermediate},
        {"species_10", "Dez descobertas", "Acerte perguntas envolvendo dez organismos distintos.", "Conhecimento taxonômico", "species", "dna", 10, AchievementLevel::Intermediate},
        {"cycle", "Um ciclo completo", "Responda tantas perguntas quanto o total de organismos selecionados em uma partida.", "Conhecimento taxonômico", "cycle", "cycle", 1, AchievementLevel::Intermediate},
        {"visit_1", "Primeira ficha", "Consulte uma ficha de organismo na enciclopédia.", "Exploração", "visits", "page", 1, AchievementLevel::Initial},
        {"visit_10", "Explorador de espécies", "Consulte fichas de dez organismos diferentes.", "Exploração", "visits", "binoculars", 10, AchievementLevel::Intermediate},
        {"visit_25", "Caderno de campo", "Consulte fichas de vinte e cinco organismos diferentes.", "Exploração", "visits", "notebook", 25, AchievementLevel::Advanced},
        {"visit_groups_3", "Além de uma linhagem", "Consulte organismos de três grupos científicos distintos.", "Exploração", "visit_groups", "globe", 3, AchievementLevel::Intermediate},
        {"answered_10", "Explorador", "Responda dez perguntas.", "Dedicação", "answered", "butterfly", 10, AchievementLevel::Initial},
        {"answered_100", "Prática constante", "Responda cem perguntas.", "Dedicação", "answered", "trail", 100, AchievementLevel::Intermediate},
        {"answered_250", "Longa jornada", "Responda duzentas e cinquenta perguntas.", "Dedicação", "answered", "mountain", 250, AchievementLevel::Advanced},
        {"games_10", "Dez jornadas", "Conclua dez partidas com respostas registradas.", "Dedicação", "games", "camp", 10, AchievementLevel::Intermediate},
        {"games_25", "Dedicação à ciência", "Conclua vinte e cinco partidas com respostas registradas.", "Dedicação", "games", "telescope", 25, AchievementLevel::Advanced},
        {"competitive_perfect", "Desafio perfeito", "Conclua uma partida competitiva sem erros.", "Desafios especiais", "competitive_perfect", "crown", 1, AchievementLevel::Special},
        {"competitive_no_hint", "Autonomia competitiva", "Conclua uma partida competitiva sem revelar nenhuma dica.", "Desafios especiais", "competitive_no_hint", "shield", 1, AchievementLevel::Special},
        {"correct_500", "Horizonte do conhecimento", "Acerte quinhentas perguntas.", "Desafios especiais", "correct", "star", 500, AchievementLevel::Special}
    };
    return rows;
}

QMap<QString, int> Achievements::evaluate(const AchievementEvidence &evidence, const QVector<Species> &catalog)
{
    QMap<QString, int> metrics, groups;
    QMap<int, Species> species;
    for (const auto &s : catalog) species.insert(s.id, s);
    QSet<int> correctSpecies;
    QSet<QString> visitedGroups, seenRuns;
    for (const auto &run : evidence.runs) {
        if (run.id.isEmpty() || seenRuns.contains(run.id)) continue;
        seenRuns.insert(run.id);
        int streak=0, correct=0, hints=run.pendingHints != 0;
        QSet<int> questions;
        for (const auto &a : run.answers) {
            if (questions.contains(a.questionNumber) || a.questionNumber<=0 || !validAnswerScoring(a,a.rulesVersion)) continue;
            questions.insert(a.questionNumber);
            ++metrics["answered"];
            hints += a.revealedHints != 0;
            streak=a.correct ? streak+1 : 0;
            metrics["streak"]=std::max(metrics["streak"],streak);
            if (!a.correct) continue;
            ++correct; ++metrics["correct"];
            // Never attach an old organism to a reused numeric ID/name.
            const auto organism=species.constFind(a.speciesId);
            if (a.speciesId<=0 || organism==species.cend() || a.scientificName.isEmpty()
                || organism->name.isEmpty() || organism->name!=a.scientificName) continue;
            correctSpecies.insert(a.speciesId);
            const auto &s=organism.value();
            if (!s.taxonomyApproved) continue;
            const auto group=s.displayGroupId();
            if (group!="unclassified") ++groups[group];
        }
        metrics["hints"] += hints;
        if (run.cycleSize>0 && questions.size()>=run.cycleSize) ++metrics["cycle"];
        if (!run.complete || questions.isEmpty() || questions.size()!=run.answers.size()) continue;
        ++metrics["games"];
        if (run.competitive) ++metrics["competitive"];
        const bool perfect=correct==questions.size();
        const bool noHint=run.hintsKnown && hints==0;
        if (questions.size()>=10) {
            if (perfect) ++metrics["perfect"];
            if (noHint) ++metrics["no_hint"];
        }
        if (run.competitive && perfect) ++metrics["competitive_perfect"];
        if (run.competitive && noHint) ++metrics["competitive_no_hint"];
    }
    metrics["species"]=correctSpecies.size();
    metrics["groups"]=groups.size();
    for (int count : groups) metrics["group_correct"]=std::max(metrics["group_correct"],count);
    metrics["visits"]=evidence.visits.size();
    for (auto visit=evidence.visits.cbegin(); visit!=evidence.visits.cend(); ++visit) {
        const auto s=species.value(visit.key());
        if (s.name==visit.value() && s.taxonomyApproved && s.displayGroupId()!="unclassified") visitedGroups.insert(s.displayGroupId());
    }
    metrics["visit_groups"]=visitedGroups.size();
    QMap<QString,int> result;
    for (const auto &d : definitions()) result[d.id]=std::min(d.target,metrics.value(d.metric));
    return result;
}

QVariantMap Achievements::present(const AchievementDefinition &d, int progress, const QString &date, bool retrospective)
{
    QString level, levelName;
    switch (d.level) {
    case AchievementLevel::Initial: level="initial"; levelName="I · Inicial · Bronze"; break;
    case AchievementLevel::Intermediate: level="intermediate"; levelName="II · Intermediário · Prata"; break;
    case AchievementLevel::Advanced: level="advanced"; levelName="III · Avançado · Ouro"; break;
    case AchievementLevel::Special: level="special"; levelName="IV · Especial · Platina"; break;
    }
    return {{"id",d.id},{"title",d.title},{"description",d.description},{"criterion",d.description},
        {"category",d.category},{"icon",d.icon},{"badge",d.icon},{"visualLevel",level},{"visualLevelName",levelName},
        {"progress",progress},{"target",d.target},{"unlocked",!date.isEmpty()},
        {"unlockedAt",date},{"retrospective",retrospective}};
}
