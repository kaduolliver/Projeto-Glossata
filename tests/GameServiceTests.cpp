#include "application/GameService.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include "presentation/GameController.h"

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSqlQuery>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>
#include <algorithm>
#include "application/TaxonomyIndex.h"

namespace {
class MemoryCatalog final : public SpeciesCatalog {
public:
    QVector<Species> items;
    QVector<Species> loadSpecies() const override { return items; }
};

class FailingHistory final : public GameHistoryRepository {
public:
    bool fail = true;
    int attempts = 0;
    int saved = 0;
    bool saveResult(const QString &, int, int) override {
        ++attempts;
        if (fail) return false;
        ++saved;
        return true;
    }
    QVector<HistoryEntry> loadTopResults(int) const override { return {}; }
};
}

class GameServiceTests : public QObject {
    Q_OBJECT

private slots:
    void initialLoad_hasSpecies();
    void gameStartsAtFirstRound();
    void extraHintCanBeRevealedOncePerRound();
    void allRoundsHaveFourDistinctChoices();
    void completedGameIsSavedOnce();
    void playerProfilesPersistAndCanBeManaged();
    void invalidCatalogCannotStartGame();
    void jsonCatalogRejectsInvalidContent();
    void incorrectAndInvalidAnswers();
    void emptyGameIsNotSaved();
    void failedSaveCanBeRetried();
    void gameKeepsStartingPlayer();
    void controllerCanRetrySqliteFailure();
    void rankingUpdatesExistingPlayer();
    void legacyRankingKeepsLatestResultPerPlayer();
    void failedRankingUpdateKeepsPreviousResult();
    void categoryQuizUsesOnlySelectedFamily();
    void controllerKeepsAnswerUntilNextQuestion();
    void taxonomyFiltersAndIdentificationLevels();
    void offlinePhotosHaveAttributionAndDecode();
    void catalogMigrationPreservesPlayerDataAndStoredRecords();
    void invalidPhotoLicenseDisablesVisualQuestion();
};

void GameServiceTests::initialLoad_hasSpecies()
{
    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    const QVector<Species> species = catalog.loadSpecies();
    QVERIFY(species.size() >= 20);
    for (const Species &item : species) {
        QVERIFY(!item.name.trimmed().isEmpty());
        QVERIFY(!item.fact.trimmed().isEmpty());
        QVERIFY(!item.hint.trimmed().isEmpty());
    }
}

void GameServiceTests::gameStartsAtFirstRound()
{
    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    GameService service(&catalog);
    service.startNewGame();

    QCOMPARE(service.round(), 1);
    QVERIFY(!service.isFinished());
}

void GameServiceTests::extraHintCanBeRevealedOncePerRound()
{
    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    GameService service(&catalog);
    service.startNewGame();

    QVERIFY(!service.hintRevealed());
    QVERIFY(service.extraHint().isEmpty());
    QVERIFY(service.revealHint());
    QVERIFY(service.hintRevealed());
    QVERIFY(!service.extraHint().isEmpty());
    QVERIFY(!service.revealHint());

    QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
    QVERIFY(!service.hintRevealed());
    QVERIFY(service.extraHint().isEmpty());
}

void GameServiceTests::allRoundsHaveFourDistinctChoices()
{
    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    GameService service(&catalog);
    service.startNewGame();

    const int speciesCount = catalog.loadSpecies().size();
    QStringList askedSpecies;
    for (int round = 1; round <= speciesCount; ++round) {
        const Question question = service.currentQuestion();
        QCOMPARE(service.round(), round);
        QCOMPARE(question.options.size(), 4);
        QVERIFY(question.options.contains(question.correctAnswer));
        QVERIFY(!question.fact.trimmed().isEmpty());
        QVERIFY(!askedSpecies.contains(question.correctAnswer, Qt::CaseInsensitive));
        askedSpecies << question.correctAnswer;

        QStringList uniqueOptions;
        for (const QString &option : question.options) {
            QVERIFY(!option.trimmed().isEmpty());
            QVERIFY(!uniqueOptions.contains(option, Qt::CaseInsensitive));
            uniqueOptions << option;
        }

        QVERIFY(service.submitAnswer(question.correctAnswer));
        QCOMPARE(service.score(), round);
        QVERIFY(!service.isFinished());
    }

    QCOMPARE(askedSpecies.size(), speciesCount);
    QCOMPARE(service.round(), speciesCount + 1);
    QVERIFY(!service.currentQuestion().correctAnswer.isEmpty());
    QVERIFY(service.currentQuestion().correctAnswer != askedSpecies.last());

    service.finishGame();
    QVERIFY(service.isFinished());
    QVERIFY(service.currentQuestion().options.isEmpty());
    const int completedScore = service.score();
    QVERIFY(!service.submitAnswer(service.currentQuestion().correctAnswer));
    QCOMPARE(service.score(), completedScore);
}

