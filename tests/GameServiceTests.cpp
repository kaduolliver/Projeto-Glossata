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
#include "application/EducationalContent.h"
#include "domain/QuizSelection.h"
#include "domain/TaxonomicPerformance.h"
#include "infrastructure/content/PhotoResolver.h"
#include "infrastructure/content/ReviewStatus.h"

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
    bool saveResult(const PlayerIdentity &, qint64, int, const QVector<AnswerRecord> &, const QString &) override {
        ++attempts;
        if (fail) return false;
        ++saved;
        return true;
    }
    QVector<HistoryEntry> loadTopResults(int) const override { return {}; }
};

// Fixture setup only: create/select an active ID before explicitly saving test games.
PlayerIdentity testPlayer(SqliteGameHistoryRepository &repo, const QString &name)
{
    auto profile=repo.profileNamed(name);
    if (!profile.selectable()) profile=repo.createPlayer(name);
    return {profile.id,name.trimmed()};
}

void bindFixtureAssessment(QJsonObject &assessment,const QString &sid,const QString &name,const QString &photo,const QString &category)
{
    auto body=assessment;body.remove("governance");body.remove("binding");
    auto source=assessment["binding"].toObject()["source"].toObject();
    if (source.isEmpty()) source=QJsonObject{{"test_only",true}};
    auto photoEvidence=assessment["binding"].toObject()["photo"].toObject();photoEvidence["id"]=photo;
    const QJsonObject proof{{"species_id",sid},{"scientific_name",name},{"category",category},
        {"photo",photoEvidence},{"source",source},{"assessment",body}};
    auto review=assessment["governance"].toObject();
    review["content_sha256"]=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(proof).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
    assessment["binding"]=proof;assessment["governance"]=review;
}

// Synthetic grading scenarios ONLY. No approval is written to shipped data.
QJsonObject gradedHintFixture()
{
    QFile file(":/resources/data/education.json");
    if (!file.open(QIODevice::ReadOnly)) return {};
    auto data=QJsonDocument::fromJson(file.readAll()).object();
    data.remove("automated_pedagogy"); // Frozen v3 assessment fixture.
    const QJsonObject reviewed{{"state","approved_editorial"},{"usable",true},{"reviewer","synthetic-test-fixture"},
        {"claim_scope","hint_assistance"},
        {"reviewed_at",QDateTime::currentDateTimeUtc().addSecs(-60).toString(Qt::ISODateWithMs)},
        {"due_at",QDateTime::currentDateTimeUtc().addDays(1).toString(Qt::ISODateWithMs)},
        {"content_sha256",QString(64,'a')}};
    auto species=data["species"].toObject(),notes=data["notes"].toObject();
    for (auto it=species.begin();it!=species.end();++it) {
        auto s=it.value().toObject(),taxonomy=s["taxonomic_assistance"].toObject();
        const auto photo=notes[s["morphological_hint"].toString()].toObject()["photo_id"].toString();
        taxonomy["grade"]="moderate";taxonomy["governance"]=reviewed;
        bindFixtureAssessment(taxonomy,it.key(),s["scientific_name"].toString(),photo,"taxonomic");s["taxonomic_assistance"]=taxonomy;
        it.value()=s;
        for (const auto &ref : s["etymology_hints"].toObject()) {
            const auto key=ref.toString();auto n=notes[key].toObject(),a=n["assistance"].toObject();
            a["grade"]="strong";a["governance"]=reviewed;
            bindFixtureAssessment(a,it.key(),s["scientific_name"].toString(),photo,"etymological");n["assistance"]=a;notes[key]=n;
        }
        const auto key=s["morphological_hint"].toString();
        if (!key.isEmpty()) {
            auto n=notes[key].toObject(),a=n["assistance"].toObject();
            a["grade"]="light";a["governance"]=reviewed;
            bindFixtureAssessment(a,it.key(),s["scientific_name"].toString(),photo,"morphological");n["assistance"]=a;notes[key]=n;
        }
    }
    data["species"]=species;data["notes"]=notes;return data;
}
QJsonObject progressiveFixture()
{
    QFile file(":/resources/data/education.json");if(!file.open(QIODevice::ReadOnly))return {};
    auto data=QJsonDocument::fromJson(file.readAll()).object();
    auto automatic=data["automated_pedagogy"].toObject(),species=automatic["species"].toObject(),notes=data["notes"].toObject();
    const QJsonObject review{{"state","legacy"},{"usable",true},{"content_sha256",QString(64,'a')}};
    for(auto it=species.begin();it!=species.end();++it) {
        auto entry=it.value().toObject();QJsonArray recipes;
        for(int i=0;i<3;++i) {
            const QString key="synthetic:"+it.key()+":"+QString::number(i);
            const QString text=QString("Synthetic test-only %1: distinct documented clue.").arg(i);
            notes[key]=QJsonObject{{"text",text},{"source_url","https://example.org/test-only"},{"governance",review}};
            recipes.append(QJsonObject{{"id",key},{"kind","note"},{"category","morphological"},
                {"text",text},{"ranks",QJsonArray{"genus"}},{"grade",i==0?"light":i==1?"moderate":"strong"},
                {"source_url","https://example.org/test-only"},{"source_title","Synthetic fixture"},
                {"locator","test-only"},{"scientific_topic","morphology"},{"source_note",key},{"rationale","Synthetic test-only assistance fixture."}});
        }
        entry["recipes"]=recipes;it.value()=entry;
    }
    automatic["species"]=species;auto body=automatic;body.remove("evaluation");body.remove("usable");
    auto ev=automatic["evaluation"].toObject();ev["content_sha256"]=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(body).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());automatic["evaluation"]=ev;
    data["automated_pedagogy"]=automatic;data["notes"]=notes;return data;
}

}

class GameServiceTests : public QObject {
    Q_OBJECT

private slots:
    void offlineReviewExpiryAndDrafts();
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
    void permanentIdentitySurvivesRenameDeleteAndSelectionDuringGame();
    void sessionWithoutIdentityCannotChooseAnOwnerAtSaveTime();
    void controllerCanRetrySqliteFailure();
    void rankingUpdatesExistingPlayer();
    void legacyRankingKeepsLatestResultPerPlayer();
    void failedRankingUpdateKeepsPreviousResult();
    void categoryQuizUsesOnlySelectedFamily();
    void controllerKeepsAnswerUntilNextQuestion();
    void taxonomyFiltersAndIdentificationLevels();
    void explorationShortcutsPreserveHierarchyAndQuizFilters();
    void explorationPhotographsFailClosed();
    void newTaxonomicRanksRequireIndependentReview();
    void offlinePhotosHaveAttributionAndDecode();
    void catalogMigrationPreservesPlayerDataAndStoredRecords();
    void phylogeneticCatalogMigrationRollsBackAndReopens();
    void invalidPhotoLicenseDisablesVisualQuestion();
    void photoResolverRejectsMissingCorruptAndUnverifiedResources();
    void visualEligibilityAndHintsAcrossRanks();
    void introductionsAndMissingContent();
    void speciesProfilesHaveSourcesAndMatchIdentity();
    void exactHintScoringAndLongGames();
    void fractionalHistoryMigrationAndAnswerLedger();
    void hintContentMatchesQuestionAndAlternatives();
    void performanceUsesIdsAndHandlesMissingClassification();
    void exportVerifiedCatalogAudit();
    void replacementsRetireOldSpeciesWithoutDeletingHistory();
    void competitiveLifecycleAndConfigurationIsolation();
    void questionPoliciesAndReviewGates();
    void answerMetricsDoNotDoubleCount();
    void versionedMigrationReconstructsKnownRecords();
    void failedSchemaMigrationPreservesOriginalDatabase();
    void contextualAssistanceAndUnknownDistractors();
    void assistanceSnapshotsAndLegacyV2Migration();
    void assistanceChangesCompetitiveContentVersion();
    void progressiveCoverageHistoryAndReports();
};

void GameServiceTests::initialLoad_hasSpecies()
{
    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    const QVector<Species> species = catalog.loadSpecies();
    QVERIFY(species.size() >= 20);
    for (const Species &item : species) {
        QVERIFY(!item.name.trimmed().isEmpty());
        QVERIFY(!item.educationalKingdom.isEmpty());
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
    JsonSpeciesCatalog source;MemoryCatalog catalog;
    for (const auto &s : source.loadSpecies()) if (QSet<int>{63,67,73,135}.contains(s.id)) catalog.items.append(s);
    GameService service(&catalog);
    service.startNewGame({},"genus");

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

    const int speciesCount = service.catalogSize();
    QStringList askedSpecies;
    for (int round = 1; round <= speciesCount; ++round) {
        const Question question = service.currentQuestion();
        QCOMPARE(service.round(), round);
        QCOMPARE(question.options.size(), 4);
        QVERIFY(question.options.contains(question.correctAnswer));
        QCOMPARE(question.visual,question.photo.role=="identification");
        QVERIFY(question.photo.valid);
        QCOMPARE(question.photo.image,question.species.image);
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
        QCOMPARE(service.scoreUnits(), qint64(round*6));
        QVERIFY(!service.isFinished());
    }

    QCOMPARE(askedSpecies.size(), speciesCount);
    QCOMPARE(service.round(), speciesCount + 1);
    QVERIFY(!service.currentQuestion().correctAnswer.isEmpty());
    QVERIFY(service.currentQuestion().correctAnswer != askedSpecies.last());

    service.finishGame();
    QVERIFY(service.isFinished());
    QVERIFY(service.currentQuestion().options.isEmpty());
    const int completedScore = service.scoreUnits();
    QVERIFY(!service.submitAnswer(service.currentQuestion().correctAnswer));
    QCOMPARE(service.scoreUnits(), completedScore);
}

void GameServiceTests::completedGameIsSavedOnce()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    JsonSpeciesCatalog catalog(QStringLiteral(":/resources/data/species.json"));
    SqliteGameHistoryRepository history(temporaryDirectory.filePath(QStringLiteral("nested/history.sqlite")));
    GameService service(&catalog, &history);
    service.startNewGame({},"species",history.createPlayer("Test Player").identity());

    for (int answer = 0; answer < 2; ++answer) {
        QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
    }
    service.finishGame();

    service.saveCurrentResult();
    service.saveCurrentResult();

    const QVector<HistoryEntry> entries = history.loadTopResults(10);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.front().playerName, QStringLiteral("Test Player"));
    QCOMPARE(entries.front().scoreUnits, qint64(12));
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
    QVERIFY(service.isFinished()); // Four names without approved photos are insufficient.
    catalog.items=eligibleQuizSpecies(JsonSpeciesCatalog().loadSpecies(),"species").mid(0,4);
    for (Species &s : catalog.items) { s.fact.clear(); s.hint.clear(); }
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
    const QList<QByteArray> invalidDocuments{"{", "{}", "[null, {}, {\"name\":\"   \",\"fact\":\"B\"}]"};
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
    QCOMPARE(service.scoreUnits(), 0);
    QCOMPARE(service.round(), 2);
    QVERIFY(!service.isFinished());
}

