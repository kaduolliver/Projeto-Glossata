#include "application/GameService.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include "infrastructure/persistence/SqliteGameHistoryRepository.h"
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QProcess>
#include <QUuid>
#include <cstdlib>

namespace {
struct Inspection {
    QSqlDatabase db;
    explicit Inspection(const QString &path) {
        db=QSqlDatabase::addDatabase("QSQLITE",QUuid::createUuid().toString()); db.setDatabaseName(path); db.open();
    }
    ~Inspection() { const auto name=db.connectionName(); db.close(); db={}; QSqlDatabase::removeDatabase(name); }
    bool exec(const QString &sql) { QSqlQuery q(db); return q.exec(sql); }
    int count(const QString &table) { QSqlQuery q(db); return q.exec("SELECT COUNT(*) FROM "+table) && q.next() ? q.value(0).toInt() : -1; }
};
GameConfiguration shortCompetitive() { auto c=GameService::competitiveConfiguration(); c.questionLimit=3; return c; }
int availableSlot(const GameService &game) {
    const auto hints=game.currentQuestion().hints;
    for (int i=2;i>=0;--i) if (hints[i].available()) return i;
    return -1;
}
void start(GameService &game,SqliteGameHistoryRepository &repo) {
    game.setCheckpointRepository(&repo);
    auto player=repo.profileNamed("Synthetic player"); if (!player.selectable()) player=repo.createPlayer("Synthetic player");
    game.startConfiguredGame(shortCompetitive(),player.identity());
}
int worker(const QString &path,const QString &mode) {
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); start(game,repo);
    if (game.isFinished() || repo.recoverableSessions().isEmpty()) return 2;
    if (mode=="hint") { const int slot=availableSlot(game); if (slot<0 || !game.revealHintSlot(slot)) return 3; }
    else {
        const int count=mode=="final" ? 3 : 1;
        for (int i=0;i<count;++i) if (!game.submitAnswer(game.currentQuestion().correctAnswer)) return 4;
    }
    if (mode=="uncommitted") {
        Inspection inspection(path);
        if (!inspection.exec("BEGIN IMMEDIATE") || !inspection.exec("UPDATE session_checkpoints SET revision=revision+999")) return 5;
        std::_Exit(0); // Deliberately skip SQLite/Qt destructors while a transaction is open.
    }
    std::_Exit(0); // The committed checkpoint must survive without normal shutdown.
}
}

class SessionRecoveryTests : public QObject {
    Q_OBJECT
private slots:
    void recoverHintsAnswersAndExactQuestionOrder();
    void recoveredCompetitiveCompletionIsAtomicAndIdempotent();
    void failedCheckpointDoesNotConfirmAnAnswerOrRevealAHint();
    void failedFinalizationLeavesRecoverableFinishedSession();
    void abandonmentAndStaleWritersCannotChangeRanking();
    void invalidSnapshotsAndChangedContentAreRetainedAndRefused();
    void transitionsCannotEraseAnswersOrHints();
    void v5MigrationPreservesIdsResultsAndCanRepeat();
    void abruptProcess_data();
    void abruptProcess();
    void consistentBackupRestoresAValidSession();
    void failedInitialWriteAndMigrationPreserveExistingResults();
    void retiredOwnerAndReusedNameRemainSeparateAfterRecovery();
};

