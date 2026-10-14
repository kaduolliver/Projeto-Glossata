#pragma once
#include "domain/Species.h"
#include <QSet>

inline bool approvedForQuiz(const Species &s, const QString &rank)
{
    return !s.name.trimmed().isEmpty() && s.taxonomyApproved && s.photoApproved && s.photo.valid
        && s.photo.approvedRanks.contains(rank) && !s.answerAtRank(rank).trimmed().isEmpty();
}

inline bool canBeDistractor(const Species &candidate, const Species &correct, const QString &rank)
{
    if (candidate.answerAtRank(rank).compare(correct.answerAtRank(rank), Qt::CaseInsensitive) == 0) return false;
    return !candidate.answerAtRank(rank).trimmed().isEmpty();
}

inline QVector<Species> eligibleQuizSpecies(QVector<Species> pool, const QString &rank)
{
    for (int i = static_cast<int>(pool.size()) - 1; i >= 0; --i) {
        const Species &s = pool.at(i);
        if (!approvedForQuiz(s,rank)) pool.removeAt(i);
    }
    QSet<QString> answers;
    for (const auto &s : pool) answers.insert(s.answerAtRank(rank).trimmed().toCaseFolded());
    return answers.size() >= 4 ? pool : QVector<Species>{};
}
