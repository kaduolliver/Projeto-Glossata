// Tests included in GameServiceTests.cpp; all persistence is temporary.
namespace {
QVariantMap achievementRow(const QVariantList &rows, const QString &id) {
    for (const auto &v:rows) if (v.toMap().value("id")==id) return v.toMap();
    return {};
}
QString achievementAnswer(const GameController &controller, const QVector<Species> &species) {
    for (const auto &s:species) if (s.id==controller.questionPhoto().value("speciesId").toInt())
        return s.answerAtRank("genus");
    return {};
}
}

void GameServiceTests::achievementCriteriaAndInvalidEvents()
{
    QCOMPARE(Achievements::definitions().size(),30);
    QSet<QString> ids,badges,levels;
    for (const auto &d:Achievements::definitions()) {
        QVERIFY(!ids.contains(d.id)); ids.insert(d.id); badges.insert(d.icon); QVERIFY(d.target>0);
        const auto locked=Achievements::present(d,0,{},false),unlocked=Achievements::present(d,d.target,"2026-01-01T00:00:00Z",false);
        QVERIFY(!locked["visualLevelName"].toString().isEmpty());
        QCOMPARE(locked["visualLevel"],unlocked["visualLevel"]);
        levels.insert(locked["visualLevel"].toString());
    }
    QCOMPARE(levels,(QSet<QString>{"initial","intermediate","advanced","special"}));
    QCOMPARE(badges.size(),30);
    AchievementEvidence e;AchievementRun run;run.id="synthetic";run.complete=true;run.hintsKnown=true;run.cycleSize=10;
    QVector<Species> species;
    for(int i=1;i<=10;++i) {
        Species s;s.id=i;s.name=QString("Synthetic %1").arg(i);s.taxonomy={{"group:"+QString::number(i%3),"Synthetic group "+QString::number(i%3),"kingdom"}};species.append(s);
        run.answers.append({i,i,s.name,"photo","genus","Synthetic","Synthetic",true,0,6,6});
    }
    e.runs={run};auto p=Achievements::evaluate(e,species);
    QCOMPARE(p["correct_10"],10);QCOMPARE(p["correct_50"],10);QCOMPARE(p["perfect_game"],1);QCOMPARE(p["no_hint_game"],1);
    QCOMPARE(p["groups_3"],3);QCOMPARE(p["species_10"],10);QCOMPARE(p["cycle"],1);
    e.runs.append(run);QCOMPARE(Achievements::evaluate(e,species),p);
    e.runs={run};e.runs[0].answers.append(run.answers.first());
    auto duplicated=Achievements::evaluate(e,species);QCOMPARE(duplicated["correct_50"],10);QCOMPARE(duplicated["perfect_game"],0);
    e.runs={run};e.runs[0].pendingHints=1;QCOMPARE(Achievements::evaluate(e,species)["no_hint_game"],0);
    e.runs[0].pendingHints=0;e.runs[0].hintsKnown=false;QCOMPARE(Achievements::evaluate(e,species)["no_hint_game"],0);
    e.runs={run};e.runs[0].answers[0].awardedUnits=999;QCOMPARE(Achievements::evaluate(e,species)["correct_50"],9);
    e.runs={run};e.runs[0].complete=false;QCOMPARE(Achievements::evaluate(e,species)["first_game"],0);
    e.runs={run};e.runs[0].answers[3].correct=false;e.runs[0].answers[3].selectedAnswer="Wrong";e.runs[0].answers[3].awardedUnits=0;
    QCOMPARE(Achievements::evaluate(e,species)["perfect_game"],0);
    e.runs={run};e.runs[0].competitive=true;QCOMPARE(Achievements::evaluate(e,species)["competitive_perfect"],1);
    e.runs[0].complete=false;QCOMPARE(Achievements::evaluate(e,species)["competitive_perfect"],0);
    e.runs={run};for(auto &s:species) s.name="Changed identity";QCOMPARE(Achievements::evaluate(e,species)["groups_3"],0);
}

