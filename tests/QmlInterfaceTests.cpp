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
#include <QScopeGuard>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QFile>
#include <QDesktopServices>
#include <cmath>

// Intercept actual link activations; never launch a browser during UI tests.
class TestUrlRecorder : public QObject {
    Q_OBJECT
public:
    QList<QUrl> urls;
public slots:
    void capture(const QUrl &url) { urls.append(url); }
};

namespace {
QStringList interfaceWarnings;
double colorContrast(QColor a, QColor b)
{
    const auto luminance = [](QColor c) {
        const auto linear = [](double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
        return 0.2126 * linear(c.redF()) + 0.7152 * linear(c.greenF()) + 0.0722 * linear(c.blueF());
    };
    const auto x = luminance(a), y = luminance(b);
    return (qMax(x, y) + 0.05) / (qMin(x, y) + 0.05);
}
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
    void encyclopedicCreditsHaveIndependentPointerAndKeyboardInteraction();
    void photographicExploreNavigationAndCaptures();
    void progressiveExploreNavigationAndCaptures();
    void encyclopedicPhotographsStayCenteredWithoutCropping();
    void encyclopedicCardsHandleLongTextAndUnavailablePhotos();
    void scientificExploreSearchFiltersNavigationAndCaptures();
    void dailyDiscoveryNavigationIndicatorsAndCaptures();
    void designSystemStatesAndContrast();
    void homeCarouselPhotographsAndNavigation();
    void homeCarouselTimingAndCrossfade();
    void separateExploreAndPlayPreservesSessions();
    void photographEdgesFadeWithoutHidingTheOrganism();
    void explorationCardsKeepBackgroundProportions();
    void navigateAndConfirmAnswers();
    void recoveryControlsResumeAndAbandonWithoutChangingScores();
};

void QmlInterfaceTests::encyclopedicCreditsHaveIndependentPointerAndKeyboardInteraction()
{
    interfaceWarnings.clear();
    const auto oldHandler = qInstallMessageHandler(captureMessage);
    const auto restore = qScopeGuard([oldHandler] { qInstallMessageHandler(oldHandler); });
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQuick
        import QtQuick.Controls
        import "qrc:/resources/qml/components"
        import "qrc:/resources/qml/style" as Style
        ApplicationWindow {
            required property var testEntry
            property bool animations: true
            onAnimationsChanged: Style.Theme.motionEnabled = animations
            visible: true; color: Style.Theme.canvas
            ExploreCard {
                objectName: "interactionCard"; x: 40; y: 40
                entry: testEntry; encyclopedic: true
                width: Math.min(410, parent.width * 0.3); height: implicitHeight
            }
        }
    )", QUrl("qrc:/credits-interaction-test.qml"));
    JsonSpeciesCatalog catalog; JsonExplorationMediaCatalog media; TaxonomyIndex index(catalog, &media);
    QVariantList entries{index.domains().at(1), index.featuredOrganisms().first()};
    QSet<QString> licenses;
    for (const auto &value : index.search({})["groups"].toList()) {
        const auto photo = value.toMap()["photo"].toMap(); const auto license = photo["license"].toString();
        if (!photo["image"].toString().isEmpty() && !licenses.contains(license)) { entries.append(value); licenses.insert(license); }
    }
    QVERIFY(licenses.contains("CC-BY 2.0") && licenses.contains("CC-BY 3.0") && licenses.contains("CC-BY 4.0") && licenses.contains("CC0 1.0"));
    // Only an in-memory presentation fixture; never a shipped attribution or permission.
    auto special = entries.first().toMap(), specialPhoto = special["photo"].toMap();
    specialPhoto["contextualAttribution"] = false;
    specialPhoto["attributionNotice"] = "Synthetic photo-specific attribution request for interaction testing.";
    special["photo"] = specialPhoto; entries.append(special);
    QScopedPointer<QObject> object(component.createWithInitialProperties({{"testEntry", entries.first()}}));
    QVERIFY2(object, qPrintable(component.errorString()));
    auto window = qobject_cast<QQuickWindow *>(object.data()); QVERIFY(window); window->requestActivate();
    auto card = findVisualItem(window->contentItem(), "interactionCard"); QVERIFY(card);
    auto button = findVisualItem(card, "exploreCardCreditsButton"); QVERIFY(button);
    auto background = findVisualItem(button, "textActionBackground"); QVERIFY(background);
    auto surface = findVisualItem(card, "encyclopediaCardSurface"); QVERIFY(surface);
    auto row = findVisualItem(card, "exploreCardCreditsRow"); QVERIFY(row);
    QSignalSpy buttonHover(button, SIGNAL(hoveredChanged()));
    QSignalSpy colors(background, SIGNAL(colorChanged()));
    QSignalSpy surfaceColors(surface, SIGNAL(colorChanged()));
    QSignalSpy navigations(card, SIGNAL(clicked()));
    QSignalSpy creditClicks(button, SIGNAL(clicked()));
    const auto item = [&](const QString &name) { return findVisualItem(window->contentItem(), name); };
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size);
        for (const bool animations : {true, false}) {
            window->setProperty("animations", animations);
            for (const auto &value : entries) {
                window->setProperty("testEntry", value); QTest::qWait(40);
                const auto entry = value.toMap(), photo = entry["photo"].toMap();
                QVERIFY(button->width() <= button->implicitWidth() + 1);
                QVERIFY(button->width() < row->width() - 20);
                QCOMPARE(button->x(), 0.);
                const auto center = button->mapToScene(button->boundingRect().center()).toPoint();
                QTest::mouseMove(window, center); QTest::qWait(220);
                QVERIFY(button->property("hovered").toBool());
                buttonHover.clear(); colors.clear(); surfaceColors.clear();
                const auto still = window->grabWindow().copy(button->mapRectToScene(button->boundingRect()).toAlignedRect());
                for (int i = 0; i < (entry["kind"] == "organism" ? 40 : 12); ++i) {
                    QTest::mouseMove(window, center + QPoint(i % 3 - 1, 0)); QTest::qWait(30);
                }
                QCOMPARE(buttonHover.count(), 0); QCOMPARE(colors.count(), 0); QCOMPARE(surfaceColors.count(), 0);
                QCOMPARE(window->grabWindow().copy(button->mapRectToScene(button->boundingRect()).toAlignedRect()), still);
                // Crossing each edge settles once; neither hover nor geometry oscillates.
                for (const QPointF edge : {QPointF(-2, button->height()/2), QPointF(button->width()+2, button->height()/2),
                                          QPointF(button->width()/2, -2), QPointF(button->width()/2, button->height()+2)}) {
                    QTest::mouseMove(window, button->mapToScene(edge).toPoint()); QTest::qWait(140);
                    QVERIFY(!button->property("hovered").toBool());
                    QTest::mouseMove(window, center); QTest::qWait(140);
                    QVERIFY(button->property("hovered").toBool());
                }
                navigations.clear(); creditClicks.clear();
                // Empty row and the pixel just beyond the action are inert.
                for (const qreal x : {button->width() + 2, row->width() - 2}) {
                    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, row->mapToScene({x, row->height()/2}).toPoint());
                    QTest::qWait(20); QCOMPARE(navigations.count(), 0); QCOMPARE(creditClicks.count(), 0);
                    QVERIFY(!item("exploreCardCreditsClose"));
                }
                auto inlineCredit = findVisualItem(card, "encyclopediaCardAttribution");
                QCOMPARE(inlineCredit->isVisible(), !photo["contextualAttribution"].toBool());
                if (!photo["attributionNotice"].toString().isEmpty())
                    QVERIFY(inlineCredit->property("text").toString().contains(photo["attributionNotice"].toString()));
                QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, center);
                QTRY_VERIFY(item("exploreCardCreditsClose") && item("exploreCardCreditsClose")->isVisible());
                QCOMPARE(creditClicks.count(), 1); QCOMPARE(navigations.count(), 0);
                QVERIFY(item("exploreCardCreditAuthor")->property("text").toString().contains(photo["author"].toString()));
                QVERIFY(item("exploreCardCreditLicense")->property("text").toString().contains(photo["license"].toString()));
                QCOMPARE(item("exploreCardCreditHistory")->property("text").toString(), QStringLiteral("Modificações históricas registradas\n") + photo["modifications"].toString());
                QTest::keyClick(window, Qt::Key_Escape); QTest::qWait(40);
                QVERIFY(button->hasActiveFocus());
                card->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_Tab);
                QVERIFY(button->hasActiveFocus()); QVERIFY(button->property("visualFocus").toBool());
                QTest::keyClick(window, Qt::Key_Space);
                QTRY_VERIFY(item("exploreCardCreditsClose") && item("exploreCardCreditsClose")->isVisible());
                QCOMPARE(navigations.count(), 0); QCOMPARE(creditClicks.count(), 2);
                QTest::keyClick(window, Qt::Key_Escape); QTest::qWait(40);
                card->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_Space);
                QCOMPARE(navigations.count(), 1); QCOMPARE(creditClicks.count(), 2);
                auto frame = findVisualItem(card, "encyclopediaPhotoFrame");
                QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, frame->mapToScene(frame->boundingRect().center()).toPoint());
                QCOMPARE(navigations.count(), 2); QCOMPARE(creditClicks.count(), 2);
            }
        }
    }
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
}

