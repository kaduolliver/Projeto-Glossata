#include "presentation/GameController.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include "infrastructure/discord/DiscordRichPresence.h"
#include <QIcon>
#include <QtTest/QtTest>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QTemporaryDir>
#include <QDir>
#include <functional>

namespace {
QStringList interfaceWarnings;
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
    void navigateAndConfirmAnswers();
};

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
            if ((!controller.questionPhoto().value("image").toString().isEmpty() && species.image == controller.questionPhoto().value("image").toString()) || (species.fact == controller.factText() && controller.answerOptions().contains(species.name))) return species.name;
        }
        return QString();
    };

    const auto runtimeHandler = qInstallMessageHandler(captureMessage);
    QTest::qWait(220);
    QVERIFY(snapshot("01-home"));
    QVERIFY(click("homePlayButton"));
    QCOMPARE(window->property("page").toString(), QString("categories"));
    QVERIFY(snapshot("02-categories"));
    for (int kingdom = 0; kingdom < 5; ++kingdom) {
        QVERIFY(click("kingdomButton" + QString::number(kingdom)));
        QCOMPARE(window->property("page").toString(), QString("taxonomy"));
        QCOMPARE(controller.browseTaxon().value("id").toString(), controller.categories().at(kingdom + 1).toMap().value("id").toString());
        QVERIFY(snapshot("kingdom-" + QString::number(kingdom)));
        QVERIFY(click("taxonButton0"));
        QVERIFY(controller.breadcrumbs().size() >= 2);
        QVERIFY(click("breadcrumbButton0"));
        QCOMPARE(controller.breadcrumbs().size(), 1);
        QVERIFY(click("startTaxonButton"));
        QCOMPARE(window->property("page").toString(), QString("quiz"));
        controller.stopGame();
        showPage("results");
        QVERIFY(click("returnExplorationButton"));
        QCOMPARE(window->property("page").toString(), QString("taxonomy"));
        QVERIFY(click("returnKingdomsButton"));
    }
    QVERIFY(click("categoryButton0"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QVERIFY(snapshot("03-quiz"));
    const int correctIndex = controller.answerOptions().indexOf(correctAnswer());
    QVERIFY(correctIndex >= 0);
    QVERIFY(click("answerOption" + QString::number(correctIndex)));
    QCOMPARE(controller.answeredCount(), 0);
    QVERIFY(click("confirmAnswerButton"));
    QCOMPARE(window->property("page").toString(), QString("feedback"));
    QCOMPARE(controller.answeredCount(), 1);
    QVERIFY(controller.answerDetails().value("correct").toBool());
    QVERIFY(snapshot("04-correct"));
    showPage("info");
    QVERIFY(snapshot("06-information"));
    showPage("feedback");
    QVERIFY(click("nextQuestionButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QVERIFY(!controller.answerPending());
    const int nextCorrectIndex = controller.answerOptions().indexOf(correctAnswer());
    QVERIFY(click("answerOption" + QString::number((nextCorrectIndex + 1) % 4)));
    QVERIFY(click("confirmAnswerButton"));
    QVERIFY(!controller.answerDetails().value("correct").toBool());
    QVERIFY(snapshot("05-incorrect"));
    controller.stopGame();
    showPage("results");
    QVERIFY(snapshot("07-results"));
    for (const QString &page : {QString("statistics"), QString("achievements"), QString("settings")}) {
        showPage(page);
        QVERIFY(snapshot(page));
    }
    window->resize(1000, 700);
    controller.startGame();
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
        for (int i = 1; i <= 5; ++i) QVERIFY(history.saveResult("Perfil " + QString::number(i), i, i));
    }
    controller.loadRanking();
    showPage("statistics");
    QVERIFY(snapshot("compact-full-statistics"));
    controller.startTaxonGame("edu:Plantae", "species");
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
    QVERIFY(click("startTaxonButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QCOMPARE(controller.identificationRankLabel(), taxonomicRankLabel("family"));
    QVERIFY(snapshot("compact-family-quiz"));
    controller.answerSelected(controller.answerOptions().first());
    showPage("info");
    QVERIFY(snapshot("compact-family-information"));
    qInstallMessageHandler(runtimeHandler);
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
}

QTEST_MAIN(QmlInterfaceTests)
#include "QmlInterfaceTests.moc"
