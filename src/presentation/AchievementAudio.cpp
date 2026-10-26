#include "AchievementAudio.h"
#include <QDebug>

AchievementAudio::AchievementAudio(AppPreferences &preferences, std::unique_ptr<EffectOutput> output,
                                   QObject *parent, std::function<qint64()> now)
    : QObject(parent), preferences_(preferences), output_(output ? std::move(output) : makeSoundEffectOutput()),
      now_(std::move(now))
{
    clock_.start();
    if (!now_) now_ = [this] { return clock_.elapsed(); };
    traceEnabled_ = qEnvironmentVariable("GLOSSATA_AUDIO_TRACE") == QStringLiteral("1");
    cooldownTimer_.setSingleShot(true);
    cooldownTimer_.setTimerType(Qt::PreciseTimer);
    connect(&cooldownTimer_, &QTimer::timeout, this, &AchievementAudio::outputChanged);
    connect(output_.get(), &EffectOutput::stateChanged, this, &AchievementAudio::outputChanged);
    connect(output_.get(), &EffectOutput::interruptionRequested, this, [this](const QString &reason) {
        if (traceEnabled_) qInfo().noquote() << "GlossataAudio" << clock_.elapsed() << "stop-requested=" + reason;
    });
    connect(output_.get(), &EffectOutput::playbackEnded, this, [this](const QString &reason) {
        if (traceEnabled_) qInfo().noquote() << "GlossataAudio" << clock_.elapsed() << "ended=" + reason;
        emit playbackEnded(reason);
    });
    connect(&preferences_, &AppPreferences::achievementSoundsEnabledChanged, this, [this] {
        if (!preferences_.achievementSoundsEnabled()) output_->interrupt(QStringLiteral("sounds-disabled"));
        emit outputChanged();
    });
    connect(&preferences_, &AppPreferences::effectsVolumeChanged, this, [this] {
        output_->setVolume(preferences_.effectsVolume() / 100.f);
        if (preferences_.effectsVolume() == 0) output_->interrupt(QStringLiteral("volume-zero"));
        emit outputChanged();
    });
}

void AchievementAudio::setPresentationAllowed(bool allowed)
{
    if (allowed_ == allowed) return;
    allowed_ = allowed;
    if (!allowed) output_->interrupt(QStringLiteral("presentation-suspended"));
    emit presentationAllowedChanged();
    emit outputChanged();
}

bool AchievementAudio::coolingDown() const
{
    return hasPlayed_ && now_() - lastPlayed_ < MinimumIntervalMs;
}

bool AchievementAudio::previewBusy() const { return output_->playing() || coolingDown(); }

bool AchievementAudio::previewReady() const
{
    return allowed_ && preferences_.effectsVolume() > 0 && output_->ready() && !previewBusy();
}

bool AchievementAudio::present(const QString &achievementId)
{
    if (achievementId.trimmed().isEmpty() || consumed_.contains(achievementId)) return false;
    consumed_.insert(achievementId);
    return preferences_.achievementSoundsEnabled() && play(achievementId);
}

bool AchievementAudio::preview() { return play(QStringLiteral("preview")); }

bool AchievementAudio::play(const QString &reason)
{
    if (!previewReady()) return false;
    hasPlayed_ = true; lastPlayed_ = now_();
    cooldownTimer_.start(MinimumIntervalMs);
    output_->play(preferences_.effectsVolume() / 100.f);
    if (traceEnabled_) qInfo().noquote() << "GlossataAudio" << clock_.elapsed() << "requested=" + reason << "volume=" << preferences_.effectsVolume();
    emit playbackRequested(reason, preferences_.effectsVolume());
    emit outputChanged();
    return true;
}
