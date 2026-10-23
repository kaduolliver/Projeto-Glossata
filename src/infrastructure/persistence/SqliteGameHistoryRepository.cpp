#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include "PlayerIdentityMigration.h"
#include "PlayerNameMigration.h"
#include "SessionCheckpointStorage.h"
#include "domain/PlayerNamePolicy.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QUuid>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

namespace {
int connectionCounter = 0;

bool validCurrentSchema(const QSqlDatabase &database, bool withLocalUser = false)
{
    // user_version identifies a migration, not proof that its tables/guards
    // survived. Refuse incomplete stores; never rebuild them over player data.
    QMap<QString, QString> tables{
        {"player_profiles", "id,name,name_key,origin,deleted_at,legacy_name_key"},
        {"history", "id,player_id,player_name,score_units,correct_answers,played_at"},
        {"ranking_records", "ranking_key,player_id,player_name,best_units,best_correct,best_played_at,last_units,last_correct,last_played_at"},
        {"game_runs", "id,player_id,player_name,finished_at,mode,question_limit,group_id,rank,difficulty,rules_version,content_version,ranking_key,score_units,correct_answers,answered_count,is_complete"},
        {"answer_records", "game_id,question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot"},
        {"identity_migration_notes", "id,player_id,source_table,source_key,reason"},
        {"legacy_history_results", "original_id,player_name,score_units,correct_answers,played_at,player_id"},
        {"player_name_policy", "id,policy_version,unicode_version,qt_version"},
        {"player_name_migration", "player_id,name_snapshot,old_key,new_key,policy_version,unicode_version,migrated_at,active_collision"},
        {"session_checkpoints", "id,player_id,state,revision,payload,sha256,created_at,updated_at"}
    };
    if (withLocalUser) tables.insert("local_user_binding", "id,player_id,selected_at,selection_reason");
    QSqlQuery q(database);
    auto object = [&](const QString &name, const QString &type, const QString &table) {
        q.prepare("SELECT 1 FROM sqlite_master WHERE name=? AND type=? AND tbl_name=?");
        q.addBindValue(name); q.addBindValue(type); q.addBindValue(table);
        const bool found=q.exec() && q.next(); q.finish(); return found;
    };
    for (auto it=tables.cbegin(); it!=tables.cend(); ++it) {
        if (!object(it.key(),"table",it.key())
            || !q.exec("SELECT "+it.value()+" FROM \""+it.key()+"\" LIMIT 0")) return false;
        q.finish();
    }
    QMap<QString, QString> guards{
        {"player_identity_immutable","player_profiles"}, {"player_identity_retained","player_profiles"},
        {"player_name_insert","player_profiles"}, {"player_name_update","player_profiles"},
        {"game_owner_required","game_runs"}, {"game_owner_immutable","game_runs"},
        {"game_name_snapshot_immutable","game_runs"},
        {"checkpoint_identity_immutable","session_checkpoints"}, {"checkpoint_retained","session_checkpoints"}
    };
    if (withLocalUser) {
        guards.insert("local_user_insert_once","local_user_binding");
        guards.insert("local_user_immutable","local_user_binding");
        guards.insert("local_user_retained","local_user_binding");
        guards.insert("local_user_profile_retained","player_profiles");
    }
    for (auto it=guards.cbegin(); it!=guards.cend(); ++it)
        if (!object(it.key(),"trigger",it.value())) return false;
    if (!object("player_name_conflicts","view","player_name_conflicts")) return false;
    QMap<QString, QString> references{
        {"history","player_id:player_profiles:id"}, {"ranking_records","player_id:player_profiles:id"},
        {"game_runs","player_id:player_profiles:id"},
        {"session_checkpoints","player_id:player_profiles:id"},
        {"identity_migration_notes","player_id:player_profiles:id"},
        {"player_name_migration","player_id:player_profiles:id"},
        {"legacy_history_results","player_id:player_profiles:id"}
    };
    if (withLocalUser) references.insert("local_user_binding","player_id:player_profiles:id");
    for (auto it=references.cbegin(); it!=references.cend(); ++it) {
        if (!q.exec("PRAGMA foreign_key_list(\""+it.key()+"\")")) return false;
        bool found=false;
        while (q.next()) if (q.value(3).toString()+":"+q.value(2).toString()+":"+q.value(4).toString()==it.value()) found=true;
        q.finish(); if (!found) return false;
    }
    // Some supported pre-v2 answer tables never declared this FK. Check the
    // actual relation without rebuilding the table or rewriting legacy rows.
    if (!q.exec("SELECT 1 FROM answer_records a LEFT JOIN game_runs g ON g.id=a.game_id WHERE g.id IS NULL LIMIT 1")) return false;
    const bool orphan=q.next(); q.finish();
    if (orphan) return false;
    if (withLocalUser) {
        if (!q.exec("SELECT 1 FROM local_user_binding b LEFT JOIN player_profiles p ON p.id=b.player_id WHERE b.id<>1 OR p.id IS NULL OR p.origin<>'profile' OR p.deleted_at IS NOT NULL LIMIT 1")) return false;
        if (q.next()) return false;
    }
    return true;
}

bool migrateLocalUser(const QSqlDatabase &database)
{
    QSqlQuery q(database);
    return q.exec("CREATE TABLE IF NOT EXISTS local_user_binding(id INTEGER PRIMARY KEY CHECK(id=1),player_id TEXT NOT NULL UNIQUE REFERENCES player_profiles(id),selected_at TEXT NOT NULL,selection_reason TEXT NOT NULL CHECK(selection_reason IN ('new_installation','single_existing','explicit_transition')))")
        && q.exec("CREATE TRIGGER IF NOT EXISTS local_user_insert_once BEFORE INSERT ON local_user_binding WHEN EXISTS(SELECT 1 FROM local_user_binding) OR NOT EXISTS(SELECT 1 FROM player_profiles WHERE id=NEW.player_id AND origin='profile' AND deleted_at IS NULL) BEGIN SELECT RAISE(ABORT,'local user binding is permanent and requires an active identity'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS local_user_immutable BEFORE UPDATE ON local_user_binding BEGIN SELECT RAISE(ABORT,'local user binding is immutable'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS local_user_retained BEFORE DELETE ON local_user_binding BEGIN SELECT RAISE(ABORT,'retain local user binding'); END")
        && q.exec("CREATE TRIGGER IF NOT EXISTS local_user_profile_retained BEFORE UPDATE OF origin,deleted_at ON player_profiles WHEN OLD.id=(SELECT player_id FROM local_user_binding WHERE id=1) AND (NEW.origin<>'profile' OR NEW.deleted_at IS NOT NULL) BEGIN SELECT RAISE(ABORT,'retain active local identity'); END");
}

QString answerSnapshot(const AnswerRecord &answer)
{
    QJsonArray ceilings,ids,payloads;
    const bool legacy=answer.rulesVersion==LegacyScoringRules || answer.rulesVersion=="legacy";
    for (int i=0;i<3;++i) {
        ceilings.append(legacy && (answer.revealedHints & (1<<i)) ? legacyMaximumScoreUnits(1<<i) : answer.ceilings[i]);
        ids.append(answer.assessmentIds[i]);
        payloads.append(answer.assessmentPayloads[i]);
    }
    QJsonObject snapshot{{"schema",1},{"rules",answer.rulesVersion},
        {"ceilings",ceilings},{"assessment_ids",ids},{"assessment_payloads",payloads},
        {"options",QJsonArray::fromStringList(answer.options)}};
    if(answer.rulesVersion==CurrentScoringRules)snapshot["content_version"]=answer.contentVersion;
    return QString::fromUtf8(QJsonDocument(snapshot).toJson(QJsonDocument::Compact));
}

bool readSnapshot(const QString &json,AnswerRecord &answer)
{
    QJsonParseError error;
    const auto document=QJsonDocument::fromJson(json.toUtf8(),&error);
    const auto value=document.object();
    const auto ceilings=value.value("ceilings").toArray(),ids=value.value("assessment_ids").toArray();
    const auto payloads=value.value("assessment_payloads").toArray();
    if (error.error!=QJsonParseError::NoError || value.value("schema").toInt()!=1
        || value.value("rules").toString()!=answer.rulesVersion || ceilings.size()!=3 || ids.size()!=3 || payloads.size()!=3) return false;
    for (int i=0;i<3;++i) {
        if (!ceilings[i].isDouble() || ceilings[i].toDouble()!=ceilings[i].toInt() || !ids[i].isString()) return false;
        answer.ceilings[i]=ceilings[i].toInt();answer.assessmentIds[i]=ids[i].toString();
        if (!payloads[i].isObject()) return false;
        answer.assessmentPayloads[i]=payloads[i].toObject();
    }
    if (!value.value("options").isArray()) return false;
    for (const auto &option : value.value("options").toArray()) {
        if (!option.isString()) return false;
        answer.options.append(option.toString());
    }
    answer.contentVersion=value.value("content_version").toString();
    return validAnswerScoring(answer,answer.rulesVersion);
}

bool upsertLegacyRanking(const QSqlDatabase &database, const QString &key, const QString &name,
                   qint64 units, int correct, const QString &date)
{
    QSqlQuery query(database);
    const QString better = "(excluded.best_units>ranking_records.best_units OR (excluded.best_units=ranking_records.best_units AND (excluded.best_correct>ranking_records.best_correct OR (excluded.best_correct=ranking_records.best_correct AND excluded.best_played_at<ranking_records.best_played_at))))";
    query.prepare("INSERT INTO ranking_records VALUES (?,?,?,?,?,?,?,?) ON CONFLICT(ranking_key,player_name) DO UPDATE SET "
        "player_name=excluded.player_name,best_units=CASE WHEN " + better + " THEN excluded.best_units ELSE ranking_records.best_units END,"
        "best_correct=CASE WHEN " + better + " THEN excluded.best_correct ELSE ranking_records.best_correct END,"
        "best_played_at=CASE WHEN " + better + " THEN excluded.best_played_at ELSE ranking_records.best_played_at END,"
        "last_units=CASE WHEN excluded.last_played_at>=ranking_records.last_played_at THEN excluded.last_units ELSE ranking_records.last_units END,"
        "last_correct=CASE WHEN excluded.last_played_at>=ranking_records.last_played_at THEN excluded.last_correct ELSE ranking_records.last_correct END,"
        "last_played_at=MAX(excluded.last_played_at,ranking_records.last_played_at)");
    for (const QVariant &value : QVariantList{key,name,units,correct,date,units,correct,date}) query.addBindValue(value);
    return query.exec();
}

QString resolvedDatabasePath(const QString &requestedPath)
{
    QString resolvedPath = requestedPath;
    if (!QDir::isAbsolutePath(resolvedPath)) {
        QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (base.isEmpty()) {
            base = QDir::homePath();
        }

        QDir baseDirectory(base);
        if (!baseDirectory.exists() && !baseDirectory.mkpath(QStringLiteral("."))) {
            return {};
        }
        resolvedPath = baseDirectory.filePath(resolvedPath);
    }

    const QString parentPath = QFileInfo(resolvedPath).absolutePath();
    QDir parentDirectory(parentPath);
    if (!parentDirectory.exists() && !parentDirectory.mkpath(QStringLiteral("."))) {
        return {};
    }

    return QFileInfo(resolvedPath).absoluteFilePath();
}

bool upsertRanking(const QSqlDatabase &database, const QString &key, const PlayerIdentity &player,
                   qint64 units, int correct, const QString &date)
{
    QSqlQuery q(database);
    const QString better = "(excluded.best_units>ranking_records.best_units OR (excluded.best_units=ranking_records.best_units AND (excluded.best_correct>ranking_records.best_correct OR (excluded.best_correct=ranking_records.best_correct AND excluded.best_played_at<ranking_records.best_played_at))))";
    q.prepare("INSERT INTO ranking_records(ranking_key,player_id,player_name,best_units,best_correct,best_played_at,last_units,last_correct,last_played_at) VALUES(?,?,?,?,?,?,?,?,?) ON CONFLICT(ranking_key,player_id) DO UPDATE SET "
        "player_name=excluded.player_name,best_units=CASE WHEN "+better+" THEN excluded.best_units ELSE ranking_records.best_units END,"
        "best_correct=CASE WHEN "+better+" THEN excluded.best_correct ELSE ranking_records.best_correct END,"
        "best_played_at=CASE WHEN "+better+" THEN excluded.best_played_at ELSE ranking_records.best_played_at END,"
        "last_units=CASE WHEN excluded.last_played_at>=ranking_records.last_played_at THEN excluded.last_units ELSE ranking_records.last_units END,"
        "last_correct=CASE WHEN excluded.last_played_at>=ranking_records.last_played_at THEN excluded.last_correct ELSE ranking_records.last_correct END,"
        "last_played_at=MAX(excluded.last_played_at,ranking_records.last_played_at)");
    for (const QVariant &v : QVariantList{key,player.id,player.displayName,units,correct,date,units,correct,date}) q.addBindValue(v);
    return q.exec();
}
}

