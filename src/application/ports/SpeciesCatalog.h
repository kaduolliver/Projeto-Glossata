#pragma once

#include "domain/Species.h"

#include <QVector>

class SpeciesCatalog {
public:
    virtual ~SpeciesCatalog() = default;
    virtual QVector<Species> loadSpecies() const = 0;
    virtual QVector<Taxon> loadTaxons() const { return {}; }
    // Scientific navigation uses the installed catalog, independently of rows
    // retained by persistence for historical games and migrations.
    virtual QVector<Species> loadCurrentSpecies() const { return loadSpecies(); }
    virtual QVector<Taxon> loadCurrentTaxons() const { return loadTaxons(); }
};