void GameServiceTests::completedGameIsSavedOnce()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    SqliteGameHistoryRepository history(temporaryDirectory.filePath(QStringLiteral("nested/history.sqlite")));
    GameService service(&catalog, &history);
    service.startNewGame();

    for (int answer = 0; answer < 2; ++answer) {
        QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
    }
    service.finishGame();

    service.saveCurrentResult(QStringLiteral("Test Player"));
    service.saveCurrentResult(QStringLiteral("Test Player"));

    const QVector<HistoryEntry> entries = history.loadTopResults(10);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.front().playerName, QStringLiteral("Test Player"));
    QCOMPARE(entries.front().score, 2);
}

void GameServiceTests::playerProfilesPersistAndCanBeManaged()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    SqliteGameHistoryRepository profiles(temporaryDirectory.filePath(QStringLiteral("profiles.sqlite")));
    QVERIFY(profiles.addPlayerProfile(QStringLiteral(" Ada ")));
    QVERIFY(profiles.addPlayerProfile(QStringLiteral("Lia")));
    QVERIFY(!profiles.addPlayerProfile(QStringLiteral("ada")));

    QCOMPARE(profiles.loadPlayerProfiles(), (QStringList{QStringLiteral("Ada"), QStringLiteral("Lia")}));
    QVERIFY(profiles.renamePlayerProfile(QStringLiteral("Ada"), QStringLiteral("Ada Lovelace")));
    QVERIFY(!profiles.renamePlayerProfile(QStringLiteral("Ada Lovelace"), QStringLiteral("Lia")));
    QVERIFY(profiles.deletePlayerProfile(QStringLiteral("Lia")));

    QCOMPARE(profiles.loadPlayerProfiles(), (QStringList{QStringLiteral("Ada Lovelace")}));
    SqliteGameHistoryRepository reopened(temporaryDirectory.filePath(QStringLiteral("profiles.sqlite")));
    QCOMPARE(reopened.loadPlayerProfiles(), (QStringList{QStringLiteral("Ada Lovelace")}));
}

void GameServiceTests::invalidCatalogCannotStartGame()
{
    MemoryCatalog catalog;
    catalog.items = {
        {1, "One", "Fact", "Hint"},
        {2, "Two", "Fact", "Hint"},
        {3, "Three", "Fact", "Hint"},
        {4, " one ", "Fact", "Hint"},
        {5, "Four", "Fact", "   "},
        {6, "Five", "  ", "Hint"}
    };
    GameService service(&catalog);
    service.startNewGame();
    QVERIFY(service.isFinished());
    QCOMPARE(service.round(), 0);
    QVERIFY(service.currentQuestion().options.isEmpty());
    QVERIFY(!service.revealHint());
    QVERIFY(!service.submitAnswer("One"));
    catalog.items.push_back({7, "Four", "Fact", "Hint"});
    service.startNewGame();
    QVERIFY(!service.isFinished());
    QCOMPARE(service.currentQuestion().options.size(), 4);
    GameService missingCatalog;
    missingCatalog.startNewGame();
    QVERIFY(missingCatalog.isFinished());
}

void GameServiceTests::jsonCatalogRejectsInvalidContent()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("species.json");
    JsonSpeciesCatalog catalog(path);
    QVERIFY(catalog.loadSpecies().isEmpty());
    const QList<QByteArray> invalidDocuments{"{", "{}", "[null, {}, {\"name\":\"A\",\"fact\":\"B\",\"hint\":\"   \"}]"};
    for (const QByteArray &document : invalidDocuments) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(document), document.size());
        file.close();
        QVERIFY(catalog.loadSpecies().isEmpty());
    }
}

void GameServiceTests::incorrectAndInvalidAnswers()
{
    JsonSpeciesCatalog catalog;
    GameService service(&catalog);
    service.startNewGame();
    const Question question = service.currentQuestion();
    QVERIFY(!service.submitAnswer("Unknown"));
    QCOMPARE(service.round(), 1);
    QCOMPARE(service.currentQuestion().correctAnswer, question.correctAnswer);
    QString wrong;
    for (const QString &option : question.options) {
        if (option != question.correctAnswer) { wrong = option; break; }
    }
    QVERIFY(!wrong.isEmpty());
    QVERIFY(!service.submitAnswer(wrong));
    QCOMPARE(service.score(), 0);
    QCOMPARE(service.round(), 2);
    QVERIFY(!service.isFinished());
}

