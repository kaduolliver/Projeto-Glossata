#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>
#include "domain/Hint.h"

struct HistoryEntry {
    QString playerName;
    qint64 scoreUnits = 0;
    int correctAnswers = 0;
    QDateTime playedAt;
};

class GameHistoryRepository {
public:
    virtual ~GameHistoryRepository() = default;
    // Replaces the previous result for this player name with the latest game.
    virtual bool saveResult(const QString &playerName, qint64 scoreUnits, int correctAnswers,
                            const QVector<AnswerRecord> &answers={},const QString &gameId={}) = 0;
    virtual QVector<HistoryEntry> loadTopResults(int limit = 10) const = 0;
};
