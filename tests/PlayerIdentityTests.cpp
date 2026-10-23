#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include "infrastructure/persistence/PlayerIdentityMigration.h"
#include "domain/PlayerNamePolicy.h"
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QUuid>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlRecord>

namespace {
struct Inspection {
    QSqlDatabase db;
    Inspection(const QString &path) {
        db=QSqlDatabase::addDatabase("QSQLITE",QUuid::createUuid().toString()); db.setDatabaseName(path); db.open();
    }
    ~Inspection() { const auto name=db.connectionName(); db.close(); db={}; QSqlDatabase::removeDatabase(name); }
};
QStringList contents(const QSqlDatabase &db)
{
    QStringList result, tables;
    QSqlQuery q(db);
    if (!q.exec("SELECT name,sql FROM sqlite_master WHERE type='table' ORDER BY name")) return {};
    while (q.next()) { tables.append(q.value(0).toString()); result.append(q.value(1).toString()); }
    for (const auto &table : tables) {
        if (!q.exec("SELECT * FROM \"" + table + "\" ORDER BY rowid")) return {};
        while (q.next()) {
            QVariantList values;
            for (int i=0; i<q.record().count(); ++i) values.append(q.value(i));
            result.append(table + ":" + QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(values)).toJson(QJsonDocument::Compact)));
        }
    }
    return result;
}
bool seedV3(const QString &path)
{
    Inspection inspection(path); QSqlQuery q(inspection.db);
    const QStringList statements{
        "CREATE TABLE player_profiles(name TEXT PRIMARY KEY COLLATE NOCASE)",
        "INSERT INTO player_profiles VALUES('Ada')",
        "CREATE TABLE history(id INTEGER PRIMARY KEY,player_name TEXT NOT NULL,score_units INTEGER NOT NULL,correct_answers INTEGER NOT NULL,played_at TEXT NOT NULL)",
        "INSERT INTO history VALUES(8,'Ada',6,1,'2026-01-02T00:00:00Z')",
        "CREATE TABLE legacy_history_results(original_id INTEGER PRIMARY KEY,player_name TEXT NOT NULL,score_units INTEGER NOT NULL,correct_answers INTEGER NOT NULL,played_at TEXT NOT NULL)",
        "INSERT INTO legacy_history_results VALUES(7,'Ada',12,2,'2026-01-01T00:00:00Z')",
        "CREATE TABLE ranking_records(ranking_key TEXT NOT NULL,player_name TEXT NOT NULL COLLATE NOCASE,best_units INTEGER NOT NULL,best_correct INTEGER NOT NULL,best_played_at TEXT NOT NULL,last_units INTEGER NOT NULL,last_correct INTEGER NOT NULL,last_played_at TEXT NOT NULL,PRIMARY KEY(ranking_key,player_name))",
        "INSERT INTO ranking_records VALUES('free','Ada',12,2,'2026-01-01T00:00:00Z',6,1,'2026-01-02T00:00:00Z')",
        "CREATE TABLE game_runs(id TEXT PRIMARY KEY,player_name TEXT NOT NULL,finished_at TEXT NOT NULL,mode TEXT NOT NULL DEFAULT 'free',question_limit INTEGER NOT NULL DEFAULT 0,group_id TEXT NOT NULL DEFAULT '',rank TEXT NOT NULL DEFAULT '',difficulty TEXT NOT NULL DEFAULT 'balanced',rules_version TEXT NOT NULL DEFAULT 'legacy',content_version TEXT NOT NULL DEFAULT '',ranking_key TEXT NOT NULL DEFAULT 'free',score_units INTEGER,correct_answers INTEGER,answered_count INTEGER,is_complete INTEGER NOT NULL DEFAULT 0)",
        "INSERT INTO game_runs VALUES('old-run','Ada','2026-01-02T00:00:00Z','free',0,'','genus','balanced','legacy','','free',6,1,1,1)",
        "CREATE TABLE answer_records(game_id TEXT NOT NULL REFERENCES game_runs(id),question_number INTEGER,species_id INTEGER,scientific_name TEXT,photo_id TEXT,rank TEXT,selected_answer TEXT,correct_answer TEXT,is_correct INTEGER,revealed_hints INTEGER,maximum_units INTEGER,awarded_units INTEGER,rules_version TEXT,hint_snapshot TEXT,PRIMARY KEY(game_id,question_number))",
        R"sql(INSERT INTO answer_records VALUES('old-run',1,73,'Synthetic organism','legacy-photo','genus','Synthetic','Synthetic',1,0,6,6,'legacy','{"schema":1,"rules":"legacy","ceilings":[0,0,0],"assessment_ids":["","",""],"assessment_payloads":[{},{},{}],"options":[]}'))sql",
        "PRAGMA user_version=3"};
    for (const auto &sql:statements) if (!q.exec(sql)) return false;
    return true;
}
QStringList schema(const QSqlDatabase &db)
{
    QSqlQuery q(db); QStringList result;
    if(q.exec("SELECT type,name,sql FROM sqlite_master ORDER BY type,name"))
        while(q.next()) result.append(q.value(0).toString()+"|"+q.value(1).toString()+"|"+q.value(2).toString());
    return result;
}
}

