#include "presentation/AppPreferences.h"
#include "presentation/AchievementAudio.h"
#include "RecordingEffectOutput.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QSettings>
#include <QFile>
#include <QDir>

namespace {
QByteArray fileContents(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}
}

class PreferencesAudioTests : public QObject {
    Q_OBJECT
private slots:
    void defaultsDoNotCreateAFile();
    void validFileRemainsUntouched();
    void changesSurviveReopeningAndAvoidRedundantWrites();
    void debounceAndShutdownFlush();
    void invalidValues_data();
    void invalidValues();
    void pendingRecoveryNeverAutosaves();
    void recoveryWriteFailureAndRetry();
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
    QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.canRetryPersistence());
    QVERIFY(preferences.flush()); QVERIFY(preferences.retrySave()); QVERIFY(!QFile::exists(path));
}

void PreferencesAudioTests::validFileRemainsUntouched()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { QSettings settings(path, QSettings::IniFormat);
      settings.setValue("interface/motionEnabled", false); settings.setValue("audio/achievementSoundsEnabled", true);
      settings.setValue("audio/effectsVolume", 72); settings.setValue("future/untouched", "keep"); settings.sync();
      QCOMPARE(settings.status(), QSettings::NoError); }
    { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite));
      QVERIFY(file.setFileTime(QDateTime::currentDateTimeUtc().addSecs(-120), QFileDevice::FileModificationTime)); }
    const auto bytes = fileContents(path); const auto modified = QFileInfo(path).lastModified(); QVERIFY(!bytes.isEmpty());
    { AppPreferences preferences(path);
      QVERIFY(!preferences.motionEnabled()); QVERIFY(preferences.achievementSoundsEnabled()); QCOMPARE(preferences.effectsVolume(), 72);
      QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.canRetryPersistence());
      QVERIFY(preferences.persistenceError().isEmpty()); QTest::qWait(300);
      QVERIFY(preferences.flush()); QVERIFY(preferences.retrySave());
      preferences.setMotionEnabled(false); preferences.setAchievementSoundsEnabled(true); preferences.setEffectsVolume(72);
      QVERIFY(preferences.flush()); }
    QCOMPARE(fileContents(path), bytes); QCOMPARE(QFileInfo(path).lastModified(), modified);
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
    QTest::addColumn<QVariant>("motion");
    QTest::addColumn<QVariant>("sounds");
    QTest::addColumn<QVariant>("volume");
    QTest::addColumn<bool>("expectedMotion");
    QTest::addColumn<bool>("expectedSounds");
    QTest::addColumn<int>("expectedVolume");
    for (const auto &value : QVariantList{-1, 101, QString("loud"), QString("35.5"), QString("nan"), QString("999999999999")})
        QTest::newRow(qPrintable(value.toString())) << QVariant(false) << QVariant(true) << value << false << true << 35;
    QTest::newRow("invalid-motion") << QVariant("maybe") << QVariant(true) << QVariant(72) << true << true << 72;
    QTest::newRow("invalid-sounds") << QVariant(false) << QVariant("yes") << QVariant(72) << false << false << 72;
    QTest::newRow("multiple-invalid-values") << QVariant("maybe") << QVariant("yes") << QVariant("loud") << true << false << 35;
}

void PreferencesAudioTests::invalidValues()
{
    QFETCH(QVariant, motion); QFETCH(QVariant, sounds); QFETCH(QVariant, volume);
    QFETCH(bool, expectedMotion); QFETCH(bool, expectedSounds); QFETCH(int, expectedVolume);
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { QSettings settings(path, QSettings::IniFormat);
      settings.setValue("interface/motionEnabled", motion); settings.setValue("audio/achievementSoundsEnabled", sounds);
      settings.setValue("audio/effectsVolume", volume); settings.setValue("future/untouched", "keep");
      settings.setValue("future/effectsVolume", 900); settings.sync(); QCOMPARE(settings.status(), QSettings::NoError); }
    const auto original = fileContents(path); const auto modified = QFileInfo(path).lastModified(); QVERIFY(!original.isEmpty());
    { AppPreferences preferences(path);
      QCOMPARE(preferences.motionEnabled(), expectedMotion); QCOMPARE(preferences.achievementSoundsEnabled(), expectedSounds);
      QCOMPARE(preferences.effectsVolume(), expectedVolume); QVERIFY(!preferences.persistenceError().isEmpty());
      QVERIFY(preferences.recoveryPending()); QVERIFY(preferences.canRetryPersistence());
      QTest::qWait(300); QVERIFY(!preferences.flush());
      QCOMPARE(fileContents(path), original); QCOMPARE(QFileInfo(path).lastModified(), modified); }
    QCOMPARE(fileContents(path), original); // Closing without explicit recovery preserves the original.
    AppPreferences preferences(path); QVERIFY(preferences.recoveryPending());
    QVERIFY(preferences.retrySave()); // No preference setter or manual edit is needed.
    QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.canRetryPersistence()); QVERIFY(preferences.persistenceError().isEmpty());
    QCOMPARE(preferences.motionEnabled(), expectedMotion); QCOMPARE(preferences.achievementSoundsEnabled(), expectedSounds);
    QCOMPARE(preferences.effectsVolume(), expectedVolume);
    QCOMPARE(QSettings(path, QSettings::IniFormat).value("future/untouched").toString(), QString("keep"));
    QCOMPARE(QSettings(path, QSettings::IniFormat).value("future/effectsVolume").toInt(), 900);
    { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite));
      QVERIFY(file.setFileTime(QDateTime::currentDateTimeUtc().addSecs(-120), QFileDevice::FileModificationTime)); }
    const auto corrected = fileContents(path); const auto savedAt = QFileInfo(path).lastModified();
    QVERIFY(preferences.retrySave()); QVERIFY(preferences.flush());
    QCOMPARE(fileContents(path), corrected); QCOMPARE(QFileInfo(path).lastModified(), savedAt);
    AppPreferences reopened(path);
    QCOMPARE(reopened.motionEnabled(), expectedMotion); QCOMPARE(reopened.achievementSoundsEnabled(), expectedSounds);
    QCOMPARE(reopened.effectsVolume(), expectedVolume); QVERIFY(reopened.persistenceError().isEmpty()); QVERIFY(!reopened.recoveryPending());
}

