#pragma once
#include "domain/Species.h"
#include "domain/Hint.h"
#include <QJsonObject>
#include <QVariantMap>
class EducationalContent {
public:
    EducationalContent();
    explicit EducationalContent(const QJsonObject &data) : data_(data) {}
    QVariantMap introduction(const QString &taxonId) const;
    QVariantMap hint(const Species &species, const QString &rank, const QStringList &options) const;
    QVariantMap curiosity(const Species &species) const;
    QVariantMap profile(const Species &species) const;
    QuestionHints questionHints(const Species &species,const QString &rank,const QStringList &options,
                                const QVector<Species> &pool,bool competitive=false) const;
    QString pedagogyVersion() const { return data_.value("automated_pedagogy").toObject().value("evaluation").toObject().value("content_sha256").toString(); }
    QVariantMap nameOrigin(const Species &species) const;
private:
    QuestionHints progressiveHints(const Species &,const QString &,const QStringList &,const QVector<Species> &) const;
    QJsonObject data_;
    mutable QString verifiedAutomaticHash_;
    QJsonObject resolve(const QJsonValue &reference) const;
    QJsonObject speciesContent(const Species &species) const;
};