class PlayerIdentityTests : public QObject {
    Q_OBJECT
private slots:
    void createRenameReopenAndKeepOwnership();
    void retireAndReuseNameWithoutInheritingResults();
    void sameGameCannotChangeOwner();
    void renameAndReusePreviousNameKeepDistinctRankings();
    void invalidIdentityAndNamesAreRefused();
    void migrateV3PreservesUnresolvedRecordsAndSnapshots();
    void migrationIsIdempotentWithoutNewIdsOrNotes();
    void migrationRollbackPreservesOldSchemaAndRecords();
    void legacyUnicodeProfilesStaySeparate();
    void migrationDoesNotClaimRenamedDeletedOrReusedNames();
    void unicodeEquivalentNames_data();
    void unicodeEquivalentNames();
    void unicodeDistinctionsAndDisplaySpelling();
    void unicodeRenameConflictAndOwnCaseChange();
    void migrateV4AuditsConflictsWithoutChangingOwners();
    void migrateV4FailureRollsBackKeysAndGuards();
    void migrateV4RetiredProfilesKeepNamesAndResults();
    void unsupportedPolicyAndInconsistentKeysFailClosed();
    void incompleteCurrentSchemaFailsClosed_data();
    void incompleteCurrentSchemaFailsClosed();
    void incompleteV5MigrationRollsBackAndKeepsResults();
    void activeRankingFiltersBeforeLimitAndPreservesHistory();
    void historicalPagesAndStoredTieBreaks();
    void activeRankingWithNoEligiblePlayers();
};

void PlayerIdentityTests::activeRankingFiltersBeforeLimitAndPreservesHistory()
{
    QTemporaryDir dir; const auto path=dir.filePath("active-ranking.sqlite"); QVERIFY(seedV3(path));
    QString legacyId, retiredId, reusedId;
    QStringList expected, before;
    {
        SqliteGameHistoryRepository repo(path);
        legacyId=repo.loadRanking("free").first().playerId;
        for (int i=0; i<6; ++i) {
            const auto p=i==0 ? repo.profileNamed("Ada") : repo.createPlayer(QString("Ativo %1").arg(i));
            QVERIFY(p.selectable()); QVERIFY(repo.saveResult(p.identity(),6-i,1)); expected.append(p.id);
        }
        QVERIFY(repo.createPlayer("Sem partidas").selectable());
        auto retired=repo.createPlayer("Nome reutilizado"); retiredId=retired.id;
        QVERIFY(repo.saveResult(retired.identity(),60,10)); QVERIFY(repo.deletePlayer(retired.id));
        auto reused=repo.createPlayer("Nome reutilizado"); reusedId=reused.id;
        QVERIFY(reused.selectable()); QVERIFY(reused.id!=retiredId);
        QVERIFY(repo.loadPlayerResult("free",reusedId).isEmpty());
        Inspection inspection(path); before=contents(inspection.db); QVERIFY(!before.isEmpty());
        const auto top=repo.loadActiveRanking("free"); QCOMPARE(top.size(),5);
        for (int i=0;i<5;++i) {
            QCOMPARE(top[i].playerId,expected[i]); QVERIFY(!top[i].deletedProfile); QVERIFY(!top[i].legacyIdentity);
        }
        QCOMPARE(repo.loadActiveRanking("free",100).size(),6);
        QCOMPARE(repo.loadRanking("free").first().playerId,retiredId); // Existing mixed contract retained.
        const auto history=repo.loadRecordHistory("free"); QCOMPARE(history.size(),8);
        QCOMPARE(history[0].playerId,retiredId); QVERIFY(history[0].deletedProfile);
        QCOMPARE(history[1].playerId,legacyId); QVERIFY(history[1].legacyIdentity);
        QCOMPARE(history[1].snapshotName,QString("Ada")); QCOMPARE(history[1].scoreUnits,qint64(12));
        QCOMPARE(history[1].playedAt,QDateTime::fromString("2026-01-01T00:00:00Z",Qt::ISODate));
        QCOMPARE(history[1].identityOrigin,QString("legacy_unresolved")); QCOMPARE(history[1].rankingKey,QString("free"));
        QCOMPARE(repo.loadPlayerResult("free",expected[0]).first().scoreUnits,qint64(6));
        QCOMPARE(repo.playerMetrics("free",expected[0]).answered,0);
        QVERIFY(repo.loadPlayerResult("free",reusedId).isEmpty());
        QCOMPARE(repo.playerMetrics("free",reusedId).answered,0);
        QCOMPARE(repo.playerMetrics("free",legacyId).answered,1);
        QCOMPARE(contents(inspection.db),before); // Queries do not rewrite scores, provenance, games or schema.
    }
    SqliteGameHistoryRepository reopened(path);
    QCOMPARE(reopened.loadActiveRanking("free").first().playerId,expected[0]);
    QCOMPARE(reopened.loadRecordHistory("free").first().playerId,retiredId);
    QVERIFY(reopened.loadPlayerResult("free",reusedId).isEmpty());
    Inspection inspection(path); QCOMPARE(contents(inspection.db),before);
    for (int i=0;i<4;++i) QVERIFY(reopened.deletePlayer(expected[i]));
    QCOMPARE(reopened.loadActiveRanking("free").size(),2);
    QCOMPARE(reopened.loadRecordHistory("free").size(),8);
}