void GameServiceTests::emptyGameIsNotSaved()
{
    JsonSpeciesCatalog catalog;
    FailingHistory history;
    GameService service(&catalog, &history);
    service.startNewGame();
    QCOMPARE(service.saveCurrentResult(), GameService::SaveStatus::NotNeeded);
    service.finishGame();
    QCOMPARE(service.saveCurrentResult(), GameService::SaveStatus::NotNeeded);
    QCOMPARE(history.attempts, 0);
}

void GameServiceTests::failedSaveCanBeRetried()
{
    JsonSpeciesCatalog catalog;
    FailingHistory history;
    GameService service(&catalog, &history);
    service.startNewGame({},"species",{"synthetic-player-id","Player"});
    QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
    service.finishGame();
    QCOMPARE(service.saveCurrentResult(), GameService::SaveStatus::Failed);
    history.fail = false;
    QCOMPARE(service.saveCurrentResult(), GameService::SaveStatus::Saved);
    QCOMPARE(service.saveCurrentResult(), GameService::SaveStatus::NotNeeded);
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

void GameServiceTests::permanentIdentitySurvivesRenameDeleteAndSelectionDuringGame()
{
    QTemporaryDir dir; const auto path=dir.filePath("session-identity.sqlite");
    GameController controller(nullptr,path); QVERIFY(controller.addPlayerProfile("Ada"));
    controller.startGame(); const auto owner=controller.sessionPlayerId();
    QCOMPARE(owner,controller.currentPlayerId()); QCOMPARE(controller.sessionPlayerName(),QString("Ada"));
    controller.answerSelected(controller.answerOptions().first());
    QVERIFY(controller.renameCurrentPlayerProfile("Lia")); QCOMPARE(controller.currentPlayerId(),owner);
    QCOMPARE(controller.sessionPlayerName(),QString("Ada"));
    QVERIFY(controller.addPlayerProfile("Bia")); QVERIFY(controller.currentPlayerId()!=owner);
    controller.selectPlayerProfile(owner); QVERIFY(controller.deleteCurrentPlayerProfile());
    QVERIFY(controller.addPlayerProfile("Lia")); const auto replacement=controller.currentPlayerId(); QVERIFY(replacement!=owner);
    controller.stopGame(); QVERIFY(!controller.resultPending());
    QCOMPARE(controller.ranking().size(),1); const auto result=controller.ranking().first().toMap();
    QCOMPARE(result["playerId"].toString(),owner); QCOMPARE(result["snapshotName"].toString(),QString("Ada"));
    QVERIFY(result["deletedProfile"].toBool()); QVERIFY(controller.personalRecord().isEmpty());
    QCOMPARE(controller.playerMetrics()["answered"].toInt(),0);
    controller.startGame(); QCOMPARE(controller.sessionPlayerId(),replacement);
    controller.answerSelected(controller.answerOptions().first()); controller.stopGame();
    QCOMPARE(controller.ranking().size(),2);
    SqliteGameHistoryRepository inspection(path);
    QCOMPARE(inspection.playerMetrics("free",owner).answered,1);
    QCOMPARE(inspection.playerMetrics("free",replacement).answered,1);
}

void GameServiceTests::sessionWithoutIdentityCannotChooseAnOwnerAtSaveTime()
{
    QTemporaryDir dir; SqliteGameHistoryRepository history(dir.filePath("anonymous.sqlite")); JsonSpeciesCatalog catalog;
    GameService service(&catalog,&history); service.startNewGame();
    QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer)); service.finishGame();
    QCOMPARE(service.saveCurrentResult(),GameService::SaveStatus::Failed); QVERIFY(history.loadTopResults().isEmpty());
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
        const int score = controller.scoreUnits();
        controller.stopGame();
        QVERIFY(controller.resultPending());
        QVERIFY(controller.ranking().isEmpty());
        controller.startGame();
        QVERIFY(controller.gameFinished());
        QCOMPARE(controller.scoreUnits(), score);
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
        QVERIFY(history.saveResult(testPlayer(history, "Ada"), 10, 10));
        QVERIFY(history.saveResult(testPlayer(history, "Lia"), 5, 5));
        QVERIFY(history.saveResult(testPlayer(history, " ada "), 2, 2));
        const auto entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 2);
        QCOMPARE(entries.at(0).playerName, QString("Ada"));
        QCOMPARE(entries.at(1).playerName, QString("Lia"));
        QCOMPARE(entries.at(0).scoreUnits, 10);
        QCOMPARE(entries.at(0).lastScoreUnits, qint64(2));
        QVERIFY(entries.at(1).playedAt.isValid());
        QVERIFY(!history.saveResult(testPlayer(history, "   "), 99, 99));
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadTopResults(10).size(), 2);
    QCOMPARE(reopened.loadTopResults(10).first().scoreUnits, 10);
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
        QCOMPARE(entries.first().playerName, QString("ADA"));
        QCOMPARE(entries.first().scoreUnits, qint64(120));
        QCOMPARE(entries.first().lastPlayedAt.toString(Qt::ISODate), QString("2026-02-01T00:00:00Z"));
        QVERIFY(history.saveResult(testPlayer(history, "Ada"), 1, 1));
        QCOMPARE(history.loadTopResults(10).size(), 3); // Old ranking retained separately from the new identity.
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadTopResults(10).size(), 3);
    QCOMPARE(reopened.loadPlayerResult("free",reopened.profileNamed("Ada").id).first().lastScoreUnits, qint64(1));
}

void GameServiceTests::failedRankingUpdateKeepsPreviousResult()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("update.sqlite");
    SqliteGameHistoryRepository history(path);
    QVERIFY(history.saveResult(testPlayer(history, "Ada"), 10, 10));
    const QDateTime previousDate = history.loadTopResults(10).first().playedAt;
    {
        auto database = QSqlDatabase::addDatabase("QSQLITE", "update_failure_test");
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec("CREATE TRIGGER reject_update BEFORE UPDATE ON history BEGIN SELECT RAISE(FAIL, 'test update failure'); END"));
        QVERIFY(!history.saveResult(testPlayer(history, "Ada"), 2, 2));
        auto entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.first().scoreUnits, 10);
        QCOMPARE(entries.first().playedAt, previousDate);
        QVERIFY(query.exec("DROP TRIGGER reject_update"));
        QVERIFY(history.saveResult(testPlayer(history, "Ada"), 2, 2));
        entries = history.loadTopResults(10);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.first().scoreUnits, 10);
        QCOMPARE(entries.first().lastScoreUnits,qint64(2));
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
        if (species.id == controller.questionPhoto().value("speciesId").toInt()) { correct = species; break; }
    }
    QVERIFY(!correct.name.isEmpty());
    const auto before=controller.questionPhoto();
    const auto options=controller.answerOptions();
    controller.answerSelected("Invalid answer");
    QVERIFY(!controller.answerPending());
    controller.answerSelected(correct.name);
    QVERIFY(controller.answerPending());
    QCOMPARE(controller.answeredCount(), 1);
    QCOMPARE(controller.scoreUnits(), qint64(6));
    QCOMPARE(controller.answerDetails().value("name").toString(), correct.name);
    QCOMPARE(controller.answerDetails().value("family").toString(), correct.family);
    QVERIFY(controller.answerDetails().value("correct").toBool());
    QCOMPARE(controller.questionPhoto(),before);
    QCOMPARE(controller.answerOptions(),options);
    QCOMPARE(controller.answerDetails().value("photoId"),before.value("photoId"));
    QCOMPARE(controller.answerDetails().value("image"),before.value("image"));
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
    QCOMPARE(index.domains().size(), 4);
    int total = 0;
    for (const QVariant &entry : index.kingdoms().mid(1)) {
        const QString id = entry.toMap().value("id").toString();
        const auto selected = index.speciesFor(id);
        if(id=="domain:Archaea") {QVERIFY(selected.isEmpty());QVERIFY(index.identificationLevels(id).isEmpty());continue;}
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
        QVERIFY(question.visual);
        QVERIFY(question.photo.valid);
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
    service.startNewGame({},"genus");
    QSet<QString> realms;
    for (int i = 0; i < service.catalogSize(); ++i) {
        realms.insert(service.currentQuestion().species.educationalKingdom);
        service.submitAnswer(service.currentQuestion().correctAnswer);
    }
    QCOMPARE(realms.size(), 5); // Frozen legacy membership remains readable.
    QVERIFY(realms.contains("Monera"));
    QVERIFY(!index.identificationLevels("edu:Monera").isEmpty());
}

void GameServiceTests::explorationShortcutsPreserveHierarchyAndQuizFilters()
{
    JsonSpeciesCatalog catalog;
    JsonExplorationMediaCatalog media;
    TaxonomyIndex index(catalog,&media);
    const auto shortcuts=index.quickAccess();
    QCOMPARE(shortcuts.size(),3);
    const QStringList names{"Animalia","Plantae","Fungi"};
    const QStringList lineages{"Opisthokonta","Archaeplastida","Opisthokonta"};
    for(int i=0;i<shortcuts.size();++i) {
        const auto entry=shortcuts[i].toMap();
        QCOMPARE(entry["title"].toString(),names[i]);
        QCOMPARE(entry["rank"].toString(),QString("kingdom"));
        const auto path=index.breadcrumbs(entry["id"].toString());
        QCOMPARE(path[0].toMap()["title"].toString(),QString("Eukaryota"));
        QCOMPARE(path[1].toMap()["title"].toString(),lineages[i]);
        QCOMPARE(path.last().toMap()["id"],entry["id"]);
        QVERIFY(!entry["photo"].toMap().isEmpty());
        GameService service(&catalog);
        service.startNewGame(entry["id"].toString(),"genus");
        QVERIFY(!service.isFinished());
        const int size=service.catalogSize();
        for(int j=0;j<size;++j) {
            QVERIFY(service.currentQuestion().species.belongsTo(entry["id"].toString()));
            QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
        }
    }
    // Every catalog organism is reachable by following actual children, including
    // paths with intermediate ranks. Shortcuts neither reparent nor hide nodes.
    QStringList pending{"domain:Bacteria","domain:Archaea","domain:Eukaryota"};
    QSet<QString> visited;
    QStringList withoutPresentation;
    while(!pending.isEmpty()) {
        const auto id=pending.takeFirst();
        QVERIFY(!visited.contains(id));visited.insert(id);
        const auto photo=index.describe(id)["photo"].toMap();
        if(photo.value("image").toString().isEmpty() && photo.value("pendingReason").toString().isEmpty())withoutPresentation.append(id);
        for(const auto &child:index.children(id))pending.append(child.toMap()["id"].toString());
    }
    QVERIFY2(withoutPresentation.isEmpty(),qPrintable(withoutPresentation.join(", ")));
    const auto organisms=catalog.loadSpecies();
    QCOMPARE(organisms.size(),122);
    for(const auto &organism:organisms) {
        for(const auto &node:organism.taxonomy)QVERIFY(visited.contains(node.id));
        QCOMPARE(index.count(organism.taxonomy.last().id),1);
    }
    const auto discoba=index.describe("clade:Discoba")["photo"].toMap();
    QVERIFY(discoba.contains("image") || discoba.contains("pendingReason"));
    QVERIFY(!index.describe("domain:Archaea")["photo"].toMap().isEmpty());
    QCOMPARE(index.count("domain:Archaea"),0);
}

