#pragma once

#include "application/GameService.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include "infrastructure/persistence/SqliteGameHistoryRepository.h"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QMap>

class GameController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString questionText READ questionText NOTIFY questionTextChanged)
    Q_PROPERTY(QString factText READ factText NOTIFY factTextChanged)
    Q_PROPERTY(QString extraHint READ extraHint NOTIFY extraHintChanged)
    Q_PROPERTY(bool hintRevealed READ hintRevealed NOTIFY extraHintChanged)
    Q_PROPERTY(QStringList answerOptions READ answerOptions NOTIFY answerOptionsChanged)
    Q_PROPERTY(QStringList playerProfiles READ playerProfiles NOTIFY playerProfilesChanged)
    Q_PROPERTY(QString currentPlayerName READ currentPlayerName WRITE setCurrentPlayerName NOTIFY currentPlayerNameChanged)
    Q_PROPERTY(QString sessionPlayerName READ sessionPlayerName NOTIFY gameFinishedChanged)
    Q_PROPERTY(bool resultPending READ resultPending NOTIFY resultPendingChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int round READ round NOTIFY roundChanged)
    Q_PROPERTY(bool gameFinished READ gameFinished NOTIFY gameFinishedChanged)
    Q_PROPERTY(QString feedbackText READ feedbackText NOTIFY feedbackTextChanged)
    Q_PROPERTY(QVariantList ranking READ ranking NOTIFY rankingChanged)
    Q_PROPERTY(int answeredCount READ answeredCount NOTIFY scoreChanged)
    Q_PROPERTY(int catalogSize READ catalogSize NOTIFY gameFinishedChanged)
    Q_PROPERTY(bool answerPending READ answerPending NOTIFY answerDetailsChanged)
    Q_PROPERTY(QVariantMap answerDetails READ answerDetails NOTIFY answerDetailsChanged)
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)
    Q_PROPERTY(QVariantList categoryStats READ categoryStats NOTIFY scoreChanged)
    Q_PROPERTY(QVariantList achievements READ achievements NOTIFY scoreChanged)
    Q_PROPERTY(QString categoryName READ categoryName NOTIFY gameFinishedChanged)

public:
    explicit GameController(QObject *parent = nullptr, const QString &databasePath = QString());

    QString questionText() const;
    QString factText() const;
    QString extraHint() const;
    bool hintRevealed() const;
    QStringList answerOptions() const;
    QStringList playerProfiles() const;
    QString currentPlayerName() const;
    QString sessionPlayerName() const { return sessionPlayerName_; }
    bool resultPending() const { return resultPending_; }
    void setCurrentPlayerName(const QString &name);
    int score() const;
    int round() const;
    bool gameFinished() const;
    QString feedbackText() const;
    QVariantList ranking() const;
    int answeredCount() const { return service_.answeredCount(); }
    int catalogSize() const { return service_.catalogSize(); }
    bool answerPending() const { return answerPending_; }
    QVariantMap answerDetails() const { return answerDetails_; }
    QVariantList categories() const;
    QVariantList categoryStats() const;
    QVariantList achievements() const;
    QString categoryName() const { return categoryName_; }

public slots:
    void startGame();
    void startCategoryGame(const QString &family);
    void nextQuestion();
    void answerSelected(const QString &selectedOption);
    void revealHint();
    void stopGame();
    void retrySaveResult();
    bool addPlayerProfile(const QString &name);
    bool renameCurrentPlayerProfile(const QString &name);
    bool deleteCurrentPlayerProfile();
    void loadRanking();

signals:
    void questionTextChanged();
    void factTextChanged();
    void extraHintChanged();
    void answerOptionsChanged();
    void playerProfilesChanged();
    void currentPlayerNameChanged();
    void scoreChanged();
    void roundChanged();
    void gameFinishedChanged();
    void feedbackTextChanged();
    void rankingChanged();
    void resultPendingChanged();
    void answerDetailsChanged();

private:
    JsonSpeciesCatalog catalog_;
    SqliteGameHistoryRepository historyRepository_;
    GameService service_;
    QString currentPlayerName_;
    QString sessionPlayerName_;
    bool resultPending_ = false;
    QString feedbackText_;
    QString categoryName_;
    bool answerPending_ = false;
    QVariantMap answerDetails_;
    QMap<QString, QPair<int, int>> categoryStats_;
    int correctStreak_ = 0;
    int bestStreak_ = 0;
};
