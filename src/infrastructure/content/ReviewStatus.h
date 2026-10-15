#pragma once
#include <QDateTime>
#include <QJsonObject>

// Offline expiry still applies when an installed bundle ages without updates.
inline bool usableReview(const QJsonObject &review)
{
    const QString state = review.value("state").toString();
    if (!review.value("usable").toBool()
        || (state != "legacy" && state != "approved_editorial" && state != "approved_expert")) return false;
    const auto due = QDateTime::fromString(review.value("due_at").toString(), Qt::ISODateWithMs);
    if (state == "legacy") return review.value("content_sha256").toString().size() == 64;
    const auto reviewed = QDateTime::fromString(review.value("reviewed_at").toString(), Qt::ISODateWithMs);
    return due.isValid() && reviewed.isValid() && !review.value("reviewer").toString().isEmpty()
        && reviewed <= QDateTime::currentDateTimeUtc() && due > QDateTime::currentDateTimeUtc();
}
