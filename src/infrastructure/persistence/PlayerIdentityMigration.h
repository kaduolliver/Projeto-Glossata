#pragma once
#include <QSqlDatabase>

// Caller owns the SQLite transaction, including all preceding schema upgrades.
bool migratePlayerIdentity(const QSqlDatabase &database);