SqliteGameHistoryRepository::SqliteGameHistoryRepository(const QString &databasePath)
    : databasePath_(resolvedDatabasePath(databasePath)), database_(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("glossata_history_%1").arg(++connectionCounter)))
{
    if (databasePath_.isEmpty()) {
        qWarning() << "Não foi possível preparar o caminho do banco de dados.";
        return;
    }
    database_.setDatabaseName(databasePath_);
    if (!database_.open()) {
        qWarning() << "Não foi possível abrir o banco:" << database_.lastError().text();
        return;
    }

    QSqlQuery query(database_);
    if (!query.exec("PRAGMA foreign_keys=ON") || !query.exec("PRAGMA synchronous=FULL")
        || !query.exec("PRAGMA busy_timeout=2000") || !query.exec("PRAGMA user_version") || !query.next()) {
        database_.close(); return;
    }
    const int version = query.value(0).toInt();
    query.finish();
    if (version > 7) { database_.close(); return; }
    if (version < 0) { database_.close(); return; }
    if (version >= 6) {
        if (!validCurrentSchema(database_, version == 7) || !validatePlayerNames(database_)
            || !query.exec("PRAGMA foreign_key_check") || query.next()) {
            qWarning() << "Incompatible or incomplete player database; original retained";
            database_.close(); return;
        }
        if (version == 7) return;
    }
    if (!database_.transaction()) { database_.close(); return; }
    auto exec = [&](const QString &sql) { return query.exec(sql); };
    auto columns = [&](const QString &table) {
        QSet<QString> result;
        query.exec("PRAGMA table_info(" + table + ")");
        while (query.next()) result.insert(query.value(1).toString());
        query.finish(); return result;
    };
    bool ok = true;
    if (version < 4) {
        ok = exec("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, player_name TEXT NOT NULL, score INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)")
            && exec("CREATE TABLE IF NOT EXISTS player_profiles (name TEXT PRIMARY KEY COLLATE NOCASE)");
        if (ok && !columns("history").contains("score_units"))
            ok = exec("ALTER TABLE history RENAME COLUMN score TO score_units") && exec("UPDATE history SET score_units=score_units*6");
        // Preserve even obsolete duplicate rows before retaining the existing unique-name policy.
        if (ok && version < 2) ok = exec("CREATE TABLE IF NOT EXISTS legacy_history_results (original_id INTEGER PRIMARY KEY, player_name TEXT NOT NULL, score_units INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)")
            && exec("INSERT OR IGNORE INTO legacy_history_results SELECT id,player_name,score_units,correct_answers,played_at FROM history");
        ok = ok && exec("CREATE TABLE IF NOT EXISTS game_runs (id TEXT PRIMARY KEY, player_name TEXT NOT NULL, finished_at TEXT NOT NULL)")
            && exec("CREATE TABLE IF NOT EXISTS answer_records (game_id TEXT NOT NULL REFERENCES game_runs(id), question_number INTEGER NOT NULL, species_id INTEGER NOT NULL, scientific_name TEXT NOT NULL, photo_id TEXT NOT NULL, rank TEXT NOT NULL, selected_answer TEXT NOT NULL, correct_answer TEXT NOT NULL, is_correct INTEGER NOT NULL CHECK(is_correct IN (0,1)), revealed_hints INTEGER NOT NULL CHECK(revealed_hints BETWEEN 0 AND 7), maximum_units INTEGER NOT NULL CHECK(maximum_units IN (2,3,6)), awarded_units INTEGER NOT NULL CHECK(awarded_units IN (0,2,3,6)), PRIMARY KEY(game_id,question_number), CHECK(awarded_units=CASE WHEN is_correct=1 THEN maximum_units ELSE 0 END))");
        const auto existing = columns("game_runs");
        for (const auto &column : QStringList{
            "mode TEXT NOT NULL DEFAULT 'free'", "question_limit INTEGER NOT NULL DEFAULT 0", "group_id TEXT NOT NULL DEFAULT ''",
            "rank TEXT NOT NULL DEFAULT ''", "difficulty TEXT NOT NULL DEFAULT 'balanced'", "rules_version TEXT NOT NULL DEFAULT 'legacy'",
            "content_version TEXT NOT NULL DEFAULT ''", "ranking_key TEXT NOT NULL DEFAULT 'free'", "score_units INTEGER",
            "correct_answers INTEGER", "answered_count INTEGER", "is_complete INTEGER NOT NULL DEFAULT 0"}) {
            if (ok && !existing.contains(column.section(' ', 0, 0))) ok = exec("ALTER TABLE game_runs ADD COLUMN " + column);
        }
        ok = ok && exec("CREATE TABLE IF NOT EXISTS ranking_records (ranking_key TEXT NOT NULL, player_name TEXT NOT NULL COLLATE NOCASE, best_units INTEGER NOT NULL, best_correct INTEGER NOT NULL, best_played_at TEXT NOT NULL, last_units INTEGER NOT NULL, last_correct INTEGER NOT NULL, last_played_at TEXT NOT NULL, PRIMARY KEY(ranking_key,player_name))")
            && exec("CREATE INDEX IF NOT EXISTS game_runs_player_configuration ON game_runs(player_name COLLATE NOCASE,ranking_key)");
        if (ok && version < 2) {
            ok = exec("UPDATE game_runs SET score_units=(SELECT SUM(awarded_units) FROM answer_records WHERE game_id=game_runs.id), correct_answers=(SELECT SUM(is_correct) FROM answer_records WHERE game_id=game_runs.id), answered_count=(SELECT COUNT(*) FROM answer_records WHERE game_id=game_runs.id), is_complete=1 WHERE EXISTS(SELECT 1 FROM answer_records WHERE game_id=game_runs.id)");
            QVector<QVariantList> available;
            if (ok) ok = query.exec("SELECT player_name,score_units,correct_answers,finished_at FROM game_runs WHERE score_units IS NOT NULL ORDER BY finished_at,rowid");
            while (query.next()) available.append({query.value(0),query.value(1),query.value(2),query.value(3)});
            query.finish();
            if (ok) ok = query.exec("SELECT player_name,score_units,correct_answers,played_at FROM legacy_history_results ORDER BY played_at,original_id");
            while (query.next()) available.append({query.value(0),query.value(1),query.value(2),query.value(3)});
            query.finish();
            for (const auto &row : available) if (ok) ok = upsertLegacyRanking(database_, "free", row[0].toString(),row[1].toLongLong(),row[2].toInt(),row[3].toString());
        }
        const auto answerColumns=columns("answer_records");
        if (ok && !answerColumns.contains("rules_version")) ok=exec("ALTER TABLE answer_records ADD COLUMN rules_version TEXT NOT NULL DEFAULT 'identification-v2'");
        if (ok && !answerColumns.contains("hint_snapshot")) ok=exec("ALTER TABLE answer_records ADD COLUMN hint_snapshot TEXT NOT NULL DEFAULT '{}'");
        if (ok && version<3) {
            QVector<QVariantList> legacy;
            ok=query.exec("SELECT a.game_id,a.question_number,a.revealed_hints,g.rules_version FROM answer_records a JOIN game_runs g ON g.id=a.game_id");
            while (query.next()) legacy.append({query.value(0),query.value(1),query.value(2),query.value(3)});
            query.finish();
            for (const auto &row : legacy) {
                AnswerRecord answer;answer.revealedHints=row[2].toInt();answer.rulesVersion=row[3].toString();
                // Freeze the historical category policy; never UPDATE points/rankings.
                for (int i=0;i<3;++i) if (answer.revealedHints & (1<<i)) answer.ceilings[i]=legacyMaximumScoreUnits(1<<i);
                query.prepare("UPDATE answer_records SET rules_version=?,hint_snapshot=? WHERE game_id=? AND question_number=?");
                for (const auto &v : QVariantList{answer.rulesVersion,answerSnapshot(answer),row[0],row[1]}) query.addBindValue(v);
                if (!query.exec()) {ok=false;break;}
            }
        }
        if (ok) ok = migratePlayerIdentity(database_);
    }
    if (ok && version<5) ok = migratePlayerNames(database_);
    if (ok) ok = validatePlayerNames(database_) && (version>=6 || migrateSessionCheckpoints(database_)) && validCurrentSchema(database_) && migrateLocalUser(database_) && validCurrentSchema(database_,true);
    if (ok) {
        ok = exec("PRAGMA foreign_key_check");
        if (query.next()) ok = false;
        query.finish();
    }
    ok = ok && exec("PRAGMA user_version=7");
    if (!ok || !database_.commit()) {
        qWarning() << "Não foi possível migrar o histórico:" << query.lastError().text() << database_.lastError().text();
        database_.rollback(); database_.close();
    }
}

