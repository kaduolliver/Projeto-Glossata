#include "presentation/GameController.h"
#include "infrastructure/discord/DiscordRichPresence.h"

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
    const bool smokeTest = app.arguments().contains(QStringLiteral("--smoke-test"));
    std::unique_ptr<QTemporaryDir> smokeDirectory;
    if (smokeTest) {
        smokeDirectory = std::make_unique<QTemporaryDir>();
        if (!smokeDirectory->isValid()) return 4;
    }
    app.setWindowIcon(QIcon(QStringLiteral(":/resources/branding/glossata.ico")));

    const QJsonObject config = discordConfiguration();
    DiscordRichPresence presence(config.value("applicationId").toString(), config.value("largeImage").toString());
    if (!smokeTest && config.value("enabled").toBool()) presence.start();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &presence, &DiscordRichPresence::stop);

    GameController controller(nullptr, smokeTest ? smokeDirectory->filePath("smoke.sqlite") : QString());
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.rootContext()->setContextProperty("discordPresence", &presence);
    engine.load(QUrl(QStringLiteral("qrc:/resources/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
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
