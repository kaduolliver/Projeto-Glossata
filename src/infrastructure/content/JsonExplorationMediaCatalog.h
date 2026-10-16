#pragma once
#include "application/ports/ExplorationMediaCatalog.h"
#include <QHash>

class JsonExplorationMediaCatalog final : public ExplorationMediaCatalog {
public:
    explicit JsonExplorationMediaCatalog(const QString &root = QStringLiteral(":"));
    QVariantMap photoFor(const QString &taxonId) const override;
private:
    QHash<QString, QVariantMap> photos_;
};
