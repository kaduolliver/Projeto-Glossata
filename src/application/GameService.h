#pragma once

#include "application/ports/GameHistoryRepository.h"
#include "application/ports/SpeciesCatalog.h"
#include "domain/GameSession.h"
#include "domain/Question.h"
#include <QVector>
#include <QStringList>
#include "application/EducationalContent.h"
#include "domain/TaxonomicPerformance.h"

class GameService {
public:
    enum class SaveStatus { NotNeeded, Saved, Failed };
    explicit GameService(SpeciesCatalog *catalog = nullptr, GameHistoryRepository *history = nullptr);

    void startNewGame(const QString &filter = QString(), const QString &rank = QStringLiteral("species"));
    Question currentQuestion() const;
    QString currentFact() const;
    QString extraHint() const;
    bool hintRevealed() const;
    bool submitAnswer(const QString &selectedOption);
    bool revealHint();
    bool revealHint(HintType type);
    int revealedHints() const { return revealedHints_; }
    int possibleScoreUnits() const { return maximumScoreUnits(revealedHints_); }
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
    SaveStatus saveCurrentResult(const QString &playerName);

private:
    void buildQuestionForCurrentSpecies();
    void shuffleCycle();

    SpeciesCatalog *catalog_ = nullptr;
    GameHistoryRepository *history_ = nullptr;
    QVector<Species> species_;
    int currentSpeciesIndex_ = 0;
    bool resultSaved_ = false;
    int revealedHints_ = 0;
    GameSession session_;
    Question currentQuestion_;
    QString identificationRank_ = QStringLiteral("species");
    EducationalContent content_;
    TaxonomicPerformance performance_;
};
