#include "application/TaxonomyIndex.h"
#include <QSet>
#include <QMap>
#include <QUrl>
#include <algorithm>
#include "domain/QuizSelection.h"

TaxonomyIndex::TaxonomyIndex(const SpeciesCatalog &catalog, const ExplorationMediaCatalog *media,
                             const EducationalContent &content)
    : catalog_(catalog), media_(media), species_(catalog.loadSpecies()), content_(content)
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

QVariantMap TaxonomyIndex::homeCatalog(const QDate &localDate) const
{
    // Read one coherent catalog snapshot. Stable IDs, rather than rows, photos
    // or historical educational groups, define the three organism counts.
    QMap<int, Species> organisms;
    for (const auto &s : catalog_.loadSpecies())
        if (s.id > 0 && !organisms.contains(s.id)) organisms.insert(s.id, s);
    QSet<QString> domains;
    QStringList domainNames;
    int photographs = 0;
    QVariantList candidates;
    for (const auto &s : organisms) {
        QString speciesTaxon;
        for (const auto &node : s.taxonomy) {
            if (node.rank == "domain" && !node.id.isEmpty() && !domains.contains(node.id)) {
                domains.insert(node.id);
                domainNames.append(node.name);
            }
            if (node.rank == "species" && node.name == s.name && taxons_.contains(node.id)) speciesTaxon = node.id;
        }
        // Reuse the existing per-organism eligibility rule, without requiring
        // four quiz alternatives. Supporting photos never identify a species.
        if (s.photo.role != "identification" || !approvedForQuiz(s, "species")) continue;
        ++photographs;
        const auto profile = content_.profile(s);
        const auto note = profile.value("summary").toMap();
        const auto review = note.value("governance").toMap();
        const auto state = review.value("state").toString();
        const QUrl source(note.value("source_url").toString());
        if (speciesTaxon.isEmpty() || note.value("text").toString().trimmed().isEmpty()
            || note.value("source_title").toString().trimmed().isEmpty()
            || !source.isValid() || source.scheme() != "https" || source.host().isEmpty()
            || (state != "approved_editorial" && state != "approved_expert")) continue;
        const auto &p = s.photo;
        const auto attribution = p.attribution.toVariantMap();
        const auto commonName = profile.value("commonName").toString().trimmed();
        const QString version = attribution.value("license_version").toString();
        const QString license = p.license.toUpper() + (version.isEmpty()
            ? QStringLiteral(" · versão não comprovada") : " " + version);
        candidates.append(QVariantMap{{"speciesId", s.id}, {"taxonId", speciesTaxon},
            {"scientificName", s.name}, {"commonName", commonName.isEmpty() ? s.commonName : commonName},
            {"note", note}, {"reviewNotice", state == "approved_expert"
                ? QStringLiteral("Texto com revisão especializada registrada; consulte a fonte e o escopo.")
                : QStringLiteral("Texto com revisão editorial; não equivale a revisão especializada.")},
            {"photo", QVariantMap{{"image", p.image}, {"photoId", p.id}, {"author", p.author},
                {"title", attribution.value("title")}, {"license", license}, {"licenseUrl", p.licenseUrl},
                {"sourceUrl", attribution.value("photo_url", p.sourceUrl)}, {"modifications", p.modifications},
                {"attribution", attribution}, {"width", p.width}, {"height", p.height}}}});
    }
    domainNames.sort();
    QVariantMap discovery;
    if (localDate.isValid() && !candidates.isEmpty()) {
        // A fixed cyclic order has no collisions on adjacent dates for n > 1.
        // QDate's civil day number also works across leap days and DST changes.
        const qint64 n = candidates.size();
        const auto index = (localDate.toJulianDay() % n + n) % n;
        discovery = candidates[index].toMap();
        discovery.insert("date", localDate.toString(Qt::ISODate));
        discovery.insert("dateLabel", localDate.toString("dd/MM/yyyy"));
    }
    return {{"discovery", discovery}, {"indicators", QVariantMap{
        {"organisms", organisms.size()}, {"representedDomains", domains.size()},
        {"domainNames", domainNames.join(QStringLiteral(" · "))}, {"identificationPhotos", photographs}}}};
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

QVariantList TaxonomyIndex::homeSlides() const
{
    QVariantList slides;
    QSet<QString> images;
    // Stable group IDs avoid homonymous historical nodes retained by SQLite.
    // These are the same explicit, verified associations used by exploration.
    // EducationalContent only returns introductions with usable existing reviews.
    for (const QString &id : {"taxon:cac47417164883d60794",  // Animalia
                             "taxon:5ad28aa5499d178f4f0d",  // Arthropoda
                             "taxon:56956496cf600779dc1f",  // Plantae
                             "taxon:31fdb10b7300d45a9dcf"}) { // Fungi
        if (!taxons_.contains(id)) continue;
        auto slide = describe(id);
        auto photo = slide.value("photo").toMap();
        if (!photo.value("homeReplacement").toMap().isEmpty()) photo = photo.value("homeReplacement").toMap();
        else if (!photo.value("homeVariant").toMap().isEmpty()) photo = photo.value("homeVariant").toMap();
        slide.insert("photo", photo);
        const auto introduction = slide.value("introduction").toMap();
        const QString image = photo.value("image").toString();
        const QString state = introduction.value("governance").toMap().value("state").toString();
        if (image.isEmpty() || (state != "approved_editorial" && state != "approved_expert")
            || images.contains(image)) continue;
        const QString text = introduction.value("text").toString();
        const int end = text.indexOf(". ");
        slide.insert("excerpt", end < 0 ? text : text.left(end + 1));
        slides.append(slide);
        images.insert(image);
    }
    return slides;
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
