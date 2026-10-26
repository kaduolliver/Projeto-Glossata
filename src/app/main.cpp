#include "presentation/GameController.h"
#include "infrastructure/discord/DiscordRichPresence.h"
#include "presentation/AppPreferences.h"
#include "presentation/AchievementAudio.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QTextStream>
#include <QTemporaryDir>
#include <QQuickWindow>
#include <QQuickItem>
#include <memory>

namespace {
QJsonObject discordConfiguration()
{
    QFile file(QCoreApplication::applicationDirPath() + QStringLiteral("/discord.json"));
    if (!file.exists()) file.setFileName(QStringLiteral(":/resources/data/discord.json"));
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.readAll()).object();
}

int testDiscord(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QFile log(QCoreApplication::applicationDirPath() + QStringLiteral("/discord-test.log"));
    if (!log.open(QIODevice::WriteOnly | QIODevice::Text)) return 3;
    const QJsonObject config = discordConfiguration();
    DiscordRichPresence presence(config.value("applicationId").toString(), config.value("largeImage").toString());
    QObject::connect(&presence, &DiscordRichPresence::statusChanged, &app, [&] {
        QTextStream stream(&log);
        stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << presence.status() << '\n';
        stream.flush();
        if (presence.activityPublished()) QTimer::singleShot(1500, &app, &QCoreApplication::quit);
    });
    if (!config.value("enabled").toBool()) return 3;
    QTimer::singleShot(12000, &app, [&app] { app.exit(2); });
    presence.start();
    return app.exec();
}
}

int main(int argc, char *argv[])
{
    if (argc > 1 && QString::fromLocal8Bit(argv[1]) == QStringLiteral("--discord-test")) {
        return testDiscord(argc, argv);
    }
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);
    // Keep the original data directory when the executable/display name changes.
    app.setApplicationName(QStringLiteral("glossata_quiz"));
    app.setApplicationDisplayName(QStringLiteral("Projeto Glossata"));
    app.setApplicationVersion(QStringLiteral(GLOSSATA_APP_VERSION));
    const bool smokeTest = app.arguments().contains(QStringLiteral("--smoke-test"));
    const int captureIndex = app.arguments().indexOf(QStringLiteral("--capture-home"));
    const bool captureHome = captureIndex >= 0;
    const bool captureDiscovery = app.arguments().contains(QStringLiteral("--capture-home-discovery"));
    if (captureDiscovery && !captureHome) return 6;
    if (captureHome && (captureIndex + 1 >= app.arguments().size() || smokeTest)) return 6;
    std::unique_ptr<QTemporaryDir> smokeDirectory;
    if (smokeTest || captureHome) {
        smokeDirectory = std::make_unique<QTemporaryDir>();
        if (!smokeDirectory->isValid()) return 4;
    }
    app.setWindowIcon(QIcon(QStringLiteral(":/resources/branding/glossata.ico")));

    const QJsonObject config = discordConfiguration();
    DiscordRichPresence presence(config.value("applicationId").toString(), config.value("largeImage").toString());
    if (!smokeTest && !captureHome && config.value("enabled").toBool()) presence.start();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &presence, &DiscordRichPresence::stop);

    GameController controller(nullptr, smokeDirectory ? smokeDirectory->filePath("smoke.sqlite") : QString());
    AppPreferences preferences(smokeDirectory ? smokeDirectory->filePath("preferences.ini") : AppPreferences::installationPath());
    AchievementAudio effects(preferences);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &preferences, &AppPreferences::flush);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &controller, &GameController::prepareToClose);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.rootContext()->setContextProperty("appPreferences", &preferences);
    engine.rootContext()->setContextProperty("achievementAudio", &effects);
    engine.load(QUrl(QStringLiteral("qrc:/resources/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (captureHome) {
        auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        if (!window) return 7;
        preferences.setMotionEnabled(false);
        const QString destination = app.arguments().at(captureIndex + 1);
        QTimer::singleShot(1200, &app, [window, destination, captureDiscovery, &app] {
            if (captureDiscovery) {
                // Frame the real Home through its existing scroll container.
                // This runs only in the isolated capture mode, never in play.
                auto scroll = window->findChild<QQuickItem *>(QStringLiteral("pageScroll"));
                auto panel = window->findChild<QQuickItem *>(QStringLiteral("dailyDiscoveryPanel"));
                if (!scroll || !panel) { app.exit(8); return; }
                const qreal top = scroll->property("contentY").toReal() + panel->mapToItem(scroll, QPointF()).y();
                const qreal maximum = qMax(0., scroll->property("contentHeight").toReal() - scroll->height());
                scroll->setProperty("contentY", qBound(0., top, maximum));
            }
            QTimer::singleShot(captureDiscovery ? 200 : 0, &app, [window, destination, &app] {
                const QImage image = window->grabWindow();
                const bool saved = window->property("page").toString() == QStringLiteral("home")
                    && !image.isNull() && image.save(destination, "PNG");
                app.exit(saved ? 0 : 8);
            });
        });
    }

    if (smokeTest) {
        QTimer::singleShot(500, &app, [&] {
            controller.startCompetitiveGame();
            engine.rootObjects().first()->setProperty("page", "quiz");
            const bool valid = !controller.gameFinished() && controller.answerOptions().size() == 4;
            QTimer::singleShot(500, &app, [&app, valid] { app.exit(valid ? 0 : 5); });
        });
    }

    return app.exec();
}