void PlayerIdentityTests::historicalPagesAndStoredTieBreaks()
{
    QTemporaryDir dir; const auto path=dir.filePath("history-pages.sqlite"); SqliteGameHistoryRepository repo(path);
    Inspection inspection(path); QSqlQuery q(inspection.db);
    QStringList expected;
    // Score, correct answers, oldest best date, case-insensitive name, exact name, UUID.
    const QStringList names{"Score", "Correct", "Oldest", "Ana", "Zeta"};
    for (int i=0;i<25;++i) {
        const auto name=i<5 ? names[i] : QString("Tail %1").arg(i,2,10,QChar('0'));
        const auto p=repo.createPlayer(name); QVERIFY(p.selectable()); expected.append(p.id);
        q.prepare("INSERT INTO ranking_records VALUES('free',?,?,?, ?,?,3,1,'2026-01-09T00:00:00Z')");
        q.addBindValue(p.id); q.addBindValue(name); q.addBindValue(i==0 ? 30 : i<5 ? 24 : 6);
        q.addBindValue(i==1 ? 5 : 4); q.addBindValue(i==2 ? "2026-01-01T00:00:00Z" : "2026-01-02T00:00:00Z");
        QVERIFY(q.exec());
    }
    const auto before=contents(inspection.db);
    QStringList paged;
    for (int offset=0;offset<25;offset+=7) {
        const auto page=repo.loadRecordHistory("free",7,offset); QVERIFY(page.size()<=7);
        for (const auto &entry:page) { paged.append(entry.playerId); QCOMPARE(entry.lastScoreUnits,qint64(3)); }
    }
    QCOMPARE(paged,expected);
    auto top=repo.loadActiveRanking("free"); for(int i=0;i<5;++i)QCOMPARE(top[i].playerId,expected[i]);
    QVERIFY(repo.loadRecordHistory("free",7,25).isEmpty());
    QVERIFY(repo.loadRecordHistory("free",0).isEmpty()); QVERIFY(repo.loadRecordHistory("free",-1).isEmpty());
    QVERIFY(repo.loadRecordHistory("free",5,-1).isEmpty()); QVERIFY(repo.loadActiveRanking("free",-1).isEmpty());
    QVERIFY(repo.loadRecordHistory("competitive|old-stored-configuration").isEmpty());
    QCOMPARE(contents(inspection.db),before);
    QVERIFY(repo.renamePlayer(expected[0],"Renamed"));
    QCOMPARE(repo.loadActiveRanking("free").first().playerName,QString("Renamed"));
    QCOMPARE(repo.loadRecordHistory("free").first().playerName,QString("Score"));
    // Stored competitive keys remain independently queryable; no current-rule recalculation.
    QVERIFY(q.exec("INSERT INTO ranking_records SELECT 'competitive|old-stored-configuration',player_id,player_name,best_units,best_correct,best_played_at,last_units,last_correct,last_played_at FROM ranking_records"));
    QCOMPARE(repo.loadRecordHistory("competitive|old-stored-configuration",100).size(),25);
    QCOMPARE(repo.loadRecordHistory("competitive",100).size(),25);
    QCOMPARE(repo.loadActiveRanking("competitive|old-stored-configuration").first().scoreUnits,qint64(30));
    QStringList exactTie;
    const QStringList snapshots{"ana", "Ana", "Ana"};
    for (int i=0;i<3;++i) {
        q.prepare("INSERT INTO ranking_records VALUES('competitive|ties',?,?,12,2,'2026-01-01T00:00:00Z',6,1,'2026-01-02T00:00:00Z')");
        q.addBindValue(expected[i]); q.addBindValue(snapshots[i]); QVERIFY(q.exec());
        if(i>0)exactTie.append(expected[i]);
    }
    exactTie.sort(); exactTie.append(expected[0]);
    const auto ties=repo.loadRecordHistory("competitive|ties");
    for(int i=0;i<3;++i)QCOMPARE(ties[i].playerId,exactTie[i]);
}

void PlayerIdentityTests::activeRankingWithNoEligiblePlayers()
{
    QTemporaryDir dir; const auto path=dir.filePath("no-active.sqlite"); QVERIFY(seedV3(path));
    SqliteGameHistoryRepository repo(path); QVERIFY(repo.loadActiveRanking("free").isEmpty());
    QVERIFY(repo.loadActiveRanking("competitive").isEmpty()); QCOMPARE(repo.loadRecordHistory("free").size(),1);
    auto p=repo.createPlayer("Retired"); QVERIFY(repo.saveResult(p.identity(),18,3)); QVERIFY(repo.deletePlayer(p.id));
    QVERIFY(repo.loadActiveRanking("free").isEmpty()); QCOMPARE(repo.loadRecordHistory("free").size(),2);
}

void PlayerIdentityTests::createRenameReopenAndKeepOwnership()
{
    QTemporaryDir dir; const auto path=dir.filePath("identity.sqlite"); QString id;
    {
        SqliteGameHistoryRepository repo(path); const auto ada=repo.createPlayer("Ada"); id=ada.id;
        QVERIFY(ada.selectable()); QVERIFY(!QUuid(id).isNull());
        QVERIFY(repo.saveResult(ada.identity(),12,2,{},"first"));
        QVERIFY(repo.renamePlayer(id,"Lia")); QCOMPARE(repo.player(id).displayName,QString("Lia"));
        const auto result=repo.loadPlayerResult("free",id).first();
        QCOMPARE(result.playerId,id); QCOMPARE(result.playerName,QString("Lia"));
        QCOMPARE(result.snapshotName,QString("Ada")); QCOMPARE(result.scoreUnits,qint64(12));
        QVERIFY(repo.saveResult(ada.identity(),12,2,{},"first")); // retry retains the original snapshot
        Inspection inspection(path); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("SELECT player_name,player_id FROM game_runs WHERE id='first'")); QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),QString("Ada")); QCOMPARE(q.value(1).toString(),id);
        QVERIFY(!q.exec("UPDATE player_profiles SET id='replacement'"));
    }
    SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().first().id,id);
    QCOMPARE(repo.loadPlayerResult("free",id).first().scoreUnits,qint64(12));
    QCOMPARE(repo.unresolvedLegacyPlayers(),0);
}

