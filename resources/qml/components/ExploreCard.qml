pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
Button {
    id: card
    required property var entry
    readonly property var containingWindow: Window.window
    property bool wide: false
    property bool encyclopedic: false
    property bool playSelection: false
    readonly property bool modernPresentation: encyclopedic || playSelection
    readonly property bool landscape: modernPresentation && entry.kind !== "organism" && width >= 450
    readonly property string shortClassification: entry.classificationSummary || ""
    readonly property var educationalNote: entry.educationalNote || ({})
    readonly property bool inlineAttribution: !!photo.image && photo.contextualAttribution !== true
    readonly property string photoLicenseVersion: photo.licenseVersion || (photo.attribution || ({})).license_version || ""
    property Flickable photoViewport: null
    property bool photoRequested: false
    function locatePhotoViewport() {
        if (!modernPresentation) return
        let ancestor = parent
        while (ancestor && !(ancestor instanceof Flickable)) ancestor = ancestor.parent
        photoViewport = ancestor
        updatePhotoRequest()
    }
    function updatePhotoRequest() {
        if (!modernPresentation) return
        if (!photoViewport) { photoRequested = visible; return }
        const position = modernPhotoFrame.mapToItem(photoViewport, 0, 0)
        photoRequested = visible && position.y < photoViewport.height && position.y + modernPhotoFrame.height > 0
            && position.x < photoViewport.width && position.x + modernPhotoFrame.width > 0
        let ancestor = photoViewport.parent
        while (photoRequested && ancestor) {
            if (ancestor instanceof Flickable) {
                const outerPosition = modernPhotoFrame.mapToItem(ancestor, 0, 0)
                photoRequested = outerPosition.y < ancestor.height && outerPosition.y + modernPhotoFrame.height > 0
                    && outerPosition.x < ancestor.width && outerPosition.x + modernPhotoFrame.width > 0
            }
            ancestor = ancestor.parent
        }
    }
    function schedulePhotoCheck() { if (modernPresentation && photoCheck) photoCheck.restart() }
    Timer {
        id: photoCheck
        interval: 0
        onTriggered: { if (card.photoViewport) card.updatePhotoRequest(); else card.locatePhotoViewport() }
    }
    Component.onCompleted: schedulePhotoCheck()
    onParentChanged: { photoViewport = null; schedulePhotoCheck() }
    onYChanged: schedulePhotoCheck()
    onHeightChanged: schedulePhotoCheck()
    onVisibleChanged: schedulePhotoCheck()
    Connections {
        target: card.photoViewport
        function onContentYChanged() { card.schedulePhotoCheck() }
        function onContentXChanged() { card.schedulePhotoCheck() }
        function onContentHeightChanged() { card.schedulePhotoCheck() }
        function onWidthChanged() { card.schedulePhotoCheck() }
        function onHeightChanged() { card.schedulePhotoCheck() }
    }
    // The nearest viewport may be the horizontal gallery inside the page.
    property Flickable outerViewport: null
    function locateOuterViewport() {
        let ancestor = photoViewport ? photoViewport.parent : null
        while (ancestor && !(ancestor instanceof Flickable)) ancestor = ancestor.parent
        outerViewport = ancestor
    }
    onPhotoViewportChanged: locateOuterViewport()
    Connections {
        target: card.outerViewport
        function onContentYChanged() { card.schedulePhotoCheck() }
        function onHeightChanged() { card.schedulePhotoCheck() }
        function onWidthChanged() { card.schedulePhotoCheck() }
        function onContentHeightChanged() { card.schedulePhotoCheck() }
    }
    readonly property var photo: entry.photo || ({})
    readonly property string photoCredit: photo.image ? (photo.kind === "micrograph" ? "Micrografia: " : "Fotografia: ") + photo.subject + ". " + photo.author + " · " + photo.license + ". " + (photo.identificationNote || "Representante do grupo; identificação indicada pela fonte.") + " Créditos completos em Perfis e ajustes → Créditos e transparência." : "Ilustração temporária. " + (photo.pendingReason || "Fotografia adequada ainda não disponível.")
    property color accent: entry.kingdom === "Plantae" || entry.kingdom === "Archaeplastida" ? "#7DA88A" : entry.kingdom === "Fungi" ? "#C29580" : entry.kingdom === "Eukaryota" || entry.kingdom === "SAR" ? "#86A9C9" : entry.kingdom === "Bacteria" || entry.kingdom === "Archaea" ? "#7BBABE" : "#D1B17B"
    implicitHeight: modernPresentation ? modernBody.implicitHeight + topPadding + bottomPadding : wide ? 132 : 185
    padding: modernPresentation ? 10 : 20; hoverEnabled: !modernPresentation
    // Passive, card-wide observation remains stable over the nested action.
    // Do not arbitrate hover between two QQuickControl instances.
    HoverHandler { id: cardPointer; enabled: card.modernPresentation }
    activeFocusOnTab: true
    Accessible.name: modernPresentation ? entry.title + ", " + (entry.kind === "organism" ? "organismo cadastrado" : entry.rankLabel + ", " + entry.count + (entry.count === 1 ? " organismo no catálogo" : " organismos no catálogo")) : entry.title + ", " + entry.count + (entry.count === 1 ? " espécie" : " espécies")
    Accessible.description: (entry.pathText || entry.description || "") + ". " + photoCredit
    ToolTip {
        visible: !card.modernPresentation && (card.hovered || card.visualFocus)
        delay: 600
        timeout: -1
        width: Math.min(360, card.width)
        contentItem: Text { text: card.photoCredit; textFormat: Text.PlainText; wrapMode: Text.WordWrap; color: "#EDF4EE"; font.pixelSize: 12 }
        background: Rectangle { color: "#203A43"; radius: 8; border.color: "#4B666C" }
    }
    background: Rectangle {
        objectName: card.modernPresentation ? "encyclopediaCardSurface" : ""
        radius: 17; color: card.modernPresentation ? (card.down ? Style.Theme.primaryPressed : cardPointer.hovered ? Style.Theme.surfaceHover : Style.Theme.surface) : card.hovered ? "#1C3439" : "#142B33"
        Behavior on color { enabled: card.modernPresentation && Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
        FeatheredPhoto {
            id: photograph
            visible: !card.modernPresentation
            objectName: "explorationPhoto"
            source: !card.modernPresentation && card.visible ? card.photo.image || "" : ""
            width: parent.width * 0.42
            height: parent.height
            anchors.right: parent.right
            cornerRadius: parent.radius
            focusX: card.photo.focusX === undefined ? 0.5 : card.photo.focusX
            focusY: card.photo.focusY === undefined ? 0.5 : card.photo.focusY
        }
        Rectangle { anchors.fill: parent; radius: parent.radius; color: "transparent"; border.color: card.modernPresentation ? (card.visualFocus ? Style.Theme.focus : Style.Theme.border) : card.activeFocus ? card.accent : (card.hovered ? "#526565" : "#2D444A"); border.width: (card.modernPresentation ? card.visualFocus : card.activeFocus) ? 2 : 1 }
    }
    contentItem: Item {
        KingdomArtwork { visible: !card.modernPresentation && !card.wide && !photograph.ready; kingdom: card.entry.kingdom || "Animalia"; width: 96; height: 96; anchors.right: parent.right; anchors.top: parent.top; opacity: 0.7 }
        LifeArtwork { visible: !card.modernPresentation && card.wide && !photograph.ready; width: 250; height: 115; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; opacity: 0.6 }
        ColumnLayout {
            visible: !card.modernPresentation
            anchors.left: parent.left; anchors.bottom: parent.bottom
            width: card.width * 0.60 - card.leftPadding - 8; spacing: 6
            Label { text: card.wide ? "JOGO MISTO · GÊNEROS" : card.entry.rankLabel; color: card.accent; font.pixelSize: 9; font.letterSpacing: 1; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.entry.title; color: "#E8F1E9"; font.pixelSize: card.wide ? 24 : 21; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { text: card.entry.count === 0 ? "Ainda sem organismos no catálogo" : card.entry.count + (card.entry.count === 1 ? " cadastrada · " : " cadastradas · ") + (card.wide ? card.entry.playableCount + (card.entry.playableCount === 1 ? " disponível neste modo" : " disponíveis neste modo") : (card.entry.supportingCount || 0) > 0 ? card.entry.supportingCount + " com imagem de apoio" : (card.entry.visualCount || 0) + " com foto aprovada"); color: "#91ABB0"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        GridLayout {
            id: modernBody
            visible: card.modernPresentation
            anchors.fill: parent
            columns: card.landscape ? 2 : 1
            columnSpacing: 16; rowSpacing: Style.Theme.spaceXs
            Rectangle {
                id: modernPhotoFrame
                objectName: "encyclopediaPhotoFrame"
                Layout.fillWidth: !card.landscape
                Layout.preferredWidth: card.landscape ? card.availableWidth * 0.42 : -1
                Layout.alignment: Qt.AlignTop
                implicitHeight: card.entry.kind === "organism" ? 160 : 146
                Layout.maximumHeight: implicitHeight
                radius: 10; color: Style.Theme.canvas
                // A restrained surround fills free margins without duplicating,
                // stretching or cropping the scientific photograph.
                gradient: Gradient {
                    GradientStop { position: 0; color: Style.Theme.surfaceRaised }
                    GradientStop { position: 1; color: Style.Theme.canvas }
                }
                onYChanged: card.schedulePhotoCheck()
                onWidthChanged: card.schedulePhotoCheck()
                Image {
                    id: modernPhoto
                    objectName: "encyclopediaCardPhoto"
                    // Center the entire stored raster, without cropping or upscaling.
                    readonly property real fittedScale: sourceSize.width > 0 && sourceSize.height > 0 ? Math.min(1, parent.width / sourceSize.width, parent.height / sourceSize.height) : 0
                    width: sourceSize.width * fittedScale; height: sourceSize.height * fittedScale
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2
                    source: card.modernPresentation && card.photoRequested ? card.photo.image || "" : ""
                    fillMode: Image.PreserveAspectFit; asynchronous: true; smooth: true; mipmap: true
                }
                Label { objectName: "encyclopediaCardPhotoState"; visible: modernPhoto.status !== Image.Ready; anchors.centerIn: parent; width: parent.width - 20; text: modernPhoto.status === Image.Loading ? "Carregando fotografia…" : "Fotografia ainda indisponível"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter }
            }
            ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: Style.Theme.spaceXs
            Label { text: card.playSelection && card.wide ? "JOGO MISTO · GÊNEROS" : card.entry.kind === "organism" ? "ORGANISMO · " + (card.entry.rankLabel || "") : card.entry.rankLabel || ""; textFormat: Text.PlainText; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 0.6; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.topMargin: 2 }
            Label { objectName: "encyclopediaCardTitle"; text: card.entry.title; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: 18; font.italic: card.entry.rank === "species" || card.entry.rank === "genus"; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.entry.commonName; text: card.entry.commonName || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { objectName: "encyclopediaCardCount"; text: card.entry.kind === "organism" ? card.shortClassification : card.entry.count === 0 ? "Sem organismos no catálogo" : card.entry.count + (card.entry.count === 1 ? " organismo no catálogo" : " organismos no catálogo"); textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.photo.subject && card.entry.kind !== "organism"; text: "Representante: " + (card.photo.subject || ""); textFormat: Text.PlainText; font.italic: true; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { objectName: "playCardAvailability"; visible: card.playSelection; text: card.wide ? (card.entry.playableCount || 0) + " elegíveis para identificar gêneros" : (card.entry.playLevels || []).length ? (card.entry.playLevels.length + (card.entry.playLevels.length === 1 ? " nível de identificação disponível" : " níveis de identificação disponíveis")) : "Sem partida disponível neste grupo"; textFormat: Text.PlainText; color: (card.entry.playLevels || []).length ? Style.Theme.green : Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { objectName: "encyclopediaCardSummary"; visible: !card.playSelection && !!card.educationalNote.text; text: card.educationalNote.text || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; maximumLineCount: 2; elide: Text.ElideRight; Layout.fillWidth: true; Accessible.description: "Trecho do texto existente; conteúdo integral e fonte na página correspondente." }
            Label { objectName: "encyclopediaCardAttribution"; visible: card.inlineAttribution; text: (card.photo.author || "Autoria não informada") + " · " + (card.photo.license || "Licença não informada") + (card.photo.attributionNotice ? "\n" + card.photo.attributionNotice : ""); textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: 11; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { objectName: "encyclopediaCardLicenseNotice"; visible: !!card.photo.image && !card.photoLicenseVersion; text: "Versão da licença não comprovada."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: 11; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Item { Layout.fillHeight: true; Layout.minimumHeight: 0 }
            Item {
                objectName: "exploreCardCreditsRow"
                Layout.fillWidth: true; implicitHeight: creditButton.implicitHeight
                // Empty space on this row is inert, rather than part of either
                // action. The button above this blocker keeps its own hitbox.
                MouseArea { anchors.fill: parent; acceptedButtons: Qt.LeftButton }
                TextAction {
                    id: creditButton; objectName: card.modernPresentation ? "exploreCardCreditsButton" : ""
                    text: "Fonte, origem e créditos"
                    leftPadding: 0
                    rightPadding: Style.Theme.spaceMd
                    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
                    width: Math.min(implicitWidth, parent.width)
                    anchors.left: parent.left
                    Accessible.name: text + " — " + card.entry.title
                    // The card's hover already uses surfaceHover. A separate
                    // outline remains visible against that surface as well.
                    background: Rectangle {
                        objectName: "textActionBackground"
                        radius: Style.Theme.controlRadius
                        color: !creditButton.enabled ? Style.Theme.disabledSurface : creditButton.down ? Style.Theme.primaryPressed : (creditButton.hovered || creditButton.visualFocus ? Style.Theme.surfaceRaised : "transparent")
                        border.color: creditButton.visualFocus ? Style.Theme.focus : creditButton.hovered ? Style.Theme.green : "transparent"
                        border.width: creditButton.visualFocus ? Style.Theme.focusWidth : Style.Theme.borderWidth
                        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                        Behavior on border.color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                    }
                    onClicked: { const popup = creditsLoader.item as Dialog; if (popup) popup.open(); else creditsLoader.active = true }
                }
            }
            RowLayout {
                objectName: "encyclopediaCardPrimaryAction"
                Layout.fillWidth: true; Layout.topMargin: Style.Theme.spaceXs
                Label { objectName: "exploreCardActionLabel"; text: card.playSelection ? (card.wide ? "Iniciar jogo misto" : "Selecionar grupo") : card.entry.kind === "organism" ? "Conhecer organismo" : "Explorar grupo"; color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize }
                Item { Layout.fillWidth: true }
                UiIcon { name: "arrow"; ink: Style.Theme.green; implicitWidth: 16; implicitHeight: 16 }
            }
            }
        }
    }
    Timer {
        id: releaseCredits; interval: 0
        onTriggered: { const popup = creditsLoader.item as Dialog; if (popup && !popup.opened) creditsLoader.active = false }
    }
    Loader {
        id: creditsLoader; active: false
        onLoaded: { const popup = item as Dialog; if (popup) popup.open() }
        sourceComponent: Component {
            Dialog {
                id: credits
                objectName: "exploreCardCreditsDialog"
                readonly property var attribution: card.photo.attribution || ({})
                readonly property string originalUrl: card.photo.photoUrl || card.photo.sourceUrl || ""
                readonly property string licenseUrl: card.photo.licenseUrl || attribution.license_url || attribution.license_source_url || ""
                readonly property string version: card.photoLicenseVersion
                parent: Overlay.overlay; anchors.centerIn: parent
                width: Math.min(660, parent.width - 40); height: Math.min(660, parent.height - 40)
                title: "Fonte, origem e créditos"; modal: true; focus: true
                function revealFocusedLink() {
                    const control = card.containingWindow ? card.containingWindow.activeFocusItem : null
                    let ancestor = control
                    while (ancestor && ancestor !== creditsScroll) ancestor = ancestor.parent
                    if (!ancestor || !control) return
                    const viewport = creditsScroll.contentItem as Flickable
                    if (!viewport) return
                    const top = control.mapToItem(viewport, 0, 0).y
                    const bottom = top + control.height
                    if (top < 8) viewport.contentY = Math.max(0, viewport.contentY + top - 8)
                    else if (bottom > viewport.height - 8) viewport.contentY = Math.min(viewport.contentHeight - viewport.height, viewport.contentY + bottom - viewport.height + 8)
                }
                Timer { id: revealCreditsFocus; interval: 0; onTriggered: credits.revealFocusedLink() }
                Connections { target: card.containingWindow; function onActiveFocusItemChanged() { if (credits.opened) revealCreditsFocus.restart() } }
                onOpened: closeCredits.forceActiveFocus(Qt.PopupFocusReason)
                onClosed: { creditButton.forceActiveFocus(Qt.PopupFocusReason); releaseCredits.restart() }
                footer: DialogButtonBox {
                    UiButton { id: closeCredits; objectName: "exploreCardCreditsClose"; text: "Fechar"; DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
                    onRejected: credits.close()
                }
                contentItem: ScrollView {
                    id: creditsScroll; objectName: "exploreCardCreditsContent"
                    clip: true; contentWidth: availableWidth
                    ColumnLayout {
                        width: creditsScroll.availableWidth; spacing: Style.Theme.spaceSm
                        Label { text: card.entry.title; textFormat: Text.PlainText; font.pixelSize: Style.Theme.subtitleSize; font.italic: card.entry.rank === "species" || card.entry.rank === "genus"; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { visible: !!card.photo.subject; text: "Na fotografia: " + (card.photo.subject || ""); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { objectName: "exploreCardCreditAuthor"; text: "Autoria: " + (card.photo.author || "Não documentada") + "\n" + (card.photo.title || "Título não documentado"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { objectName: "exploreCardCreditLicense"; text: "Licença declarada: " + (card.photo.license || "Não informada") + "\nVersão: " + (credits.version || "Não comprovada"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { visible: !!card.photo.sourceCredit || !!card.photo.attributionNotice; text: [card.photo.sourceCredit || "", card.photo.attributionNotice || ""].filter(value => !!value).join("\n"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { text: "Origem: " + (credits.originalUrl || "Não documentada"); textFormat: Text.PlainText; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                        TextAction { objectName: "exploreCardOriginalLink"; visible: !!credits.originalUrl; text: "Fotografia original ↗"; Accessible.description: credits.originalUrl; onClicked: Qt.openUrlExternally(credits.originalUrl) }
                        TextAction { objectName: "exploreCardObservationLink"; visible: !!credits.attribution.source && credits.attribution.source !== credits.originalUrl; text: "Observação de origem ↗"; Accessible.description: credits.attribution.source || ""; onClicked: Qt.openUrlExternally(credits.attribution.source) }
                        TextAction { objectName: "exploreCardLicenseLink"; visible: !!credits.licenseUrl; text: credits.version ? "Termos da licença ↗" : "Termos informados (versão não comprovada) ↗"; Accessible.description: credits.licenseUrl; onClicked: Qt.openUrlExternally(credits.licenseUrl) }
                        Label { objectName: "exploreCardCreditHistory"; text: "Modificações históricas registradas\n" + (card.photo.modifications || "Não documentadas."); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { objectName: "exploreCardCreditPresentation"; text: (card.playSelection ? "Apresentação atual na seleção do Jogar" : "Apresentação atual no Explorar") + "\nImagem inteira, centralizada e proporcional, sem recorte, distorção ou ampliação além dos pixels armazenados. Registros históricos podem descrever apresentações anteriores ou de outras telas; permanecem preservados."; textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { objectName: "exploreCardCreditNotices"; text: (card.photo.identificationNote || card.photo.pendingReason || "Identificação indicada pela fonte; consulte os limites do registro.") + "\n" + (card.photo.scientificReview || "") + "\n" + (card.entry.introductionReviewNotice || "") + "\n" + (card.entry.rankNote || "") + "\n" + (card.entry.placementNotice || "") + (!credits.version && card.photo.image ? "\nVersão da licença não comprovada." : "") + ((credits.attribution.rights_issues || []).length ? "\nPermissões com pendências documentadas." : "") + ((credits.attribution.scientific_source_issues || []).length ? "\nIdentificação na origem com pendência de revisão." : "") + "\nCréditos não implicam endosso. As pendências de distribuição e revisão permanecem vigentes."; textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Label { text: "Fonte científica\n" + (card.educationalNote.source_title || "Sem descrição educativa elegível para este cartão."); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        TextAction { objectName: "exploreCardScientificLink"; visible: !!card.educationalNote.source_url; text: "Referência do conteúdo educativo ↗"; Accessible.description: card.educationalNote.source_url || ""; onClicked: Qt.openUrlExternally(card.educationalNote.source_url) }
                        TextAction { objectName: "exploreCardPlacementLink"; visible: !!card.entry.sourceUrl; text: "Fonte do enquadramento ↗"; Accessible.description: card.entry.sourceUrl || ""; onClicked: Qt.openUrlExternally(card.entry.sourceUrl) }
                    }
                }
            }
        }
    }
}
