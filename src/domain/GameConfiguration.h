#pragma once

#include <QString>
#include <QVector>
#include "domain/Hint.h"

enum class GameMode { Free, Competitive };
enum class QuestionDifficulty { Broad, Balanced, Close };

struct GameConfiguration {
    static constexpr int CompetitiveQuestionCount = 20;
    GameMode mode = GameMode::Free;
    int questionLimit = 0;
    QString group;
    QString rank = "species";
    QuestionDifficulty difficulty = QuestionDifficulty::Balanced;
    QString contentVersion;
    QString rulesVersion = CurrentScoringRules;

    QString modeKey() const { return mode == GameMode::Competitive ? "competitive" : "free"; }
    QString difficultyKey() const {
        return difficulty == QuestionDifficulty::Broad ? "broad" : difficulty == QuestionDifficulty::Close ? "close" : "balanced";
    }
    QString rankingKey() const {
        if (mode == GameMode::Free) return "free";
        return QString("competitive|%1|%2|%3|%4|%5|%6")
            .arg(questionLimit).arg(group, rank, difficultyKey(), rulesVersion, contentVersion);
    }
};

struct SessionMetrics {
    int answered = 0, correct = 0, unassistedCorrect = 0, hintedQuestions = 0, revealedCategories = 0;
    std::array<int, 3> categoryUse{};
    qint64 units = 0;
    void record(const AnswerRecord &answer) {
        ++answered;
        correct += answer.correct;
        units += answer.awardedUnits;
        unassistedCorrect += answer.correct && answer.revealedHints == 0;
        hintedQuestions += answer.revealedHints != 0;
        for (int i = 0; i < 3; ++i) if (answer.revealedHints & (1 << i)) {
            ++categoryUse[i];
            ++revealedCategories;
        }
    }
    static SessionMetrics fromAnswers(const QVector<AnswerRecord> &answers) {
        SessionMetrics result;
        for (const auto &answer : answers) {
            result.record(answer);
        }
        return result;
    }
    double accuracy() const { return answered ? 100.0 * correct / answered : 0; }
    double efficiency() const { return answered ? 100.0 * units / (qint64(answered) * FullScoreUnits) : 0; }
    QString averageText() const {
        if (!answered) return "0,00";
        const qint64 denominator = qint64(answered) * FullScoreUnits;
        const qint64 hundredths = (units * 100 + denominator / 2) / denominator;
        return QString::number(hundredths / 100) + "," + QString::number(hundredths % 100).rightJustified(2, '0');
    }
};
