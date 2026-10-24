#pragma once
#include <QSqlDatabase>
bool migrateAchievements(const QSqlDatabase &);
bool validAchievementSchema(const QSqlDatabase &);
