#pragma once
#include <QSqlDatabase>

// Called inside the repository's schema transaction. Never changes identities,
// display spelling, game snapshots or results, including unresolved old buckets.
bool migratePlayerNames(const QSqlDatabase &database);
bool validatePlayerNames(const QSqlDatabase &database);
