#pragma once

#include "application/ports/GameHistoryRepository.h"
#include "application/ports/PlayerProfileRepository.h"
#include "application/ports/SessionCheckpointRepository.h"

#include <QSqlDatabase>

class SqliteGameHistoryRepository final : public GameHistoryRepository, public PlayerProfileRepository, public SessionCheckpointRepository {
public:
    explicit SqliteGameHistoryRepository(const QString &databasePath = QStringLiteral("glossata_history.sqlite"));
    ~SqliteGameHistoryRepository() override;
    bool saveResult(const PlayerIdentity &player, qint64 scoreUnits, int correctAnswers,
                    const QVector<AnswerRecord> &answers={},const QString &gameId={}) override;
    QVector<HistoryEntry> loadTopResults(int limit = 10) const override;
    bool saveConfiguredResult(const PlayerIdentity &, qint64, int, const QVector<AnswerRecord> &,
                              const QString &, const GameConfiguration &) override;
    QVector<HistoryEntry> loadRanking(const QString &key, int limit = 10) const override;
    QVector<HistoryEntry> loadPlayerResult(const QString &key, const QString &playerId) const;
    QVector<AnswerRecord> loadAnswers(const QString &gameId) const;
    SessionMetrics playerMetrics(const QString &key, const QString &playerId) const override;
    QVector<PlayerProfile> profiles() const override;
    PlayerProfile player(const QString &id) const override;
    PlayerProfile createPlayer(const QString &name) override;
    bool renamePlayer(const QString &id, const QString &name) override;
    bool deletePlayer(const QString &id) override;
    int unresolvedLegacyPlayers() const;
    // Installation binding is separate from historical identities and never reassigns results.
    bool initializeLocalUser();
    bool chooseLocalUser(const QString &id);
    PlayerProfile localUser() const;
    bool localUserChoiceRequired() const;

    bool writeCheckpoint(const QJsonObject &, qint64 expectedRevision, qint64 &revision) override;
    QVector<SessionCheckpoint> recoverableSessions() const override;
    SessionCheckpoint checkpoint(const QString &id) const override;
    bool abandonCheckpoint(const QString &id, qint64 expectedRevision) override;
    // Display-name compatibility at profile-selection boundaries only.
    // Saving and querying results always require the permanent identity.
    PlayerProfile profileNamed(const QString &name) const;
    QStringList loadPlayerProfiles() const;
    bool addPlayerProfile(const QString &name);
    bool renamePlayerProfile(const QString &currentName, const QString &newName);
    bool deletePlayerProfile(const QString &name);

private:
    QVector<HistoryEntry> queryResults(const QString &key, int limit, const QString &playerId) const;
    QString databasePath_;
    QSqlDatabase database_;
};