void GameServiceTests::explorationPhotographsFailClosed()
{
    JsonExplorationMediaCatalog embedded;
    QVERIFY(!embedded.photoFor("domain:Bacteria").isEmpty());
    QCOMPARE(embedded.photoFor("domain:Bacteria")["kind"].toString(),QString("micrograph"));
    QVERIFY(embedded.photoFor("unknown:id").isEmpty());
    QFile source(":/resources/data/exploration_photos.json");QVERIFY(source.open(QIODevice::ReadOnly));
    auto data=QJsonDocument::fromJson(source.readAll()).object();
    const auto lion=data["photos"].toObject()["lion"].toObject();
    QTemporaryDir directory;QVERIFY(directory.isValid());
    QDir().mkpath(directory.filePath("resources/data"));
    QVERIFY(QFile::copy(":/resources/data/species.json",directory.filePath("resources/data/species.json")));
    QVERIFY(QFile::copy(":/resources/data/taxonomy.json",directory.filePath("resources/data/taxonomy.json")));
    for(const auto &key:{"path","evidence_path"}) {
        const QString path=lion[key].toString();
        QDir().mkpath(QFileInfo(directory.filePath(path)).absolutePath());
        QVERIFY(QFile::copy(":/"+path,directory.filePath(path)));
    }
    auto write=[&](const QJsonObject &photo) {
        auto photos=data["photos"].toObject();photos["lion"]=photo;data["photos"]=photos;
        QFile target(directory.filePath("resources/data/exploration_photos.json"));
        if(!target.open(QIODevice::WriteOnly))return false;
        return target.write(QJsonDocument(data).toJson())>0;
    };
    QVERIFY(write(lion));
    JsonExplorationMediaCatalog valid(directory.path());
    QVERIFY(!valid.photoFor("clade:Opisthokonta").isEmpty());
    QVERIFY(valid.photoFor("domain:Bacteria").isEmpty());
    auto invalidFocus=lion;invalidFocus["focus_x"]=1.1;QVERIFY(write(invalidFocus));
    JsonExplorationMediaCatalog outside(directory.path());QVERIFY(outside.photoFor("clade:Opisthokonta").isEmpty());
    auto invalidType=lion;invalidType["focus_x"]="left";QVERIFY(write(invalidType));
    JsonExplorationMediaCatalog wrongType(directory.path());QVERIFY(wrongType.photoFor("clade:Opisthokonta").isEmpty());
    for(const auto &key:{"license_version","sha256","evidence_sha256","author","path"}) {
        auto invalid=lion;invalid[key]="invalid";QVERIFY(write(invalid));
        JsonExplorationMediaCatalog rejected(directory.path());
        QVERIFY2(rejected.photoFor("clade:Opisthokonta").isEmpty(),key);
    }
    auto bindings=data["bindings"].toObject();bindings["domain:Bacteria"]="lion";data["bindings"]=bindings;
    QVERIFY(write(lion));JsonExplorationMediaCatalog mismatched(directory.path());
    QVERIFY(mismatched.photoFor("domain:Bacteria").isEmpty());
}

void GameServiceTests::newTaxonomicRanksRequireIndependentReview()
{
    QVector<Species> pool;
    for(int i=0;i<4;++i) {
        Species s;s.id=i+1;s.name="Synthetic organism "+QString::number(i);
        s.taxonomy={{"test:clade:"+QString::number(i),"Test clade "+QString::number(i),"supergroup"}};
        s.photo.valid=true;s.photo.approvedRanks={"supergroup"};
        pool.append(s);
    }
    QVERIFY(eligibleQuizSpecies(pool,"supergroup").isEmpty());
    for(auto &s:pool)s.phylogenyApproved=true;
    QCOMPARE(eligibleQuizSpecies(pool,"supergroup").size(),4);
    pool[0].photo.approvedRanks.clear();
    QVERIFY(eligibleQuizSpecies(pool,"supergroup").isEmpty());
    JsonSpeciesCatalog catalog;TaxonomyIndex index(catalog);
    for(const auto &s:catalog.loadSpecies()) {
        QVERIFY(!s.domain().isEmpty());
        QVERIFY(!s.phylogenyApproved); // New source synthesis never auto-approves itself.
        QVERIFY(!approvedForQuiz(s,"domain"));QVERIFY(!approvedForQuiz(s,"supergroup"));
    }
    for(const auto &level:index.identificationLevels({})) {
        const auto rank=level.toMap().value("id").toString();
        QVERIFY(rank!="kingdom" && rank!="domain" && rank!="supergroup");
    }
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
    QCOMPARE(original, 19);
    QVERIFY(imported >= 100);
}

void GameServiceTests::phylogeneticCatalogMigrationRollsBackAndReopens()
{
    QTemporaryDir directory;
    const QString path=directory.filePath("legacy-catalog.sqlite");
    const QString connection="phylogeny-rollback-fixture";
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE taxons(id TEXT PRIMARY KEY,scientific_name TEXT NOT NULL,rank TEXT NOT NULL,parent_id TEXT,source TEXT,source_taxon_id TEXT,UNIQUE(source,source_taxon_id))"));
        QVERIFY(q.exec("CREATE TABLE species(id INTEGER PRIMARY KEY,scientific_name TEXT NOT NULL,common_name TEXT,taxon_id TEXT NOT NULL,educational_kingdom TEXT,image_path TEXT,image_author TEXT,image_license TEXT,image_source TEXT,source TEXT,source_taxon_id TEXT)"));
        QVERIFY(q.exec("INSERT INTO taxons VALUES('edu:Monera','Monera','educational_kingdom',NULL,'educational',NULL)"));
        QVERIFY(q.exec("CREATE TRIGGER reject_projection BEFORE INSERT ON taxons WHEN NEW.id LIKE 'domain:%' BEGIN SELECT RAISE(ABORT,'synthetic migration failure'); END"));
    }
    QSqlDatabase::removeDatabase(connection);
    {
        SqliteSpeciesCatalog failed(path);
        QVERIFY(!failed.isReady());
        QCOMPARE(failed.loadSpecies().size(),122); // Read-only bundled fallback still works.
    }
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
        QVERIFY(q.exec("PRAGMA table_info(taxons)"));QStringList columns;while(q.next())columns<<q.value(1).toString();
        QVERIFY(!columns.contains("rank_note")); // DDL was rolled back with the data.
        QVERIFY(q.exec("SELECT COUNT(*) FROM taxons"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),1);
        QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);q.finish();
        QVERIFY(q.exec("DROP TRIGGER reject_projection"));
    }
    QSqlDatabase::removeDatabase(connection);
    JsonSpeciesCatalog bundled;
    {
        SqliteSpeciesCatalog migrated(path);QVERIFY(migrated.isReady());
        QMap<int,Species> byId;for(const auto &s:migrated.loadSpecies())byId.insert(s.id,s);
        QCOMPARE(byId.size(),122);
        for(const auto &expected:bundled.loadSpecies()) {
            const auto actual=byId.value(expected.id);
            QCOMPARE(actual.name,expected.name);QCOMPARE(actual.domain(),expected.domain());
            QCOMPARE(actual.photo.id,expected.photo.id);QCOMPARE(actual.photo.sha256,expected.photo.sha256);
            QCOMPARE(actual.photo.author,expected.photo.author);QCOMPARE(actual.photo.license,expected.photo.license);
            QCOMPARE(actual.taxonomy.size(),expected.taxonomy.size());
            for(int i=0;i<actual.taxonomy.size();++i) {
                QCOMPARE(actual.taxonomy[i].id,expected.taxonomy[i].id);
                QCOMPARE(actual.taxonomy[i].rank,expected.taxonomy[i].rank);
                QCOMPARE(actual.taxonomy[i].sourceUrl,expected.taxonomy[i].sourceUrl);
            }
        }
        TaxonomyIndex index(migrated);QCOMPARE(index.domains().size(),4);
        QCOMPARE(index.count("domain:Eukaryota"),102);QCOMPARE(index.count("domain:Bacteria"),20);
    }
    {SqliteSpeciesCatalog reopened(path);QVERIFY(reopened.isReady());QCOMPARE(reopened.loadSpecies().size(),122);}
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
        QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),2);
        QVERIFY(q.exec("SELECT scientific_name FROM taxons WHERE id='edu:Monera'"));QVERIFY(q.next());QCOMPARE(q.value(0).toString(),QString("Monera"));
        QVERIFY(q.exec("SELECT payload_json FROM catalog_migration_snapshots WHERE version=2"));QVERIFY(q.next());
        const auto snapshot=QJsonDocument::fromJson(q.value(0).toByteArray()).object();
        QCOMPARE(snapshot.value("taxons").toArray().size(),1);
        QCOMPARE(snapshot.value("taxons").toArray().first().toObject().value("id").toString(),QString("edu:Monera"));
        QVERIFY(!q.next());
        QVERIFY(q.exec("PRAGMA user_version=99"));
    }
    QSqlDatabase::removeDatabase(connection);
    {SqliteSpeciesCatalog future(path);QVERIFY(!future.isReady());QCOMPARE(future.loadSpecies().size(),122);}
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
        QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),99);
    }
    QSqlDatabase::removeDatabase(connection);
}

