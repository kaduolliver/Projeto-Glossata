#include "EffectOutput.h"

#include <QMediaDevices>
#include <QAudioDevice>
#include <QSoundEffect>
#include <QUrl>

namespace {
class SoundEffectOutput final : public EffectOutput {
public:
    SoundEffectOutput()
    {
        connect(&effect_, &QSoundEffect::statusChanged, this, &EffectOutput::stateChanged);
        connect(&effect_, &QSoundEffect::playingChanged, this, &EffectOutput::stateChanged);
        connect(&devices_, &QMediaDevices::audioOutputsChanged, this, [this] {
            effect_.stop();
            effect_.setAudioDevice(QMediaDevices::defaultAudioOutput());
            emit stateChanged();
        });
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
    void stop() override { effect_.stop(); }
    void setVolume(float volume) override { effect_.setVolume(volume); }
private:
    QMediaDevices devices_;
    QSoundEffect effect_;
};
}

std::unique_ptr<EffectOutput> makeSoundEffectOutput() { return std::make_unique<SoundEffectOutput>(); }
