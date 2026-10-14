#pragma once

#include "application/ports/GameHistoryRepository.h"
#include "application/ports/SpeciesCatalog.h"
#include "domain/GameSession.h"
#include "domain/Question.h"
#include <QVector>
#include <QStringList>

class GameService {
public:
    enum class SaveStatus { NotNeeded, Saved, Failed };
    explicit GameService(SpeciesCatalog *catalog = nullptr, GameHistoryRepository *history = nullptr);

    void startNewGame(const QString &family = QString());
    Question currentQuestion() const;
    QString currentFact() const;
    QString extraHint() const;
    bool hintRevealed() const;
    bool submitAnswer(const QString &selectedOption);
    bool revealHint();
    void finishGame();
    int score() const;
    int round() const;
    int answeredCount() const { return session_.answeredCount; }
    int catalogSize() const { return static_cast<int>(species_.size()); }
    bool isFinished() const;
    QVector<HistoryEntry> history() const;
    SaveStatus saveCurrentResult(const QString &playerName);

private:
    void buildQuestionForCurrentSpecies();

    SpeciesCatalog *catalog_ = nullptr;
    GameHistoryRepository *history_ = nullptr;
    QVector<Species> species_;
    int currentSpeciesIndex_ = 0;
    bool resultSaved_ = false;
    bool hintRevealed_ = false;
    GameSession session_;
    Question currentQuestion_;
};