void GameServiceTests::catalogMigrationPreservesPlayerDataAndStoredRecords()
{
    QTemporaryDir dir;
    const QString historyPath = dir.filePath("history.sqlite");
    const QString catalogPath = dir.filePath("catalog.sqlite");
    SqliteGameHistoryRepository history(historyPath);
    QVERIFY(history.addPlayerProfile("Ada"));
    QVERIFY(history.saveResult(testPlayer(history, "Ada"), 17, 17));
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
    QCOMPARE(history.loadTopResults(5).first().scoreUnits, 17);
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

void GameServiceTests::photoResolverRejectsMissingCorruptAndUnverifiedResources()
{
    const auto species=eligibleQuizSpecies(JsonSpeciesCatalog().loadSpecies(),"species");
    QVERIFY(species.size()>=4);
    const auto photo=species.first().photo;
    QVERIFY(PhotoResolver::validate(photo,species.first().name).valid);
    Photo broken=photo;
    broken.image="qrc:/resources/images/absent/"+photo.id+".jpg";
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    broken=photo; broken.sha256=QString(64,'0');
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    broken=photo; broken.author="no rights reserved";
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    broken=photo; broken.sourceVerified=false;
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    broken=photo; broken.id="another-photo";
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    broken=photo; broken.licenseUrl="https://example.invalid/license";
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    QVERIFY(!PhotoResolver::validate(photo,"Another organism").valid);
    broken=photo; broken.image="https://example.invalid/photo.jpg";
    QVERIFY(!PhotoResolver::validate(broken,species.first().name).valid);
    for (const QString &path : QStringList{"qrc:/resources/images/test-small.png","qrc:/resources/images/test-invalid.jpg"}) {
        broken=photo; broken.image=path;
        broken.id=QFileInfo(path).completeBaseName();
        broken.downloadUrl="https://example.test/photos/"+broken.id+"/original.jpg";
        QFile bytes(path.mid(3)); QVERIFY(bytes.open(QIODevice::ReadOnly));
        broken.sha256=QCryptographicHash::hash(bytes.readAll(),QCryptographicHash::Sha256).toHex();
        const auto rejected=PhotoResolver::validate(broken,species.first().name);
        QVERIFY(!rejected.valid);
        QVERIFY(rejected.error.contains(path.endsWith("png") ? QStringLiteral("Resolução") : QStringLiteral("decodificar")));
    }
}

void GameServiceTests::visualEligibilityAndHintsAcrossRanks()
{
    JsonSpeciesCatalog catalog;
    const auto all=catalog.loadSpecies();
    GameService service(&catalog);
    EducationalContent content;
    for (const QString &rank : QStringList{"species","genus","family","order","class","phylum","kingdom"}) {
        service.startNewGame({},rank);
        if(rank=="kingdom") {QVERIFY(service.isFinished());continue;} // Only three scientific kingdoms are populated.
        QVERIFY(!service.isFinished());
        for (int i=0;i<service.catalogSize();++i) {
            const auto q=service.currentQuestion();
            QVERIFY(q.photo.valid && q.photo.sourceVerified);
            QCOMPARE(q.visual,q.photo.role=="identification");
            QVERIFY(q.photo.approvedRanks.contains(rank));
            if (q.photo.role=="supporting") {
                QCOMPARE(q.species.educationalKingdom,QString("Monera"));
                QCOMPARE(q.photo.supportingFor,q.species.name);
                QVERIFY(!q.fact.contains("Gênero:"));
                QVERIFY(!q.fact.contains("Família:"));
            } else QCOMPARE(q.photo.scientificName,q.species.name);
            QCOMPARE(q.options.size(),4);
            QSet<QString> options;
            for (const QString &option : q.options) options.insert(option.toCaseFolded());
            QCOMPARE(options.size(),4);
            QCOMPARE(q.options.count(q.correctAnswer),1);
            if (q.visual) QVERIFY(!q.fact.contains(q.species.name.section(' ',0,0),Qt::CaseInsensitive));
            QVERIFY(!q.extraHint.contains("iNaturalist",Qt::CaseInsensitive));
            if (!q.extraHint.isEmpty()) {
                QVERIFY(!q.hintSourceUrl.isEmpty());
                for (const QString &option : q.options) QVERIFY(!q.extraHint.contains(option,Qt::CaseInsensitive));
                QVERIFY(service.revealHint());
                QVERIFY(!service.revealHint());
            } else {
                QVERIFY(!service.revealHint());
            }
            service.submitAnswer("Not an option");
            QCOMPARE(service.answeredCount(),i);
            QVERIFY(service.submitAnswer(q.correctAnswer));
            int answered=0,correct=0,children=0;
            for (const auto &kingdom : service.performance().groups) {
                answered+=kingdom.answered; correct+=kingdom.correct;
                for (const auto &phylum : kingdom.phyla) children+=phylum.answered;
            }
            QCOMPARE(answered,i+1); QCOMPARE(correct,i+1); QCOMPARE(children,i+1);
        }
    }
    const auto eligible=eligibleQuizSpecies(all,"species");
    for (const Species &s : all) {
        if (s.educationalKingdom=="Monera") QCOMPARE(s.photo.role,QString("supporting"));
        if (!s.photo.valid || s.photo.approvedRanks.isEmpty())
            QVERIFY(std::none_of(eligible.cbegin(),eligible.cend(),[&s](const Species &candidate) { return candidate.id==s.id; }));
    }
    Species missing; missing.id=9999; missing.name="Unknown example";
    QVERIFY(content.hint(missing,"species",{}).isEmpty());
    QVERIFY(content.curiosity(missing).isEmpty());
}

void GameServiceTests::introductionsAndMissingContent()
{
    JsonSpeciesCatalog catalog;
    EducationalContent content;
    int described=0;
    QSet<QString> ranks;
    for (const Taxon &t : catalog.loadTaxons()) {
        const auto intro=content.introduction(t.id);
        if (intro.isEmpty()) continue;
        ++described; ranks.insert(t.rank);
        const auto words=intro.value("text").toString().split(' ',Qt::SkipEmptyParts).size();
        QVERIFY(words>=40 && words<=90);
        QVERIFY(intro.value("verified").toBool());
        QVERIFY(intro.value("source_url").toString().startsWith("https://"));
    }
    QCOMPARE(described,14);
    QVERIFY(ranks.contains("genus") && ranks.contains("family") && ranks.contains("order")
        && ranks.contains("class") && ranks.contains("phylum") && ranks.contains("kingdom"));
    QVERIFY(content.introduction("unknown").isEmpty());
    QString fungiId,plantsId;
    for(const auto &n : catalog.loadTaxons()) {if(n.name=="Fungi")fungiId=n.id;if(n.name=="Plantae")plantsId=n.id;}
    const auto fungi=content.introduction(fungiId);
    const auto plants=content.introduction(plantsId);
    QVERIFY(fungi.value("text")!=plants.value("text"));
}

void GameServiceTests::speciesProfilesHaveSourcesAndMatchIdentity()
{
    JsonSpeciesCatalog catalog;
    EducationalContent content;
    int profiles=0,confirmed=0;
    for (const Species &s : catalog.loadSpecies()) {
        const auto profile=content.profile(s);
        QVERIFY2(!profile.isEmpty(),qPrintable(s.name));
        ++profiles;
        QVERIFY(!profile.value("displayName").toString().isEmpty());
        QCOMPARE(profile.value("reviewedOn").toString().isEmpty(),s.id!=82);
        const auto review=profile.value("governance").toMap().value("education").toMap();
        QCOMPARE(review.value("state").toString(),s.id==82 ? QString("approved_editorial") : QString("legacy"));
        QCOMPARE(review.value("reviewer").toString().isEmpty(),s.id!=82);
        if (s.id==82) {
            // Restored only after the maintainer's explicit, hash-bound editorial decision.
            for (const auto &field : {QString("nutrition"),QString("curiosity")}) {
                const auto note=profile.value(field).toMap();
                QVERIFY(!note.value("text").toString().isEmpty());
                const auto noteReview=note.value("governance").toMap();
                QCOMPARE(noteReview.value("state").toString(),QString("approved_editorial"));
                QCOMPARE(noteReview.value("review_kind").toString(),QString("editorial"));
                QVERIFY(!noteReview.value("reviewer").toString().isEmpty());
            }
            QVERIFY(review.value("usable").toBool());
        }
        for (const QString &field : {QString("summary"),QString("distribution"),QString("morphology"),QString("conservation")}) {
            const auto item=profile.value(field).toMap();
            QVERIFY2(!item.value("text").toString().isEmpty(),qPrintable(s.name+":"+field));
            QVERIFY(item.value("source_url").toString().startsWith("https://"));
            const auto claimReview=item.value("governance").toMap();
            QCOMPARE(claimReview.value("state").toString(),QString("legacy"));
            QVERIFY(claimReview.value("reviewed_at").toString().isEmpty());
        }
        QVERIFY(!profile.value("distribution").toMap().value("text").toString().contains("não revisada"));
        const auto status=profile.value("conservation").toMap();
        if (status.value("status").toString()!="unconfirmed") {
            ++confirmed;
            QCOMPARE(status.value("scope_name").toString(),QString("Espécie, global"));
        }
        if (s.educationalKingdom=="Plantae" || s.educationalKingdom=="Protista" || s.educationalKingdom=="Monera")
            QVERIFY(!profile.value("nutrition").toMap().isEmpty());
        if (s.educationalKingdom=="Monera")
            QVERIFY(profile.value("morphology").toMap().value("text").toString().contains("apenas apoio"));
        QVERIFY2(!content.hint(s,"genus",{s.answerAtRank("genus")}).isEmpty(),qPrintable(s.name));
        QVERIFY(!content.curiosity(s).isEmpty());
        if (s.name=="Panthera leo") {
            QCOMPARE(profile.value("commonName").toString(),QString("Leão"));
            QVERIFY(profile.value("distribution").toMap().value("text").toString().contains("Índia"));
            QCOMPARE(status.value("status").toString(),QString("VU"));
        }
        if (s.name=="Incilius periglenes") QCOMPARE(status.value("status").toString(),QString("EX"));
        if (s.name=="Danaus plexippus") QCOMPARE(status.value("status").toString(),QString("LC"));
        Species reused=s; reused.name="Unrelated replacement";
        QVERIFY(content.profile(reused).isEmpty());
    }
    QCOMPARE(profiles,122);
    QCOMPARE(confirmed,22);
    Species missing; missing.id=9999;
    QVERIFY(content.profile(missing).isEmpty());
}

void GameServiceTests::exactHintScoringAndLongGames()
{
    JsonSpeciesCatalog source;
    MemoryCatalog catalog;
    for(const auto &s : source.loadSpecies())if(QSet<int>{63,67,73,135}.contains(s.id))catalog.items.append(s);
    QCOMPARE(catalog.items.size(),4);
    GameService service(&catalog,nullptr,EducationalContent(progressiveFixture()));
    service.startNewGame({},"genus");
    QVERIFY(service.revealHint(HintType::Morphological));QCOMPARE(service.possibleScoreUnits(),6);
    QVERIFY(!service.revealHint(HintType::Morphological));QCOMPARE(service.possibleScoreUnits(),6);
    QCOMPARE(service.revealedHints(),1); // A legacy category shortcut never reveals another same-category ID.
    std::array<int,3> order{0,1,2};
    do { for(int mask=0;mask<8;++mask)for(bool correct : {true,false}) {
        service.startNewGame({},"genus");
        QCOMPARE(service.possibleScoreUnits(),6);
        int previous=6;
        for(int index : order) {if(mask & (1<<index)) {
            QVERIFY2(service.currentQuestion().hints[index].available(),qPrintable(service.currentQuestion().species.name));
            QVERIFY(service.revealHintSlot(index));
            QVERIFY(service.possibleScoreUnits()<=previous);previous=service.possibleScoreUnits();
            QVERIFY(!service.revealHintSlot(index));
        }}
        const auto question=service.currentQuestion();
        QString answer=question.correctAnswer;
        if(!correct)for(const auto &option : question.options)if(option!=answer){answer=option;break;}
        QCOMPARE(service.submitAnswer(answer),correct);
        const int maximum=maximumScoreUnits(mask,hintCeilings(question.hints));
        QCOMPARE(service.scoreUnits(),qint64(correct ? maximum : 0));
        QCOMPARE(service.correctAnswers(),correct ? 1 : 0);
        QCOMPARE(service.answers().last().maximumUnits,maximum);
        QVERIFY(validAnswerScoring(service.answers().last(),CurrentScoringRules));
        QCOMPARE(service.answers().last().revealedHints,mask);
        QCOMPARE(service.revealedHints(),0);QCOMPARE(service.possibleScoreUnits(),6);
    }} while (std::next_permutation(order.begin(),order.end()));
    service.startNewGame({},"genus");
    for(int i=0;i<600;++i) {
        QVERIFY(service.revealHintSlot(0));
        QCOMPARE(service.possibleScoreUnits(),6); // light slot
        QVERIFY(service.revealHintSlot(2));QCOMPARE(service.possibleScoreUnits(),2);
        QVERIFY(service.revealHintSlot(1));QCOMPARE(service.possibleScoreUnits(),2);
        QVERIFY(service.submitAnswer(service.currentQuestion().correctAnswer));
        QCOMPARE(service.scoreUnits(),qint64((i+1)*2));
    }
    QCOMPARE(service.correctAnswers(),600);QCOMPARE(formatScore(service.scoreUnits()),QString("200,00"));
    QCOMPARE(formatScore(2),QString("0,33"));QCOMPARE(formatScore(3),QString("0,50"));QCOMPARE(formatScore(6),QString("1,00"));
}

void GameServiceTests::fractionalHistoryMigrationAndAnswerLedger()
{
    QTemporaryDir dir;const QString path=dir.filePath("fractional.sqlite");
    const QString connection="fractional-legacy";
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE history(id INTEGER PRIMARY KEY,player_name TEXT NOT NULL,score INTEGER NOT NULL,correct_answers INTEGER NOT NULL,played_at TEXT NOT NULL)"));
        QVERIFY(q.exec("CREATE UNIQUE INDEX history_player_name_unique ON history(player_name COLLATE NOCASE)"));
        QVERIFY(q.exec("INSERT INTO history VALUES(1,'Antigo',10,10,'2020-01-01T00:00:00Z')"));
        QVERIFY(q.exec("CREATE TABLE player_profiles(name TEXT PRIMARY KEY COLLATE NOCASE)"));
        QVERIFY(q.exec("INSERT INTO player_profiles VALUES('Antigo')"));
    }
    QSqlDatabase::removeDatabase(connection);
    QVector<AnswerRecord> answers;
    for(int i=1;i<=3;++i)answers.append({i,73,"Coprinus comatus","photo","genus","Coprinus","Coprinus",true,4,2,2});
    {
        SqliteGameHistoryRepository history(path);
        QCOMPARE(history.loadTopResults().first().scoreUnits,qint64(60));
        QVERIFY(history.saveResult(testPlayer(history, "Novo"),6,3,answers,"game-one"));
        QVERIFY(history.saveResult(testPlayer(history, "Novo"),6,3,answers,"game-one"));
        QCOMPARE(history.loadTopResults().size(),2);
        auto inspect=QSqlDatabase::addDatabase("QSQLITE","fractional-inspect");inspect.setDatabaseName(path);QVERIFY(inspect.open());
        QSqlQuery q(inspect);QVERIFY(q.exec("SELECT COUNT(*),SUM(awarded_units),SUM(is_correct) FROM answer_records"));QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(),3);QCOMPARE(q.value(1).toInt(),6);QCOMPARE(q.value(2).toInt(),3);
        QVERIFY(q.exec("CREATE TRIGGER reject_score BEFORE UPDATE ON history BEGIN SELECT RAISE(FAIL,'blocked'); END"));
        QVERIFY(!history.saveResult(testPlayer(history, "Novo"),2,1,{answers.first()},"game-two"));
        QVERIFY(q.exec("SELECT COUNT(*) FROM game_runs WHERE id='game-two'"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);
        QVERIFY(q.exec("DROP TRIGGER reject_score"));
        QVERIFY(history.saveResult(testPlayer(history, "Frações"),5,2));
        QVERIFY(history.saveResult(testPlayer(history, "Mais frações"),4,2));
        QVERIFY(history.saveResult(testPlayer(history, "novo"),2,1,{answers.first()},"game-two"));
        auto rows=history.loadTopResults();QCOMPARE(rows.size(),4);QCOMPARE(rows.last().scoreUnits,qint64(4));
        QCOMPARE(rows[1].scoreUnits,qint64(6));QCOMPARE(rows[2].scoreUnits,qint64(5));
        QCOMPARE(history.loadPlayerProfiles(),(QStringList{"Antigo","Frações","Mais frações","Novo"}));
    }
    QSqlDatabase::removeDatabase("fractional-inspect");
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadTopResults().first().scoreUnits,qint64(60));
    QCOMPARE(reopened.loadTopResults().last().scoreUnits,qint64(4));
}