void SessionRecoveryTests::recoverHintsAnswersAndExactQuestionOrder()
{
    QTemporaryDir dir; const auto path=dir.filePath("recovery.sqlite"); QString id,owner; int mask,ceiling; QJsonObject first,next;
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); start(game,repo);
        QVERIFY(!game.isFinished()); QVERIFY(game.checkpointError().isEmpty()); id=game.sessionId(); owner=game.sessionPlayer().id;
        first=SessionSnapshot::question(game.currentQuestion()); const int slot=availableSlot(game); QVERIFY(slot>=0);
        QVERIFY(game.revealHintSlot(slot)); mask=game.revealedHints(); ceiling=game.possibleScoreUnits();
        QVERIFY(repo.renamePlayer(owner,"Renamed player"));
        QCOMPARE(repo.checkpoint(id).payload["revealed"].toInt(),mask); QVERIFY(repo.loadRanking(game.configuration().rankingKey()).isEmpty());
    }
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); game.setCheckpointRepository(&repo);
        QVERIFY(game.recoverSession(id)); QCOMPARE(game.sessionPlayer().id,owner); QCOMPARE(game.sessionPlayer().displayName,QString("Synthetic player"));
        QCOMPARE(SessionSnapshot::question(game.currentQuestion()),first); QCOMPARE(game.revealedHints(),mask); QCOMPARE(game.possibleScoreUnits(),ceiling);
        QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer)); QCOMPARE(game.answeredCount(),1); QCOMPARE(game.scoreUnits(),qint64(ceiling));
        next=SessionSnapshot::question(game.currentQuestion()); QCOMPARE(game.answers().first().revealedHints,mask);
        QCOMPARE(game.answers().first().maximumUnits,ceiling); QCOMPARE(game.revealedHints(),0);
    }
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService recovered(&catalog,&repo); recovered.setCheckpointRepository(&repo);
    QVERIFY(recovered.recoverSession(id)); QCOMPARE(recovered.answeredCount(),1); QCOMPARE(recovered.round(),2);
    QCOMPARE(SessionSnapshot::question(recovered.currentQuestion()),next); QCOMPARE(recovered.scoreUnits(),qint64(ceiling));
    QCOMPARE(recovered.performance().groups.size(),1);
    // Generator state and already selected cycle order survive, not just the visible options.
    GameService twin(&catalog,&repo); twin.setCheckpointRepository(&repo); QVERIFY(twin.recoverSession(id));
    twin.setCheckpointRepository(nullptr); QVERIFY(twin.submitAnswer(twin.currentQuestion().correctAnswer));
    QVERIFY(recovered.submitAnswer(recovered.currentQuestion().correctAnswer));
    QCOMPARE(SessionSnapshot::question(recovered.currentQuestion()),SessionSnapshot::question(twin.currentQuestion()));
}

void SessionRecoveryTests::recoveredCompetitiveCompletionIsAtomicAndIdempotent()
{
    QTemporaryDir dir; const auto path=dir.filePath("final.sqlite"); QString id;
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); game.setCheckpointRepository(&repo);
        game.startConfiguredGame(GameService::competitiveConfiguration(),repo.createPlayer("Ada").identity()); id=game.sessionId();
        for (int i=0;i<19;++i) QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
        QCOMPARE(game.answeredCount(),19); QVERIFY(!game.isFinished()); QVERIFY(repo.loadRanking(game.configuration().rankingKey()).isEmpty());
    }
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); game.setCheckpointRepository(&repo);
    QVERIFY(game.recoverSession(id)); QCOMPARE(game.answeredCount(),19);
    QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer)); QVERIFY(game.isFinished());
    QCOMPARE(repo.checkpoint(id).state,QString("finished")); QVERIFY(repo.loadRanking(game.configuration().rankingKey()).isEmpty());
    QVERIFY(!game.submitAnswer("retry")); QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
    QCOMPARE(repo.checkpoint(id).state,QString("finalized")); QVERIFY(repo.recoverableSessions().isEmpty());
    QCOMPARE(game.scoreUnits(),qint64(120)); QCOMPARE(repo.loadRanking(game.configuration().rankingKey()).size(),1);
    QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::NotNeeded);
    QVERIFY(repo.saveConfiguredResult(game.sessionPlayer(),game.scoreUnits(),game.correctAnswers(),game.answers(),id,game.configuration()));
    Inspection inspection(path); QCOMPARE(inspection.count("game_runs"),1); QCOMPARE(inspection.count("answer_records"),20); QCOMPARE(inspection.count("ranking_records"),1);
    GameService duplicate(&catalog,&repo); duplicate.setCheckpointRepository(&repo); QVERIFY(!duplicate.recoverSession(id));
}

