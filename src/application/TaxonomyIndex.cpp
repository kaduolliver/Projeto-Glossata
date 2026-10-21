#include "application/TaxonomyIndex.h"
#include <QSet>
#include <QMap>
#include <QUrl>
#include <QRegularExpression>
#include <algorithm>
#include "domain/QuizSelection.h"

TaxonomyIndex::TaxonomyIndex(const SpeciesCatalog &catalog, const ExplorationMediaCatalog *media,
                             const EducationalContent &content)
    : catalog_(catalog), media_(media), content_(content)
{
    QSet<int> seen;
    for (const auto &organism : catalog.loadCurrentSpecies())
        if (organism.id > 0 && !seen.contains(organism.id)) { species_.append(organism); seen.insert(organism.id); }
    for (const Taxon &taxon : catalog.loadCurrentTaxons()) {
        if (taxon.id.isEmpty() || taxons_.contains(taxon.id)) continue;
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

QVariantList TaxonomyIndex::discoveryCandidates(int &organismCount, int &photographs, QStringList &domainNames, bool currentOnly) const
{
    // Read one coherent catalog snapshot. Stable IDs, rather than rows, photos
    // or historical educational groups, define the three organism counts.
    QMap<int, Species> organisms;
    for (const auto &s : currentOnly ? catalog_.loadCurrentSpecies() : catalog_.loadSpecies())
        if (s.id > 0 && !organisms.contains(s.id)) organisms.insert(s.id, s);
    organismCount = organisms.size();
    QSet<QString> domains;
    photographs = 0;
    domainNames.clear();
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
    return candidates;
}

QVariantMap TaxonomyIndex::homeCatalog(const QDate &localDate) const
{
    int organisms = 0, photographs = 0;
    QStringList domainNames;
    const auto candidates = discoveryCandidates(organisms, photographs, domainNames);
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
        {"organisms", organisms}, {"representedDomains", domainNames.size()},
        {"domainNames", domainNames.join(QStringLiteral(" · "))}, {"identificationPhotos", photographs}}}};
}

QVariantMap TaxonomyIndex::navigationEntry(const QString &id) const
{
    auto entry = describe(id);
    QStringList path;
    QString domainId;
    QSet<QString> visited;
    QString current = id;
    while (taxons_.contains(current) && !visited.contains(current)) {
        visited.insert(current);
        const auto node = taxons_.value(current);
        path.prepend(node.name);
        if (node.rank == "domain") domainId = node.id;
        current = node.parentId;
    }
    entry.insert("pathText", path.join(QStringLiteral(" › ")));
    entry.insert("domainId", domainId);
    entry.insert("kind", "group");
    return entry;
}

namespace {
QString searchText(const QString &text)
{
    QString result;
    for (const QChar c : text.normalized(QString::NormalizationForm_D).toCaseFolded())
        if (c.category() != QChar::Mark_NonSpacing && c.category() != QChar::Mark_SpacingCombining
            && c.category() != QChar::Mark_Enclosing) result.append(c);
    return result.simplified();
}
}

void TaxonomyIndex::buildSearchIndex() const
{
    if (searchIndexed_) return;
    QSet<QString> organismTaxons;
    for (const auto &s : species_) {
        QString id;
        for (const auto &node : s.taxonomy)
            if (node.rank == "species" && node.name == s.name && taxons_.contains(node.id)) id = node.id;
        if (id.isEmpty()) continue; // Never guess a scientific destination.
        auto entry = navigationEntry(id);
        const auto profile = content_.profile(s);
        const auto commonName = profile.value("commonName").toString().trimmed();
        entry.insert("kind", "organism");
        entry.insert("speciesId", s.id);
        entry.insert("commonName", commonName.isEmpty() ? s.commonName : commonName);
        searchIndex_.append(entry);
        organismTaxons.insert(id);
    }
    for (const auto &node : taxons_)
        if (node.rank != "educational_kingdom" && !organismTaxons.contains(node.id))
            searchIndex_.append(navigationEntry(node.id));
    searchIndexed_ = true;
}

