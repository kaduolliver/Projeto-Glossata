#pragma once
#include <QString>
#include <array>
#include <QtGlobal>

enum class HintType { None=0, Taxonomic=1, Etymological=2, Morphological=3 };
constexpr int FullScoreUnits=6;
constexpr int hintBit(HintType type) { return type<HintType::Taxonomic || type>HintType::Morphological ? 0 : 1 << (static_cast<int>(type)-1); }
constexpr int maximumScoreUnits(int revealed) {
    return revealed & hintBit(HintType::Morphological) ? 2
         : revealed & hintBit(HintType::Etymological) ? 3 : FullScoreUnits;
}
inline QString hintKey(HintType type) {
    switch(type) { case HintType::Taxonomic:return "taxonomic"; case HintType::Etymological:return "etymological";
        case HintType::Morphological:return "morphological"; default:return {}; }
}
inline HintType hintType(const QString &key) {
    for (auto type : {HintType::Taxonomic,HintType::Etymological,HintType::Morphological}) if(hintKey(type)==key)return type;
    return HintType::None;
}
// Rounding happens only here, at presentation. All accumulation stays integral.
inline QString formatScore(qint64 units) {
    const qint64 hundredths=(units%6*100+3)/6;
    return QString::number(units/6)+","+QString::number(hundredths).rightJustified(2,'0');
}
struct Hint {
    HintType type=HintType::None;
    QString text,sourceUrl,sourceTitle,unavailableReason;
    bool available() const { return !text.isEmpty() && !sourceUrl.isEmpty(); }
};
using QuestionHints=std::array<Hint,3>;
struct AnswerRecord {
    int questionNumber=0,speciesId=0;
    QString scientificName,photoId,rank,selectedAnswer,correctAnswer;
    bool correct=false;
    int revealedHints=0,maximumUnits=FullScoreUnits,awardedUnits=0;
};
