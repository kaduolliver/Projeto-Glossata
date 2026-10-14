#pragma once

#include <QString>
#include <QVector>
#include "domain/Taxon.h"
#include "domain/Photo.h"

struct Species {
    int id = 0;
    QString name;
    QString fact;
    QString hint;
    QString family;
    QString region;
    QString image;
    QString commonName;
    QString educationalKingdom;
    QVector<Taxon> taxonomy;
    QString source;
    QString sourceTaxonId;
    QString sourceUrl;
    QString imageAuthor;
    QString imageLicense;
    QString imageLicenseUrl;
    QString imageSource;
    bool visualQuiz = false;
    Photo photo;
    bool taxonomyApproved = true;
    bool photoApproved = true;
    QStringList morphologyTraits;

    QString answerAtRank(const QString &rank) const {
        if (rank == "species") return name;
        for (const Taxon &taxon : taxonomy) if (taxon.rank == rank) return taxon.name;
        return {};
    }
    bool belongsTo(const QString &taxonId) const {
        if (taxonId.isEmpty()) return true;
        if (taxonId == "edu:" + educationalKingdom) return true;
        for (const Taxon &taxon : taxonomy) if (taxon.id == taxonId) return true;
        return false;
    }
};
