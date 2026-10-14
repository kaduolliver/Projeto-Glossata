#include "presentation/GameController.h"

#include <QStandardPaths>
#include <QSet>
#include <algorithm>

GameController::GameController(QObject *parent, const QString &databasePath)
    : QObject(parent),
      catalog_(databasePath.isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/glossata_catalog.sqlite")
          : databasePath + QStringLiteral(".catalog.sqlite")),
      taxonomy_(catalog_),
      historyRepository_(databasePath.isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/glossata_history.sqlite")
          : databasePath),
      service_(&catalog_, &historyRepository_)
{
    if (historyRepository_.loadPlayerProfiles().isEmpty()) {
        historyRepository_.addPlayerProfile(QStringLiteral("Jogador 1"));
    }
    const QStringList profiles = historyRepository_.loadPlayerProfiles();
    if (!profiles.isEmpty()) {
        currentPlayerName_ = profiles.first();
    }
    startGame();
}

QString GameController::questionText() const
{
    return displayedQuestion_.prompt;
}

QString GameController::factText() const
{
    return displayedQuestion_.fact;
}

QString GameController::extraHint() const
{
    return hintRevealed() ? displayedQuestion_.extraHint : QString();
}

bool GameController::hintRevealed() const
{
    return displayedRevealedHints_ & hintBit(HintType::Morphological);
}

QStringList GameController::answerOptions() const
{
    return displayedQuestion_.options;
}

QStringList GameController::playerProfiles() const
{
    return historyRepository_.loadPlayerProfiles();
}

QString GameController::currentPlayerName() const
{
    return currentPlayerName_;
}

void GameController::setCurrentPlayerName(const QString &name)
{
    if (currentPlayerName_ == name || !playerProfiles().contains(name)) {
        return;
    }

    currentPlayerName_ = name;
    emit currentPlayerNameChanged();
}

qint64 GameController::scoreUnits() const
{
    return service_.scoreUnits();
}

int GameController::round() const
{
    return service_.round();
}

bool GameController::gameFinished() const
{
    return service_.isFinished();
}

QString GameController::feedbackText() const
{
    return feedbackText_;
}

QVariantList GameController::ranking() const
{
    QVariantList list;
    const QVector<HistoryEntry> entries = service_.history();
    for (const HistoryEntry &entry : entries) {
        QVariantMap row;
        row.insert(QStringLiteral("playerName"), entry.playerName);
        row.insert(QStringLiteral("scoreUnits"), entry.scoreUnits);
        row.insert(QStringLiteral("scoreText"), formatScore(entry.scoreUnits));
        row.insert(QStringLiteral("correctAnswers"), entry.correctAnswers);
        row.insert(QStringLiteral("playedAt"), entry.playedAt.toString(QStringLiteral("dd/MM/yyyy")));
        list.append(row);
    }
    return list;
}

void GameController::startGame()
{
    startCategoryGame(QString());
}

void GameController::startCategoryGame(const QString &family)
{
    startTaxonGame(family, QStringLiteral("species"));
}

void GameController::exploreTaxon(const QString &taxonId)
{
    if (taxonomy_.describe(taxonId).value("title").toString().isEmpty()) return;
    explorationId_ = taxonId;
    emit explorationChanged();
}

QVariantMap GameController::questionPhoto() const
{
    const Photo &p=displayedQuestion_.photo;
    return {{"image",p.valid ? p.image : QString()}, {"photoId",p.id},{"author",p.author},
        {"license",p.license},{"licenseUrl",p.licenseUrl},{"source",p.sourceUrl},{"modifications",p.modifications},
        {"kingdom",displayedQuestion_.species.educationalKingdom},{"visual",displayedQuestion_.visual},
        {"supporting",p.role=="supporting"},{"photoSubject",p.scientificName},{"speciesId",displayedQuestion_.species.id}};
}

void GameController::startTaxonGame(const QString &family, const QString &rank)
{
    if (resultPending_) {
        feedbackText_ = QStringLiteral("Salve o resultado pendente antes de iniciar outra partida.");
        emit feedbackTextChanged();
        return;
    }
    sessionPlayerName_ = currentPlayerName_;
    identificationRank_ = rank;
    sessionTaxonId_ = taxonomy_.describe(family).value("title").toString().isEmpty() ? QString() : family;
    categoryName_ = family.isEmpty() ? QStringLiteral("Todas as espécies")
        : (sessionTaxonId_.isEmpty() ? family : taxonomy_.describe(family).value("title").toString());
    answerPending_ = false;
    answerDetails_.clear();
    displayedRevealedHints_=0;
    correctStreak_ = 0;
    bestStreak_ = 0;
    service_.startNewGame(family, rank);
    displayedQuestion_=service_.currentQuestion();
    feedbackText_ = service_.currentQuestion().correctAnswer.isEmpty()
        ? QStringLiteral("Fotografias aprovadas insuficientes: são necessárias quatro respostas distintas neste nível.")
        : QStringLiteral("Escolha a opção correta para continuar.");
    emit questionTextChanged();
    emit factTextChanged();
    emit extraHintChanged();
    emit answerOptionsChanged();
    emit scoreChanged();
    emit roundChanged();
    emit gameFinishedChanged();
    emit feedbackTextChanged();
    emit answerDetailsChanged();
    loadRanking();
}

