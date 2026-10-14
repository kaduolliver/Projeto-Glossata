#pragma once
#include "domain/Species.h"
#include <QJsonObject>
#include <QVariantMap>
class EducationalContent {
public:
    EducationalContent();
    QVariantMap introduction(const QString &taxonId) const;
    QVariantMap hint(const Species &species, const QString &rank, const QStringList &options) const;
    QVariantMap curiosity(const Species &species) const;
    QVariantMap profile(const Species &species) const;
private:
    QJsonObject data_;
    QJsonObject resolve(const QJsonValue &reference) const;
    QJsonObject speciesContent(const Species &species) const;
};