SqliteGameHistoryRepository::~SqliteGameHistoryRepository()
{
    const QString connectionName = database_.connectionName();
    database_.close();
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

bool SqliteGameHistoryRepository::saveResult(const PlayerIdentity &player, qint64 scoreUnits, int correctAnswers,
                                            const QVector<AnswerRecord> &answers,const QString &gameId)
{
    GameConfiguration configuration;
    configuration.rulesVersion=answers.isEmpty() ? QStringLiteral("legacy") : answers.first().rulesVersion;
    return saveConfiguredResult(player, scoreUnits, correctAnswers, answers, gameId, configuration);
}

bool SqliteGameHistoryRepository::saveConfiguredResult(const PlayerIdentity &player, qint64 scoreUnits, int correctAnswers,
    const QVector<AnswerRecord> &answers, const QString &gameId, const GameConfiguration &configuration)
{
    const QString normalizedName = player.displayName;
    const auto owner = this->player(player.id);
    if (!database_.isOpen() || !player.isValid() || owner.id.isEmpty() || owner.legacy || scoreUnits<0 || correctAnswers<0) {
        return false;
    }
    if (configuration.rulesVersion!=CurrentScoringRules && configuration.rulesVersion!=LegacyScoringRules && configuration.rulesVersion!=PreviousScoringRules && configuration.rulesVersion!="legacy") return false;
    if (answers.isEmpty() && (configuration.rulesVersion==CurrentScoringRules || configuration.rulesVersion==PreviousScoringRules || configuration.mode==GameMode::Competitive)) return false;
    if(!answers.isEmpty()) {
        qint64 total=0; int correct=0,number=0;
        for(const auto &answer : answers) {
            if(answer.questionNumber!=++number || !validAnswerScoring(answer,configuration.rulesVersion)) return false;
            total+=answer.awardedUnits;correct+=answer.correct;
        }
        if(total!=scoreUnits || correct!=correctAnswers)return false;
    }
    if (configuration.mode == GameMode::Competitive && (configuration.questionLimit <= 0 || answers.size() > configuration.questionLimit
        || configuration.contentVersion.isEmpty() || configuration.rank.isEmpty())) return false;
    if (configuration.mode == GameMode::Competitive) {
        QSet<int> organisms;
        for (const auto &answer : answers) {
            if (organisms.contains(answer.speciesId) || answer.rank != configuration.rank) return false;
            organisms.insert(answer.speciesId);
        }
    }
    if(!database_.transaction())return false;
    QSqlQuery query(database_);
    const QString id=gameId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : gameId;
    const QString playedAt=QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    query.prepare("SELECT player_name,score_units,correct_answers,answered_count,ranking_key,mode,question_limit,group_id,rank,difficulty,rules_version,content_version,player_id FROM game_runs WHERE id=?");query.addBindValue(id);
    if(!query.exec()){database_.rollback();return false;}
    if(query.next()) {
        bool samePlayer=query.value(12).toString()==player.id && query.value(0).toString()==normalizedName && query.value(1).toLongLong()==scoreUnits
            && query.value(2).toInt()==correctAnswers && query.value(3).toInt()==answers.size()
            && query.value(4).toString()==configuration.rankingKey()
            && query.value(5).toString()==configuration.modeKey() && query.value(6).toInt()==configuration.questionLimit
            && query.value(7).toString()==configuration.group && query.value(8).toString()==configuration.rank
            && query.value(9).toString()==configuration.difficultyKey() && query.value(10).toString()==configuration.rulesVersion
            && query.value(11).toString()==configuration.contentVersion;
        query.finish();
        QSqlQuery stored(database_);
        stored.prepare("SELECT question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot FROM answer_records WHERE game_id=? ORDER BY question_number");
        stored.addBindValue(id);
        samePlayer = samePlayer && stored.exec();
        int index = 0;
        while (samePlayer && stored.next()) {
            if (index >= answers.size()) { samePlayer = false; break; }
            const auto &answer = answers[index++];
            const QVariantList values{answer.questionNumber,answer.speciesId,answer.scientificName,answer.photoId,answer.rank,
                answer.selectedAnswer,answer.correctAnswer,int(answer.correct),answer.revealedHints,answer.maximumUnits,answer.awardedUnits,answer.rulesVersion,answerSnapshot(answer)};
            for (int column = 0; column < values.size(); ++column) if (stored.value(column).toString() != values[column].toString()) samePlayer = false;
        }
        samePlayer = samePlayer && index == answers.size();
        stored.finish(); query.finish();
        if (!samePlayer || !finalizeSessionCheckpoint(database_,id,player,scoreUnits,correctAnswers,answers,configuration,true)
            || !database_.commit()) { database_.rollback(); return false; }
        return true;
    }
    const bool complete = configuration.mode == GameMode::Free || answers.size() == configuration.questionLimit;
    query.prepare("INSERT INTO game_runs(id,player_name,finished_at,mode,question_limit,group_id,rank,difficulty,rules_version,content_version,ranking_key,score_units,correct_answers,answered_count,is_complete,player_id) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    for (const QVariant &value : QVariantList{id,normalizedName,playedAt,configuration.modeKey(),configuration.questionLimit,
        configuration.group.isEmpty() ? QStringLiteral("") : configuration.group,configuration.rank,
        configuration.difficultyKey(),configuration.rulesVersion,
        configuration.contentVersion.isEmpty() ? QStringLiteral("") : configuration.contentVersion,
        configuration.rankingKey(),scoreUnits,correctAnswers,int(answers.size()),complete,player.id}) query.addBindValue(value);
    if(!query.exec()){database_.rollback();return false;}
    for(const auto &answer : answers) {
        query.prepare("INSERT INTO answer_records(game_id,question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        for(const QVariant &value : QVariantList{id,answer.questionNumber,answer.speciesId,answer.scientificName,
            answer.photoId,answer.rank,answer.selectedAnswer,answer.correctAnswer,answer.correct,
            answer.revealedHints,answer.maximumUnits,answer.awardedUnits,answer.rulesVersion,answerSnapshot(answer)})query.addBindValue(value);
        if(!query.exec()){database_.rollback();return false;}
    }
    if (complete && !upsertRanking(database_, configuration.rankingKey(), player, scoreUnits, correctAnswers, playedAt)) {
        database_.rollback(); return false;
    }
    if (configuration.mode == GameMode::Free) {
    query.prepare(QStringLiteral(
        "INSERT INTO history (player_id, player_name, score_units, correct_answers, played_at) "
        "VALUES (:player_id, :player_name, :score, :correct_answers, :played_at) "
        "ON CONFLICT (player_id) DO UPDATE SET "
        "player_name = excluded.player_name, score_units = excluded.score_units, "
        "correct_answers = excluded.correct_answers, played_at = excluded.played_at"));
    query.bindValue(QStringLiteral(":player_name"), normalizedName);
    query.bindValue(QStringLiteral(":player_id"), player.id);
    query.bindValue(QStringLiteral(":score"), scoreUnits);
    query.bindValue(QStringLiteral(":correct_answers"), correctAnswers);
    query.bindValue(QStringLiteral(":played_at"), playedAt);
    if (!query.exec()) { database_.rollback(); return false; }
    }
    if (!finalizeSessionCheckpoint(database_,id,player,scoreUnits,correctAnswers,answers,configuration,false) || !database_.commit()) {
        qWarning() << "Não foi possível salvar o resultado:" << query.lastError().text();
        database_.rollback();return false;
    }
    return true;
}