void QmlInterfaceTests::photographicExploreNavigationAndCaptures()
{
    interfaceWarnings.clear();
    const auto oldHandler = qInstallMessageHandler(captureMessage);
    const auto restore = qScopeGuard([oldHandler] { qInstallMessageHandler(oldHandler); });
    TestUrlRecorder urls; QDesktopServices::setUrlHandler("https", &urls, "capture");
    const auto restoreLinks = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
    QTemporaryDir directory; QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("explore-round-2.sqlite"));
    DiscordRichPresence presence{QString(), QString()}; QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first()); QVERIFY(window);
    window->setProperty("motionEnabled", false); window->requestActivate(); QTest::qWait(100);
    const auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    const auto activate = [&](const QString &name, bool mouse = false) {
        auto control = item(name); if (!control || !control->isVisible() || !control->isEnabled()) return false;
        control->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(60);
        if (mouse) QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, control->mapToScene(control->boundingRect().center()).toPoint());
        else QTest::keyClick(window, Qt::Key_Space);
        QTest::qWait(100); return true;
    };
    const auto edit = [&](const QString &query) {
        auto field = item("exploreSearchField"); if (!field) return false;
        field->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_Backspace);
        for (auto c : query) QTest::keyClick(window, c.toLatin1());
        return QTest::qWaitFor([&] { return controller.exploreQuery() == query && !controller.exploreSearchPending(); }, 3000);
    };
    const auto photoFits = [&] {
        auto photo = item("exploreTaxonPhoto"); if (!photo || photo->property("status").toInt() != 1) return false;
        const auto pixels = photo->property("sourceSize").toSize();
        return !pixels.isEmpty() && photo->width() > 0 && photo->height() > 0
            && qAbs(photo->width() / photo->height() - double(pixels.width()) / pixels.height()) < 0.001
            && photo->x() >= 0 && photo->y() >= 0 && photo->x() + photo->width() <= photo->parentItem()->width() + 1
            && photo->y() + photo->height() <= photo->parentItem()->height() + 1
            && qAbs(photo->x() - (photo->parentItem()->width() - photo->width()) / 2) < 0.01
            && qAbs(photo->y() - (photo->parentItem()->height() - photo->height()) / 2) < 0.01
            && photo->width() <= pixels.width() && photo->height() <= pixels.height();
    };
    QVERIFY(QDir().mkpath("previews/explore-2.2.2-creditos")); QJsonArray captures;
    const auto capture = [&](const QString &state, QSize size) {
        const QSet<QString> states{"overview", "gallery", "descendants", "contextual-credits", "credit-history", "featured-credits"};
        if (!states.contains(state)) return true; // Prior rounds retain their historical screenshots.
        QTest::qWait(220);
        const auto filename = state + "-" + QString::number(size.width()) + "x" + QString::number(size.height()) + ".png";
        const auto image = window->grabWindow();
        if (image.size() != size || !image.save("previews/explore-2.2.2-creditos/" + filename)) return false;
        captures.append(QJsonObject::fromVariantMap({{"file", filename}, {"width", size.width()}, {"height", size.height()},
            {"captured_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}, {"page", window->property("page")},
            {"query", controller.exploreQuery()}, {"domainId", controller.exploreDomain()}, {"rank", controller.exploreRank()},
            {"taxonId", controller.browseTaxon()["id"]}, {"scrollY", item("pageScroll")->property("contentY")},
            {"groupsShown", item("exploreGroupResults") ? item("exploreGroupResults")->property("count") : QVariant()},
            {"organismsShown", item("exploreOrganismResults") ? item("exploreOrganismResults")->property("count") : QVariant()}}));
        return true;
    };
    for (QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size); controller.setExploreQuery(""); controller.setExploreDomain(""); controller.setExploreRank("");
        QVERIFY(activate("navExplore")); item("pageScroll")->setProperty("contentY", 0);
        auto heading = item("exploreHeading"), field = item("exploreSearchField"); QVERIFY(heading && field);
        const auto headingRect = heading->mapRectToScene(heading->boundingRect()), fieldRect = field->mapRectToScene(field->boundingRect());
        if (size.width() == 1000) QVERIFY(fieldRect.top() >= headingRect.bottom());
        else QVERIFY(fieldRect.left() >= headingRect.right());
        QVERIFY(capture("overview", size));
        const auto domain = item("domainButton0")->property("entry").toMap();
        auto summary = findVisualItem(item("domainButton0"), "encyclopediaCardSummary");
        QCOMPARE(summary->property("text").toString(), domain["educationalNote"].toMap()["text"].toString());
        QCOMPARE(summary->property("maximumLineCount").toInt(), 2);
        QVERIFY(!findVisualItem(item("domainButton0"), "encyclopediaCardAttribution")->isVisible());
        const auto previousTaxon = controller.browseTaxon()["id"];
        QVERIFY(activate("exploreCardCreditsButton", true));
        QCOMPARE(window->property("page").toString(), QString("explore")); QCOMPARE(controller.browseTaxon()["id"], previousTaxon);
        QTRY_VERIFY2(item("exploreCardCreditsClose") && item("exploreCardCreditsClose")->isVisible(), qPrintable(interfaceWarnings.join('\n')));
        const auto metadata = domain["photo"].toMap();
        QVERIFY(item("exploreCardCreditAuthor")->property("text").toString().contains(metadata["author"].toString()));
        QVERIFY(item("exploreCardCreditLicense")->property("text").toString().contains(metadata["licenseVersion"].toString()));
        QVERIFY(capture("contextual-credits", size));
        QVERIFY(activate("exploreCardOriginalLink")); QCOMPARE(urls.urls.last().toString(), metadata["photoUrl"].toString());
        QVERIFY(activate("exploreCardLicenseLink")); QCOMPARE(urls.urls.last().toString(), metadata["licenseUrl"].toString());
        if (!domain["educationalNote"].toMap().isEmpty()) {
            QVERIFY(activate("exploreCardScientificLink")); QCOMPARE(urls.urls.last().toString(), domain["educationalNote"].toMap()["source_url"].toString());
        } else {
            QVERIFY(!summary->isVisible()); QVERIFY(!item("exploreCardScientificLink")->isVisible());
            QVERIFY(activate("exploreCardPlacementLink")); QCOMPARE(urls.urls.last().toString(), domain["sourceUrl"].toString());
        }
        QCOMPARE(item("exploreCardCreditHistory")->property("text").toString(), QStringLiteral("Modificações históricas registradas\n") + metadata["modifications"].toString());
        QVERIFY(item("exploreCardCreditPresentation")->property("text").toString().contains("sem recorte"));
        item("exploreCardCreditHistory")->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(60);
        QVERIFY(capture("credit-history", size));
        QTest::keyClick(window, Qt::Key_Escape); QTest::qWait(80);
        QVERIFY(item("exploreCardCreditsButton")->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_Space); QTest::qWait(80);
        QVERIFY(item("exploreCardCreditsClose")->hasActiveFocus());
        for (int tab = 0; tab < 4; ++tab) {
            QTest::keyClick(window, Qt::Key_Tab); QTest::qWait(40);
            auto focused = window->activeFocusItem(); QVERIFY(focused);
            const auto bounds = focused->mapRectToScene(focused->boundingRect());
            QVERIFY(bounds.top() >= 0 && bounds.bottom() <= window->height() + 1);
        }
        QVERIFY(activate("exploreCardCreditsClose", true));
        QCOMPARE(window->property("page").toString(), QString("explore"));
        QVERIFY(item("exploreCardCreditsButton")->hasActiveFocus());
        auto gallery = item("exploreFeaturedGrid"); QVERIFY(gallery);
        item("featuredOrganism0")->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(120);
        for (int i = 0; i < 3; ++i) QTest::keyClick(window, Qt::Key_Right);
        QTRY_VERIFY(item("featuredOrganism3")->hasActiveFocus());
        const auto last = item("featuredOrganism3")->mapRectToItem(gallery, item("featuredOrganism3")->boundingRect());
        QVERIFY(last.left() >= -1 && last.right() <= gallery->width() + 1);
        QTest::keyClick(window, Qt::Key_Left); QVERIFY(item("featuredOrganism2")->hasActiveFocus());
        if (gallery->property("contentWidth").toReal() > gallery->width()) {
            QVERIFY(activate("previousFeaturedOrganisms", true));
            QVERIFY(activate("nextFeaturedOrganisms"));
        }
        item("featuredOrganism0")->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(120);
        auto featured = item("featuredOrganism0");
        QVERIFY(featured->height() < 460); // Educational excerpt and an independent credits action.
        const auto entry = featured->property("entry").toMap(), photo = entry["photo"].toMap();
        const auto attribution = findVisualItem(featured, "encyclopediaCardAttribution");
        QVERIFY(!attribution->isVisible());
        QVERIFY(findVisualItem(featured, "encyclopediaCardLicenseNotice")->isVisible());
        const auto path = entry["pathText"].toString();
        QVERIFY(!findVisualItem(featured, "encyclopediaCardCount")->property("text").toString().contains(path));
        auto featuredCredits = findVisualItem(featured, "exploreCardCreditsButton"); QVERIFY(featuredCredits);
        featuredCredits->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_Space); QTest::qWait(100);
        QVERIFY(item("exploreCardCreditsClose")->isVisible());
        QVERIFY(item("exploreCardCreditLicense")->property("text").toString().contains("Não comprovada"));
        QVERIFY(capture("featured-credits", size));
        QVERIFY(activate("exploreCardScientificLink")); QCOMPARE(urls.urls.last().toString(), entry["educationalNote"].toMap()["source_url"].toString());
        QTest::keyClick(window, Qt::Key_Escape); QTest::qWait(80); QVERIFY(featuredCredits->hasActiveFocus());
        QCOMPARE(window->property("page").toString(), QString("explore"));
        QVERIFY(item("exploreFeaturedRange")->property("text").toString().endsWith("/ 4"));
        QVERIFY(item("exploreFeaturedPosition")->property("text").toString().contains(size.width() == 1920 ? "Todos visíveis" : "direita"));
        QVERIFY(capture("gallery", size));
        QVERIFY(activate("featuredOrganism0", true));
        QCOMPARE(controller.browseTaxon()["id"], entry["id"]);
        QVERIFY(item("exploreTaxonKind")->property("text").toString().contains("ORGANISMO"));
        QTRY_VERIFY(photoFits()); QVERIFY(capture("organism", size));
        QVERIFY(activate("explorePhotoCredits")); QVERIFY(item("explorePhotoDetails")->isVisible());
        QVERIFY(item("explorePhotoProvenance")->property("text").toString().contains(controller.browseTaxon()["photo"].toMap()["modifications"].toString()));
        QVERIFY(activate("explorePhotoOriginal"));
        const auto fullPhoto = controller.browseTaxon()["photo"].toMap();
        QCOMPARE(urls.urls.last().toString(), fullPhoto.value("photoUrl", fullPhoto["sourceUrl"]).toString());
        QVERIFY(activate("explorePhotoLicense")); QCOMPARE(urls.urls.last().toString(), fullPhoto["licenseUrl"].toString());
        QVERIFY(capture("photo-provenance", size));
        QVERIFY(activate("exploreClassification")); QVERIFY(item("exploreClassificationDetails")->isVisible());
        QVERIFY(activate("exploreSources")); QVERIFY(item("exploreSourceDetails")->isVisible());
        QVERIFY(activate("exploreScientificSource"));
        QCOMPARE(urls.urls.last().toString(), controller.browseTaxon()["summary"].toMap()["source_url"].toString());
        QVERIFY(capture("scientific-context", size));
        QVERIFY(activate("expandTaxonomyBreadcrumbs")); QVERIFY(capture("breadcrumbs-expanded", size));
        QVERIFY(activate("breadcrumbButton0"));
        QVERIFY(item("exploreTaxonKind")->property("text").toString().contains("GRUPO"));
        QVERIFY(!item("exploreSourceDetails")->isVisible());
        QTRY_VERIFY(photoFits()); QVERIFY(capture("group", size));
        const auto childrenBottom = item("taxonChildrenGrid")->mapToScene(QPointF(0, item("taxonChildrenGrid")->height())).y();
        QVERIFY(item("exploreScientificFooter")->mapToScene(QPointF()).y() >= childrenBottom);
        item("taxonButton0")->forceActiveFocus(Qt::TabFocusReason); QVERIFY(capture("descendants", size));
        QVERIFY(activate("returnDomainsButton"));
        QVERIFY(edit("a")); QVERIFY(activate("loadMoreExploreGroups")); QVERIFY(activate("loadMoreExploreOrganisms"));
        QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 24);
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 24);
        item("exploreOrganismResult13")->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(160);
        const auto savedScroll = item("pageScroll")->property("contentY").toReal(); QVERIFY(savedScroll > 0);
        const auto savedId = item("exploreOrganismResult13")->property("entry").toMap()["id"];
        QVERIFY(activate("exploreOrganismResult13")); QCOMPARE(controller.browseTaxon()["id"], savedId);
        QVERIFY(activate("returnDomainsButton"));
        QCOMPARE(controller.exploreQuery(), QString("a"));
        QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 24);
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 24);
        QTRY_VERIFY(item("exploreOrganismResult13")->hasActiveFocus());
        QTRY_VERIFY(qAbs(item("pageScroll")->property("contentY").toReal() - savedScroll) <= 1);
        QVERIFY(capture("restored-results", size));
        QVERIFY(edit("Panthera")); QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 1);
        controller.setExploreDomain("domain:Eukaryota"); controller.setExploreRank("species");
        QVERIFY(activate("clearExploreQuery", true));
        QCOMPARE(controller.exploreQuery(), QString()); QCOMPARE(controller.exploreDomain(), QString("domain:Eukaryota"));
        QCOMPARE(controller.exploreRank(), QString("species"));
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 12);
        QVERIFY(item("exploreSearchField")->hasActiveFocus());
        QVERIFY(activate("clearExploreSearch"));
        QVERIFY(activate("domainButton0")); QCOMPARE(controller.browseTaxon()["count"].toInt(), 0);
        QVERIFY(!item("explorePhotoAttribution")->property("text").toString().isEmpty());
        QVERIFY(capture("empty-group", size)); QVERIFY(activate("returnDomainsButton"));
        for (const auto &name : {QString("Panthera leo"), QString("Danaus plexippus")}) {
            QVERIFY(edit(name)); controller.setExploreRank("species");
            auto result = item("exploreOrganismResult0"); QVERIFY(result);
            result->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(100);
            auto photo = findVisualItem(result, "encyclopediaCardPhoto"), frame = findVisualItem(result, "encyclopediaPhotoFrame");
            QTRY_COMPARE(photo->property("status").toInt(), 1);
            QVERIFY2(qAbs(photo->x() - (frame->width() - photo->width()) / 2) < 0.01,
                qPrintable(QString("photo x=%1 width=%2; frame width=%3").arg(photo->x()).arg(photo->width()).arg(frame->width())));
            QVERIFY(qAbs(photo->y() - (frame->height() - photo->height()) / 2) < 0.01);
            const auto note = result->property("entry").toMap()["educationalNote"].toMap();
            QCOMPARE(findVisualItem(result, "encyclopediaCardSummary")->property("text").toString(), note["text"].toString());
            QVERIFY(activate("exploreOrganismResult0")); QTRY_VERIFY(photoFits());
            const auto state = name.startsWith("Panthera") ? QString("lion") : QString("monarch");
            QVERIFY(capture(state, size));
            QVERIFY(item("exploreScientificFooter")->mapToScene(QPointF()).y() >= item("exploreDescendantsState")->mapToScene(QPointF(0, item("exploreDescendantsState")->height())).y());
            QVERIFY(activate("exploreClassification")); QVERIFY(activate("exploreSources")); QVERIFY(activate("explorePhotoCredits"));
            QVERIFY(item("explorePhotoProvenance")->property("text").toString().contains("Modificações históricas registradas"));
            QVERIFY(item("explorePhotoPresentation")->property("text").toString().contains("sem recorte"));
            QVERIFY(capture(state + "-footer", size)); QVERIFY(activate("returnDomainsButton"));
            QCOMPARE(controller.exploreQuery(), name); QCOMPARE(controller.exploreRank(), QString("species"));
        }
    }
    QCOMPARE(controller.answeredCount(), 0); QVERIFY(controller.recoverableSessions().isEmpty());
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    QFile manifest("previews/explore-2.2.2-creditos/capturas.json"); QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.write(QJsonDocument(QJsonObject{{"method", "QQuickWindow::grabWindow / production QML and GameController in QtTest"},
        {"privacy", "Temporary SQLite databases; Discord disabled; links intercepted without opening a browser."},
        {"rendering", "Qt offscreen/software, scale 1, 96 DPI; reduced motion for static captures."}, {"captures", captures}}).toJson());
}

