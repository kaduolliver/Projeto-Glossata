#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

struct HistoryEntry {
    QString playerName;
    int score = 0;
    int correctAnswers = 0;
    QDateTime playedAt;
};

class GameHistoryRepository {
public:
    virtual ~GameHistoryRepository() = default;
    // Replaces the previous result for this player name with the latest game.
    virtual bool saveResult(const QString &playerName, int score, int correctAnswers) = 0;
    virtual QVector<HistoryEntry> loadTopResults(int limit = 10) const = 0;
};
