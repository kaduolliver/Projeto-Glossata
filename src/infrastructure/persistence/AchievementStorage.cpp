#include "AchievementStorage.h"
#include "SqliteGameHistoryRepository.h"
#include <QSqlQuery>
#include <QJsonArray>

bool migrateAchievements(const QSqlDatabase &db)
{
    QSqlQuery q(db);
    return q.exec("CREATE TABLE IF NOT EXISTS achievement_progress(player_id TEXT NOT NULL REFERENCES player_profiles(id),achievement_id TEXT NOT NULL,progress INTEGER NOT NULL CHECK(progress>=0),unlocked_at TEXT,retrospective INTEGER NOT NULL CHECK(retrospective IN (0,1)),PRIMARY KEY(player_id,achievement_id))")
        && q.exec("CREATE TABLE IF NOT EXISTS species_visits(player_id TEXT NOT NULL REFERENCES player_profiles(id),species_id INTEGER NOT NULL CHECK(species_id>0),scientific_name TEXT NOT NULL,visited_at TEXT NOT NULL,PRIMARY KEY(player_id,species_id))")
        && q.exec("CREATE TRIGGER IF NOT EXISTS achievement_owner_immutable BEFORE UPDATE OF player_id,achievement_id ON achievement_progress WHEN OLD.player_id IS NOT NEW.player_id OR OLD.achievement_id IS NOT NEW.achievement_id BEGIN SELECT RAISE(ABORT,'retain achievement identity'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS achievement_progress_monotonic BEFORE UPDATE ON achievement_progress WHEN NEW.progress<OLD.progress OR (OLD.unlocked_at IS NOT NULL AND (NEW.unlocked_at IS NOT OLD.unlocked_at OR NEW.retrospective<>OLD.retrospective)) BEGIN SELECT RAISE(ABORT,'retain achievement progress'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS achievement_retained BEFORE DELETE ON achievement_progress BEGIN SELECT RAISE(ABORT,'retain achievements'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS species_visit_immutable BEFORE UPDATE ON species_visits BEGIN SELECT RAISE(ABORT,'retain visit identity'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS species_visit_retained BEFORE DELETE ON species_visits BEGIN SELECT RAISE(ABORT,'retain visits'); END");
}

bool validAchievementSchema(const QSqlDatabase &db)
{
    QSqlQuery q(db);
    if (!q.exec("SELECT player_id,achievement_id,progress,unlocked_at,retrospective FROM achievement_progress LIMIT 0")
        || !q.exec("SELECT player_id,species_id,scientific_name,visited_at FROM species_visits LIMIT 0")) return false;
    for (const auto &table : {QString("achievement_progress"),QString("species_visits")}) {
        if (!q.exec("PRAGMA foreign_key_list("+table+")")) return false;
        bool found=false;
        while (q.next()) if (q.value(2)=="player_profiles" && q.value(3)=="player_id" && q.value(4)=="id") found=true;
        if (!found) return false;
    }
    for (const auto &name : {"achievement_owner_immutable","achievement_progress_monotonic","achievement_retained","species_visit_immutable","species_visit_retained"}) {
        q.prepare("SELECT 1 FROM sqlite_master WHERE type='trigger' AND name=?");q.addBindValue(name);
        if (!q.exec() || !q.next()) return false;
    }
    return true;
}

bool SqliteGameHistoryRepository::recordSpeciesVisit(const QString &playerId, const Species &species)
{
    if (!database_.isOpen() || playerId.isEmpty() || localUser().id!=playerId || species.id<=0 || species.name.isEmpty()) return false;
    QSqlQuery q(database_);
    q.prepare("INSERT INTO species_visits VALUES(?,?,?,?) ON CONFLICT(player_id,species_id) DO NOTHING");
    q.addBindValue(playerId); q.addBindValue(species.id); q.addBindValue(species.name);
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    return q.exec();
}

