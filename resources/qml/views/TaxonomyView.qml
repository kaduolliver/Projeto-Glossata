pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 18
        UiButton { objectName: "returnKingdomsButton"; text: "Voltar aos reinos"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate("categories") }
        Flow {
            Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 4
            Repeater {
                model: view.backend.breadcrumbs
                delegate: Button {
                    required property var modelData
                    required property int index
                    objectName: "breadcrumbButton" + index
                    text: modelData.title + (index < view.backend.breadcrumbs.length - 1 ? "  ›" : "")
                    flat: true; onClicked: view.backend.exploreTaxon(modelData.id)
                }
            }
        }
        Label { text: view.backend.browseTaxon.title || ""; color: "#F0F6F0"; font.pixelSize: 30; font.weight: Font.DemiBold }
        Label { text: (view.backend.browseTaxon.rankLabel || "") + " · " + view.backend.browseTaxon.count + " espécies neste grupo"; color: "#91B0B5"; font.pixelSize: 14 }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: controls.implicitHeight + 36
            radius: 16; color: "#17372D"; border.color: "#365C47"
            ColumnLayout {
                id: controls
                anchors.fill: parent; anchors.margins: 18; spacing: 14
                Label { text: "Todas as espécies de " + (view.backend.browseTaxon.title || ""); color: "#E3EEE6"; font.pixelSize: 19; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "Identificar"; color: "#9BB6A9"; font.pixelSize: 12 }
                    ComboBox { id: level; objectName: "identificationLevel"; Layout.fillWidth: true; model: view.backend.identificationLevels; textRole: "title"; valueRole: "id"; enabled: count > 0; onModelChanged: currentIndex = 0 }
                    UiButton { objectName: "startTaxonButton"; text: "Começar"; iconName: "play"; enabled: level.count > 0 && !view.backend.resultPending; onClicked: view.app.beginTaxonGame(view.backend.browseTaxon.id, level.currentValue) }
                }
                Label { text: view.backend.identificationLevels.length ? "As alternativas sempre pertencem ao nível escolhido." : "Explore este grupo. Ainda faltam quatro alternativas distintas para iniciar um quiz."; color: "#85A894"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
        }
        Label { visible: view.backend.browseTaxon.kingdom === "Protista" || view.backend.browseTaxon.kingdom === "Monera"; text: view.backend.browseTaxon.kingdom === "Protista" ? "Protista reúne Protozoa e Chromista neste modelo didático. Os táxons e nomes da fonte permanecem na árvore." : "Monera reúne Bacteria e Archaea neste modelo didático. As perguntas usam pistas taxonômicas; as fotos não bastam para identificar uma espécie bacteriana."; color: "#91ABB4"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: view.backend.browseChildren.length ? "Explore os próximos grupos" : "Você chegou ao último nível disponível."; color: "#BCD1C7"; font.pixelSize: 18; Layout.topMargin: 8 }
        GridLayout {
            Layout.fillWidth: true; columns: view.width >= 850 ? 3 : 2; columnSpacing: 14; rowSpacing: 14
            Repeater {
                model: view.backend.browseChildren
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: "taxonButton" + index
                    entry: modelData
                    Layout.fillWidth: true; Layout.preferredWidth: 1
                    onClicked: view.backend.exploreTaxon(modelData.id)
                }
            }
        }
    }
}
