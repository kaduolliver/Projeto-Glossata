#include "application/TaxonomyIndex.h"
#include <QSet>
#include <algorithm>
#include "domain/QuizSelection.h"

TaxonomyIndex::TaxonomyIndex(const SpeciesCatalog &catalog)
    : species_(catalog.loadSpecies())
{
    for (const Taxon &taxon : catalog.loadTaxons()) {
        taxons_.insert(taxon.id, taxon);
        children_[taxon.parentId].append(taxon.id);
    }
    for (const Species &species : species_) {
        QSet<QString> counted;
        for (const Taxon &taxon : species.taxonomy) counted.insert(taxon.id);
        if (!species.educationalKingdom.isEmpty()) counted.insert("edu:" + species.educationalKingdom);
        for (const QString &id : counted) ++counts_[id];
    }
}

QVector<Species> TaxonomyIndex::speciesFor(const QString &id) const
{
    QVector<Species> selected;
    for (const Species &species : species_) if (species.belongsTo(id)) selected.append(species);
    return selected;
}

int TaxonomyIndex::count(const QString &id) const
{
    return id.isEmpty() ? static_cast<int>(species_.size()) : counts_.value(id);
}

QString TaxonomyIndex::kingdomFor(const QString &id) const
{
    QString current = id;
    QSet<QString> visited;
    while (!current.isEmpty() && !visited.contains(current)) {
        if (current.startsWith("edu:")) return current.mid(4);
        visited.insert(current);
        current = taxons_.value(current).parentId;
    }
    return {};
}

QVariantMap TaxonomyIndex::describe(const QString &id) const
{
    const Taxon taxon = taxons_.value(id);
    const QString kingdom = kingdomFor(id);
    int visualCount=0,supportingCount=0;
    for (const Species &s : speciesFor(id)) if (s.photo.valid && !s.photo.approvedRanks.isEmpty()) {
        if (s.photo.role=="supporting") ++supportingCount; else ++visualCount;
    }
    return {{"id", id}, {"title", taxon.name}, {"rank", taxon.rank},
        {"rankLabel", taxonomicRankLabel(taxon.rank)}, {"kingdom", kingdom},
        {"count", count(id)}, {"visualCount",visualCount},{"supportingCount",supportingCount},{"introduction",content_.introduction(id)},
        {"source", taxon.source}, {"sourceTaxonId", taxon.sourceTaxonId},
        {"description", taxon.rank == "educational_kingdom"
            ? QStringLiteral("Explore a diversidade de %1.").arg(taxon.name)
            : QStringLiteral("%1 · Conheça os grupos e suas espécies.").arg(taxonomicRankLabel(taxon.rank))}};
}

QVariantList TaxonomyIndex::kingdoms() const
{
    QVariantList list;
    list.append(QVariantMap{{"id", QString()}, {"title", QStringLiteral("Todas as espécies")},
        {"kingdom", QString()}, {"count", count()}, {"visualCount",describe(QString()).value("visualCount")},
        {"defaultRank",QStringLiteral("genus")},
        {"playableCount", eligibleQuizSpecies(species_,QStringLiteral("genus")).size()},
        {"description", QStringLiteral("Identifique gêneros com fotografias aprovadas de diferentes reinos.")}});
    for (const QString &name : {QString("Animalia"), QString("Plantae"), QString("Fungi"), QString("Protista"), QString("Monera")}) {
        list.append(describe("edu:" + name));
    }
    return list;
}

QVariantList TaxonomyIndex::children(const QString &id) const
{
    QVector<QString> ids = children_.value(id);
    // Hide only a redundant scientific kingdom with the same name as its educational root.
    if (id.startsWith("edu:") && ids.size() == 1 && taxons_.value(ids.first()).rank == "kingdom"
        && taxons_.value(ids.first()).name == id.mid(4)) ids = children_.value(ids.first());
    std::sort(ids.begin(), ids.end(), [this](const QString &a, const QString &b) {
        return taxons_.value(a).name < taxons_.value(b).name;
    });
    QVariantList list;
    for (const QString &child : ids) if (count(child) > 0) list.append(describe(child));
    return list;
}

QVariantList TaxonomyIndex::breadcrumbs(const QString &id) const
{
    QVariantList list;
    QSet<QString> visited;
    QString current = id;
    while (!current.isEmpty() && taxons_.contains(current) && !visited.contains(current)) {
        visited.insert(current);
        const Taxon node = taxons_.value(current);
        const bool redundant = node.rank == "kingdom" && node.parentId == "edu:" + node.name;
        if (!redundant) list.prepend(describe(current));
        current = node.parentId;
    }
    return list;
}

QVariantList TaxonomyIndex::identificationLevels(const QString &id) const
{
    const QVector<Species> selected = speciesFor(id);
    QVariantList levels;
    for (const QString &rank : {QString("species"), QString("genus"), QString("family"), QString("order"), QString("class"), QString("phylum"), QString("kingdom")}) {
        QSet<QString> answers;
        int eligible = 0;
        for (const Species &species : eligibleQuizSpecies(selected, rank)) {
            const QString answer = species.answerAtRank(rank).trimmed();
            if (!answer.isEmpty()) { answers.insert(answer.toCaseFolded()); ++eligible; }
        }
        if (answers.size() >= 4) levels.append(QVariantMap{{"id", rank}, {"title", taxonomicRankLabel(rank)},
            {"answers", answers.size()}, {"count", eligible}});
    }
    return levels;
}