void GameServiceTests::hintContentMatchesQuestionAndAlternatives()
{
    JsonSpeciesCatalog catalog;EducationalContent content;const auto all=catalog.loadSpecies();
    QJsonObject coverage;
    for(const QString &rank : {QString("species"),QString("genus"),QString("family"),QString("order"),QString("class"),QString("phylum"),QString("kingdom")}) {
        const auto pool=eligibleQuizSpecies(all,rank);QSet<int> taxonomic,etymological,morphological;
        for(const auto &s : pool) {
            QStringList options{s.answerAtRank(rank)};
            for(const auto &candidate : pool)if(!options.contains(candidate.answerAtRank(rank))) {
                options.append(candidate.answerAtRank(rank));if(options.size()==4)break;
            }
            const auto hints=content.questionHints(s,rank,options,pool);
            for(int index=0;index<3;++index) {
                const auto &hint=hints[index];
                if(!hint.available()){QVERIFY(!hint.unavailableReason.isEmpty());continue;}
                AnswerRecord check{1,s.id,s.name,s.photo.id,rank,s.answerAtRank(rank),s.answerAtRank(rank),true,1<<index,hint.ceiling(),hint.ceiling()};
                check.rulesVersion=CurrentScoringRules;check.options=options;check.contentVersion=content.pedagogyVersion();
                check.ceilings[index]=hint.ceiling();check.assessmentIds[index]=hint.assessmentId;check.assessmentPayloads[index]=hint.assessmentPayload;
                QVERIFY2(validAnswerScoring(check,CurrentScoringRules),qPrintable(s.name));
                QVERIFY(hint.sourceUrl.startsWith("https://"));QVERIFY(!hint.text.contains(s.answerAtRank(rank),Qt::CaseInsensitive));
                if(hint.type==HintType::Taxonomic)taxonomic.insert(s.id);
                if(hint.type==HintType::Etymological)etymological.insert(s.id);
                if(hint.type==HintType::Morphological)morphological.insert(s.id);
            }
            auto wrong=s;wrong.id=999999;
            QVERIFY(!content.questionHints(wrong,rank,options,pool)[1].available());
            wrong=s;wrong.photo.id="changed";QVERIFY(!content.questionHints(wrong,rank,options,pool)[2].available());
            wrong=s;wrong.photo.sha256="changed";QVERIFY(!content.questionHints(wrong,rank,options,pool)[2].available());
        }
        coverage[rank]=QJsonObject{{"taxonomic",taxonomic.size()},{"etymological",etymological.size()},{"morphological",morphological.size()},{"eligible",pool.size()}};
    }
    QFile report("hint-coverage.json");QVERIFY(report.open(QIODevice::WriteOnly));report.write(QJsonDocument(coverage).toJson());
}

void GameServiceTests::performanceUsesIdsAndHandlesMissingClassification()
{
    TaxonomicPerformance performance;
    for (const QString &kingdom : QStringList{"Animalia","Plantae","Fungi","Amoebozoa","Bacteria"}) {
        Species s;
        s.taxonomy={{"group:"+kingdom,kingdom,kingdom=="Bacteria" ? "domain" : kingdom=="Amoebozoa" ? "supergroup" : "kingdom"},
                    {"id:"+kingdom,"Same label","phylum"}};
        performance.record(s,true);
    }
    QCOMPARE(performance.groups.size(),5);
    Species s;
    const Taxon animal{"group:Animalia","Animalia","kingdom"};
    s.taxonomy={animal,{"different:id","Same label","phylum"}};
    performance.record(s,false);
    QCOMPARE(performance.groups["group:Animalia"].phyla.size(),2);
    s.taxonomy={animal}; performance.record(s,true);
    s.taxonomy={animal,{"p:a","A","phylum"},{"p:b","B","phylum"}};
    performance.record(s,false);
    const auto unknown=performance.groups["group:Animalia"].phyla["unclassified:group:Animalia"];
    QCOMPARE(unknown.name,QStringLiteral("Classificação não disponível"));
    QCOMPARE(unknown.answered,2); QCOMPARE(unknown.correct,1);
    QCOMPARE(performance.groups["group:Animalia"].answered,4);
    s.taxonomy={{"group:Plantae","Plantae","kingdom"},{"division:test","Validated division","division"}};
    performance.record(s,true);
    QCOMPARE(performance.groups["group:Plantae"].phyla["division:test"].answered,1);
    s.taxonomy.clear(); performance.record(s,true);
    QCOMPARE(performance.groups["unclassified"].answered,1);
}

void GameServiceTests::exportVerifiedCatalogAudit()
{
    JsonSpeciesCatalog catalog;
    EducationalContent content;
    const auto species=catalog.loadSpecies();
    QJsonArray rows;
    for (const Species &s : species) {
        rows.append(QJsonObject{{"id",s.id},{"name",s.name},{"kingdom",s.educationalKingdom},
            {"valid_photo",s.photo.valid},{"photo_id",s.photo.id},{"error",s.photo.error},
            {"width",s.photo.width},{"height",s.photo.height},{"approved_ranks",QJsonArray::fromStringList(s.photo.approvedRanks)}});
    }
    QJsonObject ranks;
    for (const QString &rank : QStringList{"species","genus","family","order","class","phylum","kingdom"}) {
        const auto pool=eligibleQuizSpecies(species,rank);
        int hints=0,curiosities=0; QJsonObject kingdoms; QJsonArray ids;
        for (const Species &s : pool) {
            QStringList options;
            for (const Species &other : pool) options.append(other.answerAtRank(rank));
            hints+=!content.hint(s,rank,options).isEmpty();
            curiosities+=!content.curiosity(s).isEmpty();
            ids.append(s.id); kingdoms[s.educationalKingdom]=kingdoms.value(s.educationalKingdom).toInt()+1;
        }
        ranks[rank]=QJsonObject{{"eligible",pool.size()},{"with_safe_hint",hints},{"with_curiosity",curiosities},{"kingdoms",kingdoms},{"ids",ids}};
    }
    QFile output("catalog-audit.json"); QVERIFY(output.open(QIODevice::WriteOnly));
    QVERIFY(output.write(QJsonDocument(QJsonObject{{"catalog_size",species.size()},{"species",rows},{"ranks",ranks}}).toJson())>0);
}

