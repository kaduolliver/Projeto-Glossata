#include "infrastructure/persistence/SqliteGameHistoryRepository.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>

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
    if (rankingMigrated) {
        return;
    }

    // Migrate existing rankings atomically, retaining the latest game per name.
    if (!database_.transaction()) {
        qWarning() << "Não foi possível iniciar a migração do ranking:" << database_.lastError().text();
        database_.close();
        return;
    }
    if (!query.exec(QStringLiteral(
            "DELETE FROM history WHERE id NOT IN ("
            "SELECT current.id FROM history AS current WHERE NOT EXISTS ("
            "SELECT 1 FROM history AS newer "
            "WHERE newer.player_name = current.player_name COLLATE NOCASE "
            "AND (newer.played_at > current.played_at "
            "OR (newer.played_at = current.played_at AND newer.id > current.id))))"))
        || !query.exec(QStringLiteral(
            "CREATE UNIQUE INDEX IF NOT EXISTS history_player_name_unique "
            "ON history (player_name COLLATE NOCASE)"))
        || !database_.commit()) {
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

bool SqliteGameHistoryRepository::saveResult(const QString &playerName, int score, int correctAnswers)
{
    const QString normalizedName = playerName.trimmed();
    if (!database_.isOpen() || normalizedName.isEmpty()) {
        return false;
    }

    QSqlQuery query(database_);
    query.prepare(QStringLiteral(
        "INSERT INTO history (player_name, score, correct_answers, played_at) "
        "VALUES (:player_name, :score, :correct_answers, :played_at) "
        "ON CONFLICT (player_name COLLATE NOCASE) DO UPDATE SET "
        "player_name = excluded.player_name, score = excluded.score, "
        "correct_answers = excluded.correct_answers, played_at = excluded.played_at"));
    query.bindValue(QStringLiteral(":player_name"), normalizedName);
    query.bindValue(QStringLiteral(":score"), score);
    query.bindValue(QStringLiteral(":correct_answers"), correctAnswers);
    query.bindValue(QStringLiteral(":played_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (!query.exec()) {
        qWarning() << "Não foi possível salvar o resultado:" << query.lastError().text();
        return false;
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
    query.prepare(QStringLiteral("SELECT player_name, score, correct_answers, played_at FROM history ORDER BY score DESC, played_at DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return results;
    }

    while (query.next()) {
        HistoryEntry entry;
        entry.playerName = query.value(0).toString();
        entry.score = query.value(1).toInt();
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
