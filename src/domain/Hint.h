#pragma once
#include <QString>
#include <array>
#include <QtGlobal>
#include <algorithm>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QSet>
#include <QRegularExpression>
#include <QStringList>
#include <QDateTime>

enum class HintType { None=0, Taxonomic=1, Etymological=2, Morphological=3 };
constexpr int FullScoreUnits=6;
constexpr int hintBit(HintType type) { return type<HintType::Taxonomic || type>HintType::Morphological ? 0 : 1 << (static_cast<int>(type)-1); }
enum class AssistanceLevel { Unclassified=0, Light=6, Moderate=3, Strong=2 };
inline const QString CurrentScoringRules = QStringLiteral("identification-v4-progressive");
inline const QString PreviousScoringRules = QStringLiteral("identification-v3-assistance");
inline const QString LegacyScoringRules = QStringLiteral("identification-v2");
constexpr int assistanceCeiling(AssistanceLevel level) {
    return level==AssistanceLevel::Light ? 6 : level==AssistanceLevel::Moderate ? 3 : level==AssistanceLevel::Strong ? 2 : 0;
}
inline AssistanceLevel assistanceLevel(const QString &key) {
    return key=="light" ? AssistanceLevel::Light : key=="moderate" ? AssistanceLevel::Moderate
        : key=="strong" ? AssistanceLevel::Strong : AssistanceLevel::Unclassified;
}
inline QString assistanceLabel(AssistanceLevel level) {
    return level==AssistanceLevel::Light ? QStringLiteral("Leve") : level==AssistanceLevel::Moderate ? QStringLiteral("Moderada")
        : level==AssistanceLevel::Strong ? QStringLiteral("Forte") : QStringLiteral("Não classificada");
}
// Frozen interpretation for historical v2/legacy records ONLY.
constexpr int legacyMaximumScoreUnits(int revealed) {
    return revealed & hintBit(HintType::Morphological) ? 2
         : revealed & hintBit(HintType::Etymological) ? 3 : FullScoreUnits;
}
inline int maximumScoreUnits(int revealed, const std::array<int,3> &ceilings) {
    if (revealed<0 || revealed>7) return 0;
    int result=FullScoreUnits;
    for (int i=0;i<3;++i) if (revealed & (1<<i)) {
        if (ceilings[i]!=2 && ceilings[i]!=3 && ceilings[i]!=6) return 0;
        result=std::min(result,ceilings[i]);
    }
    return result;
}
inline QString hintKey(HintType type) {
    switch(type) { case HintType::Taxonomic:return "taxonomic"; case HintType::Etymological:return "etymological";
        case HintType::Morphological:return "morphological"; default:return {}; }
}
inline HintType hintType(const QString &key) {
    for (auto type : {HintType::Taxonomic,HintType::Etymological,HintType::Morphological}) if(hintKey(type)==key)return type;
    return HintType::None;
}
// Rounding happens only here, at presentation. All accumulation stays integral.
inline QString formatScore(qint64 units) {
    const qint64 hundredths=(units%6*100+3)/6;
    return QString::number(units/6)+","+QString::number(hundredths).rightJustified(2,'0');
}
struct Hint {
    HintType type=HintType::None;
    QString id; // Stable content identity, independent of category and display slot.
    QString text,sourceUrl,sourceTitle,unavailableReason;
    AssistanceLevel assistance=AssistanceLevel::Unclassified;
    QString assessmentId,assessmentState,rationale;
    QJsonObject assessmentPayload;
    bool provisional=false;
    int ceiling() const { return assistanceCeiling(assistance); }
    bool available() const { return !text.isEmpty() && !sourceUrl.isEmpty() && ceiling()>0; }
};
using QuestionHints=std::array<Hint,3>;
inline std::array<int,3> hintCeilings(const QuestionHints &hints) {
    return {hints[0].ceiling(),hints[1].ceiling(),hints[2].ceiling()};
}
struct AnswerRecord {
    int questionNumber=0,speciesId=0;
    QString scientificName,photoId,rank,selectedAnswer,correctAnswer;
    bool correct=false;
    int revealedHints=0,maximumUnits=FullScoreUnits,awardedUnits=0;
    QString rulesVersion=LegacyScoringRules;
    std::array<int,3> ceilings{};
    std::array<QString,3> assessmentIds{};
    std::array<QJsonObject,3> assessmentPayloads{};
    QStringList options; // Empty only for legacy records that never stored them.
    QString contentVersion;
};
// Shared validation for the service, history writer and history reader. These
// snapshots preserve the policy at answer time, independently of future content.
inline bool validAnswerScoring(const AnswerRecord &answer,const QString &rules) {
    if (answer.rulesVersion!=rules || answer.revealedHints<0 || answer.revealedHints>7) return false;
    int maximum=0;
    if (rules==LegacyScoringRules || rules=="legacy") {
        maximum=legacyMaximumScoreUnits(answer.revealedHints);
        for (int i=0;i<3;++i) {
            const int expected=answer.revealedHints & (1<<i) ? legacyMaximumScoreUnits(1<<i) : 0;
            if (answer.ceilings[i]!=0 && answer.ceilings[i]!=expected) return false;
            if (!answer.assessmentIds[i].isEmpty() || !answer.assessmentPayloads[i].isEmpty()) return false;
        }
    } else if (rules==PreviousScoringRules || rules==CurrentScoringRules) {
        if(rules==CurrentScoringRules && answer.contentVersion.size()!=64)return false;
        QSet<QString> choices;
        for (const auto &option : answer.options) choices.insert(option.toCaseFolded());
        if (answer.options.size()!=4 || choices.size()!=4 || answer.options.count(answer.correctAnswer)!=1
            || !answer.options.contains(answer.selectedAnswer)) return false;
        maximum=maximumScoreUnits(answer.revealedHints,answer.ceilings);
        for (int i=0;i<3;++i) {
            if (answer.revealedHints & (1<<i)) {
                const QString id=answer.assessmentIds[i];
                if (id.size()!=64 || std::any_of(id.begin(),id.end(),[](QChar c){return !QStringLiteral("0123456789abcdef").contains(c);})) return false;
                const auto proof=answer.assessmentPayloads[i];
                const QString hash=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(proof).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
                const auto assessment=proof.value("assessment").toObject();
                if (rules==CurrentScoringRules) {
                    const auto evaluation=proof.value("evaluation").toObject();
                    const auto reviewed=QDateTime::fromString(evaluation.value("reviewed_at").toString(),Qt::ISODateWithMs);
                    const auto due=QDateTime::fromString(evaluation.value("due_at").toString(),Qt::ISODateWithMs);
                    if (hash!=id || proof.value("schema").toInt()!=2 || proof.value("slot").toInt(-1)!=i
                        || answer.contentVersion.isEmpty() || proof.value("content_version").toString()!=answer.contentVersion
                        || proof.value("scientific_name").toString()!=answer.scientificName
                        || proof.value("species_id").toString()!=QString::number(answer.speciesId)
                        || proof.value("rank").toString()!=answer.rank || proof.value("photo").toObject().value("id").toString()!=answer.photoId
                        || proof.value("options").toArray()!=QJsonArray::fromStringList(answer.options)
                        || proof.value("hint_id").toString().isEmpty() || hintType(proof.value("category").toString())==HintType::None
                        || proof.value("text").toString().isEmpty() || !proof.value("source_url").toString().startsWith("https://")
                        || proof.value("locator").toString().isEmpty() || proof.value("rationale").toString().isEmpty()
                        || evaluation.value("state").toString()!="approved_automated" || evaluation.value("actor_type").toString()!="automated"
                        || evaluation.value("policy_version").toString()!="progressive-pedagogy-v1"
                        || evaluation.value("content_sha256").toString().size()!=64 || !reviewed.isValid() || !due.isValid() || due<=reviewed
                        || answer.ceilings[i]!=(i==0 ? 6 : i==1 ? 3 : 2)) return false;
                    const QRegularExpression literal("(?<![\\p{L}])"+QRegularExpression::escape(answer.correctAnswer)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
                    if(literal.match(proof.value("text").toString()).hasMatch())return false;
                    const auto recipe=proof.value("source_recipe").toObject();
                    if(recipe.value("category")!=proof.value("category") || recipe.value("source_url")!=proof.value("source_url")
                        || recipe.value("locator")!=proof.value("locator") || recipe.value("rationale")!=proof.value("rationale"))return false;
                    if(proof.value("kind").toString()=="ancestor") {
                        const QStringList ranks{"kingdom","phylum","class","order","family","genus","species"};
                        const int asked=ranks.indexOf(answer.rank),ancestorLevel=ranks.indexOf(recipe.value("node").toObject().value("rank").toString());
                        if(asked<0 || ancestorLevel<0 || ancestorLevel>=asked || (i==0 && ancestorLevel>asked-2)
                            || (i==1 && ancestorLevel<asked-3) || (i==2 && ancestorLevel<asked-2))return false;
                        const auto evidence=proof.value("option_evidence").toObject();int compatible=0;
                        for(const auto &option:answer.options) {
                            const auto e=evidence.value(option).toObject();
                            if(!e.value("known").toBool() || !e.value("compatible").isBool() || e.value("records").toArray().isEmpty())return false;
                            const auto ancestor=recipe.value("node").toObject().value("id").toString();
                            if(ancestor.isEmpty())return false;
                            for(const auto &v:e.value("records").toArray()) {
                                const auto record=v.toObject();
                                if(record.value("species_id").toString().isEmpty() || record.value("ancestor_id").toString().isEmpty()
                                    || !record.value("source_url").toString().startsWith("https://www.inaturalist.org/taxa/")
                                    || record.value("locator").toString().isEmpty()
                                    || (record.value("ancestor_id").toString()==ancestor)!=e.value("compatible").toBool())return false;
                            }
                            compatible+=e.value("compatible").toBool();
                            if(option==answer.correctAnswer && !e.value("compatible").toBool())return false;
                        }
                        if((i==0 && compatible<3) || (i==1 && (compatible<2 || compatible>3)) || (i==2 && compatible>2))return false;
                    } else if(proof.value("kind").toString()!="note" || (recipe.value("organism_scope").toBool() && !recipe.value("original_ranks").toArray().contains(answer.rank)
                            ? QStringLiteral("Sobre este organismo, sem generalizar para todo o grupo: ")+recipe.value("text").toString() : recipe.value("text").toString())!=proof.value("text").toString()
                        || !recipe.value("ranks").toArray().contains(answer.rank)
                        || assistanceCeiling(assistanceLevel(recipe.value("grade").toString()))!=answer.ceilings[i])return false;
                    const auto token=recipe.value("association_token").toString();
                    if(!token.isEmpty()) {
                        const QRegularExpression pattern("(?<![\\p{L}])"+QRegularExpression::escape(token)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
                        int matches=0;for(const auto &option:answer.options)matches+=pattern.match(option).hasMatch();
                        if(matches!=1 || !pattern.match(answer.correctAnswer).hasMatch())return false;
                    }
                    continue;
                }
                if (hash!=id || proof.value("scientific_name").toString()!=answer.scientificName
                    || proof.value("species_id").toString()!=QString::number(answer.speciesId)
                    || proof.value("category").toString()!=hintKey(static_cast<HintType>(i+1))
                    || proof.value("photo").toObject().value("id").toString()!=answer.photoId
                    || !assessment.value("ranks").toArray().contains(answer.rank)
                    || assistanceCeiling(assistanceLevel(assessment.value("grade").toString()))!=answer.ceilings[i]) return false;
                const QString context=assessment.value("context").toString();
                if (context!="safe_broad_ancestor" && context!="any_safe_options" && context!="diagnostic_contrast") return false;
                if (i==2 && answer.ceilings[i]==2 && context!="diagnostic_contrast") return false;
                const QString association=assessment.value("association_token").toString();
                if (!association.isEmpty()) {
                    const QRegularExpression token("(?<![\\p{L}])"+QRegularExpression::escape(association)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
                    int matching=0;for (const auto &option : answer.options) matching+=token.match(option).hasMatch();
                    if (matching!=1 || !token.match(answer.correctAnswer).hasMatch()) return false;
                }
                if (context=="diagnostic_contrast") for (const auto &option : answer.options) {
                    const auto evidence=assessment.value("option_evidence").toObject().value(option).toObject();
                    if (evidence.value("rank").toString()!=answer.rank || !evidence.value("compatible").isBool()
                        || evidence.value("compatible").toBool()!=(option==answer.correctAnswer)
                        || !evidence.value("source_url").toString().startsWith("https://") || evidence.value("locator").toString().isEmpty()) return false;
                }
            } else if (answer.ceilings[i]!=0 || !answer.assessmentIds[i].isEmpty() || !answer.assessmentPayloads[i].isEmpty()) return false;
        }
    } else return false;
    return maximum>0 && answer.maximumUnits==maximum
        && answer.awardedUnits==(answer.correct ? maximum : 0)
        && answer.correct==(answer.selectedAnswer==answer.correctAnswer);
}