void GameServiceTests::replacementsRetireOldSpeciesWithoutDeletingHistory()
{
    QTemporaryDir dir;
    const QString path=dir.filePath("catalog.sqlite");
    SqliteGameHistoryRepository history(dir.filePath("history.sqlite"));
    QVERIFY(history.addPlayerProfile("Ada")); QVERIFY(history.saveResult(testPlayer(history, "Ada"),17,17));
    QFile file(":/resources/data/catalog_replacements.json"); QVERIFY(file.open(QIODevice::ReadOnly));
    const auto replacement=QJsonDocument::fromJson(file.readAll()).object().value("replacements").toArray().first().toObject();
    const auto old=replacement.value("old").toObject();
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","retirement_seed"); db.setDatabaseName(path); QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE taxons(id TEXT PRIMARY KEY, scientific_name TEXT NOT NULL, rank TEXT NOT NULL,parent_id TEXT REFERENCES taxons(id), source TEXT,source_taxon_id TEXT,UNIQUE(source,source_taxon_id))"));
        QVERIFY(q.exec("CREATE TABLE species(id INTEGER PRIMARY KEY,scientific_name TEXT NOT NULL,common_name TEXT,taxon_id TEXT NOT NULL REFERENCES taxons(id),educational_kingdom TEXT,image_path TEXT,image_author TEXT,image_license TEXT,image_source TEXT,source TEXT,source_taxon_id TEXT)"));
        QVERIFY(q.exec("INSERT INTO taxons VALUES('retired:leaf','Archived taxon','species',NULL,'local',NULL)"));
        q.prepare("INSERT INTO species(id,scientific_name,taxon_id,source) VALUES(?,?,'retired:leaf','legacy')");
        q.addBindValue(old.value("id").toInt()); q.addBindValue(old.value("name").toString()); QVERIFY(q.exec());
    }
    QSqlDatabase::removeDatabase("retirement_seed");
    SqliteSpeciesCatalog catalog(path); QVERIFY(catalog.isReady());
    const auto active=catalog.loadSpecies();
    QCOMPARE(active.size(),122);
    QVERIFY(std::none_of(active.cbegin(),active.cend(),[&old](const Species &s){ return s.id==old.value("id").toInt(); }));
    QVERIFY(std::any_of(active.cbegin(),active.cend(),[&replacement](const Species &s){ return s.id==replacement.value("new_id").toInt(); }));
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","retirement_check"); db.setDatabaseName(path); QVERIFY(db.open());
        QSqlQuery q(db); q.prepare("SELECT active,scientific_name FROM species WHERE id=?");q.addBindValue(old.value("id").toInt());
        QVERIFY(q.exec() && q.next()); QCOMPARE(q.value(0).toInt(),0); QCOMPARE(q.value(1).toString(),old.value("name").toString());
        QVERIFY(q.exec("PRAGMA foreign_key_check")); QVERIFY(!q.next());
    }
    QSqlDatabase::removeDatabase("retirement_check");
    SqliteSpeciesCatalog reopened(path); QVERIFY(reopened.isReady()); QCOMPARE(reopened.loadSpecies().size(),122);
    QCOMPARE(history.loadPlayerProfiles(),QStringList{"Ada"}); QCOMPARE(history.loadTopResults(5).first().scoreUnits,17);
    Species wrong=active.first(); wrong.photo.role="supporting";wrong.photo.supportingFor=wrong.name;
    PhotoResolver::attach(wrong,wrong.photo); QVERIFY(!wrong.photo.valid);
}

void GameServiceTests::competitiveLifecycleAndConfigurationIsolation()
{
    QTemporaryDir directory;
    JsonSpeciesCatalog catalog;
    SqliteGameHistoryRepository history(directory.filePath("modes.sqlite"));
    GameService game(&catalog,&history);
    const auto configuration=GameService::competitiveConfiguration();
    QCOMPARE(configuration.questionLimit,20);
    game.startConfiguredGame(configuration,history.createPlayer("Ada").identity());
    QVERIFY(!game.isFinished());
    QSet<int> seen;
    for (int i=0;i<20;++i) {
        const auto question=game.currentQuestion();
        QVERIFY(!seen.contains(question.species.id)); seen.insert(question.species.id);
        QCOMPARE(question.photo.role,QString("identification"));
        QVERIFY(question.species.photoApproved);
        QCOMPARE(question.identificationRank,QString("genus"));
        QCOMPARE(QSet<QString>(question.options.begin(),question.options.end()).size(),4);
        QCOMPARE(question.options.count(question.correctAnswer),1);
        QVERIFY(game.submitAnswer(question.correctAnswer));
        QCOMPARE(game.isFinished(),i==19);
    }
    QCOMPARE(game.scoreUnits(),qint64(120)); QCOMPARE(game.correctAnswers(),20);
    QVERIFY(!game.submitAnswer("anything")); QCOMPARE(game.answeredCount(),20);
    QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
    QVERIFY(history.loadTopResults().isEmpty());
    QCOMPARE(history.loadRanking(configuration.rankingKey()).first().scoreUnits,qint64(120));
    QCOMPARE(history.playerMetrics(configuration.rankingKey(),history.profileNamed("ada").id).answered,20);
    QCOMPARE(history.playerMetrics(configuration.rankingKey(),history.profileNamed("Ada").id).unassistedCorrect,20);
    auto changed=configuration; changed.questionLimit=10;
    QVERIFY(history.loadRanking(changed.rankingKey()).isEmpty());
    changed=configuration; changed.difficulty=QuestionDifficulty::Close;
    QVERIFY(history.loadRanking(changed.rankingKey()).isEmpty());
    changed=configuration; changed.contentVersion="other-bundle";
    QVERIFY(history.loadRanking(changed.rankingKey()).isEmpty());
    game.startConfiguredGame(configuration,history.createPlayer("Lia").identity());
    QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
    game.finishGame(); QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
    QCOMPARE(history.loadRanking(configuration.rankingKey()).size(),1);
    QCOMPARE(history.playerMetrics(configuration.rankingKey(),history.profileNamed("Lia").id).answered,0);
    game.startNewGame({},"genus",history.profileNamed("Ada").identity());
    for(int i=0;i<30;++i) QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
    QVERIFY(!game.isFinished()); game.finishGame();
    QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
    QCOMPARE(history.loadTopResults().first().scoreUnits,qint64(180));
    QCOMPARE(history.playerMetrics("free",history.profileNamed("Ada").id).answered,30);
    auto records=game.answers();
    QVERIFY(history.saveConfiguredResult(testPlayer(history, "Ada"),180,30,records,"retry",{}));
    records.first().speciesId+=100;
    QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),180,30,records,"retry",{}));
    MemoryCatalog tiny; tiny.items=catalog.loadSpecies().mid(0,4);
    GameService tooSmall(&tiny); tooSmall.startConfiguredGame(configuration);
    QVERIFY(tooSmall.isFinished());
}

void GameServiceTests::questionPoliciesAndReviewGates()
{
    JsonSpeciesCatalog catalog; EducationalContent content;
    const auto all=catalog.loadSpecies();
    std::mt19937 random(1234); QuestionGenerator generator(content);
    for(const QString &rank : QStringList{"species","genus","family","order","class","phylum","kingdom"})
        for(const auto difficulty : {QuestionDifficulty::Broad,QuestionDifficulty::Balanced,QuestionDifficulty::Close}) {
            auto pool=eligibleQuizSpecies(all,rank);
            GameConfiguration config; config.rank=rank;config.difficulty=difficulty;
            for(const auto &s : pool) {
                const auto question=generator.generate(s,pool,config,random);
                if(question.options.isEmpty()) { QCOMPARE(difficulty,QuestionDifficulty::Close); continue; }
                QCOMPARE(question.options.size(),4); QCOMPARE(question.options.count(question.correctAnswer),1);
                QSet<QString> choices;
                for(const auto &option : question.options) {
                    choices.insert(option.toCaseFolded());
                    QVERIFY(std::any_of(pool.begin(),pool.end(),[&](const Species &candidate){return candidate.answerAtRank(rank)==option;}));
                }
                QCOMPARE(choices.size(),4);
            }
        }
    auto rejected=all;
    for(auto &s : rejected)s.taxonomyApproved=false;
    QVERIFY(eligibleQuizSpecies(rejected,"genus").isEmpty());
    for(auto &s : rejected){s.taxonomyApproved=true;s.photoApproved=false;}
    QVERIFY(eligibleQuizSpecies(rejected,"genus").isEmpty());
    for(const auto &s : all)if(s.id==20) QVERIFY(!s.photoApproved);
    GameService game(&catalog);
    for(const auto difficulty : {QuestionDifficulty::Broad,QuestionDifficulty::Balanced,QuestionDifficulty::Close}) {
        GameConfiguration config;config.group="edu:Fungi";config.rank="genus";config.difficulty=difficulty;
        game.startConfiguredGame(config);
        for(int i=0;i<game.catalogSize() && !game.isFinished();++i) {
            QCOMPARE(game.currentQuestion().species.educationalKingdom,QString("Fungi"));
            QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
        }
    }
}

void GameServiceTests::answerMetricsDoNotDoubleCount()
{
    QVector<AnswerRecord> answers;
    answers.append({1,1,"A","photo","genus","A","A",true,0,6,6});
    answers.append({2,2,"B","photo","genus","B","B",true,7,2,2});
    answers.append({3,3,"C","photo","genus","D","C",false,3,3,0});
    const auto metrics=SessionMetrics::fromAnswers(answers);
    QCOMPARE(metrics.answered,3);QCOMPARE(metrics.correct,2);QCOMPARE(metrics.units,qint64(8));
    QCOMPARE(metrics.unassistedCorrect,1);QCOMPARE(metrics.hintedQuestions,2);QCOMPARE(metrics.revealedCategories,5);
    QCOMPARE(metrics.categoryUse[0],2);QCOMPARE(metrics.categoryUse[1],2);QCOMPARE(metrics.categoryUse[2],1);
    QVERIFY(qAbs(metrics.accuracy()-200.0/3)<0.0001);QVERIFY(qAbs(metrics.efficiency()-800.0/18)<0.0001);
    QCOMPARE(metrics.averageText(),QString("0,44"));
    QCOMPARE(SessionMetrics::fromAnswers({}).averageText(),QString("0,00"));
}