void GameController::answerSelected(const QString &selectedOption)
{
    if (service_.isFinished() || answerPending_ || !service_.currentQuestion().options.contains(selectedOption)) {
        return;
    }

    const Question answeredQuestion = displayedQuestion_;
    const bool correct = service_.submitAnswer(selectedOption);
    answerPending_ = true;
    const auto profile=content_.profile(answeredQuestion.species);
    const auto curiosity=content_.curiosity(answeredQuestion.species);
    const auto record=service_.answers().last();
    answerDetails_ = {
        {QStringLiteral("correct"), correct},
        {QStringLiteral("selectedAnswer"), selectedOption},
        {QStringLiteral("name"), answeredQuestion.correctAnswer},
        {QStringLiteral("fact"), curiosity.value("text").toString()},
        {QStringLiteral("curiositySource"),curiosity.value("source_url")},
        {QStringLiteral("curiositySourceTitle"),curiosity.value("source_title")},
        {QStringLiteral("speciesName"), answeredQuestion.species.name},
        {QStringLiteral("commonName"), profile.value("commonName").toString()},
        {QStringLiteral("profile"), profile},
        {QStringLiteral("kingdom"), answeredQuestion.species.educationalKingdom},
        {QStringLiteral("sourceUrl"), answeredQuestion.species.sourceUrl},
        {QStringLiteral("author"), answeredQuestion.species.imageAuthor},
        {QStringLiteral("license"), answeredQuestion.species.imageLicense},
        {QStringLiteral("licenseUrl"), answeredQuestion.species.imageLicenseUrl},
        {QStringLiteral("source"), answeredQuestion.species.imageSource},
        {QStringLiteral("hint"), answeredQuestion.extraHint},
        {QStringLiteral("curiosityScope"),curiosity.value("scope_name").toString()},
        {QStringLiteral("hintSource"), answeredQuestion.hintSourceUrl},
        {QStringLiteral("hintSourceTitle"), answeredQuestion.hintSourceTitle},
        {QStringLiteral("modifications"),answeredQuestion.photo.modifications},
        {QStringLiteral("family"), answeredQuestion.species.family},
        {QStringLiteral("region"), profile.value("distribution").toMap().value("text").toString()},
        {QStringLiteral("image"), answeredQuestion.photo.image},
        {QStringLiteral("photoId"),answeredQuestion.photo.id}
    };
    answerDetails_.insert("awardedUnits",record.awardedUnits);
    answerDetails_.insert("awardedText",formatScore(record.awardedUnits));
    answerDetails_.insert("maximumUnits",record.maximumUnits);
    answerDetails_.insert("revealedHints",record.revealedHints);
    answerDetails_.insert("nameOrigin",content_.nameOrigin(answeredQuestion.species));
    answerDetails_.insert("supporting",answeredQuestion.photo.role=="supporting");
    answerDetails_.insert("photoSubject",answeredQuestion.photo.scientificName);
    QStringList lineage;
    for (const Taxon &node : answeredQuestion.species.taxonomy)
        if (node.rank != "educational_kingdom") lineage << taxonomicRankLabel(node.rank) + ": " + node.name;
    answerDetails_.insert("classification", lineage.join(" · "));
    correctStreak_ = correct ? correctStreak_ + 1 : 0;
    bestStreak_ = std::max(bestStreak_, correctStreak_);
    emit scoreChanged();
    emit roundChanged();
    feedbackText_ = correct
        ? QStringLiteral("Correto! Próxima rodada.")
        : QStringLiteral("Incorreto. Continue tentando.");
    emit questionTextChanged();
    emit factTextChanged();
    emit extraHintChanged();
    emit answerOptionsChanged();
    emit feedbackTextChanged();
    emit answerDetailsChanged();
}

void GameController::nextQuestion()
{
    if (!answerPending_ || service_.isFinished()) return;
    answerPending_ = false;
    displayedQuestion_=service_.currentQuestion();
    displayedRevealedHints_=0;
    emit questionTextChanged(); emit factTextChanged(); emit extraHintChanged(); emit answerOptionsChanged();
    emit answerDetailsChanged();
}

QVariantList GameController::hints() const
{
    QVariantList rows;
    for(const auto &hint : displayedQuestion_.hints) {
        const bool revealed=displayedRevealedHints_ & hintBit(hint.type);
        rows.append(QVariantMap{{"id",hintKey(hint.type)},{"available",hint.available()},
            {"revealed",revealed},{"text",revealed ? hint.text : QString()},
            {"sourceUrl",revealed ? hint.sourceUrl : QString()},{"reason",hint.unavailableReason},{"pointsText",formatScore(maximumScoreUnits(hintBit(hint.type)))}});
    }
    return rows;
}
void GameController::revealHint()
{
    revealHintCategory("morphological");
}
bool GameController::revealHintCategory(const QString &category)
{
    const HintType type=hintType(category);
    if(answerPending_ || type==HintType::None)return false;
    if(displayedRevealedHints_ & hintBit(type))return true;
    if(!service_.revealHint(type))return false;
    displayedRevealedHints_=service_.revealedHints();
    emit extraHintChanged();return true;
}

