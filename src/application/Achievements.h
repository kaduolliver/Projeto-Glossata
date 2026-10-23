#pragma once
#include "domain/Hint.h"
#include "domain/Species.h"
#include <QVariantList>
#include <QMap>
#include <QSet>

struct AchievementRun {
    QString id;
    QVector<AnswerRecord> answers;
    bool complete = false, competitive = false, hintsKnown = false;
    int pendingHints = 0, cycleSize = 0;
};
struct AchievementEvidence {
    QVector<AchievementRun> runs;
    QMap<int, QString> visits;
};
struct AchievementDefinition {
    QString id, title, description, category, metric, icon;
    int target;
};
namespace Achievements {
const QVector<AchievementDefinition> &definitions();
QMap<QString, int> evaluate(const AchievementEvidence &, const QVector<Species> &);
QVariantMap present(const AchievementDefinition &, int progress, const QString &unlockedAt, bool retrospective);
}
