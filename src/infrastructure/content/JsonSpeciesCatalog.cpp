#include "infrastructure/content/JsonSpeciesCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

JsonSpeciesCatalog::JsonSpeciesCatalog(const QString &sourcePath)
    : sourcePath_(sourcePath)
{
}

QVector<Species> JsonSpeciesCatalog::loadSpecies() const
{
    QFile file(sourcePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível abrir o catálogo:" << sourcePath_ << file.errorString();
        return {};
    }

    const QByteArray data = file.readAll();
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Catálogo inválido:" << sourcePath_ << error.errorString();
        return {};
    }
    if (!document.isArray()) {
        qWarning() << "O catálogo deve conter um array JSON:" << sourcePath_;
        return {};
    }

    QVector<Species> species;
    const QJsonArray array = document.array();
    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        Species item;
        item.id = object.value(QStringLiteral("id")).toInt();
        item.name = object.value(QStringLiteral("name")).toString().trimmed();
        item.fact = object.value(QStringLiteral("fact")).toString().trimmed();
        item.hint = object.value(QStringLiteral("hint")).toString().trimmed();
        item.family = object.value(QStringLiteral("family")).toString();
        item.region = object.value(QStringLiteral("region")).toString();
        item.image = object.value(QStringLiteral("image")).toString();
        if (!item.name.isEmpty() && !item.fact.isEmpty() && !item.hint.isEmpty()) {
            species.push_back(item);
        }
    }

    return species;
}
