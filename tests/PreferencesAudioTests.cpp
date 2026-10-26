#include "presentation/AppPreferences.h"
#include "presentation/AchievementAudio.h"
#include "RecordingEffectOutput.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QSettings>
#include <QFile>
#include <QDir>

class PreferencesAudioTests : public QObject {
    Q_OBJECT
private slots:
    void defaultsDoNotCreateAFile();
    void changesSurviveReopeningAndAvoidRedundantWrites();
    void debounceAndShutdownFlush();
    void invalidValues_data();
    void invalidValues();
    void malformedFileIsPreserved();
    void persistenceFailureAndRetry();
    void audioEnabledDisabledVolumeAndPreview();
    void audioBurstDuplicatesSuspensionAndUnavailableOutput();
    void bundledAudioHasPcmWaveHeader();
};

void PreferencesAudioTests::defaultsDoNotCreateAFile()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    AppPreferences preferences(path);
    QVERIFY(preferences.motionEnabled()); QVERIFY(!preferences.achievementSoundsEnabled());
    QCOMPARE(preferences.effectsVolume(), 35); QVERIFY(preferences.persistenceError().isEmpty());
    QVERIFY(preferences.flush()); QVERIFY(!QFile::exists(path));
}

void PreferencesAudioTests::changesSurviveReopeningAndAvoidRedundantWrites()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { AppPreferences preferences(path);
      QSignalSpy motion(&preferences, &AppPreferences::motionEnabledChanged);
      QSignalSpy volume(&preferences, &AppPreferences::effectsVolumeChanged);
      preferences.setMotionEnabled(false); preferences.setMotionEnabled(false);
      preferences.setAchievementSoundsEnabled(true); preferences.setEffectsVolume(76);
      preferences.setEffectsVolume(76); QCOMPARE(motion.size(), 1); QCOMPARE(volume.size(), 1);
      QVERIFY(preferences.flush()); }
    AppPreferences reopened(path);
    QVERIFY(!reopened.motionEnabled()); QVERIFY(reopened.achievementSoundsEnabled());
    QCOMPARE(reopened.effectsVolume(), 76);
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto bytes = file.readAll(); file.close();
    const auto modified = QFileInfo(path).lastModified();
    reopened.setMotionEnabled(false); reopened.setEffectsVolume(76); QVERIFY(reopened.flush());
    QCOMPARE(QFileInfo(path).lastModified(), modified);
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), bytes);
    reopened.setEffectsVolume(-42); QCOMPARE(reopened.effectsVolume(), 0);
    reopened.setEffectsVolume(1000); QCOMPARE(reopened.effectsVolume(), 100);
}

void PreferencesAudioTests::debounceAndShutdownFlush()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { AppPreferences preferences(path); preferences.setEffectsVolume(45);
      QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(path), 1500);
      QCOMPARE(QSettings(path, QSettings::IniFormat).value("audio/effectsVolume").toInt(), 45);
      preferences.setEffectsVolume(67); }
    AppPreferences reopened(path); QCOMPARE(reopened.effectsVolume(), 67);
}

void PreferencesAudioTests::invalidValues_data()
{
    QTest::addColumn<QVariant>("volume");
    for (const auto &value : QVariantList{-1, 101, QString("loud"), QString("35.5"), QString("nan"), QString("999999999999")})
        QTest::newRow(qPrintable(value.toString())) << value;
}

void PreferencesAudioTests::invalidValues()
{
    QFETCH(QVariant, volume);
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { QSettings settings(path, QSettings::IniFormat);
      settings.setValue("interface/motionEnabled", "maybe"); settings.setValue("audio/achievementSoundsEnabled", "yes");
      settings.setValue("audio/effectsVolume", volume); settings.setValue("future/untouched", "keep"); settings.sync(); }
    AppPreferences preferences(path);
    QVERIFY(preferences.motionEnabled()); QVERIFY(!preferences.achievementSoundsEnabled());
    QCOMPARE(preferences.effectsVolume(), 35); QVERIFY(!preferences.persistenceError().isEmpty());
    preferences.setEffectsVolume(40); QVERIFY(preferences.flush()); QVERIFY(preferences.persistenceError().isEmpty());
    QCOMPARE(QSettings(path, QSettings::IniFormat).value("future/untouched").toString(), QString("keep"));
    AppPreferences reopened(path); QCOMPARE(reopened.effectsVolume(), 40); QVERIFY(reopened.persistenceError().isEmpty());
}

void PreferencesAudioTests::malformedFileIsPreserved()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    const QByteArray malformed("[broken\nkey=value\n");
    { QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(malformed); }
    AppPreferences preferences(path); QVERIFY(!preferences.persistenceError().isEmpty());
    preferences.setEffectsVolume(20); QVERIFY(!preferences.flush());
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), malformed);
}

