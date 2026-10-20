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
#include <QQmlProperty>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <cmath>

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
    void designSystemStatesAndContrast();
    void homeCarouselPhotographsAndNavigation();
    void homeCarouselTimingAndCrossfade();
    void separateExploreAndPlayPreservesSessions();
    void photographEdgesFadeWithoutHidingTheOrganism();
    void explorationCardsKeepBackgroundProportions();
    void navigateAndConfirmAnswers();
    void recoveryControlsResumeAndAbandonWithoutChangingScores();
};

void QmlInterfaceTests::homeCarouselPhotographsAndNavigation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("carousel.sqlite"));
    const auto slides = controller.homeSlides();
    QCOMPARE(slides.size(), 4);
    QSet<QString> images;
    for (const auto &value : slides) {
        const auto slide = value.toMap(), photo = slide["photo"].toMap();
        QVERIFY(!photo["author"].toString().isEmpty());
        QVERIFY(photo["licenseUrl"].toString().startsWith("https://creativecommons.org/"));
        QCOMPARE(photo["evidenceSha256"].toString().size(), 64);
        QVERIFY(photo["width"].toInt() >= 1920);
        QVERIFY(photo["height"].toInt() >= 1080);
        QVERIFY(!images.contains(photo["image"].toString()));
        images.insert(photo["image"].toString());
        QVERIFY(slide["introduction"].toMap()["text"].toString().startsWith(slide["excerpt"].toString()));
        QVERIFY(!slide["excerpt"].toString().isEmpty());
    }
    DiscordRichPresence presence{QString(), QString()};
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY(!engine.rootObjects().isEmpty());
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    QVERIFY(window);
    auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    auto click = [&](const QString &name) {
        auto button = item(name);
        if (!button || !button->isVisible() || !button->isEnabled()) return false;
        button->forceActiveFocus(Qt::TabFocusReason);
        QTest::keyClick(window, Qt::Key_Space);
        QTest::qWait(50);
        return true;
    };
    QTRY_VERIFY(item("homeCarousel"));
    auto home = item("homeCarousel");
    QTRY_VERIFY(home->property("photoReady").toBool());
    QVERIFY(home->property("transitionDuration").toInt() > 0);
    QVERIFY(click("homeNextSlide"));
    QTRY_COMPARE(home->property("currentIndex").toInt(), 1);
    QTRY_VERIFY(home->property("photoReady").toBool());
    QTRY_COMPARE(home->property("pendingIndex").toInt(), -1);
    window->setProperty("motionEnabled", false);
    QCOMPARE(home->property("transitionDuration").toInt(), 0);
    QVERIFY(click("homePreviousSlide"));
    QTRY_COMPARE(home->property("currentIndex").toInt(), 0);
    QVERIFY(click("homePreviousSlide"));
    QTRY_COMPARE(home->property("currentIndex").toInt(), 3);
    QTest::keyClick(window, Qt::Key_Right);
    QTRY_COMPARE(home->property("currentIndex").toInt(), 0);
    QTest::keyClick(window, Qt::Key_End);
    QTRY_COMPARE(home->property("currentIndex").toInt(), 3);
    QTest::keyClick(window, Qt::Key_Home);
    QTRY_COMPARE(home->property("currentIndex").toInt(), 0);
    QVERIFY(QDir().mkpath("previews/carousel"));
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size);
        QVERIFY(!item("homeSlidePosition"));
        QVERIFY(!item("homeSlideDot4"));
        for (int i = 0; i < slides.size(); ++i) {
            QVERIFY(item("homeSlideDot" + QString::number(i))->isVisible());
            QVERIFY(click("homeSlideDot" + QString::number(i)));
            QTRY_COMPARE(home->property("currentIndex").toInt(), i);
            QTRY_VERIFY(home->property("photoReady").toBool());
            // Indicator focus may scroll the document. Capture its top explicitly.
            item("pageScroll")->setProperty("contentY", 0);
            QTest::qWait(250);
            const auto credit = item("homeCreditsButton"), caption = item("homePhotoCaption");
            QVERIFY(credit && caption);
            QVERIFY(qAbs(credit->width() - credit->implicitWidth()) < 1);
            QVERIFY(credit->width() < caption->width() / 2);
            QVERIFY(qAbs(credit->x() + credit->width() - caption->width()) < 1);
            const auto hero = item("homeHeroPhoto");
            const auto frame=window->grabWindow();
            const auto bounds=hero->mapRectToScene(hero->boundingRect());
            QSet<QRgb> colors;
            for(int x=6;x<10;++x)for(int y=1;y<5;++y) {
                const int px=qRound(bounds.left()+bounds.width()*x/10);
                const int py=qRound(bounds.top()+bounds.height()*y/10);
                colors.insert(frame.pixel(px,py));
            }
            QVERIFY2(colors.size()>10,"The actual photograph must be rendered, not an empty canvas.");
            QVERIFY(frame.save(QString("previews/carousel/slide-%1-%2x%3.png")
                .arg(i).arg(size.width()).arg(size.height())));
            QVERIFY(hero->width() > home->width() * 0.7);
            QVERIFY(hero->height() >= 400);
            for (const auto &name : {"homeExploreButton","homePlayButton","homeCreditsButton"}) {
                auto control = item(name);
                QVERIFY(control->isVisible());
                const auto bounds = control->mapRectToScene(control->boundingRect());
                QVERIFY(bounds.left() >= 0 && bounds.right() <= size.width());
            }
        }
    }
    window->resize(1240,820);
    QVERIFY(click("homeCreditsButton"));
    auto dialog = engine.rootObjects().first()->findChild<QObject *>("homeCreditsDialog");
    QVERIFY(dialog);
    QVERIFY(dialog->property("visible").toBool());
    QTest::qWait(100);
    QVERIFY(window->grabWindow().save("previews/carousel/credits-1240x820.png"));
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!dialog->property("visible").toBool());
    QVERIFY(item("homeCreditsButton")->hasActiveFocus());
    // Missing presentation media never blocks the navigation routes.
    home->setProperty("slides", QVariantList{});
    QTRY_VERIFY(!item("homeNextSlide")->isEnabled());
    QTRY_VERIFY(!item("homePreviousSlide")->isEnabled());
    QVERIFY(!item("homeCreditsButton")->isVisible());
    QTRY_COMPARE(home->property("currentIndex").toInt(), 0);
    QTRY_VERIFY(!item("homeNextSlide")->isEnabled());
    QVERIFY(click("homeExploreButton"));
    QCOMPARE(window->property("page").toString(), QString("explore"));
    QVERIFY(click("navHome"));
    QVERIFY(click("homePlayButton"));
    QCOMPARE(window->property("page").toString(), QString("categories"));
    QVERIFY(click("categoryButton0"));
    const auto options = controller.answerOptions();
    const auto photo = controller.questionPhoto();
    QVERIFY(click("navHome"));
    QVERIFY(click("homeExploreButton"));
    QVERIFY(click("navHome"));
    QVERIFY(click("homePlayButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    QCOMPARE(controller.answerOptions(), options);
    QCOMPARE(controller.questionPhoto(), photo);
}

void QmlInterfaceTests::homeCarouselTimingAndCrossfade()
{
    QTemporaryDir directory;
    GameController controller(nullptr, directory.filePath("carousel-timing.sqlite"));
    DiscordRichPresence presence{QString(), QString()};
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY(!engine.rootObjects().isEmpty());
    auto window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());QVERIFY(window);
    auto item=[window](const QString &name) { return findVisualItem(window->contentItem(),name); };
    auto home=item("homeCarousel");QVERIFY(home);
    auto timer=window->findChild<QObject *>("homeAdvanceTimer");QVERIFY(timer);
    auto dialog=window->findChild<QObject *>("homeCreditsDialog");QVERIFY(dialog);
    window->requestActivate();
    QTest::mouseMove(window,QPoint(10,10));
    item("navHome")->forceActiveFocus(Qt::TabFocusReason);
    QCOMPARE(home->property("slideInterval").toInt(),8000);
    QCOMPARE(home->property("transitionDuration").toInt(),600);
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QTRY_VERIFY(home->property("photoReady").toBool());
    QTest::qWait(7000);
    QCOMPARE(home->property("currentIndex").toInt(),0);
    QTRY_COMPARE_WITH_TIMEOUT(home->property("currentIndex").toInt(),1,2500);
    QTRY_VERIFY(home->property("transitioning").toBool());
    QTest::qWait(180);
    const auto blend=home->property("blend").toReal();
    QVERIFY(blend>0 && blend<1);
    auto first=item("homeHeroPhoto"),second=item("homeIncomingPhoto");
    QVERIFY(first->property("painted").toBool() && second->property("painted").toBool());
    QVERIFY(first->opacity()>0 && second->opacity()>0);
    QVERIFY(std::abs(first->opacity()+second->opacity()-1)<0.01);
    QTRY_COMPARE(home->property("pendingIndex").toInt(),-1);
    // Accelerate subsequent intervals only after checking the real eight seconds.
    timer->setProperty("interval",400);
    QTest::mouseMove(window,home->mapToScene(QPointF(100,100)).toPoint());
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    const int hoveredIndex=home->property("currentIndex").toInt();
    QTest::qWait(450);
    QCOMPARE(home->property("currentIndex").toInt(),hoveredIndex);
    QTest::mouseMove(window,QPoint(10,10));
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QTest::qWait(200);
    // Manual navigation must start a fresh interval rather than inherit 200 ms.
    QVERIFY(QMetaObject::invokeMethod(home,"selectSlide",Q_ARG(QVariant,2),Q_ARG(QVariant,true)));
    QTRY_COMPARE(home->property("currentIndex").toInt(),2);
    QTest::qWait(230);
    QCOMPARE(home->property("currentIndex").toInt(),2);
    // Pause for credits independently of keyboard focus.
    QVERIFY(QMetaObject::invokeMethod(dialog,"open"));
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    QTest::qWait(450);
    QCOMPARE(home->property("currentIndex").toInt(),2);
    QVERIFY(QMetaObject::invokeMethod(dialog,"close"));
    item("homeNextSlide")->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    item("navHome")->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QQuickWindow anotherWindow;
    anotherWindow.show();anotherWindow.requestActivate();
    QTRY_VERIFY(!home->property("windowActive").toBool());
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    anotherWindow.hide();window->requestActivate();
    QTRY_VERIFY(home->property("windowActive").toBool());
    window->hide();
    QTRY_VERIFY(!home->property("windowActive").toBool());
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    QTest::qWait(450);
    QCOMPARE(home->property("currentIndex").toInt(),2);
    window->show();window->requestActivate();
    QTRY_VERIFY(home->property("windowActive").toBool());
    window->setProperty("motionEnabled",false);
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    QCOMPARE(home->property("transitionDuration").toInt(),0);
    QTRY_COMPARE(home->property("pendingIndex").toInt(),-1);
    QVERIFY(QMetaObject::invokeMethod(home,"selectSlide",Q_ARG(QVariant,3),Q_ARG(QVariant,true)));
    QTRY_COMPARE(home->property("currentIndex").toInt(),3);
    QVERIFY(QMetaObject::invokeMethod(home,"selectSlide",Q_ARG(QVariant,4),Q_ARG(QVariant,true)));
    QTRY_COMPARE(home->property("currentIndex").toInt(),0);
    home->setVisible(false);
    window->setProperty("motionEnabled",true);
    QVERIFY(!home->property("autoRunning").toBool());
    home->setVisible(true);
    item("navHome")->forceActiveFocus(Qt::TabFocusReason);
    QTRY_VERIFY(home->property("autoRunning").toBool());
    window->setProperty("page","explore");
    QTRY_VERIFY(!item("homeCarousel"));
    QCOMPARE(controller.answeredCount(),0);
}

