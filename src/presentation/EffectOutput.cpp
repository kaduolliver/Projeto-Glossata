#include "EffectOutput.h"

#include <QMediaDevices>
#include <QAudioDevice>
#include <QSoundEffect>
#include <QUrl>

void EffectOutput::interrupt(const QString &reason)
{
    if (!playing()) return;
    interruptionReason_ = reason;
    emit interruptionRequested(reason);
    stop();
}

void EffectOutput::notifyPlaybackState()
{
    const bool current = playing(), previous = observedPlaying_;
    observedPlaying_ = current;
    if (current && !previous) interruptionReason_.clear();
    if (!current) {
        const auto reason = interruptionReason_.isEmpty()
            ? (ready() ? QStringLiteral("finished") : QStringLiteral("output-unavailable")) : interruptionReason_;
        interruptionReason_.clear();
        if (previous) emit playbackEnded(reason);
    }
    emit stateChanged();
}

bool EffectOutput::updateDevice(const QByteArray &id, const std::function<void()> &apply)
{
    if (deviceKnown_ && deviceId_ == id) return false;
    const bool changed = deviceKnown_;
    deviceKnown_ = true; deviceId_ = id;
    if (changed) interrupt(QStringLiteral("device-changed"));
    apply();
    emit stateChanged();
    return true;
}

namespace {
class SoundEffectOutput final : public EffectOutput {
public:
    SoundEffectOutput()
    {
        connect(&effect_, &QSoundEffect::statusChanged, this, &EffectOutput::stateChanged);
        connect(&effect_, &QSoundEffect::playingChanged, this, [this] { notifyPlaybackState(); });
        connect(&devices_, &QMediaDevices::audioOutputsChanged, this, [this] {
            synchronizeDevice();
        });
        synchronizeDevice();
        effect_.setLoopCount(1);
        effect_.setVolume(0.35f);
        effect_.setSource(QUrl(QStringLiteral("qrc:/resources/audio/achievement.wav")));
    }
    bool ready() const override
    {
        return !QMediaDevices::defaultAudioOutput().isNull() && effect_.status() == QSoundEffect::Ready;
    }
    bool playing() const override { return effect_.isPlaying(); }
    QString statusText() const override
    {
        if (QMediaDevices::defaultAudioOutput().isNull()) return QStringLiteral("Nenhuma saída de áudio disponível.");
        if (effect_.status() == QSoundEffect::Error) return QStringLiteral("O efeito sonoro não está disponível.");
        if (effect_.status() != QSoundEffect::Ready) return QStringLiteral("Preparando o efeito sonoro…");
        return {};
    }
    void play(float volume) override { effect_.setVolume(volume); effect_.play(); }
    void stop() override { if (effect_.isPlaying()) effect_.stop(); }
    void setVolume(float volume) override { effect_.setVolume(volume); }
private:
    void synchronizeDevice()
    {
        const auto device = QMediaDevices::defaultAudioOutput();
        updateDevice(device.id(), [this, device] { effect_.setAudioDevice(device); });
    }
    QMediaDevices devices_;
    QSoundEffect effect_;
};
}

std::unique_ptr<EffectOutput> makeSoundEffectOutput() { return std::make_unique<SoundEffectOutput>(); }
