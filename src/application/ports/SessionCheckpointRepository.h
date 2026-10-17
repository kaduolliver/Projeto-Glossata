#pragma once
#include "domain/SessionCheckpoint.h"
#include <QVector>

class SessionCheckpointRepository {
public:
    virtual ~SessionCheckpointRepository() = default;
    // Compare-and-swap: stale processes cannot overwrite more recent progress.
    virtual bool writeCheckpoint(const QJsonObject &payload, qint64 expectedRevision, qint64 &revision) = 0;
    virtual QVector<SessionCheckpoint> recoverableSessions() const = 0;
    virtual SessionCheckpoint checkpoint(const QString &id) const = 0;
    virtual bool abandonCheckpoint(const QString &id, qint64 expectedRevision) = 0;
};