void GameServiceTests::achievementSpeciesIdentity_data()
{
    QTest::addColumn<QString>("scenario"); QTest::addColumn<int>("expected");
    for (const auto &s : {"valid", "duplicate_answer", "same_organism", "unapproved_taxonomy"})
        QTest::newRow(s) << QString(s) << 1;
    for (const auto &s : {"missing_organism", "reused_id", "divergent_name", "missing_name", "missing_catalog_name", "invalid_score"})
        QTest::newRow(s) << QString(s) << 0;
}

void GameServiceTests::achievementSpeciesIdentity()
{
    QFETCH(QString,scenario); QFETCH(int,expected);
    Species s; s.id=1; s.name="Synthetic organism";
    s.taxonomy={{"kingdom:Synthetic","Synthetic","kingdom"}};
    AchievementRun run; run.id="identity";
    run.answers={{1,1,s.name,"photo","genus","Synthetic","Synthetic",true,0,6,6}};
    QVector<Species> catalog{s};
    if (scenario=="missing_organism") catalog.clear();
    if (scenario=="reused_id") catalog[0].name="Another organism";
    if (scenario=="divergent_name") run.answers[0].scientificName="Other name";
    if (scenario=="missing_name") run.answers[0].scientificName.clear();
    if (scenario=="missing_catalog_name") catalog[0].name.clear();
    if (scenario=="invalid_score") run.answers[0].awardedUnits=999;
    if (scenario=="duplicate_answer") run.answers.append(run.answers.first());
    if (scenario=="same_organism") { auto a=run.answers.first(); a.questionNumber=2; run.answers.append(a); }
    if (scenario=="unapproved_taxonomy") catalog[0].taxonomyApproved=false;
    AchievementEvidence evidence; evidence.runs={run};
    const auto progress=Achievements::evaluate(evidence,catalog);
    QCOMPARE(progress["species_10"],expected);
    if (scenario=="unapproved_taxonomy") QCOMPARE(progress["groups_3"],0);
}

void GameServiceTests::achievementEvidencePriority_data()
{
    QTest::addColumn<QString>("scenario"); QTest::addColumn<int>("expectedAnswers"); QTest::addColumn<bool>("complete");
    QTest::newRow("detailed_result_and_checkpoint") << QString("detailed") << 10 << true;
    QTest::newRow("missing_details_valid_checkpoint") << QString("missing") << 10 << false;
    QTest::newRow("invalid_details_valid_checkpoint") << QString("bad_details") << 10 << false;
    QTest::newRow("totals_disagree_valid_checkpoint") << QString("bad_total") << 10 << false;
    QTest::newRow("missing_details_invalid_checkpoint") << QString("bad_checkpoint") << 0 << false;
    QTest::newRow("active_with_aggregate_result") << QString("active") << 10 << false;
    QTest::newRow("abandoned_with_aggregate_result") << QString("abandoned") << 10 << false;
    QTest::newRow("recovered_session") << QString("recovered") << 10 << true;
    QTest::newRow("competitive_incomplete") << QString("competitive") << 2 << false;
    QTest::newRow("other_owner_checkpoint") << QString("other_owner") << 0 << false;
}

