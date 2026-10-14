#pragma once
#include <QString>
#include <QStringList>
struct Photo {
    QString id, image, scientificName, taxonId, author, license, licenseUrl, sourceUrl, downloadUrl, sha256, modifications;
    QStringList approvedRanks;
    bool valid = false;
    bool sourceVerified = false;
    QString error;
    int width = 0, height = 0;
};
