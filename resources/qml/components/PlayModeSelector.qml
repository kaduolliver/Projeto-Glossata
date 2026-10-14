pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: selector
    required property var app
    required property var backend
    Layout.fillWidth: true
    spacing: 10
    RowLayout {
        Layout.fillWidth: true
        Label { text: "Livre · aprenda no seu ritmo"; color: "#B1C8CC"; font.pixelSize: 13; Layout.fillWidth: true }
        UiButton { objectName: "startCompetitiveButton"; text: "Competitivo · " + selector.backend.competitiveQuestionCount + " questões"; iconName: "trophy"; compact: true; enabled: !selector.backend.resultPending; onClicked: selector.app.beginCompetitiveGame() }
    }
    RowLayout {
        Layout.fillWidth: true
        Label { text: "Alternativas no modo Livre"; color: "#91ACB4"; font.pixelSize: 12 }
        TaxonComboBox {
            objectName: "difficultySelector"; Layout.preferredWidth: 230
            model: [{id: "broad", title: "Mais amplas"}, {id: "balanced", title: "Equilibradas"}, {id: "close", title: "Mais próximas"}]
            textRole: "title"; valueRole: "id"
            currentIndex: selector.backend.difficulty === "broad" ? 0 : selector.backend.difficulty === "close" ? 2 : 1
            onActivated: selector.backend.difficulty = currentValue
        }
        Item { Layout.fillWidth: true }
    }
    Label { text: "Competitivo: gênero, alternativas equilibradas e fotos de identificação. Mesmas regras e conjunto de conteúdo para todos."; color: "#718F96"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
}
