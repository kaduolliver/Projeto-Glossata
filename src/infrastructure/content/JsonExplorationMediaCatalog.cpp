#include "infrastructure/content/JsonExplorationMediaCatalog.h"
#include <QCryptographicHash>
#include <QFile>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QUrl>

namespace {
QByteArray read(const QString &path) {
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}
bool matches(const QByteArray &bytes, const QString &hash) {
    return !bytes.isEmpty() && hash.size()==64
        && QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()==hash.toLatin1();
}
bool local(const QString &path, const QString &prefix) {
    return path.startsWith(prefix) && !path.contains("..") && !path.contains('\\');
}
QVariantMap verifiedPhoto(const QJsonObject &photo, const QString &root) {
    const QString path=photo["path"].toString(), evidence=photo["evidence_path"].toString();
    if (!local(path,"resources/images/exploration/") || !local(evidence,"resources/compliance/exploration/")) return {};
    const QByteArray proofBytes=read(root+"/"+evidence);
    if (!matches(proofBytes,photo["evidence_sha256"].toString())
        || !matches(read(root+"/"+path),photo["sha256"].toString())) return {};
    const auto proof=QJsonDocument::fromJson(proofBytes).object();
    const auto record=proof["record"].toObject(), metadata=record["metadata"].toObject();
    const QString license=photo["license"].toString(), version=photo["license_version"].toString();
    const QString licenseUrl=photo["license_url"].toString();
    const QString sourceName=metadata["LicenseShortName"].toObject()["value"].toString();
    const QUrl sourceLicense(metadata["LicenseUrl"].toObject()["value"].toString());
    QString sourcePath=sourceLicense.path();
    if(license=="cc0" && sourcePath.endsWith("/deed.en")) sourcePath.chop(8);
    if(sourcePath.endsWith('/'))sourcePath.chop(1);
    const QString expected=license=="cc0" && version=="1.0" && sourceName=="CC0"
        ? "https://creativecommons.org/publicdomain/zero/1.0/"
        : license=="cc-by" && QStringList{"1.0","2.0","2.5","3.0","4.0"}.contains(version)
            && sourceName=="CC BY "+version ? "https://creativecommons.org/licenses/by/"+version+"/" : QString();
    if (expected.isEmpty() || licenseUrl!=expected || sourceLicense.host()!="creativecommons.org"
        || sourcePath!=QUrl(expected).path().remove(QRegularExpression("/$"))
        || !metadata["Restrictions"].toObject()["value"].toString().isEmpty()) return {};
    if (photo["photo_id"].toString()!="commons:"+QString::number(record["page_id"].toInteger())
        || photo["source_revision_id"]!=record["revision_id"] || photo["download_url"]!=record["download_url"]
        || photo["collected_at"]!=proof["collected_at"] || photo["author"].toString().isEmpty()
        || photo["subject"].toString().isEmpty() || photo["modifications"].toString().isEmpty()
        // Qt inserts U+FFFC for Commons' decorative edit icons; it is not author credit.
        || photo["author"].toString()!=QTextDocumentFragment::fromHtml(metadata["Artist"].toObject()["value"].toString()).toPlainText().remove(QChar(0xfffc)).simplified()) return {};
    QImageReader reader(root+"/"+path);
    const QSize size=reader.size();
    if (!reader.canRead() || size!=QSize(photo["width"].toInt(),photo["height"].toInt())
        || qMin(size.width(),size.height())<160 || qMax(size.width(),size.height())>1024) return {};
    for(const auto &key:{"focus_x","focus_y"})if(photo.contains(key) && !photo[key].isDouble())return {};
    const double focusX=photo["focus_x"].toDouble(0.5),focusY=photo["focus_y"].toDouble(0.5);
    if(focusX<0 || focusX>1 || focusY<0 || focusY>1)return {};
    return {{"image",root==":" ? "qrc:/"+path : QUrl::fromLocalFile(root+"/"+path).toString()},
        {"subject",photo["subject"].toString()}, {"author",photo["author"].toString()},
        {"license",license.toUpper()+" "+version}, {"licenseUrl",licenseUrl},
        {"photoUrl",photo["photo_url"].toString()}, {"kind",photo["kind"].toString()},
        {"identificationNote",photo["identification_note"].toString()},
        {"focusX",focusX},{"focusY",focusY},
        {"modifications",photo["modifications"].toString()}};
}
}

