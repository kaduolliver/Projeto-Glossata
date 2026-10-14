#include "presentation/GameController.h"
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
        const QPoint point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
        QTest::qWait(40);
        return true;
    };
    auto showPage = [window](const QString &page) { window->setProperty("page", page); QTest::qWait(30); };
    auto correctAnswer = [&controller]() {
        JsonSpeciesCatalog catalog;
        for (const Species &species : catalog.loadSpecies()) {
            if (species.fact == controller.factText()) return species.name;
        }
        return QString();
    };

    const auto runtimeHandler = qInstallMessageHandler(captureMessage);
    QTest::qWait(220);
    QVERIFY(snapshot("01-home"));
    QVERIFY(click("homePlayButton"));
    QCOMPARE(window->property("page").toString(), QString("categories"));
    QVERIFY(snapshot("02-categories"));
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
    for (const QString &page : {QString("home"), QString("categories"), QString("feedback"), QString("info"), QString("results"), QString("statistics"), QString("achievements"), QString("settings")}) {
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
    qInstallMessageHandler(runtimeHandler);
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
}

QTEST_MAIN(QmlInterfaceTests)
#include "QmlInterfaceTests.moc"