bool SqliteGameHistoryRepository::achievementEvidence(const QString &playerId, AchievementEvidence &result) const
{
    result={};
    QSqlQuery q(database_);
    q.prepare("SELECT id,mode,question_limit,is_complete,answered_count FROM game_runs WHERE player_id=? ORDER BY id");q.addBindValue(playerId);
    if (!q.exec()) return false;
    QSet<QString> runs;
    while (q.next()) {
        AchievementRun run; run.id=q.value(0).toString(); runs.insert(run.id);
        run.answers=loadAnswers(run.id);
        // Aggregate-only legacy totals cannot prove individual answers or hint use.
        if (run.answers.isEmpty() || run.answers.size()!=q.value(4).toInt()) continue;
        run.competitive=q.value(1)=="competitive";
        run.complete=q.value(3).toBool() && (!run.competitive || run.answers.size()==q.value(2).toInt());
        const auto saved=checkpoint(run.id);
        if (saved.error.isEmpty() && saved.playerId==playerId && saved.state=="finalized") {
            run.hintsKnown=true; run.pendingHints=saved.payload["revealed"].toInt();
            run.cycleSize=saved.payload["species_order"].toArray().size();
        }
        // Complete competitive runs cannot have an unanswered trailing question.
        if (run.competitive && run.complete) run.hintsKnown=true;
        result.runs.append(run);
    }
    q.finish();
    q.prepare("SELECT id FROM session_checkpoints WHERE player_id=? ORDER BY id");q.addBindValue(playerId);
    if (!q.exec()) return false;
    QStringList ids;while(q.next()) ids.append(q.value(0).toString());q.finish();
    for (const auto &id : ids) {
        if (runs.contains(id)) continue;
        const auto saved=checkpoint(id);
        if (!saved.error.isEmpty() || saved.playerId!=playerId) continue;
        AchievementRun run;run.id=id;run.hintsKnown=true;run.pendingHints=saved.payload["revealed"].toInt();
        run.cycleSize=saved.payload["species_order"].toArray().size();
        run.competitive=saved.payload["configuration"].toObject()["mode"]=="competitive";
        for (const auto &v : saved.payload["answers"].toArray()) {
            AnswerRecord a; if (!SessionSnapshot::readAnswer(v.toObject(),a)) return false;
            run.answers.append(a);
        }
        // Finished checkpoints await result consolidation; abandoned ones never count as games.
        result.runs.append(run);
    }
    q.prepare("SELECT species_id,scientific_name FROM species_visits WHERE player_id=?");q.addBindValue(playerId);
    if (!q.exec()) return false;
    while (q.next()) result.visits.insert(q.value(0).toInt(),q.value(1).toString());
    return true;
}

bool SqliteGameHistoryRepository::refreshAchievements(const QString &playerId, const QVector<Species> &catalog,
    bool retrospective, QVariantList &rows, QVariantList &newUnlocks)
{
    rows.clear();newUnlocks.clear();
    if (!database_.isOpen() || playerId.isEmpty() || localUser().id!=playerId) return false;
    QSqlQuery q(database_);if (!q.exec("BEGIN IMMEDIATE")) return false;
    auto fail=[&] {database_.rollback();rows.clear();newUnlocks.clear();return false;};
    AchievementEvidence evidence;if (!achievementEvidence(playerId,evidence)) return fail();
    const auto progress=Achievements::evaluate(evidence,catalog);
    const auto now=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    for (const auto &d : Achievements::definitions()) {
        q.prepare("SELECT progress,unlocked_at,retrospective FROM achievement_progress WHERE player_id=? AND achievement_id=?");
        q.addBindValue(playerId);q.addBindValue(d.id);if (!q.exec()) return fail();
        const bool exists=q.next();
        const int value=std::max(progress.value(d.id),exists ? q.value(0).toInt() : 0);
        QString date=exists ? q.value(1).toString() : QString();
        bool retro=exists && q.value(2).toBool();
        const bool unlocked=date.isEmpty() && value>=d.target;
        if (unlocked) {date=now;retro=retrospective;}
        q.finish();
        q.prepare("INSERT INTO achievement_progress VALUES(?,?,?,?,?) ON CONFLICT(player_id,achievement_id) DO UPDATE SET progress=excluded.progress,unlocked_at=excluded.unlocked_at,retrospective=excluded.retrospective");
        q.addBindValue(playerId);q.addBindValue(d.id);q.addBindValue(value);
        q.addBindValue(date.isEmpty() ? QVariant() : QVariant(date));q.addBindValue(int(retro));
        if (!q.exec()) return fail();
        auto row=Achievements::present(d,value,date,retro);row.insert("playerId",playerId);rows.append(row);
        if (unlocked && !retrospective) newUnlocks.append(row);
    }
    if (!database_.commit()) return fail();
    return true;
}
