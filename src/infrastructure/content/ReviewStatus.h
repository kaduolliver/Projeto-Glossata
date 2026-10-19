#pragma once
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

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

// An individually reviewed assertion does not inherit the approval or expiry of
// its aggregate. Legacy assertions still need the original aggregate safeguard.
// This authorizes only the cited note, never traits, photography or assistance.
inline bool usableScientificHintNote(const QJsonObject &note,const QJsonObject &topic,const QString &kind)
{
    const auto review=note.value("governance").toObject();
    if ((kind!="etymology" && kind!="morphology") || topic.value("state")=="rejected"
        || !usableReview(review) || note.value("text").toString().isEmpty()
        || !note.value("source_url").toString().startsWith("https://")) return false;
    if (review.value("state")=="legacy") return usableReview(topic);
    const auto scope=review.value("claim_scope").toString();
    if (review.value("content_sha256").toString().size()!=64
        || (kind=="etymology" ? scope!="nomenclature" : scope!="species" && scope!="photographic_observation")) return false;
    for (const auto &value:review.value("evidence").toArray()) {
        const auto evidence=value.toObject();
        if (evidence.value("url")==note.value("source_url") && !evidence.value("locator").toString().isEmpty()
            && !evidence.value("supports").toString().isEmpty()) return true;
    }
    return false;
}