void PreferencesAudioTests::pendingRecoveryNeverAutosaves()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    { QSettings settings(path, QSettings::IniFormat);
      settings.setValue("interface/motionEnabled", false); settings.setValue("audio/effectsVolume", -1); settings.sync(); }
    const auto original = fileContents(path);
    { AppPreferences preferences(path); preferences.setMotionEnabled(true);
      QVERIFY(preferences.recoveryPending()); QTest::qWait(350); QVERIFY(!preferences.flush());
      QCOMPARE(fileContents(path), original); }
    QCOMPARE(fileContents(path), original); // Shutdown flush cannot silently repair the file either.
    AppPreferences preferences(path); QVERIFY(!preferences.motionEnabled());
    preferences.setMotionEnabled(true); QVERIFY(preferences.retrySave());
    AppPreferences reopened(path); QVERIFY(reopened.motionEnabled()); QCOMPARE(reopened.effectsVolume(), 35);
}

void PreferencesAudioTests::recoveryWriteFailureAndRetry()
{
    QTemporaryDir dir; const auto folder = dir.filePath("settings"); QVERIFY(QDir().mkdir(folder));
    const auto path = folder + "/preferences.ini";
    { QSettings settings(path, QSettings::IniFormat);
      settings.setValue("interface/motionEnabled", false); settings.setValue("audio/effectsVolume", -1);
      settings.setValue("future/untouched", "keep"); settings.sync(); }
    AppPreferences preferences(path); QVERIFY(preferences.recoveryPending());
    const auto backup = dir.filePath("preserved-settings"); QVERIFY(QDir().rename(folder, backup));
    { QFile blocker(folder); QVERIFY(blocker.open(QIODevice::WriteOnly)); blocker.write("temporary write blocker"); }
    QVERIFY(!preferences.retrySave()); QVERIFY(preferences.recoveryPending()); QVERIFY(preferences.canRetryPersistence());
    QVERIFY(preferences.persistenceError().contains(QStringLiteral("Não foi possível salvar")));
    QVERIFY(QFile::rename(folder, dir.filePath("preserved-write-blocker"))); QVERIFY(QDir().rename(backup, folder));
    QVERIFY(preferences.retrySave()); QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.canRetryPersistence());
    AppPreferences reopened(path); QVERIFY(!reopened.motionEnabled()); QCOMPARE(reopened.effectsVolume(), 35);
    QVERIFY(reopened.persistenceError().isEmpty());
    QCOMPARE(QSettings(path, QSettings::IniFormat).value("future/untouched").toString(), QString("keep"));
}

void PreferencesAudioTests::malformedFileIsPreserved()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    const QByteArray malformed("[broken\nkey=value\n");
    { QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(malformed); }
    { AppPreferences preferences(path); QVERIFY(!preferences.persistenceError().isEmpty());
      QVERIFY(preferences.persistenceError().contains(QStringLiteral("fora do aplicativo")));
      QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.canRetryPersistence());
      QVERIFY(!preferences.retrySave()); preferences.setEffectsVolume(20);
      QVERIFY(!preferences.canRetryPersistence()); QVERIFY(!preferences.flush()); QTest::qWait(300);
      QCOMPARE(fileContents(path), malformed); }
    QCOMPARE(fileContents(path), malformed);
}

void PreferencesAudioTests::persistenceFailureAndRetry()
{
    QTemporaryDir dir; const auto path = dir.filePath("preferences.ini");
    QVERIFY(QDir().mkdir(path)); // Deterministic failure, independent of OS permissions/root.
    AppPreferences preferences(path); QVERIFY(!preferences.persistenceError().isEmpty());
    preferences.setMotionEnabled(false); preferences.setEffectsVolume(64);
    QVERIFY(!preferences.flush()); QVERIFY(!preferences.retrySave()); QVERIFY(preferences.canRetryPersistence());
    QVERIFY(!preferences.recoveryPending()); QVERIFY(!preferences.motionEnabled()); QCOMPARE(preferences.effectsVolume(), 64);
    QVERIFY(QDir().rename(path, dir.filePath("preserved-blocking-directory")));
    QVERIFY(preferences.retrySave()); QVERIFY(preferences.persistenceError().isEmpty()); QVERIFY(!preferences.canRetryPersistence());
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
