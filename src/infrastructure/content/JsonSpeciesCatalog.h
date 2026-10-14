#pragma once

#include "application/ports/SpeciesCatalog.h"

#include <QString>

class JsonSpeciesCatalog final : public SpeciesCatalog {
public:
    explicit JsonSpeciesCatalog(const QString &sourcePath = QStringLiteral(":/resources/data/species.json"));
    QVector<Species> loadSpecies() const override;
    QVector<Taxon> loadTaxons() const override;

private:
    QString sourcePath_;
};
