#include "SessionCheckpointStorage.h"
#include "SqliteGameHistoryRepository.h"
#include <QSqlQuery>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDateTime>

namespace {
QString hash(const QByteArray &bytes) { return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()); }
QString now() { return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs); }
}

bool migrateSessionCheckpoints(const QSqlDatabase &database)
{
    QSqlQuery q(database);
    return q.exec("CREATE TABLE session_checkpoints(id TEXT PRIMARY KEY NOT NULL,player_id TEXT NOT NULL REFERENCES player_profiles(id),state TEXT NOT NULL CHECK(state IN ('active','finished','finalized','abandoned')),revision INTEGER NOT NULL CHECK(revision>0),payload TEXT NOT NULL,sha256 TEXT NOT NULL CHECK(length(sha256)=64),created_at TEXT NOT NULL,updated_at TEXT NOT NULL)")
        && q.exec("CREATE INDEX session_checkpoints_pending ON session_checkpoints(state,updated_at)")
        && q.exec("CREATE TRIGGER checkpoint_identity_immutable BEFORE UPDATE OF id,player_id ON session_checkpoints WHEN OLD.id IS NOT NEW.id OR OLD.player_id IS NOT NEW.player_id BEGIN SELECT RAISE(ABORT,'checkpoint ownership is immutable'); END")
        && q.exec("CREATE TRIGGER checkpoint_retained BEFORE DELETE ON session_checkpoints BEGIN SELECT RAISE(ABORT,'retain checkpoint; abandon explicitly'); END");
}

SessionCheckpoint SqliteGameHistoryRepository::checkpoint(const QString &id) const
{
    SessionCheckpoint result;
    if (!database_.isOpen()) { result.error="database_unavailable"; return result; }
    QSqlQuery q(database_);
    q.prepare("SELECT id,player_id,state,revision,updated_at,sha256,CASE WHEN length(CAST(payload AS BLOB))<=? THEN payload ELSE '' END FROM session_checkpoints WHERE id=?");
    q.addBindValue(SessionSnapshot::MaximumBytes); q.addBindValue(id);
    if (!q.exec() || !q.next()) { result.error="checkpoint_unavailable"; return result; }
    result.id=q.value(0).toString(); result.playerId=q.value(1).toString(); result.state=q.value(2).toString();
    result.revision=q.value(3).toLongLong(); result.updatedAt=q.value(4).toString();
    const auto bytes=q.value(6).toString().toUtf8(); QJsonParseError error;
    result.payload=QJsonDocument::fromJson(bytes,&error).object();
    if (hash(bytes)!=q.value(5).toString() || error.error!=QJsonParseError::NoError || !SessionSnapshot::valid(result.payload)
        || result.payload["id"]!=result.id || result.payload["player_id"]!=result.playerId
        || (result.state=="active" && result.payload["finished"].toBool()) || (result.state=="finished" && !result.payload["finished"].toBool())) result.error="checkpoint_integrity_or_rules";
    return result;
}

QVector<SessionCheckpoint> SqliteGameHistoryRepository::recoverableSessions() const
{
    QVector<SessionCheckpoint> result; if (!database_.isOpen()) return result;
    QSqlQuery q(database_); QStringList ids;
    if (!q.exec("SELECT id FROM session_checkpoints WHERE state IN ('active','finished') ORDER BY updated_at DESC,id")) return result;
    while (q.next()) ids.append(q.value(0).toString()); q.finish();
    for (const auto &id:ids) result.append(checkpoint(id));
    return result;
}

bool SqliteGameHistoryRepository::writeCheckpoint(const QJsonObject &p, qint64 expected, qint64 &revision)
{
    const auto bytes=QJsonDocument(p).toJson(QJsonDocument::Compact);
    const auto owner=player(p["player_id"].toString());
    if (!database_.isOpen() || expected<0 || owner.id.isEmpty() || owner.legacy
        || bytes.size()>SessionSnapshot::MaximumBytes || !SessionSnapshot::valid(p)) return false;
    QSqlQuery q(database_); if (!q.exec("BEGIN IMMEDIATE")) return false;
    auto fail=[&] { database_.rollback(); return false; };
    const QString id=p["id"].toString(),state=p["finished"].toBool() ? "finished" : "active";
    const auto old=checkpoint(id);
    if (expected==0) {
        if (!old.id.isEmpty() || !owner.selectable() || !p["answers"].toArray().isEmpty() || p["finished"].toBool() || p["revealed"].toInt()!=0) return fail();
        q.prepare("INSERT INTO session_checkpoints VALUES(?,?,?,1,?,?,?,?)");
        for (const auto &v:QVariantList{id,owner.id,state,QString::fromUtf8(bytes),hash(bytes),now(),now()}) q.addBindValue(v);
    } else {
        if (!old.error.isEmpty() || old.revision!=expected || (old.state!="active" && old.state!="finished")) return fail();
        if (old.payload==p) { database_.rollback(); revision=expected; return true; }
        if (!SessionSnapshot::transition(old.payload,p)) return fail();
        q.prepare("UPDATE session_checkpoints SET state=?,revision=revision+1,payload=?,sha256=?,updated_at=? WHERE id=? AND revision=?");
        for (const auto &v:QVariantList{state,QString::fromUtf8(bytes),hash(bytes),now(),id,expected}) q.addBindValue(v);
    }
    if (!q.exec() || q.numRowsAffected()!=1 || !database_.commit()) return fail();
    revision=expected+1; return true;
}

bool SqliteGameHistoryRepository::abandonCheckpoint(const QString &id, qint64 expected)
{
    if (!database_.isOpen() || expected<=0 || !database_.transaction()) return false;
    QSqlQuery q(database_);
    q.prepare("UPDATE session_checkpoints SET state='abandoned',revision=revision+1,updated_at=? WHERE id=? AND revision=? AND state IN ('active','finished')");
    q.addBindValue(now()); q.addBindValue(id); q.addBindValue(expected);
    if (!q.exec() || q.numRowsAffected()!=1 || !database_.commit()) { database_.rollback(); return false; }
    return true;
}

bool finalizeSessionCheckpoint(const QSqlDatabase &db, const QString &id, const PlayerIdentity &player,
    qint64 units, int correct, const QVector<AnswerRecord> &answers, const GameConfiguration &configuration, bool existingResult)
{
    QSqlQuery q(db);
    q.prepare("SELECT state,CASE WHEN length(CAST(payload AS BLOB))<=? THEN payload ELSE '' END,sha256,player_id FROM session_checkpoints WHERE id=?");
    q.addBindValue(SessionSnapshot::MaximumBytes); q.addBindValue(id);
    if (!q.exec()) return false;
    if (!q.next()) return true; // Existing legacy API/results without checkpoints remain compatible.
    const auto state=q.value(0).toString();
    const auto bytes=q.value(1).toString().toUtf8();
    const auto payload=QJsonDocument::fromJson(bytes).object();
    if ((state!="finished" && !(existingResult && state=="finalized")) || hash(bytes)!=q.value(2).toString()
        || payload["id"]!=id || q.value(3).toString()!=player.id
        || !SessionSnapshot::matchesResult(payload,player,units,correct,answers,configuration)) return false;
    q.finish(); if (state=="finalized") return true;
    q.prepare("UPDATE session_checkpoints SET state='finalized',revision=revision+1,updated_at=? WHERE id=? AND state='finished'");
    q.addBindValue(now()); q.addBindValue(id); return q.exec() && q.numRowsAffected()==1;
}
