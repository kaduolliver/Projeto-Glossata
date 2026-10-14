#include "infrastructure/content/SqliteSpeciesCatalog.h"
#include "infrastructure/content/JsonSpeciesCatalog.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>
#include <QSet>
#include <QHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "infrastructure/content/PhotoResolver.h"
#include "infrastructure/content/ReviewResolver.h"

SqliteSpeciesCatalog::SqliteSpeciesCatalog(const QString &databasePath)
    : database_(QSqlDatabase::addDatabase("QSQLITE", "catalog-" + QUuid::createUuid().toString()))
{
    JsonSpeciesCatalog bundled;
    species_ = bundled.loadSpecies();
    taxons_ = bundled.loadTaxons();
    if (!QDir().mkpath(QFileInfo(databasePath).absolutePath())) return;
    database_.setDatabaseName(databasePath);
    if (!database_.open()) return;
    QSqlQuery query(database_);
    query.exec("PRAGMA foreign_keys = ON");
    if (!query.exec("CREATE TABLE IF NOT EXISTS catalog_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
        || !query.exec("CREATE TABLE IF NOT EXISTS taxons (id TEXT PRIMARY KEY, scientific_name TEXT NOT NULL, rank TEXT NOT NULL, parent_id TEXT REFERENCES taxons(id), source TEXT, source_taxon_id TEXT, UNIQUE(source, source_taxon_id))")
        || !query.exec("CREATE TABLE IF NOT EXISTS species (id INTEGER PRIMARY KEY, scientific_name TEXT NOT NULL, common_name TEXT, taxon_id TEXT NOT NULL REFERENCES taxons(id), educational_kingdom TEXT, image_path TEXT, image_author TEXT, image_license TEXT, image_source TEXT, source TEXT, source_taxon_id TEXT)")
        || !query.exec("CREATE INDEX IF NOT EXISTS taxons_parent_idx ON taxons(parent_id)")
        || !query.exec("CREATE INDEX IF NOT EXISTS species_taxon_idx ON species(taxon_id)")
        || !query.exec("CREATE UNIQUE INDEX IF NOT EXISTS species_source_idx ON species(source, source_taxon_id) WHERE source_taxon_id IS NOT NULL")) return;
    QSet<QString> columns;
    query.exec("PRAGMA table_info(species)");
    while (query.next()) columns.insert(query.value(1).toString());
    bool upgraded = false;
    for (const QString &column : QStringList{"fact", "hint", "region", "source_url", "image_license_url", "visual_quiz", "photo_json", "active"}) {
        if (!columns.contains(column)) {
            const QString type=column=="active" ? " INTEGER NOT NULL DEFAULT 1" : column=="visual_quiz" ? " INTEGER DEFAULT 0" : " TEXT";
            if (!query.exec("ALTER TABLE species ADD COLUMN " + column + type)) return;
            upgraded = true;
        }
    }
    QFile speciesFile(":/resources/data/species.json"), taxonomyFile(":/resources/data/taxonomy.json"), replacementsFile(":/resources/data/catalog_replacements.json");
    if (!speciesFile.open(QIODevice::ReadOnly) || !taxonomyFile.open(QIODevice::ReadOnly) || !replacementsFile.open(QIODevice::ReadOnly)) return;
    const QByteArray replacementBytes=replacementsFile.readAll();
    const auto replacements=QJsonDocument::fromJson(replacementBytes).object().value("replacements").toArray();
    const QString fingerprint = QCryptographicHash::hash(speciesFile.readAll() + taxonomyFile.readAll()+replacementBytes, QCryptographicHash::Sha256).toHex();
    query.exec("SELECT value FROM catalog_meta WHERE key = 'bundle_hash'");
    if (query.next() && query.value(0).toString() == fingerprint && !upgraded) {
        query.finish(); ready_ = readCache(); return;
    }
    query.finish();
    if (!database_.transaction()) return;
    const auto fail = [&] {
        qWarning() << "Falha na migração do catálogo:" << query.lastError().text();
        database_.rollback();
    };
    QSet<QString> stored;
    // Parent-first traversal; do not drop tables or remove previous imported records.
    QVector<Taxon> pending = taxons_;
    while (!pending.isEmpty()) {
        bool progress = false;
        for (int i = static_cast<int>(pending.size()) - 1; i >= 0; --i) {
            const Taxon taxon = pending.at(i);
            if (!taxon.parentId.isEmpty() && !stored.contains(taxon.parentId)) continue;
            query.prepare("INSERT INTO taxons (id, scientific_name, rank, parent_id, source, source_taxon_id) VALUES (?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET scientific_name=excluded.scientific_name, rank=excluded.rank, parent_id=excluded.parent_id, source=excluded.source, source_taxon_id=excluded.source_taxon_id");
            query.addBindValue(taxon.id); query.addBindValue(taxon.name); query.addBindValue(taxon.rank);
            query.addBindValue(taxon.parentId.isEmpty() ? QVariant() : QVariant(taxon.parentId));
            query.addBindValue(taxon.source);
            query.addBindValue(taxon.sourceTaxonId.isEmpty() ? QVariant() : QVariant(taxon.sourceTaxonId));
            if (!query.exec()) { fail(); return; }
            stored.insert(taxon.id); pending.removeAt(i); progress = true;
        }
        if (!progress) { fail(); return; }
    }
    for (const Species &item : species_) {
        if (item.taxonomy.isEmpty()) continue;
        query.prepare("INSERT INTO species (id,scientific_name,common_name,taxon_id,educational_kingdom,image_path,image_author,image_license,image_source,source,source_taxon_id,fact,hint,region,source_url,image_license_url,visual_quiz,photo_json) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET scientific_name=excluded.scientific_name, common_name=excluded.common_name, taxon_id=excluded.taxon_id, educational_kingdom=excluded.educational_kingdom, image_path=excluded.image_path, image_author=excluded.image_author, image_license=excluded.image_license, image_source=excluded.image_source, source=excluded.source, source_taxon_id=excluded.source_taxon_id, fact=excluded.fact,hint=excluded.hint,region=excluded.region,source_url=excluded.source_url,image_license_url=excluded.image_license_url,visual_quiz=excluded.visual_quiz,photo_json=excluded.photo_json");
        for (const QVariant &value : QVector<QVariant>{item.id, item.name, item.commonName, item.taxonomy.last().id,
            item.educationalKingdom, item.image, item.imageAuthor, item.imageLicense, item.imageSource, item.source,
            item.sourceTaxonId.isEmpty() ? QVariant() : QVariant(item.sourceTaxonId),
            item.fact,item.hint,item.region,item.sourceUrl,item.imageLicenseUrl,item.visualQuiz,QString::fromUtf8(QJsonDocument(PhotoResolver::toJson(item.photo)).toJson(QJsonDocument::Compact))}) query.addBindValue(value);
        if (!query.exec()) { fail(); return; }
        query.prepare("UPDATE species SET active=1 WHERE id=?");
        query.addBindValue(item.id);
        if (!query.exec()) { fail(); return; }
    }
    // Retain former organisms in the database. Only explicit replacements are
    // retired; unrelated local records, profiles and ranking are untouched.
    for (const auto &value : replacements) {
        const auto replacement=value.toObject();
        const auto old=replacement.value("old").toObject();
        query.prepare("UPDATE species SET active=0 WHERE id=? AND scientific_name=? AND EXISTS(SELECT 1 FROM species WHERE id=? AND active=1)");
        query.addBindValue(old.value("id").toInt());
        query.addBindValue(old.value("name").toString());
        query.addBindValue(replacement.value("new_id").toInt());
        if (!query.exec()) { fail(); return; }
    }
    query.prepare("INSERT INTO catalog_meta(key,value) VALUES('bundle_hash',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value");
    query.addBindValue(fingerprint);
    if (!query.exec() || !database_.commit()) { fail(); return; }
    ready_ = readCache();
}

bool SqliteSpeciesCatalog::readCache()
{
    QSqlQuery query(database_);
    if (!query.exec("SELECT id,scientific_name,rank,parent_id,source,source_taxon_id FROM taxons")) return false;
    QVector<Taxon> taxons;
    QHash<QString, Taxon> index;
    while (query.next()) {
        Taxon node{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
                   query.value(3).toString(),query.value(4).toString(),query.value(5).toString()};
        taxons.append(node); index.insert(node.id,node);
    }
    if (!query.exec("SELECT id,scientific_name,common_name,taxon_id,educational_kingdom,image_path,image_author,image_license,image_source,source,source_taxon_id,fact,hint,region,source_url,image_license_url,visual_quiz,photo_json FROM species WHERE active=1 ORDER BY id")) return false;
    QVector<Species> species;
    while (query.next()) {
        Species item;
        item.id=query.value(0).toInt(); item.name=query.value(1).toString(); item.commonName=query.value(2).toString();
        item.educationalKingdom=query.value(4).toString(); item.image=query.value(5).toString();
        item.imageAuthor=query.value(6).toString(); item.imageLicense=query.value(7).toString(); item.imageSource=query.value(8).toString();
        item.source=query.value(9).toString(); item.sourceTaxonId=query.value(10).toString();
        item.fact=query.value(11).toString(); item.hint=query.value(12).toString(); item.region=query.value(13).toString();
        item.sourceUrl=query.value(14).toString(); item.imageLicenseUrl=query.value(15).toString(); item.visualQuiz=query.value(16).toBool();
        QString id=query.value(3).toString();
        QSet<QString> visited;
        while (index.contains(id) && !visited.contains(id)) {
            visited.insert(id);
            const Taxon node=index.value(id);
            item.taxonomy.prepend(node);
            if (node.rank == "family") item.family=node.name;
            id=node.parentId;
        }
        const auto metadata=QJsonDocument::fromJson(query.value(17).toString().toUtf8()).object();
        PhotoResolver::attach(item,PhotoResolver::fromJson(metadata,item.image));
        ReviewResolver::apply(item);
        species.append(item);
    }
    taxons_=taxons; species_=species;
    return true;
}

SqliteSpeciesCatalog::~SqliteSpeciesCatalog()
{
    const QString name = database_.connectionName();
    database_.close(); database_ = QSqlDatabase(); QSqlDatabase::removeDatabase(name);
}

int SqliteSpeciesCatalog::descendantCount(const QString &taxonId) const
{
    if (!ready_) return 0;
    QSqlQuery query(database_);
    query.prepare("WITH RECURSIVE descendants(id) AS (SELECT id FROM taxons WHERE id=? UNION SELECT t.id FROM taxons t JOIN descendants d ON t.parent_id=d.id) SELECT COUNT(*) FROM species WHERE active=1 AND taxon_id IN (SELECT id FROM descendants)");
    query.addBindValue(taxonId);
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}