void GameServiceTests::emptyGameIsNotSaved()
{
    JsonSpeciesCatalog catalog;
    FailingHistory history;
    GameService service(&catalog, &history);
    service.startNewGame();
    QCOMPARE(service.saveCurrentResult("Player"), GameService::SaveStatus::NotNeeded);
    service.finishGame();
    QCOMPARE(service.saveCurrentResult("Player"), GameService::SaveStatus::NotNeeded);
    QCOMPARE(history.attempts, 0);
}

void GameServiceTests::failedSaveCanBeRetried()
{
    JsonSpeciesCatalog catalog;
    FailingHistory history;
    GameService service(&catalog, &history);
    service.startNewGame();
    QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
    service.finishGame();
    QCOMPARE(service.saveCurrentResult("Player"), GameService::SaveStatus::Failed);
    history.fail = false;
    QCOMPARE(service.saveCurrentResult("Player"), GameService::SaveStatus::Saved);
    QCOMPARE(service.saveCurrentResult("Player"), GameService::SaveStatus::NotNeeded);
    QCOMPARE(history.attempts, 2);
    QCOMPARE(history.saved, 1);
}

void GameServiceTests::gameKeepsStartingPlayer()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("profiles.sqlite"));
    const QString startingPlayer = controller.currentPlayerName();
    controller.answerSelected(controller.answerOptions().first());
    QVERIFY(controller.addPlayerProfile("Ada"));
    QCOMPARE(controller.currentPlayerName(), QString("Ada"));
    QCOMPARE(controller.sessionPlayerName(), startingPlayer);
    QVERIFY(controller.renameCurrentPlayerProfile("Lia"));
    controller.stopGame();
    QCOMPARE(controller.ranking().size(), 1);
    QCOMPARE(controller.ranking().first().toMap().value("playerName").toString(), startingPlayer);
    controller.startGame();
    QCOMPARE(controller.sessionPlayerName(), QString("Lia"));
    controller.answerSelected(controller.answerOptions().first());
    QVERIFY(controller.deleteCurrentPlayerProfile());
    controller.stopGame();
    bool foundLia = false;
    for (const QVariant &row : controller.ranking()) {
        foundLia |= row.toMap().value("playerName").toString() == "Lia";
    }
    QVERIFY(foundLia);
}

void GameServiceTests::controllerCanRetrySqliteFailure()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("history.sqlite");
    GameController controller(nullptr, path);
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "failure_test");
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec("CREATE TRIGGER reject_result BEFORE INSERT ON history BEGIN SELECT RAISE(FAIL, 'test failure'); END"));
        controller.answerSelected(controller.answerOptions().first());
        const int score = controller.score();
        controller.stopGame();
        QVERIFY(controller.resultPending());
        QVERIFY(controller.ranking().isEmpty());
        controller.startGame();
        QVERIFY(controller.gameFinished());
        QCOMPARE(controller.score(), score);
        QVERIFY(query.exec("DROP TRIGGER reject_result"));
        controller.retrySaveResult();
        QVERIFY(!controller.resultPending());
        QCOMPARE(controller.ranking().size(), 1);
        controller.retrySaveResult();
        QCOMPARE(controller.ranking().size(), 1);
        controller.startGame();
        QVERIFY(!controller.gameFinished());
    }
    QSqlDatabase::removeDatabase("failure_test");
}

void GameServiceTests::rankingUpdatesExistingPlayer()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("ranking.sqlite");
    {
        SqliteGameHistoryRepository history(path);
        QVERIFY(history.saveResult("Ada", 10, 10));
        QVERIFY(history.saveResult("Lia", 5, 5));
        QVERIFY(history.saveResult(" ada ", 2, 2));
        const auto entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 2);
        QCOMPARE(entries.at(0).playerName, QString("Lia"));
        QCOMPARE(entries.at(1).playerName, QString("ada"));
        QCOMPARE(entries.at(1).score, 2);
        QCOMPARE(entries.at(1).correctAnswers, 2);
        QVERIFY(entries.at(1).playedAt.isValid());
        QVERIFY(!history.saveResult("   ", 99, 99));
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadTopResults(10).size(), 2);
    QCOMPARE(reopened.loadTopResults(10).last().score, 2);
}

