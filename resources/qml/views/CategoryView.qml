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
    property string playTab: "kingdoms"
    readonly property var playEntries: !browsingOnly ? backend.playGroups[playTab] || [] : []
    ButtonGroup { id: playTabs; exclusive: true }
    readonly property int columns: width >= 850 ? 3 : 2
    readonly property int explorationColumns: Math.max(1, Math.floor((width + 16) / (280 + 16)))
    readonly property int domainColumns: Math.max(1, Math.min(backend.categories.length - 1, explorationColumns))
    readonly property var featuredEntries: browsingOnly && !searching ? backend.featuredOrganisms : []
    readonly property int resultBatchSize: 12
    readonly property string presentationKey: JSON.stringify([backend.exploreQuery, backend.exploreDomain, backend.exploreRank])
    property int groupLimit: browsingOnly && app.explorePresentation.key === presentationKey ? app.explorePresentation.groupLimit : resultBatchSize
    property int organismLimit: browsingOnly && app.explorePresentation.key === presentationKey ? app.explorePresentation.organismLimit : resultBatchSize
    property Flickable pageViewport: null
    property bool presentationReady: false
    property bool restoring: true
    property string returnFocus: ""
    property var pendingFocusRepeater: null
    property int pendingFocusIndex: -1
    readonly property bool searching: backend.exploreQuery.trim().length > 0 || !!backend.exploreDomain || !!backend.exploreRank
    readonly property bool searchPending: backend.exploreSearchPending
    readonly property var results: browsingOnly && searching && !searchPending ? backend.exploreResults : ({})
    readonly property var presentedGroups: (results.groups || []).slice(0, groupLimit)
    readonly property var presentedOrganisms: (results.organisms || []).slice(0, organismLimit)
    readonly property var domains: [{id: "", title: "Todos os domínios"}].concat(backend.categories.slice(1))
    onPresentationKeyChanged: {
        if (presentationReady) {
            newResultsFocus.stop(); view.pendingFocusRepeater = null
            view.groupLimit = view.resultBatchSize; view.organismLimit = view.resultBatchSize
            view.returnFocus = ""
            rememberPresentation()
        }
    }
    function rememberPresentation() {
        if (!browsingOnly || !presentationReady || restoring || app.page !== "explore") return
        app.explorePresentation = {key: presentationKey, groupLimit: groupLimit, organismLimit: organismLimit,
            scrollY: pageViewport ? pageViewport.contentY : 0, galleryX: featuredGallery.contentX, focusName: returnFocus}
    }
    function openEntry(id, name) { returnFocus = name; rememberPresentation(); app.openTaxon(id) }
    Component.onCompleted: {
        let ancestor = parent
        while (ancestor && !(ancestor instanceof Flickable)) ancestor = ancestor.parent
        pageViewport = ancestor
        presentationReady = true
        restorePresentation.start()
    }
    Connections {
        target: view.pageViewport
        function onContentYChanged() { view.rememberPresentation() }
    }
    Timer {
        id: restorePresentation
        interval: 0
        onTriggered: {
            const saved = view.app.explorePresentation
            if (view.browsingOnly && saved.key === view.presentationKey) {
                view.returnFocus = saved.focusName || ""
                // Delegates/layouts exist before restoring focus and scroll.
                let control = null
                if (view.returnFocus.startsWith("exploreGroupResult")) control = groupResults.itemAt(Number(view.returnFocus.slice(18)))
                else if (view.returnFocus.startsWith("exploreOrganismResult")) control = organismResults.itemAt(Number(view.returnFocus.slice(21)))
                else if (view.returnFocus.startsWith("featuredOrganism")) control = featuredResults.itemAt(Number(view.returnFocus.slice(16)))
                if (control) control.forceActiveFocus(Qt.TabFocusReason)
                featuredGallery.contentX = Math.max(0, Math.min(saved.galleryX || 0, featuredGallery.contentWidth - featuredGallery.width))
                if (view.pageViewport) view.pageViewport.contentY = Math.max(0, Math.min(saved.scrollY || 0, view.pageViewport.contentHeight - view.pageViewport.height))
            }
            view.restoring = false
            view.rememberPresentation()
        }
    }
    Timer {
        id: newResultsFocus
        interval: 0
        onTriggered: {
            const next = view.pendingFocusRepeater ? view.pendingFocusRepeater.itemAt(view.pendingFocusIndex) : null
            view.pendingFocusRepeater = null
            if (next) next.forceActiveFocus(Qt.TabFocusReason)
        }
    }
    function loadMore(section, keyboard) {
        const repeater = section === "groups" ? groupResults : organismResults
        const firstNew = repeater.count
        if (section === "groups") groupLimit = Math.min(results.groupCount, groupLimit + resultBatchSize)
        else organismLimit = Math.min(results.organismCount, organismLimit + resultBatchSize)
        rememberPresentation()
        if (keyboard) { pendingFocusRepeater = repeater; pendingFocusIndex = firstNew; newResultsFocus.restart() }
    }
    function countLabel(count, singular, plural) { return count + " " + (count === 1 ? singular : plural) }
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: view.browsingOnly ? 14 : 20
        GridLayout {
            visible: view.browsingOnly
            objectName: "exploreHeader"
            Layout.fillWidth: true; columns: view.width >= 930 ? 2 : 1; columnSpacing: 24; rowSpacing: 12
            ColumnLayout {
                Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                Label { text: "ENCICLOPÉDIA DA VIDA"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.6 }
                Label { objectName: "exploreHeading"; text: "Explore a biodiversidade"; color: Style.Theme.text; font.pixelSize: 26; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                Label { text: "Conheça a vida pelo catálogo científico."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.labelSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
            TextField {
                id: searchField
                objectName: "exploreSearchField"
                Layout.fillWidth: true; Layout.minimumWidth: 280
                Layout.preferredWidth: view.width >= 930 ? 380 : -1
                implicitHeight: 46; leftPadding: 42; rightPadding: clearQuery.visible ? 48 : 14
                text: view.backend.exploreQuery
                placeholderText: "Nome científico, nome comum ou grupo…"
                color: Style.Theme.text; placeholderTextColor: Style.Theme.textMuted
                font.pixelSize: Style.Theme.labelSize; selectByMouse: true
                Accessible.name: "Pesquisar no catálogo local"
                Accessible.description: "Busca por nome científico, nome comum disponível ou grupo taxonômico. Combine com os filtros de domínio e nível."
                onTextEdited: view.backend.exploreQuery = text
                onAccepted: view.backend.flushExploreSearch()
                onEditingFinished: view.backend.flushExploreSearch()
                background: Rectangle { color: Style.Theme.surface; radius: Style.Theme.controlRadius; border.color: searchField.activeFocus ? Style.Theme.focus : Style.Theme.border; border.width: searchField.activeFocus ? 2 : 1 }
                UiIcon { anchors.left: parent.left; anchors.leftMargin: 14; anchors.verticalCenter: parent.verticalCenter; name: "search"; ink: Style.Theme.green; implicitWidth: 18; implicitHeight: 18 }
                TextAction {
                    id: clearQuery; objectName: "clearExploreQuery"
                    anchors.right: parent.right; anchors.rightMargin: 5; anchors.verticalCenter: parent.verticalCenter
                    visible: searchField.text.length > 0; text: "×"; width: 36; height: 36
                    Accessible.name: "Limpar somente o texto da pesquisa"
                    onClicked: { view.backend.exploreQuery = ""; view.backend.flushExploreSearch(); searchField.forceActiveFocus(Qt.TabFocusReason) }
                }
            }
        }
        RowLayout {
            visible: view.browsingOnly; Layout.fillWidth: true; spacing: Style.Theme.spaceSm
            ColumnLayout {
                Layout.fillWidth: true; Layout.maximumWidth: 300; spacing: 4
                Label { text: "Domínio"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                TaxonComboBox { objectName: "exploreDomainFilter"; Layout.fillWidth: true; model: view.domains; textRole: "title"; valueRole: "id"; currentIndex: Math.max(0, view.domains.map(entry => entry.id).indexOf(view.backend.exploreDomain)); Accessible.name: "Filtrar por domínio"; onActivated: view.backend.exploreDomain = currentValue }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.maximumWidth: 300; spacing: 4
                Label { text: "Nível taxonômico"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                TaxonComboBox { objectName: "exploreRankFilter"; Layout.fillWidth: true; model: view.backend.exploreRanks; textRole: "title"; valueRole: "id"; currentIndex: Math.max(0, view.backend.exploreRanks.map(entry => entry.id).indexOf(view.backend.exploreRank)); Accessible.name: "Filtrar por nível taxonômico"; onActivated: view.backend.exploreRank = currentValue }
            }
            TextAction { objectName: "clearExploreSearch"; text: "Limpar tudo"; enabled: view.searching; Layout.alignment: Qt.AlignBottom; onClicked: { view.backend.exploreQuery = ""; view.backend.exploreDomain = ""; view.backend.exploreRank = ""; view.backend.flushExploreSearch(); searchField.forceActiveFocus(Qt.TabFocusReason) } }
            Item { Layout.fillWidth: true }
        }
        Label { visible: !view.browsingOnly; text: "Escolha sua próxima descoberta"; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
        Label { visible: !view.browsingOnly; text: "Selecione um grupo para aprender ou desafie seus conhecimentos no competitivo."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                    Accessible.description: modelData.pathText + ". " + view.countLabel(modelData.count, "organismo", "organismos") + "; explorar este grupo."
                    ToolTip.visible: hovered || visualFocus
                    ToolTip.delay: 600
                    ToolTip.text: modelData.pathText
                    onClicked: view.browsingOnly ? view.openEntry(modelData.id, objectName) : view.app.openTaxon(modelData.id)
                }
            }
        }
        Flow {
            visible: !view.browsingOnly; Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: 8
            Repeater {
                model: [{id:"kingdoms",title:"Reinos"},{id:"domains",title:"Domínios"},{id:"advanced",title:"Grupos avançados"}]
                delegate: UiButton {
                    required property var modelData
                    objectName: "playTab_" + modelData.id
                    text: modelData.title; compact: true; secondary: view.playTab !== modelData.id
                    checkable: true; checked: view.playTab === modelData.id; ButtonGroup.group: playTabs
                    Accessible.description: "Mostrar " + modelData.title + " para selecionar um grupo; não inicia partida."
                    onClicked: view.playTab = modelData.id
                }
            }
        }
        Label { visible: !view.browsingOnly; text: view.playTab === "kingdoms" ? "Reinos em destaque · Animalia, Plantae e Fungi" : view.playTab === "domains" ? "Os três domínios · navegue até grupos mais específicos" : "Linhagens de Eukaryota · ramos intermediários do catálogo"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
        Label { visible: view.browsingOnly && !view.searching; text: "OS TRÊS DOMÍNIOS · PORTAS DE EXPLORAÇÃO"; color: Style.Theme.textMuted; font.pixelSize: 10; font.letterSpacing: 1.6; Layout.topMargin: 6; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        GridLayout {
            id: groups
            visible: !view.browsingOnly || !view.searching
            objectName: "exploreDomainsGrid"
            readonly property real cardHeight: Math.max(0, ...Array.from(children, child => child.implicitHeight))
            Layout.fillWidth: true; columns: view.browsingOnly ? view.domainColumns : view.explorationColumns; columnSpacing: 16; rowSpacing: 16
            Repeater {
                model: !view.browsingOnly ? view.playEntries : !view.searching ? view.backend.categories.slice(1) : []
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: view.browsingOnly || view.playTab === "domains" ? "domainButton" + index : "playGroupButton" + index
                    entry: modelData
                    encyclopedic: view.browsingOnly
                    playSelection: !view.browsingOnly
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: 1
                    Layout.preferredHeight: groups.cardHeight
                    Layout.columnSpan: view.browsingOnly ? (index === view.backend.categories.length - 2 ? view.domainColumns - index % view.domainColumns : 1) : 1
                    onClicked: view.browsingOnly ? view.openEntry(modelData.id, objectName) : view.app.openTaxon(modelData.id)
                }
            }
        }
        ExploreCard {
            objectName: "categoryButton0"; visible: !view.browsingOnly; Layout.fillWidth: true
            entry: view.backend.playMixed; wide: true; playSelection: true
            enabled: !view.backend.resultPending && (entry.playLevels || []).some(level => level.id === entry.defaultRank)
            onClicked: view.app.beginTaxonGame("", entry.defaultRank)
        }
        RowLayout {
            visible: view.browsingOnly && !view.searching; Layout.fillWidth: true
            Label { text: "Organismos em destaque"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Layout.fillWidth: true; wrapMode: Text.WordWrap; Accessible.role: Accessible.Heading }
            TextAction { objectName: "allExploreOrganisms"; text: "Ver organismos →"; onClicked: view.backend.exploreRank = "species" }
            Label { objectName: "exploreFeaturedRange"; text: featuredGallery.firstVisible + "–" + featuredGallery.lastVisible + " / " + view.featuredEntries.length; Accessible.name: "Destaques visíveis: " + text; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
            TextAction { objectName: "previousFeaturedOrganisms"; text: "‹ Anteriores"; Accessible.name: "Organismos em destaque anteriores"; enabled: featuredGallery.contentX > 1; onClicked: featuredGallery.step(-1) }
            TextAction { objectName: "nextFeaturedOrganisms"; text: "Próximos ›"; Accessible.name: "Próximos organismos em destaque"; enabled: featuredGallery.contentX < featuredGallery.contentWidth - featuredGallery.width - 1; onClicked: featuredGallery.step(1) }
        }
        Flickable {
            id: featuredGallery
            objectName: "exploreFeaturedGrid"
            visible: view.browsingOnly && !view.searching
            Layout.fillWidth: true; implicitHeight: featuredRow.implicitHeight
            contentWidth: featuredRow.width; contentHeight: height
            clip: true; boundsBehavior: Flickable.StopAtBounds; flickableDirection: Flickable.HorizontalFlick
            // Whole logical pixels avoid layout rounding creating a phantom
            // scroll range when all four cards fit in the viewport.
            readonly property real cardWidth: width >= 1180 ? Math.floor((width - 48) / 4) : Math.max(280, Math.floor((width - 32) / 2.6))
            readonly property int firstVisible: Math.min(view.featuredEntries.length, Math.floor((contentX + 1) / (cardWidth + 16)) + 1)
            readonly property int lastVisible: Math.min(view.featuredEntries.length, Math.ceil((contentX + width - 1) / (cardWidth + 16)))
            function step(direction) { contentX = Math.max(0, Math.min(contentWidth - width, contentX + direction * (cardWidth + 16))) }
            function reveal(card) {
                if (card.x < contentX) contentX = card.x
                else if (card.x + card.width > contentX + width) contentX = card.x + card.width - width
            }
            onContentXChanged: view.rememberPresentation()
            Behavior on contentX { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationFast } }
            RowLayout {
                id: featuredRow; spacing: 16
                Repeater {
                    id: featuredResults
                    model: view.featuredEntries
                    delegate: ExploreCard {
                        required property var modelData
                        required property int index
                        objectName: "featuredOrganism" + index
                        entry: modelData; encyclopedic: true
                        Layout.preferredWidth: featuredGallery.cardWidth
                        Layout.minimumWidth: featuredGallery.cardWidth; Layout.maximumWidth: featuredGallery.cardWidth
                        Layout.fillHeight: true
                        onActiveFocusChanged: if (activeFocus) featuredGallery.reveal(this)
                        Keys.onRightPressed: { const next = featuredResults.itemAt(index + 1); if (next) next.forceActiveFocus(Qt.TabFocusReason) }
                        Keys.onLeftPressed: { const previous = featuredResults.itemAt(index - 1); if (previous) previous.forceActiveFocus(Qt.TabFocusReason) }
                        onClicked: view.openEntry(modelData.id, objectName)
                    }
                }
            }
        }
        Label { objectName: "exploreFeaturedPosition"; visible: view.browsingOnly && !view.searching; text: featuredGallery.firstVisible + "–" + featuredGallery.lastVisible + " de " + view.featuredEntries.length + " destaques" + (featuredGallery.contentX < featuredGallery.contentWidth - featuredGallery.width - 1 ? " · Há mais cartões à direita ›" : featuredGallery.contentX > 1 ? " · ‹ Há cartões à esquerda" : " · Todos visíveis"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { visible: view.browsingOnly && !view.searching; text: "Destaques com fotografia de identificação e resumo elegíveis no catálogo. As contagens representam organismos cadastrados, não a diversidade total dos grupos."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        ColumnLayout {
            visible: view.browsingOnly && view.searching
            Layout.fillWidth: true; spacing: 16
            Label { objectName: "exploreResultSummary"; text: view.searchPending ? "Atualizando pesquisa…" : view.countLabel(view.results.groupCount || 0, "grupo taxonômico", "grupos taxonômicos") + " · " + view.countLabel(view.results.organismCount || 0, "organismo cadastrado", "organismos cadastrados"); color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
            Label { objectName: "exploreEmptyState"; visible: !view.searchPending && view.results.total === 0; text: "Nenhum resultado para esta combinação. Tente outro nome, domínio ou nível taxonômico. A busca considera somente os dados disponíveis no catálogo local."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { visible: view.results.groupCount > 0; text: "Grupos taxonômicos"; color: Style.Theme.green; font.pixelSize: Style.Theme.labelSize; font.weight: Font.DemiBold }
            GridLayout {
                id: groupResultGrid
                objectName: "exploreGroupResultsGrid"
                readonly property real cardHeight: Math.max(0, ...Array.from(children, child => child.implicitHeight))
                Layout.fillWidth: true; columns: view.explorationColumns; columnSpacing: 16; rowSpacing: 16
                Repeater {
                    id: groupResults
                    objectName: "exploreGroupResults"
                    model: view.presentedGroups
                    delegate: ExploreCard {
                        required property var modelData
                        required property int index
                        objectName: "exploreGroupResult" + index
                        entry: modelData; encyclopedic: true
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: 1
                        Layout.preferredHeight: groupResultGrid.cardHeight
                        onClicked: view.openEntry(modelData.id, objectName)
                    }
                }
            }
            RowLayout {
                visible: view.results.groupCount > 0; Layout.fillWidth: true
                Label { objectName: "exploreGroupsShown"; text: "Exibindo " + groupResults.count + " de " + view.countLabel(view.results.groupCount || 0, "grupo taxonômico", "grupos taxonômicos"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                UiButton { objectName: "loadMoreExploreGroups"; visible: groupResults.count < view.results.groupCount; text: "Carregar mais grupos"; compact: true; onClicked: view.loadMore("groups", visualFocus) }
            }
            Label { visible: view.results.organismCount > 0; text: "Organismos cadastrados"; color: Style.Theme.green; font.pixelSize: Style.Theme.labelSize; font.weight: Font.DemiBold }
            GridLayout {
                id: organismResultGrid
                objectName: "exploreOrganismResultsGrid"
                readonly property real cardHeight: Math.max(0, ...Array.from(children, child => child.implicitHeight))
                Layout.fillWidth: true; columns: view.explorationColumns; columnSpacing: 16; rowSpacing: 16
                Repeater {
                    id: organismResults
                    objectName: "exploreOrganismResults"
                    model: view.presentedOrganisms
                    delegate: ExploreCard {
                        required property var modelData
                        required property int index
                        objectName: "exploreOrganismResult" + index
                        entry: modelData; encyclopedic: true
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: 1
                        Layout.preferredHeight: organismResultGrid.cardHeight
                        onClicked: view.openEntry(modelData.id, objectName)
                    }
                }
            }
            RowLayout {
                visible: view.results.organismCount > 0; Layout.fillWidth: true
                Label { objectName: "exploreOrganismsShown"; text: "Exibindo " + organismResults.count + " de " + view.countLabel(view.results.organismCount || 0, "organismo cadastrado", "organismos cadastrados"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                UiButton { objectName: "loadMoreExploreOrganisms"; visible: organismResults.count < view.results.organismCount; text: "Carregar mais organismos"; compact: true; onClicked: view.loadMore("organisms", visualFocus) }
            }
        }
        Label { text: "Três domínios são uma organização didática. Hipóteses de dois domínios situam os eucariotos dentro de Archaea. As relações profundas e os ranks variam conforme a fonte; a navegação não é uma árvore evolutiva completa."; color: view.browsingOnly ? Style.Theme.textMuted : "#91ADB2"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