void QmlInterfaceTests::separateExploreAndPlayPreservesSessions()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("navigation.sqlite"));
    DiscordRichPresence presence{QString(), QString()};
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY(!engine.rootObjects().isEmpty());
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    QVERIFY(window);
    window->setProperty("motionEnabled", false);
    auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    auto click = [&](const QString &name) {
        auto button = item(name);
        if (!button || !button->isVisible() || !button->isEnabled()) return false;
        button->forceActiveFocus(Qt::TabFocusReason);
        // Keyboard activation also covers controls below the viewport.
        QTest::keyClick(window, Qt::Key_Space);
        QTest::qWait(50);
        return true;
    };
    auto selected = [&](const QString &name) { return item(name)->property("selected").toBool(); };
    QVERIFY(QDir().mkpath("previews/navigation"));
    auto capture = [&](const QString &name) {
        QTest::qWait(120);
        return window->grabWindow().save("previews/navigation/" + name + ".png");
    };
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size);
        const QString suffix = QString::number(size.width()) + "x" + QString::number(size.height());
        QVERIFY(click("navHome")); QVERIFY(selected("navHome"));
        QVERIFY(capture("home-" + suffix));
        // All six destinations remain inside the window and keyboard-reachable.
        qreal previousBottom = 0;
        for (const QString &name : {"navHome", "navExplore", "navPlay", "navStatistics", "navAchievements", "navSettings"}) {
            auto nav = item(name); QVERIFY(nav && nav->isVisible());
            const auto rect = nav->mapRectToScene(nav->boundingRect());
            QVERIFY(rect.top() >= previousBottom);
            QVERIFY(rect.bottom() <= size.height());
            QVERIFY(rect.right() < item("sectionHeader")->mapToScene(QPointF()).x());
            previousBottom = rect.bottom();
        }
        item("navHome")->forceActiveFocus(Qt::TabFocusReason);
        QTest::keyClick(window, Qt::Key_Tab); QVERIFY(item("navExplore")->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_Space); QTest::qWait(50);
        QCOMPARE(window->property("page").toString(), QString("explore"));
        QVERIFY(selected("navExplore")); QVERIFY(!selected("navPlay"));
        QVERIFY(!item("playModeSelector")->isVisible());
        QVERIFY(!item("categoryButton0")->isVisible());
        QVERIFY(capture("explore-" + suffix));
        for (int i = 0; i < 3; ++i) {
            QVERIFY(click("domainButton" + QString::number(i)));
            QCOMPARE(window->property("page").toString(), QString("taxonomy"));
            QVERIFY(selected("navExplore"));
            QVERIFY(item("breadcrumbButton0")->isVisible());
            QVERIFY(!item("startTaxonButton")->isVisible());
            QVERIFY(!item("playModeSelector")->isVisible());
            QVERIFY(click("returnDomainsButton"));
            QCOMPARE(window->property("page").toString(), QString("explore"));
        }
        QVERIFY(click("quickAccessButton0"));
        QVERIFY(controller.breadcrumbs().size() > 1);
        QVERIFY(capture("lineage-" + suffix));
        const auto ancestor = controller.breadcrumbs().first().toMap().value("id");
        QVERIFY(click("breadcrumbButton0"));
        QCOMPARE(controller.browseTaxon().value("id"), ancestor);
        QVERIFY(selected("navExplore"));
        QVERIFY(click("navPlay")); QVERIFY(selected("navPlay"));
        QCOMPARE(window->property("page").toString(), QString("categories"));
        QVERIFY(item("playModeSelector")->isVisible());
        QVERIFY(item("categoryButton0")->isVisible());
        QVERIFY(capture("play-" + suffix));
        item("domainButton2")->forceActiveFocus(Qt::TabFocusReason);
        QTest::qWait(50);
        const auto focusedRect = item("domainButton2")->mapRectToItem(item("pageScroll"), item("domainButton2")->boundingRect());
        QVERIFY(focusedRect.top() >= -1);
        QVERIFY(focusedRect.bottom() <= item("pageScroll")->height() + 1);
        QVERIFY(click("quickAccessButton0"));
        QVERIFY(selected("navPlay")); QVERIFY(item("startTaxonButton")->isVisible());
        QVERIFY(click("returnDomainsButton"));
        QCOMPARE(window->property("page").toString(), QString("categories"));
    }
    QCOMPARE(controller.answeredCount(), 0);
    QVERIFY(controller.recoverableSessions().isEmpty()); // Browsing never starts a session.
    QVERIFY(click("categoryButton0"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    const auto photo = controller.questionPhoto();
    const auto alternatives = controller.answerOptions();
    const auto sessionGroup = controller.sessionTaxonId();
    const auto availableHints = controller.hints();
    for (const auto &hint : availableHints) {
        if (hint.toMap().value("available").toBool()) {
            QVERIFY(controller.revealHintCategory(hint.toMap().value("id").toString()));
            break;
        }
    }
    const auto hints = controller.hints();
    const auto ceiling = controller.possibleScoreText();
    QVERIFY(click("navExplore"));
    QVERIFY(click("quickAccessButton1"));
    QCOMPARE(controller.questionPhoto(), photo);
    QCOMPARE(controller.answerOptions(), alternatives);
    QCOMPARE(controller.sessionTaxonId(), sessionGroup);
    QCOMPARE(controller.hints(), hints);
    QCOMPARE(controller.possibleScoreText(), ceiling);
    QVERIFY(item("headerResumeGame")->isVisible());
    QVERIFY(click("headerResumeGame"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    controller.answerSelected(alternatives.first());
    QVERIFY(controller.answerPending());
    window->setProperty("page", "info"); QTest::qWait(50);
    const auto score = controller.scoreUnits();
    const auto answered = controller.answeredCount();
    QVERIFY(click("navExplore"));
    QVERIFY(click("navPlay"));
    QCOMPARE(window->property("page").toString(), QString("info"));
    QCOMPARE(controller.answeredCount(), answered);
    QCOMPARE(controller.scoreUnits(), score);
    QVERIFY(controller.answerPending());
    QVERIFY(click("speciesContinueButton"));
    QCOMPARE(window->property("page").toString(), QString("quiz"));
    controller.stopGame();
    window->setProperty("page", "results"); QTest::qWait(50);
    QVERIFY(click("navExplore")); QVERIFY(click("navPlay"));
    QCOMPARE(window->property("page").toString(), QString("results"));
    QVERIFY(click("returnExplorationButton"));
    QCOMPARE(window->property("page").toString(), QString("categories"));
    // A real failed write in the isolated SQLite database keeps the global
    // pending-result route intact, even after visiting another section.
    QVERIFY(click("categoryButton0"));
    controller.answerSelected(controller.answerOptions().first());
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", "navigation-failure");
        db.setDatabaseName(directory.filePath("navigation.sqlite"));
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("CREATE TRIGGER reject_navigation_insert BEFORE INSERT ON history BEGIN SELECT RAISE(FAIL,'synthetic navigation failure'); END"));
        QVERIFY(query.exec("CREATE TRIGGER reject_navigation_update BEFORE UPDATE ON history BEGIN SELECT RAISE(FAIL,'synthetic navigation failure'); END"));
        controller.stopGame();
        QVERIFY(controller.resultPending());
        const auto pendingScore = controller.scoreUnits();
        for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
            window->resize(size);
            QVERIFY(click("navExplore"));
            QVERIFY(controller.resultPending());
            QVERIFY(capture("pending-" + QString::number(size.width()) + "x" + QString::number(size.height())));
            QVERIFY(click("navPlay"));
            QCOMPARE(window->property("page").toString(), QString("results"));
            QCOMPARE(controller.scoreUnits(), pendingScore);
            QVERIFY(!item("returnExplorationButton")->isEnabled());
        }
        QVERIFY(query.exec("DROP TRIGGER reject_navigation_insert"));
        QVERIFY(query.exec("DROP TRIGGER reject_navigation_update"));
        controller.retrySaveResult();
        QVERIFY(!controller.resultPending());
        db.close();
    }
    QSqlDatabase::removeDatabase("navigation-failure");
    QVERIFY(click("navSettings")); QVERIFY(click("creditsButton"));
    QCOMPARE(window->property("page").toString(), QString("credits"));
    QVERIFY(selected("navSettings"));
    QCOMPARE(item("sectionTitle")->property("text").toString(), QString::fromUtf8("Créditos e transparência"));
}

