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
        UiButton { objectName: "returnDomainsButton"; text: "Voltar aos domínios"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate("categories") }
        Flow {
            Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 4
            Repeater {
                model: view.backend.breadcrumbs
                delegate: TextAction {
                    required property var modelData
                    required property int index
                    objectName: "breadcrumbButton" + index
                    text: modelData.title + (index < view.backend.breadcrumbs.length - 1 ? "  ›" : "")
                    onClicked: view.backend.exploreTaxon(modelData.id)
                }
            }
        }
        Label { text: view.backend.browseTaxon.title || ""; color: "#F0F6F0"; font.pixelSize: 30; font.weight: Font.DemiBold }
        Label { text: (view.backend.browseTaxon.rankLabel || "") + " · " + view.backend.browseTaxon.count + " espécies neste grupo"; color: "#91B0B5"; font.pixelSize: 14 }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 4
            Label { text: view.backend.browseTaxon.introduction.text || view.backend.browseTaxon.description || "Introdução científica ainda não disponível para este grupo."; color: "#9DB8BC"; font.pixelSize: 13; wrapMode: Text.WordWrap; lineHeight: 1.35; Layout.fillWidth: true }
            Label { text: (view.backend.browseTaxon.rankNote || "") + " " + (view.backend.browseTaxon.placementNotice || ""); color: "#D9BA7D"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            TextAction { visible: !!view.backend.browseTaxon.sourceUrl; text: "Fonte do enquadramento ↗"; onClicked: Qt.openUrlExternally(view.backend.browseTaxon.sourceUrl) }
            TextAction { visible: !!view.backend.browseTaxon.introduction.source_url; text: "Fonte: " + (view.backend.browseTaxon.introduction.source_title || "Referência científica"); onClicked: Qt.openUrlExternally(view.backend.browseTaxon.introduction.source_url) }
        }
        PlayModeSelector { objectName: "playModeSelector"; app: view.app; backend: view.backend }
        Label { visible: view.backend.gameFinished && view.backend.feedbackText.indexOf("alternativas aprovadas") >= 0; text: view.backend.feedbackText; color: "#D9B879"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                    TaxonComboBox { id: level; objectName: "identificationLevel"; Layout.fillWidth: true; model: view.backend.identificationLevels; textRole: "title"; valueRole: "id"; enabled: count > 0; onModelChanged: currentIndex = 0 }
                    UiButton { objectName: "startTaxonButton"; text: "Começar"; iconName: "play"; enabled: level.count > 0 && !view.backend.resultPending; onClicked: view.app.beginTaxonGame(view.backend.browseTaxon.id, level.currentValue) }
                }
                Label { text: view.backend.identificationLevels.length && level.currentIndex >= 0 ? view.backend.identificationLevels[level.currentIndex].count + " registros elegíveis neste nível; consulte os avisos das fotografias." : "Sem quiz neste nível: são necessárias quatro alternativas distintas e revisões adequadas. Grupos vazios continuam disponíveis para exploração."; color: "#85A894"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
        }
        Label { visible: view.backend.browseTaxon.supportingCount > 0; text: "Há imagens de apoio neste grupo. A identificação usa pistas; essas fotos podem representar colônias, sintomas ou organismos relacionados, sem comprovar a espécie."; color: "#91ABB4"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: view.backend.browseChildren.length ? "Explore os próximos grupos" : "Você chegou ao último nível disponível."; color: "#BCD1C7"; font.pixelSize: 18; Layout.topMargin: 8 }
        GridLayout {
            objectName: "taxonChildrenGrid"
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