void GameServiceTests::legacyRankingKeepsLatestResultPerPlayer()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("legacy.sqlite");
    {
        auto database = QSqlDatabase::addDatabase("QSQLITE", "legacy_test");
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec("CREATE TABLE history (id INTEGER PRIMARY KEY AUTOINCREMENT, player_name TEXT NOT NULL, score INTEGER NOT NULL, correct_answers INTEGER NOT NULL, played_at TEXT NOT NULL)"));
        QVERIFY(query.exec("INSERT INTO history (player_name, score, correct_answers, played_at) VALUES "
            "('Ada', 20, 20, '2026-01-01T00:00:00Z'),"
            "('ada', 3, 3, '2026-02-01T00:00:00Z'),"
            "('ADA', 2, 2, '2026-02-01T00:00:00Z'),"
            "('Ada', 10, 10, '2026-01-15T00:00:00Z'),"
            "('Lia', 7, 7, '2026-01-01T00:00:00Z')"));
    }
    QSqlDatabase::removeDatabase("legacy_test");
    {
        SqliteGameHistoryRepository history(path);
        const auto entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 2);
        QCOMPARE(entries.last().playerName, QString("ADA"));
        QCOMPARE(entries.last().score, 2);
        QCOMPARE(entries.last().playedAt.toString(Qt::ISODate), QString("2026-02-01T00:00:00Z"));
        QVERIFY(history.saveResult("Ada", 1, 1));
        QCOMPARE(history.loadTopResults(10).size(), 2);
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadTopResults(10).size(), 2);
    QCOMPARE(reopened.loadTopResults(10).last().score, 1);
}

void GameServiceTests::failedRankingUpdateKeepsPreviousResult()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("update.sqlite");
    SqliteGameHistoryRepository history(path);
    QVERIFY(history.saveResult("Ada", 10, 10));
    const QDateTime previousDate = history.loadTopResults(10).first().playedAt;
    {
        auto database = QSqlDatabase::addDatabase("QSQLITE", "update_failure_test");
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec("CREATE TRIGGER reject_update BEFORE UPDATE ON history BEGIN SELECT RAISE(FAIL, 'test update failure'); END"));
        QVERIFY(!history.saveResult("Ada", 2, 2));
        auto entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.first().score, 10);
        QCOMPARE(entries.first().playedAt, previousDate);
        QVERIFY(query.exec("DROP TRIGGER reject_update"));
        QVERIFY(history.saveResult("Ada", 2, 2));
        entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.first().score, 2);
    }
    QSqlDatabase::removeDatabase("update_failure_test");
}

void GameServiceTests::categoryQuizUsesOnlySelectedFamily()
{
    JsonSpeciesCatalog catalog;
    GameService service(&catalog);
    service.startNewGame("Saturniidae");
    QVERIFY(!service.isFinished());
    const int count = service.catalogSize();
    QVERIFY(count >= 4);
    QStringList asked;
    for (int i = 0; i < count; ++i) {
        const Question question = service.currentQuestion();
        QCOMPARE(question.species.family, QString("Saturniidae"));
        QVERIFY(!asked.contains(question.correctAnswer));
        asked << question.correctAnswer;
        for (const QString &option : question.options) {
            bool valid = false;
            for (const Species &species : catalog.loadSpecies()) {
                valid |= species.name == option && species.family == "Saturniidae";
            }
            QVERIFY(valid);
        }
        QVERIFY(service.submitAnswer(question.correctAnswer));
    }
    QCOMPARE(service.answeredCount(), count);
    QVERIFY(!service.isFinished());
    service.startNewGame("Unknown family");
    QVERIFY(service.isFinished());
    QCOMPARE(service.answeredCount(), 0);
}

