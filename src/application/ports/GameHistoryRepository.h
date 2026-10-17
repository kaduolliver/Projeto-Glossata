#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>
#include "domain/Hint.h"
#include "domain/GameConfiguration.h"
#include "domain/PlayerIdentity.h"

struct HistoryEntry {
    QString playerName;
    qint64 scoreUnits = 0;
    int correctAnswers = 0;
    QDateTime playedAt;
    qint64 lastScoreUnits = 0;
    int lastCorrectAnswers = 0;
    QDateTime lastPlayedAt;
    QString playerId;
    QString snapshotName;
    bool legacyIdentity = false;
    bool deletedProfile = false;
};

class GameHistoryRepository {
public:
    virtual ~GameHistoryRepository() = default;
    // Retains the best score and records the latest game separately.
    virtual bool saveResult(const PlayerIdentity &player, qint64 scoreUnits, int correctAnswers,
                            const QVector<AnswerRecord> &answers={},const QString &gameId={}) = 0;
    virtual QVector<HistoryEntry> loadTopResults(int limit = 10) const = 0;
    virtual SessionMetrics playerMetrics(const QString &, const QString &) const { return {}; }
    virtual bool saveConfiguredResult(const PlayerIdentity &player, qint64 units, int correct,
        const QVector<AnswerRecord> &answers, const QString &id, const GameConfiguration &configuration) {
        Q_UNUSED(configuration);
        return saveResult(player, units, correct, answers, id);
    }
    virtual QVector<HistoryEntry> loadRanking(const QString &key, int limit = 10) const {
        return key == "free" ? loadTopResults(limit) : QVector<HistoryEntry>{};
    }
};
