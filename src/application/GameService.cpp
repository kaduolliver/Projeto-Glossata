#include "application/GameService.h"

#include <algorithm>
#include <random>

namespace {
QStringList buildOptionSet(const QVector<Species> &species, const Species &correct)
{
    const QString correctName = correct.name.trimmed();
    QStringList distractors;
    for (const Species &candidate : species) {
        const QString candidateName = candidate.name.trimmed();
        if (candidateName.isEmpty() || candidateName.compare(correctName, Qt::CaseInsensitive) == 0
            || distractors.contains(candidateName, Qt::CaseInsensitive)) {
            continue;
        }
        distractors << candidateName;
    }

    std::shuffle(distractors.begin(), distractors.end(), std::mt19937(std::random_device{}()));

    QStringList options{correctName};
    while (options.size() < 4 && !distractors.isEmpty()) {
        options << distractors.takeFirst();
    }

    std::shuffle(options.begin(), options.end(), std::mt19937(std::random_device{}()));
    return options;
}
}

GameService::GameService(SpeciesCatalog *catalog, GameHistoryRepository *history)
    : catalog_(catalog), history_(history)
{
}

void GameService::startNewGame(const QString &family)
{
    species_.clear();
    currentSpeciesIndex_ = 0;
    resultSaved_ = false;
    hintRevealed_ = false;
    session_ = GameSession();
    currentQuestion_ = Question();

    if (catalog_ == nullptr) {
        session_.round = 0;
        session_.finished = true;
        return;
    }

    const QVector<Species> loadedSpecies = catalog_->loadSpecies();
    for (Species species : loadedSpecies) {
        if (!family.isEmpty() && species.family != family) {
            continue;
        }
        species.name = species.name.trimmed();
        species.fact = species.fact.trimmed();
        species.hint = species.hint.trimmed();
        if (species.name.isEmpty() || species.fact.isEmpty() || species.hint.isEmpty()) {
            continue;
        }

        const bool alreadyLoaded = std::any_of(species_.cbegin(), species_.cend(), [&species](const Species &loaded) {
            return loaded.name.compare(species.name, Qt::CaseInsensitive) == 0;
        });
        if (!alreadyLoaded) {
            species_.push_back(species);
        }
    }

    if (species_.size() < 4) {
        session_.round = 0;
        session_.finished = true;
        return;
    }

    std::shuffle(species_.begin(), species_.end(), std::mt19937(std::random_device{}()));
    buildQuestionForCurrentSpecies();
}

Question GameService::currentQuestion() const
{
    return currentQuestion_;
}

QString GameService::currentFact() const
{
    return currentQuestion_.fact;
}

QString GameService::extraHint() const
{
    return hintRevealed_ ? currentQuestion_.extraHint : QString();
}

bool GameService::hintRevealed() const
{
    return hintRevealed_;
}

bool GameService::submitAnswer(const QString &selectedOption)
{
    if (session_.finished || !currentQuestion_.options.contains(selectedOption)) {
        return false;
    }

    const bool isCorrect = selectedOption == currentQuestion_.correctAnswer;
    ++session_.answeredCount;
    if (isCorrect) {
        ++session_.score;
    }

    ++session_.round;
    ++currentSpeciesIndex_;
    hintRevealed_ = false;
    if (currentSpeciesIndex_ >= species_.size()) {
        const QString previousSpeciesName = species_.last().name;
        std::shuffle(species_.begin(), species_.end(), std::mt19937(std::random_device{}()));
        if (species_.size() > 1 && species_.front().name == previousSpeciesName) {
            std::swap(species_[0], species_[1]);
        }
        currentSpeciesIndex_ = 0;
    }
    buildQuestionForCurrentSpecies();
    return isCorrect;
}

bool GameService::revealHint()
{
    if (session_.finished || hintRevealed_ || currentQuestion_.extraHint.isEmpty()) {
        return false;
    }

    hintRevealed_ = true;
    return true;
}

void GameService::finishGame()
{
    if (session_.finished) {
        return;
    }

    session_.finished = true;
    currentQuestion_.options.clear();
}

int GameService::score() const
{
    return session_.score;
}

int GameService::round() const
{
    return session_.round;
}

bool GameService::isFinished() const
{
    return session_.finished;
}

QVector<HistoryEntry> GameService::history() const
{
    if (history_ == nullptr) {
        return {};
    }

    return history_->loadTopResults(5);
}

GameService::SaveStatus GameService::saveCurrentResult(const QString &playerName)
{
    if (!session_.finished || session_.answeredCount == 0 || resultSaved_) {
        return SaveStatus::NotNeeded;
    }

    if (history_ == nullptr || playerName.trimmed().isEmpty()
        || !history_->saveResult(playerName.trimmed(), session_.score, session_.score)) {
        return SaveStatus::Failed;
    }
    resultSaved_ = true;
    return SaveStatus::Saved;
}

void GameService::buildQuestionForCurrentSpecies()
{
    if (currentSpeciesIndex_ < 0 || currentSpeciesIndex_ >= species_.size()) {
        currentQuestion_ = Question{};
        session_.finished = true;
        return;
    }

    const Species correct = species_.at(currentSpeciesIndex_);
    const QStringList options = buildOptionSet(species_, correct);
    if (options.size() != 4) {
        currentQuestion_ = Question{};
        session_.finished = true;
        return;
    }

    currentQuestion_ = Question{
        QStringLiteral("Qual nome corresponde a esta espécie?"),
        correct.fact,
        correct.hint,
        correct.name,
        options,
        correct
    };
}
