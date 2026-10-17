#pragma once
#include "domain/SessionCheckpoint.h"
#include <QSqlDatabase>

bool migrateSessionCheckpoints(const QSqlDatabase &database);
// Must run inside the same transaction as the final game/ranking writes.
bool finalizeSessionCheckpoint(const QSqlDatabase &database, const QString &id, const PlayerIdentity &player,
    qint64 units, int correct, const QVector<AnswerRecord> &answers, const GameConfiguration &configuration, bool existingResult);