void GameServiceTests::versionedMigrationReconstructsKnownRecords()
{
    QTemporaryDir directory;const auto path=directory.filePath("v1.sqlite");
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","v1-seed");db.setDatabaseName(path);QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE history(id INTEGER PRIMARY KEY,player_name TEXT,score_units INTEGER,correct_answers INTEGER,played_at TEXT)"));
        QVERIFY(q.exec("INSERT INTO history VALUES(1,'Ada',2,1,'2026-01-02T00:00:00Z')"));
        QVERIFY(q.exec("CREATE TABLE player_profiles(name TEXT PRIMARY KEY COLLATE NOCASE)"));
        QVERIFY(q.exec("INSERT INTO player_profiles VALUES('Ada')"));
        QVERIFY(q.exec("CREATE TABLE game_runs(id TEXT PRIMARY KEY,player_name TEXT NOT NULL,finished_at TEXT NOT NULL)"));
        QVERIFY(q.exec("CREATE TABLE answer_records(game_id TEXT,question_number INTEGER,species_id INTEGER,scientific_name TEXT,photo_id TEXT,rank TEXT,selected_answer TEXT,correct_answer TEXT,is_correct INTEGER,revealed_hints INTEGER,maximum_units INTEGER,awarded_units INTEGER,PRIMARY KEY(game_id,question_number))"));
        QVERIFY(q.exec("INSERT INTO game_runs VALUES('known','Ada','2026-01-01T00:00:00Z')"));
        QVERIFY(q.exec("INSERT INTO answer_records VALUES('known',1,73,'Coprinus comatus','photo','genus','Coprinus','Coprinus',1,0,6,6)"));
    }
    QSqlDatabase::removeDatabase("v1-seed");
    {
        SqliteGameHistoryRepository history(path);
        const auto rows=history.loadTopResults(); QVERIFY(!rows.isEmpty()); const auto row=rows.first();
        QCOMPARE(row.scoreUnits,qint64(6));QCOMPARE(row.lastScoreUnits,qint64(2));
        QCOMPARE(history.loadPlayerProfiles(),QStringList{"Ada"});
    }
    { SqliteGameHistoryRepository reopened(path); const auto rows=reopened.loadTopResults();
      QVERIFY(!rows.isEmpty()); QCOMPARE(rows.first().scoreUnits,qint64(6)); }
    auto db=QSqlDatabase::addDatabase("QSQLITE","v2-inspect");db.setDatabaseName(path);QVERIFY(db.open());
    {
        QSqlQuery q(db);QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),6);
        QVERIFY(q.exec("SELECT COUNT(*) FROM answer_records"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),1);
        q.finish();
        // The supported legacy table has no declared FK. Orphans must still be
        // refused on reopen, with every record retained for investigation.
        QVERIFY(q.exec("INSERT INTO answer_records SELECT 'missing-game',question_number,species_id,scientific_name,photo_id,rank,selected_answer,correct_answer,is_correct,revealed_hints,maximum_units,awarded_units,rules_version,hint_snapshot FROM answer_records WHERE game_id='known'"));
        { SqliteGameHistoryRepository refused(path); QVERIFY(refused.profiles().isEmpty()); }
        QVERIFY(q.exec("SELECT COUNT(*) FROM answer_records")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),2);
    }
    db.close();db={};QSqlDatabase::removeDatabase("v2-inspect");
}

void GameServiceTests::failedSchemaMigrationPreservesOriginalDatabase()
{
    QTemporaryDir directory; const QString path=directory.filePath("rollback.sqlite");
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","rollback-seed"); db.setDatabaseName(path); QVERIFY(db.open());
        QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE history(id INTEGER PRIMARY KEY,player_name TEXT,score INTEGER,correct_answers INTEGER,played_at TEXT)"));
        QVERIFY(q.exec("INSERT INTO history VALUES(1,'Ada',25,25,'2026-01-01T00:00:00Z')"));
        QVERIFY(q.exec("CREATE TRIGGER reject_migration BEFORE UPDATE ON history BEGIN SELECT RAISE(FAIL,'migration interrupted'); END"));
    }
    QSqlDatabase::removeDatabase("rollback-seed");
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.loadTopResults().isEmpty()); }
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","rollback-inspect");db.setDatabaseName(path);QVERIFY(db.open());
        QSqlQuery q(db);QVERIFY(q.exec("SELECT score FROM history"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),25);q.finish();
        QVERIFY(q.exec("PRAGMA user_version"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);q.finish();
        QVERIFY(q.exec("DROP TRIGGER reject_migration"));
    }
    QSqlDatabase::removeDatabase("rollback-inspect");
    SqliteGameHistoryRepository recovered(path);
    QCOMPARE(recovered.loadTopResults().first().scoreUnits,qint64(150));
    QVERIFY(recovered.saveResult(testPlayer(recovered, "Ada"),60,10));
    QVERIFY(recovered.saveResult(testPlayer(recovered, "Ada"),102,17));
    QCOMPARE(recovered.loadTopResults().first().scoreUnits,qint64(150));
    QCOMPARE(recovered.loadPlayerResult("free",recovered.profileNamed("Ada").id).first().lastScoreUnits,qint64(102));
    QVERIFY(recovered.saveResult(testPlayer(recovered, "Bia"),150,25));
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","tie-inspect");db.setDatabaseName(path);QVERIFY(db.open());
        QSqlQuery q(db);QVERIFY(q.exec("UPDATE ranking_records SET best_played_at='2026-01-01T00:00:00Z'"));
    }
    QSqlDatabase::removeDatabase("tie-inspect");
    QCOMPARE(recovered.loadTopResults().first().playerName,QString("Ada"));
}

void GameServiceTests::offlineReviewExpiryAndDrafts()
{
    const auto now = QDateTime::currentDateTimeUtc();
    QJsonObject review{{"state", "approved_editorial"}, {"usable", true}, {"reviewer", "test-human"},
                       {"reviewed_at", now.addDays(-1).toString(Qt::ISODateWithMs)},
                       {"due_at", now.addDays(30).toString(Qt::ISODateWithMs)}};
    QVERIFY(usableReview(review));
    review.insert("due_at", now.addDays(-1).toString(Qt::ISODateWithMs));
    QVERIFY(!usableReview(review));
    review.insert("state", "draft");
    review.insert("due_at", now.addDays(30).toString(Qt::ISODateWithMs));
    QVERIFY(!usableReview(review));
    review.insert("state", "approved_expert"); review.remove("reviewer");
    QVERIFY(!usableReview(review));
}

void GameServiceTests::contextualAssistanceAndUnknownDistractors()
{
    JsonSpeciesCatalog source;QVector<Species> pool;
    for (const auto &s : source.loadSpecies()) if (QSet<int>{63,67,73,135}.contains(s.id)) pool.append(s);
    QCOMPARE(pool.size(),4);
    const auto correct=pool.first();QStringList options;
    for (const auto &s : pool) options.append(s.answerAtRank("genus"));
    auto data=gradedHintFixture();EducationalContent content(data);
    auto hints=content.questionHints(correct,"genus",options,pool,true);
    QVERIFY(hints[0].available());QCOMPARE(hints[0].assistance,AssistanceLevel::Moderate);
    QVERIFY(hints[1].available());QCOMPARE(hints[1].assistance,AssistanceLevel::Strong);
    QVERIFY(hints[2].available());QCOMPARE(hints[2].assistance,AssistanceLevel::Light);
    auto notes=data["notes"].toObject();
    const auto species=data["species"].toObject()[QString::number(correct.id)].toObject();
    const QString etymologyKey=species["etymology_hints"].toObject()["genus"].toString();
    auto etymology=notes[etymologyKey].toObject(),assessment=etymology["assistance"].toObject();
    assessment["grade"]="light";
    bindFixtureAssessment(assessment,QString::number(correct.id),correct.name,correct.photo.id,"etymological");
    etymology["assistance"]=assessment;notes[etymologyKey]=etymology;data["notes"]=notes;
    QCOMPARE(EducationalContent(data).questionHints(correct,"genus",options,pool,true)[1].assistance,AssistanceLevel::Light);
    auto review=assessment["governance"].toObject();review["state"]="draft";review["usable"]=false;
    assessment["governance"]=review;etymology["assistance"]=assessment;notes[etymologyKey]=etymology;data["notes"]=notes;
    QVERIFY(EducationalContent(data).questionHints(correct,"genus",options,pool)[1].available());
    auto pending=EducationalContent(data).questionHints(correct,"genus",options,pool,true)[1];
    QVERIFY(!pending.available());QVERIFY(pending.unavailableReason.contains("pendente"));
    QVERIFY(!EducationalContent(data).nameOrigin(correct).isEmpty()); // learning survives assistance draft
    review["state"]="approved_editorial";review["usable"]=true;
    review["due_at"]=QDateTime::currentDateTimeUtc().addDays(-1).toString(Qt::ISODateWithMs);
    assessment["governance"]=review;etymology["assistance"]=assessment;notes[etymologyKey]=etymology;data["notes"]=notes;
    QVERIFY(!EducationalContent(data).questionHints(correct,"genus",options,pool)[1].available());
    etymology.remove("assistance");notes[etymologyKey]=etymology;data["notes"]=notes;
    QVERIFY(!EducationalContent(data).questionHints(correct,"genus",options,pool)[1].available());

    const QString morphologyKey=species["morphological_hint"].toString();
    auto morphology=notes[morphologyKey].toObject(),diagnostic=morphology["assistance"].toObject();
    diagnostic["grade"]="strong";diagnostic["context"]="diagnostic_contrast";
    QJsonObject evidence;
    for (const auto &option : options) evidence[option]=QJsonObject{{"rank","genus"},{"compatible",option==correct.answerAtRank("genus")},
        {"source_url","https://example.org/synthetic-comparison"},{"locator","test-only documented comparison"}};
    diagnostic["option_evidence"]=evidence;
    bindFixtureAssessment(diagnostic,QString::number(correct.id),correct.name,correct.photo.id,"morphological");
    morphology["assistance"]=diagnostic;notes[morphologyKey]=morphology;data["notes"]=notes;
    auto strong=EducationalContent(data).questionHints(correct,"genus",options,pool,true)[2];
    QVERIFY(strong.available());QCOMPARE(strong.ceiling(),2);
    evidence.remove(options.last());diagnostic["option_evidence"]=evidence;
    bindFixtureAssessment(diagnostic,QString::number(correct.id),correct.name,correct.photo.id,"morphological");
    morphology["assistance"]=diagnostic;notes[morphologyKey]=morphology;data["notes"]=notes;
    QVERIFY(!EducationalContent(data).questionHints(correct,"genus",options,pool,true)[2].available());
    auto unknown=pool;unknown.last().taxonomy.clear();
    QVERIFY(!content.questionHints(correct,"genus",options,unknown,true)[0].available());

    EducationalContent actual;
    for(const auto &s:source.loadSpecies())if(QSet<int>{67,131,133}.contains(s.id)) {
        const auto choices=QStringList{s.name,"Example alpha","Example beta","Example gamma"};
        const auto h=actual.questionHints(s,"species",choices,source.loadSpecies(),true);
        QVERIFY(h[2].available());QCOMPARE(h[2].assistance,AssistanceLevel::Strong);
        QCOMPARE(h[2].assessmentState,QString("approved_automated"));
    }

}