void QmlInterfaceTests::encyclopedicPhotographsStayCenteredWithoutCropping()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl("qrc:/resources/qml/components/ExploreCard.qml"));
    JsonSpeciesCatalog catalog; JsonExplorationMediaCatalog media; TaxonomyIndex index(catalog, &media);
    auto entry = index.quickAccess().first().toMap();
    QScopedPointer<QObject> object(component.createWithInitialProperties({{"entry", entry}, {"encyclopedic", true}}));
    QVERIFY2(object, qPrintable(component.errorString()));
    auto card = qobject_cast<QQuickItem *>(object.data()); QVERIFY(card);
    QQuickWindow window; window.resize(1000,700); card->setParentItem(window.contentItem()); window.show();
    auto photo = findVisualItem(card, "encyclopediaCardPhoto"); QVERIFY(photo);
    auto frame = findVisualItem(card, "encyclopediaPhotoFrame"); QVERIFY(frame);
    for (int width : {280, 372, 510}) {
        card->setWidth(width); QTest::qWait(40); card->setHeight(card->implicitHeight());
        QTRY_COMPARE(photo->property("status").toInt(), 1);
        for (double focus : {0.1, 0.5, 0.9}) {
            // Geometry fixture only. No shipped photo metadata is edited.
            auto metadata = entry["photo"].toMap(); metadata["focusX"] = focus; metadata["focusY"] = 1. - focus;
            entry["photo"] = metadata; card->setProperty("entry", entry); QTest::qWait(30);
            const auto pixels = photo->property("sourceSize").toSize(); QVERIFY(!pixels.isEmpty());
            QVERIFY(photo->width() > 0 && photo->height() > 0);
            QVERIFY(qAbs(photo->width() / photo->height() - double(pixels.width()) / pixels.height()) < 0.001);
            QVERIFY(photo->x() >= -0.01 && photo->y() >= -0.01);
            QVERIFY(photo->x() + photo->width() <= frame->width() + 0.01);
            QVERIFY(photo->y() + photo->height() <= frame->height() + 0.01);
            QCOMPARE(photo->property("fillMode").toInt(), 1); // PreserveAspectFit.
            QVERIFY2(qAbs(photo->x() - (frame->width() - photo->width()) / 2) < 0.01,
                qPrintable(QString("x=%1 width=%2 frame=%3 centered=%4").arg(photo->x()).arg(photo->width()).arg(frame->width()).arg(QQmlProperty(photo, "anchors.alignWhenCentered").read().toBool())));
            QVERIFY(qAbs(photo->y() - (frame->height() - photo->height()) / 2) < 0.01);
            QVERIFY(photo->width() <= pixels.width() && photo->height() <= pixels.height());
        }
        QVERIFY(findVisualItem(card, "explorationPhoto")->property("source").toUrl().isEmpty());
    }
    entry["count"] = 1; card->setProperty("entry", entry);
    QCOMPARE(findVisualItem(card, "encyclopediaCardCount")->property("text").toString(), QString("1 organismo no catálogo"));
}

