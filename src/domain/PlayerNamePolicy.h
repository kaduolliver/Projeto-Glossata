#pragma once

#include <QString>

// Display spelling and canonical identity matching are deliberately separate.
// This policy uses Unicode SIMPLE case folding (Qt), not full case folding,
// transliteration, compatibility normalization or removal of diacritics.
namespace PlayerNamePolicy {
inline QString version() { return QStringLiteral("nfc-simple-casefold-v1"); }
inline int unicodeVersion() { return int(QChar::currentUnicodeVersion()); }
inline QString displayName(const QString &input) { return input.trimmed(); }
inline bool validDisplayName(const QString &display) { return !display.isEmpty() && display.size()<=32; }
inline QString key(const QString &input) {
    return displayName(input).normalized(QString::NormalizationForm_C)
        .toCaseFolded().normalized(QString::NormalizationForm_C);
}
}
