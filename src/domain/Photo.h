#pragma once
#include <QString>
#include <QStringList>
#include <QJsonObject>
struct Photo {
    QString id, image, scientificName, taxonId, author, license, licenseUrl, sourceUrl, downloadUrl, sha256, modifications;
    QStringList approvedRanks;
    QString role = "identification";
    QString supportingFor;
    bool valid = false;
    bool sourceVerified = false;
    QString error;
    QJsonObject attribution;
    int width = 0, height = 0;
};