void GameServiceTests::achievementEvidencePriority()
{
    QFETCH(QString,scenario); QFETCH(int,expectedAnswers); QFETCH(bool,complete);
    QTemporaryDir dir; const auto path=dir.filePath("evidence.sqlite");
    JsonSpeciesCatalog catalog; SqliteGameHistoryRepository repo(path);
    QVERIFY(repo.initializeLocalUser()); const auto owner=repo.localUser();
    const auto player=scenario=="other_owner" ? repo.createPlayer("Other owner") : owner;
    GameService game(&catalog,&repo); game.setCheckpointRepository(&repo);
    auto config=GameService::competitiveConfiguration();
    if (scenario!="competitive") { config.mode=GameMode::Free; config.questionLimit=0; }
    else config.questionLimit=3;
    game.startConfiguredGame(config,player.identity());
    const int count=scenario=="competitive" ? 2 : 10;
    for (int i=0;i<count;++i) QVERIFY(game.submitAnswer(game.currentQuestion().correctAnswer));
    int slot=-1; for (int i=0;i<3;++i) if (game.currentQuestion().hints[i].available()) {slot=i;break;}
    QVERIFY(slot>=0); QVERIFY(game.revealHintSlot(slot)); // trailing, unanswered hint is evidence too
    const auto id=game.sessionId();
    if (scenario=="recovered") {
        GameService recovered(&catalog,&repo); recovered.setCheckpointRepository(&repo);
        QVERIFY(recovered.recoverSession(id)); QCOMPARE(recovered.answeredCount(),10);
        recovered.finishGame(); QCOMPARE(recovered.saveCurrentResult(),GameService::SaveStatus::Saved);
    } else if (scenario=="abandoned") {
        const auto saved=repo.checkpoint(id); QVERIFY(repo.abandonCheckpoint(id,saved.revision));
    } else if (scenario!="active" && scenario!="competitive" && scenario!="other_owner") {
        game.finishGame(); QCOMPARE(game.saveCurrentResult(),GameService::SaveStatus::Saved);
    }
    const auto connection=QUuid::createUuid().toString();
    auto inspection=QSqlDatabase::addDatabase("QSQLITE",connection); inspection.setDatabaseName(path); QVERIFY(inspection.open());
    const auto close=qScopeGuard([&]{inspection.close();inspection={};QSqlDatabase::removeDatabase(connection);});
    QSqlQuery q(inspection);
    if (scenario=="active" || scenario=="abandoned" || scenario=="competitive" || scenario=="other_owner") {
        // Synthetic aggregate legacy result sharing a real checkpoint ID; no production history is edited.
        q.prepare("INSERT INTO game_runs(id,player_id,player_name,finished_at,score_units,correct_answers,answered_count,is_complete) VALUES(?,?,?,'2026-01-01T00:00:00Z',60,10,0,1)");
        q.addBindValue(id); q.addBindValue(owner.id); q.addBindValue(owner.displayName); QVERIFY(q.exec());
    }
    if (scenario=="missing" || scenario=="bad_checkpoint") QVERIFY(q.exec("DELETE FROM answer_records"));
    if (scenario=="bad_details") QVERIFY(q.exec("UPDATE answer_records SET hint_snapshot='{}' WHERE question_number=1"));
    if (scenario=="bad_total") QVERIFY(q.exec("UPDATE game_runs SET score_units=999"));
    if (scenario=="bad_checkpoint") QVERIFY(q.exec("UPDATE session_checkpoints SET sha256='"+QString(64,'0')+"'"));
    const auto fingerprint=[&] {
        QStringList rows;
        for (const auto &table : {"game_runs","answer_records","session_checkpoints"}) {
            q.exec("SELECT * FROM "+QString(table)+" ORDER BY 1");
            while(q.next()) for(int i=0;i<q.record().count();++i) rows.append(q.value(i).toString());
        }
        return rows;
    };
    const auto history=fingerprint(); QVariantList rows,events;
    QVERIFY(repo.refreshAchievements(owner.id,catalog.loadCurrentSpecies(),true,rows,events)); QVERIFY(events.isEmpty());
    QCOMPARE(achievementRow(rows,"answered_100")["progress"].toInt(),expectedAnswers);
    QCOMPARE(achievementRow(rows,"first_game")["unlocked"].toBool(),complete);
    QVERIFY(!achievementRow(rows,"first_competitive")["unlocked"].toBool());
    QVERIFY(!achievementRow(rows,"no_hint_game")["unlocked"].toBool());
    QCOMPARE(achievementRow(rows,"first_hint")["unlocked"].toBool(),expectedAnswers>0);
    const auto first=rows;
    for(int i=0;i<2;++i) { QVERIFY(repo.refreshAchievements(owner.id,catalog.loadCurrentSpecies(),true,rows,events)); QCOMPARE(rows,first); QVERIFY(events.isEmpty()); }
    SqliteGameHistoryRepository reopened(path); QVERIFY(reopened.initializeLocalUser());
    QVERIFY(reopened.refreshAchievements(owner.id,catalog.loadCurrentSpecies(),true,rows,events)); QCOMPARE(rows,first); QVERIFY(events.isEmpty());
    QCOMPARE(fingerprint(),history); // no repairs, score rewrites, ownership changes or history deletion
}

