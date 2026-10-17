#include "SessionCheckpoint.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <cmath>

namespace {
bool integer(const QJsonValue &v, qint64 low=0, qint64 high=1000000) {
    return v.isDouble() && std::isfinite(v.toDouble()) && v.toDouble()>=low
        && v.toDouble()<=high && std::floor(v.toDouble())==v.toDouble();
}
QStringList strings(const QJsonArray &a) {
    QStringList result; for (const auto &v:a) { if (!v.isString()) return {}; result.append(v.toString()); } return result;
}
QJsonObject hint(const Hint &h) {
    return {{"category",hintKey(h.type)},{"id",h.id},{"text",h.text},{"source",h.sourceUrl},{"title",h.sourceTitle},
        {"unavailable_reason",h.unavailableReason},{"assistance",int(h.assistance)},{"assessment_id",h.assessmentId},
        {"assessment_state",h.assessmentState},{"rationale",h.rationale},{"proof",h.assessmentPayload},{"provisional",h.provisional}};
}
bool readHint(const QJsonObject &o, Hint &h) {
    if (!integer(o["assistance"],0,6) || !o["proof"].isObject() || !o["provisional"].isBool()) return false;
    h.type=hintType(o["category"].toString()); h.id=o["id"].toString(); h.text=o["text"].toString();
    h.sourceUrl=o["source"].toString(); h.sourceTitle=o["title"].toString(); h.unavailableReason=o["unavailable_reason"].toString();
    h.assistance=static_cast<AssistanceLevel>(o["assistance"].toInt()); h.assessmentId=o["assessment_id"].toString();
    h.assessmentState=o["assessment_state"].toString(); h.rationale=o["rationale"].toString();
    h.assessmentPayload=o["proof"].toObject(); h.provisional=o["provisional"].toBool();
    return (int(h.assistance)==0 || h.ceiling()>0) && hint(h)==o;
}
bool questionShape(const QJsonObject &o) {
    const auto options=strings(o["options"].toArray()); const auto hints=o["hints"].toArray();
    if (!integer(o["species_id"],1) || o["name"].toString().isEmpty() || o["photo_id"].toString().isEmpty()
        || o["rank"].toString().isEmpty() || options.size()!=4 || QSet<QString>(options.begin(),options.end()).size()!=4
        || options.contains("") || !options.contains(o["correct_answer"].toString()) || hints.size()!=3 || !o["visual"].isBool()) return false;
    for (const auto &v:hints) { Hint h; if (!v.isObject() || !readHint(v.toObject(),h)) return false; }
    return true;
}
}

