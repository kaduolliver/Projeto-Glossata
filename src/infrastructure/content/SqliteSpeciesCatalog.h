#pragma once
#include "application/ports/SpeciesCatalog.h"
#include <QSqlDatabase>

// A separate catalog database keeps content migrations independent of player data.
class SqliteSpeciesCatalog final : public SpeciesCatalog {
public:
    explicit SqliteSpeciesCatalog(const QString &databasePath);
    ~SqliteSpeciesCatalog() override;
    QVector<Species> loadSpecies() const override { return species_; }
    QVector<Taxon> loadTaxons() const override { return taxons_; }
    QVector<Species> loadCurrentSpecies() const override { return currentSpecies_; }
    QVector<Taxon> loadCurrentTaxons() const override { return currentTaxons_; }
    int descendantCount(const QString &taxonId) const;
    bool isReady() const { return ready_; }

private:
    bool readCache();
    QSqlDatabase database_;
    QVector<Species> species_;
    QVector<Taxon> taxons_;
    QVector<Species> currentSpecies_;
    QVector<Taxon> currentTaxons_;
    bool ready_ = false;
};