void GameServiceTests::achievementsPersistRecoverAndEmitOnce()
{
    QTemporaryDir dir;const auto path=dir.filePath("achievements.sqlite");QString owner,session,date;
    JsonSpeciesCatalog catalog;const auto species=catalog.loadCurrentSpecies();
    {
        GameController c(nullptr,path);owner=c.currentPlayerId();QSignalSpy unlocks(&c,&GameController::achievementUnlocked);
        QVERIFY(c.startTaxonGame({},"genus"));
        c.answerSelected("not an option");QCOMPARE(unlocks.size(),0);
        for(int i=0;i<3;++i) {c.answerSelected(achievementAnswer(c,species));c.nextQuestion();}
        const auto row=achievementRow(c.achievements(),"first_answer");QVERIFY(row["unlocked"].toBool());date=row["unlockedAt"].toString();
        QCOMPARE(achievementRow(c.achievements(),"correct_5")["progress"].toInt(),3);
        const auto count=unlocks.size();c.answerSelected("not an option");QCOMPARE(unlocks.size(),count);
        SqliteGameHistoryRepository repo(path);session=repo.recoverableSessions().first().id;
        QVERIFY(c.renameCurrentPlayerProfile("Renamed learner"));QCOMPARE(c.currentPlayerId(),owner);QCOMPARE(achievementRow(c.achievements(),"first_answer")["unlockedAt"].toString(),date);
    }
    {
        GameController c(nullptr,path);QSignalSpy unlocks(&c,&GameController::achievementUnlocked);QCOMPARE(c.currentPlayerId(),owner);
        QCOMPARE(achievementRow(c.achievements(),"correct_5")["progress"].toInt(),3);QVERIFY(c.resumeSession(session));QCOMPARE(unlocks.size(),0);
        for(int i=0;i<2;++i){c.answerSelected(achievementAnswer(c,species));const auto n=unlocks.size();c.answerSelected(achievementAnswer(c,species));QCOMPARE(unlocks.size(),n);c.nextQuestion();}
        QCOMPARE(achievementRow(c.achievements(),"correct_5")["progress"].toInt(),5);QVERIFY(achievementRow(c.achievements(),"correct_5")["unlocked"].toBool());
        c.stopGame();const auto rows=c.achievements();const auto n=unlocks.size();c.retrySaveResult();QCOMPARE(c.achievements(),rows);QCOMPARE(unlocks.size(),n);
        QVERIFY(c.startTaxonGame({},"genus"));QCOMPARE(c.achievements(),rows);
    }
    GameController reopened(nullptr,path);QCOMPARE(achievementRow(reopened.achievements(),"first_answer")["unlockedAt"].toString(),date);
    QCOMPARE(achievementRow(reopened.achievements(),"correct_5")["progress"].toInt(),5);
}