void SessionRecoveryTests::failedCheckpointDoesNotConfirmAnAnswerOrRevealAHint()
{
    QTemporaryDir dir; const auto path=dir.filePath("failure.sqlite"); SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog;
    GameService game(&catalog,&repo); start(game,repo); const auto initial=game.sessionSnapshot(); const auto id=game.sessionId();
    Inspection inspection(path);
    QVERIFY(inspection.exec("CREATE TRIGGER fail_checkpoint AFTER UPDATE ON session_checkpoints BEGIN SELECT RAISE(ABORT,'synthetic checkpoint write failure'); END"));
    const int slot=availableSlot(game); QVERIFY(slot>=0); QVERIFY(!game.revealHintSlot(slot)); QCOMPARE(game.revealedHints(),0);
    QCOMPARE(game.sessionSnapshot(),initial); QCOMPARE(repo.checkpoint(id).payload,initial);
    QVERIFY(!game.submitAnswer(game.currentQuestion().correctAnswer)); QCOMPARE(game.answeredCount(),0); QCOMPARE(game.scoreUnits(),qint64(0));
    QCOMPARE(game.sessionSnapshot(),initial); QVERIFY(!game.checkpointError().isEmpty());
    QVERIFY(inspection.exec("DROP TRIGGER fail_checkpoint")); QVERIFY(game.revealHintSlot(slot));
    const auto hinted=game.sessionSnapshot();
    QVERIFY(inspection.exec("CREATE TRIGGER fail_checkpoint BEFORE UPDATE ON session_checkpoints BEGIN SELECT RAISE(ABORT,'synthetic checkpoint write failure'); END"));
    QVERIFY(!game.submitAnswer(game.currentQuestion().correctAnswer)); QCOMPARE(game.sessionSnapshot(),hinted); QCOMPARE(repo.checkpoint(id).payload,hinted);
    QVERIFY(inspection.exec("DROP TRIGGER fail_checkpoint"));
    GameService reopened(&catalog,&repo); reopened.setCheckpointRepository(&repo); QVERIFY(reopened.recoverSession(id));
    QCOMPARE(reopened.revealedHints(),1<<slot); QVERIFY(reopened.submitAnswer(reopened.currentQuestion().correctAnswer));
    QCOMPARE(reopened.scoreUnits(),qint64(reopened.answers().first().maximumUnits));
}

void SessionRecoveryTests::failedFinalizationLeavesRecoverableFinishedSession()
{
    QTemporaryDir dir; const auto path=dir.filePath("final-failure.sqlite"); QString id;
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); start(game,repo); id=game.sessionId();
        for (int i=0;i<3;++i) QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
        Inspection inspection(path);
        QVERIFY(inspection.exec("CREATE TRIGGER fail_final AFTER UPDATE ON session_checkpoints WHEN NEW.state='finalized' BEGIN SELECT RAISE(ABORT,'synthetic finalization failure'); END"));
        QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Failed);
        QCOMPARE(inspection.count("game_runs"),0); QCOMPARE(inspection.count("answer_records"),0); QCOMPARE(inspection.count("ranking_records"),0);
        QCOMPARE(repo.checkpoint(id).state,QString("finished"));
        QVERIFY(inspection.exec("DROP TRIGGER fail_final"));
    }
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); game.setCheckpointRepository(&repo);
    QVERIFY(game.recoverSession(id)); QVERIFY(game.isFinished()); QCOMPARE(game.scoreUnits(),qint64(18));
    QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved); QCOMPARE(repo.loadRanking(game.configuration().rankingKey()).size(),1);
}

