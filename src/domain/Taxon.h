#pragma once
#include <QString>

struct Taxon {
    QString id;
    QString name;
    QString rank;
    QString parentId;
    QString source;
    QString sourceTaxonId;
    QString sourceUrl;
    QString rankNote;
    QString description;
};

inline QString taxonomicRankLabel(const QString &rank)
{
    if (rank == "species") return QStringLiteral("Espécie");
    if (rank == "genus") return QStringLiteral("Gênero");
    if (rank == "family") return QStringLiteral("Família");
    if (rank == "order") return QStringLiteral("Ordem");
    if (rank == "class") return QStringLiteral("Classe");
    if (rank == "phylum") return QStringLiteral("Filo / Divisão");
    if (rank == "kingdom") return QStringLiteral("Reino científico");
    if (rank == "domain") return QStringLiteral("Domínio");
    if (rank == "supergroup") return QStringLiteral("Supergrupo · clado sem rank formal");
    if (rank == "clade") return QStringLiteral("Clado sem rank formal");
    if (rank == "educational_kingdom") return QStringLiteral("Agrupamento histórico");
    if (rank == "subphylum") return QStringLiteral("Subfilo");
    if (rank == "subclass") return QStringLiteral("Subclasse");
    if (rank == "suborder") return QStringLiteral("Subordem");
    if (rank == "superfamily") return QStringLiteral("Superfamília");
    if (rank == "subfamily") return QStringLiteral("Subfamília");
    if (rank == "tribe") return QStringLiteral("Tribo");
    if (rank == "subtribe") return QStringLiteral("Subtribo");
    if (rank == "subgenus") return QStringLiteral("Subgênero");
    if (rank == "infraclass") return QStringLiteral("Infraclasse");
    if (rank == "superorder") return QStringLiteral("Superordem");
    if (rank == "infraorder") return QStringLiteral("Infraordem");
    if (rank == "subterclass") return QStringLiteral("Subterclasse");
    if (rank == "section") return QStringLiteral("Seção");
    if (rank == "complex") return QStringLiteral("Complexo de espécies");
    return rank;
}
