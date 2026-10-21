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
    readonly property int columns: width >= 850 ? 3 : 2
    readonly property bool searching: backend.exploreQuery.trim().length > 0 || !!backend.exploreDomain || !!backend.exploreRank
    readonly property var results: browsingOnly ? backend.exploreResults : ({})
    readonly property var domains: [{id: "", title: "Todos os domínios"}].concat(backend.categories.slice(1))
    function commitSearch() { searchDelay.stop(); backend.exploreQuery = searchField.text }
    Timer { id: searchDelay; interval: 220; onTriggered: view.commitSearch() }
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 20
        GridLayout {
            visible: view.browsingOnly
            Layout.fillWidth: true; columns: view.width >= 1100 ? 2 : 1; columnSpacing: 32; rowSpacing: 16
            ColumnLayout {
                Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                Label { text: "ENCICLOPÉDIA DA VIDA"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.6 }
                Label { text: "Explore a biodiversidade"; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                Label { text: "Fotografias, nomes e caminhos para conhecer o catálogo científico."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
            TextField {
                id: searchField
                objectName: "exploreSearchField"
                Layout.fillWidth: true; Layout.minimumWidth: 280
                implicitHeight: 50; leftPadding: 42; rightPadding: 14
                text: view.backend.exploreQuery
                placeholderText: "Nome científico, nome comum ou grupo…"
                color: Style.Theme.text; placeholderTextColor: Style.Theme.textMuted
                font.pixelSize: Style.Theme.labelSize; selectByMouse: true
                Accessible.name: "Pesquisar no catálogo local"
                Accessible.description: "Busca por nome científico, nome comum disponível ou grupo taxonômico. Combine com os filtros de domínio e nível."
                onTextEdited: searchDelay.restart()
                onAccepted: view.commitSearch()
                onEditingFinished: view.commitSearch()
                background: Rectangle { color: Style.Theme.surface; radius: Style.Theme.controlRadius; border.color: searchField.activeFocus ? Style.Theme.focus : Style.Theme.border; border.width: searchField.activeFocus ? 2 : 1 }
                UiIcon { anchors.left: parent.left; anchors.leftMargin: 14; anchors.verticalCenter: parent.verticalCenter; name: "search"; ink: Style.Theme.green; implicitWidth: 18; implicitHeight: 18 }
            }
        }
        RowLayout {
            visible: view.browsingOnly; Layout.fillWidth: true; spacing: Style.Theme.spaceSm
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Label { text: "Domínio"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                TaxonComboBox { objectName: "exploreDomainFilter"; Layout.fillWidth: true; model: view.domains; textRole: "title"; valueRole: "id"; currentIndex: Math.max(0, view.domains.map(entry => entry.id).indexOf(view.backend.exploreDomain)); Accessible.name: "Filtrar por domínio"; onActivated: view.backend.exploreDomain = currentValue }
            }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Label { text: "Nível taxonômico"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                TaxonComboBox { objectName: "exploreRankFilter"; Layout.fillWidth: true; model: view.backend.exploreRanks; textRole: "title"; valueRole: "id"; currentIndex: Math.max(0, view.backend.exploreRanks.map(entry => entry.id).indexOf(view.backend.exploreRank)); Accessible.name: "Filtrar por nível taxonômico"; onActivated: view.backend.exploreRank = currentValue }
            }
            TextAction { objectName: "clearExploreSearch"; text: "Limpar"; enabled: view.searching; Layout.alignment: Qt.AlignBottom; onClicked: { view.backend.exploreQuery = ""; view.backend.exploreDomain = ""; view.backend.exploreRank = ""; searchField.forceActiveFocus(Qt.TabFocusReason) } }
        }
        Label { visible: !view.browsingOnly; text: "Escolha um grupo para jogar"; color: "#EDF6EF"; font.pixelSize: 29; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { visible: !view.browsingOnly; text: "Configure o modo Livre ou comece uma partida competitiva."; color: "#91ACB4"; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        PlayModeSelector { objectName: "playModeSelector"; visible: !view.browsingOnly; app: view.app; backend: view.backend }
        Flow {
            visible: !view.browsingOnly || !view.searching
            Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 10
            Label { text: "ACESSO RÁPIDO"; color: "#74979D"; font.pixelSize: 10; font.letterSpacing: 1.2; height: 36; verticalAlignment: Text.AlignVCenter }
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
        }
        ExploreCard { objectName: "categoryButton0"; visible: !view.browsingOnly; Layout.fillWidth: true; entry: view.backend.categories[0]; wide: true; onClicked: view.app.beginTaxonGame("", entry.defaultRank) }
        Label { visible: !view.browsingOnly || !view.searching; text: "OS TRÊS DOMÍNIOS · PORTAS DE EXPLORAÇÃO"; color: view.browsingOnly ? Style.Theme.textMuted : "#74979D"; font.pixelSize: 10; font.letterSpacing: 1.6; Layout.topMargin: 6; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        GridLayout {
            id: groups
            visible: !view.browsingOnly || !view.searching
            Layout.fillWidth: true; columns: view.browsingOnly ? 3 : view.columns; columnSpacing: 16; rowSpacing: 16
            Repeater {
                model: view.backend.categories.slice(1)
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: "domainButton" + index
                    entry: modelData
                    encyclopedic: view.browsingOnly
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: 1
                    Layout.columnSpan: !view.browsingOnly && view.columns === 2 && index === 2 ? 2 : 1
                    onClicked: view.app.openTaxon(modelData.id)
                }
            }
        }
        RowLayout {
            visible: view.browsingOnly && !view.searching; Layout.fillWidth: true
            Label { text: "Organismos em destaque"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Layout.fillWidth: true; wrapMode: Text.WordWrap; Accessible.role: Accessible.Heading }
            TextAction { objectName: "allExploreOrganisms"; text: "Ver organismos →"; onClicked: view.backend.exploreRank = "species" }
        }
        GridLayout {
            visible: view.browsingOnly && !view.searching
            Layout.fillWidth: true; columns: view.width >= 1100 ? 4 : 2; columnSpacing: 16; rowSpacing: 16
            Repeater {
                model: view.browsingOnly ? view.backend.featuredOrganisms : []
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: "featuredOrganism" + index
                    entry: modelData; encyclopedic: true
                    Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.alignment: Qt.AlignTop
                    onClicked: view.app.openTaxon(modelData.id)
                }
            }
        }
        Label { visible: view.browsingOnly && !view.searching; text: "Destaques com fotografia de identificação e resumo elegíveis no catálogo. As contagens representam organismos cadastrados, não a diversidade total dos grupos."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        ColumnLayout {
            visible: view.browsingOnly && view.searching
            Layout.fillWidth: true; spacing: 16
            Label { objectName: "exploreResultSummary"; text: (view.results.groupCount || 0) + " grupos taxonômicos · " + (view.results.organismCount || 0) + " organismos cadastrados"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
            Label { objectName: "exploreEmptyState"; visible: view.results.total === 0; text: "Nenhum resultado para esta combinação. Tente outro nome, domínio ou nível taxonômico. A busca considera somente os dados disponíveis no catálogo local."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { visible: view.results.groupCount > 0; text: "Grupos taxonômicos"; color: Style.Theme.green; font.pixelSize: Style.Theme.labelSize; font.weight: Font.DemiBold }
            GridLayout {
                Layout.fillWidth: true; columns: view.columns; columnSpacing: 16; rowSpacing: 16
                Repeater {
                    model: view.searching ? view.results.groups || [] : []
                    delegate: ExploreCard {
                        required property var modelData
                        required property int index
                        objectName: "exploreGroupResult" + index
                        entry: modelData; encyclopedic: true
                        Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.alignment: Qt.AlignTop
                        onClicked: view.app.openTaxon(modelData.id)
                    }
                }
            }
            Label { visible: view.results.organismCount > 0; text: "Organismos cadastrados"; color: Style.Theme.green; font.pixelSize: Style.Theme.labelSize; font.weight: Font.DemiBold }
            GridLayout {
                Layout.fillWidth: true; columns: view.columns; columnSpacing: 16; rowSpacing: 16
                Repeater {
                    model: view.searching ? view.results.organisms || [] : []
                    delegate: ExploreCard {
                        required property var modelData
                        required property int index
                        objectName: "exploreOrganismResult" + index
                        entry: modelData; encyclopedic: true
                        Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.alignment: Qt.AlignTop
                        onClicked: view.app.openTaxon(modelData.id)
                    }
                }
            }
        }
        Label { text: "Três domínios são uma organização didática. Hipóteses de dois domínios situam os eucariotos dentro de Archaea. As relações profundas e os ranks variam conforme a fonte; a navegação não é uma árvore evolutiva completa."; color: view.browsingOnly ? Style.Theme.textMuted : "#91ADB2"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
