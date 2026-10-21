pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
Button {
    id: card
    required property var entry
    property bool wide: false
    property bool encyclopedic: false
    readonly property bool landscape: encyclopedic && entry.kind !== "organism" && width >= 450
    // Presentation of an existing path, without inferring ranks or ancestry.
    readonly property string shortClassification: (entry.pathText || "").split(" › ").slice(0, -1).slice(-2).join(" · ")
    property Flickable photoViewport: null
    property bool photoRequested: false
    readonly property real photoFocusX: validFocus(photo.focusX)
    readonly property real photoFocusY: validFocus(photo.focusY)
    function validFocus(value) { return typeof value === "number" && isFinite(value) && value >= 0 && value <= 1 ? value : 0.5 }
    function locatePhotoViewport() {
        if (!encyclopedic) return
        let ancestor = parent
        while (ancestor && !(ancestor instanceof Flickable)) ancestor = ancestor.parent
        photoViewport = ancestor
        updatePhotoRequest()
    }
    function updatePhotoRequest() {
        if (!encyclopedic) return
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
    function schedulePhotoCheck() { if (encyclopedic && photoCheck) photoCheck.restart() }
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
    implicitHeight: encyclopedic ? modernBody.implicitHeight + topPadding + bottomPadding : wide ? 132 : 185
    padding: encyclopedic ? 10 : 20; hoverEnabled: true
    activeFocusOnTab: true
    Accessible.name: encyclopedic ? entry.title + ", " + (entry.kind === "organism" ? "organismo cadastrado" : entry.rankLabel + ", " + entry.count + (entry.count === 1 ? " organismo no catálogo" : " organismos no catálogo")) : entry.title + ", " + entry.count + (entry.count === 1 ? " espécie" : " espécies")
    Accessible.description: (entry.pathText || entry.description || "") + ". " + photoCredit
    ToolTip {
        visible: !card.encyclopedic && (card.hovered || card.visualFocus)
        delay: 600
        timeout: -1
        width: Math.min(360, card.width)
        contentItem: Text { text: card.photoCredit; textFormat: Text.PlainText; wrapMode: Text.WordWrap; color: "#EDF4EE"; font.pixelSize: 12 }
        background: Rectangle { color: "#203A43"; radius: 8; border.color: "#4B666C" }
    }
    background: Rectangle {
        radius: 17; color: card.encyclopedic ? (card.down ? Style.Theme.primaryPressed : card.hovered ? Style.Theme.surfaceHover : Style.Theme.surface) : card.hovered ? "#1C3439" : "#142B33"
        Behavior on color { enabled: card.encyclopedic && Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
        FeatheredPhoto {
            id: photograph
            visible: !card.encyclopedic
            objectName: "explorationPhoto"
            source: !card.encyclopedic && card.visible ? card.photo.image || "" : ""
            width: parent.width * 0.42
            height: parent.height
            anchors.right: parent.right
            cornerRadius: parent.radius
            focusX: card.photo.focusX === undefined ? 0.5 : card.photo.focusX
            focusY: card.photo.focusY === undefined ? 0.5 : card.photo.focusY
        }
        Rectangle { anchors.fill: parent; radius: parent.radius; color: "transparent"; border.color: card.encyclopedic ? (card.visualFocus ? Style.Theme.focus : Style.Theme.border) : card.activeFocus ? card.accent : (card.hovered ? "#526565" : "#2D444A"); border.width: (card.encyclopedic ? card.visualFocus : card.activeFocus) ? 2 : 1 }
    }
    contentItem: Item {
        KingdomArtwork { visible: !card.encyclopedic && !card.wide && !photograph.ready; kingdom: card.entry.kingdom || "Animalia"; width: 96; height: 96; anchors.right: parent.right; anchors.top: parent.top; opacity: 0.7 }
        LifeArtwork { visible: !card.encyclopedic && card.wide && !photograph.ready; width: 250; height: 115; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; opacity: 0.6 }
        ColumnLayout {
            visible: !card.encyclopedic
            anchors.left: parent.left; anchors.bottom: parent.bottom
            width: card.width * 0.60 - card.leftPadding - 8; spacing: 6
            Label { text: card.wide ? "JOGO MISTO · GÊNEROS" : card.entry.rankLabel; color: card.accent; font.pixelSize: 9; font.letterSpacing: 1; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.entry.title; color: "#E8F1E9"; font.pixelSize: card.wide ? 24 : 21; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { text: card.entry.count === 0 ? "Ainda sem organismos no catálogo" : card.entry.count + (card.entry.count === 1 ? " cadastrada · " : " cadastradas · ") + (card.wide ? card.entry.playableCount + (card.entry.playableCount === 1 ? " disponível neste modo" : " disponíveis neste modo") : (card.entry.supportingCount || 0) > 0 ? card.entry.supportingCount + " com imagem de apoio" : (card.entry.visualCount || 0) + " com foto aprovada"); color: "#91ABB0"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        GridLayout {
            id: modernBody
            visible: card.encyclopedic
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
                    // A focal point is not a reviewed crop boundary. Fit the
                    // entire raster, using focus only within the free margins.
                    readonly property real fittedScale: sourceSize.width > 0 && sourceSize.height > 0 ? Math.min(parent.width / sourceSize.width, parent.height / sourceSize.height) : 0
                    width: sourceSize.width * fittedScale; height: sourceSize.height * fittedScale
                    x: Math.max(0, Math.min(parent.width - width, parent.width / 2 - card.photoFocusX * width))
                    y: Math.max(0, Math.min(parent.height - height, parent.height / 2 - card.photoFocusY * height))
                    source: card.encyclopedic && card.photoRequested ? card.photo.image || "" : ""
                    fillMode: Image.PreserveAspectFit; asynchronous: true; smooth: true; mipmap: true
                }
                Label { visible: modernPhoto.status !== Image.Ready; anchors.centerIn: parent; width: parent.width - 20; text: modernPhoto.status === Image.Loading ? "Carregando fotografia…" : "Fotografia ainda indisponível"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter }
            }
            ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: Style.Theme.spaceXs
            Label { text: card.entry.kind === "organism" ? "ORGANISMO · " + (card.entry.rankLabel || "") : card.entry.rankLabel || ""; textFormat: Text.PlainText; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 0.6; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.topMargin: 2 }
            Label { objectName: "encyclopediaCardTitle"; text: card.entry.title; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: 18; font.italic: card.entry.rank === "species" || card.entry.rank === "genus"; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.entry.commonName; text: card.entry.commonName || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { objectName: "encyclopediaCardCount"; text: card.entry.kind === "organism" ? card.shortClassification : card.entry.count === 0 ? "Sem organismos no catálogo" : card.entry.count + (card.entry.count === 1 ? " organismo no catálogo" : " organismos no catálogo"); textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.photo.subject && card.entry.kind !== "organism"; text: "Representante: " + (card.photo.subject || ""); textFormat: Text.PlainText; font.italic: true; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { objectName: "encyclopediaCardAttribution"; text: card.photo.image ? (card.photo.author || "Autoria não informada") + " · " + (card.photo.license || "Licença não informada") : card.photo.pendingReason || "Fotografia adequada ainda não disponível."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: 11; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Item { Layout.fillHeight: true; Layout.minimumHeight: 0 }
            RowLayout {
                Layout.fillWidth: true; Layout.topMargin: 0
                Label { text: card.entry.kind === "organism" ? "Conhecer organismo" : "Explorar grupo"; color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize }
                Item { Layout.fillWidth: true }
                UiIcon { name: "arrow"; ink: Style.Theme.green; implicitWidth: 16; implicitHeight: 16 }
            }
            }
        }
    }
}
