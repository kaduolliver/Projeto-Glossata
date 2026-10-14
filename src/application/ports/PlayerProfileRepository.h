#pragma once

#include <QString>
#include <QStringList>

class PlayerProfileRepository {
public:
    virtual ~PlayerProfileRepository() = default;
    virtual QStringList loadPlayerProfiles() const = 0;
    virtual bool addPlayerProfile(const QString &name) = 0;
    virtual bool renamePlayerProfile(const QString &currentName, const QString &newName) = 0;
    virtual bool deletePlayerProfile(const QString &name) = 0;
};