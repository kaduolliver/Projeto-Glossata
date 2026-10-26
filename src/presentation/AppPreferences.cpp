#include "AppPreferences.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

namespace {
bool readBoolean(const QVariant &value, bool fallback, bool &invalid)
{
    const auto text = value.toString().trimmed().toLower();
    if (text == "true" || text == "1") return true;
    if (text == "false" || text == "0") return false;
    invalid = true;
    return fallback;
}
}

QString AppPreferences::installationPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + QStringLiteral("/preferences.ini");
}

AppPreferences::AppPreferences(const QString &filePath, QObject *parent)
    : QObject(parent), path_(filePath)
{
    saveTimer_.setSingleShot(true);
    saveTimer_.setInterval(250);
    connect(&saveTimer_, &QTimer::timeout, this, &AppPreferences::flush);
    if (path_.isEmpty() || QFileInfo(path_).isDir()) {
        setError(QStringLiteral("Não foi possível ler as preferências. Usando os valores padrão."));
        return;
    }
    QSettings settings(path_, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    bool invalid = false;
    motion_ = readBoolean(settings.value("interface/motionEnabled", true), true, invalid);
    sounds_ = readBoolean(settings.value("audio/achievementSoundsEnabled", false), false, invalid);
    const auto text = settings.value("audio/effectsVolume", 35).toString();
    bool number = false;
    const auto volume = text.toInt(&number);
    if (number && QRegularExpression(QStringLiteral("^[0-9]+$")).match(text).hasMatch()
        && volume >= 0 && volume <= 100) volume_ = volume;
    else invalid = true;
    if (settings.status() != QSettings::NoError) {
        motion_ = true; sounds_ = false; volume_ = 35;
        if (settings.status() == QSettings::FormatError) preserveMalformedFile();
        else setError(QStringLiteral("Não foi possível ler as preferências. Usando os valores padrão; o arquivo foi preservado."));
    } else if (invalid) {
        recoveryPending_ = true;
        setError(QStringLiteral("Algumas preferências eram inválidas e usam valores padrão temporários. Use Tentar salvar preferências para guardar as correções."));
    }
}

AppPreferences::~AppPreferences() { if (dirty_) flush(); }

void AppPreferences::setError(const QString &message)
{
    if (error_ == message) return;
    error_ = message;
    emit persistenceErrorChanged();
}

void AppPreferences::scheduleSave()
{
    const bool couldRetry = canRetryPersistence();
    dirty_ = true;
    // Recovering invalid values requires the explicit save action, including
    // when another preference is changed while that recovery is pending.
    if (!recoveryPending_ && !malformed_) saveTimer_.start();
    if (couldRetry != canRetryPersistence()) emit persistenceStateChanged();
}

void AppPreferences::preserveMalformedFile()
{
    malformed_ = true;
    recoveryPending_ = false;
    setError(QStringLiteral("O arquivo de preferências está danificado e foi preservado. É necessário corrigir o arquivo fora do aplicativo e reabrir o Glossata."));
    emit persistenceStateChanged();
}

void AppPreferences::setMotionEnabled(bool enabled)
{
    if (motion_ == enabled) return;
    motion_ = enabled; scheduleSave(); emit motionEnabledChanged();
}

void AppPreferences::setAchievementSoundsEnabled(bool enabled)
{
    if (sounds_ == enabled) return;
    sounds_ = enabled; scheduleSave(); emit achievementSoundsEnabledChanged();
}

void AppPreferences::setEffectsVolume(int volume)
{
    volume = qBound(0, volume, 100);
    if (volume_ == volume) return;
    volume_ = volume; scheduleSave(); emit effectsVolumeChanged();
}

bool AppPreferences::flush()
{
    return save(false);
}

bool AppPreferences::retrySave()
{
    return save(true);
}

bool AppPreferences::save(bool allowRecovery)
{
    saveTimer_.stop();
    // Qt shares parsed INI data between instances; a second instance may no
    // longer report the original FormatError. Keep that failure latched.
    if (malformed_) return false;
    if (recoveryPending_ && !allowRecovery) return false;
    if (!dirty_ && !recoveryPending_) return error_.isEmpty();
    const auto failed = [this] {
        setError(QStringLiteral("Não foi possível salvar as preferências. As alterações valem nesta execução; tente salvar novamente."));
        return false;
    };
    if (path_.isEmpty() || !QDir().mkpath(QFileInfo(path_).absolutePath())) return failed();
    QSettings settings(path_, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    // Force parsing before writing. Never rewrite a malformed settings file.
    settings.allKeys();
    if (settings.status() == QSettings::FormatError) {
        preserveMalformedFile();
        return false;
    }
    if (settings.status() != QSettings::NoError || !settings.isWritable()) return failed();
    settings.setValue("interface/motionEnabled", motion_);
    settings.setValue("audio/achievementSoundsEnabled", sounds_);
    settings.setValue("audio/effectsVolume", volume_);
    settings.sync(); // Atomic INI replacement remains required (Qt default).
    if (settings.status() != QSettings::NoError) return failed();
    dirty_ = false;
    recoveryPending_ = false;
    emit persistenceStateChanged();
    setError({});
    return true;
}