void PlayerIdentityTests::retireAndReuseNameWithoutInheritingResults()
{
    QTemporaryDir dir; const auto path=dir.filePath("retired.sqlite"); QString oldId,newId;
    {
        SqliteGameHistoryRepository repo(path); auto old=repo.createPlayer("Ada"); oldId=old.id;
        QVERIFY(repo.saveResult(old.identity(),12,2,{},"old")); QVERIFY(repo.deletePlayer(old.id));
        QVERIFY(repo.profiles().isEmpty()); QVERIFY(repo.player(oldId).deleted);
        auto fresh=repo.createPlayer("Ada"); newId=fresh.id; QVERIFY(fresh.selectable()); QVERIFY(newId!=oldId);
        QVERIFY(repo.loadPlayerResult("free",newId).isEmpty());
        QVERIFY(repo.saveResult(old.identity(),6,1,{},"finishing-old-session")); // A session may finish after retirement.
        QVERIFY(repo.saveResult(fresh.identity(),3,1,{},"new"));
        QCOMPARE(repo.loadRanking("free").size(),2);
        QVERIFY(repo.loadPlayerResult("free",oldId).first().deletedProfile);
        QCOMPARE(repo.loadPlayerResult("free",newId).first().scoreUnits,qint64(3));
        Inspection inspection(path); QSqlQuery q(inspection.db);
        QVERIFY(!q.exec("DELETE FROM player_profiles"));
    }
    SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().first().id,newId);
    QCOMPARE(repo.loadPlayerResult("free",oldId).first().scoreUnits,qint64(12));
}

void PlayerIdentityTests::sameGameCannotChangeOwner()
{
    QTemporaryDir dir; const auto path=dir.filePath("retry.sqlite"); SqliteGameHistoryRepository repo(path);
    const auto first=repo.createPlayer("Ada"), second=repo.createPlayer("Bia");
    QVERIFY(repo.saveResult(first.identity(),6,1,{},"known"));
    QVERIFY(!repo.saveResult(second.identity(),6,1,{},"known"));
    QVERIFY(repo.loadPlayerResult("free",second.id).isEmpty());
    Inspection inspection(path); QSqlQuery q(inspection.db); QVERIFY(q.exec("PRAGMA foreign_keys=ON"));
    q.prepare("UPDATE game_runs SET player_id=? WHERE id='known'"); q.addBindValue(second.id); QVERIFY(!q.exec());
    QVERIFY(!q.exec("UPDATE game_runs SET player_name='Another name' WHERE id='known'"));
    QCOMPARE(repo.loadRanking("free").size(),1);
}

void PlayerIdentityTests::renameAndReusePreviousNameKeepDistinctRankings()
{
    QTemporaryDir dir; SqliteGameHistoryRepository repo(dir.filePath("names.sqlite"));
    auto ada=repo.createPlayer("Ada"); QVERIFY(repo.saveResult(ada.identity(),6,1));
    QVERIFY(repo.renamePlayer(ada.id,"Lia")); auto newAda=repo.createPlayer("Ada"); QVERIFY(newAda.selectable());
    QVERIFY(newAda.id!=ada.id); QVERIFY(repo.loadPlayerResult("free",newAda.id).isEmpty());
    QVERIFY(repo.saveResult(newAda.identity(),3,1));
    QCOMPARE(repo.loadRanking("free").size(),2);
    QCOMPARE(repo.loadPlayerResult("free",ada.id).first().playerName,QString("Lia"));
    QVERIFY(!repo.renamePlayer(newAda.id,"Lia"));
}

void PlayerIdentityTests::invalidIdentityAndNamesAreRefused()
{
    QTemporaryDir dir; SqliteGameHistoryRepository repo(dir.filePath("validation.sqlite"));
    QVERIFY(repo.createPlayer("João").selectable()); QVERIFY(!repo.createPlayer("JOÃO").selectable());
    QVERIFY(!repo.createPlayer(QString::fromUtf8("Joa\xCC\x83o")).selectable());
    QVERIFY(!repo.createPlayer("   ").selectable()); QVERIFY(!repo.createPlayer(QString(33,'x')).selectable());
    QVERIFY(!repo.saveResult(PlayerIdentity{"unknown","João"},6,1));
    QVERIFY(!repo.saveResult(PlayerIdentity{},6,1)); QVERIFY(repo.loadRanking("free").isEmpty());
}

void PlayerIdentityTests::migrateV3PreservesUnresolvedRecordsAndSnapshots()
{
    QTemporaryDir dir; const auto path=dir.filePath("v3.sqlite"); QVERIFY(seedV3(path));
    SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().size(),1);
    QCOMPARE(repo.unresolvedLegacyPlayers(),1); const auto active=repo.profiles().first();
    auto result=repo.loadRanking("free").first(); QVERIFY(result.legacyIdentity); QVERIFY(result.playerId!=active.id);
    QCOMPARE(result.scoreUnits,qint64(12)); QCOMPARE(result.lastScoreUnits,qint64(6));
    QVERIFY(repo.loadPlayerResult("free",active.id).isEmpty()); QCOMPARE(repo.playerMetrics("free",active.id).answered,0);
    QCOMPARE(repo.playerMetrics("free",result.playerId).answered,1);
    QCOMPARE(repo.loadAnswers("old-run").first().photoId,QString("legacy-photo"));
    QCOMPARE(repo.loadAnswers("old-run").first().awardedUnits,6);
    QVERIFY(!repo.saveResult(repo.player(result.playerId).identity(),6,1));
    Inspection inspection(path); QSqlQuery q(inspection.db);
    QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),6);
    QVERIFY(q.exec("SELECT player_name,score_units,correct_answers,played_at FROM legacy_history_v3")); QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(),QString("Ada")); QCOMPARE(q.value(1).toInt(),6);
    QVERIFY(q.exec("SELECT best_units,last_units FROM legacy_ranking_records_v3")); QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(),12); QCOMPARE(q.value(1).toInt(),6);
    QVERIFY(q.exec("PRAGMA foreign_key_check")); QVERIFY(!q.next());
}

