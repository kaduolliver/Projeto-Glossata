#pragma once

#include "application/ports/GameHistoryRepository.h"
#include "application/ports/SpeciesCatalog.h"
#include "domain/GameSession.h"
#include "domain/Question.h"
#include <QVector>
#include <QStringList>
#include "application/EducationalContent.h"
#include "domain/TaxonomicPerformance.h"
#include "application/QuestionGenerator.h"
#include "application/ports/SessionCheckpointRepository.h"

class GameService {
public:
    enum class SaveStatus { NotNeeded, Saved, Failed };
    explicit GameService(SpeciesCatalog *catalog = nullptr, GameHistoryRepository *history = nullptr,
                         const EducationalContent &content = EducationalContent());

    void startNewGame(const QString &filter = QString(), const QString &rank = QStringLiteral("species"), const PlayerIdentity &player = {});
    void startConfiguredGame(GameConfiguration configuration, const PlayerIdentity &player = {});
    PlayerIdentity sessionPlayer() const { return session_.player; }
    static GameConfiguration competitiveConfiguration();
    static QString contentFingerprint(const QByteArray &education);
    GameConfiguration configuration() const { return session_.configuration; }
    SessionMetrics metrics() const { return SessionMetrics::fromAnswers(session_.answers); }
    Question currentQuestion() const;
    QString currentFact() const;
    QString extraHint() const;
    bool hintRevealed() const;
    bool submitAnswer(const QString &selectedOption);
    bool revealHint();
    bool revealHint(HintType type); // Compatibility: first hint in this category; never silently reveals the next.
    bool revealHintSlot(int slot);
    int revealedHints() const { return revealedHints_; }
    int possibleScoreUnits() const { return maximumScoreUnits(revealedHints_,hintCeilings(currentQuestion_.hints)); }
    QVector<AnswerRecord> answers() const { return session_.answers; }
    void finishGame();
    qint64 scoreUnits() const;
    int correctAnswers() const { return session_.correctAnswers; }
    int round() const;
    int answeredCount() const { return session_.answeredCount; }
    int catalogSize() const { return static_cast<int>(species_.size()); }
    bool isFinished() const;
    TaxonomicPerformance performance() const { return performance_; }
    QVector<HistoryEntry> history() const;
    SaveStatus saveCurrentResult();
    void setCheckpointRepository(SessionCheckpointRepository *repository) { checkpoints_=repository; }
    bool recoverSession(const QString &id);
    bool checkpointProgress();
    bool hasCheckpoint() const { return checkpointRevision_>0 && !resultSaved_; }
    QString sessionId() const { return session_.id; }
    QString checkpointError() const { return checkpointError_; }
    QJsonObject sessionSnapshot() const;

private:
    void buildQuestionForCurrentSpecies();
    void shuffleCycle();
    QString contentIdentity() const;
    bool ensureCheckpoint();

    SpeciesCatalog *catalog_ = nullptr;
    GameHistoryRepository *history_ = nullptr;
    QVector<Species> species_;
    QVector<Species> optionPool_;
    int currentSpeciesIndex_ = 0;
    bool resultSaved_ = false;
    int revealedHints_ = 0;
    GameSession session_;
    Question currentQuestion_;
    QString identificationRank_ = QStringLiteral("species");
    EducationalContent content_;
    std::mt19937 random_{std::random_device{}()};
    TaxonomicPerformance performance_;
    SessionCheckpointRepository *checkpoints_ = nullptr;
    qint64 checkpointRevision_ = 0;
    QString checkpointError_, contentIdentity_;
    QJsonArray presentedQuestions_;
};
