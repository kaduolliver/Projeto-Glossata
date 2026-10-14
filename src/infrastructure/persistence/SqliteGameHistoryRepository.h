#pragma once

#include "application/ports/GameHistoryRepository.h"
#include "application/ports/PlayerProfileRepository.h"

#include <QSqlDatabase>

class SqliteGameHistoryRepository final : public GameHistoryRepository, public PlayerProfileRepository {
public:
    explicit SqliteGameHistoryRepository(const QString &databasePath = QStringLiteral("glossata_history.sqlite"));
    ~SqliteGameHistoryRepository() override;
    bool saveResult(const QString &playerName, qint64 scoreUnits, int correctAnswers,
                    const QVector<AnswerRecord> &answers={},const QString &gameId={}) override;
    QVector<HistoryEntry> loadTopResults(int limit = 10) const override;
    bool saveConfiguredResult(const QString &, qint64, int, const QVector<AnswerRecord> &,
                              const QString &, const GameConfiguration &) override;
    QVector<HistoryEntry> loadRanking(const QString &key, int limit = 10) const override;
    QVector<HistoryEntry> loadPlayerResult(const QString &key, const QString &name) const;
    SessionMetrics playerMetrics(const QString &key, const QString &name) const override;
    QStringList loadPlayerProfiles() const override;
    bool addPlayerProfile(const QString &name) override;
    bool renamePlayerProfile(const QString &currentName, const QString &newName) override;
    bool deletePlayerProfile(const QString &name) override;

private:
    QVector<HistoryEntry> queryResults(const QString &key, int limit, const QString &name) const;
    QString databasePath_;
    QSqlDatabase database_;
};
