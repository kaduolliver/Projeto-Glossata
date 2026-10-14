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