void QmlInterfaceTests::progressiveExploreNavigationAndCaptures()
{
    interfaceWarnings.clear();
    const auto oldHandler = qInstallMessageHandler(captureMessage);
    const auto restore = qScopeGuard([oldHandler] { qInstallMessageHandler(oldHandler); });
    QTemporaryDir directory; QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("explore-round-1.sqlite"));
    DiscordRichPresence presence{QString(), QString()}; QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first()); QVERIFY(window);
    window->setProperty("motionEnabled", false); window->requestActivate();
    const auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    const auto activate = [&](const QString &name) {
        auto control = item(name); if (!control || !control->isVisible() || !control->isEnabled()) return false;
        control->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(40);
        QTest::keyClick(window, Qt::Key_Space); QTest::qWait(80); return true;
    };
    const auto edit = [&](const QString &text) {
        auto field = item("exploreSearchField"); if (!field) return false;
        field->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_Backspace);
        for (auto c : text) QTest::keyClick(window, c.toLatin1());
        return controller.exploreQuery() == text && field->property("text").toString() == text;
    };
    const auto selectFilter = [&](const QString &name, int index) {
        auto combo = item(name); if (!combo) return false;
        combo->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_Space); QTest::qWait(30);
        QTest::keyClick(window, Qt::Key_Home);
        for (int i = 0; i < index; ++i) QTest::keyClick(window, Qt::Key_Down);
        QTest::keyClick(window, Qt::Key_Return); QTest::qWait(60); return true;
    };
    QString prefixFailure;
    const auto verifyPrefix = [&](const QString &prefix, const QVariantList &expected, int count) {
        QSet<QString> ids;
        for (int i = 0; i < count; ++i) {
            auto card = item(prefix + QString::number(i));
            if (!card || card->width() < 280) { prefixFailure = QString("%1%2 missing/narrow: %3").arg(prefix).arg(i).arg(card ? card->width() : -1); return false; }
            const auto entry = card->property("entry").toMap();
            const auto key = entry["id"].toString() + "/" + entry["speciesId"].toString();
            if (ids.contains(key) || entry != expected[i].toMap()) { prefixFailure = QString("%1%2 data mismatch: %3 / expected %4; type %5").arg(prefix).arg(i).arg(key, expected[i].toMap()["id"].toString(), QString::fromLatin1(card->property("entry").typeName())); return false; }
            ids.insert(key);
            const auto bounds = card->mapRectToScene(card->boundingRect());
            if (bounds.left() < 0 || bounds.right() > window->width() + 1) { prefixFailure = QString("%1%2 outside window: %3..%4").arg(prefix).arg(i).arg(bounds.left()).arg(bounds.right()); return false; }
        }
        if (item(prefix + QString::number(count))) { prefixFailure = prefix + " has an extra card"; return false; }
        return true;
    };
    const auto lazyPhotos = [&] {
        const auto scroll = item("pageScroll");
        const std::function<bool(QQuickItem *)> inspect = [&](QQuickItem *node) {
            if (node->property("encyclopedic").toBool() && node->isVisible()) {
                auto frame = findVisualItem(node, "encyclopediaPhotoFrame");
                auto photo = findVisualItem(node, "encyclopediaCardPhoto");
                auto viewport = qobject_cast<QQuickItem *>(node->property("photoViewport").value<QObject *>());
                if (!frame || !photo || !viewport) return false;
                const bool intersects = frame->mapRectToItem(scroll, frame->boundingRect()).intersects(scroll->boundingRect())
                    && frame->mapRectToItem(viewport, frame->boundingRect()).intersects(viewport->boundingRect());
                const auto source = node->property("photo").toMap()["image"].toString();
                if (!source.isEmpty() && photo->property("source").toUrl().isEmpty() == intersects) return false;
                if (!findVisualItem(node, "explorationPhoto")->property("source").toUrl().isEmpty()) return false;
            }
            for (auto child : node->childItems()) if (!inspect(child)) return false;
            return true;
        };
        return inspect(window->contentItem());
    };
    QVERIFY(QDir().mkpath("previews/explore-2.2.2-round-1")); QJsonArray captures;
    const auto capture = [&](const QString &state, QSize size) {
        QTest::qWait(220);
        const auto filename = state + "-" + QString::number(size.width()) + "x" + QString::number(size.height()) + ".png";
        const auto image = window->grabWindow();
        if (image.size() != size || !image.save("previews/explore-2.2.2-round-1/" + filename)) return false;
        captures.append(QJsonObject::fromVariantMap({{"file", filename}, {"width", size.width()}, {"height", size.height()},
            {"captured_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}, {"page", window->property("page")},
            {"query", controller.exploreQuery()}, {"domainId", controller.exploreDomain()}, {"rank", controller.exploreRank()},
            {"taxonId", controller.browseTaxon()["id"]},
            {"groupsShown", item("exploreGroupResults") ? item("exploreGroupResults")->property("count") : QVariant()},
            {"organismsShown", item("exploreOrganismResults") ? item("exploreOrganismResults")->property("count") : QVariant()}}));
        return true;
    };
    int speciesIndex = -1;
    const auto ranks = controller.exploreRanks();
    for (int i = 0; i < ranks.size(); ++i) if (ranks[i].toMap()["id"] == "species") speciesIndex = i;
    QVERIFY(speciesIndex > 0);
    for (QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size); controller.setExploreQuery(""); controller.setExploreDomain(""); controller.setExploreRank("");
        QVERIFY(activate("navExplore")); item("pageScroll")->setProperty("contentY", 0);
        QVERIFY(capture("domains", size));
        for (int i = 0; i < 3; ++i) QVERIFY(item("domainButton" + QString::number(i))->width() >= 280);
        for (int i = 0; i < controller.featuredOrganisms().size(); ++i) QVERIFY(item("featuredOrganism" + QString::number(i))->width() >= 280);
        QTRY_VERIFY(lazyPhotos());
        item("featuredOrganism0")->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(100);
        QTRY_VERIFY(lazyPhotos()); QVERIFY(capture("fitted-featured", size));
        QVERIFY(edit("a"));
        QTRY_VERIFY(!controller.exploreSearchPending());
        const auto broad = controller.exploreResults();
        QVERIFY(broad["groupCount"].toInt() > 24 && broad["organismCount"].toInt() > 24);
        QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 12);
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 12);
        QTRY_VERIFY2(verifyPrefix("exploreGroupResult", broad["groups"].toList(), 12), qPrintable(prefixFailure));
        QTRY_VERIFY2(verifyPrefix("exploreOrganismResult", broad["organisms"].toList(), 12), qPrintable(prefixFailure));
        QTRY_VERIFY(lazyPhotos()); QVERIFY(capture("broad-first-page", size));
        QVERIFY(activate("loadMoreExploreGroups")); QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 24);
        QTRY_VERIFY(item("exploreGroupResult12")->hasActiveFocus());
        QTRY_VERIFY2(verifyPrefix("exploreGroupResult", broad["groups"].toList(), 24), qPrintable(prefixFailure));
        QCOMPARE(controller.exploreResults(), broad); QTRY_VERIFY(lazyPhotos()); QVERIFY(capture("more-groups", size));
        QVERIFY(activate("loadMoreExploreOrganisms")); QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 24);
        QTRY_VERIFY2(verifyPrefix("exploreOrganismResult", broad["organisms"].toList(), 24), qPrintable(prefixFailure));
        QCOMPARE(controller.exploreResults(), broad); QTRY_VERIFY(lazyPhotos());
        QVERIFY(selectFilter("exploreDomainFilter", 2)); QCOMPARE(controller.exploreDomain(), QString("domain:Bacteria"));
        QCOMPARE(controller.exploreQuery(), QString("a"));
        QCOMPARE(item("exploreGroupResults")->property("count").toInt(), qMin(12, controller.exploreResults()["groupCount"].toInt()));
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 12);
        QVERIFY(selectFilter("exploreRankFilter", speciesIndex)); QCOMPARE(controller.exploreRank(), QString("species"));
        QCOMPARE(controller.exploreResults()["groupCount"].toInt(), 0); QCOMPARE(controller.exploreResults()["organismCount"].toInt(), 20);
        QVERIFY(activate("loadMoreExploreOrganisms")); QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 20);
        QVERIFY(!item("loadMoreExploreOrganisms")->isVisible());
        QTRY_VERIFY2(verifyPrefix("exploreOrganismResult", controller.exploreResults()["organisms"].toList(), 20), qPrintable(prefixFailure));
        QTRY_VERIFY(lazyPhotos()); QVERIFY(capture("filtered-last-page", size));
        QVERIFY(edit("Panthera leo"));
        QVERIFY(selectFilter("exploreDomainFilter", 0));
        QVERIFY(!controller.exploreSearchPending()); QCOMPARE(controller.exploreQuery(), QString("Panthera leo"));
        QCOMPARE(controller.exploreResults()["organismCount"].toInt(), 1);
        QVERIFY(item("exploreResultSummary")->property("text").toString().contains("1 organismo cadastrado"));
        const auto destination = controller.exploreResults()["organisms"].toList().first().toMap()["id"];
        QVERIFY(activate("exploreOrganismResult0")); QCOMPARE(controller.browseTaxon()["id"], destination);
        const auto trail = controller.breadcrumbs(); QVERIFY(trail.size() > 4);
        const auto compactHeight = item("taxonomyBreadcrumbs")->height();
        for (int i = 1; i <= trail.size() - 4; ++i) QVERIFY(!item("breadcrumbButton" + QString::number(i))->isVisible());
        auto current = item("breadcrumbButton" + QString::number(trail.size() - 1)); QVERIFY(current->isVisible());
        QVERIFY(current->property("text").toString().endsWith(" · atual"));
        QVERIFY(capture("compact-breadcrumbs", size));
        QVERIFY(activate("expandTaxonomyBreadcrumbs"));
        for (int i = 0; i < trail.size(); ++i) QVERIFY(item("breadcrumbButton" + QString::number(i))->isVisible());
        QVERIFY(item("taxonomyBreadcrumbs")->height() >= compactHeight);
        QTest::keyClick(window, Qt::Key_Tab); QTRY_VERIFY(item("breadcrumbButton1")->hasActiveFocus());
        QVERIFY(capture("expanded-breadcrumbs", size));
        QVERIFY(activate("expandTaxonomyBreadcrumbs"));
        QTRY_COMPARE(item("taxonomyBreadcrumbs")->height(), compactHeight);
        QVERIFY(activate("expandTaxonomyBreadcrumbs")); QVERIFY(activate("breadcrumbButton1"));
        QCOMPARE(controller.browseTaxon()["id"], trail[1].toMap()["id"]);
        QVERIFY(item("returnDomainsButton")->property("visualFocus").toBool());
        QVERIFY(activate("returnDomainsButton")); QCOMPARE(controller.exploreQuery(), QString("Panthera leo"));
        QCOMPARE(item("exploreSearchField")->property("text").toString(), controller.exploreQuery());
        QCOMPARE(item("exploreOrganismResults")->property("count").toInt(), 1);
        QVERIFY(capture("return-to-results", size));
        QVERIFY(edit("Pan"));
        QVERIFY(activate("clearExploreSearch")); QTest::qWait(260);
        QCOMPARE(controller.exploreQuery(), QString()); QCOMPARE(item("exploreSearchField")->property("text").toString(), QString());
        QVERIFY(!controller.exploreSearchPending()); QCOMPARE(item("exploreGroupResults")->property("count").toInt(), 0);
        QVERIFY(edit("Pan"));
        window->setProperty("page", "home"); QTest::qWait(260); QVERIFY(activate("navExplore"));
        QCOMPARE(item("exploreSearchField")->property("text").toString(), QString("Pan"));
        QCOMPARE(controller.exploreQuery(), QString("Pan")); QVERIFY(!controller.exploreSearchPending());
        QVERIFY(edit("Panthera")); controller.setExploreRank("genus");
        QVERIFY(!controller.exploreSearchPending());
        QVERIFY(item("exploreResultSummary")->property("text").toString().startsWith("1 grupo taxonômico ·"));
        QVERIFY(edit("zzzz-no-result-123")); QTRY_VERIFY(!controller.exploreSearchPending());
        QVERIFY(item("exploreEmptyState")->isVisible()); QVERIFY(capture("empty-search", size));
    }
    // Tear down views before queued photo/focus checks can run. Pending work
    // must belong to the view, rather than retain a destroyed QML context.
    for (int i = 0; i < 6; ++i) {
        window->setProperty("page", "explore");
        controller.setExploreQuery(i % 2 ? "a" : "e"); controller.flushExploreSearch();
        window->setProperty("page", "taxonomy");
    }
    QTest::qWait(100);
    QVERIFY(controller.recoverableSessions().isEmpty()); QCOMPARE(controller.answeredCount(), 0);
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    QFile manifest("previews/explore-2.2.2-round-1/capturas.json"); QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.write(QJsonDocument(QJsonObject{{"method", "QQuickWindow::grabWindow / production QML and GameController in QtTest"},
        {"privacy", "Temporary SQLite databases; Discord disabled; no player data accessed."},
        {"rendering", "Qt offscreen/software, scale 1, 96 DPI; reduced motion for static captures."}, {"captures", captures}}).toJson());
}

