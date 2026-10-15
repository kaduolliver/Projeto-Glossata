#pragma once

#include "domain/Species.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "infrastructure/content/ReviewStatus.h"

// The bundle is immutable. Apply the same review gates to JSON and SQLite.
class ReviewResolver {
public:
    static void apply(Species &species) {
        static const QJsonObject content = [] {
            QFile file(":/resources/data/education.json");
            return file.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(file.readAll()).object() : QJsonObject{};
        }();
        const auto entry = content.value("species").toObject().value(QString::number(species.id)).toObject();
        const auto review = entry.value("review").toObject();
        const auto governance = entry.value("governance").toObject();
        const bool matches = entry.value("scientific_name").toString() == species.name;
        species.taxonomyApproved = matches && usableReview(governance.value("taxonomy").toObject());
        species.photoApproved = matches
            && (governance.value("photo").toObject().value("state").toString() != "legacy"
                || review.value("photo_id").toString() == species.photo.id)
            && usableReview(governance.value("photo").toObject());
        species.morphologyTraits.clear();
        if (matches && usableReview(governance.value("morphology").toObject()))
            for (const auto &trait : entry.value("morphology_traits").toArray()) species.morphologyTraits.append(trait.toString());
    }
};
