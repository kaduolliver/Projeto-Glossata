#pragma once

#include "AppPreferences.h"
#include "EffectOutput.h"
#include <QElapsedTimer>
#include <QSet>
#include <functional>

class AchievementAudio : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool presentationAllowed READ presentationAllowed WRITE setPresentationAllowed NOTIFY presentationAllowedChanged)
    Q_PROPERTY(bool available READ available NOTIFY outputChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY outputChanged)
public:
    explicit AchievementAudio(AppPreferences &preferences, std::unique_ptr<EffectOutput> output = {},
                              QObject *parent = nullptr, std::function<qint64()> now = {});
    bool presentationAllowed() const { return allowed_; }
    void setPresentationAllowed(bool allowed);
    bool available() const { return output_->ready(); }
    QString statusText() const { return output_->statusText(); }
    Q_INVOKABLE bool present(const QString &achievementId);
    Q_INVOKABLE bool preview();
signals:
    void presentationAllowedChanged();
    void outputChanged();
    // A request to the output, not a claim of physical audibility.
    void playbackRequested(const QString &reason, int volume);
private:
    bool play(const QString &reason);
    AppPreferences &preferences_;
    std::unique_ptr<EffectOutput> output_;
    QElapsedTimer clock_;
    std::function<qint64()> now_;
    QSet<QString> consumed_;
    bool allowed_ = false, hasPlayed_ = false;
    qint64 lastPlayed_ = 0;
};
