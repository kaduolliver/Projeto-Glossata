#include "presentation/GameController.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include "infrastructure/discord/DiscordRichPresence.h"
#include <QIcon>
#include <QtTest/QtTest>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QTemporaryDir>
#include <QDir>
#include <functional>
#include <QColor>
#include <QSet>

namespace {
QStringList interfaceWarnings;
QString scientificGroup(const QString &name) {
    JsonSpeciesCatalog catalog;
    for(const auto &node : catalog.loadTaxons()) if(node.name==name)return node.id;
    return {};
}
QQuickItem *findVisualItem(QQuickItem *root, const QString &name)
{
    if (root->objectName() == name) return root;
    for (QQuickItem *child : root->childItems())
        if (QQuickItem *found = findVisualItem(child, name)) return found;
    return nullptr;
}
void captureMessage(QtMsgType type, const QMessageLogContext &, const QString &message)
{
    if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) interfaceWarnings << message;
}
}

class QmlInterfaceTests : public QObject {
    Q_OBJECT
private slots:
    void photographEdgesFadeWithoutHidingTheOrganism();
    void explorationCardsKeepBackgroundProportions();
    void navigateAndConfirmAnswers();
};

void QmlInterfaceTests::photographEdgesFadeWithoutHidingTheOrganism()
{
    QQmlEngine engine;
    QQmlComponent component(&engine,QUrl("qrc:/resources/qml/components/FeatheredPhoto.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object,qPrintable(component.errorString()));
    auto photo=qobject_cast<QQuickItem *>(object.data());QVERIFY(photo);
    QQuickWindow window;window.resize(200,160);photo->setParentItem(window.contentItem());
    photo->setSize(QSizeF(160,120));window.show();
    QVERIFY(photo->setProperty("source",QUrl("qrc:/resources/images/exploration/amanita.jpg")));
    QTRY_VERIFY_WITH_TIMEOUT(photo->property("ready").toBool(),3000);
    QTest::qWait(100);
    auto grab=photo->grabToImage();QVERIFY(grab);
    QTRY_VERIFY(!grab->image().isNull());
    const auto image=grab->image();
    QVERIFY(image.hasAlphaChannel());
    QVERIFY(image.pixelColor(120,60).alpha()>240);
    QVERIFY(image.pixelColor(1,60).alpha()<30);
    QVERIFY(image.pixelColor(120,4).alpha()>240); // Full height, without top/bottom fade or letterboxing.
    QVERIFY(image.pixelColor(159,0).alpha()<30); // Rounded outer corner.
    QVERIFY(image.pixelColor(10,60).alpha()>image.pixelColor(1,60).alpha());
    QVERIFY(image.pixelColor(10,60).alpha()<image.pixelColor(80,60).alpha());
    QVERIFY(photo->setProperty("source",QUrl("qrc:/resources/images/exploration/bacteria.jpg")));
    photo->setSize(QSizeF(180,96));
    QTRY_VERIFY_WITH_TIMEOUT(photo->property("ready").toBool(),3000);
    QTest::qWait(100);
    auto portrait=photo->grabToImage();QVERIFY(portrait);QTRY_VERIFY(!portrait->image().isNull());
    QVERIFY(portrait->image().pixelColor(150,48).alpha()>240);
    QVERIFY(portrait->image().pixelColor(150,3).alpha()>240);
    QVERIFY(portrait->image().pixelColor(55,48).alpha()>50);
    QVERIFY(photo->setProperty("source",QUrl()));
    QVERIFY(!photo->property("ready").toBool());
}

void QmlInterfaceTests::explorationCardsKeepBackgroundProportions()
{
    QVERIFY(QDir().mkpath("previews"));
    QQmlEngine engine;
    QQmlComponent component(&engine,QUrl("qrc:/resources/qml/components/ExploreCard.qml"));
    const QVariantMap entry{{"title","A biodiversidade de Eukaryota"},{"rankLabel","Supergrupo"},
        {"count",122},{"kingdom","Eukaryota"},{"visualCount",100},{"playableCount",100},
        {"photo",QVariantMap{{"image","qrc:/resources/images/exploration/amanita.jpg"},
            {"subject","Amanita muscaria"},{"author","Bernard Spragg. NZ"},{"license","CC0 1.0"}}}};
    QScopedPointer<QObject> object(component.createWithInitialProperties({{"entry",entry}}));
    QVERIFY2(object,qPrintable(component.errorString()));
    auto card=qobject_cast<QQuickItem *>(object.data());QVERIFY(card);
    QQuickWindow window;window.resize(1100,420);card->setParentItem(window.contentItem());window.show();
    for(const QSize size:{QSize(300,185),QSize(420,185),QSize(600,132),QSize(980,132)}) {
        card->setProperty("wide",size.height()==132);card->setSize(size);
        auto photo=findVisualItem(card,"explorationPhoto");QVERIFY(photo);
        QTRY_VERIFY(photo->property("ready").toBool());QTest::qWait(60);
        QCOMPARE(photo->height(),double(size.height()));
        QVERIFY(qAbs(photo->width()/size.width()-0.42)<0.001);
        QCOMPARE(photo->mapToItem(card,QPointF(photo->width(),0)).x(),double(size.width()));
        auto grab=card->grabToImage();QVERIFY(grab);QTRY_VERIFY(!grab->image().isNull());
        QVERIFY(grab->image().save(QString("previews/card-%1.png").arg(size.width())));
        const auto pixels=grab->image();
        QVERIFY(pixels.pixelColor(size.width()-1,0).alpha()<40);
        QVERIFY(pixels.pixelColor(size.width()-5,size.height()/2).alpha()>240);
    }
}

void QmlInterfaceTests::navigateAndConfirmAnswers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    GameController controller(nullptr, directory.filePath("ui.sqlite"));
    DiscordRichPresence presence{QString(), QString()};
    const auto previousHandler = qInstallMessageHandler(captureMessage);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    qInstallMessageHandler(previousHandler);
    QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    QVERIFY(window);
    const QIcon applicationIcon(QStringLiteral(":/resources/branding/glossata.ico"));
    QVERIFY(!applicationIcon.isNull());
    QVERIFY(!applicationIcon.pixmap(32, 32).isNull());
    QDir().mkpath("previews");
    QVERIFY(applicationIcon.pixmap(256, 256).toImage().save("previews/window-icon.png"));
    window->setIcon(applicationIcon);
    window->setProperty("motionEnabled", false);
    QDir().mkpath("previews");
    auto snapshot = [window](const QString &name) {
        QTest::qWait(80);
        const QImage image = window->grabWindow();
        return !image.isNull() && image.save("previews/" + name + ".png");
    };
    auto click = [window](const QString &name) {
        const std::function<QQuickItem *(QQuickItem *)> findItem = [&](QQuickItem *item) -> QQuickItem * {
            if (item->objectName() == name) return item;
            for (QQuickItem *child : item->childItems()) {
                if (QQuickItem *found = findItem(child)) return found;
            }
            return nullptr;
        };
        QQuickItem *button = findItem(window->contentItem());
        if (!button || !button->isVisible() || !button->isEnabled()) return false;
        QPoint point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        if (point.y()<90) {
            if (QQuickItem *scroll=findVisualItem(window->contentItem(),"pageScroll")) {
                scroll->setProperty("contentY",qMax(0.0,scroll->property("contentY").toReal()+point.y()-120));
                QTest::qWait(40);
                point=button->mapToScene(QPointF(button->width()/2,button->height()/2)).toPoint();
            }
        }
        if (point.y() > window->height() - 30) {
            if (QQuickItem *scroll = findVisualItem(window->contentItem(), "pageScroll")) {
                scroll->setProperty("contentY", scroll->property("contentY").toReal() + point.y() - window->height() + 100);
                QTest::qWait(40);
                point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
            }
        }
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
        QTest::qWait(40);
        return true;
    };
    auto showPage = [window](const QString &page) { window->setProperty("page", page); QTest::qWait(30); };
    auto correctAnswer = [&controller]() {
        JsonSpeciesCatalog catalog;
        for (const Species &species : catalog.loadSpecies()) {
            if (species.id == controller.questionPhoto().value("speciesId").toInt()) {
                for (const QString &rank : QStringList{"species","genus","family","order","class","phylum","kingdom"})
                    if (taxonomicRankLabel(rank)==controller.identificationRankLabel()) return species.answerAtRank(rank);
            }
        }
        return QString();
    };

    const auto runtimeHandler = qInstallMessageHandler(captureMessage);
    QTest::qWait(220);
    QVERIFY(snapshot("01-home"));
    showPage("settings");
    QVERIFY(click("creditsButton"));
    QCOMPARE(window->property("page").toString(), QString("credits"));
    QCOMPARE(controller.legalInformation().value("photos").toList().size(), 120);
    const auto presentationPhotos=controller.legalInformation().value("explorationPhotos").toList();
    QVERIFY(presentationPhotos.size()>8);
    QVERIFY(!controller.legalInformation().value("catalogVersion").toString().isEmpty());
    QCOMPARE(controller.legalInformation().value("catalogHash").toString().size(), 64);
    QVERIFY(snapshot("credits-transparency"));
    showPage("home");
    QVERIFY(click("homePlayButton"));
    QCOMPARE(window->property("page").toString(), QString("categories"));
    QVERIFY(snapshot("02-categories"));
    QCOMPARE(controller.categories().size(),4);
    const auto verifyControls=[window](bool taxonomy) {
        const auto selector=findVisualItem(window->contentItem(),"playModeSelector");
        const auto difficulty=findVisualItem(window->contentItem(),"difficultySelector");
        const auto competitive=findVisualItem(window->contentItem(),"startCompetitiveButton");
        const auto later=findVisualItem(window->contentItem(),taxonomy ? "taxonChildrenGrid" : "quickAccessButton0");
        if(!selector || !difficulty || !competitive || !later)return false;
        return selector->mapToScene(QPointF(0,selector->height())).y() <= later->mapToScene(QPointF()).y()
            && competitive->mapToScene(QPointF()).y()<difficulty->mapToScene(QPointF()).y();
    };
    QVERIFY(verifyControls(false));
    for(int group=0;group<3;++group) {
        auto shortcut=findVisualItem(window->contentItem(),"quickAccessButton"+QString::number(group));
        QVERIFY(shortcut);shortcut->forceActiveFocus();QVERIFY(shortcut->hasActiveFocus());
        const QString id=controller.quickAccessGroups()[group].toMap()["id"].toString();
        QVERIFY(click("quickAccessButton"+QString::number(group)));
        QCOMPARE(window->property("page").toString(),QString("taxonomy"));
        QCOMPARE(controller.browseTaxon()["id"].toString(),id);
        QVERIFY(verifyControls(true));
        QCOMPARE(controller.breadcrumbs().first().toMap()["title"].toString(),QString("Eukaryota"));
        QVERIFY(snapshot("quick-access-"+QString::number(group)));
        QVERIFY(click("returnDomainsButton"));
    }
    for(int domain=0;domain<3;++domain) {
        auto card=findVisualItem(window->contentItem(),"domainButton"+QString::number(domain));QVERIFY(card);
        auto photo=findVisualItem(card,"explorationPhoto");QVERIFY(photo);
        QTRY_VERIFY(photo->property("ready").toBool());
        QCOMPARE(photo->height(),card->height());
        QVERIFY(qAbs(photo->width()/card->width()-0.42)<0.001);
    }
    auto categoriesScroll=findVisualItem(window->contentItem(),"pageScroll");QVERIFY(categoriesScroll);
    categoriesScroll->setProperty("contentY",qMax(0.0,categoriesScroll->property("contentHeight").toReal()-categoriesScroll->height()));
    QVERIFY(snapshot("categories-quick-access"));
    categoriesScroll->setProperty("contentY",0);
    window->resize(1000,700);QTest::qWait(40);
    for(int group=0;group<3;++group) {
        auto shortcut=findVisualItem(window->contentItem(),"quickAccessButton"+QString::number(group));QVERIFY(shortcut);
        shortcut->forceActiveFocus();QVERIFY(shortcut->hasActiveFocus());
        const auto position=shortcut->mapToScene(QPointF(shortcut->width()/2,shortcut->height()/2));
        QVERIFY(position.y()>80 && position.y()<window->height()-30);
        QTest::keyClick(window,Qt::Key_Space);QTest::qWait(40);
        QCOMPARE(window->property("page").toString(),QString("taxonomy"));
        QCOMPARE(controller.browseTaxon()["id"],controller.quickAccessGroups()[group].toMap()["id"]);
        QVERIFY(click("returnDomainsButton"));
    }
    QVERIFY(snapshot("compact-quick-access"));
    QVERIFY(verifyControls(false));
    window->resize(1920,1080);QTest::qWait(80);
    QVERIFY(snapshot("categories-wide"));QVERIFY(verifyControls(false));
    window->resize(1240,820);QTest::qWait(40);
    for (int domain = 0; domain < 3; ++domain) {
        QVERIFY(click("domainButton" + QString::number(domain)));
        QCOMPARE(window->property("page").toString(), QString("taxonomy"));
        QCOMPARE(controller.browseTaxon().value("id").toString(), controller.categories().at(domain + 1).toMap().value("id").toString());
        QCOMPARE(controller.browseTaxon().value("rank").toString(),QString("domain"));
        QVERIFY(snapshot("domain-" + QString::number(domain)));
        if(controller.browseTaxon().value("count").toInt()>0) {
            QVERIFY(click("taxonButton0"));
            QVERIFY(controller.breadcrumbs().size() >= 2);
            QVERIFY(click("breadcrumbButton0"));
            QCOMPARE(controller.breadcrumbs().size(), 1);
        }
        if (controller.identificationLevels().isEmpty()) {
            QVERIFY(!click("startTaxonButton"));
            QCOMPARE(controller.browseTaxon().value("count").toInt(),0);
            QVERIFY(click("returnDomainsButton"));
            continue;
        }
        QVERIFY(click("startTaxonButton"));
        QCOMPARE(window->property("page").toString(), QString("quiz"));
        if (controller.browseTaxon().value("title").toString()=="Bacteria") {
            QVERIFY(controller.questionPhoto().value("supporting").toBool());
            QVERIFY(!controller.questionPhoto().value("visual").toBool());
            QVERIFY(controller.questionText().contains("pistas"));
            QVERIFY(snapshot("bacteria-support-quiz"));
            controller.answerSelected(correctAnswer()); showPage("info");
            QVERIFY(controller.answerDetails().value("supporting").toBool());
            QVERIFY(snapshot("bacteria-support-information"));
        }
        controller.stopGame();
        showPage("results");
        QVERIFY(click("returnExplorationButton"));
        QCOMPARE(window->property("page").toString(), QString("taxonomy"));
        QVERIFY(click("returnDomainsButton"));
    }
    for(const QString &clade : QStringList{"Opisthokonta","Archaeplastida","SAR","Amoebozoa","Discoba"}) {
        controller.exploreTaxon("clade:"+clade);showPage("taxonomy");
        QCOMPARE(controller.breadcrumbs().size(),2);
        QCOMPARE(controller.browseTaxon().value("rank").toString(),QString("supergroup"));
        QVERIFY(!controller.browseTaxon().value("sourceUrl").toString().isEmpty());
        QVERIFY(snapshot("lineage-"+clade));
        if(clade=="Discoba") {QVERIFY(controller.identificationLevels().isEmpty());QVERIFY(!click("startTaxonButton"));}
        QVERIFY(click("returnDomainsButton"));
    }
    QVERIFY(click("categoryButton0"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QVERIFY(snapshot("03-quiz"));
    const auto questionPhoto=controller.questionPhoto();
    const int correctIndex = controller.answerOptions().indexOf(correctAnswer());
    QVERIFY(correctIndex >= 0);
    QVERIFY(click("answerOption" + QString::number(correctIndex)));
    QCOMPARE(controller.answeredCount(), 0);
    if (controller.hasHint()) {
        QVERIFY(click("hintButton"));
        const auto offered=controller.hints();int available=-1;
        for(int i=0;i<3;++i)if(offered[i].toMap()["available"].toBool()){available=i;break;}
        QVERIFY(available>=0);QVERIFY(click("hintCategory"+QString::number(available)));
        QVERIFY(click("closeHintPanelButton"));
    }
    const auto hint=controller.extraHint();
    const auto choices=controller.answerOptions();
    const auto round=controller.round();
    const auto selected=findVisualItem(window->contentItem(),"quizView")->property("selectedOption");
    QVERIFY(click("backFromQuizButton"));
    QVERIFY(snapshot("end-confirmation"));
    auto dialog=window->findChild<QObject *>("endGameDialog");
    QVERIFY(dialog && dialog->property("visible").toBool());
    QVERIFY(findVisualItem(window->contentItem(),"continueGameButton")->hasActiveFocus());
    QTest::keyClick(window,Qt::Key_Return);
    QTest::qWait(50);
    QVERIFY(!dialog->property("visible").toBool());
    QCOMPARE(controller.questionPhoto(),questionPhoto);
    QCOMPARE(controller.answerOptions(),choices);
    QCOMPARE(controller.extraHint(),hint);
    QCOMPARE(controller.round(),round);
    QCOMPARE(findVisualItem(window->contentItem(),"quizView")->property("selectedOption"),selected);
    QVERIFY(click("confirmAnswerButton"));
    QCOMPARE(window->property("page").toString(), QString("feedback"));
    QCOMPARE(controller.answeredCount(), 1);
    QVERIFY(controller.answerDetails().value("correct").toBool());
    QCOMPARE(controller.answerDetails().value("image"),questionPhoto.value("image"));
    QCOMPARE(controller.answerDetails().value("photoId"),questionPhoto.value("photoId"));
    QCOMPARE(controller.questionPhoto(),questionPhoto);
    QVERIFY(snapshot("04-correct"));
    showPage("info");
    QVERIFY(snapshot("06-information"));
    const auto described=controller.answerDetails().value("profile").toMap();
    QVERIFY(!described.value("summary").toMap().isEmpty());
    auto displayName=findVisualItem(window->contentItem(),"speciesDisplayName");
    QVERIFY(displayName);
    QCOMPARE(displayName->property("text").toString(),described.value("displayName").toString());
    for (const QString &section : {QString("distribution"),QString("morphology"),QString("conservation")}) {
        auto item=findVisualItem(window->contentItem(),"speciesSection_"+section);
        QVERIFY(item && item->isVisible());
        QVERIFY(item->height()>=38);
    }
    QVERIFY(click("speciesSourcesButton"));
    QVERIFY(snapshot("information-sources"));
    QVERIFY(click("speciesSourcesButton"));
    QVERIFY(click("speciesContinueButton"));
    QCOMPARE(window->property("page").toString(),QString("quiz"));
    QVERIFY(!controller.answerPending());
    controller.answerSelected(correctAnswer());
    showPage("feedback");
    QVERIFY(click("nextQuestionButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QVERIFY(!controller.answerPending());
    const int nextCorrectIndex = controller.answerOptions().indexOf(correctAnswer());
    QVERIFY(click("answerOption" + QString::number((nextCorrectIndex + 1) % 4)));
    QVERIFY(click("confirmAnswerButton"));
    QVERIFY(!controller.answerDetails().value("correct").toBool());
    QVERIFY(snapshot("05-incorrect"));
    controller.nextQuestion();
    showPage("quiz");
    QVERIFY(click("backFromQuizButton"));
    QVERIFY(click("endGameButton"));
    QCOMPARE(window->property("page").toString(),QString("categories"));
    QVERIFY(controller.gameFinished());
    QCOMPARE(controller.ranking().size(),1);
    const auto savedScore=controller.scoreUnits();
    controller.stopGame();
    QCOMPARE(controller.ranking().size(),1);
    QCOMPARE(controller.ranking().first().toMap().value("scoreUnits").toLongLong(),savedScore);
    showPage("results");
    QVERIFY(snapshot("07-results"));
    for (const QString &page : {QString("statistics"), QString("achievements"), QString("settings")}) {
        showPage(page);
        QVERIFY(snapshot(page));
    }
    window->resize(1000, 700);
    controller.startTaxonGame({},"genus");
    showPage("quiz");
    QVERIFY(snapshot("compact-quiz"));
    controller.answerSelected(correctAnswer());
    for (const QString &page : {QString("home"), QString("categories"), QString("taxonomy"), QString("feedback"), QString("info"), QString("results"), QString("statistics"), QString("achievements"), QString("settings")}) {
        showPage(page);
        QVERIFY(snapshot("compact-" + page));
    }
    controller.nextQuestion();
    for (int i = 1; i < controller.catalogSize(); ++i) {
        controller.answerSelected(correctAnswer());
        controller.nextQuestion();
    }
    controller.stopGame();
    {
        SqliteGameHistoryRepository history(directory.filePath("ui.sqlite"));
        for (int i = 1; i <= 5; ++i) QVERIFY(history.saveResult(history.createPlayer("Perfil " + QString::number(i)).identity(), i, i));
    }
    controller.loadRanking();
    showPage("statistics");
    QVERIFY(snapshot("compact-full-statistics"));
    int aggregate=0;
    for (int i=0;i<controller.categoryStats().size();++i) {
        const auto kingdom=controller.categoryStats().at(i).toMap();
        aggregate+=kingdom.value("answered").toInt();
        auto icon=findVisualItem(window->contentItem(),"performanceIcon"+QString::number(i));
        QVERIFY(icon); QCOMPARE(icon->property("kingdom").toString(),kingdom.value("kingdom").toString());
    }
    QCOMPARE(aggregate,controller.answeredCount());
    auto statsScroll=findVisualItem(window->contentItem(),"pageScroll");
    QVERIFY(statsScroll);
    statsScroll->setProperty("contentY",qMax(0.0,statsScroll->property("contentHeight").toReal()-statsScroll->height()));
    QVERIFY(snapshot("compact-statistics-lower-groups"));
    controller.startTaxonGame(scientificGroup("Plantae"), "genus");
    for (int i = 0; i < controller.catalogSize() && controller.questionPhoto().value("image").toString().isEmpty(); ++i) {
        controller.answerSelected(controller.answerOptions().first());
        controller.nextQuestion();
    }
    QVERIFY(!controller.questionPhoto().value("image").toString().isEmpty());
    window->resize(1240, 820);
    showPage("quiz");
    QVERIFY(snapshot("photo-quiz"));
    auto portrait = findVisualItem(window->contentItem(), "questionPortrait");
    QVERIFY(portrait);
    QVERIFY(portrait->property("hasPhoto").toBool());
    controller.answerSelected(correctAnswer());
    QVERIFY(controller.answerPending());
    showPage("info");
    QVERIFY(snapshot("photo-information"));
    window->resize(1000, 700);
    QVERIFY(snapshot("compact-photo-information"));
    controller.stopGame();
    JsonSpeciesCatalog taxonomyCatalog;
    QString orderId;
    for (const Taxon &node : taxonomyCatalog.loadTaxons()) if (node.name == "Lepidoptera") orderId = node.id;
    QVERIFY(!orderId.isEmpty());
    controller.exploreTaxon(orderId);
    showPage("taxonomy");
    auto selector = findVisualItem(window->contentItem(), "identificationLevel");
    QVERIFY(selector);
    int familyIndex = -1;
    for (int i = 0; i < controller.identificationLevels().size(); ++i)
        if (controller.identificationLevels().at(i).toMap().value("id").toString() == "family") familyIndex = i;
    QVERIFY(familyIndex >= 0);
    selector->setProperty("currentIndex", familyIndex);
    auto comboBackground=findVisualItem(selector,"taxonComboBackground");
    QVERIFY(comboBackground);
    QCOMPARE(comboBackground->property("radius").toInt(),12);
    QVERIFY(comboBackground->property("color").value<QColor>().lightness()<160);
    selector->forceActiveFocus();
    QVERIFY(selector->hasActiveFocus());
    QTest::keyClick(window,Qt::Key_Space);
    QTest::qWait(50);
    auto popup=selector->findChild<QObject *>("taxonComboPopup");
    QVERIFY(popup && popup->property("visible").toBool());
    QVERIFY(snapshot("compact-taxonomy-popup"));
    auto popupBackground=findVisualItem(window->contentItem(),"taxonPopupBackground");
    QVERIFY(popupBackground);
    QCOMPARE(popupBackground->property("radius").toInt(),12);
    QVERIFY(popupBackground->property("color").value<QColor>().lightness()<160);
    QTest::keyClick(window,Qt::Key_Escape);
    QTest::qWait(40);
    auto breadcrumb=findVisualItem(window->contentItem(),"breadcrumbButton0");
    QVERIFY(breadcrumb); breadcrumb->forceActiveFocus();
    auto breadcrumbBackground=findVisualItem(breadcrumb,"textActionBackground");
    QVERIFY(breadcrumbBackground);
    QCOMPARE(breadcrumbBackground->property("radius").toInt(),10);
    QVERIFY(breadcrumbBackground->property("color").value<QColor>().lightness()<160);
    QVERIFY(breadcrumb->hasActiveFocus());
    QVERIFY(click("startTaxonButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QCOMPARE(controller.identificationRankLabel(), taxonomicRankLabel("family"));
    QVERIFY(snapshot("compact-family-quiz"));
    controller.answerSelected(controller.answerOptions().first());
    showPage("info");
    QVERIFY(snapshot("compact-family-information"));
    controller.nextQuestion(); showPage("quiz");
    const auto group=controller.sessionTaxonId();
    QVERIFY(click("backFromQuizButton"));
    QVERIFY(snapshot("compact-end-confirmation"));
    QVERIFY(click("continueGameButton"));
    QVERIFY(!controller.gameFinished());
    QVERIFY(click("backFromQuizButton"));
    QVERIFY(click("endGameButton"));
    QCOMPARE(window->property("page").toString(),QString("taxonomy"));
    QCOMPARE(controller.browseTaxon().value("id").toString(),group);
    // Exercise a long, sourced species account rather than relying only on the
    // random question picked by the normal flow, including its lower controls.
    controller.startTaxonGame(scientificGroup("Animalia"),"genus");
    for (int i=0;i<controller.catalogSize() && controller.questionPhoto().value("speciesId").toInt()!=126;++i) {
        controller.answerSelected(correctAnswer()); controller.nextQuestion();
    }
    QCOMPARE(controller.questionPhoto().value("speciesId").toInt(),126);
    window->resize(1240,820); showPage("quiz");
    QVERIFY(snapshot("quiz-lion"));
    controller.answerSelected(correctAnswer());
    window->resize(1240,820); showPage("info");
    QCOMPARE(findVisualItem(window->contentItem(),"speciesDisplayName")->property("text").toString(),QString("Leão"));
    QVERIFY(findVisualItem(window->contentItem(),"speciesSection_nutrition")->isVisible());
    QVERIFY(snapshot("species-lion-information"));
    window->resize(1000,700);
    QVERIFY(snapshot("compact-species-lion-information"));
    QVERIFY(click("speciesClassificationButton"));
    QVERIFY(controller.answerDetails().value("classification").toString().contains("Eukaryota"));
    QVERIFY(controller.answerDetails().value("classification").toString().contains("Opisthokonta"));
    const auto phylogenyReview=controller.answerDetails().value("profile").toMap().value("governance").toMap().value("phylogeny").toMap();
    QCOMPARE(phylogenyReview.value("state").toString(),QString("draft"));
    QVERIFY(phylogenyReview.value("reviewer").toString().isEmpty());
    QVERIFY(snapshot("compact-phylogeny-classification"));
    QVERIFY(click("speciesClassificationButton"));
    QVERIFY(click("speciesSourcesButton"));
    auto speciesScroll=findVisualItem(window->contentItem(),"pageScroll");
    QVERIFY(speciesScroll);
    speciesScroll->setProperty("contentY",qMax(0.0,speciesScroll->property("contentHeight").toReal()-speciesScroll->height()));
    QVERIFY(snapshot("compact-species-lion-sources"));
    QVERIFY(click("speciesContinueButton"));
    QCOMPARE(window->property("page").toString(),QString("quiz"));
    QVERIFY(!controller.answerPending());
    controller.startTaxonGame({},"genus");
    // Keep a stable list while iterating; temporary QVariantLists have different iterators.
    auto hasThree=[&](){const auto h=controller.hints();return h.size()==3 && h[0].toMap()["available"].toBool() && h[1].toMap()["available"].toBool() && h[2].toMap()["available"].toBool();};
    for(int i=0;i<controller.catalogSize() && !hasThree();++i){controller.answerSelected(correctAnswer());controller.nextQuestion();}
    QVERIFY(hasThree());window->resize(1240,820);showPage("quiz");
    const auto pointsBefore=controller.scoreUnits();const int correctBefore=controller.correctAnswers();
    QVERIFY(click("hintButton"));QCOMPARE(controller.possibleScoreText(),QString("1,00"));
    QCOMPARE(controller.scoreUnits(),pointsBefore);
    for(const auto &hint:controller.hints())QVERIFY(hint.toMap()["text"].toString().isEmpty());
    QVERIFY(snapshot("hint-category-selection"));
    QVERIFY(click("hintCategory2"));QCOMPARE(controller.possibleScoreText(),QString("0,33"));
    QCOMPARE(controller.hints()[0].toMap()["pointsText"].toString(),QString("0,33"));
    QVERIFY(findVisualItem(window->contentItem(),"hintCeiling0")->property("text").toString().contains("0,33"));
    QVERIFY(click("hintCategory0"));QCOMPARE(controller.possibleScoreText(),QString("0,33"));
    QVERIFY(click("hintCategory1"));QCOMPARE(controller.possibleScoreText(),QString("0,33"));
    QVERIFY(snapshot("hint-categories-revealed"));
    QVERIFY(click("reportHintButton"));QTRY_VERIFY(findVisualItem(window->contentItem(),"hintReportProblem"));
    findVisualItem(window->contentItem(),"hintReportProblem")->setProperty("text","A dica precisa de correção.");
    QVERIFY(click("prepareHintReportButton"));
    QVERIFY(findVisualItem(window->contentItem(),"hintReportPreview")->property("text").toString().contains("A dica precisa de correção."));
    QVERIFY(snapshot("hint-report-local-draft"));
    // Close the report with Escape; no browser/network operation is performed.
    QTest::keyClick(window,Qt::Key_Escape);QTest::qWait(40);
    window->resize(1000,700);QTest::qWait(40);QVERIFY(snapshot("compact-hint-categories"));
    QVERIFY(click("closeHintPanelButton"));
    controller.answerSelected(correctAnswer());showPage("feedback");
    QCOMPARE(controller.scoreUnits(),pointsBefore+2);QCOMPARE(controller.correctAnswers(),correctBefore+1);
    QCOMPARE(controller.answerDetails()["awardedText"].toString(),QString("0,33"));
    QVERIFY(!controller.revealHintCategory(controller.hints()[0].toMap()["id"].toString()));
    controller.nextQuestion();showPage("quiz");QCOMPARE(controller.possibleScoreText(),QString("1,00"));
    controller.stopGame();showPage("categories");QVERIFY(click("startCompetitiveButton"));
    QCOMPARE(controller.gameMode(),QString("competitive"));
    const auto competitiveHints=controller.hints();
    QVERIFY(std::any_of(competitiveHints.begin(),competitiveHints.end(),[](const QVariant &v){return v.toMap()["available"].toBool();}));
    QSet<int> competitiveSpecies;
    for(int i=0;i<20;++i) {
        const int id=controller.questionPhoto().value("speciesId").toInt();
        QVERIFY(!competitiveSpecies.contains(id)); competitiveSpecies.insert(id);
        const QString correct=correctAnswer();
        const int option=controller.answerOptions().indexOf(correct);QVERIFY(option>=0);
        QVERIFY(click("answerOption"+QString::number(option)));
        QVERIFY(click("confirmAnswerButton"));
        QCOMPARE(window->property("page").toString(),QString("feedback"));
        const int answered=controller.answeredCount();
        controller.answerSelected(correct);QCOMPARE(controller.answeredCount(),answered);
        if(i==19) { QVERIFY(controller.gameFinished()); QVERIFY(snapshot("competitive-final-feedback")); }
        QVERIFY(click("nextQuestionButton"));
        QCOMPARE(window->property("page").toString(),i==19 ? QString("results") : QString("quiz"));
    }
    QCOMPARE(controller.scoreUnits(),qint64(120));QCOMPARE(controller.answeredCount(),20);
    QVERIFY(!controller.resultPending()); QVERIFY(snapshot("competitive-results"));
    showPage("statistics");
    QVERIFY(click("competitiveRankingButton"));QCOMPARE(controller.rankingMode(),QString("competitive"));
    QVERIFY(!controller.ranking().isEmpty());
    QVERIFY(snapshot("competitive-statistics"));
    QVERIFY(click("freeRankingButton"));QCOMPARE(controller.rankingMode(),QString("free"));
    qInstallMessageHandler(runtimeHandler);
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
}

QTEST_MAIN(QmlInterfaceTests)
#include "QmlInterfaceTests.moc"
