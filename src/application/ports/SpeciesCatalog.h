#pragma once

#include "domain/Species.h"

#include <QVector>

class SpeciesCatalog {
public:
    virtual ~SpeciesCatalog() = default;
    virtual QVector<Species> loadSpecies() const = 0;
};
