#include "PlayerIdentityMigration.h"
#include "domain/PlayerNamePolicy.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QVector>
#include <QDebug>
#include <QStringList>
#include <QVariant>

bool migratePlayerIdentity(const QSqlDatabase &database)
{
    QSqlQuery q(database);
    auto exec = [&](const QString &sql) {
        if (q.exec(sql)) return true;
        qWarning() << "Identity migration failed:" << q.lastError().text(); return false;
    };
    auto id = [] { return QUuid::createUuid().toString(QUuid::WithoutBraces); };
    if (!exec("ALTER TABLE player_profiles RENAME TO legacy_player_profiles_v3")
        || !exec("ALTER TABLE history RENAME TO legacy_history_v3")
        || !exec("ALTER TABLE ranking_records RENAME TO legacy_ranking_records_v3")
        || !exec("CREATE TABLE player_profiles (id TEXT PRIMARY KEY NOT NULL, name TEXT NOT NULL, name_key TEXT NOT NULL, origin TEXT NOT NULL CHECK(origin IN ('profile','legacy_unresolved')), deleted_at TEXT, legacy_name_key TEXT COLLATE NOCASE)")
        || !exec("CREATE INDEX player_active_name_key ON player_profiles(name_key) WHERE deleted_at IS NULL AND origin='profile'")
        || !exec("CREATE UNIQUE INDEX legacy_identity_name_key ON player_profiles(legacy_name_key) WHERE origin='legacy_unresolved'")
        || !exec("CREATE TABLE identity_migration_notes (id INTEGER PRIMARY KEY, player_id TEXT NOT NULL REFERENCES player_profiles(id), source_table TEXT NOT NULL, source_key TEXT NOT NULL, reason TEXT NOT NULL)")
        || !exec("SELECT name FROM legacy_player_profiles_v3 ORDER BY name COLLATE NOCASE,name")) return false;
    QStringList names;
    while (q.next()) names.append(q.value(0).toString());
    q.finish();
    for (const auto &name : names) {
        const QString playerId = id();
        q.prepare("INSERT INTO player_profiles(id,name,name_key,origin) VALUES(?,?,?,'profile')");
        q.addBindValue(playerId); q.addBindValue(name);
        q.addBindValue(PlayerNamePolicy::key(name));
        if (!q.exec()) return false;
        q.prepare("INSERT INTO identity_migration_notes(player_id,source_table,source_key,reason) VALUES(?,'legacy_player_profiles_v3',?,'profile_id_created; historical ownership not inferred from name')");
        q.addBindValue(playerId); q.addBindValue(name);
        if (!q.exec()) return false;
    }
    // Older schemas may not have this archive. No records are manufactured.
    if (!exec("CREATE TABLE IF NOT EXISTS legacy_history_results(original_id INTEGER PRIMARY KEY,player_name TEXT NOT NULL,score_units INTEGER NOT NULL,correct_answers INTEGER NOT NULL,played_at TEXT NOT NULL)")
        || !exec("SELECT player_name COLLATE NOCASE FROM legacy_ranking_records_v3 UNION SELECT player_name FROM legacy_history_v3 UNION SELECT player_name FROM game_runs UNION SELECT player_name FROM legacy_history_results")) return false;
    names.clear(); while (q.next()) names.append(q.value(0).toString()); q.finish();
    for (const auto &name : names) {
        const QString playerId = id();
        q.prepare("INSERT INTO player_profiles(id,name,name_key,origin,legacy_name_key) VALUES(?,?,?,'legacy_unresolved',?)");
        for (const auto &v : QStringList{playerId,name,name,name}) q.addBindValue(v);
        if (!q.exec()) return false;
        q.prepare("INSERT INTO identity_migration_notes(player_id,source_table,source_key,reason) VALUES(?,'pre_id_name_bucket',?,'unresolved: old NOCASE grouping may contain different people; no active profile linked')");
        q.addBindValue(playerId); q.addBindValue(name);
        if (!q.exec()) return false;
    }
    if (!exec("CREATE TABLE history(id INTEGER PRIMARY KEY AUTOINCREMENT,player_id TEXT NOT NULL UNIQUE REFERENCES player_profiles(id),player_name TEXT NOT NULL,score_units INTEGER NOT NULL,correct_answers INTEGER NOT NULL,played_at TEXT NOT NULL)")
        || !exec("CREATE TABLE ranking_records(ranking_key TEXT NOT NULL,player_id TEXT NOT NULL REFERENCES player_profiles(id),player_name TEXT NOT NULL,best_units INTEGER NOT NULL,best_correct INTEGER NOT NULL,best_played_at TEXT NOT NULL,last_units INTEGER NOT NULL,last_correct INTEGER NOT NULL,last_played_at TEXT NOT NULL,PRIMARY KEY(ranking_key,player_id))")
        || !exec("ALTER TABLE game_runs ADD COLUMN player_id TEXT REFERENCES player_profiles(id)")
        || !exec("ALTER TABLE legacy_history_results ADD COLUMN player_id TEXT REFERENCES player_profiles(id)")) return false;
    const QString owner = "(SELECT p.id FROM player_profiles p WHERE p.origin='legacy_unresolved' AND p.legacy_name_key=s.player_name COLLATE NOCASE)";
    if (!exec("INSERT INTO history SELECT s.id,"+owner+",s.player_name,s.score_units,s.correct_answers,s.played_at FROM legacy_history_v3 s WHERE NOT EXISTS(SELECT 1 FROM legacy_history_v3 newer WHERE newer.player_name=s.player_name COLLATE NOCASE AND (newer.played_at>s.played_at OR (newer.played_at=s.played_at AND newer.id>s.id)))")
        || !exec("INSERT INTO ranking_records SELECT s.ranking_key,"+owner+",s.player_name,s.best_units,s.best_correct,s.best_played_at,s.last_units,s.last_correct,s.last_played_at FROM legacy_ranking_records_v3 s")
        || !exec("UPDATE game_runs AS s SET player_id="+owner)
        || !exec("UPDATE legacy_history_results AS s SET player_id="+owner)
        || !exec("CREATE INDEX game_runs_identity_configuration ON game_runs(player_id,ranking_key)")
        || !exec("SELECT COUNT(*) FROM game_runs WHERE player_id IS NULL") || !q.next() || q.value(0).toInt()!=0) return false;
    q.finish();
    // Existing Unicode name collisions stay separate; only new conflicts are blocked.
    if (!exec("INSERT INTO identity_migration_notes(player_id,source_table,source_key,reason) SELECT p.id,'legacy_player_profiles_v3',p.name,'existing Unicode name collision preserved; select by ID and rename explicitly' FROM player_profiles p WHERE p.origin='profile' AND EXISTS(SELECT 1 FROM player_profiles other WHERE other.origin='profile' AND other.id<>p.id AND other.name_key=p.name_key)")
        || !exec("CREATE TRIGGER player_identity_immutable BEFORE UPDATE OF id ON player_profiles BEGIN SELECT RAISE(ABORT,'player identity is immutable'); END")
        || !exec("CREATE TRIGGER player_identity_retained BEFORE DELETE ON player_profiles BEGIN SELECT RAISE(ABORT,'retire profiles; retain identity and results'); END")
        || !exec("CREATE TRIGGER player_name_insert BEFORE INSERT ON player_profiles WHEN NEW.origin='profile' AND NEW.deleted_at IS NULL AND EXISTS(SELECT 1 FROM player_profiles WHERE origin='profile' AND deleted_at IS NULL AND name_key=NEW.name_key) BEGIN SELECT RAISE(ABORT,'active player name conflict'); END")
        || !exec("CREATE TRIGGER player_name_update BEFORE UPDATE OF name,name_key,deleted_at ON player_profiles WHEN NEW.origin='profile' AND NEW.deleted_at IS NULL AND EXISTS(SELECT 1 FROM player_profiles WHERE origin='profile' AND deleted_at IS NULL AND id<>NEW.id AND name_key=NEW.name_key) BEGIN SELECT RAISE(ABORT,'active player name conflict'); END")
        || !exec("CREATE TRIGGER game_owner_required BEFORE INSERT ON game_runs WHEN NEW.player_id IS NULL BEGIN SELECT RAISE(ABORT,'game requires player identity'); END")
        || !exec("CREATE TRIGGER game_owner_immutable BEFORE UPDATE OF player_id ON game_runs WHEN OLD.player_id IS NOT NEW.player_id BEGIN SELECT RAISE(ABORT,'game ownership is immutable'); END")
        || !exec("CREATE TRIGGER game_name_snapshot_immutable BEFORE UPDATE OF player_name ON game_runs WHEN OLD.player_name IS NOT NEW.player_name BEGIN SELECT RAISE(ABORT,'game name snapshot is immutable'); END")) return false;
    return true;
}