QVariantList TaxonomyIndex::searchRanks() const
{
    QSet<QString> ranks;
    for (const auto &node : taxons_) if (node.rank != "educational_kingdom" && !node.rank.isEmpty()) ranks.insert(node.rank);
    QStringList ordered;
    for (const QString &rank : {"domain", "supergroup", "kingdom", "clade", "phylum", "class", "order", "family", "genus", "species"})
        if (ranks.remove(rank)) ordered.append(rank);
    auto remainder = ranks.values(); remainder.sort(); ordered.append(remainder);
    QVariantList result{{QVariantMap{{"id", ""}, {"title", QStringLiteral("Todos os níveis")}}}};
    for (const auto &rank : ordered) result.append(QVariantMap{{"id", rank}, {"title", taxonomicRankLabel(rank)}});
    return result;
}

QVariantMap TaxonomyIndex::search(const QString &query, const QString &domainId, const QString &rank) const
{
    buildSearchIndex();
    const auto words = searchText(query).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QVector<QPair<int, QVariantMap>> matches;
    for (const auto &value : searchIndex_) {
        const auto entry = value.toMap();
        if ((!domainId.isEmpty() && entry.value("domainId").toString() != domainId)
            || (!rank.isEmpty() && entry.value("rank").toString() != rank)) continue;
        const QString name = searchText(entry.value("title").toString());
        const QString common = searchText(entry.value("commonName").toString());
        const QString searchable = name + " " + common + " " + searchText(entry.value("pathText").toString())
            + " " + searchText(entry.value("rankLabel").toString());
        if (!std::all_of(words.begin(), words.end(), [&](const QString &word) { return searchable.contains(word); })) continue;
        const auto needle = searchText(query);
        const int relevance = name == needle ? 0 : name.startsWith(needle) ? 1 : name.contains(needle) ? 2
            : common.contains(needle) ? 3 : 4;
        matches.append({relevance, entry});
    }
    std::sort(matches.begin(), matches.end(), [](const auto &a, const auto &b) {
        if (a.first != b.first) return a.first < b.first;
        const auto an = a.second.value("title").toString(), bn = b.second.value("title").toString();
        if (an != bn) return an < bn;
        if (a.second.value("id") != b.second.value("id")) return a.second.value("id").toString() < b.second.value("id").toString();
        return a.second.value("speciesId").toInt() < b.second.value("speciesId").toInt();
    });
    QVariantList groups, organisms;
    for (const auto &match : matches)
        (match.second.value("kind") == "organism" ? organisms : groups).append(match.second);
    return {{"groups", groups}, {"organisms", organisms}, {"total", matches.size()},
        {"groupCount", groups.size()}, {"organismCount", organisms.size()}};
}

