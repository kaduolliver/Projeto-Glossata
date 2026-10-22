pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
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
    property bool breadcrumbsExpanded: false
    property bool sourcesOpen: false
    property bool classificationOpen: false
    property Flickable pageViewport: null
    readonly property var containingWindow: Window.window
    readonly property bool organism: backend.browseTaxon.kind === "organism"
    readonly property string originalPhotoUrl: photo.photoUrl || photo.sourceUrl || ""
    readonly property var trail: backend.breadcrumbs
    readonly property int hiddenAncestorCount: Math.max(0, trail.length - 4)
    readonly property var photo: backend.browseTaxon.photo || ({})
    readonly property var introduction: browsingOnly && (backend.browseTaxon.summary || ({})).text ? backend.browseTaxon.summary : backend.browseTaxon.introduction
    function visit(id, keyboard) {
        backend.exploreTaxon(id)
        returnButton.forceActiveFocus(keyboard ? Qt.TabFocusReason : Qt.OtherFocusReason)
    }
    Component.onCompleted: {
        let ancestor = parent
        while (ancestor && !(ancestor instanceof Flickable)) ancestor = ancestor.parent
        pageViewport = ancestor
        positionPlayTop.restart()
    }
    // Only position this page. The selection's saved tab/scroll is untouched.
    Timer {
        id: positionPlayTop; interval: 0
        onTriggered: if (!view.browsingOnly && view.pageViewport) {
            view.pageViewport.cancelFlick()
            view.pageViewport.contentY = 0
        }
    }
    Connections { target: view.backend; function onExplorationChanged() {
        view.photoDetailsOpen = false; view.breadcrumbsExpanded = false; view.sourcesOpen = false; view.classificationOpen = false
        playSources.close(); positionPlayTop.restart()
    } }
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width; spacing: 18
        UiButton { id: returnButton; objectName: "returnDomainsButton"; Layout.topMargin: view.browsingOnly ? 0 : Style.Theme.spaceXs; text: view.browsingOnly ? "Voltar à pesquisa e aos domínios" : "Voltar à seleção de grupos"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate(view.browsingOnly ? "explore" : "categories") }
        Flow {
            objectName: "taxonomyBreadcrumbs"
            Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: 4
            TextAction {
                objectName: "breadcrumbButton0"
                readonly property var node: view.trail[0] || ({})
                text: (node.title || "") + (view.trail.length > 1 ? "  ›" : view.browsingOnly ? " · atual" : "")
                width: Math.min(implicitWidth, view.width)
                font.weight: view.browsingOnly && view.trail.length === 1 ? Font.DemiBold : Font.Normal
                Accessible.description: (node.rankLabel || "") + ". " + (view.trail.length === 1 ? "Nível atual." : "Navegar para este ancestral.")
                onClicked: view.visit(node.id, visualFocus)
            }
            TextAction {
                objectName: "expandTaxonomyBreadcrumbs"
                visible: view.browsingOnly && view.hiddenAncestorCount > 0
                text: view.breadcrumbsExpanded ? "Ocultar níveis ↑" : "… " + view.hiddenAncestorCount + (view.hiddenAncestorCount === 1 ? " nível ↓" : " níveis ↓")
                checkable: true; checked: view.breadcrumbsExpanded
                Accessible.name: view.breadcrumbsExpanded ? "Ocultar ancestrais intermediários" : "Mostrar todos os ancestrais intermediários"
                Accessible.description: view.breadcrumbsExpanded ? "Todos os níveis estão expandidos e disponíveis por teclado." : "Expande " + view.hiddenAncestorCount + " níveis entre o domínio e os ancestrais próximos; cada nível pode ser acessado."
                onClicked: view.breadcrumbsExpanded = !view.breadcrumbsExpanded
            }
            Repeater {
                model: view.trail.slice(1)
                delegate: TextAction {
                    required property var modelData
                    required property int index
                    readonly property int trailIndex: index + 1
                    readonly property bool currentLevel: trailIndex === view.trail.length - 1
                    objectName: "breadcrumbButton" + trailIndex
                    visible: !view.browsingOnly || view.breadcrumbsExpanded || trailIndex > view.hiddenAncestorCount
                    text: modelData.title + (currentLevel ? (view.browsingOnly ? " · atual" : "") : "  ›")
                    font.weight: view.browsingOnly && currentLevel ? Font.DemiBold : Font.Normal
                    width: Math.min(implicitWidth, view.width)
                    Accessible.description: modelData.rankLabel + ". " + (currentLevel ? "Nível atual." : "Navegar para este ancestral.")
                    onClicked: view.visit(modelData.id, visualFocus)
                }
            }
        }
        Rectangle {
            visible: view.browsingOnly
            objectName: "exploreTaxonOverview"
            Layout.fillWidth: true; implicitHeight: overview.implicitHeight + 32
            radius: Style.Theme.cardRadius; color: Style.Theme.surface; border.color: Style.Theme.border
            RowLayout {
                id: overview
                anchors.fill: parent; anchors.margins: 16; spacing: 24
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 10
                    Label { objectName: "exploreTaxonKind"; text: view.organism ? "ORGANISMO NO CATÁLOGO" : "GRUPO TAXONÔMICO"; color: view.organism ? Style.Theme.gold : Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { objectName: "exploreTaxonTitle"; text: view.backend.browseTaxon.title || ""; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; font.italic: view.backend.browseTaxon.rank === "species" || view.backend.browseTaxon.rank === "genus"; wrapMode: Text.Wrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                    Label { visible: !!view.backend.browseTaxon.commonName; text: view.backend.browseTaxon.commonName || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { text: (view.backend.browseTaxon.rankLabel || "") + (view.organism ? "" : " · " + view.backend.browseTaxon.count + (view.backend.browseTaxon.count === 1 ? " organismo cadastrado" : " organismos cadastrados")); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.labelSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { visible: !view.organism; text: "Contagem do catálogo local; não representa a diversidade total deste táxon."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                Rectangle {
                    Layout.preferredWidth: view.width * (view.organism ? 0.48 : 0.42); Layout.preferredHeight: view.organism ? 260 : 210
                    radius: 12; color: Style.Theme.surfaceRaised
                    gradient: Gradient { GradientStop { position: 0; color: Style.Theme.surfaceRaised } GradientStop { position: 1; color: Style.Theme.canvas } }
                    Image {
                        id: taxonPhoto; objectName: "exploreTaxonPhoto"
                        readonly property real fittedScale: sourceSize.width > 0 && sourceSize.height > 0 ? Math.min(1, (parent.width - 12) / sourceSize.width, (parent.height - 12) / sourceSize.height) : 0
                        width: sourceSize.width * fittedScale; height: sourceSize.height * fittedScale
                        x: (parent.width - width) / 2
                        y: (parent.height - height) / 2
                        source: view.browsingOnly ? view.photo.image || "" : ""; fillMode: Image.PreserveAspectFit; asynchronous: true; smooth: true; mipmap: true
                    }
                    Label { visible: taxonPhoto.status !== Image.Ready; anchors.centerIn: parent; width: parent.width - 24; text: taxonPhoto.status === Image.Loading ? "Carregando fotografia…" : "Fotografia ainda indisponível"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter }
                }
            }
        }
        ColumnLayout {
            visible: view.browsingOnly; Layout.fillWidth: true; spacing: 6
            Label { objectName: "explorePhotoAttribution"; text: view.photo.image ? (view.photo.kind === "micrograph" ? "Micrografia: " : "Fotografia: ") + (view.photo.subject || "") + " · " + (view.photo.author || "Autoria não informada") + " · " + (view.photo.license || "Licença não informada") : view.photo.pendingReason || "Fotografia adequada ainda não disponível."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!view.photo.image; text: view.photo.identificationNote || "Representante do grupo; identificação indicada pela fonte."; textFormat: Text.PlainText; color: Style.Theme.gold; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        Label { visible: !view.browsingOnly; text: view.backend.browseTaxon.title || ""; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; font.italic: view.backend.browseTaxon.rank === "species" || view.backend.browseTaxon.rank === "genus"; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
        Label { visible: !view.browsingOnly; text: (view.backend.browseTaxon.rankLabel || "") + " · " + view.backend.browseTaxon.count + (view.backend.browseTaxon.count === 1 ? " organismo no catálogo" : " organismos no catálogo"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Rectangle {
            visible: view.browsingOnly
            Layout.fillWidth: true; implicitHeight: introductionHeading.implicitHeight + 1
            color: "transparent"
            Label { id: introductionHeading; text: view.organism ? "Sobre este organismo" : "Conheça este grupo"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: view.browsingOnly ? 8 : 4
            Label { objectName: "exploreIntroductionText"; text: view.introduction.text || view.backend.browseTaxon.description || "Introdução científica ainda não disponível para este grupo."; textFormat: Text.PlainText; color: "#9DB8BC"; font.pixelSize: 13; wrapMode: Text.WordWrap; lineHeight: 1.35; Layout.fillWidth: true }
            Label { objectName: "taxonIntroductionReview"; text: view.backend.browseTaxon.introductionReviewNotice || ""; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: (view.backend.browseTaxon.rankNote || "") + " " + (view.backend.browseTaxon.placementNotice || ""); color: "#D9BA7D"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            TextAction { id: playSourcesButton; objectName: "playTaxonSourcesButton"; visible: !view.browsingOnly; text: "Fonte, origem e créditos"; Accessible.description: "Consultar fontes deste grupo, origem e estados de revisão; não inicia uma partida."; onClicked: playSources.open() }
        }
        Label { visible: !view.browsingOnly && view.backend.gameFinished && view.backend.feedbackText.indexOf("alternativas aprovadas") >= 0; text: view.backend.feedbackText; color: "#D9B879"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Rectangle {
            objectName: "playConfigurationPanel"
            visible: !view.browsingOnly
            Layout.fillWidth: true; implicitHeight: controls.implicitHeight + 2 * Style.Theme.spaceMd
            radius: Style.Theme.cardRadius; color: Style.Theme.surfaceRaised; border.color: Style.Theme.border
            ColumnLayout {
                id: controls
                anchors.fill: parent; anchors.margins: Style.Theme.spaceMd; spacing: Style.Theme.spaceXs
                Label { text: "Configurar identificação"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                GridLayout {
                    id: setupControls
                    // Use available panel space, not the grid's own maximum
                    // width derived from its capped controls.
                    readonly property bool wide: controls.width >= 820
                    columns: wide ? 3 : 2; columnSpacing: Style.Theme.spaceLg; rowSpacing: Style.Theme.spaceXs
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.row: 0; Layout.column: 0
                        Layout.columnSpan: setupControls.wide ? 1 : 2; Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        Layout.fillWidth: true; Layout.preferredWidth: 330; Layout.maximumWidth: 360; spacing: Style.Theme.spaceXs
                        Label { text: "Identificar"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                        TaxonComboBox { id: level; objectName: "identificationLevel"; Layout.fillWidth: true; model: view.backend.identificationLevels; textRole: "title"; valueRole: "id"; Accessible.name: "Nível de identificação no modo Livre"; enabled: count > 0; onModelChanged: currentIndex = 0 }
                    }
                    Label { objectName: "playDifficultyNotice"; text: "Livre · alternativas " + ({broad: "Mais amplas", balanced: "Equilibradas", close: "Mais próximas"}[view.backend.difficulty] || ""); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; horizontalAlignment: Text.AlignLeft; Layout.row: setupControls.wide ? 0 : 1; Layout.column: setupControls.wide ? 1 : 0; Layout.fillWidth: true; Accessible.description: "Dificuldade preservada da tela principal de Jogar." }
                    UiButton { objectName: "startTaxonButton"; text: "Iniciar partida"; iconName: "play"; compact: true; Layout.row: setupControls.wide ? 0 : 1; Layout.column: setupControls.wide ? 2 : 1; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter; enabled: level.count > 0 && !view.backend.resultPending; onClicked: view.app.beginTaxonGame(view.backend.browseTaxon.id, level.currentValue) }
                }
                Label { objectName: "playEligibleCount"; text: level.currentIndex >= 0 && level.currentIndex < view.backend.identificationLevels.length ? view.backend.identificationLevels[level.currentIndex].count + (view.backend.identificationLevels[level.currentIndex].count === 1 ? " organismo elegível neste nível" : " organismos elegíveis neste nível") + " · " + view.backend.identificationLevels[level.currentIndex].answers + " alternativas distintas. A dificuldade escolhida pode limitar a partida; consulte os avisos das fotografias." : "Sem partida disponível: são necessárias quatro alternativas distintas e revisões adequadas. Você pode continuar navegando pelos grupos."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
        }
        Label { visible: view.backend.browseTaxon.supportingCount > 0; text: "Há imagens de apoio neste grupo. A identificação usa pistas; essas fotos podem representar colônias, sintomas ou organismos relacionados, sem comprovar a espécie."; color: "#91ABB4"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { objectName: "exploreDescendantsState"; text: view.backend.browseChildren.length ? "Explore os próximos grupos" : view.browsingOnly && view.backend.browseTaxon.count === 0 ? "Este grupo ainda não tem organismos no catálogo." : "Você chegou ao último nível disponível."; color: view.browsingOnly ? Style.Theme.text : "#BCD1C7"; font.pixelSize: 18; Layout.topMargin: 8; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        GridLayout {
            id: childGrid
            objectName: "taxonChildrenGrid"
            readonly property real cardHeight: Math.max(0, ...Array.from(children, child => child.implicitHeight))
            Layout.fillWidth: true; columns: Math.max(1, Math.floor((view.width + 14) / (280 + 14))); columnSpacing: 14; rowSpacing: 14
            Repeater {
                model: view.browsingOnly ? view.backend.browseChildren : view.backend.playChildren
                delegate: ExploreCard {
                    required property var modelData
                    required property int index
                    objectName: "taxonButton" + index
                    entry: modelData
                    encyclopedic: view.browsingOnly
                    playSelection: !view.browsingOnly
                    Layout.fillWidth: true; Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    Layout.preferredHeight: childGrid.cardHeight
                    onClicked: view.visit(modelData.id, visualFocus)
                }
            }
        }
        Rectangle {
            objectName: "exploreScientificFooter"
            visible: view.browsingOnly
            Layout.fillWidth: true; implicitHeight: scientificDetails.implicitHeight + 24
            color: Style.Theme.surface; radius: Style.Theme.controlRadius; border.color: Style.Theme.border
            ColumnLayout {
                id: scientificDetails; anchors.fill: parent; anchors.margins: 12; spacing: 6
                TextAction { objectName: "exploreClassification"; text: "Classificação taxonômica " + (view.classificationOpen ? "↑" : "↓"); checkable: true; checked: view.classificationOpen; onClicked: view.classificationOpen = !view.classificationOpen }
                ColumnLayout {
                    objectName: "exploreClassificationDetails"; visible: view.classificationOpen; Layout.fillWidth: true; spacing: 4
                    Repeater {
                        model: view.trail
                        delegate: Label {
                            required property var modelData
                            text: modelData.rankLabel + " · " + modelData.title; textFormat: Text.PlainText
                            color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true
                        }
                    }
                }
                TextAction { objectName: "exploreSources"; text: "Fontes e proveniência científica " + (view.sourcesOpen ? "↑" : "↓"); checkable: true; checked: view.sourcesOpen; onClicked: view.sourcesOpen = !view.sourcesOpen }
                ColumnLayout {
                    objectName: "exploreSourceDetails"; visible: view.sourcesOpen; Layout.fillWidth: true; spacing: 6
                    Label { text: view.introduction.source_title || "Referência científica ainda não disponível."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Flow {
                        Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: 8
                        TextAction { objectName: "exploreScientificSource"; visible: !!view.introduction.source_url; text: "Referência do texto ↗"; Accessible.description: view.introduction.source_url || ""; onClicked: Qt.openUrlExternally(view.introduction.source_url) }
                        TextAction { objectName: "explorePlacementSource"; visible: !!view.backend.browseTaxon.sourceUrl; text: "Fonte do enquadramento ↗"; Accessible.description: view.backend.browseTaxon.sourceUrl || ""; onClicked: Qt.openUrlExternally(view.backend.browseTaxon.sourceUrl) }
                    }
                }
            TextAction { objectName: "explorePhotoCredits"; visible: !!view.photo.image; text: view.photoDetailsOpen ? "Ocultar foto e créditos ↑" : "Foto, origem e créditos ↓"; Accessible.description: "Expandir os créditos e as limitações desta fotografia na página."; onClicked: view.photoDetailsOpen = !view.photoDetailsOpen }
            ColumnLayout {
                objectName: "explorePhotoDetails"
                visible: view.photoDetailsOpen; Layout.fillWidth: true; spacing: 6
                Label { objectName: "explorePhotoProvenance"; text: (view.photo.title || "Título não informado") + "\nModificações históricas registradas\n" + (view.photo.modifications || "Alterações não documentadas."); textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { objectName: "explorePhotoPresentation"; text: "Apresentação atual no Explorar: imagem inteira, centralizada e proporcional, sem recorte, distorção ou ampliação além dos pixels armazenados. Os metadados podem descrever apresentações anteriores ou de outras telas; permanecem preservados."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Flow {
                    Layout.fillWidth: true; Layout.preferredHeight: childrenRect.height; spacing: 8
                    TextAction { objectName: "explorePhotoOriginal"; visible: !!view.originalPhotoUrl; text: "Fotografia original ↗"; Accessible.description: view.originalPhotoUrl; onClicked: Qt.openUrlExternally(view.originalPhotoUrl) }
                    TextAction { objectName: "explorePhotoLicense"; visible: !!view.photo.licenseUrl; text: "Termos da licença ↗"; Accessible.description: view.photo.licenseUrl || ""; onClicked: Qt.openUrlExternally(view.photo.licenseUrl) }
                    TextAction { text: "Créditos e transparência"; onClicked: view.app.navigate("credits") }
                }
            }
            }
        }
    }
    Dialog {
        id: playSources; objectName: "playTaxonSourcesDialog"
        readonly property var taxon: view.backend.browseTaxon
        readonly property var note: view.introduction || ({})
        readonly property var review: note.governance || ({})
        parent: Overlay.overlay; anchors.centerIn: parent
        width: Math.min(680, parent.width - 40); height: Math.min(650, parent.height - 40)
        title: "Fonte, origem e créditos"; modal: true; focus: true
        closePolicy: Popup.CloseOnEscape
        function revealFocusedLink() {
            const control = view.containingWindow ? view.containingWindow.activeFocusItem : null
            let ancestor = control
            while (ancestor && ancestor !== playSourcesScroll) ancestor = ancestor.parent
            const viewport = playSourcesScroll.contentItem as Flickable
            if (!ancestor || !control || !viewport) return
            const top = control.mapToItem(viewport, 0, 0).y
            const bottom = top + control.height
            if (top < 8) viewport.contentY = Math.max(0, viewport.contentY + top - 8)
            else if (bottom > viewport.height - 8) viewport.contentY = Math.min(viewport.contentHeight - viewport.height, viewport.contentY + bottom - viewport.height + 8)
        }
        Timer { id: revealSourceFocus; interval: 0; onTriggered: playSources.revealFocusedLink() }
        Connections { target: view.containingWindow; function onActiveFocusItemChanged() { if (playSources.opened) revealSourceFocus.restart() } }
        onOpened: {
            const viewport = playSourcesScroll.contentItem as Flickable
            if (viewport) viewport.contentY = 0
            closePlaySources.forceActiveFocus(Qt.PopupFocusReason)
        }
        onClosed: if (!view.browsingOnly) playSourcesButton.forceActiveFocus(Qt.PopupFocusReason)
        footer: DialogButtonBox {
            UiButton { id: closePlaySources; objectName: "playTaxonSourcesClose"; text: "Fechar"; compact: true; DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            onRejected: playSources.close()
        }
        contentItem: ScrollView {
            id: playSourcesScroll; objectName: "playTaxonSourcesContent"
            clip: true; contentWidth: availableWidth
            ColumnLayout {
                width: playSourcesScroll.availableWidth; spacing: Style.Theme.spaceSm
                Label { text: "Identificação taxonômica"; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
                Label { objectName: "playSourceIdentity"; text: (playSources.taxon.title || "") + " · " + (playSources.taxon.rankLabel || "") + "\nID local: " + (playSources.taxon.id || ""); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Label { text: "Enquadramento científico"; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
                Label { objectName: "playSourcePlacement"; text: [playSources.taxon.rankNote || "", playSources.taxon.placementNotice || "", playSources.taxon.sourceUrl || "Referência não disponível para este grupo."].filter(value => !!value).join("\n"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                TextAction { objectName: "playPlacementSourceLink"; visible: !!playSources.taxon.sourceUrl; text: "Abrir fonte do enquadramento ↗"; Accessible.description: playSources.taxon.sourceUrl || ""; onClicked: Qt.openUrlExternally(playSources.taxon.sourceUrl) }
                Label { text: "Referência educativa"; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
                Label { objectName: "playSourceEducation"; text: [playSources.note.source_title || "Sem referência educativa utilizável registrada para este grupo.", playSources.note.source_url || "", playSources.taxon.introductionReviewNotice || ""].filter(value => !!value).join("\n"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                TextAction { objectName: "playEducationSourceLink"; visible: !!playSources.note.source_url; text: "Abrir referência educativa ↗"; Accessible.description: playSources.note.source_url || ""; onClicked: Qt.openUrlExternally(playSources.note.source_url) }
                Label {
                    objectName: "playSourceReview"
                    text: [["Revisão", playSources.review.label], ["Estado registrado", playSources.review.state], ["Responsável registrado", playSources.review.reviewer], ["Data registrada", playSources.review.reviewed_at], ["Tipo de revisão", playSources.review.review_kind], ["Validade registrada", playSources.review.due_at], ["Escopo", playSources.note.scope_name], ["Limitações", playSources.review.limitation]].filter(field => !!field[1]).map(field => field[0] + ": " + field[1]).join("\n")
                    textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true
                }
                Label { text: "Proveniência"; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
                Label {
                    objectName: "playSourceProvenance"
                    text: [["Origem do táxon", playSources.taxon.source], ["Identificador na origem", playSources.taxon.sourceTaxonId], ["Registro da revisão", playSources.review.reason], ["Escopo declarado", playSources.review.claim_scope], ["SHA-256 do conteúdo revisado", playSources.review.content_sha256]].filter(field => !!field[1]).map(field => field[0] + ": " + field[1]).join("\n")
                    textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true
                }
                Repeater {
                    model: playSources.review.evidence || []
                    delegate: ColumnLayout {
                        id: evidenceEntry
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                        Label { text: [modelData.url || "", modelData.locator || "", modelData.supports || ""].filter(value => !!value).join("\n"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        TextAction { objectName: "playSourceEvidenceLink" + evidenceEntry.index; visible: !!evidenceEntry.modelData.url; text: "Abrir evidência registrada ↗"; Accessible.description: evidenceEntry.modelData.url || ""; onClicked: Qt.openUrlExternally(evidenceEntry.modelData.url) }
                    }
                }
                Label { text: "Fontes disponíveis no catálogo local, consultáveis offline. Links externos dependem de conexão. As fotografias dos cartões descendentes possuem seus próprios créditos contextuais; nenhuma fotografia é apresentada neste diálogo."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            }
        }
    }
}
