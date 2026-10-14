#pragma once

#include <QString>
#include <QStringList>
#include "domain/Species.h"

struct Question {
    QString prompt;
    QString fact;
    QString extraHint;
    QString correctAnswer;
    QStringList options;
    Species species;
};