QVector<AnswerRecord> SqliteGameHistoryRepository::loadAnswers(const QString &gameId) const
{
    QVector<AnswerRecord> result;
    if (!database_.isOpen()) return result;
    QSqlQuery query(database_);
    query.prepare("SELECT question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot FROM answer_records WHERE game_id=? ORDER BY question_number");
    query.addBindValue(gameId);
    if (!query.exec()) return {};
    while (query.next()) {
        AnswerRecord a{query.value(0).toInt(),query.value(1).toInt(),query.value(2).toString(),query.value(3).toString(),
            query.value(4).toString(),query.value(5).toString(),query.value(6).toString(),query.value(7).toBool(),
            query.value(8).toInt(),query.value(9).toInt(),query.value(10).toInt()};
        a.rulesVersion=query.value(11).toString();
        if (a.questionNumber!=result.size()+1 || !readSnapshot(query.value(12).toString(),a)) return {};
        result.append(a);
    }
    return result;
}

QVector<HistoryEntry> SqliteGameHistoryRepository::loadTopResults(int limit) const
{
    return loadRanking("free", limit);
}

QVector<HistoryEntry> SqliteGameHistoryRepository::loadRanking(const QString &key, int limit) const
{
    return queryResults(key, limit, {});
}
QVector<HistoryEntry> SqliteGameHistoryRepository::loadPlayerResult(const QString &key, const QString &playerId) const
{
    return playerId.isEmpty() ? QVector<HistoryEntry>{} : queryResults(key, 1, playerId);
}
QVector<HistoryEntry> SqliteGameHistoryRepository::queryResults(const QString &key, int limit, const QString &playerId) const
{
    QVector<HistoryEntry> results;
    if (!database_.isOpen()) {
        return results;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("SELECT CASE WHEN p.origin='legacy_unresolved' THEN r.player_name ELSE p.name END AS display_name,r.best_units,r.best_correct,r.best_played_at,r.last_units,r.last_correct,r.last_played_at,r.player_id,r.player_name,p.origin,p.deleted_at FROM ranking_records r JOIN player_profiles p ON p.id=r.player_id WHERE r.ranking_key=:key AND (:id='' OR r.player_id=:id) ORDER BY r.best_units DESC,r.best_correct DESC,r.best_played_at ASC,display_name COLLATE NOCASE ASC,display_name ASC,r.player_id ASC LIMIT :limit"));
    query.bindValue(":key", key);
    query.bindValue(":id", playerId.isEmpty() ? QStringLiteral("") : playerId);
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return results;
    }

    while (query.next()) {
        HistoryEntry entry;
        entry.playerName = query.value(0).toString();
        entry.scoreUnits = query.value(1).toLongLong();
        entry.correctAnswers = query.value(2).toInt();
        entry.playedAt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        entry.lastScoreUnits=query.value(4).toLongLong();
        entry.lastCorrectAnswers=query.value(5).toInt();
        entry.lastPlayedAt=QDateTime::fromString(query.value(6).toString(),Qt::ISODate);
        entry.playerId=query.value(7).toString(); entry.snapshotName=query.value(8).toString();
        entry.legacyIdentity=query.value(9).toString()=="legacy_unresolved";
        entry.deletedProfile=!query.value(10).isNull();
        results.push_back(entry);
    }

    return results;
}