void PreferencesAudioTests::persistenceFailureAndRetry()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    QVERIFY(QDir().mkdir(path)); // Deterministic failure, independent of OS permissions/root.
    AppPreferences preferences(path); QVERIFY(!preferences.persistenceError().isEmpty());
    preferences.setMotionEnabled(false); preferences.setEffectsVolume(64);
    QVERIFY(!preferences.flush()); QVERIFY(!preferences.motionEnabled()); QCOMPARE(preferences.effectsVolume(), 64);
    QVERIFY(QDir().rename(path, dir.filePath("preserved-blocking-directory")));
    QVERIFY(preferences.flush()); QVERIFY(preferences.persistenceError().isEmpty());
    AppPreferences reopened(path); QVERIFY(!reopened.motionEnabled()); QCOMPARE(reopened.effectsVolume(), 64);
}

void PreferencesAudioTests::audioEnabledDisabledVolumeAndPreview()
{
    QTemporaryDir dir; AppPreferences preferences(dir.filePath("preferences.ini"));
    auto output = std::make_unique<RecordingEffectOutput>(); auto device = output.get();
    qint64 now = 0; AchievementAudio audio(preferences, std::move(output), nullptr, [&] { return now; });
    QSignalSpy requested(&audio, &AchievementAudio::playbackRequested);
    audio.setPresentationAllowed(true);
    QVERIFY(!audio.present("muted")); QCOMPARE(device->plays, 0);
    QVERIFY(audio.preview()); QCOMPARE(device->plays, 1); QCOMPARE(device->lastVolume, 0.35f);
    QCOMPARE(requested.first()[0].toString(), QString("preview"));
    preferences.setAchievementSoundsEnabled(true); now += 1200;
    QVERIFY(!audio.present("muted")); // Enabling never replays a consumed event.
    preferences.setEffectsVolume(0); QVERIFY(!audio.present("zero")); QVERIFY(!audio.preview());
    preferences.setEffectsVolume(100); QVERIFY(audio.present("maximum")); QCOMPARE(device->lastVolume, 1.f);
    preferences.setEffectsVolume(19); QCOMPARE(device->lastVolume, 0.19f);
    preferences.setAchievementSoundsEnabled(false); QVERIFY(device->stops > 0);
    now += 1200; QVERIFY(audio.preview()); QCOMPARE(device->lastVolume, 0.19f);
}

void PreferencesAudioTests::audioBurstDuplicatesSuspensionAndUnavailableOutput()
{
    QTemporaryDir dir; AppPreferences preferences(dir.filePath("preferences.ini")); preferences.setAchievementSoundsEnabled(true);
    auto output = std::make_unique<RecordingEffectOutput>(); auto device = output.get();
    qint64 now = 0; AchievementAudio audio(preferences, std::move(output), nullptr, [&] { return now; });
    audio.setPresentationAllowed(true);
    QVERIFY(audio.present("first")); QVERIFY(!audio.present("second")); QVERIFY(!audio.present("third"));
    QCOMPARE(device->plays, 1); now = 1199; QVERIFY(!audio.preview());
    now = 1200; QVERIFY(!audio.present("first")); QVERIFY(!audio.present("second"));
    QVERIFY(audio.present("fourth")); QCOMPARE(device->plays, 2);
    now += 1200; device->playing_ = true; QVERIFY(!audio.present("busy"));
    audio.setPresentationAllowed(false); QVERIFY(!device->playing_); QVERIFY(!audio.present("suspended")); QVERIFY(!audio.preview());
    audio.setPresentationAllowed(true); QVERIFY(!audio.present("suspended")); QCOMPARE(device->plays, 2);
    device->ready_ = false; QVERIFY(!audio.present("unavailable")); QVERIFY(!audio.preview());
    device->ready_ = true; QVERIFY(!audio.present("unavailable"));
    QVERIFY(audio.present("fresh")); QCOMPARE(device->plays, 3);
    QVERIFY(!audio.present(""));
}

void PreferencesAudioTests::bundledAudioHasPcmWaveHeader()
{
    QFile file(":/resources/audio/achievement.wav"); QVERIFY(file.open(QIODevice::ReadOnly));
    const auto data = file.readAll(); QCOMPARE(data.first(4), QByteArray("RIFF"));
    QCOMPARE(data.mid(8, 4), QByteArray("WAVE")); QCOMPARE(data.size(), 75014);
}

QTEST_GUILESS_MAIN(PreferencesAudioTests)
#include "PreferencesAudioTests.moc"
