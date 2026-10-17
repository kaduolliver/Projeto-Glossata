#include "presentation/GameController.h"
#include <QUrlQuery>

#include <QStandardPaths>
#include <QCoreApplication>
#include <QSet>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

GameController::GameController(QObject *parent, const QString &databasePath)
    : QObject(parent),
      catalog_(databasePath.isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/glossata_catalog.sqlite")
          : databasePath + QStringLiteral(".catalog.sqlite")),
      taxonomy_(catalog_, &explorationMedia_),
      historyRepository_(databasePath.isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/glossata_history.sqlite")
          : databasePath),
      service_(&catalog_, &historyRepository_)
{
    if (historyRepository_.loadPlayerProfiles().isEmpty()) {
        historyRepository_.createPlayer(QStringLiteral("Jogador 1"));
    }
    const auto profiles = historyRepository_.profiles();
    if (!profiles.isEmpty()) {
        currentPlayerName_ = profiles.first().displayName;
        currentPlayerId_ = profiles.first().id;
    }
    startGame();
}

QVariantMap GameController::legalInformation() const
{
    QFile file(":/resources/data/credits.json");
    if (!file.open(QIODevice::ReadOnly)) return {};
    const auto data = QJsonDocument::fromJson(file.readAll()).object();
    QFile catalogFile(":/resources/data/catalog_manifest.json");
    QJsonObject catalog;
    if (catalogFile.open(QIODevice::ReadOnly)) catalog = QJsonDocument::fromJson(catalogFile.readAll()).object();
    QVariantList photos;
    const auto records = data.value("photos").toObject();
    for (auto it = records.begin(); it != records.end(); ++it) photos.append(it.value().toObject().toVariantMap());
    QVariantList explorationPhotos;
    const auto presentation=data.value("exploration_photos").toObject();
    for(auto it=presentation.begin();it!=presentation.end();++it) explorationPhotos.append(it.value().toObject().toVariantMap());
    return {{"notice", data.value("notice").toString()}, {"photos", photos},
            {"explorationPhotos",explorationPhotos},
            {"applicationVersion", QCoreApplication::applicationVersion()},
            {"catalogVersion", catalog.value("version").toString()}, {"catalogHash", catalog.value("content_sha256").toString()},
            {"limitations", QStringLiteral("Catálogo legado com revisão humana e permissões ainda incompletas. Testes automáticos não são certificação jurídica ou científica.")}};
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
    selectPlayerProfile(historyRepository_.profileNamed(name).id);
}

void GameController::selectPlayerProfile(const QString &id)
{
    const auto profile = historyRepository_.player(id);
    if (!profile.selectable() || currentPlayerId_ == id) return;
    currentPlayerId_ = profile.id;
    currentPlayerName_ = profile.displayName;
    emit currentPlayerNameChanged();
    loadRanking();
}

QVariantList GameController::playerProfileItems() const
{
    QVariantList result;
    for (const auto &p : historyRepository_.profiles()) result.append(QVariantMap{{"playerId",p.id},{"name",p.displayName}});
    return result;
}

QString GameController::identityNotice() const
{
    const int count = historyRepository_.unresolvedLegacyPlayers();
    return count ? QStringLiteral("Há %1 conjunto(s) de resultados anteriores à identificação permanente. Permanecem no ranking como legado, sem vínculo presumido com perfis atuais. As métricas pessoais incluem somente o ID deste perfil.").arg(count) : QString();
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
    return ranking_;
}

void GameController::setRankingMode(const QString &mode)
{
    if (mode != "free" && mode != "competitive") return;
    rankingMode_ = mode;
    loadRanking();
}
void GameController::setDifficulty(const QString &difficulty)
{
    if (difficulty != "broad" && difficulty != "balanced" && difficulty != "close") return;
    difficulty_ = difficulty;
    emit difficultyChanged();
}
namespace {
QVariantMap presentMetrics(const SessionMetrics &metrics)
{
    return {{"answered",metrics.answered},{"correct",metrics.correct},{"scoreText",formatScore(metrics.units)},{"accuracy",QString::number(metrics.accuracy(),'f',1).replace('.',',')},
        {"efficiency",QString::number(metrics.efficiency(),'f',1).replace('.',',')},
        {"average",metrics.averageText()},{"unassistedCorrect",metrics.unassistedCorrect},
        {"hintedQuestions",metrics.hintedQuestions},{"revealedCategories",metrics.revealedCategories},
        {"taxonomic",metrics.categoryUse[0]},{"etymological",metrics.categoryUse[1]},{"morphological",metrics.categoryUse[2]}};
}
}
QVariantMap GameController::sessionMetrics() const
{
    return presentMetrics(service_.metrics());
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
        {"kingdom",displayedQuestion_.species.displayGroup()},{"domain",displayedQuestion_.species.domain()},{"visual",displayedQuestion_.visual},
        {"supporting",p.role=="supporting"},{"photoSubject",p.scientificName},{"speciesId",displayedQuestion_.species.id},
        {"attribution",p.attribution.toVariantMap()}};
}