void SessionRecoveryTests::abandonmentAndStaleWritersCannotChangeRanking()
{
    QTemporaryDir dir; const auto path=dir.filePath("stale.sqlite"); SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog;
    GameService first(&catalog,&repo); start(first,repo); const auto id=first.sessionId();
    SqliteGameHistoryRepository otherRepo(path); GameService stale(&catalog,&otherRepo); stale.setCheckpointRepository(&otherRepo); QVERIFY(stale.recoverSession(id));
    const int slot=availableSlot(first); QVERIFY(slot>=0); QVERIFY(first.revealHintSlot(slot));
    QVERIFY(!stale.submitAnswer(stale.currentQuestion().correctAnswer)); QCOMPARE(stale.answeredCount(),0);
    QVERIFY(!stale.revealHintSlot(slot)); QCOMPARE(repo.checkpoint(id).payload["revealed"].toInt(),1<<slot);
    const auto saved=repo.checkpoint(id); QVERIFY(repo.abandonCheckpoint(id,saved.revision));
    QVERIFY(!repo.abandonCheckpoint(id,saved.revision)); QCOMPARE(repo.checkpoint(id).state,QString("abandoned"));
    QCOMPARE(repo.checkpoint(id).payload,saved.payload); QVERIFY(repo.recoverableSessions().isEmpty()); QVERIFY(repo.loadTopResults().isEmpty());
    QVERIFY(!first.submitAnswer(first.currentQuestion().correctAnswer)); QVERIFY(!stale.recoverSession(id));
    Inspection inspection(path); QCOMPARE(inspection.count("session_checkpoints"),1); QCOMPARE(inspection.count("game_runs"),0);
}

void SessionRecoveryTests::invalidSnapshotsAndChangedContentAreRetainedAndRefused()
{
    QTemporaryDir dir; const auto path=dir.filePath("invalid.sqlite"); SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog;
    GameService game(&catalog,&repo); start(game,repo); const auto id=game.sessionId();
    EducationalContent different(QJsonObject{{"synthetic_changed_content",true}}); GameService changed(&catalog,&repo,different); changed.setCheckpointRepository(&repo);
    QVERIFY(!changed.recoverSession(id)); QCOMPARE(repo.checkpoint(id).state,QString("active"));
    Inspection inspection(path); QVERIFY(inspection.exec("UPDATE session_checkpoints SET payload='{}'"));
    QVERIFY(!repo.checkpoint(id).error.isEmpty()); QCOMPARE(repo.recoverableSessions().size(),1);
    GameService refused(&catalog,&repo); refused.setCheckpointRepository(&repo); QVERIFY(!refused.recoverSession(id));
    const auto corrupted=repo.checkpoint(id); QVERIFY(repo.abandonCheckpoint(id,corrupted.revision));
    QCOMPARE(inspection.count("session_checkpoints"),1); QCOMPARE(inspection.count("ranking_records"),0);
}

void SessionRecoveryTests::transitionsCannotEraseAnswersOrHints()
{
    QTemporaryDir dir; SqliteGameHistoryRepository repo(dir.filePath("transitions.sqlite")); JsonSpeciesCatalog catalog;
    GameService game(&catalog,&repo); start(game,repo); const auto id=game.sessionId();
    const auto unhinted=game.sessionSnapshot(); const int slot=availableSlot(game); QVERIFY(slot>=0); QVERIFY(game.revealHintSlot(slot));
    auto saved=repo.checkpoint(id); qint64 revision=saved.revision;
    QVERIFY(!repo.writeCheckpoint(unhinted,saved.revision,revision));
    QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer)); saved=repo.checkpoint(id);
    QVERIFY(!repo.writeCheckpoint(unhinted,saved.revision,revision));
    auto changed=game.sessionSnapshot(); changed["units"]=60000;
    QVERIFY(!repo.writeCheckpoint(changed,saved.revision,revision));
    auto questions=changed["questions"].toArray(); auto old=questions.first().toObject(); old["prompt"]="changed already presented question"; questions[0]=old;
    changed=game.sessionSnapshot(); changed["questions"]=questions;
    QVERIFY(!repo.writeCheckpoint(changed,saved.revision,revision));
    QCOMPARE(repo.checkpoint(id).payload,game.sessionSnapshot());
}

