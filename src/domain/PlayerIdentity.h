#pragma once

#include <QString>

// Passed by value at session start. A display name is never an identity key.
struct PlayerIdentity {
    QString id;
    QString displayName;
    bool isValid() const { return !id.isEmpty() && !displayName.trimmed().isEmpty(); }
};

struct PlayerProfile {
    QString id;
    QString displayName;
    bool deleted = false;
    bool legacy = false;
    PlayerIdentity identity() const { return {id, displayName}; }
    bool selectable() const { return !id.isEmpty() && !deleted && !legacy; }
};