void GameController::startTaxonGame(const QString &family, const QString &rank)
{
    GameConfiguration configuration;
    configuration.group = family;
    configuration.rank = rank;
    configuration.difficulty = difficulty_ == "broad" ? QuestionDifficulty::Broad
        : difficulty_ == "close" ? QuestionDifficulty::Close : QuestionDifficulty::Balanced;
    startWithConfiguration(configuration);
}

void GameController::startCompetitiveGame()
{
    startWithConfiguration(GameService::competitiveConfiguration());
}

void GameController::startWithConfiguration(const GameConfiguration &configuration)
{
    const QString family = configuration.group;
    const QString rank = configuration.rank;
    if (resultPending_) {
        feedbackText_ = QStringLiteral("Salve o resultado pendente antes de iniciar outra partida.");
        emit feedbackTextChanged();
        return;
    }
    const auto player = historyRepository_.player(currentPlayerId_);
    if (!player.selectable()) {
        feedbackText_ = QStringLiteral("Selecione um perfil ativo antes de iniciar a partida.");
        emit feedbackTextChanged(); return;
    }
    identificationRank_ = rank;
    sessionTaxonId_ = taxonomy_.describe(family).value("title").toString().isEmpty() ? QString() : family;
    categoryName_ = family.isEmpty() ? QStringLiteral("Todas as espécies")
        : (sessionTaxonId_.isEmpty() ? family : taxonomy_.describe(family).value("title").toString());
    answerPending_ = false;
    answerDetails_.clear();
    displayedRevealedHints_=0;
    correctStreak_ = 0;
    bestStreak_ = 0;
    service_.startConfiguredGame(configuration, player.identity());
    rankingMode_ = configuration.modeKey();
    displayedQuestion_=service_.currentQuestion();
    feedbackText_ = service_.currentQuestion().correctAnswer.isEmpty()
        ? QStringLiteral("Não há quatro alternativas aprovadas adequadas a este nível e à dificuldade selecionada.")
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
        {QStringLiteral("kingdom"), answeredQuestion.species.displayGroup()},
        {QStringLiteral("domain"), answeredQuestion.species.domain()},
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
    answerDetails_.insert("attribution",answeredQuestion.photo.attribution.toVariantMap());
    answerDetails_.insert("speciesId",answeredQuestion.species.id);
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
    if (service_.isFinished()) { retrySaveResult(); emit gameFinishedChanged(); }
}

void GameController::nextQuestion()
{
    if (!answerPending_) return;
    if (service_.isFinished()) { answerPending_ = false; emit answerDetailsChanged(); return; }
    answerPending_ = false;
    displayedQuestion_=service_.currentQuestion();
    displayedRevealedHints_=0;
    emit questionTextChanged(); emit factTextChanged(); emit extraHintChanged(); emit answerOptionsChanged();
    emit answerDetailsChanged();
}

QVariantList GameController::hints() const
{
    QVariantList rows;
    for(int slot=0;slot<3;++slot) {
        const auto &hint=displayedQuestion_.hints[slot];
        const bool revealed=displayedRevealedHints_ & (1<<slot);
        rows.append(QVariantMap{{"id",hint.id.isEmpty() ? hintKey(hint.type) : hint.id},{"category",hintKey(hint.type)},{"available",hint.available()},
            {"revealed",revealed},{"text",revealed ? hint.text : QString()},
            {"sourceUrl",revealed ? hint.sourceUrl : QString()},{"reason",hint.unavailableReason},
            {"assistance",assistanceLabel(hint.assistance)},{"provisional",hint.provisional},
            {"evaluationPolicy",hint.assessmentPayload.value("evaluation").toObject().value("policy_version").toString()},
            {"evaluationDate",hint.assessmentPayload.value("evaluation").toObject().value("reviewed_at").toString()},
            {"assessmentState",hint.assessmentState},{"rationale",revealed ? hint.rationale : QString()},
            {"ceilingUnits",hint.ceiling()},
            {"pointsText",hint.available() ? formatScore(maximumScoreUnits(displayedRevealedHints_|(1<<slot),hintCeilings(displayedQuestion_.hints))) : QString()}});
    }
    return rows;
}
void GameController::revealHint()
{
    revealHintCategory(displayedQuestion_.hints[2].id);
}
bool GameController::revealHintCategory(const QString &category)
{
    if(answerPending_)return false;
    for(int i=0;i<3;++i) {
        const auto &hint=displayedQuestion_.hints[i];
        if(hint.id!=category && hintKey(hint.type)!=category)continue;
        if(displayedRevealedHints_&(1<<i))return true;
        if(!service_.revealHintSlot(i))return false;
        displayedRevealedHints_=service_.revealedHints();emit extraHintChanged();return true;
    }
    return false;
}
QVariantMap GameController::hintReport(const QString &id,const QString &problem) const
{
    for(int i=0;i<3;++i) {
        const auto &hint=displayedQuestion_.hints[i];
        if(hint.id!=id || !(displayedRevealedHints_&(1<<i)) || !hint.available())continue;
        // Before answering, organism ID is sufficient. Scientific name, correct
        // answer, other hints, profile and local files never enter this draft.
        const QString text=QStringLiteral("Problema em dica do Projeto Glossata\nOrganismo (ID público): %1\nNível: %2\nDica: %3\nTexto apresentado: %4\nCategoria: %5\nAssistência: %6\nAlternativas apresentadas: %7\nPolítica: %8\nAvaliação: %9\nProblema relatado: %10")
            .arg(QString::number(displayedQuestion_.species.id),displayedQuestion_.identificationRank,hint.id,hint.text,
                 hintKey(hint.type),assistanceLabel(hint.assistance),displayedQuestion_.options.join(" | "),
                 hint.assessmentPayload.value("evaluation").toObject().value("policy_version").toString(),hint.assessmentId,problem.left(3000));
        QUrl url("https://github.com/kaduolliver/Projeto-Glossata/issues/new");QUrlQuery query;
        query.addQueryItem("title","Problema em dica / organismo "+QString::number(displayedQuestion_.species.id));
        query.addQueryItem("body",text);url.setQuery(query);
        return {{"text",text},{"url",url.toString(QUrl::FullyEncoded)}};
    }
    return {};
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
    resultPending_ = service_.saveCurrentResult() == GameService::SaveStatus::Failed;
    feedbackText_ = resultPending_
        ? QStringLiteral("Não foi possível salvar a pontuação. Tente salvar novamente antes de fechar o jogo.")
        : QStringLiteral("Partida encerrada. Pontuação: %1.").arg(scoreText());
    emit resultPendingChanged();
    emit feedbackTextChanged();
    loadRanking();
}