void GameServiceTests::controllerKeepsAnswerUntilNextQuestion()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("answer.sqlite"));
    JsonSpeciesCatalog catalog;
    Species correct;
    for (const Species &species : catalog.loadSpecies()) {
        if ((!controller.questionPhoto().value("image").toString().isEmpty() && species.image == controller.questionPhoto().value("image").toString()) || (species.fact == controller.factText() && controller.answerOptions().contains(species.name))) { correct = species; break; }
    }
    QVERIFY(!correct.name.isEmpty());
    controller.answerSelected("Invalid answer");
    QVERIFY(!controller.answerPending());
    controller.answerSelected(correct.name);
    QVERIFY(controller.answerPending());
    QCOMPARE(controller.answeredCount(), 1);
    QCOMPARE(controller.score(), 1);
    QCOMPARE(controller.answerDetails().value("name").toString(), correct.name);
    QCOMPARE(controller.answerDetails().value("family").toString(), correct.family);
    QVERIFY(controller.answerDetails().value("correct").toBool());
    QCOMPARE(controller.categoryStats().first().toMap().value("answered").toInt(), 1);
    QVERIFY(controller.achievements().first().toMap().value("unlocked").toBool());
    controller.answerSelected(controller.answerOptions().first());
    QCOMPARE(controller.answeredCount(), 1);
    controller.nextQuestion();
    QVERIFY(!controller.answerPending());
    controller.answerSelected(controller.answerOptions().first());
    QCOMPARE(controller.answeredCount(), 2);
    controller.stopGame();
    controller.startGame();
    QVERIFY(!controller.answerPending());
    QVERIFY(controller.answerDetails().isEmpty());
    QVERIFY(controller.categoryStats().isEmpty());
    QVERIFY(!controller.achievements().first().toMap().value("unlocked").toBool());
}

void GameServiceTests::taxonomyFiltersAndIdentificationLevels()
{
    JsonSpeciesCatalog catalog;
    TaxonomyIndex index(catalog);
    QCOMPARE(index.kingdoms().size(), 6);
    int total = 0;
    for (const QVariant &entry : index.kingdoms().mid(1)) {
        const QString id = entry.toMap().value("id").toString();
        const auto selected = index.speciesFor(id);
        QVERIFY(selected.size() >= 20);
        QCOMPARE(index.count(id), selected.size());
        total += selected.size();
        for (const Species &s : selected) QVERIFY(s.belongsTo(id));
        QVERIFY(!index.children(id).isEmpty());
    }
    QCOMPARE(total, catalog.loadSpecies().size());
    QString lepidoptera;
    for (const Taxon &t : catalog.loadTaxons()) if (t.name == "Lepidoptera") lepidoptera = t.id;
    QVERIFY(!lepidoptera.isEmpty());
    QStringList ranks;
    for (const QVariant &level : index.identificationLevels(lepidoptera)) ranks << level.toMap().value("id").toString();
    QVERIFY(ranks.contains("family"));
    QVERIFY(!ranks.contains("order"));
    GameService service(&catalog);
    service.startNewGame(lepidoptera, "family");
    QVERIFY(!service.isFinished());
    for (int i = 0; i < service.catalogSize(); ++i) {
        const auto question = service.currentQuestion();
        QVERIFY(question.species.belongsTo(lepidoptera));
        QCOMPARE(question.correctAnswer, question.species.answerAtRank("family"));
        QCOMPARE(question.options.size(), 4);
        QVERIFY(!question.visual);
        QSet<QString> distinct;
        for (const QString &option : question.options) distinct.insert(option.toCaseFolded());
        QCOMPARE(distinct.size(), 4);
        QVERIFY(service.submitAnswer(question.correctAnswer));
    }
    service.startNewGame(lepidoptera, "order");
    QVERIFY(service.isFinished());
    const Species leaf = catalog.loadSpecies().first();
    const QString leafId = leaf.taxonomy.last().id;
    QCOMPARE(index.count(leafId), 1);
    QVERIFY(index.identificationLevels(leafId).isEmpty());
    QVERIFY(index.children(leafId).isEmpty());
    QVERIFY(index.breadcrumbs(leafId).size() >= 5);
    service.startNewGame();
    QSet<QString> realms;
    for (int i = 0; i < 40; ++i) {
        realms.insert(service.currentQuestion().species.educationalKingdom);
        service.submitAnswer(service.currentQuestion().correctAnswer);
    }
    QCOMPARE(realms.size(), 5);
}