void PlayerIdentityTests::migrationIsIdempotentWithoutNewIdsOrNotes()
{
    QTemporaryDir dir; const auto path=dir.filePath("repeated.sqlite"); QVERIFY(seedV3(path)); QString activeId,legacyId; QStringList before;
    {
        SqliteGameHistoryRepository repo(path); activeId=repo.profiles().first().id; legacyId=repo.loadRanking("free").first().playerId;
        Inspection inspection(path); before=schema(inspection.db);
    }
    for(int i=0;i<3;++i) {
        SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().first().id,activeId);
        QCOMPARE(repo.loadRanking("free").first().playerId,legacyId);
        Inspection inspection(path); QCOMPARE(schema(inspection.db),before); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("SELECT COUNT(*) FROM identity_migration_notes")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),2);
        QVERIFY(q.exec("SELECT COUNT(*) FROM game_runs")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),1);
    }
}

void PlayerIdentityTests::migrationRollbackPreservesOldSchemaAndRecords()
{
    QTemporaryDir dir; const auto path=dir.filePath("rollback.sqlite"); QVERIFY(seedV3(path)); QStringList before;
    {
        Inspection inspection(path); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("CREATE TRIGGER fail_identity BEFORE UPDATE ON game_runs BEGIN SELECT RAISE(ABORT,'synthetic migration interruption'); END"));
        before=schema(inspection.db);
    }
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.profiles().isEmpty()); QVERIFY(failed.loadRanking("free").isEmpty()); }
    {
        Inspection inspection(path); QCOMPARE(schema(inspection.db),before); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),3);
        QVERIFY(q.exec("SELECT best_units,last_units FROM ranking_records")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),12); QCOMPARE(q.value(1).toInt(),6);
        QVERIFY(q.exec("SELECT player_name,score_units FROM game_runs")); QVERIFY(q.next()); QCOMPARE(q.value(0).toString(),QString("Ada")); QCOMPARE(q.value(1).toInt(),6);
        QVERIFY(q.exec("DROP TRIGGER fail_identity"));
    }
    SqliteGameHistoryRepository recovered(path); QCOMPARE(recovered.profiles().size(),1); QCOMPARE(recovered.unresolvedLegacyPlayers(),1);
}

void PlayerIdentityTests::legacyUnicodeProfilesStaySeparate()
{
    QTemporaryDir dir; const auto path=dir.filePath("unicode.sqlite"); QVERIFY(seedV3(path));
    {
        Inspection inspection(path); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("INSERT INTO player_profiles VALUES('João'),('JOÃO')"));
    }
    SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().size(),3);
    auto one=repo.profileNamed("João"), two=repo.profileNamed("JOÃO"); QVERIFY(one.id!=two.id);
    QVERIFY(one.selectable()); QVERIFY(two.selectable()); QVERIFY(!repo.createPlayer("joão").selectable());
    QVERIFY(repo.renamePlayer(one.id,"João Silva")); QVERIFY(repo.renamePlayer(two.id,"João Souza"));
    QCOMPARE(repo.player(one.id).displayName,QString("João Silva"));
}

void PlayerIdentityTests::migrationDoesNotClaimRenamedDeletedOrReusedNames()
{
    QTemporaryDir dir; const auto path=dir.filePath("ambiguous.sqlite"); QVERIFY(seedV3(path));
    {
        Inspection inspection(path); QSqlQuery q(inspection.db);
        QVERIFY(q.exec("UPDATE player_profiles SET name='Lia'"));
        QVERIFY(q.exec("INSERT INTO player_profiles VALUES('Ada')")); // Could be another person. No evidence of ownership.
    }
    SqliteGameHistoryRepository repo(path); const auto old=repo.loadRanking("free").first(); QVERIFY(old.legacyIdentity);
    for(const auto &p:repo.profiles()) {
        QVERIFY(p.id!=old.playerId); QVERIFY(repo.loadPlayerResult("free",p.id).isEmpty());
        QCOMPARE(repo.playerMetrics("free",p.id).answered,0);
    }
    QVERIFY(repo.saveResult(repo.profileNamed("Ada").identity(),3,1));
    QCOMPARE(repo.loadRanking("free").size(),2); QCOMPARE(repo.loadRanking("free").first().scoreUnits,qint64(12));
}

void PlayerIdentityTests::unicodeEquivalentNames_data()
{
    QTest::addColumn<QString>("first"); QTest::addColumn<QString>("equivalent");
    QTest::newRow("portuguese") << QString("João") << QString("JOÃO");
    QTest::newRow("ascii") << QString("Maria") << QString("MARIA");
    QTest::newRow("canonical-decomposition") << QString("João") << QString::fromUtf8("Joa\xCC\x83o");
    QTest::newRow("greek") << QString("Σίσυφος") << QString("ΣΊΣΥΦΟΣ");
    QTest::newRow("cyrillic") << QString("Иван") << QString("ИВАН");
    QTest::newRow("supplementary-deseret") << QString::fromUcs4(U"\U00010400") << QString::fromUcs4(U"\U00010428");
    QTest::newRow("external-whitespace") << QString("Maria") << QString(" \tMARIA\n");
}