void SessionRecoveryTests::v5MigrationPreservesIdsResultsAndCanRepeat()
{
    QTemporaryDir dir; const auto path=dir.filePath("v5.sqlite"); QString owner;
    { SqliteGameHistoryRepository repo(path); const auto p=repo.createPlayer("João"); owner=p.id; QVERIFY(repo.saveResult(p.identity(),18,3)); }
    {
        Inspection i(path); QVERIFY(i.exec("DROP TRIGGER checkpoint_identity_immutable")); QVERIFY(i.exec("DROP TRIGGER checkpoint_retained"));
        QVERIFY(i.exec("DROP TABLE session_checkpoints")); QVERIFY(i.exec("PRAGMA user_version=5"));
    }
    for (int i=0;i<3;++i) {
        SqliteGameHistoryRepository repo(path); QCOMPARE(repo.player(owner).displayName,QString("João"));
        QCOMPARE(repo.loadPlayerResult("free",owner).first().scoreUnits,qint64(18)); QVERIFY(repo.recoverableSessions().isEmpty());
        Inspection inspection(path); QCOMPARE(inspection.count("player_name_policy"),1); QCOMPARE(inspection.count("ranking_records"),1);
    }
}

void SessionRecoveryTests::abruptProcess_data()
{
    QTest::addColumn<QString>("mode"); for (const auto &mode:{"hint","answer","final","uncommitted"}) QTest::newRow(mode)<<QString(mode);
}
void SessionRecoveryTests::abruptProcess()
{
    QFETCH(QString,mode); QTemporaryDir dir; const auto path=dir.filePath("crash.sqlite");
    QProcess child; child.start(QCoreApplication::applicationFilePath(),{"--checkpoint-child",path,mode});
    QVERIFY(child.waitForFinished(30000)); QCOMPARE(child.exitCode(),0);
    SqliteGameHistoryRepository repo(path); QCOMPARE(repo.recoverableSessions().size(),1); const auto checkpoint=repo.recoverableSessions().first();
    QVERIFY(checkpoint.error.isEmpty()); JsonSpeciesCatalog catalog; GameService recovered(&catalog,&repo); recovered.setCheckpointRepository(&repo);
    QVERIFY(recovered.recoverSession(checkpoint.id)); QCOMPARE(recovered.answeredCount(),mode=="hint" ? 0 : mode=="final" ? 3 : 1);
    QCOMPARE(recovered.isFinished(),mode=="final"); QCOMPARE(recovered.scoreUnits(),qint64(recovered.answeredCount()*6));
    if (mode=="hint") QVERIFY(recovered.revealedHints()>0);
    if (mode=="uncommitted") QCOMPARE(checkpoint.revision,qint64(2));
    Inspection inspection(path); QCOMPARE(inspection.count("game_runs"),0); QCOMPARE(inspection.count("ranking_records"),0);
}

void SessionRecoveryTests::consistentBackupRestoresAValidSession()
{
    QTemporaryDir dir; const auto path=dir.filePath("source.sqlite"),backup=dir.filePath("backup.sqlite"),restored=dir.filePath("restored.sqlite");
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); start(game,repo);
    QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer)); const auto expected=game.sessionSnapshot();
    auto run=[&](const QStringList &args) { QProcess process; process.start(QStringLiteral(GLOSSATA_PYTHON),QStringList{QStringLiteral(GLOSSATA_BACKUP_SCRIPT)}+args); return process.waitForFinished(30000) && process.exitCode()==0; };
    QVERIFY(run({"create","--source",path,"--output",backup}));
    QVERIFY(run({"verify","--source",backup}));
    QVERIFY(run({"restore","--source",backup,"--output",restored}));
    SqliteGameHistoryRepository recoveredRepo(restored); GameService recovered(&catalog,&recoveredRepo); recovered.setCheckpointRepository(&recoveredRepo);
    QVERIFY(recovered.recoverSession(game.sessionId())); QCOMPARE(recovered.sessionSnapshot(),expected);
    QVERIFY(!run({"restore","--source",backup,"--output",path})); // Never overwrite a live/previous database.
}

