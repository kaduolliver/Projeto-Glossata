#include "application/GameService.h"

#include <algorithm>
#include <random>
#include <limits>
#include "domain/QuizSelection.h"
#include <QMap>
#include <QSet>

namespace {
QStringList buildOptionSet(const QVector<Species> &species, const Species &correct, const QString &rank)
{
    const QString correctName = correct.answerAtRank(rank).trimmed();
    QStringList distractors;
    for (const Species &candidate : species) {
        const QString candidateName = candidate.answerAtRank(rank).trimmed();
        if (!canBeDistractor(candidate, correct, rank)
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

void GameService::startNewGame(const QString &filter, const QString &rank)
{
    identificationRank_ = rank;
    species_.clear();
    currentSpeciesIndex_ = 0;
    resultSaved_ = false;
    hintRevealed_ = false;
    session_ = GameSession();
    performance_ = TaxonomicPerformance();
    currentQuestion_ = Question();

    if (catalog_ == nullptr) {
        session_.round = 0;
        session_.finished = true;
        return;
    }

    const QVector<Species> loadedSpecies = catalog_->loadSpecies();
    for (Species species : loadedSpecies) {
        if (!filter.isEmpty() && !species.belongsTo(filter) && species.family != filter) {
            continue;
        }
        species.name = species.name.trimmed();
        species.fact = species.fact.trimmed();
        species.hint = species.hint.trimmed();
        if (species.name.isEmpty()
            || species.answerAtRank(rank).trimmed().isEmpty()) {
            continue;
        }

        const bool alreadyLoaded = std::any_of(species_.cbegin(), species_.cend(), [&species](const Species &loaded) {
            return loaded.name.compare(species.name, Qt::CaseInsensitive) == 0;
        });
        if (!alreadyLoaded) {
            species_.push_back(species);
        }
    }

    species_ = eligibleQuizSpecies(species_, rank);
    QSet<QString> distinctAnswers;
    for (const Species &species : species_) distinctAnswers.insert(species.answerAtRank(rank).toCaseFolded());
    if (distinctAnswers.size() < 4) {
        session_.round = 0;
        session_.finished = true;
        return;
    }

    shuffleCycle();
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
    performance_.record(currentQuestion_.species,isCorrect);
    ++session_.answeredCount;
    if (isCorrect) {
        ++session_.score;
    }

    ++session_.round;
    ++currentSpeciesIndex_;
    hintRevealed_ = false;
    if (currentSpeciesIndex_ >= species_.size()) {
        const QString previousSpeciesName = species_.last().name;
        shuffleCycle();
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
    const QStringList options = buildOptionSet(species_, correct, identificationRank_);
    if (options.size() != 4) {
        currentQuestion_ = Question{};
        session_.finished = true;
        return;
    }

    const bool visual = correct.photo.valid && correct.photo.approvedRanks.contains(identificationRank_);
    const QString prompt = identificationRank_ == "species"
        ? QStringLiteral("Qual espécie está registrada nesta fotografia?")
        : QStringLiteral("Identifique: %1").arg(taxonomicRankLabel(identificationRank_));
    const QString fact = QStringLiteral("Observe a fotografia. Compare a forma, as estruturas e os padrões visíveis.");
    const auto hint=content_.hint(correct,identificationRank_,options);
    currentQuestion_ = Question{
        prompt,
        fact,
        hint.value("text").toString(),
        correct.answerAtRank(identificationRank_),
        options,
        correct,
        identificationRank_,
        visual,
        correct.photo,
        hint.value("source_url").toString(),
        hint.value("source_title").toString()
    };
}

void GameService::shuffleCycle()
{
    std::mt19937 random(std::random_device{}());
    QMap<QString, QVector<Species>> groups;
    for (const Species &species : species_) groups[species.educationalKingdom].append(species);
    const int total = static_cast<int>(species_.size());
    QMap<QString, int> weight, balance;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        std::shuffle(it.value().begin(), it.value().end(), random);
        weight[it.key()] = static_cast<int>(it.value().size());
        balance[it.key()] = std::uniform_int_distribution<int>(0, std::max(1, total / 3))(random);
    }
    species_.clear();
    while (species_.size() < total) {
        QString chosen;
        int best = std::numeric_limits<int>::min();
        for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
            if (it.value().isEmpty()) continue;
            balance[it.key()] += weight[it.key()];
            if (balance[it.key()] > best) { best = balance[it.key()]; chosen = it.key(); }
        }
        balance[chosen] -= total;
        species_.append(groups[chosen].takeLast());
    }
}
