#pragma once

#include "domain/Hint.h"
#include "domain/GameConfiguration.h"
#include <QVector>
struct GameSession {
    qint64 scoreUnits = 0;
    int correctAnswers = 0;
    int answeredCount = 0;
    int round = 1;
    bool finished = false;
    QString id;
    QVector<AnswerRecord> answers;
    GameConfiguration configuration;
};