void QmlInterfaceTests::encyclopedicCardsHandleLongTextAndUnavailablePhotos()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl("qrc:/resources/qml/components/ExploreCard.qml"));
    const QVariantMap entry{{"title", "Testus fixture with an unusually long scientific name for layout verification"},
        {"kind", "organism"}, {"rank", "species"}, {"rankLabel", QStringLiteral("Espécie")}, {"count", 1},
        {"pathText", "Fixture domain > A very long family name > A very long genus name > Testus fixture"},
        {"commonName", "Synthetic test record with a long common name"},
        {"photo", QVariantMap{{"pendingReason", "Synthetic missing-photo state; no scientific content or attribution is created."}}}};
    QScopedPointer<QObject> object(component.createWithInitialProperties({{"entry", entry}, {"encyclopedic", true}}));
    QVERIFY2(object, qPrintable(component.errorString()));
    auto card = qobject_cast<QQuickItem *>(object.data()); QVERIFY(card);
    QQuickWindow window; window.resize(1000,700); card->setParentItem(window.contentItem()); window.show();
    for (const int width : {242, 370, 510}) {
        card->setWidth(width); QTest::qWait(30); card->setHeight(card->implicitHeight()); QTest::qWait(30);
        QVERIFY(card->height() > 300);
        const std::function<void(QQuickItem *)> verify = [&](QQuickItem *node) {
            if (node->isVisible() && node->metaObject()->indexOfProperty("paintedHeight") >= 0) {
                QVERIFY(!node->property("truncated").toBool());
                QVERIFY(node->property("paintedHeight").toReal() <= node->height() + 1);
                const auto bounds = node->mapRectToItem(card, node->boundingRect());
                QVERIFY(bounds.left() >= 0 && bounds.right() <= card->width() + 1);
                QVERIFY(bounds.top() >= 0 && bounds.bottom() <= card->height() + 1);
            }
            for (auto child : node->childItems()) verify(child);
        };
        verify(card);
        card->forceActiveFocus(Qt::TabFocusReason); QVERIFY(card->property("visualFocus").toBool());
        QVERIFY(findVisualItem(card, "encyclopediaCardPhoto")->property("source").toUrl().isEmpty());
    }
}

void QmlInterfaceTests::scientificExploreSearchFiltersNavigationAndCaptures()
{
    interfaceWarnings.clear();
    const auto previousHandler = qInstallMessageHandler(captureMessage);
    const auto restoreHandler = qScopeGuard([previousHandler] { qInstallMessageHandler(previousHandler); });
    QTemporaryDir directory; QVERIFY(directory.isValid());
    GameController controller(nullptr, directory.filePath("explore-2.2.2.sqlite"));
    DiscordRichPresence presence{QString(), QString()};
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first()); QVERIFY(window);
    window->setProperty("motionEnabled", false); window->requestActivate();
    const auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    const auto activate = [&](const QString &name) {
        auto control = item(name); if (!control || !control->isVisible() || !control->isEnabled()) return false;
        control->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(20);
        QTest::keyClick(window, Qt::Key_Space); QTest::qWait(80); return true;
    };
    const auto reset = [&] { controller.setExploreQuery(""); controller.setExploreDomain(""); controller.setExploreRank(""); };
    const auto typeSearch = [&](const QString &query) {
        auto field = item("exploreSearchField"); if (!field) return false;
        field->forceActiveFocus(Qt::TabFocusReason);
        QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        for (const auto c : query) QTest::keyClick(window, c.toLatin1());
        return QTest::qWaitFor([&] { return controller.exploreQuery() == query && !controller.exploreSearchPending(); }, 3000);
    };
    const auto selectFilter = [&](const QString &name, int index) {
        auto combo = item(name); if (!combo) return false;
        combo->forceActiveFocus(Qt::TabFocusReason); QTest::keyClick(window, Qt::Key_Space);
        QTest::qWait(40); QTest::keyClick(window, Qt::Key_Home);
        for (int i = 0; i < index; ++i) QTest::keyClick(window, Qt::Key_Down);
        QTest::keyClick(window, Qt::Key_Return); QTest::qWait(80); return true;
    };
    QVERIFY(QDir().mkpath("previews/explore-2.2.2"));
    QJsonArray captures;
    const auto capture = [&](const QString &state, QSize size) {
        QTest::qWait(200);
        const QString name = state + "-" + QString::number(size.width()) + "x" + QString::number(size.height()) + ".png";
        const auto image = window->grabWindow();
        if (image.size() != size || !image.save("previews/explore-2.2.2/" + name)) return false;
        captures.append(QJsonObject::fromVariantMap({{"file", name}, {"captured_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
            {"width", size.width()}, {"height", size.height()}, {"page", window->property("page")},
            {"taxonId", controller.browseTaxon()["id"]}, {"query", controller.exploreQuery()},
            {"domainId", controller.exploreDomain()}, {"rank", controller.exploreRank()}}));
        return true;
    };
    const auto fits = [&](QQuickItem *label) {
        return label && !label->property("truncated").toBool()
            && label->property("paintedHeight").toReal() <= label->height() + 1;
    };
    int speciesIndex = -1;
    const auto ranks = controller.exploreRanks();
    for (int i = 0; i < ranks.size(); ++i) if (ranks[i].toMap()["id"] == "species") speciesIndex = i;
    QVERIFY(speciesIndex > 0);
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size); reset(); QVERIFY(activate("navExplore"));
        QCOMPARE(window->property("currentSection").toString(), QString("explore"));
        QVERIFY(!item("playModeSelector")->isVisible()); QVERIFY(!item("categoryButton0")->isVisible());
        item("navExplore")->forceActiveFocus(Qt::MouseFocusReason); item("pageScroll")->setProperty("contentY", 0);
        QVERIFY(capture("explore", size));
        for (int i = 0; i < 3; ++i) {
            auto card = item("domainButton" + QString::number(i)); QVERIFY(card);
            const auto bounds = card->mapRectToScene(card->boundingRect());
            QVERIFY(bounds.left() >= 0 && bounds.right() <= size.width());
            QVERIFY(fits(findVisualItem(card, "encyclopediaCardTitle")));
        }
        auto featured = item("featuredOrganism0"); QVERIFY(featured);
        featured->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(80);
        QVERIFY(featured->property("visualFocus").toBool());
        const auto focused = featured->mapRectToItem(item("pageScroll"), featured->boundingRect());
        QVERIFY(focused.top() >= -1 && focused.bottom() <= item("pageScroll")->height() + 1);
        QVERIFY(capture("featured", size));
        QVERIFY(typeSearch("Panthera")); QTRY_COMPARE(controller.exploreQuery(), QString("Panthera"));
        QVERIFY(controller.exploreResults()["groupCount"].toInt() > 0);
        QVERIFY(controller.exploreResults()["organismCount"].toInt() > 1);
        QVERIFY(capture("search", size));
        QVERIFY(selectFilter("exploreDomainFilter", 3)); QTRY_COMPARE(controller.exploreDomain(), QString("domain:Eukaryota"));
        QVERIFY(selectFilter("exploreRankFilter", speciesIndex)); QTRY_COMPARE(controller.exploreRank(), QString("species"));
        QCOMPARE(controller.exploreResults()["groupCount"].toInt(), 0);
        QVERIFY(capture("filtered", size));
        const auto destination = controller.exploreResults()["organisms"].toList().first().toMap()["id"];
        QVERIFY(activate("exploreOrganismResult0"));
        QCOMPARE(controller.browseTaxon()["id"], destination);
        QCOMPARE(window->property("currentSection").toString(), QString("explore"));
        QVERIFY(!item("startTaxonButton")->isVisible()); QVERIFY(fits(item("exploreTaxonTitle")));
        const auto summary = controller.browseTaxon()["summary"].toMap();
        if (!summary.isEmpty()) QCOMPARE(item("exploreIntroductionText")->property("text"), summary["text"]);
        QVERIFY(capture("taxon", size));
        if (item("explorePhotoCredits")->isVisible()) {
            QVERIFY(activate("explorePhotoCredits")); QVERIFY(item("explorePhotoDetails")->isVisible());
            QVERIFY(capture("photo-credits", size));
        }
        QVERIFY(activate("breadcrumbButton0"));
        QCOMPARE(controller.browseTaxon()["id"].toString(), QString("domain:Eukaryota"));
        QVERIFY(!item("explorePhotoDetails")->isVisible());
        auto child = item("taxonButton0"); QVERIFY(child);
        const auto childId = controller.browseChildren().first().toMap()["id"];
        child->forceActiveFocus(Qt::TabFocusReason); QTest::qWait(80);
        QVERIFY(fits(findVisualItem(child, "encyclopediaCardTitle")));
        QVERIFY(capture("descendants", size));
        QVERIFY(activate("taxonButton0")); QCOMPARE(controller.browseTaxon()["id"], childId);
        QCOMPARE(controller.breadcrumbs().last().toMap()["id"], childId);
        QVERIFY(activate("returnDomainsButton"));
        QCOMPARE(item("exploreSearchField")->property("text").toString(), QString("Panthera"));
        QCOMPARE(item("exploreDomainFilter")->property("currentValue").toString(), QString("domain:Eukaryota"));
        QCOMPARE(item("exploreRankFilter")->property("currentValue").toString(), QString("species"));
        QVERIFY(typeSearch("zzzz-no-match-123"));
        QVERIFY(item("exploreEmptyState")->isVisible()); QVERIFY(fits(item("exploreEmptyState")));
        QVERIFY(capture("empty-search", size));
        QVERIFY(activate("clearExploreSearch")); QVERIFY(item("exploreSearchField")->hasActiveFocus());
        QVERIFY(activate("domainButton0")); QCOMPARE(controller.browseTaxon()["id"].toString(), QString("domain:Archaea"));
        QCOMPARE(controller.browseTaxon()["count"].toInt(), 0);
        QVERIFY(capture("empty-group", size));
    }
    QCOMPARE(controller.answeredCount(), 0); QVERIFY(controller.recoverableSessions().isEmpty());
    QVERIFY(activate("navPlay")); QVERIFY(activate("categoryButton0"));
    const auto photo = controller.questionPhoto(), metrics = controller.sessionMetrics();
    const auto answers = controller.answerOptions(); const auto hints = controller.hints();
    const auto player = controller.currentPlayerId(); const auto group = controller.sessionTaxonId();
    const auto checkpoints = controller.recoverableSessions();
    QVERIFY(activate("navExplore")); QVERIFY(typeSearch("Panthera"));
    controller.setExploreRank("species"); QVERIFY(activate("exploreOrganismResult0")); QVERIFY(activate("breadcrumbButton0"));
    QCOMPARE(controller.questionPhoto(), photo); QCOMPARE(controller.answerOptions(), answers);
    QCOMPARE(controller.hints(), hints); QCOMPARE(controller.currentPlayerId(), player);
    QCOMPARE(controller.sessionTaxonId(), group); QCOMPARE(controller.sessionMetrics(), metrics);
    QCOMPARE(controller.recoverableSessions(), checkpoints);
    QVERIFY(activate("headerResumeGame")); QCOMPARE(window->property("page").toString(), QString("quiz"));
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    QFile manifest("previews/explore-2.2.2/capturas.json"); QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.write(QJsonDocument(QJsonObject{{"method", "QQuickWindow::grabWindow / real production QML and GameController in QtTest"},
        {"privacy", "Temporary SQLite databases and default profile; Discord disabled; no player data accessed."},
        {"rendering", "Qt offscreen/software, scale 1, 96 DPI; reduced motion for static captures."},
        {"captures", captures}}).toJson());
}