void GameServiceTests::offlinePhotosHaveAttributionAndDecode()
{
    JsonSpeciesCatalog catalog;
    int imported = 0, original = 0;
    QSet<int> ids;
    QSet<QString> sources;
    for (const Species &s : catalog.loadSpecies()) {
        QVERIFY(!ids.contains(s.id)); ids.insert(s.id);
        if (s.source == "legacy") { ++original; QVERIFY(s.id <= 22); continue; }
        ++imported;
        QVERIFY(!sources.contains(s.sourceTaxonId)); sources.insert(s.sourceTaxonId);
        QVERIFY(s.imageLicense == "cc0" || s.imageLicense == "cc-by");
        QVERIFY(!s.imageAuthor.isEmpty());
        QVERIFY(s.imageSource.startsWith("https://www.inaturalist.org/observations/"));
        QVERIFY(!s.taxonomy.isEmpty());
        QVERIFY(!s.answerAtRank("species").isEmpty());
        if (!s.image.isEmpty()) {
            QVERIFY(s.image.startsWith("qrc:/"));
            QImageReader reader(s.image.mid(3));
            QVERIFY2(reader.canRead(), qPrintable(s.name + ": " + reader.errorString()));
            QVERIFY(!reader.read().isNull());
        }
        if (s.educationalKingdom == "Monera") QVERIFY(!s.visualQuiz);
    }
    QCOMPARE(original, 22);
    QVERIFY(imported >= 100);
}

void GameServiceTests::catalogMigrationPreservesPlayerDataAndStoredRecords()
{
    QTemporaryDir dir;
    const QString historyPath = dir.filePath("history.sqlite");
    const QString catalogPath = dir.filePath("catalog.sqlite");
    SqliteGameHistoryRepository history(historyPath);
    QVERIFY(history.addPlayerProfile("Ada"));
    QVERIFY(history.saveResult("Ada", 17, 17));
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", "catalog_migration_test");
        db.setDatabaseName(catalogPath); QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE taxons(id TEXT PRIMARY KEY, scientific_name TEXT NOT NULL, rank TEXT NOT NULL,parent_id TEXT REFERENCES taxons(id), source TEXT,source_taxon_id TEXT,UNIQUE(source,source_taxon_id))"));
        QVERIFY(q.exec("CREATE TABLE species(id INTEGER PRIMARY KEY,scientific_name TEXT NOT NULL,common_name TEXT,taxon_id TEXT NOT NULL REFERENCES taxons(id),educational_kingdom TEXT,image_path TEXT,image_author TEXT,image_license TEXT,image_source TEXT,source TEXT,source_taxon_id TEXT)"));
        QVERIFY(q.exec("INSERT INTO taxons VALUES('stored:test','Stored species','species',NULL,'local',NULL)"));
        QVERIFY(q.exec("INSERT INTO species(id,scientific_name,taxon_id,source) VALUES(99999,'Stored species','stored:test','local')"));
    }
    QSqlDatabase::removeDatabase("catalog_migration_test");
    int size;
    {
        SqliteSpeciesCatalog catalog(catalogPath);
        QVERIFY(catalog.isReady());
        TaxonomyIndex index(catalog);
        size = catalog.loadSpecies().size();
        QVERIFY(size >= 123);
        for (const QVariant &root : index.kingdoms().mid(1)) {
            const QString id = root.toMap().value("id").toString();
            QCOMPARE(catalog.descendantCount(id), index.count(id));
        }
    }
    SqliteSpeciesCatalog reopened(catalogPath);
    QVERIFY(reopened.isReady());
    QCOMPARE(reopened.loadSpecies().size(), size);
    const auto reopenedSpecies = reopened.loadSpecies();
    QVERIFY(std::any_of(reopenedSpecies.begin(), reopenedSpecies.end(), [](const Species &s) { return s.id == 99999; }));
    QCOMPARE(history.loadPlayerProfiles(), QStringList{"Ada"});
    QCOMPARE(history.loadTopResults(5).first().score, 17);
}

void GameServiceTests::invalidPhotoLicenseDisablesVisualQuestion()
{
    QTemporaryDir dir;
    QFile bundled(":/resources/data/species.json"); QVERIFY(bundled.open(QIODevice::ReadOnly));
    const QJsonArray records = QJsonDocument::fromJson(bundled.readAll()).array();
    QJsonObject record;
    for (const auto &value : records) if (value.toObject().value("source") == "inaturalist") { record=value.toObject(); break; }
    const QString path = dir.filePath("invalid-photo.json");
    for (const QString &license : QStringList{"cc-by-nc", "cc-by-sa", "cc-by-nd", "", "unknown"}) {
        QJsonObject photo = record.value("photo").toObject();
        photo.insert("license", license); photo.insert("visual_quiz", true); record.insert("photo", photo);
        QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(QJsonArray{record}).toJson()); file.close();
        const auto loaded = JsonSpeciesCatalog(path).loadSpecies();
        QCOMPARE(loaded.size(), 1);
        QVERIFY(loaded.first().image.isEmpty());
        QVERIFY(!loaded.first().visualQuiz);
    }
}

QTEST_MAIN(GameServiceTests)
#include "GameServiceTests.moc"
