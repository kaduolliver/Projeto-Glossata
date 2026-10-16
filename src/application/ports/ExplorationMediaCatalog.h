#pragma once
#include <QString>
#include <QVariantMap>

// Presentation photographs never confer quiz eligibility or scientific approval.
class ExplorationMediaCatalog {
public:
    virtual ~ExplorationMediaCatalog() = default;
    virtual QVariantMap photoFor(const QString &taxonId) const = 0;
};
