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
    Q_PROPERTY(bool playing READ playing NOTIFY outputChanged)
    Q_PROPERTY(bool previewBusy READ previewBusy NOTIFY outputChanged)
    Q_PROPERTY(bool previewReady READ previewReady NOTIFY outputChanged)
public:
    // 1.20-second resource plus a 100 ms guard; actual playing also prevents overlap.
    static constexpr int MinimumIntervalMs = 1300;
    explicit AchievementAudio(AppPreferences &preferences, std::unique_ptr<EffectOutput> output = {},
                              QObject *parent = nullptr, std::function<qint64()> now = {});
    bool presentationAllowed() const { return allowed_; }
    void setPresentationAllowed(bool allowed);
    bool available() const { return output_->ready(); }
    QString statusText() const { return output_->statusText(); }
    bool playing() const { return output_->playing(); }
    bool previewBusy() const;
    bool previewReady() const;
    Q_INVOKABLE bool present(const QString &achievementId);
    Q_INVOKABLE bool preview();
signals:
    void presentationAllowedChanged();
    void outputChanged();
    // A request to the output, not a claim of physical audibility.
    void playbackRequested(const QString &reason, int volume);
    void playbackEnded(const QString &reason);
private:
    bool play(const QString &reason);
    bool coolingDown() const;
    AppPreferences &preferences_;
    std::unique_ptr<EffectOutput> output_;
    QElapsedTimer clock_;
    std::function<qint64()> now_;
    QSet<QString> consumed_;
    bool allowed_ = false, hasPlayed_ = false;
    qint64 lastPlayed_ = 0;
    QTimer cooldownTimer_;
    bool traceEnabled_ = false;
};
