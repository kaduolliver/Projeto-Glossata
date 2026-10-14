#include "application/EducationalContent.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
namespace {
QVariantMap note(const QJsonObject &item)
{
    if (!item.value("verified").toBool() || item.value("text").toString().isEmpty()
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
    const auto refs=speciesContent(s).value("name_origin").toObject().value("parts").toArray();
    QStringList texts,urls;
    QString title;
    for(const auto &ref : refs) {
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
                                               const QVector<Species> &pool) const
{
    QuestionHints result;
    for(int i=0;i<3;++i) {
        result[i].type=static_cast<HintType>(i+1);
    }
    result[0].unavailableReason=QStringLiteral("Sem contexto seguro para estas alternativas.");
    result[1].unavailableReason=QStringLiteral("Origem do nome não verificada para este nível.");
    result[2].unavailableReason=QStringLiteral("Sem característica física adequada a esta questão.");
    auto mentionsOption=[&options](const QString &text) {
        for(const QString &option : options) {
            const QRegularExpression pattern("(?<![\\p{L}])"+QRegularExpression::escape(option)+"(?![\\p{L}])",QRegularExpression::CaseInsensitiveOption);
            if(pattern.match(text).hasMatch())return true;
        }
        return false;
    };
    auto assign=[&](int index,const QVariantMap &item) {
        if(item.isEmpty() || mentionsOption(item.value("text").toString()))return;
        result[index].text=item.value("text").toString();
        result[index].sourceUrl=item.value("source_url").toString();
        result[index].sourceTitle=item.value("source_title").toString();
        result[index].unavailableReason.clear();
    };
    // Only a broader, verified ancestor is permitted. At least two distinct
    // choices must remain compatible; an option with missing ancestry is not
    // counted as compatible. Never use a more specific taxon to encode a rank.
    const QStringList ranks={"kingdom","phylum","class","order","family","genus","species"};
    const int asked=ranks.indexOf(rank);
    for(auto it=s.taxonomy.crbegin();it!=s.taxonomy.crend();++it) {
        const int level=ranks.indexOf(it->rank);
        if(level<0 || asked<0 || level>=asked || it->source!="inaturalist" || it->sourceTaxonId.isEmpty())continue;
        int compatible=0;
        for(const QString &option : options) {
            bool known=false,all=true;
            for(const Species &candidate : pool) if(candidate.answerAtRank(rank)==option) {
                known=true; all=all && candidate.belongsTo(it->id);
            }
            compatible+=known && all;
        }
        if(compatible<2 || (it->rank=="kingdom" && compatible==options.size()))continue;
        assign(0,{{"text",QStringLiteral("Na classificação científica, pertence ao grupo %1: %2. Compare os integrantes pela fotografia e pelas outras informações.").arg(taxonomicRankLabel(it->rank),it->name)},
                  {"source_url","https://www.inaturalist.org/taxa/"+it->sourceTaxonId},
                  {"source_title","iNaturalist · classificação do grupo"}});
        if(result[0].available())break;
    }
    const auto item=speciesContent(s);
    assign(1,note(resolve(item.value("etymology_hints").toObject().value(rank))));
    // Existing reviewed appearance notes are reused, but habitat/diet clues and
    // vague group fallbacks must not silently become high-assistance morphology.
    const auto morphology=note(resolve(item.value("morphological_hint")));
    if(morphology.value("ranks").toList().contains(rank)
       && morphology.value("photo_id").toString()==s.photo.id
       && !leaks(morphology.value("text").toString(),s,options))assign(2,morphology);
    return result;
}
QVariantMap EducationalContent::profile(const Species &s) const
{
    const auto item=speciesContent(s);
    if (item.isEmpty()) return {};
    QVariantMap result{{"commonName",item.value("common_name").toString()},
                       {"displayName",item.value("display_name").toString()},
                       {"reviewedOn",item.value("reviewed_on").toString()}};
    for (const QString &field : {QString("summary"),QString("distribution"),QString("nutrition"),
                                QString("morphology"),QString("conservation")})
        result.insert(field,note(resolve(item.value(field))));
    return result;
}
QVariantMap EducationalContent::introduction(const QString &id) const { return note(resolve(data_.value("taxons").toObject().value(id))); }
QVariantMap EducationalContent::hint(const Species &s, const QString &rank, const QStringList &options) const
{
    QJsonArray candidates=speciesContent(s).value("hints").toArray();
    const QJsonObject scopes=data_.value("group_hints").toObject();
    for (auto it=s.taxonomy.crbegin(); it!=s.taxonomy.crend(); ++it)
        for (const auto &candidate : scopes.value(it->id).toArray()) candidates.append(candidate);
    for (const auto &candidate : scopes.value("edu:"+s.educationalKingdom).toArray()) candidates.append(candidate);
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
    return note(resolve(scopes.value("edu:"+s.educationalKingdom)));
}
