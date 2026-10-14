#include "infrastructure/persistence/SqliteGameHistoryRepository.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QUuid>

namespace {
int connectionCounter = 0;

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
    if(!query.exec("PRAGMA foreign_keys=ON")){database_.close();return;}
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, player_name TEXT NOT NULL, score INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)"))
        || !query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS player_profiles (name TEXT PRIMARY KEY COLLATE NOCASE)"))) {
        qWarning() << "Não foi possível inicializar o banco:" << query.lastError().text();
        database_.close();
        return;
    }

    if (!query.exec(QStringLiteral("SELECT 1 FROM sqlite_master WHERE type = 'index' AND name = 'history_player_name_unique'"))) {
        qWarning() << "Não foi possível verificar a migração do ranking:" << query.lastError().text();
        database_.close();
        return;
    }
    const bool rankingMigrated = query.next();
    query.finish();
    // One transaction covers the existing uniqueness rule and the score scale.
    // The score_units column is the migration marker: reopening never scales twice.
    if (!database_.transaction()) {
        qWarning() << "Não foi possível iniciar a migração do ranking:" << database_.lastError().text();
        database_.close();
        return;
    }
    bool ok=true;
    if(!rankingMigrated) ok=query.exec(QStringLiteral(
            "DELETE FROM history WHERE id NOT IN ("
            "SELECT current.id FROM history AS current WHERE NOT EXISTS ("
            "SELECT 1 FROM history AS newer "
            "WHERE newer.player_name = current.player_name COLLATE NOCASE "
            "AND (newer.played_at > current.played_at "
            "OR (newer.played_at = current.played_at AND newer.id > current.id))))"))
        && query.exec(QStringLiteral(
            "CREATE UNIQUE INDEX IF NOT EXISTS history_player_name_unique "
            "ON history (player_name COLLATE NOCASE)"));
    bool scaled=false;
    ok=ok && query.exec("PRAGMA table_info(history)");
    while(query.next()) if(query.value(1).toString()=="score_units")scaled=true;
    query.finish();
    if(ok && !scaled) ok=query.exec("ALTER TABLE history RENAME COLUMN score TO score_units")
        && query.exec("UPDATE history SET score_units=score_units*6");
    ok=ok && query.exec("CREATE TABLE IF NOT EXISTS game_runs (id TEXT PRIMARY KEY, player_name TEXT NOT NULL, finished_at TEXT NOT NULL)")
        && query.exec("CREATE TABLE IF NOT EXISTS answer_records (game_id TEXT NOT NULL REFERENCES game_runs(id), question_number INTEGER NOT NULL, species_id INTEGER NOT NULL, scientific_name TEXT NOT NULL, photo_id TEXT NOT NULL, rank TEXT NOT NULL, selected_answer TEXT NOT NULL, correct_answer TEXT NOT NULL, is_correct INTEGER NOT NULL CHECK(is_correct IN (0,1)), revealed_hints INTEGER NOT NULL CHECK(revealed_hints BETWEEN 0 AND 7), maximum_units INTEGER NOT NULL CHECK(maximum_units IN (2,3,6)), awarded_units INTEGER NOT NULL CHECK(awarded_units IN (0,2,3,6)), PRIMARY KEY(game_id,question_number), CHECK(awarded_units=CASE WHEN is_correct=1 THEN maximum_units ELSE 0 END))");
    if (!ok || !database_.commit()) {
        qWarning() << "Não foi possível migrar o ranking:" << query.lastError().text() << database_.lastError().text();
        database_.rollback();
        database_.close();
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
    const QString normalizedName = playerName.trimmed();
    if (!database_.isOpen() || normalizedName.isEmpty() || scoreUnits<0 || correctAnswers<0) {
        return false;
    }
    if(!answers.isEmpty()) {
        qint64 total=0; int correct=0,number=0;
        for(const auto &answer : answers) {
            if(answer.questionNumber!=++number || answer.maximumUnits!=maximumScoreUnits(answer.revealedHints)
                || answer.awardedUnits!=(answer.correct ? answer.maximumUnits : 0)
                || answer.revealedHints<0 || answer.revealedHints>7)return false;
            total+=answer.awardedUnits;correct+=answer.correct;
        }
        if(total!=scoreUnits || correct!=correctAnswers)return false;
    }
    if(!database_.transaction())return false;
    QSqlQuery query(database_);
    const QString id=gameId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : gameId;
    const QString playedAt=QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    query.prepare("SELECT player_name FROM game_runs WHERE id=?");query.addBindValue(id);
    if(!query.exec()){database_.rollback();return false;}
    if(query.next()) {
        const bool samePlayer=query.value(0).toString()==normalizedName;
        query.finish();database_.rollback();return samePlayer;
    }
    query.prepare("INSERT INTO game_runs(id,player_name,finished_at) VALUES (?,?,?)");
    query.addBindValue(id);query.addBindValue(normalizedName);query.addBindValue(playedAt);
    if(!query.exec()){database_.rollback();return false;}
    for(const auto &answer : answers) {
        query.prepare("INSERT INTO answer_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");
        for(const QVariant &value : QVariantList{id,answer.questionNumber,answer.speciesId,answer.scientificName,
            answer.photoId,answer.rank,answer.selectedAnswer,answer.correctAnswer,answer.correct,
            answer.revealedHints,answer.maximumUnits,answer.awardedUnits})query.addBindValue(value);
        if(!query.exec()){database_.rollback();return false;}
    }
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
    if (!query.exec() || !database_.commit()) {
        qWarning() << "Não foi possível salvar o resultado:" << query.lastError().text();
        database_.rollback();return false;
    }
    return true;
}

QVector<HistoryEntry> SqliteGameHistoryRepository::loadTopResults(int limit) const
{
    QVector<HistoryEntry> results;
    if (!database_.isOpen()) {
        return results;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral("SELECT player_name, score_units, correct_answers, played_at FROM history ORDER BY score_units DESC, played_at DESC LIMIT :limit"));
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
        results.push_back(entry);
    }

    return results;
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
