#pragma once

#include "application/GameService.h"
#include "infrastructure/content/SqliteSpeciesCatalog.h"
#include "application/TaxonomyIndex.h"
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
    Q_PROPERTY(qint64 scoreUnits READ scoreUnits NOTIFY scoreChanged)
    Q_PROPERTY(QString scoreText READ scoreText NOTIFY scoreChanged)
    Q_PROPERTY(int correctAnswers READ correctAnswers NOTIFY scoreChanged)
    Q_PROPERTY(QVariantList hints READ hints NOTIFY extraHintChanged)
    Q_PROPERTY(QString possibleScoreText READ possibleScoreText NOTIFY extraHintChanged)
    Q_PROPERTY(int round READ round NOTIFY roundChanged)
    Q_PROPERTY(bool gameFinished READ gameFinished NOTIFY gameFinishedChanged)
    Q_PROPERTY(QString feedbackText READ feedbackText NOTIFY feedbackTextChanged)
    Q_PROPERTY(QVariantList ranking READ ranking NOTIFY rankingChanged)
    Q_PROPERTY(QString rankingMode READ rankingMode WRITE setRankingMode NOTIFY rankingChanged)
    Q_PROPERTY(QVariantMap personalRecord READ personalRecord NOTIFY rankingChanged)
    Q_PROPERTY(QVariantMap sessionMetrics READ sessionMetrics NOTIFY scoreChanged)
    Q_PROPERTY(QVariantMap playerMetrics READ playerMetrics NOTIFY rankingChanged)
    Q_PROPERTY(QString gameMode READ gameMode NOTIFY gameFinishedChanged)
    Q_PROPERTY(int questionLimit READ questionLimit NOTIFY gameFinishedChanged)
    Q_PROPERTY(int competitiveQuestionCount READ competitiveQuestionCount CONSTANT)
    Q_PROPERTY(QString difficulty READ difficulty WRITE setDifficulty NOTIFY difficultyChanged)
    Q_PROPERTY(int answeredCount READ answeredCount NOTIFY scoreChanged)
    Q_PROPERTY(int catalogSize READ catalogSize NOTIFY gameFinishedChanged)
    Q_PROPERTY(bool answerPending READ answerPending NOTIFY answerDetailsChanged)
    Q_PROPERTY(QVariantMap answerDetails READ answerDetails NOTIFY answerDetailsChanged)
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)
    Q_PROPERTY(QVariantList categoryStats READ categoryStats NOTIFY scoreChanged)
    Q_PROPERTY(QVariantList achievements READ achievements NOTIFY scoreChanged)
    Q_PROPERTY(QString categoryName READ categoryName NOTIFY gameFinishedChanged)
    Q_PROPERTY(QVariantMap browseTaxon READ browseTaxon NOTIFY explorationChanged)
    Q_PROPERTY(QVariantList browseChildren READ browseChildren NOTIFY explorationChanged)
    Q_PROPERTY(QVariantList breadcrumbs READ breadcrumbs NOTIFY explorationChanged)
    Q_PROPERTY(QVariantList identificationLevels READ identificationLevels NOTIFY explorationChanged)
    Q_PROPERTY(QVariantMap questionPhoto READ questionPhoto NOTIFY questionTextChanged)
    Q_PROPERTY(QVariantMap legalInformation READ legalInformation CONSTANT)
    Q_PROPERTY(QString identificationRankLabel READ identificationRankLabel NOTIFY gameFinishedChanged)
    Q_PROPERTY(QString sessionTaxonId READ sessionTaxonId NOTIFY gameFinishedChanged)
    Q_PROPERTY(bool hasHint READ hasHint NOTIFY extraHintChanged)
    Q_PROPERTY(QString hintSourceUrl READ hintSourceUrl NOTIFY extraHintChanged)
    Q_PROPERTY(QVariantList taxonomicStats READ categoryStats NOTIFY scoreChanged)

public:
    QVariantMap legalInformation() const;
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
    qint64 scoreUnits() const;
    QString scoreText() const { return formatScore(scoreUnits()); }
    int correctAnswers() const { return service_.correctAnswers(); }
    QVariantList hints() const;
    QString possibleScoreText() const { return formatScore(maximumScoreUnits(displayedRevealedHints_)); }
    int round() const;
    bool gameFinished() const;
    QString feedbackText() const;
    QVariantList ranking() const;
    QString rankingMode() const { return rankingMode_; }
    void setRankingMode(const QString &mode);
    QVariantMap personalRecord() const { return personalRecord_; }
    QVariantMap sessionMetrics() const;
    QVariantMap playerMetrics() const { return playerMetrics_; }
    QString gameMode() const { return service_.configuration().modeKey(); }
    int questionLimit() const { return service_.configuration().questionLimit; }
    int competitiveQuestionCount() const { return GameConfiguration::CompetitiveQuestionCount; }
    QString difficulty() const { return difficulty_; }
    void setDifficulty(const QString &difficulty);
    int answeredCount() const { return service_.answeredCount(); }
    int catalogSize() const { return service_.catalogSize(); }
    bool answerPending() const { return answerPending_; }
    QVariantMap answerDetails() const { return answerDetails_; }
    QVariantList categories() const;
    QVariantList categoryStats() const;
    QVariantList achievements() const;
    QString categoryName() const { return categoryName_; }
    QVariantMap browseTaxon() const { return taxonomy_.describe(explorationId_); }
    QVariantList browseChildren() const { return taxonomy_.children(explorationId_); }
    QVariantList breadcrumbs() const { return taxonomy_.breadcrumbs(explorationId_); }
    QVariantList identificationLevels() const { return taxonomy_.identificationLevels(explorationId_); }
    QVariantMap questionPhoto() const;
    QString identificationRankLabel() const { return taxonomicRankLabel(identificationRank_); }
    QString sessionTaxonId() const { return sessionTaxonId_; }
    bool hasHint() const { return !displayedQuestion_.extraHint.isEmpty(); }
    QString hintSourceUrl() const { return displayedQuestion_.hintSourceUrl; }

public slots:
    void startGame();
    void startCategoryGame(const QString &family);
    void startTaxonGame(const QString &taxonId, const QString &rank);
    void startCompetitiveGame();
    void exploreTaxon(const QString &taxonId);
    void nextQuestion();
    void answerSelected(const QString &selectedOption);
    void revealHint();
    bool revealHintCategory(const QString &category);
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
    void explorationChanged();
    void difficultyChanged();

private:
    void startWithConfiguration(const GameConfiguration &configuration);
    SqliteSpeciesCatalog catalog_;
    TaxonomyIndex taxonomy_;
    SqliteGameHistoryRepository historyRepository_;
    GameService service_;
    QString currentPlayerName_;
    QString sessionPlayerName_;
    bool resultPending_ = false;
    QString feedbackText_;
    QString categoryName_;
    QString explorationId_;
    QString sessionTaxonId_;
    QString identificationRank_ = "species";
    bool answerPending_ = false;
    QVariantMap answerDetails_;
    Question displayedQuestion_;
    int displayedRevealedHints_=0;
    EducationalContent content_;
    int correctStreak_ = 0;
    int bestStreak_ = 0;
    QString rankingMode_ = "free";
    QString difficulty_ = "balanced";
    QVariantList ranking_;
    QVariantMap personalRecord_;
    QVariantMap playerMetrics_;
};