void QmlInterfaceTests::designSystemStatesAndContrast()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQuick
        import QtQuick.Controls
        import "qrc:/resources/qml/components"
        import "qrc:/resources/qml/style" as Style
        ApplicationWindow {
            visible: true; width: 520; height: 430; color: Style.Theme.canvas
            property bool animations: true
            onAnimationsChanged: Style.Theme.motionEnabled = animations
            property int animationDuration: Style.Theme.durationFast
            property color foreground: Style.Theme.text
            property color focusColor: Style.Theme.focus
            property color canvasColor: Style.Theme.canvas
            property int activations: 0
            Column {
                x: 30; y: 30; spacing: 20
                UiButton { objectName: "themeButton"; text: "Explorar"; onClicked: activations++ }
                NavItem { objectName: "themeNav"; text: "Inicio"; selected: true }
                TextAction { objectName: "themeAction"; text: "Fontes" }
                StatCard { title: "Catalogo"; value: "122" }
            }
        }
    )", QUrl("qrc:/design-system-test.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object, qPrintable(component.errorString()));
    auto window = qobject_cast<QQuickWindow *>(object.data()); QVERIFY(window);
    auto button = findVisualItem(window->contentItem(), "themeButton"); QVERIFY(button);
    auto background = button->property("background").value<QQuickItem *>(); QVERIFY(background);
    const auto luminance = [](QColor c) {
        const auto linear = [](double v) { return v <= 0.04045 ? v/12.92 : std::pow((v+0.055)/1.055, 2.4); };
        return 0.2126*linear(c.redF()) + 0.7152*linear(c.greenF()) + 0.0722*linear(c.blueF());
    };
    const auto contrast = [&](QColor a, QColor b) { const auto x=luminance(a), y=luminance(b); return (qMax(x,y)+0.05)/(qMin(x,y)+0.05); };
    window->setProperty("animations", false);
    QCOMPARE(window->property("animationDuration").toInt(), 0);
    QTest::qWait(180);
    const QColor text = window->property("foreground").value<QColor>();
    for (bool secondary : {false, true}) for (bool danger : {false, true}) {
        button->setProperty("secondary", secondary); button->setProperty("danger", danger);
        for (int state = 0; state < 3; ++state) {
            QTest::mouseMove(window, state ? button->mapToScene(QPointF(20,20)).toPoint() : QPoint(510,420));
            button->setProperty("down", state == 2);
            QTest::qWait(1);
            QVERIFY(contrast(text,background->property("color").value<QColor>()) >= 4.5);
            button->setProperty("down", false);
        }
    }
    button->setProperty("secondary", false); button->setProperty("danger", false);
    button->forceActiveFocus(Qt::TabFocusReason);
    QCOMPARE(QQmlProperty(background, "border.width").read().toInt(), 2);
    QVERIFY(contrast(window->property("focusColor").value<QColor>(), window->property("canvasColor").value<QColor>()) >= 3.0);
    QTest::keyClick(window, Qt::Key_Space); QCOMPARE(window->property("activations").toInt(), 1);
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(findVisualItem(window->contentItem(), "themeNav")->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(findVisualItem(window->contentItem(), "themeAction")->hasActiveFocus());
    button->setEnabled(false);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, button->mapToScene(QPointF(20,20)).toPoint());
    QCOMPARE(window->property("activations").toInt(), 1);
    window->setProperty("animations", true);
    QVERIFY(window->property("animationDuration").toInt() > 0);
}

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
    auto reviewStatus=findVisualItem(window->contentItem(),"speciesReviewStatus");
    QVERIFY(reviewStatus && reviewStatus->isVisible());
    QVERIFY(reviewStatus->property("text").toString().contains(QStringLiteral("Responsável declarado")));
    QVERIFY(reviewStatus->property("text").toString().contains(QStringLiteral("Autoria não autenticada")));
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
    controller.stopGame();
    // Earlier independent UI scenarios deliberately opened other games. Their
    // persisted sessions now require an explicit decision instead of vanishing.
    for (const auto &pending:controller.recoverableSessions()) QVERIFY(controller.abandonSession(pending.toMap()["id"].toString()));
    showPage("categories");QVERIFY(click("startCompetitiveButton"));
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