SessionMetrics SqliteGameHistoryRepository::playerMetrics(const QString &key, const QString &playerId) const
{
    SessionMetrics metrics;
    if (!database_.isOpen() || playerId.isEmpty()) return metrics;
    QSqlQuery query(database_);
    query.prepare("SELECT id FROM game_runs WHERE player_id=:id AND ranking_key=:key AND is_complete=1");
    query.bindValue(":id",playerId); query.bindValue(":key",key);
    if (!query.exec()) return metrics;
    QStringList games;while(query.next())games.append(query.value(0).toString());query.finish();
    for(const auto &id:games)for(const auto &answer:loadAnswers(id))metrics.record(answer);
    return metrics;
}

QStringList SqliteGameHistoryRepository::loadPlayerProfiles() const
{
    QStringList profiles;
    for (const auto &profile : this->profiles()) profiles.append(profile.displayName);
    return profiles;
}

bool SqliteGameHistoryRepository::addPlayerProfile(const QString &name)
{
    return createPlayer(name).selectable();
}

bool SqliteGameHistoryRepository::renamePlayerProfile(const QString &currentName, const QString &newName)
{
    return renamePlayer(profileNamed(currentName).id, newName);
}

bool SqliteGameHistoryRepository::deletePlayerProfile(const QString &name)
{
    return deletePlayer(profileNamed(name).id);
}

