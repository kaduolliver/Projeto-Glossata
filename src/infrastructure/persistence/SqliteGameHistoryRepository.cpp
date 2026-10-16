#include "infrastructure/persistence/SqliteGameHistoryRepository.h"

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

namespace {
int connectionCounter = 0;

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

bool upsertRanking(const QSqlDatabase &database, const QString &key, const QString &name,
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
    if (!query.exec("PRAGMA foreign_keys=ON") || !query.exec("PRAGMA user_version") || !query.next()) {
        database_.close(); return;
    }
    const int version = query.value(0).toInt();
    query.finish();
    if (version > 3 || !database_.transaction()) { database_.close(); return; }
    auto exec = [&](const QString &sql) { return query.exec(sql); };
    auto columns = [&](const QString &table) {
        QSet<QString> result;
        query.exec("PRAGMA table_info(" + table + ")");
        while (query.next()) result.insert(query.value(1).toString());
        query.finish(); return result;
    };
    bool ok = exec("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, player_name TEXT NOT NULL, score INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)")
        && exec("CREATE TABLE IF NOT EXISTS player_profiles (name TEXT PRIMARY KEY COLLATE NOCASE)");
    if (ok && !columns("history").contains("score_units"))
        ok = exec("ALTER TABLE history RENAME COLUMN score TO score_units") && exec("UPDATE history SET score_units=score_units*6");
    // Preserve even obsolete duplicate rows before retaining the existing unique-name policy.
    if (ok && version < 2) ok = exec("CREATE TABLE IF NOT EXISTS legacy_history_results (original_id INTEGER PRIMARY KEY, player_name TEXT NOT NULL, score_units INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)")
        && exec("INSERT OR IGNORE INTO legacy_history_results SELECT id,player_name,score_units,correct_answers,played_at FROM history");
    ok = ok && exec("DELETE FROM history WHERE id NOT IN (SELECT current.id FROM history current WHERE NOT EXISTS (SELECT 1 FROM history newer WHERE newer.player_name=current.player_name COLLATE NOCASE AND (newer.played_at>current.played_at OR (newer.played_at=current.played_at AND newer.id>current.id))))")
        && exec("CREATE UNIQUE INDEX IF NOT EXISTS history_player_name_unique ON history(player_name COLLATE NOCASE)")
        && exec("CREATE TABLE IF NOT EXISTS game_runs (id TEXT PRIMARY KEY, player_name TEXT NOT NULL, finished_at TEXT NOT NULL)")
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
        for (const auto &row : available) if (ok) ok = upsertRanking(database_, "free", row[0].toString(),row[1].toLongLong(),row[2].toInt(),row[3].toString());
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
    if (ok) {
        ok = exec("PRAGMA foreign_key_check");
        if (query.next()) ok = false;
        query.finish();
    }
    ok = ok && exec("PRAGMA user_version=3");
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

bool SqliteGameHistoryRepository::saveResult(const QString &playerName, qint64 scoreUnits, int correctAnswers,
                                            const QVector<AnswerRecord> &answers,const QString &gameId)
{
    GameConfiguration configuration;
    configuration.rulesVersion=answers.isEmpty() ? QStringLiteral("legacy") : answers.first().rulesVersion;
    return saveConfiguredResult(playerName, scoreUnits, correctAnswers, answers, gameId, configuration);
}

bool SqliteGameHistoryRepository::saveConfiguredResult(const QString &playerName, qint64 scoreUnits, int correctAnswers,
    const QVector<AnswerRecord> &answers, const QString &gameId, const GameConfiguration &configuration)
{
    const QString normalizedName = playerName.trimmed();
    if (!database_.isOpen() || normalizedName.isEmpty() || scoreUnits<0 || correctAnswers<0) {
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
    query.prepare("SELECT player_name,score_units,correct_answers,answered_count,ranking_key,mode,question_limit,group_id,rank,difficulty,rules_version,content_version FROM game_runs WHERE id=?");query.addBindValue(id);
    if(!query.exec()){database_.rollback();return false;}
    if(query.next()) {
        bool samePlayer=query.value(0).toString()==normalizedName && query.value(1).toLongLong()==scoreUnits
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
        query.finish();database_.rollback();return samePlayer;
    }
    const bool complete = configuration.mode == GameMode::Free || answers.size() == configuration.questionLimit;
    query.prepare("INSERT INTO game_runs(id,player_name,finished_at,mode,question_limit,group_id,rank,difficulty,rules_version,content_version,ranking_key,score_units,correct_answers,answered_count,is_complete) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    for (const QVariant &value : QVariantList{id,normalizedName,playedAt,configuration.modeKey(),configuration.questionLimit,
        configuration.group.isEmpty() ? QStringLiteral("") : configuration.group,configuration.rank,
        configuration.difficultyKey(),configuration.rulesVersion,
        configuration.contentVersion.isEmpty() ? QStringLiteral("") : configuration.contentVersion,
        configuration.rankingKey(),scoreUnits,correctAnswers,int(answers.size()),complete}) query.addBindValue(value);
    if(!query.exec()){database_.rollback();return false;}
    for(const auto &answer : answers) {
        query.prepare("INSERT INTO answer_records(game_id,question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        for(const QVariant &value : QVariantList{id,answer.questionNumber,answer.speciesId,answer.scientificName,
            answer.photoId,answer.rank,answer.selectedAnswer,answer.correctAnswer,answer.correct,
            answer.revealedHints,answer.maximumUnits,answer.awardedUnits,answer.rulesVersion,answerSnapshot(answer)})query.addBindValue(value);
        if(!query.exec()){database_.rollback();return false;}
    }
    if (complete && !upsertRanking(database_, configuration.rankingKey(), normalizedName, scoreUnits, correctAnswers, playedAt)) {
        database_.rollback(); return false;
    }
    if (configuration.mode == GameMode::Free) {
    query.prepare(QStringLiteral(
        "INSERT INTO history (player_name, score_units, correct_answers, played_at) "
        "VALUES (:player_name, :score, :correct_answers, :played_at) "
        "ON CONFLICT (player_name COLLATE NOCASE) DO UPDATE SET "
        "player_name = excluded.player_name, score_units = excluded.score_units, "
        "correct_answers = excluded.correct_answers, played_at = excluded.played_at"));
    query.bindValue(QStringLiteral(":player_name"), normalizedName);
    query.bindValue(QStringLiteral(":score"), scoreUnits);
    query.bindValue(QStringLiteral(":correct_answers"), correctAnswers);
    query.bindValue(QStringLiteral(":played_at"), playedAt);
    if (!query.exec()) { database_.rollback(); return false; }
    }
    if (!database_.commit()) {
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
QVector<HistoryEntry> SqliteGameHistoryRepository::loadPlayerResult(const QString &key, const QString &name) const
{
    return name.trimmed().isEmpty() ? QVector<HistoryEntry>{} : queryResults(key, 1, name.trimmed());
}
QVector<HistoryEntry> SqliteGameHistoryRepository::queryResults(const QString &key, int limit, const QString &name) const
{
    QVector<HistoryEntry> results;
    if (!database_.isOpen()) {
        return results;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("SELECT player_name,best_units,best_correct,best_played_at,last_units,last_correct,last_played_at FROM ranking_records WHERE ranking_key=:key AND (:name='' OR player_name=:name COLLATE NOCASE) ORDER BY best_units DESC,best_correct DESC,best_played_at ASC,player_name COLLATE NOCASE ASC,player_name ASC LIMIT :limit"));
    query.bindValue(":key", key);
    query.bindValue(":name", name.isEmpty() ? QStringLiteral("") : name);
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
        results.push_back(entry);
    }

    return results;
}

SessionMetrics SqliteGameHistoryRepository::playerMetrics(const QString &key, const QString &name) const
{
    SessionMetrics metrics;
    if (!database_.isOpen() || name.trimmed().isEmpty()) return metrics;
    QSqlQuery query(database_);
    query.prepare("SELECT id FROM game_runs WHERE player_name=:name COLLATE NOCASE AND ranking_key=:key AND is_complete=1");
    query.bindValue(":name",name.trimmed()); query.bindValue(":key",key);
    if (!query.exec()) return metrics;
    QStringList games;while(query.next())games.append(query.value(0).toString());query.finish();
    for(const auto &id:games)for(const auto &answer:loadAnswers(id))metrics.record(answer);
    return metrics;
}

QStringList SqliteGameHistoryRepository::loadPlayerProfiles() const
{
    QStringList profiles;
    if (!database_.isOpen()) {
        return profiles;
    }

    QSqlQuery query(database_);
    if (!query.exec(QStringLiteral("SELECT name FROM player_profiles ORDER BY name COLLATE NOCASE"))) {
        return profiles;
    }

    while (query.next()) {
        profiles << query.value(0).toString();
    }
    return profiles;
}

bool SqliteGameHistoryRepository::addPlayerProfile(const QString &name)
{
    const QString profileName = name.trimmed();
    if (!database_.isOpen() || profileName.isEmpty()) {
        return false;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("INSERT INTO player_profiles (name) VALUES (:name)"));
    query.bindValue(QStringLiteral(":name"), profileName);
    return query.exec();
}

bool SqliteGameHistoryRepository::renamePlayerProfile(const QString &currentName, const QString &newName)
{
    const QString profileName = currentName.trimmed();
    const QString newProfileName = newName.trimmed();
    if (!database_.isOpen() || profileName.isEmpty() || newProfileName.isEmpty()) {
        return false;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("UPDATE player_profiles SET name = :new_name WHERE name = :current_name"));
    query.bindValue(QStringLiteral(":new_name"), newProfileName);
    query.bindValue(QStringLiteral(":current_name"), profileName);
    return query.exec() && query.numRowsAffected() == 1;
}

bool SqliteGameHistoryRepository::deletePlayerProfile(const QString &name)
{
    const QString profileName = name.trimmed();
    if (!database_.isOpen() || profileName.isEmpty()) {
        return false;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("DELETE FROM player_profiles WHERE name = :name"));
    query.bindValue(QStringLiteral(":name"), profileName);
    return query.exec() && query.numRowsAffected() == 1;
}
