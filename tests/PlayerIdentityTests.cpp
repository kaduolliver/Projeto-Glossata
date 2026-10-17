#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include "infrastructure/persistence/PlayerIdentityMigration.h"
#include "domain/PlayerNamePolicy.h"
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
    void unicodeEquivalentNames_data();
    void unicodeEquivalentNames();
    void unicodeDistinctionsAndDisplaySpelling();
    void unicodeRenameConflictAndOwnCaseChange();
    void migrateV4AuditsConflictsWithoutChangingOwners();
    void migrateV4FailureRollsBackKeysAndGuards();
    void migrateV4RetiredProfilesKeepNamesAndResults();
    void unsupportedPolicyAndInconsistentKeysFailClosed();
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

QTEST_GUILESS_MAIN(PlayerIdentityTests)
#include "PlayerIdentityTests.moc"
