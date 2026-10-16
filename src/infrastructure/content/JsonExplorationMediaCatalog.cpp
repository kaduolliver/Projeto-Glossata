#include "infrastructure/content/JsonExplorationMediaCatalog.h"
#include <QCryptographicHash>
#include <QFile>
#include <QImageReader>
#include <QJsonDocument>
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
        || photo["author"].toString()!=QTextDocumentFragment::fromHtml(metadata["Artist"].toObject()["value"].toString()).toPlainText().simplified()) return {};
    QImageReader reader(root+"/"+path);
    const QSize size=reader.size();
    if (!reader.canRead() || size!=QSize(photo["width"].toInt(),photo["height"].toInt())
        || qMin(size.width(),size.height())<160 || qMax(size.width(),size.height())>1024) return {};
    return {{"image",root==":" ? "qrc:/"+path : QUrl::fromLocalFile(root+"/"+path).toString()},
        {"subject",photo["subject"].toString()}, {"author",photo["author"].toString()},
        {"license",license.toUpper()+" "+version}, {"licenseUrl",licenseUrl},
        {"photoUrl",photo["photo_url"].toString()}, {"kind",photo["kind"].toString()},
        {"identificationNote",photo["identification_note"].toString()},
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
    for(auto it=bindings.begin();it!=bindings.end();++it)
        if(verified.contains(it.value().toString())) photos_.insert(it.key(),verified.value(it.value().toString()));
    if(verified.contains(data["mixed_photo"].toString())) photos_.insert(QString(),verified.value(data["mixed_photo"].toString()));
}

QVariantMap JsonExplorationMediaCatalog::photoFor(const QString &taxonId) const {
    return photos_.value(taxonId); // Never borrow a different group's photograph.
}
