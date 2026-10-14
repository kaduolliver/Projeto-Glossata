#include "infrastructure/content/JsonSpeciesCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QHash>

QVector<Taxon> JsonSpeciesCatalog::loadTaxons() const
{
    QFile file(QStringLiteral(":/resources/data/taxonomy.json"));
    if (!file.open(QIODevice::ReadOnly)) return {};
    QVector<Taxon> taxons;
    for (const auto &value : QJsonDocument::fromJson(file.readAll()).array()) {
        const QJsonObject object = value.toObject();
        Taxon taxon{object.value("id").toString(), object.value("name").toString(), object.value("rank").toString(),
            object.value("parent_id").toString(), object.value("source").toString(), object.value("source_taxon_id").toString()};
        if (!taxon.id.isEmpty() && !taxon.name.isEmpty() && !taxon.rank.isEmpty()) taxons.append(taxon);
    }
    return taxons;
}

JsonSpeciesCatalog::JsonSpeciesCatalog(const QString &sourcePath)
    : sourcePath_(sourcePath)
{
}

QVector<Species> JsonSpeciesCatalog::loadSpecies() const
{
    QFile file(sourcePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível abrir o catálogo:" << sourcePath_ << file.errorString();
        return {};
    }

    const QByteArray data = file.readAll();
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Catálogo inválido:" << sourcePath_ << error.errorString();
        return {};
    }
    if (!document.isArray()) {
        qWarning() << "O catálogo deve conter um array JSON:" << sourcePath_;
        return {};
    }

    QVector<Species> species;
    QHash<QString, Taxon> taxons;
    for (const Taxon &taxon : loadTaxons()) taxons.insert(taxon.id, taxon);
    const QJsonArray array = document.array();
    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        Species item;
        item.id = object.value(QStringLiteral("id")).toInt();
        item.name = object.value(QStringLiteral("name")).toString().trimmed();
        item.fact = object.value(QStringLiteral("fact")).toString().trimmed();
        item.hint = object.value(QStringLiteral("hint")).toString().trimmed();
        item.family = object.value(QStringLiteral("family")).toString();
        item.region = object.value(QStringLiteral("region")).toString();
        item.image = object.value(QStringLiteral("image")).toString();
        item.commonName = object.value("common_name").toString();
        item.educationalKingdom = object.value("educational_kingdom").toString();
        item.source = object.value("source").toString();
        item.sourceTaxonId = object.value("source_taxon_id").toString();
        item.sourceUrl = object.value("source_url").toString();
        for (const auto &id : object.value("taxonomy").toArray()) {
            if (taxons.contains(id.toString())) item.taxonomy.append(taxons.value(id.toString()));
        }
        const QJsonObject photo = object.value("photo").toObject();
        item.imageAuthor = photo.value("author").toString();
        item.imageLicense = photo.value("license").toString();
        item.imageLicenseUrl = photo.value("license_url").toString();
        item.imageSource = photo.value("source").toString();
        item.visualQuiz = photo.value("visual_quiz").toBool();
        const bool validImage = (item.imageLicense == "cc0" || item.imageLicense == "cc-by")
            && !item.imageAuthor.isEmpty() && !item.imageSource.isEmpty()
            && item.image.startsWith("qrc:/") && QFile::exists(":" + item.image.mid(4));
        if (!validImage) { item.image.clear(); item.visualQuiz = false; }
        if (!item.name.isEmpty() && !item.fact.isEmpty() && !item.hint.isEmpty()) {
            species.push_back(item);
        }
    }

    return species;
}
