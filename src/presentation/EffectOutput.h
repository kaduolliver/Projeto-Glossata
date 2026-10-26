#pragma once

#include <QObject>
#include <memory>

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
signals:
    void stateChanged();
};

std::unique_ptr<EffectOutput> makeSoundEffectOutput();
