#include "application/EducationalContent.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include "infrastructure/content/ReviewStatus.h"
namespace {
QVariantMap note(const QJsonObject &item)
{
    if (!usableReview(item.value("governance").toObject()) || item.value("text").toString().isEmpty()
        || item.value("source_url").toString().isEmpty()) return {};
    return item.toVariantMap();
}
bool leaks(const QString &text, const Species &s, const QStringList &options)
{
    QStringList forbidden=options;
    forbidden << s.name << s.name.section(' ',0,0);
    for (const Taxon &node : s.taxonomy) forbidden << node.name;
    for (const QString &name : forbidden) {
        if (name.size()<3) continue;
        const QRegularExpression pattern("(?<![\\p{L}])" + QRegularExpression::escape(name) + "(?![\\p{L}])",
            QRegularExpression::CaseInsensitiveOption);
        if (pattern.match(text).hasMatch()) return true;
    }
    return false;
}
}
EducationalContent::EducationalContent()
{
    QFile file(":/resources/data/education.json");
    if (file.open(QIODevice::ReadOnly)) data_=QJsonDocument::fromJson(file.readAll()).object();
}
QJsonObject EducationalContent::resolve(const QJsonValue &reference) const
{
    return reference.isString() ? data_.value("notes").toObject().value(reference.toString()).toObject() : reference.toObject();
}
QJsonObject EducationalContent::speciesContent(const Species &s) const
{
    const auto item=data_.value("species").toObject().value(QString::number(s.id)).toObject();
    return item.value("scientific_name").toString()==s.name ? item : QJsonObject{};
}
QVariantMap EducationalContent::nameOrigin(const Species &s) const
{
    const auto topic=speciesContent(s).value("governance").toObject().value("etymology").toObject();
    const auto refs=speciesContent(s).value("name_origin").toObject().value("parts").toArray();
    QStringList texts,urls;
    QString title;
    for(const auto &ref : refs) {
        if(!usableScientificHintNote(resolve(ref),topic,"etymology"))return {};
        const auto part=note(resolve(ref));if(part.isEmpty())return {};
        texts.append(part.value("text").toString());
        const auto url=part.value("source_url").toString();
        if(!urls.contains(url))urls.append(url);
        if(title.isEmpty())title=part.value("source_title").toString();
    }
    return texts.isEmpty() ? QVariantMap{} : QVariantMap{{"text",texts.join(' ')},{"source_url",urls.first()},
        {"source_title",title},{"source_urls",urls},{"verified",true}};
}
QuestionHints EducationalContent::questionHints(const Species &s,const QString &rank,const QStringList &options,
                                               const QVector<Species> &pool,bool competitive) const
{
    if (data_.contains("automated_pedagogy")) return progressiveHints(s,rank,options,pool);
    QuestionHints result;
    for(int i=0;i<3;++i) {
        result[i].type=static_cast<HintType>(i+1);
    }
    result[0].unavailableReason=QStringLiteral("Sem contexto seguro para estas alternativas.");
    result[1].unavailableReason=QStringLiteral("Origem do nome não verificada para este nível.");
    result[2].unavailableReason=QStringLiteral("Sem característica física adequada a esta questão.");
    if (options.size()!=4 || QSet<QString>(options.begin(),options.end()).size()!=4 || !options.contains(s.answerAtRank(rank))) return result;
    const auto item=speciesContent(s);
    auto mentionsOption=[&options](const QString &text) {
        for(const QString &option : options) {
            const QRegularExpression pattern("(?<![\\p{L}])"+QRegularExpression::escape(option)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
            if(pattern.match(text).hasMatch())return true;
        }
        return false;
    };
    auto assign=[&](int index,const QVariantMap &item) {
        if(item.value("text").toString().isEmpty() || item.value("source_url").toString().isEmpty()
            || mentionsOption(item.value("text").toString()))return;
        auto &hint=result[index];
        const QVariantMap assessment=item.value("assistance").toMap();
        hint.assistance=assistanceLevel(assessment.value("grade").toString());
        const QJsonObject review=QJsonObject::fromVariantMap(assessment.value("governance").toMap());
        hint.assessmentState=review.value("state").toString();
        hint.rationale=assessment.value("rationale").toString();
        hint.assessmentId=review.value("content_sha256").toString();
        hint.assessmentPayload=QJsonObject::fromVariantMap(assessment.value("binding").toMap());
        const bool approved=usableReview(review) && review.value("claim_scope").toString()=="hint_assistance"
            && (hint.assessmentState=="approved_editorial" || hint.assessmentState=="approved_expert");
        hint.provisional=!approved;
        if (hint.ceiling()==0 || hint.rationale.isEmpty() || hint.assessmentId.size()!=64
            || !assessment.value("ranks").toList().contains(rank)) {
            hint.unavailableReason=QStringLiteral("Assistência sem classificação válida para este nível."); return;
        }
        auto declared=QJsonObject::fromVariantMap(assessment);declared.remove("governance");declared.remove("binding");
        const auto boundSource=hint.assessmentPayload.value("source").toObject();
        if (declared!=hint.assessmentPayload.value("assessment").toObject()
            || hint.assessmentPayload.value("photo").toObject().value("sha256").toString()!=s.photo.sha256
            || (index!=0 && (boundSource.value("text").toString()!=item.value("text").toString()
                            || boundSource.value("source_url").toString()!=item.value("source_url").toString()))) {
            hint.unavailableReason=QStringLiteral("Texto ou avaliação de assistência alterados; regenere e revise o conteúdo.");return;
        }
        AnswerRecord proof{1,s.id,s.name,s.photo.id,rank,s.answerAtRank(rank),s.answerAtRank(rank),true,
            hintBit(hint.type),hint.ceiling(),hint.ceiling()};
        proof.rulesVersion=PreviousScoringRules;proof.ceilings[index]=hint.ceiling();
        proof.options=options;
        proof.assessmentIds[index]=hint.assessmentId;proof.assessmentPayloads[index]=hint.assessmentPayload;
        if (!validAnswerScoring(proof,PreviousScoringRules)) {
            hint.unavailableReason=QStringLiteral("Avaliação de assistência incompatível com o conteúdo, nível ou alternativas; requer contexto documentado.");return;
        }
        if (assessment.value("suitability").toString()=="inadequate") {
            hint.unavailableReason=QStringLiteral("Conteúdo inadequado como dica prévia; disponível na aprendizagem após a resposta."); return;
        }
        if ((competitive && !approved) || ((!approved) && (hint.assessmentState=="approved_editorial" || hint.assessmentState=="approved_expert"))
            || hint.assessmentState=="rejected" || hint.assessmentState=="expired" || hint.assessmentState=="needs_review") {
            hint.unavailableReason=QStringLiteral("Classificação de assistência pendente ou vencida; exige revisão humana rastreável."); return;
        }
        result[index].text=item.value("text").toString();
        result[index].sourceUrl=item.value("source_url").toString();
        result[index].sourceTitle=item.value("source_title").toString();
        result[index].unavailableReason.clear();
    };
    // Only a broader, verified ancestor is permitted. At least three distinct
    // choices must remain positively compatible. All four require known ancestry;
    // missing data is never evidence of exclusion. Counts gate safety, not grade.
    const QStringList ranks={"domain","supergroup","kingdom","phylum","class","order","family","genus","species"};
    const int asked=ranks.indexOf(rank);
    for(auto it=s.taxonomy.crbegin();it!=s.taxonomy.crend();++it) {
        if (!s.taxonomyApproved) break;
        const int level=ranks.indexOf(it->rank);
        if(level<0 || asked<0 || level>=asked || it->source!="inaturalist" || it->sourceTaxonId.isEmpty())continue;
        int compatible=0,knownOptions=0;
        for(const QString &option : options) {
            bool known=false,complete=true,all=true;
            for(const Species &candidate : pool) if(candidate.answerAtRank(rank)==option) {
                known=true; all=all && candidate.belongsTo(it->id);
                complete=complete && std::any_of(candidate.taxonomy.begin(),candidate.taxonomy.end(),[&](const Taxon &node){return node.rank==it->rank && !node.sourceTaxonId.isEmpty();});
            }
            compatible+=known && complete && all;
            knownOptions+=known && complete;
        }
        if(knownOptions!=4 || compatible<3 || (it->rank=="kingdom" && compatible==options.size()))continue;
        assign(0,{{"text",item.value("taxonomic_assistance").toObject().value("template").toString().arg(taxonomicRankLabel(it->rank),it->name)},
                  {"source_url","https://www.inaturalist.org/taxa/"+it->sourceTaxonId},
                  {"source_title","iNaturalist · classificação do grupo"},{"assistance",item.value("taxonomic_assistance").toVariant()}});
        if(result[0].available())break;
    }
    const auto etymology=resolve(item.value("etymology_hints").toObject().value(rank));
    if (usableScientificHintNote(etymology,item.value("governance").toObject().value("etymology").toObject(),"etymology"))
        assign(1,note(etymology));
    // Existing reviewed appearance notes are reused, but habitat/diet clues and
    // vague group fallbacks must not silently become high-assistance morphology.
    const auto morphology=note(resolve(item.value("morphological_hint")));
    bool distinguishes = true;
    if (!s.morphologyTraits.isEmpty()) {
        int documentedAlternatives = 0, sharedAlternatives = 0;
        for (const auto &option : options) {
            if (option == s.answerAtRank(rank)) continue;
            bool documented = false, shared = false;
            for (const auto &candidate : pool) if (candidate.answerAtRank(rank)==option) {
                documented = documented || !candidate.morphologyTraits.isEmpty();
                shared = shared || std::all_of(s.morphologyTraits.begin(),s.morphologyTraits.end(),
                    [&](const QString &trait) { return candidate.morphologyTraits.contains(trait); });
            }
            documentedAlternatives += documented;
            sharedAlternatives += shared;
        }
        distinguishes = documentedAlternatives < 3 || sharedAlternatives < 3;
    }
    if(distinguishes && usableScientificHintNote(QJsonObject::fromVariantMap(morphology),item.value("governance").toObject().value("morphology").toObject(),"morphology")
       && morphology.value("ranks").toList().contains(rank)
       && morphology.value("photo_id").toString()==s.photo.id
       && !leaks(morphology.value("text").toString(),s,options))assign(2,morphology);
    return result;
}
QuestionHints EducationalContent::progressiveHints(const Species &s,const QString &rank,const QStringList &options,
                                                    const QVector<Species> &pool) const
{
    QuestionHints result;
    for(int i=0;i<3;++i) {
        result[i].id=QString("slot-%1").arg(i);
        result[i].assistance=i==0 ? AssistanceLevel::Light : i==1 ? AssistanceLevel::Moderate : AssistanceLevel::Strong;
        result[i].unavailableReason=QStringLiteral("Não há conteúdo distinto com evidência suficiente para este grau e estas alternativas.");
    }
    const auto automatic=data_.value("automated_pedagogy").toObject();
    const auto evaluation=automatic.value("evaluation").toObject();
    auto body=automatic;body.remove("evaluation");body.remove("usable");
    const auto contentVersion=evaluation.value("content_sha256").toString();
    if(verifiedAutomaticHash_.isEmpty())verifiedAutomaticHash_=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(body).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
    const auto hash=verifiedAutomaticHash_;
    const auto due=QDateTime::fromString(evaluation.value("due_at").toString(),Qt::ISODateWithMs);
    const auto reviewed=QDateTime::fromString(evaluation.value("reviewed_at").toString(),Qt::ISODateWithMs);
    const auto entry=automatic.value("species").toObject().value(QString::number(s.id)).toObject();
    if(!automatic.value("usable").toBool() || hash!=contentVersion || !due.isValid() || !reviewed.isValid()
        || reviewed>QDateTime::currentDateTimeUtc() || due<=QDateTime::currentDateTimeUtc()
        || entry.value("scientific_name").toString()!=s.name
        || entry.value("photo").toObject().value("id").toString()!=s.photo.id
        || entry.value("photo").toObject().value("sha256").toString()!=s.photo.sha256
        || !s.taxonomyApproved || options.size()!=4 || QSet<QString>(options.begin(),options.end()).size()!=4
        || !options.contains(s.answerAtRank(rank))) {
        for(auto &h:result)h.unavailableReason=QStringLiteral("Avaliação pedagógica vencida/alterada ou vínculo de identidade e foto incompatível; é necessária nova avaliação documentada.");
        return result;
    }
    const QStringList ranks{"kingdom","phylum","class","order","family","genus","species"};
    const int asked=ranks.indexOf(rank);
    QSet<QString> used;
    int previousAncestor=-1;
    for(int slot=0;slot<3;++slot) {
        // Prefer educational diversity at a compatible grade; never relabel a
        // weak description as strong to fill a slot. Then try sourced ancestry.
        QJsonArray candidates;
        for(const auto &v:entry.value("recipes").toArray())if(v.toObject().value("kind")=="note")candidates.append(v);
        const auto recipes=entry.value("recipes").toArray();
        if(slot==1) {
            for(const auto &v:recipes)if(v.toObject().value("kind")=="ancestor")candidates.append(v);
        } else for(int r=int(recipes.size())-1;r>=0;--r)if(recipes[r].toObject().value("kind")=="ancestor")candidates.append(recipes[r]);
        for(const auto &value:candidates) {
            const auto recipe=value.toObject();const auto id=recipe.value("id").toString();
            if(used.contains(id))continue;
            const auto type=hintType(recipe.value("category").toString());
            QString text=recipe.value("text").toString(), url=recipe.value("source_url").toString();
            QJsonObject evidence;int ancestorLevel=-1;
            if(recipe.value("kind")=="note") {
                if(!recipe.value("ranks").toArray().contains(rank)
                    || assistanceCeiling(assistanceLevel(recipe.value("grade").toString()))!=result[slot].ceiling())continue;
                const auto item=speciesContent(s);const auto topic=recipe.value("scientific_topic").toString();
                const auto source=resolve(recipe.value("source_note"));
                if(!usableScientificHintNote(source,item.value("governance").toObject().value(topic).toObject(),topic)
                    || (type==HintType::Morphological && source.value("photo_id").toString()!=s.photo.id)
                    || source.value("text").toString()!=recipe.value("source_text").toString(text) || source.value("source_url").toString()!=url)continue;
                if(recipe.value("organism_scope").toBool() && !recipe.value("original_ranks").toArray().contains(rank))
                    text=QStringLiteral("Sobre este organismo, sem generalizar para todo o grupo: ")+text;
                const auto token=recipe.value("association_token").toString();
                if(!token.isEmpty()) {
                    const QRegularExpression match("(?<![\\p{L}])"+QRegularExpression::escape(token)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
                    int count=0;for(const auto &option:options)count+=match.match(option).hasMatch();
                    if(count!=1 || !match.match(s.answerAtRank(rank)).hasMatch())continue;
                }
            } else {
                const auto n=recipe.value("node").toObject();ancestorLevel=ranks.indexOf(n.value("rank").toString());
                if(asked<0 || ancestorLevel<0 || ancestorLevel>=asked || ancestorLevel<=previousAncestor
                    || !s.belongsTo(n.value("id").toString()))continue;
                // Specificity and additional sourced information are primary;
                // compatibility counts only constrain safe contextual selection.
                if((slot==0 && ancestorLevel>asked-2) || (slot==1 && ancestorLevel<asked-3)
                    || (slot==2 && ancestorLevel<asked-2))continue;
                int compatible=0;bool complete=true;
                for(const auto &option:options) {
                    bool known=false,all=true,none=true;QJsonArray records;
                    for(const auto &candidate:pool)if(candidate.answerAtRank(rank)==option) {
                        if(!candidate.taxonomyApproved) {complete=false;break;}
                        auto found=std::find_if(candidate.taxonomy.cbegin(),candidate.taxonomy.cend(),[&](const Taxon &t){return t.rank==n.value("rank").toString() && t.source=="inaturalist" && !t.sourceTaxonId.isEmpty();});
                        if(found==candidate.taxonomy.cend()){complete=false;break;}
                        known=true;const bool match=found->id==n.value("id").toString();all=all&&match;none=none&&!match;
                        records.append(QJsonObject{{"species_id",QString::number(candidate.id)},{"ancestor_id",found->id},
                            {"source_url","https://www.inaturalist.org/taxa/"+found->sourceTaxonId},{"locator","taxon / ancestors"}});
                    }
                    complete=complete&&known&&(all||none);compatible+=known&&all;
                    evidence[option]=QJsonObject{{"known",known},{"compatible",known&&all},{"records",records}};
                }
                if(!complete || (slot==0 && compatible<3) || (slot==1 && (compatible<2 || compatible>3)) || (slot==2 && compatible>2))continue;
                text=QStringLiteral("O organismo fotografado pertence a %1: %2. %3")
                    .arg(taxonomicRankLabel(n.value("rank").toString()),n.value("name").toString(),
                         slot==0 ? QStringLiteral("É um contexto amplo, compartilhado por várias opções.")
                                 : QStringLiteral("Compare a classificação das alternativas; esta informação se refere ao organismo, não a todas as espécies das opções."));
            }
            // Scientific names inside translations are not the only disclosure
            // risk. Existing curated etymological grades remain independent.
            const QRegularExpression leak("(?<![\\p{L}])"+QRegularExpression::escape(s.answerAtRank(rank))+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
            bool mentions=false;for(const auto &option:options)mentions=mentions||text.contains(option,Qt::CaseInsensitive);
            if(text.isEmpty() || leak.match(text).hasMatch() || mentions)continue;
            QJsonObject proof{{"schema",2},{"slot",slot},{"hint_id",QString::number(s.id)+":"+rank+":"+id},
                {"species_id",QString::number(s.id)},{"scientific_name",s.name},{"photo",entry.value("photo")},
                {"rank",rank},{"category",hintKey(type)},{"kind",recipe.value("kind")},{"text",text},
                {"source_url",url},{"locator",recipe.value("locator")},{"rationale",recipe.value("rationale")},
                {"evaluation",evaluation},{"content_version",contentVersion},{"options",QJsonArray::fromStringList(options)},
                {"option_evidence",evidence},{"source_recipe",recipe}};
            auto &h=result[slot];h.type=type;h.id=proof.value("hint_id").toString();h.text=text;h.sourceUrl=url;
            h.sourceTitle=recipe.value("source_title").toString();h.rationale=recipe.value("rationale").toString();
            h.assessmentState="approved_automated";h.assessmentPayload=proof;
            h.assessmentId=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(proof).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
            AnswerRecord check{1,s.id,s.name,s.photo.id,rank,s.answerAtRank(rank),s.answerAtRank(rank),true,1<<slot,h.ceiling(),h.ceiling()};
            check.rulesVersion=CurrentScoringRules;check.options=options;check.contentVersion=contentVersion;
            check.ceilings[slot]=h.ceiling();check.assessmentIds[slot]=h.assessmentId;check.assessmentPayloads[slot]=proof;
            if(!validAnswerScoring(check,CurrentScoringRules)){h.text.clear();continue;}
            h.unavailableReason.clear();used.insert(id);if(ancestorLevel>=0)previousAncestor=ancestorLevel;break;
        }
    }
    return result;
}
QVariantMap EducationalContent::profile(const Species &s) const
{
    const auto item=speciesContent(s);
    if (item.isEmpty()) return {};
    QVariantMap result{{"commonName",item.value("common_name").toString()},
                       {"displayName",item.value("display_name").toString()},
                       {"governance",item.value("governance").toVariant()},
                       {"reviewedOn",item.value("governance").toObject().value("education").toObject().value("reviewed_at").toString()},
                       {"issueUrl",data_.value("governance_policy").toObject().value("issue_url").toString()},
                       {"taxonomySystem",data_.value("governance_policy").toObject().value("taxonomy_system").toString()},
                       {"phylogeny",item.value("phylogeny").toObject().toVariantMap()}};
    for (const QString &field : {QString("summary"),QString("distribution"),QString("nutrition"),
                                QString("morphology"),QString("conservation"),QString("curiosity"),QString("hint")})
        result.insert(field,note(resolve(item.value(field))));
    const auto hints = item.value("hints").toArray();
    if (!hints.isEmpty()) result.insert("hint",note(resolve(hints.first())));
    return result;
}
QVariantMap EducationalContent::introduction(const QString &id) const { return note(resolve(data_.value("taxons").toObject().value(id))); }
QVariantMap EducationalContent::hint(const Species &s, const QString &rank, const QStringList &options) const
{
    QJsonArray candidates=speciesContent(s).value("hints").toArray();
    const QJsonObject scopes=data_.value("group_hints").toObject();
    for (auto it=s.taxonomy.crbegin(); it!=s.taxonomy.crend(); ++it)
        for (const auto &candidate : scopes.value(it->id).toArray()) candidates.append(candidate);
    for (const auto &candidate : candidates) {
        const QJsonObject item=resolve(candidate);
        if (!item.value("ranks").toArray().contains(rank) || note(item).isEmpty() || leaks(item.value("text").toString(),s,options)) continue;
        return note(item);
    }
    return {};
}
QVariantMap EducationalContent::curiosity(const Species &s) const
{
    auto result=note(resolve(speciesContent(s).value("curiosity")));
    if (!result.isEmpty()) return result;
    const QJsonObject scopes=data_.value("group_curiosities").toObject();
    for (auto it=s.taxonomy.crbegin(); it!=s.taxonomy.crend(); ++it) {
        result=note(resolve(scopes.value(it->id)));
        if (!result.isEmpty()) return result;
    }
    return {};
}