void PlayerIdentityTests::unicodeEquivalentNames()
{
    QFETCH(QString,first); QFETCH(QString,equivalent);
    QTemporaryDir dir; const auto path=dir.filePath("equivalence.sqlite"); QString owner,otherId;
    {
        SqliteGameHistoryRepository repo(path); const auto p=repo.createPlayer(first); owner=p.id;
        QVERIFY(p.selectable()); QCOMPARE(PlayerNamePolicy::key(first),PlayerNamePolicy::key(equivalent));
        QCOMPARE(PlayerNamePolicy::key(PlayerNamePolicy::key(first)),PlayerNamePolicy::key(first));
        QVERIFY(!repo.createPlayer(equivalent).selectable()); QCOMPARE(repo.profileNamed(equivalent).id,owner);
        const auto other=repo.createPlayer("Other"); otherId=other.id;
        QVERIFY(!repo.renamePlayer(other.id,equivalent)); QCOMPARE(repo.player(other.id).displayName,QString("Other"));
        QVERIFY(repo.saveResult(p.identity(),6,1,{},"unicode-game"));
    }
    SqliteGameHistoryRepository reopened(path);
    QVERIFY(!reopened.createPlayer(equivalent).selectable());
    QCOMPARE(reopened.loadPlayerResult("free",owner).first().scoreUnits,qint64(6));
    QVERIFY(reopened.loadPlayerResult("free",otherId).isEmpty());
}

void PlayerIdentityTests::unicodeDistinctionsAndDisplaySpelling()
{
    QTemporaryDir dir; const auto path=dir.filePath("spelling.sqlite"); QString owner;
    const auto decomposed=QString::fromUtf8("Joa\xCC\x83o");
    {
        SqliteGameHistoryRepository repo(path); auto p=repo.createPlayer("  "+decomposed+"  "); owner=p.id;
        QVERIFY(p.selectable()); QCOMPARE(p.displayName,decomposed); // Only external spaces change.
        QVERIFY(repo.createPlayer("Joao").selectable()); // Accents remain significant.
        QVERIFY(repo.createPlayer("A").selectable()); QVERIFY(repo.createPlayer("А").selectable()); // Latin/Cyrillic.
        QVERIFY(repo.createPlayer("李明").selectable()); QVERIFY(repo.createPlayer("李明明").selectable());
        QVERIFY(repo.createPlayer("Ana Maria").selectable()); QVERIFY(repo.createPlayer("Ana  Maria").selectable());
        QVERIFY(repo.createPlayer("Straße").selectable()); QVERIFY(repo.createPlayer("STRASSE").selectable()); // Simple, not full folding.
        QVERIFY(repo.createPlayer("İ").selectable()); QVERIFY(repo.createPlayer("i").selectable()); // No locale-specific mapping.
        QVERIFY(repo.saveResult(p.identity(),6,1,{},"spelling"));
    }
    SqliteGameHistoryRepository reopened(path); QCOMPARE(reopened.player(owner).displayName,decomposed);
    QCOMPARE(reopened.loadPlayerResult("free",owner).first().snapshotName,decomposed);
    Inspection inspection(path); QSqlQuery q(inspection.db);
    q.prepare("SELECT name,name_key FROM player_profiles WHERE id=?"); q.addBindValue(owner);
    QVERIFY(q.exec()); QVERIFY(q.next()); QCOMPARE(q.value(0).toString(),decomposed); QCOMPARE(q.value(1).toString(),QString("joão"));
}

void PlayerIdentityTests::unicodeRenameConflictAndOwnCaseChange()
{
    QTemporaryDir dir; SqliteGameHistoryRepository repo(dir.filePath("rename.sqlite"));
    auto p=repo.createPlayer("João"), other=repo.createPlayer("Maria");
    QVERIFY(repo.saveResult(p.identity(),12,2)); QVERIFY(repo.renamePlayer(p.id,"JOÃO"));
    QCOMPARE(repo.player(p.id).displayName,QString("JOÃO"));
    QVERIFY(!repo.renamePlayer(other.id,"joão")); QVERIFY(!repo.renamePlayer(p.id,"MARIA"));
    QCOMPARE(repo.loadPlayerResult("free",p.id).first().snapshotName,QString("João"));
    QVERIFY(repo.deletePlayer(p.id)); auto fresh=repo.createPlayer("João");
    QVERIFY(fresh.selectable()); QVERIFY(fresh.id!=p.id); QVERIFY(repo.loadPlayerResult("free",fresh.id).isEmpty());
}

