#include "application/GameService.h"

#include <algorithm>
#include <random>
#include <limits>
#include "domain/QuizSelection.h"
#include <QMap>
#include <QSet>
#include <QUuid>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonArray>
#include <sstream>
#include <locale>

GameService::GameService(SpeciesCatalog *catalog, GameHistoryRepository *history,const EducationalContent &content)
    : catalog_(catalog), history_(history), content_(content)
{
}

void GameService::startNewGame(const QString &filter, const QString &rank, const PlayerIdentity &player)
{
    GameConfiguration configuration;
    configuration.group = filter;
    configuration.rank = rank;
    startConfiguredGame(configuration, player);
}

GameConfiguration GameService::competitiveConfiguration()
{
    GameConfiguration configuration;
    configuration.mode = GameMode::Competitive;
    configuration.questionLimit = GameConfiguration::CompetitiveQuestionCount;
    configuration.rank = "genus";
    static const QString fingerprint = [] {
        QFile file(":/resources/data/education.json");
        return file.open(QIODevice::ReadOnly) ? contentFingerprint(file.readAll()) : QString();
    }();
    configuration.contentVersion = fingerprint;
    return configuration;
}

QString GameService::contentFingerprint(const QByteArray &education)
{
    QByteArray bytes;
    for (const QString &path : {QString(":/resources/data/species.json"), QString(":/resources/data/taxonomy.json")}) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return {};
        bytes+=file.readAll();
    }
    bytes+=education; // Includes assistance grades, scopes, bindings and review states.
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}

void GameService::startConfiguredGame(GameConfiguration configuration, const PlayerIdentity &player)
{
    // Legacy rules interpret stored games; new sessions always use current rules.
    configuration.rulesVersion=CurrentScoringRules;
    const QString filter = configuration.group;
    const QString rank = configuration.rank;
    identificationRank_ = rank;
    species_.clear();
    currentSpeciesIndex_ = 0;
    resultSaved_ = false;
    revealedHints_ = 0;
    session_ = GameSession();
    session_.player = player;
    session_.configuration = configuration;
    session_.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    performance_ = TaxonomicPerformance();
    currentQuestion_ = Question();
    checkpointRevision_=0;
    checkpointError_.clear();
    presentedQuestions_=QJsonArray();
    contentIdentity_=contentIdentity();

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
    if (configuration.mode == GameMode::Competitive) {
        for (int i = int(species_.size()) - 1; i >= 0; --i)
            if (species_[i].photo.role != "identification") species_.removeAt(i);
    }
    optionPool_ = species_;
    const QuestionGenerator generator(content_);
    if (configuration.difficulty == QuestionDifficulty::Close)
        for (int i = int(species_.size()) - 1; i >= 0; --i)
            if (generator.generate(species_[i], optionPool_, configuration, random_).options.size() != 4) species_.removeAt(i);
    if (configuration.mode == GameMode::Competitive && (configuration.questionLimit <= 0 || species_.size() < configuration.questionLimit)) {
        session_.round = 0;
        session_.finished = true;
        return;
    }
    QSet<QString> distinctAnswers;
    for (const Species &species : species_) distinctAnswers.insert(species.answerAtRank(rank).toCaseFolded());
    if (distinctAnswers.size() < 4) {
        session_.round = 0;
        session_.finished = true;
        return;
    }

    shuffleCycle();
    buildQuestionForCurrentSpecies();
    if (!session_.finished && !checkpointProgress()) {
        session_.finished=true;
        currentQuestion_.options.clear();
    }
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
    return hintRevealed() ? currentQuestion_.extraHint : QString();
}

bool GameService::hintRevealed() const
{
    return revealedHints_ & hintBit(HintType::Morphological);
}

bool GameService::submitAnswer(const QString &selectedOption)
{
    if (session_.finished || !currentQuestion_.options.contains(selectedOption)) {
        return false;
    }
    if (!ensureCheckpoint()) return false;
    const GameService before=*this;

    const bool isCorrect = selectedOption == currentQuestion_.correctAnswer;
    const int maximum=possibleScoreUnits();
    session_.answers.append({session_.round,currentQuestion_.species.id,currentQuestion_.species.name,
        currentQuestion_.photo.id,currentQuestion_.identificationRank,selectedOption,currentQuestion_.correctAnswer,
        isCorrect,revealedHints_,maximum,isCorrect ? maximum : 0});
    auto &record=session_.answers.last();
    record.rulesVersion=session_.configuration.rulesVersion;
    record.options=currentQuestion_.options;
    record.contentVersion=content_.pedagogyVersion();
    for (int i=0;i<3;++i) if (revealedHints_ & (1<<i)) {
        record.ceilings[i]=currentQuestion_.hints[i].ceiling();
        record.assessmentIds[i]=currentQuestion_.hints[i].assessmentId;
        record.assessmentPayloads[i]=currentQuestion_.hints[i].assessmentPayload;
    }
    performance_.record(currentQuestion_.species,isCorrect);
    ++session_.answeredCount;
    if (isCorrect) {
        session_.scoreUnits+=maximum;
        ++session_.correctAnswers;
    }

    if (session_.configuration.mode == GameMode::Competitive
        && session_.answeredCount >= session_.configuration.questionLimit) {
        session_.finished = true;
        revealedHints_ = 0;
        currentQuestion_.options.clear();
        if (!checkpointProgress()) { const auto error=checkpointError_; *this=before; checkpointError_=error; return false; }
        return isCorrect;
    }
    ++session_.round;
    ++currentSpeciesIndex_;
    revealedHints_ = 0;
    if (currentSpeciesIndex_ >= species_.size()) {
        const QString previousSpeciesName = species_.last().name;
        shuffleCycle();
        if (species_.size() > 1 && species_.front().name == previousSpeciesName) {
            std::swap(species_[0], species_[1]);
        }
        currentSpeciesIndex_ = 0;
    }
    buildQuestionForCurrentSpecies();
    if (!checkpointProgress()) { const auto error=checkpointError_; *this=before; checkpointError_=error; return false; }
    return isCorrect;
}