void SessionRecoveryTests::failedInitialWriteAndMigrationPreserveExistingResults()
{
    QTemporaryDir dir; const auto path=dir.filePath("initial-failure.sqlite"); QString owner;
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo);
        const auto p=repo.createPlayer("Synthetic player"); owner=p.id; QVERIFY(repo.saveResult(p.identity(),12,2));
        Inspection i(path); QVERIFY(i.exec("CREATE TRIGGER fail_initial BEFORE INSERT ON session_checkpoints BEGIN SELECT RAISE(ABORT,'synthetic first checkpoint failure'); END"));
        start(game,repo); QVERIFY(game.isFinished()); QVERIFY(!game.checkpointError().isEmpty());
        QCOMPARE(i.count("session_checkpoints"),0); QCOMPARE(repo.loadPlayerResult("free",owner).first().scoreUnits,qint64(12));
        QVERIFY(i.exec("DROP TRIGGER fail_initial"));
        QVERIFY(i.exec("DROP TRIGGER checkpoint_identity_immutable")); QVERIFY(i.exec("DROP TRIGGER checkpoint_retained"));
        QVERIFY(i.exec("DROP TABLE session_checkpoints")); QVERIFY(i.exec("CREATE VIEW session_checkpoints AS SELECT 1 AS id")); QVERIFY(i.exec("PRAGMA user_version=5"));
    }
    { SqliteGameHistoryRepository failed(path); QVERIFY(failed.profiles().isEmpty()); }
    {
        Inspection i(path); QSqlQuery q(i.db); QVERIFY(q.exec("PRAGMA user_version")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),5); q.finish();
        QVERIFY(q.exec("SELECT best_units FROM ranking_records")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),12); q.finish();
        QVERIFY(i.exec("DROP VIEW session_checkpoints"));
    }
    SqliteGameHistoryRepository recovered(path); QCOMPARE(recovered.player(owner).displayName,QString("Synthetic player"));
    QCOMPARE(recovered.loadPlayerResult("free",owner).first().scoreUnits,qint64(12));
}

void SessionRecoveryTests::retiredOwnerAndReusedNameRemainSeparateAfterRecovery()
{
    QTemporaryDir dir; const auto path=dir.filePath("owner.sqlite"); QString owner,replacement,id;
    {
        SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService game(&catalog,&repo); start(game,repo);
        owner=game.sessionPlayer().id; id=game.sessionId(); QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
        QVERIFY(repo.deletePlayer(owner)); replacement=repo.createPlayer("Synthetic player").id; QVERIFY(!replacement.isEmpty()); QVERIFY(owner!=replacement);
    }
    SqliteGameHistoryRepository repo(path); JsonSpeciesCatalog catalog; GameService recovered(&catalog,&repo); recovered.setCheckpointRepository(&repo);
    QVERIFY(recovered.recoverSession(id)); QCOMPARE(recovered.sessionPlayer().id,owner);
    while (!recovered.isFinished()) QVERIFY(recovered.submitAnswer(recovered.currentQuestion().correctAnswer));
    QCOMPARE(recovered.saveCurrentResult(),GameService::SaveStatus::Saved);
    const auto result=repo.loadPlayerResult(recovered.configuration().rankingKey(),owner).first();
    QVERIFY(result.deletedProfile); QCOMPARE(result.scoreUnits,qint64(18));
    QVERIFY(repo.loadPlayerResult(recovered.configuration().rankingKey(),replacement).isEmpty());
}

int main(int argc,char **argv)
{
    QCoreApplication application(argc,argv);
    if (argc==4 && QString::fromLocal8Bit(argv[1])=="--checkpoint-child") return worker(QString::fromLocal8Bit(argv[2]),QString::fromLocal8Bit(argv[3]));
    SessionRecoveryTests tests; return QTest::qExec(&tests,argc,argv);
}
#include "SessionRecoveryTests.moc"