QVector<PlayerProfile> SqliteGameHistoryRepository::profiles() const
{
    QVector<PlayerProfile> result;
    if (!database_.isOpen()) return result;
    QSqlQuery q(database_);
    if (!q.exec("SELECT id,name FROM player_profiles WHERE origin='profile' AND deleted_at IS NULL ORDER BY name COLLATE NOCASE,name,id")) return result;
    while (q.next()) result.append({q.value(0).toString(),q.value(1).toString(),false,false});
    return result;
}

PlayerProfile SqliteGameHistoryRepository::player(const QString &id) const
{
    if (!database_.isOpen() || id.isEmpty()) return {};
    QSqlQuery q(database_); q.prepare("SELECT id,name,deleted_at,origin FROM player_profiles WHERE id=?"); q.addBindValue(id);
    if (!q.exec() || !q.next()) return {};
    return {q.value(0).toString(),q.value(1).toString(),!q.value(2).isNull(),q.value(3).toString()=="legacy_unresolved"};
}

PlayerProfile SqliteGameHistoryRepository::profileNamed(const QString &name) const
{
    const auto active = profiles();
    PlayerProfile selected;
    for (const auto &p : active) if (p.displayName == PlayerNamePolicy::displayName(name)) {
        if (!selected.id.isEmpty()) return {};
        selected=p;
    }
    if (!selected.id.isEmpty()) return selected;
    for (const auto &p : active) if (PlayerNamePolicy::key(p.displayName) == PlayerNamePolicy::key(name)) {
        if (!selected.id.isEmpty()) return {}; // A Unicode collision from an old DB is not a merge.
        selected = p;
    }
    return selected;
}

