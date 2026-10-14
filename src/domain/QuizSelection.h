#pragma once
#include "domain/Species.h"
#include <QSet>

inline bool canBeDistractor(const Species &candidate, const Species &correct, const QString &rank)
{
    if (candidate.answerAtRank(rank).compare(correct.answerAtRank(rank), Qt::CaseInsensitive) == 0) return false;
    // A textual taxonomic clue must identify a single option, even without a photo.
    if (rank == "species" && correct.source == "inaturalist" && !correct.visualQuiz
        && candidate.name.section(' ', 0, 0).compare(correct.name.section(' ', 0, 0), Qt::CaseInsensitive) == 0) return false;
    return !candidate.answerAtRank(rank).trimmed().isEmpty();
}

inline QVector<Species> eligibleQuizSpecies(QVector<Species> pool, const QString &rank)
{
    for (int i = static_cast<int>(pool.size()) - 1; i >= 0; --i) {
        const Species &s = pool.at(i);
        if (s.name.trimmed().isEmpty() || s.fact.trimmed().isEmpty() || s.hint.trimmed().isEmpty()
            || s.answerAtRank(rank).trimmed().isEmpty()) pool.removeAt(i);
    }
    bool changed;
    do {
        QVector<Species> valid;
        for (const Species &s : pool) {
            QSet<QString> alternatives;
            for (const Species &candidate : pool)
                if (canBeDistractor(candidate, s, rank)) alternatives.insert(candidate.answerAtRank(rank).toCaseFolded());
            if (alternatives.size() >= 3) valid.append(s);
        }
        changed = valid.size() != pool.size();
        pool = valid;
    } while (changed && !pool.isEmpty());
    return pool;
}
