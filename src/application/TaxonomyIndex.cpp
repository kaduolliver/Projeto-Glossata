#include "application/TaxonomyIndex.h"
#include <QSet>
#include <algorithm>
#include "domain/QuizSelection.h"

TaxonomyIndex::TaxonomyIndex(const SpeciesCatalog &catalog, const ExplorationMediaCatalog *media)
    : media_(media), species_(catalog.loadSpecies())
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
        const auto node=taxons_.value(current);
        if (node.rank=="kingdom" || node.rank=="supergroup" || node.rank=="domain") return node.name;
        visited.insert(current);
        current = taxons_.value(current).parentId;
    }
    return {};
}

QVariantMap TaxonomyIndex::describe(const QString &id) const
{
    const Taxon taxon = taxons_.value(id);
    const QString kingdom = kingdomFor(id);
    const QString sourceUrl=!taxon.sourceUrl.isEmpty() ? taxon.sourceUrl
        : taxon.source=="inaturalist" && !taxon.sourceTaxonId.isEmpty() ? "https://www.inaturalist.org/taxa/"+taxon.sourceTaxonId : QString();
    int visualCount=0,supportingCount=0;
    for (const Species &s : speciesFor(id)) if (s.taxonomyApproved && s.photoApproved && s.photo.valid && !s.photo.approvedRanks.isEmpty()) {
        if (s.photo.role=="supporting") ++supportingCount; else ++visualCount;
    }
    return {{"id", id}, {"title", taxon.name}, {"rank", taxon.rank},
        {"photo",media_ ? media_->photoFor(id) : QVariantMap()},
        {"rankLabel", taxonomicRankLabel(taxon.rank)}, {"kingdom", kingdom},
        {"count", count(id)}, {"visualCount",visualCount},{"supportingCount",supportingCount},{"introduction",content_.introduction(id)},
        {"source", taxon.source}, {"sourceTaxonId", taxon.sourceTaxonId},
        {"sourceUrl",sourceUrl}, {"rankNote",taxon.rankNote},
        {"placementNotice",QStringLiteral("Enquadramento filogenético com fontes; revisão humana pendente. Ramos intermediários podem estar omitidos.")},
        {"description", !taxon.description.isEmpty() ? taxon.description
            : QStringLiteral("%1 · Conheça os grupos e suas espécies.").arg(taxonomicRankLabel(taxon.rank))}};
}

QVariantList TaxonomyIndex::domains() const
{
    QVariantList list;
    list.append(QVariantMap{{"id", QString()}, {"title", QStringLiteral("Todas as espécies")},
        {"kingdom", QString()}, {"count", count()}, {"visualCount",describe(QString()).value("visualCount")},
        {"defaultRank",QStringLiteral("genus")}, {"photo",media_ ? media_->photoFor(QString()) : QVariantMap()},
        {"playableCount", eligibleQuizSpecies(species_,QStringLiteral("genus")).size()},
        {"description", QStringLiteral("Identifique gêneros de diferentes linhagens evolutivas.")}});
    QVector<QString> roots=children_.value(QString());
    std::sort(roots.begin(),roots.end(),[this](const QString &a,const QString &b){return taxons_.value(a).name<taxons_.value(b).name;});
    for (const auto &id : roots) if(taxons_.value(id).rank=="domain") list.append(describe(id));
    return list;
}

QVariantList TaxonomyIndex::quickAccess() const
{
    QVariantList list;
    for(const QString &name : {QString("Animalia"),QString("Plantae"),QString("Fungi")}) {
        for(const auto &node : taxons_) if(node.rank=="kingdom" && node.name==name && count(node.id)>0) {
            auto entry=describe(node.id);
            QStringList path;
            for(const auto &ancestor : breadcrumbs(node.id)) path.append(ancestor.toMap().value("title").toString());
            entry.insert("pathText",path.join(" › "));
            list.append(entry);
            break;
        }
    }
    return list;
}

QVariantList TaxonomyIndex::children(const QString &id) const
{
    QVector<QString> ids = children_.value(id);
    std::sort(ids.begin(), ids.end(), [this](const QString &a, const QString &b) {
        return taxons_.value(a).name < taxons_.value(b).name;
    });
    QVariantList list;
    for (const QString &child : ids) if (count(child)>0 || taxons_.value(child).source=="glossata_phylogeny") list.append(describe(child));
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
        list.prepend(describe(current));
        current = node.parentId;
    }
    return list;
}

QVariantList TaxonomyIndex::identificationLevels(const QString &id) const
{
    if (levelCache_.contains(id)) return levelCache_.value(id);
    const QVector<Species> selected = speciesFor(id);
    QVariantList levels;
    for (const QString &rank : {QString("species"), QString("genus"), QString("family"), QString("order"), QString("class"), QString("phylum"), QString("kingdom"),QString("supergroup"),QString("domain")}) {
        QSet<QString> answers;
        int eligible = 0;
        for (const Species &species : eligibleQuizSpecies(selected, rank)) {
            const QString answer = species.answerAtRank(rank).trimmed();
            if (!answer.isEmpty()) { answers.insert(answer.toCaseFolded()); ++eligible; }
        }
        if (answers.size() >= 4) levels.append(QVariantMap{{"id", rank}, {"title", taxonomicRankLabel(rank)},
            {"answers", answers.size()}, {"count", eligible}});
    }
    levelCache_.insert(id,levels);
    return levels;
}
