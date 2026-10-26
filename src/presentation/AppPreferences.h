#pragma once

#include <QObject>
#include <QTimer>

// Application preferences never read or write player databases.
// Callers must provide a path: tests cannot accidentally use the real installation.
class AppPreferences : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool motionEnabled READ motionEnabled WRITE setMotionEnabled NOTIFY motionEnabledChanged)
    Q_PROPERTY(bool achievementSoundsEnabled READ achievementSoundsEnabled WRITE setAchievementSoundsEnabled NOTIFY achievementSoundsEnabledChanged)
    Q_PROPERTY(int effectsVolume READ effectsVolume WRITE setEffectsVolume NOTIFY effectsVolumeChanged)
    Q_PROPERTY(QString persistenceError READ persistenceError NOTIFY persistenceErrorChanged)
public:
    explicit AppPreferences(const QString &filePath, QObject *parent = nullptr);
    ~AppPreferences() override;
    static QString installationPath();
    bool motionEnabled() const { return motion_; }
    bool achievementSoundsEnabled() const { return sounds_; }
    int effectsVolume() const { return volume_; }
    QString persistenceError() const { return error_; }
    void setMotionEnabled(bool enabled);
    void setAchievementSoundsEnabled(bool enabled);
    void setEffectsVolume(int volume);
    Q_INVOKABLE bool flush();
signals:
    void motionEnabledChanged();
    void achievementSoundsEnabledChanged();
    void effectsVolumeChanged();
    void persistenceErrorChanged();
private:
    void scheduleSave();
    void setError(const QString &message);
    QString path_, error_;
    bool motion_ = true, sounds_ = false, dirty_ = false, malformed_ = false;
    int volume_ = 35;
    QTimer saveTimer_;
};
