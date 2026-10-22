pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
GridLayout {
    id: selector
    required property var app
    required property var backend
    Layout.fillWidth: true
    columns: width >= 700 ? 2 : 1
    columnSpacing: Style.Theme.spaceMd; rowSpacing: Style.Theme.spaceMd
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true
        implicitHeight: freeContent.implicitHeight + 32
        radius: Style.Theme.cardRadius; color: Style.Theme.surface; border.color: Style.Theme.green
        ColumnLayout {
            id: freeContent; anchors.fill: parent; anchors.margins: 16; spacing: 8
            RowLayout {
                UiIcon { name: "play"; ink: Style.Theme.green; implicitWidth: 20; implicitHeight: 20 }
                Label { text: "Livre"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Layout.fillWidth: true }
                Label { text: "NO SEU RITMO"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize }
            }
            Label { text: "Escolha um grupo e o nível de identificação. Aprenda sem limite de questões."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Alternativas"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                TaxonComboBox {
                    objectName: "difficultySelector"; Layout.fillWidth: true; Layout.maximumWidth: 300
                    model: [{id: "broad", title: "Mais amplas"}, {id: "balanced", title: "Equilibradas"}, {id: "close", title: "Mais próximas"}]
                    textRole: "title"; valueRole: "id"; Accessible.name: "Dificuldade das alternativas no modo Livre"
                    currentIndex: selector.backend.difficulty === "broad" ? 0 : selector.backend.difficulty === "close" ? 2 : 1
                    onActivated: selector.backend.difficulty = currentValue
                }
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true
        implicitHeight: competitiveContent.implicitHeight + 32
        radius: Style.Theme.cardRadius; color: Style.Theme.surface; border.color: Style.Theme.gold
        ColumnLayout {
            id: competitiveContent; anchors.fill: parent; anchors.margins: 16; spacing: 8
            RowLayout {
                UiIcon { name: "trophy"; ink: Style.Theme.gold; implicitWidth: 20; implicitHeight: 20 }
                Label { text: "Competitivo"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Layout.fillWidth: true }
            }
            Label { text: "Conjunto geral do catálogo, identificação de gêneros e alternativas equilibradas. Somente fotos de identificação elegíveis; regras iguais para todos."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            UiButton { objectName: "startCompetitiveButton"; text: "Iniciar competitivo · " + selector.backend.competitiveQuestionCount + " questões"; iconName: "trophy"; compact: true; enabled: !selector.backend.resultPending; onClicked: selector.app.beginCompetitiveGame() }
        }
    }
}
