pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
Item {
    id: view
    required property var app
    required property var backend
    property bool browsingOnly: false
    readonly property int columns: width >= 850 ? 3 : 2
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 20
        Label { text: view.browsingOnly ? "Explore a biodiversidade" : "Escolha um grupo para jogar"; color: "#EDF6EF"; font.pixelSize: 29; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: view.browsingOnly ? "Navegue pelos domínios, linhagens e fontes científicas, sem iniciar uma partida." : "Configure o modo Livre ou comece uma partida competitiva."; color: "#91ACB4"; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        PlayModeSelector { objectName: "playModeSelector"; visible: !view.browsingOnly; app: view.app; backend: view.backend }
        RowLayout {
            Layout.fillWidth: true; spacing: 10
            Label { text: "ACESSO RÁPIDO"; color: "#74979D"; font.pixelSize: 10; font.letterSpacing: 1.2 }
            Repeater {
                model: view.backend.quickAccessGroups
                delegate: TextAction {
                    required property var modelData
                    required property int index
                    objectName: "quickAccessButton" + index
                    text: modelData.title
                    Accessible.description: modelData.pathText + ". " + modelData.count + " organismos; explorar este grupo."
                    ToolTip.visible: hovered || visualFocus
                    ToolTip.delay: 600
                    ToolTip.text: modelData.pathText
                    onClicked: view.app.openTaxon(modelData.id)
                }
            }
            Item { Layout.fillWidth: true }
        }
        ExploreCard { objectName: "categoryButton0"; visible: !view.browsingOnly; Layout.fillWidth: true; entry: view.backend.categories[0]; wide: true; onClicked: view.app.beginTaxonGame("", entry.defaultRank) }
        Label { text: "OS TRÊS DOMÍNIOS · PORTAS DE EXPLORAÇÃO"; color: "#74979D"; font.pixelSize: 10; font.letterSpacing: 1.6; Layout.topMargin: 6; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Flow {
            id: groups
            Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 16
            Repeater {
                model: view.backend.categories.slice(1)
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: "domainButton" + index
                    entry: modelData
                    width: Math.floor(view.columns === 2 && index === 2 ? groups.width : (groups.width - (view.columns - 1) * 16) / view.columns)
                    onClicked: view.app.openTaxon(modelData.id)
                }
            }
        }
        Label { text: "Três domínios são uma organização didática. Hipóteses de dois domínios situam os eucariotos dentro de Archaea. As relações profundas e os ranks variam conforme a fonte; a navegação não é uma árvore evolutiva completa."; color: "#91ADB2"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
