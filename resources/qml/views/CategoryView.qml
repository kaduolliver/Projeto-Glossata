pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
Item {
    id: view
    required property var app
    required property var backend
    readonly property int columns: width >= 850 ? 3 : 2
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 20
        Label { text: "Escolha sua próxima descoberta"; color: "#EDF6EF"; font.pixelSize: 29; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: "Explore os domínios da vida e suas linhagens evolutivas no seu ritmo."; color: "#91ACB4"; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        PlayModeSelector { app: view.app; backend: view.backend }
        ExploreCard { objectName: "categoryButton0"; Layout.fillWidth: true; entry: view.backend.categories[0]; wide: true; onClicked: view.app.beginTaxonGame("", entry.defaultRank) }
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
