#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include "domain/PlayerIdentity.h"

class PlayerProfileRepository {
public:
    virtual ~PlayerProfileRepository() = default;
    virtual QVector<PlayerProfile> profiles() const = 0;
    virtual PlayerProfile player(const QString &id) const = 0;
    virtual PlayerProfile createPlayer(const QString &name) = 0;
    virtual bool renamePlayer(const QString &id, const QString &name) = 0;
    virtual bool deletePlayer(const QString &id) = 0;
};