QJsonObject SessionSnapshot::configuration(const GameConfiguration &c) {
    return {{"mode",c.modeKey()},{"limit",c.questionLimit},{"group",c.group},{"rank",c.rank},
        {"difficulty",c.difficultyKey()},{"rules",c.rulesVersion},{"content",c.contentVersion}};
}
bool SessionSnapshot::readConfiguration(const QJsonObject &o, GameConfiguration &c) {
    if ((o["mode"]!="free" && o["mode"]!="competitive") || !integer(o["limit"],0,10000)
        || (o["difficulty"]!="broad" && o["difficulty"]!="balanced" && o["difficulty"]!="close")) return false;
    c.mode=o["mode"]=="free" ? GameMode::Free : GameMode::Competitive; c.questionLimit=o["limit"].toInt();
    c.group=o["group"].toString(); c.rank=o["rank"].toString(); c.rulesVersion=o["rules"].toString(); c.contentVersion=o["content"].toString();
    c.difficulty=o["difficulty"]=="broad" ? QuestionDifficulty::Broad : o["difficulty"]=="close" ? QuestionDifficulty::Close : QuestionDifficulty::Balanced;
    return configuration(c)==o && !c.rank.isEmpty() && (c.mode==GameMode::Free || (c.questionLimit>0 && !c.contentVersion.isEmpty()));
}
QJsonObject SessionSnapshot::answer(const AnswerRecord &a) {
    QJsonArray ceilings,ids,proofs;
    for (int i=0;i<3;++i) { ceilings.append(a.ceilings[i]); ids.append(a.assessmentIds[i]); proofs.append(a.assessmentPayloads[i]); }
    return {{"number",a.questionNumber},{"species_id",a.speciesId},{"name",a.scientificName},{"photo_id",a.photoId},
        {"rank",a.rank},{"selected",a.selectedAnswer},{"correct_answer",a.correctAnswer},{"correct",a.correct},
        {"revealed",a.revealedHints},{"maximum",a.maximumUnits},{"awarded",a.awardedUnits},{"rules",a.rulesVersion},
        {"ceilings",ceilings},{"assessment_ids",ids},{"proofs",proofs},{"options",QJsonArray::fromStringList(a.options)},{"content",a.contentVersion}};
}
bool SessionSnapshot::readAnswer(const QJsonObject &o, AnswerRecord &a) {
    for (const auto &key: {"number","species_id","revealed","maximum","awarded"}) if (!integer(o[key])) return false;
    const auto ceilings=o["ceilings"].toArray(),ids=o["assessment_ids"].toArray(),proofs=o["proofs"].toArray();
    if (ceilings.size()!=3 || ids.size()!=3 || proofs.size()!=3 || !o["correct"].isBool()) return false;
    a.questionNumber=o["number"].toInt(); a.speciesId=o["species_id"].toInt(); a.scientificName=o["name"].toString();
    a.photoId=o["photo_id"].toString(); a.rank=o["rank"].toString(); a.selectedAnswer=o["selected"].toString();
    a.correctAnswer=o["correct_answer"].toString(); a.correct=o["correct"].toBool(); a.revealedHints=o["revealed"].toInt();
    a.maximumUnits=o["maximum"].toInt(); a.awardedUnits=o["awarded"].toInt(); a.rulesVersion=o["rules"].toString();
    a.options=strings(o["options"].toArray()); a.contentVersion=o["content"].toString();
    for (int i=0;i<3;++i) {
        if (!integer(ceilings[i],0,6) || !ids[i].isString() || !proofs[i].isObject()) return false;
        a.ceilings[i]=ceilings[i].toInt(); a.assessmentIds[i]=ids[i].toString(); a.assessmentPayloads[i]=proofs[i].toObject();
    }
    return answer(a)==o && validAnswerScoring(a,a.rulesVersion);
}
QJsonObject SessionSnapshot::question(const Question &q) {
    QJsonArray hints; for (const auto &h:q.hints) hints.append(hint(h));
    return {{"species_id",q.species.id},{"name",q.species.name},{"photo_id",q.photo.id},{"photo_sha256",q.photo.sha256},
        {"prompt",q.prompt},{"fact",q.fact},{"extra_hint",q.extraHint},{"correct_answer",q.correctAnswer},
        {"options",QJsonArray::fromStringList(q.options)},{"rank",q.identificationRank},{"visual",q.visual},
        {"hint_source",q.hintSourceUrl},{"hint_title",q.hintSourceTitle},{"hints",hints}};
}
bool SessionSnapshot::readQuestion(const QJsonObject &o, const QVector<Species> &pool, Question &q) {
    if (!questionShape(o)) return false;
    auto found=std::find_if(pool.begin(),pool.end(),[&](const Species &s){return s.id==o["species_id"].toInt();});
    if (found==pool.end() || found->name!=o["name"].toString() || found->photo.id!=o["photo_id"].toString()
        || found->photo.sha256!=o["photo_sha256"].toString() || found->answerAtRank(o["rank"].toString())!=o["correct_answer"].toString()) return false;
    q.species=*found; q.photo=found->photo; q.prompt=o["prompt"].toString(); q.fact=o["fact"].toString(); q.extraHint=o["extra_hint"].toString();
    q.correctAnswer=o["correct_answer"].toString(); q.options=strings(o["options"].toArray()); q.identificationRank=o["rank"].toString();
    q.visual=o["visual"].toBool(); q.hintSourceUrl=o["hint_source"].toString(); q.hintSourceTitle=o["hint_title"].toString();
    for (int i=0;i<3;++i) if (!readHint(o["hints"].toArray()[i].toObject(),q.hints[i])) return false;
    return question(q)==o;
}
bool SessionSnapshot::valid(const QJsonObject &p) {
    GameConfiguration c;
    if (!integer(p["schema"],Schema,Schema) || p["id"].toString().isEmpty() || p["player_id"].toString().isEmpty()
        || p["player_name"].toString().trimmed().isEmpty() || p["content_identity"].toString().size()!=64
        || !readConfiguration(p["configuration"].toObject(),c) || c.rulesVersion!=CurrentScoringRules
        || !integer(p["units"],0,60000) || !integer(p["correct"],0,10000) || !integer(p["round"],1,10001)
        || !integer(p["revealed"],0,7) || !integer(p["species_index"],0,10000)
        || !p["finished"].isBool() || !p["answers"].isArray() || !p["questions"].isArray()
        || p["generator"].toString().isEmpty() || p["generator"].toString().size()>20000) return false;
    const auto answers=p["answers"].toArray(),questions=p["questions"].toArray(),order=p["species_order"].toArray(),pool=p["option_pool"].toArray();
    const bool finished=p["finished"].toBool(); const int n=answers.size();
    if (n>10000 || questions.isEmpty() || questions.size()>10001 || order.isEmpty() || pool.isEmpty()
        || p["species_index"].toInt()>=order.size() || (questions.size()!=(n+1)
            && !(finished && c.mode==GameMode::Competitive && n==c.questionLimit && questions.size()==n))
        || p["round"].toInt()!=(finished && c.mode==GameMode::Competitive && n==c.questionLimit ? n : n+1)) return false;
    QSet<int> poolIds,orderIds,asked;
    for (const auto &v:pool) { if (!integer(v,1) || poolIds.contains(v.toInt())) return false; poolIds.insert(v.toInt()); }
    for (const auto &v:order) { if (!integer(v,1) || orderIds.contains(v.toInt()) || !poolIds.contains(v.toInt())) return false; orderIds.insert(v.toInt()); }
    qint64 total=0; int correct=0;
    for (int i=0;i<questions.size();++i) {
        const auto q=questions[i].toObject();
        if (!questionShape(q) || q["rank"]!=c.rank || !orderIds.contains(q["species_id"].toInt())) return false;
        if (c.mode==GameMode::Competitive && asked.contains(q["species_id"].toInt())) return false;
        asked.insert(q["species_id"].toInt());
        if (i<n) {
            AnswerRecord a;
            if (!readAnswer(answers[i].toObject(),a) || a.questionNumber!=i+1 || a.rulesVersion!=c.rulesVersion
                || a.speciesId!=q["species_id"].toInt() || a.scientificName!=q["name"].toString() || a.photoId!=q["photo_id"].toString()
                || a.rank!=c.rank || a.correctAnswer!=q["correct_answer"].toString() || QJsonArray::fromStringList(a.options)!=q["options"]) return false;
            for (int slot=0;slot<3;++slot) if (a.revealedHints & (1<<slot)) {
                Hint h; if (!readHint(q["hints"].toArray()[slot].toObject(),h) || !h.available()
                    || a.ceilings[slot]!=h.ceiling() || a.assessmentIds[slot]!=h.assessmentId || a.assessmentPayloads[slot]!=h.assessmentPayload) return false;
            }
            total+=a.awardedUnits; correct+=a.correct;
        }
    }
    const auto current=questions.last().toObject();
    if (current["species_id"]!=order[p["species_index"].toInt()]) return false;
    if (questions.size()==n && p["revealed"].toInt()!=0) return false;
    if (questions.size()==n+1) for (int i=0;i<3;++i) if (p["revealed"].toInt() & (1<<i)) {
        Hint h; if (!readHint(current["hints"].toArray()[i].toObject(),h) || !h.available()) return false;
        AnswerRecord a; a.questionNumber=n+1; a.speciesId=current["species_id"].toInt(); a.scientificName=current["name"].toString();
        a.photoId=current["photo_id"].toString(); a.rank=c.rank; a.correctAnswer=current["correct_answer"].toString(); a.selectedAnswer=a.correctAnswer;
        a.correct=true; a.rulesVersion=c.rulesVersion; a.options=strings(current["options"].toArray()); a.revealedHints=1<<i;
        a.contentVersion=p["pedagogy_version"].toString(); a.ceilings[i]=h.ceiling(); a.maximumUnits=h.ceiling(); a.awardedUnits=h.ceiling();
        a.assessmentIds[i]=h.assessmentId; a.assessmentPayloads[i]=h.assessmentPayload;
        if (!validAnswerScoring(a,c.rulesVersion)) return false;
    }
    return total==p["units"].toDouble() && correct==p["correct"].toInt()
        && (c.mode==GameMode::Free || (n<=c.questionLimit && questions.size()<=c.questionLimit && (finished || n<c.questionLimit)));
}
bool SessionSnapshot::transition(const QJsonObject &a, const QJsonObject &b) {
    if (!valid(a) || !valid(b) || a["finished"].toBool()) return false;
    for (const auto &key:{"id","player_id","player_name","configuration","content_identity","pedagogy_version","option_pool"}) if (a[key]!=b[key]) return false;
    const auto aa=a["answers"].toArray(),ba=b["answers"].toArray(),aq=a["questions"].toArray(),bq=b["questions"].toArray();
    if (ba.size()<aa.size() || ba.size()>aa.size()+1 || bq.size()<aq.size() || bq.size()>aq.size()+1) return false;
    for (int i=0;i<aa.size();++i) if (aa[i]!=ba[i]) return false;
    for (int i=0;i<aq.size();++i) if (aq[i]!=bq[i]) return false;
    if (aa.size()==ba.size()) return (a["revealed"].toInt() & b["revealed"].toInt())==a["revealed"].toInt()
        && a["species_order"]==b["species_order"] && a["species_index"]==b["species_index"];
    return ba.last().toObject()["revealed"]==a["revealed"];
}
bool SessionSnapshot::matchesResult(const QJsonObject &p, const PlayerIdentity &player, qint64 units, int correct,
    const QVector<AnswerRecord> &answers, const GameConfiguration &c) {
    if (!valid(p) || !p["finished"].toBool() || p["player_id"]!=player.id || p["player_name"]!=player.displayName
        || p["units"].toDouble()!=units || p["correct"].toInt()!=correct || p["configuration"]!=configuration(c)) return false;
    QJsonArray rows; for (const auto &a:answers) rows.append(answer(a)); return rows==p["answers"].toArray();
}