void QmlInterfaceTests::dailyDiscoveryNavigationIndicatorsAndCaptures()
{
    interfaceWarnings.clear();
    const auto previousHandler = qInstallMessageHandler(captureMessage);
    const auto restoreHandler = qScopeGuard([previousHandler] { qInstallMessageHandler(previousHandler); });
    TestUrlRecorder urlRecorder;
    QDesktopServices::setUrlHandler("https", &urlRecorder, "capture");
    const auto restoreUrls = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
    QTemporaryDir directory;
    QDate date = QDate::currentDate();
    GameController controller(nullptr, directory.filePath("daily-interface.sqlite"), [&] { return date; });
    DiscordRichPresence presence{QString(), QString()};
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl("qrc:/resources/qml/Main.qml"));
    QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(interfaceWarnings.join('\n')));
    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first()); QVERIFY(window);
    const auto item = [window](const QString &name) { return findVisualItem(window->contentItem(), name); };
    const auto activate = [&](const QString &name) {
        auto control = item(name); if (!control) return false;
        control->forceActiveFocus(Qt::TabFocusReason);
        QTest::keyClick(window, Qt::Key_Space); QTest::qWait(50); return true;
    };
    window->setProperty("motionEnabled", false);
    window->requestActivate();
    QTRY_VERIFY(item("dailyDiscoveryPanel"));
    QTRY_COMPARE(item("dailyDiscoveryPhoto")->property("status").toInt(), 1);
    const auto indicators = controller.homeCatalogIndicators();
    QCOMPARE(item("homePhotoCount")->property("title").toString(), QStringLiteral("Identificação por espécie"));
    QVERIFY(item("homePhotoCount")->property("description").toString().contains("nível de espécie"));
    QVERIFY(item("homePhotoCount")->property("description").toString().contains("não é o total de fotos"));
    for (const auto &pair : {qMakePair("homeOrganismCount", "organisms"),
                            qMakePair("homeDomainCount", "representedDomains"),
                            qMakePair("homePhotoCount", "identificationPhotos")})
        QCOMPARE(item(pair.first)->property("value").toString(), QString::number(indicators[pair.second].toInt()));
    const auto verifyDiscovery = [&] {
        QCOMPARE(item("dailyDiscoveryScientificName")->property("text").toString(), controller.dailyDiscovery()["scientificName"].toString());
        QCOMPARE(item("dailyDiscoveryText")->property("text").toString(), controller.dailyDiscovery()["note"].toMap()["text"].toString());
    };
    verifyDiscovery();
    const int firstId = controller.dailyDiscovery()["speciesId"].toInt();
    date = date.addDays(1); controller.refreshHomeCatalog();
    QTRY_VERIFY(controller.dailyDiscovery()["speciesId"].toInt() != firstId);
    verifyDiscovery();
    date = QDate::currentDate(); controller.refreshHomeCatalog();
    QTRY_COMPARE(item("dailyDiscoveryPhoto")->property("status").toInt(), 1);
    QVERIFY(QDir().mkpath("previews/daily-discovery"));
    QFile context("previews/daily-discovery/context.json");
    QVERIFY(context.open(QIODevice::WriteOnly));
    context.write(QJsonDocument(QJsonObject::fromVariantMap({{"discovery", controller.dailyDiscovery()},
        {"indicators", controller.homeCatalogIndicators()}})).toJson());
    context.close();
    for (const QSize size : {QSize(1000,700), QSize(1240,820), QSize(1920,1080)}) {
        window->resize(size);
        item("navHome")->forceActiveFocus(Qt::MouseFocusReason);
        item("pageScroll")->setProperty("contentY", 0);
        QTest::qWait(300);
        for (const QString &name : {"homeOrganismCount", "homeDomainCount", "homePhotoCount"}) {
            auto card = item(name); QVERIFY(card);
            auto container = findVisualItem(card, "statCardIconContainer"); QVERIFY(container);
            auto icon = findVisualItem(container, "statCardIcon"); QVERIFY(icon);
            QCOMPARE(container->width(), 48.); QCOMPARE(container->height(), 48.);
            QCOMPARE(icon->width(), 28.); QCOMPARE(icon->height(), 28.);
            QVERIFY(qAbs(container->mapToItem(card, container->boundingRect().center()).y() - card->height() / 2) < 1);
            QVERIFY(qAbs(icon->x() + icon->width() / 2 - container->width() / 2) < 1);
            QVERIFY(qAbs(icon->y() + icon->height() / 2 - container->height() / 2) < 1);
            QVERIFY(card->boundingRect().contains(container->mapRectToItem(card, container->boundingRect())));
            QVERIFY(QQmlProperty(card, "Accessible.name", qmlContext(card)).read().toString().contains(card->property("value").toString()));
        }
        const QString suffix = QString::number(size.width()) + "x" + QString::number(size.height()) + ".png";
        QVERIFY(window->grabWindow().save("previews/daily-discovery/home-" + suffix));
        auto explore = item("dailyDiscoveryExplore"); QVERIFY(explore);
        explore->forceActiveFocus(Qt::TabFocusReason);
        QTest::qWait(100);
        QVERIFY(explore->hasActiveFocus());
        QVERIFY(explore->property("visualFocus").toBool());
        QTest::keyClick(window, Qt::Key_Tab);
        QTRY_VERIFY(item("dailyDiscoveryCredits")->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_Backtab, Qt::ShiftModifier);
        QTRY_VERIFY(explore->hasActiveFocus());
        const auto scroll = item("pageScroll");
        const auto focusBounds = explore->mapRectToItem(scroll, explore->boundingRect());
        QVERIFY(focusBounds.top() >= -1 && focusBounds.bottom() <= scroll->height() + 1);
        const auto panel = item("dailyDiscoveryPanel");
        for (const QString &name : {"dailyDiscoveryScientificName", "dailyDiscoveryCommonName", "dailyDiscoveryText"}) {
            auto label = item(name); QVERIFY(label);
            if (!label->isVisible()) continue;
            QVERIFY(!label->property("truncated").toBool());
            QVERIFY(label->property("paintedHeight").toReal() <= label->height() + 1);
            QVERIFY(panel->boundingRect().contains(label->mapRectToItem(panel, label->boundingRect())));
            QVERIFY(colorContrast(label->property("color").value<QColor>(), panel->property("color").value<QColor>()) >= 4.5);
        }
        // Keep the keyboard focus, then frame the complete discovery rather
        // than only the focused button at the bottom of the viewport.
        const qreal panelTop = scroll->property("contentY").toReal() + panel->mapToItem(scroll, QPointF()).y();
        scroll->setProperty("contentY", qBound(0., panelTop,
            qMax(0., scroll->property("contentHeight").toReal() - scroll->height())));
        QTest::qWait(100);
        QVERIFY(window->grabWindow().save("previews/daily-discovery/discovery-" + suffix));
        scroll->setProperty("contentY", qMax(0., scroll->property("contentHeight").toReal() - scroll->height()));
        QTest::qWait(100);
        QVERIFY(window->grabWindow().save("previews/daily-discovery/indicators-" + suffix));
        auto source = item("dailyDiscoverySource"); QVERIFY(source);
        QTest::mouseMove(window, source->mapToScene(source->boundingRect().center()).toPoint());
        QTRY_VERIFY(source->property("hovered").toBool());
        QTest::qWait(650);
        QQmlProperty tooltip(source, "ToolTip.visible", qmlContext(source));
        QVERIFY(tooltip.isValid()); QVERIFY(!tooltip.read().toBool());
        QVERIFY(source->property("text").toString().contains(controller.dailyDiscovery()["note"].toMap()["source_title"].toString()));
        QCOMPARE(QQmlProperty(source, "Accessible.description", qmlContext(source)).read().toString(), controller.dailyDiscovery()["note"].toMap()["source_url"].toString());
        int links = urlRecorder.urls.size();
        QVERIFY(activate("dailyDiscoverySource"));
        QCOMPARE(urlRecorder.urls.size(), links + 1);
        QCOMPARE(urlRecorder.urls.last(), QUrl(controller.dailyDiscovery()["note"].toMap()["source_url"].toString()));
        QVERIFY(source->property("visualFocus").toBool());
        QVERIFY(!tooltip.read().toBool());
        QVERIFY(activate("dailyDiscoveryCredits"));
        auto dialog = window->findChild<QObject *>("dailyDiscoveryCreditsDialog"); QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QCOMPARE(window->property("page").toString(), QString("home"));
        QVERIFY(dialog->property("modal").toBool());
        QVERIFY(!window->findChild<QObject *>("homeCreditsDialog")->property("visible").toBool());
        QVERIFY(dialog->property("x").toReal() >= 0 && dialog->property("y").toReal() >= 0);
        QVERIFY(dialog->property("width").toReal() <= window->width());
        QVERIFY(dialog->property("height").toReal() <= window->height());
        auto photo = controller.dailyDiscovery()["photo"].toMap();
        auto attribution = photo["attribution"].toMap();
        QCOMPARE(item("dailyCreditSubject")->property("text").toString(), controller.dailyDiscovery()["scientificName"].toString());
        QVERIFY(item("dailyCreditAuthor")->property("text").toString().contains(photo["author"].toString()));
        QVERIFY(item("dailyCreditOrigin")->property("text").toString().contains(photo["sourceUrl"].toString()));
        QVERIFY(item("dailyCreditModifications")->property("text").toString().contains(photo["modifications"].toString()));
        if (attribution["license_version"].toString().isEmpty()) {
            QVERIFY(item("dailyCreditLicense")->property("text").toString().contains("Não comprovada"));
            QVERIFY(item("dailyCreditNotices")->property("text").toString().contains("não comprovada"));
        }
        if (!attribution["rights_issues"].toList().isEmpty())
            QVERIFY(item("dailyCreditNotices")->property("text").toString().contains("Permissões com pendências"));
        // The contextual data follows the displayed discovery if midnight
        // occurs while the dialog is open; it never adopts carousel credits.
        date = date.addDays(1); controller.refreshHomeCatalog();
        QTRY_COMPARE(item("dailyCreditSubject")->property("text").toString(), controller.dailyDiscovery()["scientificName"].toString());
        date = QDate::currentDate(); controller.refreshHomeCatalog();
        QTRY_COMPARE(item("dailyCreditSubject")->property("text").toString(), controller.dailyDiscovery()["scientificName"].toString());
        for (const auto &link : {qMakePair("dailyCreditPhotoLink", photo["sourceUrl"].toString()),
                                 qMakePair("dailyCreditObservationLink", attribution["source"].toString()),
                                 qMakePair("dailyCreditLicenseLink", dialog->property("licenseUrl").toString())}) {
            if (!item(link.first)->isVisible()) continue;
            links = urlRecorder.urls.size();
            QVERIFY(activate(link.first));
            QCOMPARE(urlRecorder.urls.size(), links + 1);
            QCOMPARE(urlRecorder.urls.last(), QUrl(link.second));
        }
        QVERIFY(activate("dailyDiscoveryCreditsClose"));
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QTRY_VERIFY(item("dailyDiscoveryCredits")->hasActiveFocus());
        QVERIFY(activate("dailyDiscoveryCredits"));
        QTest::keyClick(window, Qt::Key_Tab);
        QVERIFY(window->activeFocusItem() != item("dailyDiscoveryCredits"));
        item("dailyDiscoveryCreditsClose")->forceActiveFocus(Qt::TabFocusReason);
        QTest::qWait(150);
        QVERIFY(window->grabWindow().save("previews/daily-discovery/credits-" + suffix));
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QTRY_VERIFY(item("dailyDiscoveryCredits")->hasActiveFocus());
    }
    // Enter exploration directly while a quiz is running; preserve its state.
    QVERIFY(activate("navPlay")); QVERIFY(activate("categoryButton0"));
    const auto options = controller.answerOptions(); const auto photo = controller.questionPhoto();
    QVERIFY(activate("navHome"));
    const auto taxon = controller.dailyDiscovery()["taxonId"].toString();
    QVERIFY(activate("dailyDiscoveryExplore"));
    QCOMPARE(window->property("page").toString(), QString("taxonomy"));
    QCOMPARE(window->property("currentSection").toString(), QString("explore"));
    QCOMPARE(controller.browseTaxon()["id"].toString(), taxon);
    QVERIFY(!item("startTaxonButton")->isVisible());
    QVERIFY(activate("returnDomainsButton")); QCOMPARE(window->property("page").toString(), QString("explore"));
    QVERIFY(activate("navPlay")); QCOMPARE(window->property("page").toString(), QString("quiz"));
    QCOMPARE(controller.answerOptions(), options); QCOMPARE(controller.questionPhoto(), photo);
    QVERIFY(activate("navHome"));
    QVERIFY(activate("dailyDiscoveryCredits"));
    QVERIFY(activate("dailyDiscoveryAllCredits")); QCOMPARE(window->property("page").toString(), QString("credits"));
    QVERIFY(activate("navHome"));
    auto home = item("homeCarousel"); QVERIFY(home);
    home->setProperty("discovery", QVariantMap{});
    QTRY_VERIFY(item("dailyDiscoveryEmpty")->isVisible());
    QVERIFY(!item("dailyDiscoveryExplore")->isVisible());
    QVERIFY(item("homeExploreButton")->isEnabled() && item("homePlayButton")->isEnabled());
    window->resize(1000, 700); item("pageScroll")->setProperty("contentY", 200); QTest::qWait(100);
    QVERIFY(window->grabWindow().save("previews/daily-discovery/empty-1000x700.png"));
    QVERIFY2(interfaceWarnings.isEmpty(), qPrintable(interfaceWarnings.join('\n')));
}

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
            const auto title = item("homeCaptionTitle"), description = item("homeCaptionDescription");
            const auto scientificName = item("homeCaptionScientificName");
            QVERIFY(title && description && scientificName);
            QCOMPARE(title->property("text").toString(), slides[i].toMap()["title"].toString());
            QCOMPARE(description->property("text").toString(), slides[i].toMap()["excerpt"].toString());
            QCOMPARE(scientificName->property("text").toString(), slides[i].toMap()["photo"].toMap()["subject"].toString());
            QVERIFY(QQmlProperty(scientificName, "font.italic").read().toBool());
            QVERIFY(QQmlProperty(title, "font.pixelSize").read().toInt() > QQmlProperty(description, "font.pixelSize").read().toInt());
            QCOMPARE(QQmlProperty(title, "font.weight").read().toInt(), int(QFont::DemiBold));
            const auto titleBounds = title->mapRectToScene(title->boundingRect());
            const auto descriptionBounds = description->mapRectToScene(description->boundingRect());
            const auto nameBounds = scientificName->mapRectToScene(scientificName->boundingRect());
            QVERIFY(titleBounds.bottom() <= descriptionBounds.top());
            QVERIFY(descriptionBounds.bottom() <= nameBounds.top());
            for (auto text : {title, description, scientificName}) {
                QVERIFY(!text->property("truncated").toBool());
                QVERIFY(text->property("paintedHeight").toReal() <= text->height() + 1);
                const auto bounds = text->mapRectToScene(text->boundingRect());
                const auto heroBounds = item("homeHeroPhoto")->mapRectToScene(item("homeHeroPhoto")->boundingRect());
                QVERIFY(heroBounds.contains(bounds));
            }
            if (i == 0) {
                auto shade = item("homeCaptionShade");
                QVERIFY(shade);
                auto grab = shade->grabToImage();
                QVERIFY(grab);
                QTRY_VERIFY(!grab->image().isNull());
                const auto veil = grab->image();
                const int center = veil.width() / 2, bottom = veil.height() - 2;
                QVERIFY(veil.pixelColor(center, 0).alpha() < 5);
                QVERIFY(veil.pixelColor(center, bottom).alpha() > 220);
                QVERIFY(veil.pixelColor(0, bottom).alpha() < 10);
                QVERIFY(veil.pixelColor(veil.width() - 2, veil.height() / 4).alpha()
                    < veil.pixelColor(center, bottom).alpha());
                QVERIFY(veil.pixelColor(center, veil.height() / 3).alpha()
                    < veil.pixelColor(center, veil.height() * 2 / 3).alpha());
                // Contrast against a white photo pixel under the actual veil.
                for (auto text : {title, description, scientificName, credit}) {
                    const auto point = shade->mapFromItem(text, QPointF(2,
                        qMin(text->height() / 2, QQmlProperty(text, "font.pixelSize").read().toReal() / 2)));
                    const auto ink = veil.pixelColor(qBound(0, qRound(point.x() * veil.width() / shade->width()), veil.width() - 1),
                        qBound(0, qRound(point.y() * veil.height() / shade->height()), veil.height() - 1));
                    const auto alpha = ink.alphaF();
                    const auto background = QColor::fromRgbF(ink.redF() * alpha + 1 - alpha,
                        ink.greenF() * alpha + 1 - alpha, ink.blueF() * alpha + 1 - alpha);
                    const auto foreground = text == credit
                        ? credit->property("contentItem").value<QQuickItem *>()->property("color").value<QColor>()
                        : text->property("color").value<QColor>();
                    QVERIFY2(colorContrast(foreground, background) >= (text == title ? 3.0 : 4.5),
                        qPrintable(text->objectName() + " must remain readable over bright photos"));
                }
            }
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
    QTest::qWait(50); // Let the resized layout settle before a real mouse click.
    auto dot = item("homeSlideDot0");
    QVERIFY(dot);
    auto dotBackground = dot->property("background").value<QQuickItem *>();
    QVERIFY(dotBackground);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        dot->mapToScene(dot->boundingRect().center()).toPoint());
    QTRY_COMPARE(home->property("currentIndex").toInt(), 0);
    QTRY_VERIFY(home->property("photoReady").toBool());
    QTRY_VERIFY(dot->hasActiveFocus());
    QVERIFY(!dot->property("visualFocus").toBool());
    QCOMPARE(QQmlProperty(dotBackground, "border.color").read().value<QColor>().alpha(), 0);
    QTest::keyClick(window, Qt::Key_Tab);
    QTRY_VERIFY(item("homeSlideDot1")->hasActiveFocus());
    QVERIFY(item("homeSlideDot1")->property("visualFocus").toBool());
    QTest::keyClick(window, Qt::Key_Backtab, Qt::ShiftModifier);
    QTRY_VERIFY(dot->hasActiveFocus());
    QVERIFY(dot->property("visualFocus").toBool());
    QVERIFY(QQmlProperty(dotBackground, "border.color").read().value<QColor>().alpha() > 0);
    QVERIFY(click("homeCreditsButton"));
    auto dialog = engine.rootObjects().first()->findChild<QObject *>("homeCreditsDialog");
    QVERIFY(dialog);
    QVERIFY(dialog->property("visible").toBool());
    QTest::qWait(100);
    QVERIFY(window->grabWindow().save("previews/carousel/credits-1240x820.png"));
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!dialog->property("visible").toBool());
    QVERIFY(item("homeCreditsButton")->hasActiveFocus());
    auto creditButton = item("homeCreditsButton");
    QTest::mouseMove(window, creditButton->mapToScene(creditButton->boundingRect().center()).toPoint());
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        creditButton->mapToScene(creditButton->boundingRect().center()).toPoint());
    QTRY_VERIFY(dialog->property("visible").toBool());
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!dialog->property("visible").toBool());
    QTRY_VERIFY(creditButton->hasActiveFocus());
    QVERIFY(!creditButton->property("visualFocus").toBool());
    item("pageScroll")->setProperty("contentY", 0);
    QTest::qWait(50);
    QVERIFY(window->grabWindow().save("previews/carousel/mouse-focus-1240x820.png"));
    QTest::keyClick(window, Qt::Key_Tab);
    QTest::keyClick(window, Qt::Key_Backtab, Qt::ShiftModifier);
    QTRY_VERIFY(creditButton->hasActiveFocus());
    QVERIFY(creditButton->property("visualFocus").toBool());
    QTest::qWait(50);
    QVERIFY(window->grabWindow().save("previews/carousel/keyboard-focus-1240x820.png"));
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
    const auto slides = controller.homeSlides();
    const auto captionMatches = [&](int index) {
        const auto slide = slides[index].toMap();
        return item("homeCaptionTitle")->property("text").toString() == slide["title"].toString()
            && item("homeCaptionDescription")->property("text").toString() == slide["excerpt"].toString()
            && item("homeCaptionScientificName")->property("text").toString() == slide["photo"].toMap()["subject"].toString()
            && home->property("photo").toMap()["image"] == slide["photo"].toMap()["image"];
    };
    const auto ordinaryPhotoPoint = [&] {
        auto photo = item("homeHeroPhoto");
        return photo->mapToScene(QPointF(photo->width() * 0.7, 40)).toPoint();
    };
    window->requestActivate();
    // Hover over the photograph itself must not pause the carousel.
    QTest::mouseMove(window,ordinaryPhotoPoint());
    item("navHome")->forceActiveFocus(Qt::TabFocusReason);
    QCOMPARE(home->property("slideInterval").toInt(),8000);
    QCOMPARE(home->property("transitionDuration").toInt(),600);
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QTRY_VERIFY(home->property("photoReady").toBool());
    QTest::qWait(7000);
    QCOMPARE(home->property("currentIndex").toInt(),0);
    QTRY_VERIFY_WITH_TIMEOUT(home->property("transitioning").toBool(),2500);
    QCOMPARE(home->property("currentIndex").toInt(),0);
    QVERIFY(captionMatches(0));
    QTest::qWait(180);
    const auto blend=home->property("blend").toReal();
    QVERIFY(blend>0 && blend<1);
    auto first=item("homeHeroPhoto"),second=item("homeIncomingPhoto");
    QVERIFY(first->property("painted").toBool() && second->property("painted").toBool());
    QVERIFY(first->opacity()>0 && second->opacity()>0);
    QVERIFY(std::abs(first->opacity()+second->opacity()-1)<0.01);
    QCOMPARE(home->property("currentIndex").toInt(),0);
    QVERIFY(captionMatches(0));
    QVERIFY(QDir().mkpath("previews/carousel"));
    QVERIFY(window->grabWindow().save("previews/carousel/transition-in-progress-1240x820.png"));
    QTRY_COMPARE(home->property("pendingIndex").toInt(),-1);
    QCOMPARE(home->property("currentIndex").toInt(),1);
    QVERIFY(captionMatches(1));
    QVERIFY(window->grabWindow().save("previews/carousel/transition-complete-1240x820.png"));
    // Resume with a full real interval, rather than the remainder before pause.
    QTest::qWait(1800);
    auto caption = item("homePhotoCaption");
    QTest::mouseMove(window,caption->mapToScene(caption->boundingRect().center()).toPoint());
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    QTest::qWait(450);
    QCOMPARE(home->property("currentIndex").toInt(),1);
    QTest::mouseMove(window,ordinaryPhotoPoint());
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QTest::qWait(7000);
    QCOMPARE(home->property("currentIndex").toInt(),1);
    QCOMPARE(home->property("pendingIndex").toInt(),-1);
    QTRY_COMPARE_WITH_TIMEOUT(home->property("pendingIndex").toInt(),2,2500);
    QTRY_COMPARE(home->property("currentIndex").toInt(),2);
    QVERIFY(captionMatches(2));
    // Accelerate only the remaining checks; keep an interval longer than 600 ms.
    timer->setProperty("interval",1000);
    QTest::mouseMove(window,home->mapToScene(QPointF(100,100)).toPoint());
    QTRY_VERIFY(home->property("autoRunning").toBool());
    auto explore = item("homeExploreButton");
    QTest::mouseMove(window,explore->mapToScene(explore->boundingRect().center()).toPoint());
    QVERIFY(home->property("autoRunning").toBool());
    const auto bounds = [&](const QString &name) {
        auto control = item(name);
        return control->mapRectToScene(control->boundingRect());
    };
    const auto previousBounds = bounds("homePreviousSlide"), firstDotBounds = bounds("homeSlideDot0");
    QVector<QPointF> emptyControlSpaces{
        QPointF((previousBounds.right() + firstDotBounds.left()) / 2, firstDotBounds.center().y()),
        QPointF((bounds("homeSlideDot3").right() + bounds("homeNextSlide").left()) / 2, firstDotBounds.center().y())
    };
    for (int i = 0; i < 3; ++i) {
        emptyControlSpaces.append(QPointF((bounds("homeSlideDot" + QString::number(i)).right()
            + bounds("homeSlideDot" + QString::number(i + 1)).left()) / 2, firstDotBounds.center().y()));
    }
    if (firstDotBounds.top() > previousBounds.top())
        emptyControlSpaces.append(QPointF(firstDotBounds.center().x(),
            (firstDotBounds.top() + previousBounds.top()) / 2));
    for (const QString &name : {"homePreviousSlide", "homeSlideDot0", "homeSlideDot1", "homeSlideDot2",
                               "homeSlideDot3", "homeNextSlide", "homeCreditsButton"}) {
        auto control = item(name);
        QTest::mouseMove(window,control->mapToScene(control->boundingRect().center()).toPoint());
        QTRY_VERIFY(!home->property("autoRunning").toBool());
        // Empty space inside the same bar must resume, even between two dots.
        for (const auto &point : emptyControlSpaces) {
            QTest::mouseMove(window,point.toPoint());
            QTRY_VERIFY(home->property("autoRunning").toBool());
            QVERIFY(!home->property("pointerInteracting").toBool());
        }
    }
    QCOMPARE(home->property("currentIndex").toInt(),2);
    QTest::mouseMove(window,QPoint(10,10));
    QTRY_VERIFY(home->property("autoRunning").toBool());
    QTest::qWait(300);
    // Manual navigation restarts the interval and commits metadata only at end.
    QVERIFY(QMetaObject::invokeMethod(home,"selectSlide",Q_ARG(QVariant,3),Q_ARG(QVariant,true)));
    QCOMPARE(home->property("currentIndex").toInt(),2);
    QVERIFY(captionMatches(2));
    QTRY_COMPARE(home->property("currentIndex").toInt(),3);
    QVERIFY(captionMatches(3));
    QTest::qWait(250);
    QCOMPARE(home->property("pendingIndex").toInt(),-1);
    // Pause for credits independently of keyboard focus.
    QVERIFY(QMetaObject::invokeMethod(dialog,"open"));
    QTRY_VERIFY(!home->property("autoRunning").toBool());
    QTest::qWait(1100);
    QCOMPARE(home->property("currentIndex").toInt(),3);
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
    QCOMPARE(home->property("currentIndex").toInt(),3);
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
    QVERIFY(captionMatches(0));
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
                StatCard { objectName: "defaultStatCard"; title: "Catalogo"; value: "122" }
            }
        }
    )", QUrl("qrc:/design-system-test.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object, qPrintable(component.errorString()));
    auto window = qobject_cast<QQuickWindow *>(object.data()); QVERIFY(window);
    auto defaultCard = findVisualItem(window->contentItem(), "defaultStatCard"); QVERIFY(defaultCard);
    QCOMPARE(defaultCard->implicitHeight(), 118.);
    QCOMPARE(findVisualItem(defaultCard, "statCardIconContainer")->implicitWidth(), 42.);
    QCOMPARE(findVisualItem(defaultCard, "statCardIcon")->implicitWidth(), 23.);
    auto button = findVisualItem(window->contentItem(), "themeButton"); QVERIFY(button);
    auto background = button->property("background").value<QQuickItem *>(); QVERIFY(background);
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
            QVERIFY(colorContrast(text,background->property("color").value<QColor>()) >= 4.5);
            button->setProperty("down", false);
        }
    }
    button->setProperty("secondary", false); button->setProperty("danger", false);
    button->forceActiveFocus(Qt::TabFocusReason);
    QCOMPARE(QQmlProperty(background, "border.width").read().toInt(), 2);
    QVERIFY(colorContrast(window->property("focusColor").value<QColor>(), window->property("canvasColor").value<QColor>()) >= 3.0);
    QTest::keyClick(window, Qt::Key_Space); QCOMPARE(window->property("activations").toInt(), 1);
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(findVisualItem(window->contentItem(), "themeNav")->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(findVisualItem(window->contentItem(), "themeAction")->hasActiveFocus());
    // Mouse focus still operates the buttons; only keyboard focus draws gold.
    const QColor focusColor = window->property("focusColor").value<QColor>();
    for (const QString &name : {"themeButton", "themeNav", "themeAction"}) {
        auto control = findVisualItem(window->contentItem(), name);
        QVERIFY(control);
        auto surface = control->property("background").value<QQuickItem *>();
        QVERIFY(surface);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            control->mapToScene(control->boundingRect().center()).toPoint());
        QTRY_VERIFY(control->hasActiveFocus());
        QVERIFY(!control->property("visualFocus").toBool());
        QVERIFY(QQmlProperty(surface, "border.color").read().value<QColor>() != focusColor);
        QTest::mouseMove(window, QPoint(510,420));
        if (name == "themeAction") QCOMPARE(surface->property("color").value<QColor>().alpha(), 0);
        QTest::keyClick(window, Qt::Key_Tab);
        QTest::keyClick(window, Qt::Key_Backtab, Qt::ShiftModifier);
        QTRY_VERIFY(control->hasActiveFocus());
        QVERIFY(control->property("visualFocus").toBool());
        QCOMPARE(QQmlProperty(surface, "border.color").read().value<QColor>(), focusColor);
        QCOMPARE(QQmlProperty(surface, "border.width").read().toInt(), 2);
    }
    QCOMPARE(window->property("activations").toInt(), 2);
    button->setEnabled(false);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, button->mapToScene(QPointF(20,20)).toPoint());
    QCOMPARE(window->property("activations").toInt(), 2);
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
