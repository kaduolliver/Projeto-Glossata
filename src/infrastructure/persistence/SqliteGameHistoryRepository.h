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
    QStringList loadPlayerProfiles() const override;
    bool addPlayerProfile(const QString &name) override;
    bool renamePlayerProfile(const QString &currentName, const QString &newName) override;
    bool deletePlayerProfile(const QString &name) override;

private:
    QString databasePath_;
    QSqlDatabase database_;
};
