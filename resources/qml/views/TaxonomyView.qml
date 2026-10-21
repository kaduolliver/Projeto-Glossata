pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style
Item {
    id: view
    required property var app
    required property var backend
    property bool browsingOnly: false
    property bool photoDetailsOpen: false
    readonly property var photo: backend.browseTaxon.photo || ({})
    readonly property var introduction: browsingOnly && (backend.browseTaxon.summary || ({})).text ? backend.browseTaxon.summary : backend.browseTaxon.introduction
    function visit(id) {
        backend.exploreTaxon(id)
        if (browsingOnly) returnButton.forceActiveFocus(Qt.OtherFocusReason)
    }
    Connections { target: view.backend; function onExplorationChanged() { view.photoDetailsOpen = false } }
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 18
        UiButton { id: returnButton; objectName: "returnDomainsButton"; text: view.browsingOnly ? "Voltar à pesquisa e aos domínios" : "Voltar aos domínios"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate(view.browsingOnly ? "explore" : "categories") }
        Flow {
            Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 4
            Repeater {
                model: view.backend.breadcrumbs
                delegate: TextAction {
                    required property var modelData
                    required property int index
                    objectName: "breadcrumbButton" + index
                    text: modelData.title + (index < view.backend.breadcrumbs.length - 1 ? "  ›" : "")
                    width: Math.min(implicitWidth, view.width)
                    Accessible.description: modelData.rankLabel + ". " + (index === view.backend.breadcrumbs.length - 1 ? "Nível atual." : "Navegar para este ancestral.")
                    onClicked: view.visit(modelData.id)
                }
            }
        }
        Rectangle {
            visible: view.browsingOnly
            Layout.fillWidth: true; implicitHeight: overview.implicitHeight + 40
            radius: Style.Theme.cardRadius; color: Style.Theme.surface; border.color: Style.Theme.border
            RowLayout {
                id: overview
                anchors.fill: parent; anchors.margins: 20; spacing: 24
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 10
                    Label { text: view.backend.browseTaxon.kind === "organism" ? "ORGANISMO NO CATÁLOGO" : "GRUPO TAXONÔMICO"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { objectName: "exploreTaxonTitle"; text: view.backend.browseTaxon.title || ""; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; font.italic: view.backend.browseTaxon.rank === "species" || view.backend.browseTaxon.rank === "genus"; wrapMode: Text.Wrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                    Label { visible: !!view.backend.browseTaxon.commonName; text: view.backend.browseTaxon.commonName || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { text: (view.backend.browseTaxon.rankLabel || "") + " · " + view.backend.browseTaxon.count + (view.backend.browseTaxon.count === 1 ? " organismo cadastrado" : " organismos cadastrados"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.labelSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { text: "Contagem restrita ao catálogo local. Não representa a diversidade total deste táxon."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                Rectangle {
                    Layout.preferredWidth: view.width * 0.38; Layout.preferredHeight: 206
                    color: Style.Theme.surfaceRaised
                    Image { id: taxonPhoto; objectName: "exploreTaxonPhoto"; anchors.fill: parent; anchors.margins: 6; source: view.photo.image || ""; fillMode: Image.PreserveAspectFit; asynchronous: true; smooth: true; mipmap: true }
                    Label { visible: taxonPhoto.status !== Image.Ready; anchors.centerIn: parent; width: parent.width - 24; text: taxonPhoto.status === Image.Loading ? "Carregando fotografia…" : "Fotografia ainda indisponível"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter }
                }
            }
        }
        ColumnLayout {
            visible: view.browsingOnly; Layout.fillWidth: true; spacing: 6
            Label { text: view.photo.image ? (view.photo.kind === "micrograph" ? "Micrografia: " : "Fotografia: ") + (view.photo.subject || "") + " · " + (view.photo.author || "Autoria não informada") + " · " + (view.photo.license || "Licença não informada") : view.photo.pendingReason || "Fotografia adequada ainda não disponível."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!view.photo.image; text: view.photo.identificationNote || "Representante do grupo; identificação indicada pela fonte."; textFormat: Text.PlainText; color: Style.Theme.gold; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            TextAction { objectName: "explorePhotoCredits"; visible: !!view.photo.image; text: view.photoDetailsOpen ? "Ocultar foto e créditos ↑" : "Foto, origem e créditos ↓"; Accessible.description: "Expandir os créditos e as limitações desta fotografia na página."; onClicked: view.photoDetailsOpen = !view.photoDetailsOpen }
            ColumnLayout {
                objectName: "explorePhotoDetails"
                visible: view.photoDetailsOpen; Layout.fillWidth: true; spacing: 6
                Label { text: (view.photo.title || "Título não informado") + "\n" + (view.photo.modifications || "Alterações não documentadas.") + "\nOs registros de origem e as permissões mantêm os limites documentados em Créditos e transparência."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Flow {
                    Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 8
                    TextAction { visible: !!view.photo.photoUrl; text: "Fotografia original ↗"; onClicked: Qt.openUrlExternally(view.photo.photoUrl) }
                    TextAction { visible: !!view.photo.licenseUrl; text: "Termos da licença ↗"; onClicked: Qt.openUrlExternally(view.photo.licenseUrl) }
                    TextAction { text: "Créditos e transparência"; onClicked: view.app.navigate("credits") }
                }
            }
        }
        Label { visible: !view.browsingOnly; text: view.backend.browseTaxon.title || ""; color: "#F0F6F0"; font.pixelSize: 30; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { visible: !view.browsingOnly; text: (view.backend.browseTaxon.rankLabel || "") + " · " + view.backend.browseTaxon.count + " espécies neste grupo"; color: "#91B0B5"; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 4
            Label { objectName: "exploreIntroductionText"; text: view.introduction.text || view.backend.browseTaxon.description || "Introdução científica ainda não disponível para este grupo."; textFormat: Text.PlainText; color: "#9DB8BC"; font.pixelSize: 13; wrapMode: Text.WordWrap; lineHeight: 1.35; Layout.fillWidth: true }
            Label { visible: view.browsingOnly; text: view.backend.browseTaxon.introductionReviewNotice || ""; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: (view.backend.browseTaxon.rankNote || "") + " " + (view.backend.browseTaxon.placementNotice || ""); color: "#D9BA7D"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            TextAction { visible: !!view.backend.browseTaxon.sourceUrl; text: "Fonte do enquadramento ↗"; onClicked: Qt.openUrlExternally(view.backend.browseTaxon.sourceUrl) }
            TextAction { visible: !!view.introduction.source_url; Layout.maximumWidth: view.width; text: "Fonte: " + (view.introduction.source_title || "Referência científica"); Accessible.description: view.introduction.source_url || ""; onClicked: Qt.openUrlExternally(view.introduction.source_url) }
        }
        PlayModeSelector { objectName: "playModeSelector"; visible: !view.browsingOnly; app: view.app; backend: view.backend }
        Label { visible: !view.browsingOnly && view.backend.gameFinished && view.backend.feedbackText.indexOf("alternativas aprovadas") >= 0; text: view.backend.feedbackText; color: "#D9B879"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Rectangle {
            visible: !view.browsingOnly
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
        Label { text: view.backend.browseChildren.length ? "Explore os próximos grupos" : view.browsingOnly && view.backend.browseTaxon.count === 0 ? "Este grupo ainda não tem organismos no catálogo." : "Você chegou ao último nível disponível."; color: view.browsingOnly ? Style.Theme.text : "#BCD1C7"; font.pixelSize: 18; Layout.topMargin: 8; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                    encyclopedic: view.browsingOnly
                    Layout.fillWidth: true; Layout.preferredWidth: 1
                    Layout.alignment: Qt.AlignTop
                    onClicked: view.visit(modelData.id)
                }
            }
        }
    }
}