void GameController::stopGame()
{
    if (service_.isFinished()) {
        return;
    }

    service_.finishGame();
    retrySaveResult();
    emit answerOptionsChanged();
    emit gameFinishedChanged();
}

void GameController::retrySaveResult()
{
    if (!service_.isFinished()) {
        return;
    }
    resultPending_ = service_.saveCurrentResult(sessionPlayerName_) == GameService::SaveStatus::Failed;
    feedbackText_ = resultPending_
        ? QStringLiteral("Não foi possível salvar a pontuação. Tente salvar novamente antes de fechar o jogo.")
        : QStringLiteral("Partida encerrada. Pontuação: %1.").arg(scoreText());
    emit resultPendingChanged();
    emit feedbackTextChanged();
    loadRanking();
}

void GameController::loadRanking()
{
    emit rankingChanged();
}

QVariantList GameController::categories() const
{
    return taxonomy_.kingdoms();
}

QVariantList GameController::categoryStats() const
{
    QVariantList list;
    const auto performance=service_.performance();
    for (const QString &id : QStringList{"edu:Animalia","edu:Plantae","edu:Fungi","edu:Protista","edu:Monera","edu:unknown"}) {
        if (!performance.groups.contains(id)) continue;
        const auto &group=performance.groups[id];
        QVariantList phyla;
        for (const auto &p : group.phyla) phyla.append(QVariantMap{{"id",p.id},{"title",p.name},{"answered",p.answered},{"correct",p.correct},{"accuracy",p.answered ? 100*p.correct/p.answered : 0}});
        list.append(QVariantMap{{"id",group.id},{"title",group.name},{"kingdom",group.name},{"answered",group.answered},{"correct",group.correct},{"phyla",phyla}});
    }
    return list;
}

QVariantList GameController::achievements() const
{
    QVariantList list;
    const auto add = [&list](const QString &name, const QString &description, const QString &icon, int value, int target) {
        list.append(QVariantMap{
            {QStringLiteral("title"), name}, {QStringLiteral("description"), description},
            {QStringLiteral("icon"), icon}, {QStringLiteral("progress"), std::min(value, target)},
            {QStringLiteral("target"), target}, {QStringLiteral("unlocked"), value >= target}
        });
    };
    add(QStringLiteral("Primeiro voo"), QStringLiteral("Responda sua primeira pergunta."), "leaf", answeredCount(), 1);
    add(QStringLiteral("Olhar atento"), QStringLiteral("Acerte cinco perguntas."), "target", correctAnswers(), 5);
    add(QStringLiteral("Explorador"), QStringLiteral("Responda dez perguntas."), "butterfly", answeredCount(), 10);
    add(QStringLiteral("Em sequência"), QStringLiteral("Faça três acertos seguidos."), "spark", bestStreak_, 3);
    add(QStringLiteral("Conhecedor"), QStringLiteral("Acerte vinte perguntas."), "book", correctAnswers(), 20);
    add(QStringLiteral("Um ciclo completo"), QStringLiteral("Explore todo o grupo escolhido."), "trophy", answeredCount(), std::max(1, catalogSize()));
    return list;
}

bool GameController::addPlayerProfile(const QString &name)
{
    const QString profileName = name.trimmed();
    if (profileName.isEmpty() || profileName.size() > 32
        || !historyRepository_.addPlayerProfile(profileName)) {
        feedbackText_ = QStringLiteral("Nome inválido ou já cadastrado.");
        emit feedbackTextChanged();
        return false;
    }

    emit playerProfilesChanged();
    setCurrentPlayerName(profileName);
    feedbackText_ = QStringLiteral("Perfil criado.");
    emit feedbackTextChanged();
    return true;
}

bool GameController::renameCurrentPlayerProfile(const QString &name)
{
    const QString profileName = name.trimmed();
    if (profileName.isEmpty() || profileName.size() > 32
        || !historyRepository_.renamePlayerProfile(currentPlayerName_, profileName)) {
        feedbackText_ = QStringLiteral("Não foi possível renomear o perfil.");
        emit feedbackTextChanged();
        return false;
    }

    currentPlayerName_ = profileName;
    emit currentPlayerNameChanged();
    emit playerProfilesChanged();
    feedbackText_ = QStringLiteral("Perfil renomeado.");
    emit feedbackTextChanged();
    return true;
}

bool GameController::deleteCurrentPlayerProfile()
{
    if (playerProfiles().size() <= 1 || !historyRepository_.deletePlayerProfile(currentPlayerName_)) {
        feedbackText_ = QStringLiteral("Mantenha ao menos um perfil cadastrado.");
        emit feedbackTextChanged();
        return false;
    }

    currentPlayerName_ = playerProfiles().first();
    emit currentPlayerNameChanged();
    emit playerProfilesChanged();
    feedbackText_ = QStringLiteral("Perfil removido.");
    emit feedbackTextChanged();
    return true;
}
