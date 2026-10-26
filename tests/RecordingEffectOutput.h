#pragma once
#include "presentation/EffectOutput.h"

class RecordingEffectOutput final : public EffectOutput {
public:
    bool ready() const override { return ready_; }
    bool playing() const override { return playing_; }
    QString statusText() const override { return ready_ ? QString() : QStringLiteral("Saída de áudio indisponível no teste."); }
    void play(float volume) override {
        ++plays; lastVolume = volume;
        if (simulatePlayback) { playing_ = true; notifyPlaybackState(); }
    }
    void stop() override { ++stops; playing_ = false; notifyPlaybackState(); }
    void setVolume(float volume) override { lastVolume = volume; }
    void finishNaturally() { playing_ = false; notifyPlaybackState(); }
    bool selectDevice(const QByteArray &id) { return updateDevice(id, [this] { ++deviceApplications; }); }
    bool simulatePlayback = false;
    bool ready_ = true, playing_ = false;
    int plays = 0, stops = 0;
    int deviceApplications = 0;
    float lastVolume = -1;
};
