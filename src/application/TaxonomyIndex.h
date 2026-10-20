#pragma once
#include "application/ports/SpeciesCatalog.h"
#include "application/ports/ExplorationMediaCatalog.h"
#include <QHash>
#include <QVariantList>
#include <QVariantMap>
#include "application/EducationalContent.h"

class TaxonomyIndex {
public:
    explicit TaxonomyIndex(const SpeciesCatalog &catalog, const ExplorationMediaCatalog *media = nullptr);
    QVector<Species> speciesFor(const QString &id) const;
    QVariantList domains() const;
    QVariantList quickAccess() const;
    QVariantList homeSlides() const;
    QVariantList kingdoms() const { return domains(); } // Compatibility for older adapters.
    QVariantList children(const QString &id) const;
    QVariantList breadcrumbs(const QString &id) const;
    QVariantList identificationLevels(const QString &id) const;
    QVariantMap describe(const QString &id) const;
    QString kingdomFor(const QString &id) const;
    int count(const QString &id = QString()) const;
private:
    const ExplorationMediaCatalog *media_;
    QVector<Species> species_;
    QHash<QString, Taxon> taxons_;
    QHash<QString, int> counts_;
    QHash<QString, QVector<QString>> children_;
    EducationalContent content_;
    mutable QHash<QString, QVariantList> levelCache_;
};
