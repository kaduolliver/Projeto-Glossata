pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Dialog {
    id: panel
    required property var backend
    objectName: "hintPanel"
    property int selectedCategory: -1
    readonly property var labels: [
        {title: "Taxonômica", description: "Contexto da classificação.", icon: "book"},
        {title: "Etimológica", description: "Origem do nome.", icon: "spark"},
        {title: "Morfológica", description: "Características físicas.", icon: "shape"}
    ]
    readonly property var selectedHint: selectedCategory >= 0 ? backend.hints[selectedCategory] : ({})
    parent: Overlay.overlay
    modal: true; focus: true; anchors.centerIn: parent
    width: Math.min(540,parent ? parent.width-32 : 540)
    height: Math.min(740,Overlay.overlay ? Overlay.overlay.height-40 : 740)
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    header: Label { text: "ESCOLHA SUA DICA"; font.pixelSize: 20; font.weight: Font.DemiBold; color: panel.palette.text; padding: 20 }
    background: Rectangle { radius: 18; color: panel.palette.window; border.color: panel.palette.mid }
    contentItem: ScrollView {
        id: scroll
        clip: true; contentWidth: availableWidth
        ColumnLayout {
            width: scroll.availableWidth; spacing: 12
            Label { text: "Ao combinar dicas, vale a menor pontuação."; color: "#96AFB3"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Repeater {
                model: panel.labels
                delegate: Button {
                    id: choice
                    required property var modelData
                    required property int index
                    readonly property var hint: panel.backend.hints[index] || ({})
                    objectName: "hintCategory" + index
                    enabled: !!hint.available; hoverEnabled: true; padding: 14
                    Layout.fillWidth: true; implicitHeight: Math.max(112, contentItem.implicitHeight + topPadding + bottomPadding); opacity: enabled ? 1 : 0.55
                    Accessible.name: modelData.title + ". Assistência " + hint.assistance + ". " + (hint.available ? "Revelar esta dica mantém o acerto em até " + hint.pointsText + " ponto." : hint.reason)
                    background: Rectangle {
                        radius: 12
                        color: panel.selectedCategory===choice.index ? "#134C36" : (choice.hovered ? "#1B3440" : "#132832")
                        border.color: choice.activeFocus ? "#BCF6D9" : (panel.selectedCategory===choice.index ? "#35D480" : "#2B424D")
                        border.width: choice.activeFocus ? 2 : 1
                    }
                    contentItem: RowLayout {
                        spacing: 12
                        UiIcon { name: choice.modelData.icon; ink: "#79CE9E" }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Label { text: choice.modelData.title + (choice.hint.revealed ? " · revelada" : ""); color: "#E6F1E9"; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Label { text: choice.hint.available ? choice.modelData.description : choice.hint.reason; color: "#96AFB3"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { text: "Assistência " + choice.hint.assistance + (choice.hint.provisional ? " · provisória, revisão pendente" : ""); color: "#DDD3A4"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { objectName: "hintCeiling" + choice.index; visible: !!choice.hint.available; text: "Revelar esta dica mantém o acerto em até " + choice.hint.pointsText + " ponto."; color: "#71DCA0"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        }
                    }
                    onClicked: { if (panel.backend.revealHintCategory(hint.id)) panel.selectedCategory=index }
                }
            }
            Label { objectName: "selectedHintText"; visible: !!panel.selectedHint.revealed; text: panel.selectedHint.text || ""; color: "#DDD3A4"; font.pixelSize: 14; wrapMode: Text.WordWrap; textFormat: Text.PlainText; Layout.fillWidth: true }
            TextAction { visible: !!panel.selectedHint.sourceUrl; text: "Consultar fonte ↗"; onClicked: Qt.openUrlExternally(panel.selectedHint.sourceUrl) }
        }
    }
    footer: ColumnLayout {
        spacing: 8
        Label { text: "Pontuação possível: " + panel.backend.possibleScoreText; color: "#71DCA0"; font.pixelSize: 13; Layout.leftMargin: 20 }
        UiButton { objectName: "closeHintPanelButton"; text: "Voltar à pergunta"; secondary: true; Layout.fillWidth: true; Layout.margins: 20; onClicked: panel.close() }
    }
}
