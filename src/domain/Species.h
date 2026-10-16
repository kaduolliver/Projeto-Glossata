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
    QString educationalKingdom; // Historical compatibility only; never a scientific rank.
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
    bool phylogenyApproved = false;
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
    QString domain() const { return answerAtRank("domain"); }
    QString displayGroup() const {
        for (const QString &rank : {QString("kingdom"),QString("supergroup"),QString("domain")}) {
            const auto name=answerAtRank(rank); if (!name.isEmpty()) return name;
        }
        return {};
    }
    QString displayGroupId() const {
        const auto name=displayGroup();
        for (const auto &node : taxonomy) if (node.name==name) return node.id;
        return "unclassified";
    }
};