void PlayerIdentityTests::migrateV4AuditsConflictsWithoutChangingOwners()
{
    QTemporaryDir dir; const auto path=dir.filePath("v4.sqlite"); QVERIFY(seedV3(path)); QString firstId,secondId;
    {
        Inspection i(path); QSqlQuery q(i.db);
        QVERIFY(q.exec("INSERT INTO player_profiles VALUES('João'),('JOÃO')"));
        QVERIFY(i.db.transaction()); QVERIFY(migratePlayerIdentity(i.db)); QVERIFY(q.exec("PRAGMA user_version=4")); QVERIFY(i.db.commit());
        QVERIFY(q.exec("SELECT id FROM player_profiles WHERE name='João'")); QVERIFY(q.next()); firstId=q.value(0).toString(); q.finish();
        QVERIFY(q.exec("SELECT id FROM player_profiles WHERE name='JOÃO'")); QVERIFY(q.next()); secondId=q.value(0).toString(); q.finish();
        // Simulate a key produced before the unified policy. Keep the actual ID.
        q.prepare("UPDATE player_profiles SET name_key='old-key' WHERE id=?"); q.addBindValue(firstId); QVERIFY(q.exec());
        q.prepare("INSERT INTO ranking_records VALUES('free',?,'João',18,3,'2026-01-03',6,1,'2026-01-04')"); q.addBindValue(firstId); QVERIFY(q.exec());
    }
    QStringList migratedSchema; QString timestamp;
    {
        SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().size(),3);
        QCOMPARE(repo.player(firstId).displayName,QString("João")); QCOMPARE(repo.player(secondId).displayName,QString("JOÃO"));
        QVERIFY(!repo.createPlayer("joão").selectable()); QVERIFY(repo.profileNamed("joão").id.isEmpty());
        QCOMPARE(repo.profileNamed("João").id,firstId); QCOMPARE(repo.profileNamed("JOÃO").id,secondId);
        QCOMPARE(repo.loadPlayerResult("free",firstId).first().scoreUnits,qint64(18)); QVERIFY(repo.loadPlayerResult("free",secondId).isEmpty());
        QCOMPARE(repo.loadAnswers("old-run").first().photoId,QString("legacy-photo"));
        QCOMPARE(repo.loadAnswers("old-run").first().awardedUnits,6);
        Inspection i(path); QSqlQuery q(i.db); migratedSchema=schema(i.db);
        QVERIFY(q.exec("SELECT COUNT(*) FROM player_name_conflicts")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),2);
        q.prepare("SELECT old_key,new_key,active_collision,migrated_at FROM player_name_migration WHERE player_id=?"); q.addBindValue(firstId);
        QVERIFY(q.exec()); QVERIFY(q.next()); QCOMPARE(q.value(0).toString(),QString("old-key")); QCOMPARE(q.value(1).toString(),QString("joão"));
        QCOMPARE(q.value(2).toInt(),1); timestamp=q.value(3).toString(); QVERIFY(!timestamp.isEmpty());
        QVERIFY(q.exec("SELECT COUNT(*) FROM player_name_migration")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),3);
        QVERIFY(q.exec("SELECT policy_version,unicode_version FROM player_name_policy")); QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),PlayerNamePolicy::version()); QCOMPARE(q.value(1).toInt(),PlayerNamePolicy::unicodeVersion());
    }
    for(int n=0;n<2;++n) {
        SqliteGameHistoryRepository repo(path); QCOMPARE(repo.profiles().size(),3);
        Inspection i(path); QCOMPARE(schema(i.db),migratedSchema); QSqlQuery q(i.db);
        q.prepare("SELECT migrated_at FROM player_name_migration WHERE player_id=?"); q.addBindValue(firstId);
        QVERIFY(q.exec()); QVERIFY(q.next()); QCOMPARE(q.value(0).toString(),timestamp);
        QCOMPARE(repo.loadPlayerResult("free",firstId).first().scoreUnits,qint64(18));
    }
    SqliteGameHistoryRepository repo(path); QVERIFY(repo.renamePlayer(firstId,"João Silva"));
    Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec("SELECT COUNT(*) FROM player_name_conflicts")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
    QVERIFY(q.exec("SELECT COUNT(*) FROM player_name_migration WHERE active_collision=1")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),2); // Audit is retained.
}

void PlayerIdentityTests::migrateV4FailureRollsBackKeysAndGuards()
{
    QTemporaryDir dir; const auto path=dir.filePath("v4-rollback.sqlite"); QVERIFY(seedV3(path)); QStringList before;
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(i.db.transaction()); QVERIFY(migratePlayerIdentity(i.db));
        QVERIFY(q.exec("PRAGMA user_version=4")); QVERIFY(i.db.commit());
        QVERIFY(q.exec("CREATE TRIGGER fail_name_migration AFTER UPDATE OF name_key ON player_profiles BEGIN SELECT RAISE(ABORT,'synthetic interruption after key update'); END"));
        before=schema(i.db);
    }
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.profiles().isEmpty()); }
    {
        Inspection i(path); QCOMPARE(schema(i.db),before); QSqlQuery q(i.db);
        QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),4);
        QVERIFY(q.exec("SELECT best_units FROM ranking_records")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),12);
        QVERIFY(q.exec("DROP TRIGGER fail_name_migration"));
    }
    SqliteGameHistoryRepository recovered(path); QCOMPARE(recovered.profiles().size(),1);
    QCOMPARE(recovered.loadRanking("free").first().scoreUnits,qint64(12));
}

