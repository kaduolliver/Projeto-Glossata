#include "infrastructure/content/PhotoResolver.h"
#include <QFile>
#include <QImageReader>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QHash>
#include <QSize>
#include <QFileInfo>
#include <QUrl>
#include <QJsonDocument>

Photo PhotoResolver::fromJson(const QJsonObject &m, const QString &image)
{
    Photo p;
    p.id=m.value("id").toString(); p.image=image; p.scientificName=m.value("scientific_name").toString();
    p.taxonId=m.value("taxon_id").toString(); p.author=m.value("author").toString();
    p.license=m.value("license").toString(); p.licenseUrl=m.value("license_url").toString();
    p.sourceUrl=m.value("source").toString(); p.downloadUrl=m.value("url").toString();
    p.sha256=m.value("sha256").toString(); p.modifications=m.value("modifications").toString();
    p.sourceVerified=m.value("source_verified").toBool();
    static const QJsonObject credits = [] {
        QFile file(":/resources/data/credits.json");
        return file.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(file.readAll()).object().value("photos").toObject() : QJsonObject{};
    }();
    p.attribution=credits.value(p.id).toObject();
    p.role=m.value("role").toString("identification");
    p.supportingFor=m.value("supporting_for").toString();
    for (const auto &rank : m.value("approved_ranks").toArray()) p.approvedRanks.append(rank.toString());
    return p;
}
QJsonObject PhotoResolver::toJson(const Photo &p)
{
    return {{"id",p.id},{"image",p.image},{"scientific_name",p.scientificName},{"taxon_id",p.taxonId},
        {"author",p.author},{"license",p.license},{"license_url",p.licenseUrl},{"source",p.sourceUrl},
        {"url",p.downloadUrl},{"sha256",p.sha256},{"modifications",p.modifications},
        {"approved_ranks",QJsonArray::fromStringList(p.approvedRanks)},{"source_verified",p.sourceVerified},
        {"role",p.role},{"supporting_for",p.supportingFor}};
}
Photo PhotoResolver::validate(Photo p, const QString &expectedName)
{
    p.valid=false;
    const auto reject=[&p](const QString &error) { p.error=error; return p; };
    const bool association=p.role=="identification" ? p.scientificName==expectedName
        : p.role=="supporting" && p.supportingFor==expectedName && !p.scientificName.isEmpty();
    if (!p.sourceVerified || p.id.isEmpty() || p.taxonId.isEmpty() || !association)
        return reject(QStringLiteral("Associação taxonômica da fotografia não validada."));
    if (QFileInfo(p.image).completeBaseName()!=p.id
        || !QUrl(p.downloadUrl).path().contains("/photos/"+p.id+"/"))
        return reject(QStringLiteral("Identidade do arquivo da fotografia não confere."));
    const QString version=p.attribution.value("license_version").toString("4.0");
    const QString requiredLicense = p.license == "cc0" ? "https://creativecommons.org/publicdomain/zero/1.0/"
        : p.license == "cc-by" && QStringList{"1.0","2.0","2.5","3.0","4.0"}.contains(version)
            ? "https://creativecommons.org/licenses/by/"+version+"/" : QString();
    const bool documentedException=p.attribution.value("distribution_exception").toBool()
        && p.attribution.value("sha256").toString()==p.sha256
        && p.attribution.value("license").toString()==p.license
        && p.attribution.value("license_url").toString()==p.licenseUrl
        && p.attribution.value("rights_issues").toArray().isEmpty();
    if (!documentedException && (requiredLicense.isEmpty() || p.licenseUrl != requiredLicense))
        return reject(QStringLiteral("Licença da fotografia não aprovada."));
    if (p.author.trimmed().isEmpty() || p.author == "no rights reserved" || p.sourceUrl.isEmpty()
        || p.downloadUrl.isEmpty() || p.modifications.isEmpty() || p.sha256.size()!=64)
        return reject(QStringLiteral("Créditos ou rastreabilidade incompletos."));
    if (QUrl(p.sourceUrl).scheme()!="https" || QUrl(p.sourceUrl).host().isEmpty()
        || QUrl(p.downloadUrl).scheme()!="https") return reject(QStringLiteral("Fonte da fotografia inválida."));
    if (!p.image.startsWith("qrc:/resources/images/")) return reject(QStringLiteral("Fotografia local indisponível."));
    // RCC content is immutable within a process; cache successful byte/decoder checks.
    static QHash<QString,QSize> decodedResources;
    const QString key=p.image+"|"+p.sha256;
    if (decodedResources.contains(key)) {
        const QSize size=decodedResources.value(key);
        p.width=size.width(); p.height=size.height(); p.valid=true; p.error.clear(); return p;
    }
    QFile file(p.image.mid(3));
    if (!file.open(QIODevice::ReadOnly)) return reject(QStringLiteral("Arquivo da fotografia não encontrado."));
    if (QCryptographicHash::hash(file.readAll(),QCryptographicHash::Sha256).toHex() != p.sha256.toLatin1())
        return reject(QStringLiteral("Integridade da fotografia não confere."));
    QImageReader reader(p.image.mid(3));
    const QImage decoded=reader.read();
    if (decoded.isNull()) return reject(QStringLiteral("Não foi possível decodificar a fotografia."));
    p.width=decoded.width(); p.height=decoded.height();
    if (qMin(p.width,p.height)<300) return reject(QStringLiteral("Resolução insuficiente para identificação."));
    decodedResources.insert(key,decoded.size());
    p.valid=true; p.error.clear(); return p;
}
void PhotoResolver::attach(Species &s, Photo photo)
{
    if (photo.role=="supporting") {
        if (s.educationalKingdom!="Monera") photo.sourceVerified=false;
    } else if (!s.sourceTaxonId.isEmpty() && photo.taxonId!=s.sourceTaxonId) photo.sourceVerified=false;
    s.photo=validate(photo,s.name);
    s.image=s.photo.valid ? s.photo.image : QString();
    s.imageAuthor=s.photo.author; s.imageLicense=s.photo.license; s.imageLicenseUrl=s.photo.licenseUrl;
    s.imageSource=s.photo.sourceUrl; s.visualQuiz=s.photo.valid && s.photo.role=="identification" && s.photo.approvedRanks.contains("species");
}
