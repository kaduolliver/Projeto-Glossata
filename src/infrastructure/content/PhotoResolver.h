#pragma once
#include "domain/Species.h"
#include <QJsonObject>
class PhotoResolver {
public:
    static Photo fromJson(const QJsonObject &metadata, const QString &image);
    static QJsonObject toJson(const Photo &photo);
    static Photo validate(Photo photo, const QString &expectedName);
    static void attach(Species &species, Photo photo);
};