void GameController::loadRanking()
{
    const QString key = rankingMode_ == "competitive" ? GameService::competitiveConfiguration().rankingKey() : QString("free");
    auto row = [](const HistoryEntry &entry) {
        return QVariantMap{{"playerName",entry.playerName},{"playerId",entry.playerId},{"snapshotName",entry.snapshotName},
            {"legacyIdentity",entry.legacyIdentity},{"deletedProfile",entry.deletedProfile},{"scoreUnits",entry.scoreUnits},
            {"scoreText",formatScore(entry.scoreUnits)},{"correctAnswers",entry.correctAnswers},
            {"playedAt",entry.playedAt.toString("dd/MM/yyyy")},{"lastScoreText",formatScore(entry.lastScoreUnits)},
            {"lastScoreUnits",entry.lastScoreUnits},{"lastCorrectAnswers",entry.lastCorrectAnswers},
            {"lastPlayedAt",entry.lastPlayedAt.toString("dd/MM/yyyy")}};
    };
    ranking_.clear();
    for (const auto &entry : historyRepository_.loadRanking(key,5)) ranking_.append(row(entry));
    const auto personal = historyRepository_.loadPlayerResult(key,currentPlayerId_);
    personalRecord_ = personal.isEmpty() ? QVariantMap{} : row(personal.first());
    playerMetrics_ = presentMetrics(historyRepository_.playerMetrics(key,currentPlayerId_));
    emit rankingChanged();
}

QVariantList GameController::categories() const
{
    return taxonomy_.domains();
}

QVariantList GameController::categoryStats() const
{
    QVariantList list;
    const auto performance=service_.performance();
    for (const auto &group : performance.groups) {
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
    const auto created = historyRepository_.createPlayer(profileName);
    if (!created.selectable()) {
        feedbackText_ = QStringLiteral("Nome inválido ou já cadastrado.");
        emit feedbackTextChanged();
        return false;
    }

    emit playerProfilesChanged();
    selectPlayerProfile(created.id);
    feedbackText_ = QStringLiteral("Perfil criado.");
    emit feedbackTextChanged();
    return true;
}

bool GameController::renameCurrentPlayerProfile(const QString &name)
{
    const QString profileName = name.trimmed();
    if (profileName.isEmpty() || profileName.size() > 32
        || !historyRepository_.renamePlayer(currentPlayerId_, profileName)) {
        feedbackText_ = QStringLiteral("Não foi possível renomear o perfil.");
        emit feedbackTextChanged();
        return false;
    }

    currentPlayerName_ = profileName;
    emit currentPlayerNameChanged();
    loadRanking();
    emit playerProfilesChanged();
    feedbackText_ = QStringLiteral("Perfil renomeado.");
    emit feedbackTextChanged();
    return true;
}

bool GameController::deleteCurrentPlayerProfile()
{
    if (playerProfiles().size() <= 1 || !historyRepository_.deletePlayer(currentPlayerId_)) {
        feedbackText_ = QStringLiteral("Mantenha ao menos um perfil cadastrado.");
        emit feedbackTextChanged();
        return false;
    }

    const auto next = historyRepository_.profiles().first();
    currentPlayerName_ = next.displayName;
    currentPlayerId_ = next.id;
    emit currentPlayerNameChanged();
    loadRanking();
    emit playerProfilesChanged();
    feedbackText_ = QStringLiteral("Perfil removido.");
    emit feedbackTextChanged();
    return true;
}
