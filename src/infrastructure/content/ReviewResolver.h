#pragma once

#include "domain/Species.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
        const bool matches = entry.value("scientific_name").toString() == species.name;
        species.taxonomyApproved = matches && review.value("taxonomy").toString() == "approved";
        species.photoApproved = matches && review.value("photo").toString() == "approved"
            && review.value("photo_id").toString() == species.photo.id;
        species.morphologyTraits.clear();
        if (matches && review.value("morphology").toString() == "approved")
            for (const auto &trait : entry.value("morphology_traits").toArray()) species.morphologyTraits.append(trait.toString());
    }
};
