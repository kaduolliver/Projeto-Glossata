#pragma once

#include "domain/GameSession.h"
#include "domain/Question.h"
#include <QJsonObject>

// A resumable engine snapshot, independent of the ranking and of QML pages.
struct SessionCheckpoint {
    QString id, playerId, state, updatedAt, error;
    qint64 revision = 0;
    QJsonObject payload;
};

namespace SessionSnapshot {
constexpr int Schema = 1;
constexpr int MaximumBytes = 32 * 1024 * 1024;
QJsonObject answer(const AnswerRecord &record);
bool readAnswer(const QJsonObject &value, AnswerRecord &record);
QJsonObject question(const Question &question);
bool readQuestion(const QJsonObject &value, const QVector<Species> &pool, Question &question);
QJsonObject configuration(const GameConfiguration &configuration);
bool readConfiguration(const QJsonObject &value, GameConfiguration &configuration);
bool valid(const QJsonObject &payload);
bool transition(const QJsonObject &previous, const QJsonObject &next);
bool matchesResult(const QJsonObject &payload, const PlayerIdentity &player, qint64 units, int correct,
                   const QVector<AnswerRecord> &answers, const GameConfiguration &configuration);
}