void QmlInterfaceTests::recoveryControlsResumeAndAbandonWithoutChangingScores()
{
    QTemporaryDir dir; const auto path=dir.filePath("recover-ui.sqlite"); QString id; QStringList options; qint64 score;
    {
        GameController first(nullptr,path); QVERIFY(first.startCompetitiveGame());
        const auto hints=first.hints();
        auto hint=std::find_if(hints.begin(),hints.end(),[](const QVariant &v){return v.toMap()["available"].toBool();});
        QVERIFY(hint!=hints.end()); QVERIFY(first.revealHintCategory(hint->toMap()["id"].toString()));
        first.answerSelected(first.answerOptions().first()); first.nextQuestion();
        QCOMPARE(first.answeredCount(),1); score=first.scoreUnits(); options=first.answerOptions();
        QVERIFY(first.prepareToClose()); // A normal close suspends, it does not finish/rank the game.
        SqliteGameHistoryRepository inspect(path); QCOMPARE(inspect.recoverableSessions().size(),1); id=inspect.recoverableSessions().first().id;
        QVERIFY(inspect.loadRanking(GameService::competitiveConfiguration().rankingKey()).isEmpty());
    }
    GameController controller(nullptr,path); QCOMPARE(controller.recoverableSessions().size(),1);
    QVERIFY(!controller.startCompetitiveGame()); // A new preview cannot silently replace the saved game.
    GameController second(nullptr,path);
    DiscordRichPresence presence{QString(),QString()}; QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController",&controller); engine.rootContext()->setContextProperty("discordPresence",&presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml")); QVERIFY(!engine.rootObjects().isEmpty());
    auto window=qobject_cast<QQuickWindow *>(engine.rootObjects().first()); QVERIFY(window);
    window->setProperty("motionEnabled", false);
    // Exploration does not consume, replace or hide a recoverable session.
    QVERIFY(QDir().mkpath("previews/navigation"));
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size);
        auto explore=findVisualItem(window->contentItem(),"navExplore"); QVERIFY(explore);
        explore->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window,Qt::Key_Space); QTest::qWait(80);
        QCOMPARE(window->property("page").toString(),QString("explore"));
        QCOMPARE(controller.recoverableSessions().size(),1);
        QVERIFY(findVisualItem(window->contentItem(),"sessionRecoveryBanner")->isVisible());
        QVERIFY(window->grabWindow().save("previews/navigation/recovery-"+QString::number(size.width())+"x"+QString::number(size.height())+".png"));
    }
    window->resize(1240,820); QTest::qWait(50);
    auto banner=findVisualItem(window->contentItem(),"sessionRecoveryBanner"); QVERIFY(banner); QVERIFY(banner->isVisible());
    auto resume=findVisualItem(window->contentItem(),"resumeSessionButton"); QVERIFY(resume); QVERIFY(resume->isEnabled());
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,resume->mapToScene(QPointF(resume->width()/2,resume->height()/2)).toPoint());
    QTRY_COMPARE(window->property("page").toString(),QString("quiz"));
    QCOMPARE(controller.answeredCount(),1); QCOMPARE(controller.scoreUnits(),score); QCOMPARE(controller.answerOptions(),options);
    QVERIFY(controller.recoverableSessions().isEmpty()); QVERIFY(!controller.answerPending());
    QVERIFY(QDir().mkpath("previews")); QVERIFY(window->grabWindow().save("previews/recovered-quiz.png"));
    // A second window now offers the same durable session; abandonment is explicit.
    engine.rootContext()->setContextProperty("gameController",&second); window->setProperty("backend",QVariant::fromValue(&second));
    window->setProperty("page","home"); QTest::qWait(80);
    QVERIFY(window->grabWindow().save("previews/session-recovery.png"));
    auto abandon=findVisualItem(window->contentItem(),"abandonSessionButton"); QVERIFY(abandon); QVERIFY(abandon->isVisible());
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,abandon->mapToScene(QPointF(abandon->width()/2,abandon->height()/2)).toPoint());
    auto dialog=window->findChild<QObject *>("abandonRecoveryDialog"); QVERIFY(dialog); QTRY_VERIFY(dialog->property("visible").toBool());
    QVERIFY(QMetaObject::invokeMethod(dialog,"accept")); QTRY_VERIFY(second.recoverableSessions().isEmpty());
    SqliteGameHistoryRepository inspect(path); QCOMPARE(inspect.checkpoint(id).state,QString("abandoned")); QVERIFY(inspect.loadTopResults().isEmpty());
    QVERIFY(window->close());
}

QTEST_MAIN(QmlInterfaceTests)
#include "QmlInterfaceTests.moc"