void GameServiceTests::assistanceSnapshotsAndLegacyV2Migration()
{
    QTemporaryDir directory;const auto path=directory.filePath("v2-assistance.sqlite");
    GameConfiguration old;old.mode=GameMode::Competitive;old.questionLimit=1;old.rank="genus";
    old.contentVersion="old-content";old.rulesVersion=LegacyScoringRules;
    {
        auto db=QSqlDatabase::addDatabase("QSQLITE","assistance-v2");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
        QVERIFY(q.exec("CREATE TABLE history(id INTEGER PRIMARY KEY,player_name TEXT,score_units INTEGER,correct_answers INTEGER,played_at TEXT)"));
        QVERIFY(q.exec("CREATE TABLE game_runs(id TEXT PRIMARY KEY,player_name TEXT NOT NULL,finished_at TEXT NOT NULL,rules_version TEXT NOT NULL,ranking_key TEXT NOT NULL,score_units INTEGER,correct_answers INTEGER,answered_count INTEGER,is_complete INTEGER)"));
        q.prepare("INSERT INTO game_runs VALUES('old','Ada','2026-01-01T00:00:00Z',?,?,3,1,1,1)");q.addBindValue(LegacyScoringRules);q.addBindValue(old.rankingKey());QVERIFY(q.exec());
        QVERIFY(q.exec("CREATE TABLE answer_records(game_id TEXT,question_number INTEGER,species_id INTEGER,scientific_name TEXT,photo_id TEXT,rank TEXT,selected_answer TEXT,correct_answer TEXT,is_correct INTEGER,revealed_hints INTEGER,maximum_units INTEGER,awarded_units INTEGER,PRIMARY KEY(game_id,question_number))"));
        QVERIFY(q.exec("INSERT INTO answer_records VALUES('old',1,67,'Trametes versicolor','photo','genus','Trametes','Trametes',1,2,3,3)"));
        QVERIFY(q.exec("CREATE TABLE ranking_records(ranking_key TEXT,player_name TEXT COLLATE NOCASE,best_units INTEGER,best_correct INTEGER,best_played_at TEXT,last_units INTEGER,last_correct INTEGER,last_played_at TEXT,PRIMARY KEY(ranking_key,player_name))"));
        q.prepare("INSERT INTO ranking_records VALUES(?,'Ada',3,1,'2026-01-01T00:00:00Z',3,1,'2026-01-01T00:00:00Z')");q.addBindValue(old.rankingKey());QVERIFY(q.exec());
        QVERIFY(q.exec("PRAGMA user_version=2"));
    }
    QSqlDatabase::removeDatabase("assistance-v2");
    auto current=old;current.rulesVersion=PreviousScoringRules;
    AnswerRecord answer{1,67,"Trametes versicolor","photo","genus","Trametes","Trametes",true,6,2,2};
    answer.rulesVersion=PreviousScoringRules;answer.ceilings={0,2,6};
    answer.options={"Trametes","Stropharia","Coprinus","Clathrus"};
    for (int i : {1,2}) {
        QJsonObject a{{"grade",i==1 ? "strong" : "light"},{"ranks",QJsonArray{"genus"}},{"context","any_safe_options"}};
        bindFixtureAssessment(a,"67",answer.scientificName,answer.photoId,hintKey(static_cast<HintType>(i+1)));
        answer.assessmentIds[i]=a["governance"].toObject()["content_sha256"].toString();answer.assessmentPayloads[i]=a["binding"].toObject();
    }
    {
        SqliteGameHistoryRepository history(path);
        const auto migrated=history.loadAnswers("old");QCOMPARE(migrated.size(),1);
        QCOMPARE(migrated.first().maximumUnits,3);QCOMPARE(migrated.first().ceilings[1],3);
        QCOMPARE(history.loadRanking(old.rankingKey()).first().scoreUnits,qint64(3));
        QVERIFY(history.loadRanking(current.rankingKey()).isEmpty());
        QVERIFY(history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{answer},"new",current));
        QVERIFY(history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{answer},"new",current));
        auto bad=answer;bad.ceilings[1]=4;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"bad-ceiling",current));
        bad=answer;bad.assessmentIds[1].clear();QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"missing-proof",current));
        bad=answer;bad.maximumUnits=3;bad.awardedUnits=3;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),3,1,{bad},"wrong-cap",current));
        bad=answer;bad.ceilings[1]=6;bad.maximumUnits=6;bad.awardedUnits=6;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),6,1,{bad},"forged-grade",current));
        bad=answer;bad.revealedHints=2;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"unrevealed-charge",current));
        bad=answer;bad.assessmentPayloads[1].insert("scientific_name","Unrelated species");QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"tampered-payload",current));
        bad=answer;bad.rulesVersion=LegacyScoringRules;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"wrong-policy",current));
        bad=answer;bad.ceilings[2]=3;QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Ada"),2,1,{bad},"new",current));
        QCOMPARE(history.playerMetrics(current.rankingKey(),history.profileNamed("Ada").id).correct,1);
        QCOMPARE(history.playerMetrics(current.rankingKey(),history.profileNamed("Ada").id).units,qint64(2));
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadAnswers("old").first().awardedUnits,3);
    QCOMPARE(reopened.loadAnswers("new").first().ceilings[1],2);
    QCOMPARE(reopened.loadAnswers("new").first().ceilings[2],6);
    QCOMPARE(reopened.loadAnswers("new").first().assessmentIds[1],answer.assessmentIds[1]);
    QCOMPARE(reopened.loadAnswers("new").first().options,answer.options);
    QCOMPARE(reopened.loadRanking(old.rankingKey()).first().scoreUnits,qint64(3));
    QCOMPARE(reopened.loadRanking(current.rankingKey()).first().scoreUnits,qint64(2));
}

void GameServiceTests::assistanceChangesCompetitiveContentVersion()
{
    QFile file(":/resources/data/education.json");QVERIFY(file.open(QIODevice::ReadOnly));
    const auto original=file.readAll();auto data=QJsonDocument::fromJson(original).object();
    QCOMPARE(GameService::contentFingerprint(original),GameService::competitiveConfiguration().contentVersion);
    auto notes=data["notes"].toObject();auto note=notes["species:67:etymology:species"].toObject();
    auto assessment=note["assistance"].toObject();assessment["grade"]="light";note["assistance"]=assessment;
    notes["species:67:etymology:species"]=note;data["notes"]=notes;
    QVERIFY(GameService::contentFingerprint(QJsonDocument(data).toJson())!=GameService::contentFingerprint(original));
    auto old=GameService::competitiveConfiguration();old.rulesVersion=LegacyScoringRules;
    QVERIFY(old.rankingKey()!=GameService::competitiveConfiguration().rankingKey());
}

void GameServiceTests::progressiveCoverageHistoryAndReports()
{
    JsonSpeciesCatalog source;EducationalContent content;const auto all=source.loadSpecies();
    QJsonArray rows;int completeCompetitive=0;QSet<QString> coveredGroups;
    for(const QString &rank:{QString("species"),QString("genus"),QString("family"),QString("order"),QString("class"),QString("phylum"),QString("kingdom")}) {
        const auto pool=eligibleQuizSpecies(all,rank);std::mt19937 random(42);
        for(const auto &s:pool) {
            GameConfiguration cfg;cfg.rank=rank;if(rank=="genus")cfg.mode=GameMode::Competitive;
            const auto q=QuestionGenerator(content).generate(s,pool,cfg,random);QCOMPARE(q.options.size(),4);
            QJsonArray hints;int available=0;QSet<QString> ids,categories;
            for(int i=0;i<3;++i) {
                const auto &h=q.hints[i];QCOMPARE(h.ceiling(),i==0?6:i==1?3:2);
                hints.append(QJsonObject{{"slot",i+1},{"id",h.id},{"category",hintKey(h.type)},
                    {"available",h.available()},{"text",h.text},{"reason",h.unavailableReason}});
                if(!h.available())continue;
                ++available;QVERIFY(!ids.contains(h.id));ids.insert(h.id);categories.insert(hintKey(h.type));
                QCOMPARE(h.assessmentState,QString("approved_automated"));
            }
            rows.append(QJsonObject{{"id",s.id},{"name",s.name},{"rank",rank},{"available",available},
                {"repeated_categories",categories.size()<available},
                {"exception_reason",categories.size()<available ? "No suitable, sourced note of another category at the required grade; distinct ancestral contexts retained." : ""},{"hints",hints}});
            if(rank=="genus" && s.photo.role=="identification" && available==3){++completeCompetitive;coveredGroups.insert(s.displayGroup());}
        }
    }
    QVERIFY(completeCompetitive>=20);QVERIFY(coveredGroups.size()>=3);
    QFile coverage("progressive-hint-coverage.json");QVERIFY(coverage.open(QIODevice::WriteOnly));
    coverage.write(QJsonDocument(QJsonObject{{"policy","progressive-pedagogy-v1"},{"complete_competitive",completeCompetitive},
        {"coverage_scope","Seed 42, complete eligible pool per rank; runtime alternatives can change availability."},{"rows",rows}}).toJson());
    QTemporaryDir directory;const auto path=directory.filePath("new-progressive.sqlite");QVector<AnswerRecord> saved;
    {
        SqliteGameHistoryRepository history(path);GameService game(&source,&history);
        game.startConfiguredGame(GameService::competitiveConfiguration(),history.createPlayer("Synthetic Player").identity());
        for(int i=0;i<20;++i){QVERIFY(!game.isFinished());const auto q=game.currentQuestion();
            for(int slot:{2,0,1})if(q.hints[slot].available())QVERIFY(game.revealHintSlot(slot));
            QVERIFY(game.submitAnswer(q.correctAnswer));QVERIFY(validAnswerScoring(game.answers().last(),CurrentScoringRules));}
        QCOMPARE(game.answeredCount(),20);QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
        saved=game.answers();QCOMPARE(history.loadAnswers(game.configuration().rulesVersion).size(),0);
        // Save a known game ID as well to test snapshot round trips and tampering.
        auto cfg=game.configuration();QVERIFY(history.saveConfiguredResult(testPlayer(history, "Synthetic Player"),game.scoreUnits(),20,saved,"snapshot-v4",cfg));
        auto bad=saved;auto &a=bad[0];a.contentVersion="changed";
        QVERIFY(!history.saveConfiguredResult(testPlayer(history, "Synthetic Player"),game.scoreUnits(),20,bad,"bad-v4",cfg));
    }
    {
        SqliteGameHistoryRepository reopened(path);const auto loaded=reopened.loadAnswers("snapshot-v4");QCOMPARE(loaded.size(),20);
        for(int i=0;i<20;++i){QCOMPARE(loaded[i].awardedUnits,saved[i].awardedUnits);QCOMPARE(loaded[i].assessmentIds,saved[i].assessmentIds);QCOMPARE(loaded[i].contentVersion,saved[i].contentVersion);}
    }
    GameController controller(nullptr,directory.filePath("reports.sqlite"));controller.startCompetitiveGame();
    auto hints=controller.hints();int slot=-1;
    for(int i=0;i<3;++i)if(hints[i].toMap()["available"].toBool()){slot=i;break;}
    QVERIFY(slot>=0);const auto id=hints[slot].toMap()["id"].toString();
    QVERIFY(controller.hintReport(id,"test").isEmpty());
    QVERIFY(controller.revealHintCategory(id));const auto score=controller.possibleScoreText();
    const auto report=controller.hintReport(id,"A descrição precisa de correção.");QVERIFY(!report.isEmpty());
    const auto text=report["text"].toString();QVERIFY(text.contains("A descrição precisa de correção."));
    QVERIFY(text.contains(controller.hints()[slot].toMap()["text"].toString()));
    QVERIFY(!text.contains(controller.currentPlayerName()));QVERIFY(!text.contains(directory.path()));
    QVERIFY(!text.contains("correctAnswer"));QCOMPARE(controller.possibleScoreText(),score);
    for(int i=0;i<3;++i)if(i!=slot)QVERIFY(controller.hintReport(hints[i].toMap()["id"].toString(),"test").isEmpty());
}

QTEST_MAIN(GameServiceTests)
#include "GameServiceTests.moc"
