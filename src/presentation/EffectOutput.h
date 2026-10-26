#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>
#include <functional>

// Injectable boundary: policy/UI tests do not create an audio device.
class EffectOutput : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual bool ready() const = 0;
    virtual bool playing() const = 0;
    virtual QString statusText() const = 0;
    virtual void play(float volume) = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
    // Stop only an active playback, retaining the reason before Qt emits its
    // playingChanged signal. Idle refreshes must not masquerade as cutoffs.
    void interrupt(const QString &reason);
signals:
    void stateChanged();
    void playbackEnded(const QString &reason);
    void interruptionRequested(const QString &reason);
protected:
    void notifyPlaybackState();
    bool updateDevice(const QByteArray &id, const std::function<void()> &apply);
private:
    bool observedPlaying_ = false, deviceKnown_ = false;
    QByteArray deviceId_;
    QString interruptionReason_;
};

std::unique_ptr<EffectOutput> makeSoundEffectOutput();
