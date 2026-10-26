#include "AchievementAudio.h"

AchievementAudio::AchievementAudio(AppPreferences &preferences, std::unique_ptr<EffectOutput> output,
                                   QObject *parent, std::function<qint64()> now)
    : QObject(parent), preferences_(preferences), output_(output ? std::move(output) : makeSoundEffectOutput()),
      now_(std::move(now))
{
    clock_.start();
    if (!now_) now_ = [this] { return clock_.elapsed(); };
    connect(output_.get(), &EffectOutput::stateChanged, this, &AchievementAudio::outputChanged);
    connect(&preferences_, &AppPreferences::achievementSoundsEnabledChanged, this, [this] {
        if (!preferences_.achievementSoundsEnabled()) output_->stop();
    });
    connect(&preferences_, &AppPreferences::effectsVolumeChanged, this, [this] {
        output_->setVolume(preferences_.effectsVolume() / 100.f);
        if (preferences_.effectsVolume() == 0) output_->stop();
    });
}

void AchievementAudio::setPresentationAllowed(bool allowed)
{
    if (allowed_ == allowed) return;
    allowed_ = allowed;
    if (!allowed) output_->stop();
    emit presentationAllowedChanged();
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
    const auto now = now_();
    if (!allowed_ || preferences_.effectsVolume() == 0 || !output_->ready() || output_->playing()
        || (hasPlayed_ && now - lastPlayed_ < 1200)) return false;
    hasPlayed_ = true; lastPlayed_ = now;
    output_->play(preferences_.effectsVolume() / 100.f);
    emit playbackRequested(reason, preferences_.effectsVolume());
    return true;
}
