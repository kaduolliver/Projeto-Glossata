#pragma once

#include "application/EducationalContent.h"
#include "domain/GameConfiguration.h"
#include "domain/QuizSelection.h"
#include "domain/Question.h"
#include <algorithm>
#include <random>
#include <QMap>

// Works on the already filtered, reviewed pool. Taxonomic proximity and
// documented visible traits are separate signals, never synonyms.
class QuestionGenerator {
public:
    explicit QuestionGenerator(const EducationalContent &content) : content_(content) {}

    static int taxonomicProximity(const Species &a, const Species &b, const QString &rank) {
        const QStringList ranks{"kingdom", "phylum", "class", "order", "family", "genus", "species"};
        const int asked = ranks.indexOf(rank);
        int shared = -1;
        for (const auto &node : a.taxonomy) {
            const int level = ranks.indexOf(node.rank);
            if (level >= 0 && level < asked && b.belongsTo(node.id)) shared = std::max(shared, level);
        }
        return shared;
    }

    Question generate(const Species &correct, const QVector<Species> &pool, const GameConfiguration &config,
                      std::mt19937 &random) const {
        struct Candidate { QString answer; int proximity; int similarity; };
        const auto belongs = [&config](const Species &s) {
            return config.group.isEmpty() || s.belongsTo(config.group) || s.family == config.group;
        };
        if (!approvedForQuiz(correct,config.rank) || !belongs(correct)) return {};
        QMap<QString, Candidate> byAnswer;
        for (const auto &s : pool) {
            if (!approvedForQuiz(s,config.rank) || !belongs(s)) continue;
            if (!canBeDistractor(s, correct, config.rank)) continue;
            int similarity = 0;
            for (const auto &trait : correct.morphologyTraits) similarity += s.morphologyTraits.contains(trait);
            const int proximity = taxonomicProximity(correct, s, config.rank);
            // Close questions need documented proximity or reviewed shared traits.
            const int threshold = config.rank == "species" || config.rank == "genus" ? 4
                : config.rank == "family" ? 3 : config.rank == "order" ? 2 : 1;
            if (config.difficulty == QuestionDifficulty::Close && proximity < threshold && similarity == 0) continue;
            const QString answer = s.answerAtRank(config.rank).trimmed();
            const QString key = answer.toCaseFolded();
            if (!byAnswer.contains(key) || byAnswer[key].proximity < proximity)
                byAnswer[key] = {answer, proximity, similarity};
        }
        QVector<Candidate> candidates;
        for (const auto &s : byAnswer) candidates.append(s);
        if (candidates.size() < 3) return {};
        std::shuffle(candidates.begin(), candidates.end(), random);
        std::stable_sort(candidates.begin(), candidates.end(), [&config](const Candidate &a, const Candidate &b) {
            if (config.difficulty == QuestionDifficulty::Broad) return a.proximity < b.proximity;
            if (a.proximity != b.proximity) return a.proximity > b.proximity;
            return a.similarity > b.similarity;
        });
        // Balanced compares two nearby choices and one broader choice. This
        // leaves room for a safe context shared by at least three alternatives.
        QStringList options{correct.answerAtRank(config.rank)};
        if (config.difficulty == QuestionDifficulty::Balanced && candidates.size() > 3) {
            for (int index : {0, 1, int(candidates.size() - 1)}) options.append(candidates[index].answer);
        } else for (int i = 0; i < 3; ++i) options.append(candidates[i].answer);
        std::shuffle(options.begin(), options.end(), random);
        const auto hints = content_.questionHints(correct, config.rank, options, pool,config.mode==GameMode::Competitive);
        const bool supporting = correct.photo.role == "supporting";
        return {supporting ? QStringLiteral("Identifique pelas pistas: %1").arg(taxonomicRankLabel(config.rank))
                          : QStringLiteral("Identifique: %1").arg(taxonomicRankLabel(config.rank)),
                supporting ? QStringLiteral("A fotografia é apenas apoio. Use as informações documentadas.")
                           : QStringLiteral("Compare a forma, as estruturas e os padrões visíveis na fotografia."),
                hints[2].text, correct.answerAtRank(config.rank), options, correct, config.rank, !supporting,
                correct.photo, hints[2].sourceUrl, hints[2].sourceTitle, hints};
    }
private:
    const EducationalContent &content_;
};