void PlayerIdentityTests::migrateV4RetiredProfilesKeepNamesAndResults()
{
    QTemporaryDir dir; const auto path=dir.filePath("retired-v4.sqlite"); QVERIFY(seedV3(path));
    const auto name=QString::fromUtf8("Joa\xCC\x83o");
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(i.db.transaction()); QVERIFY(migratePlayerIdentity(i.db));
        QVERIFY(q.exec("PRAGMA user_version=4")); QVERIFY(i.db.commit());
        q.prepare("INSERT INTO player_profiles(id,name,name_key,origin,deleted_at) VALUES('retired',?,'old-deleted-key','profile','2026-01-01')");
        q.addBindValue(name); QVERIFY(q.exec());
        QVERIFY(q.exec("INSERT INTO ranking_records VALUES('free','retired','Original snapshot',24,4,'2026-01-01',6,1,'2026-01-02')"));
    }
    SqliteGameHistoryRepository repo(path);
    QCOMPARE(repo.player("retired").displayName,name); QVERIFY(repo.player("retired").deleted);
    auto fresh=repo.createPlayer("JOÃO"); QVERIFY(fresh.selectable()); QVERIFY(fresh.id!="retired");
    QCOMPARE(repo.loadPlayerResult("free","retired").first().scoreUnits,qint64(24));
    QVERIFY(repo.loadPlayerResult("free",fresh.id).isEmpty());
    Inspection i(path); QSqlQuery q(i.db);
    QVERIFY(q.exec("SELECT name_key,deleted_at FROM player_profiles WHERE id='retired'")); QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(),QString("joão")); QCOMPARE(q.value(1).toString(),QString("2026-01-01"));
    QVERIFY(q.exec("SELECT active_collision FROM player_name_migration WHERE player_id='retired'")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
    QVERIFY(q.exec("SELECT COUNT(*) FROM player_name_conflicts")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
    // The DB guard must reject a fresh active conflict even outside repository APIs.
    QVERIFY(!q.exec("INSERT INTO player_profiles(id,name,name_key,origin) VALUES('bad','joão','joão','profile')"));
    QVERIFY(!q.exec("UPDATE player_profiles SET deleted_at=NULL WHERE id='retired'"));
    QCOMPARE(repo.player("retired").deleted,true);
}

void PlayerIdentityTests::unsupportedPolicyAndInconsistentKeysFailClosed()
{
    QTemporaryDir dir; const auto path=dir.filePath("policy.sqlite"); QString id;
    { SqliteGameHistoryRepository repo(path); auto p=repo.createPlayer("João"); id=p.id; QVERIFY(repo.saveResult(p.identity(),6,1)); }
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec("UPDATE player_name_policy SET unicode_version=-1"));
    }
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.profiles().isEmpty()); QVERIFY(!failed.createPlayer("Other").selectable()); }
    {
        Inspection i(path); QSqlQuery q(i.db);
        QVERIFY(q.exec(QString("UPDATE player_name_policy SET unicode_version=%1").arg(PlayerNamePolicy::unicodeVersion())));
        QVERIFY(q.exec("UPDATE player_profiles SET name_key='inconsistent' WHERE origin='profile'"));
    }
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.profiles().isEmpty()); }
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec("SELECT player_id,best_units FROM ranking_records")); QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),id); QCOMPARE(q.value(1).toInt(),6);
    }
}

void PlayerIdentityTests::incompleteCurrentSchemaFailsClosed_data()
{
    QTest::addColumn<QString>("damage");
    QTest::newRow("missing-name-guard") << QString("DROP TRIGGER player_name_insert");
    QTest::newRow("missing-checkpoint-guard") << QString("DROP TRIGGER checkpoint_identity_immutable");
    QTest::newRow("missing-checkpoint-table") << QString("DROP TABLE session_checkpoints");
    QTest::newRow("missing-answer-column") << QString("ALTER TABLE answer_records RENAME COLUMN hint_snapshot TO unsupported_snapshot");
    QTest::newRow("foreign-key-violation") << QString("UPDATE ranking_records SET player_id='unknown-fixture-id'");
    QTest::newRow("future-schema") << QString("PRAGMA user_version=99");
}

void PlayerIdentityTests::incompleteCurrentSchemaFailsClosed()
{
    QFETCH(QString, damage);
    QTemporaryDir dir; const auto path=dir.filePath("incompatible.sqlite"); QString owner;
    {
        SqliteGameHistoryRepository repo(path); const auto p=repo.createPlayer("João"); owner=p.id;
        QVERIFY(p.selectable()); QVERIFY(repo.saveResult(p.identity(),12,2,{},"retained-game"));
    }
    QStringList before;
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec(damage)); before=schema(i.db);
    }
    for (int attempt=0; attempt<2; ++attempt) {
        SqliteGameHistoryRepository refused(path);
        QVERIFY(refused.profiles().isEmpty());
        QVERIFY(!refused.createPlayer("JOÃO").selectable());
        QVERIFY(!refused.saveResult({owner,"João"},6,1,{},"must-not-be-recorded"));
    }
    Inspection i(path); QCOMPARE(schema(i.db),before); QSqlQuery q(i.db);
    QVERIFY(q.exec("SELECT player_id,score_units FROM game_runs")); QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(),owner); QCOMPARE(q.value(1).toInt(),12); QVERIFY(!q.next());
    QVERIFY(q.exec("SELECT COUNT(*) FROM player_profiles")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),1);
}

void PlayerIdentityTests::incompleteV5MigrationRollsBackAndKeepsResults()
{
    QTemporaryDir dir; const auto path=dir.filePath("incomplete-v5.sqlite"); QString owner; QStringList before;
    {
        SqliteGameHistoryRepository repo(path); auto p=repo.createPlayer("João"); owner=p.id;
        QVERIFY(repo.saveResult(p.identity(),18,3,{},"retained-v5"));
    }
    {
        Inspection i(path); QSqlQuery q(i.db);
        QVERIFY(q.exec("DROP TABLE session_checkpoints"));
        QVERIFY(q.exec("ALTER TABLE answer_records RENAME COLUMN hint_snapshot TO unsupported_snapshot"));
        QVERIFY(q.exec("PRAGMA user_version=5")); before=schema(i.db);
    }
    for (int attempt=0; attempt<2; ++attempt) {
        SqliteGameHistoryRepository refused(path); QVERIFY(refused.profiles().isEmpty());
        Inspection i(path); QCOMPARE(schema(i.db),before); QSqlQuery q(i.db);
        QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),5); q.finish();
        QVERIFY(q.exec("SELECT player_id,score_units FROM game_runs")); QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),owner); QCOMPARE(q.value(1).toInt(),18);
    }
    // Repair only the synthetic fixture; production never guesses a schema repair.
    { Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec("ALTER TABLE answer_records RENAME COLUMN unsupported_snapshot TO hint_snapshot")); }
    SqliteGameHistoryRepository recovered(path); QCOMPARE(recovered.profiles().first().id,owner);
    QCOMPARE(recovered.loadPlayerResult("free",owner).first().scoreUnits,qint64(18));
}

QTEST_GUILESS_MAIN(PlayerIdentityTests)
#include "PlayerIdentityTests.moc"