void GameServiceTests::achievementHintsAndCompletion()
{
    QTemporaryDir dir;JsonSpeciesCatalog catalog;const auto species=catalog.loadCurrentSpecies();
    const auto recoveryPath=dir.filePath("hint-recovery.sqlite");QString recoveryId;
    {
        GameController active(nullptr,recoveryPath);QVERIFY(active.startTaxonGame({},"genus"));
        for(int i=0;i<10;++i){active.answerSelected(achievementAnswer(active,species));active.nextQuestion();}
        QString hint;for(const auto &v:active.hints())if(v.toMap()["available"].toBool()){hint=v.toMap()["id"].toString();break;}
        QVERIFY(!hint.isEmpty());QVERIFY(active.revealHintCategory(hint));
        SqliteGameHistoryRepository repo(recoveryPath);recoveryId=repo.recoverableSessions().first().id;
    }
    {
        GameController recovered(nullptr,recoveryPath);QVERIFY(recovered.resumeSession(recoveryId));recovered.stopGame();
        QVERIFY(!achievementRow(recovered.achievements(),"no_hint_game")["unlocked"].toBool());
        QVERIFY(achievementRow(recovered.achievements(),"first_hint")["unlocked"].toBool());
        QCOMPARE(achievementRow(recovered.achievements(),"answered_100")["progress"].toInt(),10);
    }
    GameController c(nullptr,dir.filePath("hints.sqlite"));QVERIFY(c.startTaxonGame({},"genus"));
    for(int i=0;i<10;++i){c.answerSelected(achievementAnswer(c,species));c.nextQuestion();}
    QString hint;for(const auto &v:c.hints())if(v.toMap()["available"].toBool()){hint=v.toMap()["id"].toString();break;}
    QVERIFY(!hint.isEmpty());QVERIFY(c.revealHintCategory(hint));QVERIFY(achievementRow(c.achievements(),"first_hint")["unlocked"].toBool());
    c.stopGame();QVERIFY(!achievementRow(c.achievements(),"no_hint_game")["unlocked"].toBool());QVERIFY(achievementRow(c.achievements(),"perfect_game")["unlocked"].toBool());
    QVERIFY(c.startCompetitiveGame());c.answerSelected(achievementAnswer(c,species));c.stopGame();
    QVERIFY(!achievementRow(c.achievements(),"first_competitive")["unlocked"].toBool());
    QVERIFY(c.startCompetitiveGame());
    while(!c.gameFinished()){c.answerSelected(achievementAnswer(c,species));c.nextQuestion();}
    QVERIFY(!c.resultPending());QVERIFY(achievementRow(c.achievements(),"first_competitive")["unlocked"].toBool());
    QVERIFY(achievementRow(c.achievements(),"competitive_no_hint")["unlocked"].toBool());QVERIFY(achievementRow(c.achievements(),"competitive_perfect")["unlocked"].toBool());
    QVERIFY(achievementRow(c.achievements(),"no_hint_game")["unlocked"].toBool());
}

void GameServiceTests::achievementExplorationAndHistoricalRecognition()
{
    QTemporaryDir dir;const auto path=dir.filePath("historical.sqlite");QString owner,other;
    JsonSpeciesCatalog catalog;const auto species=catalog.loadCurrentSpecies();
    {
        SqliteGameHistoryRepository repo(path);const auto a=repo.createPlayer("Local learner"),b=repo.createPlayer("Other learner");owner=a.id;other=b.id;
        QVERIFY(repo.chooseLocalUser(owner));QVector<AnswerRecord> answers;
        for(int i=0;i<10;++i)answers.append({i+1,species[i].id,species[i].name,"photo","genus","Synthetic","Synthetic",true,0,6,6});
        QVERIFY(repo.saveResult(a.identity(),60,10,answers,"verified-history"));
        QVERIFY(repo.saveResult(b.identity(),600,100,{},"unrelated-history"));
        QVariantList rows,events;QVERIFY(!repo.refreshAchievements(other,species,false,rows,events));
    }
    QVariantList stored;
    {
        GameController c(nullptr,path);QSignalSpy unlocks(&c,&GameController::achievementUnlocked);
        QCOMPARE(achievementRow(c.achievements(),"correct_50")["progress"].toInt(),10);
        QVERIFY(achievementRow(c.achievements(),"correct_10")["retrospective"].toBool());
        QVERIFY(!achievementRow(c.achievements(),"no_hint_game")["unlocked"].toBool());
        QVERIFY(!achievementRow(c.achievements(),"visit_1")["unlocked"].toBool());
        QVERIFY(!c.recordExplorationVisit("invalid"));
        QStringList taxa;for(const auto &s:species)for(const auto &t:s.taxonomy)if(t.rank=="species" && t.name==s.name)taxa.append(t.id);
        QVERIFY(taxa.size()>=25);
        c.exploreTaxon(taxa.first());QVERIFY(!achievementRow(c.achievements(),"visit_1")["unlocked"].toBool());
        QVERIFY(c.recordExplorationVisit(taxa.first()));const int n=unlocks.size();QVERIFY(c.recordExplorationVisit(taxa.first()));QCOMPARE(unlocks.size(),n);
        for(int i=1;i<25;++i){c.exploreTaxon(taxa[i]);QVERIFY(c.recordExplorationVisit(taxa[i]));}
        QVERIFY(achievementRow(c.achievements(),"visit_25")["unlocked"].toBool());stored=c.achievements();
    }
    GameController c(nullptr,path);QCOMPARE(c.achievements(),stored);QCOMPARE(c.currentPlayerId(),owner);
}
