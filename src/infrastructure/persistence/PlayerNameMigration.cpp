#include "PlayerNameMigration.h"
#include "domain/PlayerNamePolicy.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QVector>
#include <QDebug>

bool validatePlayerNames(const QSqlDatabase &database)
{
    QSqlQuery q(database);
    if (!q.exec("SELECT policy_version,unicode_version FROM player_name_policy WHERE id=1") || !q.next()
        || q.value(0).toString()!=PlayerNamePolicy::version()
        || q.value(1).toInt()!=PlayerNamePolicy::unicodeVersion()) {
        qWarning() << "Unsupported player name policy; explicit migration required";
        return false;
    }
    q.finish();
    if (!q.exec("SELECT name,name_key FROM player_profiles WHERE origin='profile'")) return false;
    while (q.next()) if (q.value(1).toString()!=PlayerNamePolicy::key(q.value(0).toString())) {
        qWarning() << "Player name key inconsistent; retain database and investigate by ID";
        return false;
    }
    return true;
}

bool migratePlayerNames(const QSqlDatabase &database)
{
    QSqlQuery q(database);
    auto exec=[&](const QString &sql) {
        if (q.exec(sql)) return true;
        qWarning() << "Player name migration failed:" << q.lastError().text(); return false;
    };
    if (!exec("CREATE TABLE player_name_policy(id INTEGER PRIMARY KEY CHECK(id=1),policy_version TEXT NOT NULL,unicode_version INTEGER NOT NULL,qt_version TEXT NOT NULL)")
        || !exec("CREATE TABLE player_name_migration(player_id TEXT PRIMARY KEY REFERENCES player_profiles(id),name_snapshot TEXT NOT NULL,old_key TEXT NOT NULL,new_key TEXT NOT NULL,policy_version TEXT NOT NULL,unicode_version INTEGER NOT NULL,migrated_at TEXT NOT NULL,active_collision INTEGER NOT NULL DEFAULT 0 CHECK(active_collision IN (0,1)))")
        // A nonunique index intentionally permits preexisting collisions. Only
        // these two guards are suspended, within the encompassing transaction.
        || !exec("DROP TRIGGER player_name_insert") || !exec("DROP TRIGGER player_name_update")
        || !exec("SELECT id,name,name_key FROM player_profiles WHERE origin='profile' ORDER BY id")) return false;
    QVector<QVariantList> rows;
    while (q.next()) rows.append({q.value(0),q.value(1),q.value(2)});
    q.finish();
    for (const auto &row:rows) {
        const QString key=PlayerNamePolicy::key(row[1].toString());
        q.prepare("INSERT INTO player_name_migration(player_id,name_snapshot,old_key,new_key,policy_version,unicode_version,migrated_at) VALUES(?,?,?,?,?,?,strftime('%Y-%m-%dT%H:%M:%fZ','now'))");
        for (const auto &v:QVariantList{row[0],row[1],row[2],key,PlayerNamePolicy::version(),PlayerNamePolicy::unicodeVersion()}) q.addBindValue(v);
        if (!q.exec()) return false;
        q.prepare("UPDATE player_profiles SET name_key=? WHERE id=?"); q.addBindValue(key); q.addBindValue(row[0]);
        if (!q.exec() || q.numRowsAffected()!=1) return false;
    }
    if (!exec("CREATE VIEW player_name_conflicts AS SELECT p.id AS player_id,p.name,p.name_key FROM player_profiles p WHERE p.origin='profile' AND p.deleted_at IS NULL AND EXISTS(SELECT 1 FROM player_profiles other WHERE other.origin='profile' AND other.deleted_at IS NULL AND other.id<>p.id AND other.name_key=p.name_key)")
        || !exec("UPDATE player_name_migration SET active_collision=1 WHERE player_id IN (SELECT player_id FROM player_name_conflicts)")
        || !exec("CREATE TRIGGER player_name_insert BEFORE INSERT ON player_profiles WHEN NEW.origin='profile' AND NEW.deleted_at IS NULL AND EXISTS(SELECT 1 FROM player_profiles WHERE origin='profile' AND deleted_at IS NULL AND name_key=NEW.name_key) BEGIN SELECT RAISE(ABORT,'active player name conflict'); END")
        || !exec("CREATE TRIGGER player_name_update BEFORE UPDATE OF name,name_key,deleted_at,origin ON player_profiles WHEN NEW.origin='profile' AND NEW.deleted_at IS NULL AND EXISTS(SELECT 1 FROM player_profiles WHERE origin='profile' AND deleted_at IS NULL AND id<>NEW.id AND name_key=NEW.name_key) BEGIN SELECT RAISE(ABORT,'active player name conflict'); END")) return false;
    q.prepare("INSERT INTO player_name_policy VALUES(1,?,?,?)");
    q.addBindValue(PlayerNamePolicy::version()); q.addBindValue(PlayerNamePolicy::unicodeVersion()); q.addBindValue(QString::fromLatin1(qVersion()));
    return q.exec() && validatePlayerNames(database);
}
