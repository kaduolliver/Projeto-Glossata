#pragma once
#include "domain/Species.h"
#include <QMap>
#include <QSet>

struct PerformanceCount {
    QString id, name;
    int answered=0, correct=0;
};
struct KingdomPerformance : PerformanceCount {
    QMap<QString,PerformanceCount> phyla;
};
class TaxonomicPerformance {
public:
    QMap<QString,KingdomPerformance> groups;
    void record(const Species &s, bool correct) {
        const QString id=s.educationalKingdom.isEmpty() ? "edu:unknown" : "edu:"+s.educationalKingdom;
        auto &kingdom=groups[id];
        kingdom.id=id; kingdom.name=s.educationalKingdom.isEmpty() ? QStringLiteral("Classificação não disponível") : s.educationalKingdom;
        ++kingdom.answered; if (correct) ++kingdom.correct;
        QMap<QString,QString> known;
        for (const Taxon &t : s.taxonomy)
            if ((t.rank=="phylum" || t.rank=="division") && !t.id.isEmpty() && !t.name.isEmpty()) known.insert(t.id,t.name);
        const bool resolved=known.size()==1;
        const QString phylumId=resolved ? known.firstKey() : "unclassified:"+id;
        auto &phylum=kingdom.phyla[phylumId];
        phylum.id=phylumId; phylum.name=resolved ? known.first() : QStringLiteral("Classificação não disponível");
        ++phylum.answered; if (correct) ++phylum.correct;
    }
};