bool GameService::revealHint()
{
    return revealHintSlot(2);
}
bool GameService::revealHint(HintType type)
{
    for(int i=0;i<3;++i)if(currentQuestion_.hints[i].type==type)return revealHintSlot(i);
    return false;
}
bool GameService::revealHintSlot(int slot)
{
    if(session_.finished || slot<0 || slot>2 || (revealedHints_&(1<<slot)) || !currentQuestion_.hints[slot].available())return false;
    if (!ensureCheckpoint()) return false;
    const int before=revealedHints_;
    revealedHints_|=1<<slot;
    if (!checkpointProgress()) { revealedHints_=before; return false; }
    return true;
}

void GameService::finishGame()
{
    if (session_.finished) {
        return;
    }

    if (!ensureCheckpoint()) return;
    const auto before=currentQuestion_;
    session_.finished = true;
    currentQuestion_.options.clear();
    if (!checkpointProgress()) { session_.finished=false; currentQuestion_=before; }
}

qint64 GameService::scoreUnits() const
{
    return session_.scoreUnits;
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

    return history_->loadRanking(session_.configuration.rankingKey(), 5);
}

GameService::SaveStatus GameService::saveCurrentResult()
{
    if (!session_.finished || resultSaved_) {
        return SaveStatus::NotNeeded;
    }
    if (session_.answeredCount==0) {
        if (hasCheckpoint() && !checkpoints_->abandonCheckpoint(session_.id,checkpointRevision_)) return SaveStatus::Failed;
        resultSaved_=true;
        return SaveStatus::NotNeeded;
    }

    if (history_ == nullptr || !session_.player.isValid()
        || !history_->saveConfiguredResult(session_.player, session_.scoreUnits, session_.correctAnswers, session_.answers, session_.id, session_.configuration)) {
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

    currentQuestion_ = QuestionGenerator(content_).generate(species_.at(currentSpeciesIndex_), optionPool_, session_.configuration, random_);
    if (currentQuestion_.options.size() != 4) session_.finished = true;
    else presentedQuestions_.append(SessionSnapshot::question(currentQuestion_));
}

void GameService::shuffleCycle()
{
    auto &random=random_;
    QMap<QString, QVector<Species>> groups;
    for (const Species &species : species_) groups[species.displayGroupId()].append(species);
    const int total = static_cast<int>(species_.size());
    QMap<QString, int> weight, balance;
    QHash<int,int> completeness;
    if(session_.configuration.mode==GameMode::Competitive) {
        std::mt19937 assessmentRandom(42);
        for(const auto &s:species_) {
            const auto q=QuestionGenerator(content_).generate(s,optionPool_,session_.configuration,assessmentRandom);
            completeness[s.id]=std::count_if(q.hints.begin(),q.hints.end(),[](const Hint &h){return h.available();});
        }
    }
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        std::shuffle(it.value().begin(), it.value().end(), random);
        if(!completeness.isEmpty())std::stable_sort(it.value().begin(),it.value().end(),[&](const Species &a,const Species &b){return completeness.value(a.id)>completeness.value(b.id);});
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

QString GameService::contentIdentity() const
{
    QByteArray bytes=contentFingerprint(content_.identity().toUtf8()).toUtf8();
    if (catalog_) for (const auto &s:catalog_->loadSpecies()) {
        QJsonArray taxa,traits; for (const auto &t:s.taxonomy) taxa.append(QJsonObject{{"id",t.id},{"name",t.name},{"rank",t.rank}});
        for (const auto &t:s.morphologyTraits) traits.append(t);
        bytes+=QJsonDocument(QJsonObject{{"id",s.id},{"name",s.name},{"photo",s.photo.id},{"sha256",s.photo.sha256},
            {"photo_valid",s.photo.valid},{"photo_approved",s.photoApproved},{"taxonomy_approved",s.taxonomyApproved},
            {"phylogeny_approved",s.phylogenyApproved},{"taxonomy",taxa},{"traits",traits},{"attribution",s.photo.attribution}}).toJson(QJsonDocument::Compact);
    }
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}

QJsonObject GameService::sessionSnapshot() const
{
    QJsonArray answers,order,pool;
    for (const auto &a:session_.answers) answers.append(SessionSnapshot::answer(a));
    for (const auto &s:species_) order.append(s.id);
    for (const auto &s:optionPool_) pool.append(s.id);
    std::ostringstream generator; generator.imbue(std::locale::classic()); generator<<random_;
    return {{"schema",SessionSnapshot::Schema},{"id",session_.id},{"player_id",session_.player.id},{"player_name",session_.player.displayName},
        {"configuration",SessionSnapshot::configuration(session_.configuration)},{"content_identity",contentIdentity_},
        {"pedagogy_version",content_.pedagogyVersion()},{"units",double(session_.scoreUnits)},{"correct",session_.correctAnswers},
        {"round",session_.round},{"finished",session_.finished},{"revealed",revealedHints_},{"answers",answers},{"questions",presentedQuestions_},
        {"species_order",order},{"option_pool",pool},{"species_index",currentSpeciesIndex_},{"generator",QString::fromStdString(generator.str())}};
}

bool GameService::checkpointProgress()
{
    if (!checkpoints_) return true;
    if (resultSaved_) return true;
    if (!session_.player.isValid() || !checkpoints_->writeCheckpoint(sessionSnapshot(),checkpointRevision_,checkpointRevision_)) {
        checkpointError_=QStringLiteral("Não foi possível salvar o progresso. A ação não foi confirmada. Verifique espaço, acesso ao banco e se outra janela retomou esta partida.");
        return false;
    }
    checkpointError_.clear(); return true;
}

bool GameService::ensureCheckpoint()
{
    return !checkpoints_ || checkpointRevision_>0 || checkpointProgress();
}

bool GameService::recoverSession(const QString &id)
{
    checkpointError_=QStringLiteral("A sessão não pode ser retomada: integridade, regras ou conteúdo incompatíveis. O registro foi preservado; você pode abandoná-lo sem alterar o ranking.");
    if (!checkpoints_ || !catalog_) return false;
    const auto saved=checkpoints_->checkpoint(id); const auto p=saved.payload;
    if (!saved.error.isEmpty() || saved.revision<=0 || (saved.state!="active" && saved.state!="finished")
        || !SessionSnapshot::valid(p) || p["content_identity"].toString()!=contentIdentity()) {
        return false;
    }
    const auto loaded=catalog_->loadSpecies();
    QVector<Species> order,pool;
    auto load=[&](const QJsonArray &ids,QVector<Species> &result) {
        for (const auto &v:ids) {
            auto it=std::find_if(loaded.begin(),loaded.end(),[&](const Species &s){return s.id==v.toInt();});
            if (it==loaded.end()) return false;
            auto s=*it; s.name=s.name.trimmed(); s.fact=s.fact.trimmed(); s.hint=s.hint.trimmed(); result.append(s);
        }
        return true;
    };
    if (!load(p["species_order"].toArray(),order) || !load(p["option_pool"].toArray(),pool)) return false;
    Question question;
    for (const auto &v:p["questions"].toArray()) if (!SessionSnapshot::readQuestion(v.toObject(),pool,question)) return false;
    std::istringstream generator(p["generator"].toString().toStdString()); generator.imbue(std::locale::classic()); std::mt19937 random;
    if (!(generator>>random)) return false;
    generator>>std::ws; if (!generator.eof()) return false;
    GameSession session; session.id=id; session.player={p["player_id"].toString(),p["player_name"].toString()};
    if (!SessionSnapshot::readConfiguration(p["configuration"].toObject(),session.configuration)) return false;
    TaxonomicPerformance performance;
    for (const auto &v:p["answers"].toArray()) {
        AnswerRecord a; if (!SessionSnapshot::readAnswer(v.toObject(),a)) return false; session.answers.append(a);
        auto it=std::find_if(pool.begin(),pool.end(),[&](const Species &s){return s.id==a.speciesId;}); if (it==pool.end()) return false;
        performance.record(*it,a.correct);
    }
    session.scoreUnits=qint64(p["units"].toDouble()); session.correctAnswers=p["correct"].toInt(); session.answeredCount=session.answers.size();
    session.round=p["round"].toInt(); session.finished=p["finished"].toBool();
    session_=session; species_=order; optionPool_=pool; currentSpeciesIndex_=p["species_index"].toInt(); random_=random;
    currentQuestion_=question; if (session.finished) currentQuestion_.options.clear();
    identificationRank_=session.configuration.rank; revealedHints_=p["revealed"].toInt(); performance_=performance;
    checkpointRevision_=saved.revision; resultSaved_=false; presentedQuestions_=p["questions"].toArray();
    contentIdentity_=p["content_identity"].toString(); checkpointError_.clear(); return true;
}
