#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QUuid>

namespace {
struct Inspection {
    QSqlDatabase db;
    Inspection(const QString &path) {
        db=QSqlDatabase::addDatabase("QSQLITE",QUuid::createUuid().toString()); db.setDatabaseName(path); db.open();
    }
    ~Inspection() { const auto name=db.connectionName(); db.close(); db={}; QSqlDatabase::removeDatabase(name); }
};
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
};

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
    QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),4);
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

QTEST_GUILESS_MAIN(PlayerIdentityTests)
#include "PlayerIdentityTests.moc"
