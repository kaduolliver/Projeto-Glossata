#pragma once
#include "presentation/EffectOutput.h"

class RecordingEffectOutput final : public EffectOutput {
public:
    bool ready() const override { return ready_; }
    bool playing() const override { return playing_; }
    QString statusText() const override { return ready_ ? QString() : QStringLiteral("Saída de áudio indisponível no teste."); }
    void play(float volume) override { ++plays; lastVolume = volume; }
    void stop() override { ++stops; playing_ = false; }
    void setVolume(float volume) override { lastVolume = volume; }
    bool ready_ = true, playing_ = false;
    int plays = 0, stops = 0;
    float lastVolume = -1;
};