QVariantList TaxonomyIndex::featuredOrganisms() const
{
    int organisms = 0, photographs = 0;
    QStringList domains;
    const auto candidates = discoveryCandidates(organisms, photographs, domains, true);
    QVariantList result, remaining;
    QSet<QString> groups;
    for (const auto &candidate : candidates) {
        const auto item = candidate.toMap();
        const auto id = item.value("taxonId").toString();
        if (!taxons_.contains(id)) continue;
        auto entry = navigationEntry(id);
        entry.insert("kind", "organism");
        entry.insert("speciesId", item.value("speciesId"));
        entry.insert("commonName", item.value("commonName"));
        // Keep the eligibility, original photo and attribution used by the
        // daily discovery. No supporting image or new approval is introduced.
        auto photo = item.value("photo").toMap();
        const auto attribution = photo.value("attribution").toMap();
        // These legacy photos already pass PhotoResolver's local-use checks.
        // Move their supplied CC BY credit to the same adjacent offline dialog,
        // without inferring a version or changing the distribution decision.
        photo.insert("contextualAttribution", attribution.value("license").toString() == "cc-by"
            && attribution.value("author") == photo.value("author")
            && !attribution.value("evidence_paths").toList().isEmpty());
        photo.insert("subject", item.value("scientificName"));
        photo.insert("identificationNote", QStringLiteral("Identificação por espécie elegível no catálogo; consulte origem e limitações."));
        entry.insert("photo", photo);
        const auto group = kingdomFor(id);
        if (!groups.contains(group) && result.size() < 4) { result.append(entry); groups.insert(group); }
        else remaining.append(entry);
    }
    for (const auto &entry : remaining) { if (result.size() >= 4) break; result.append(entry); }
    return result;
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
    QString commonName;
    QVariantMap summary;
    bool organism = false;
    for (const auto &s : speciesFor(id)) if (taxon.rank == "species" && s.name == taxon.name) {
        organism = true;
        const auto profile = content_.profile(s);
        const auto name = profile.value("commonName").toString().trimmed();
        summary = profile.value("summary").toMap();
        commonName = name.isEmpty() ? s.commonName : name;
        break;
    }
    const auto introduction = content_.introduction(id);
    auto educationalNote = organism ? summary : introduction;
    const auto noteState = educationalNote.value("governance").toMap().value("state").toString();
    const QUrl noteSource(educationalNote.value("source_url").toString());
    // EducationalContent has already checked the recorded review's usability.
    // Cards accept only attributed editorial/expert text, never description fallbacks.
    if ((noteState != "approved_editorial" && noteState != "approved_expert")
        || educationalNote.value("text").toString().trimmed().isEmpty()
        || educationalNote.value("source_title").toString().trimmed().isEmpty()
        || !noteSource.isValid() || noteSource.scheme() != "https" || noteSource.host().isEmpty()) educationalNote.clear();
    QVector<Taxon> ancestors;
    QSet<QString> visited;
    QString parent = taxon.parentId;
    while (taxons_.contains(parent) && !visited.contains(parent)) {
        visited.insert(parent);
        ancestors.prepend(taxons_.value(parent));
        parent = taxons_.value(parent).parentId;
    }
    QVector<Taxon> classification;
    // Prefer existing family/genus nodes. Rank labels also distinguish homonyms.
    for (const QString &rank : {QString("family"), QString("genus")})
        for (auto it = ancestors.crbegin(); it != ancestors.crend(); ++it)
            if (it->rank == rank) { classification.append(*it); break; }
    if (classification.isEmpty()) classification = ancestors.mid(qMax<qsizetype>(0, ancestors.size() - 2));
    QStringList classificationText;
    for (const auto &node : classification)
        classificationText.append(taxonomicRankLabel(node.rank) + ": " + node.name);
    const auto review = (summary.isEmpty() ? introduction : summary).value("governance").toMap().value("state").toString();
    for (const Species &s : speciesFor(id)) if (s.taxonomyApproved && s.photoApproved && s.photo.valid && !s.photo.approvedRanks.isEmpty()) {
        if (s.photo.role=="supporting") ++supportingCount; else ++visualCount;
    }
    return {{"id", id}, {"title", taxon.name}, {"rank", taxon.rank},
        {"kind", organism ? "organism" : "group"}, {"commonName", commonName}, {"summary", summary},
        {"educationalNote", educationalNote}, {"classificationSummary", classificationText.join(QStringLiteral(" · "))},
        {"introductionReviewNotice", review == "approved_expert" ? QStringLiteral("Texto com revisão especializada registrada; consulte o escopo na fonte.")
            : review == "approved_editorial" ? QStringLiteral("Texto com revisão editorial; não equivale a revisão especializada.")
            : QStringLiteral("Introdução sem revisão utilizável registrada para este grupo.")},
        {"photo",media_ ? media_->photoFor(id) : QVariantMap()},
        {"rankLabel", taxonomicRankLabel(taxon.rank)}, {"kingdom", kingdom},
        {"count", count(id)}, {"visualCount",visualCount},{"supportingCount",supportingCount},{"introduction",introduction},
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