PlayerProfile SqliteGameHistoryRepository::createPlayer(const QString &name)
{
    const QString display = PlayerNamePolicy::displayName(name);
    if (!database_.isOpen() || !PlayerNamePolicy::validDisplayName(display)) return {};
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery q(database_); q.prepare("INSERT INTO player_profiles(id,name,name_key,origin) VALUES(?,?,?,'profile')");
    q.addBindValue(id); q.addBindValue(display); q.addBindValue(PlayerNamePolicy::key(display));
    return q.exec() ? player(id) : PlayerProfile{};
}

bool SqliteGameHistoryRepository::renamePlayer(const QString &id, const QString &name)
{
    const QString display = PlayerNamePolicy::displayName(name);
    const auto current = player(id);
    if (!current.selectable() || !PlayerNamePolicy::validDisplayName(display)) return false;
    if (display==current.displayName) return true;
    QSqlQuery q(database_); q.prepare("UPDATE player_profiles SET name=?,name_key=? WHERE id=? AND deleted_at IS NULL AND origin='profile'");
    q.addBindValue(display); q.addBindValue(PlayerNamePolicy::key(display)); q.addBindValue(id);
    return q.exec() && q.numRowsAffected()==1;
}

bool SqliteGameHistoryRepository::deletePlayer(const QString &id)
{
    if (!database_.isOpen() || id.isEmpty()) return false;
    QSqlQuery q(database_); q.prepare("UPDATE player_profiles SET deleted_at=? WHERE id=? AND origin='profile' AND deleted_at IS NULL");
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)); q.addBindValue(id);
    return q.exec() && q.numRowsAffected()==1;
}

