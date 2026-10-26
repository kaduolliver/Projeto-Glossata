#pragma once
#include "presentation/AppPreferences.h"
#include "presentation/AchievementAudio.h"
#include "RecordingEffectOutput.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTemporaryDir>

class TestAppServices final : public QObject {
public:
    explicit TestAppServices(QObject *parent)
        : QObject(parent), preferences(directory.filePath("preferences.ini")),
          audio(preferences, std::make_unique<RecordingEffectOutput>()) {}
    QTemporaryDir directory;
    AppPreferences preferences;
    AchievementAudio audio;
};

inline void installTestAppServices(QQmlApplicationEngine &engine)
{
    if (engine.rootContext()->contextProperty("appPreferences").isValid()) return;
    auto services = new TestAppServices(&engine);
    engine.rootContext()->setContextProperty("appPreferences", &services->preferences);
    engine.rootContext()->setContextProperty("achievementAudio", &services->audio);
}

inline void setMotionEnabled(QQuickWindow *window, bool enabled)
{
    window->property("preferences").value<QObject *>()->setProperty("motionEnabled", enabled);
}