JsonExplorationMediaCatalog::JsonExplorationMediaCatalog(const QString &root) {
    const auto data=QJsonDocument::fromJson(read(root+"/resources/data/exploration_photos.json")).object();
    QHash<QString,QVariantMap> verified;
    const auto photos=data["photos"].toObject();
    for(auto it=photos.begin();it!=photos.end();++it) {
        const auto photo=verifiedPhoto(it.value().toObject(),root);
        if(!photo.isEmpty()) verified.insert(it.key(),photo);
    }
    const auto bindings=data["bindings"].toObject();
    QHash<int,QJsonObject> organisms;
    QHash<QString,QJsonObject> taxons;
    for(const auto &entry:QJsonDocument::fromJson(read(root+"/resources/data/taxonomy.json")).array()) {
        const auto taxon=entry.toObject();taxons.insert(taxon["id"].toString(),taxon);
    }
    for(const auto &entry:QJsonDocument::fromJson(read(root+"/resources/data/species.json")).array()) {
        const auto organism=entry.toObject();organisms.insert(organism["id"].toInt(),organism);
    }
    for(auto it=bindings.begin();it!=bindings.end();++it) {
        const QString key=it.value().toString();
        const auto photo=photos[key].toObject();
        bool compatible=false;
        if(photo.contains("catalog_species_id")) {
            const auto organism=organisms.value(photo["catalog_species_id"].toInt());
            compatible=organism["name"]==photo["subject"] && organism["taxonomy"].toArray().contains(it.key());
        } else {
            const auto scope=photo["scope"].toObject();
            compatible=scope["taxon_ids"].toArray().contains(it.key()) && !scope["rationale"].toString().isEmpty()
                && scope["evidence_url"]==photo["photo_url"];
            if(scope.contains("classification_evidence")) {
                const QString proofPath=scope["classification_evidence"].toString();
                const auto proof=local(proofPath,"resources/compliance/exploration/") ? read(root+"/"+proofPath) : QByteArray();
                const auto assertion=QJsonDocument::fromJson(proof).object()["assertion"].toObject();
                bool found=assertion["canonical_name"]==taxons[it.key()]["name"];
                for(const auto &ancestor:assertion["lineage"].toArray())if(ancestor.toObject()["name"]==taxons[it.key()]["name"])found=true;
                compatible=compatible && matches(proof,scope["classification_evidence_sha256"].toString())
                    && assertion["canonical_name"]==photo["subject"] && assertion["status"]=="accepted" && found;
            }
        }
        if((it.key()=="domain:Bacteria" || it.key()=="domain:Archaea") && photo["kind"]!="micrograph")compatible=false;
        if(!taxons.contains(it.key()) || (taxons[it.key()]["rank"]=="species" && taxons[it.key()]["name"]!=photo["subject"]))compatible=false;
        if(compatible && verified.contains(key))photos_.insert(it.key(),verified.value(key));
    }
    const auto pending=data["pending"].toObject();
    for(auto it=pending.begin();it!=pending.end();++it)
        if(!photos_.contains(it.key()))photos_.insert(it.key(),{{"pendingReason",it.value().toObject()["reason"].toString()}});
    if(verified.contains(data["mixed_photo"].toString())) photos_.insert(QString(),verified.value(data["mixed_photo"].toString()));
}

QVariantMap JsonExplorationMediaCatalog::photoFor(const QString &taxonId) const {
    return photos_.value(taxonId); // Never borrow a different group's photograph.
}