int SqliteGameHistoryRepository::unresolvedLegacyPlayers() const
{
    if (!database_.isOpen()) return 0;
    QSqlQuery q(database_);
    return q.exec("SELECT COUNT(*) FROM player_profiles WHERE origin='legacy_unresolved'") && q.next() ? q.value(0).toInt() : 0;
}

PlayerProfile SqliteGameHistoryRepository::localUser() const
{
    if (!database_.isOpen()) return {};
    QSqlQuery q(database_);
    if (!q.exec("SELECT player_id FROM local_user_binding WHERE id=1") || !q.next()) return {};
    return player(q.value(0).toString());
}

bool SqliteGameHistoryRepository::localUserChoiceRequired() const
{
    return database_.isOpen() && localUser().id.isEmpty() && profiles().size()>1;
}

bool SqliteGameHistoryRepository::initializeLocalUser()
{
    if (!database_.isOpen()) return false;
    QSqlQuery q(database_);
    if (!q.exec("BEGIN IMMEDIATE")) return false;
    const auto existing=localUser();
    if (!existing.id.isEmpty()) { database_.rollback(); return existing.selectable(); }
    const auto candidates=profiles();
    if (candidates.size()>1) { database_.rollback(); return false; }
    const auto chosen=candidates.isEmpty() ? createPlayer(QStringLiteral("Jogador 1")) : candidates.first();
    if (!chosen.selectable()) { database_.rollback(); return false; }
    q.prepare("INSERT INTO local_user_binding VALUES(1,?,?,?)");
    q.addBindValue(chosen.id); q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    q.addBindValue(candidates.isEmpty() ? "new_installation" : "single_existing");
    if (!q.exec() || !database_.commit()) { database_.rollback(); return false; }
    return true;
}

bool SqliteGameHistoryRepository::chooseLocalUser(const QString &id)
{
    if (!database_.isOpen()) return false;
    QSqlQuery q(database_);
    if (!q.exec("BEGIN IMMEDIATE")) return false;
    const auto existing=localUser();
    if (!existing.id.isEmpty()) { database_.rollback(); return existing.selectable() && existing.id==id; }
    if (!player(id).selectable()) { database_.rollback(); return false; }
    q.prepare("INSERT INTO local_user_binding VALUES(1,?,?,'explicit_transition')");
    q.addBindValue(id); q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!q.exec() || !database_.commit()) { database_.rollback(); return false; }
    return true;
}
