pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

FocusScope {
    id: view
    objectName: "homeCarousel"
    required property var app
    required property var backend
    property var slides: backend.homeSlides
    property int currentIndex: 0
    readonly property var slide: slides.length ? slides[currentIndex] : ({})
    readonly property var photo: slide.photo || ({})
    readonly property bool photoReady: (activeBuffer === 0 ? layerA.painted : layerB.painted)
    readonly property int transitionDuration: Style.Theme.homeCrossfadeDuration
    readonly property int slideInterval: Style.Theme.homeSlideInterval
    readonly property bool windowActive: Window.window ? Window.window.active && Window.window.visible
        && Window.window.visibility !== Window.Minimized : false
    readonly property var focusedControl: Window.window ? Window.window.activeFocusItem : null
    readonly property bool keyboardInteracting: activeFocus && focusedControl
        && (focusedControl.focusReason === Qt.TabFocusReason || focusedControl.focusReason === Qt.BacktabFocusReason
            || focusedControl.focusReason === Qt.ShortcutFocusReason || focusedControl.focusReason === Qt.OtherFocusReason)
    readonly property bool pointerInteracting: captionHover.hovered || controlsHover.hovered
        || !!(activeFocus && focusedControl && focusedControl.down)
    readonly property bool canAutoAdvance: visible && windowActive && Style.Theme.motionEnabled
        && slides.length > 1 && !pointerInteracting && !keyboardInteracting && !credits.visible
    readonly property bool autoRunning: advanceTimer.running
    readonly property bool transitioning: crossfade.running
    property int activeBuffer: 0
    property int pendingIndex: -1
    property real blend: 0
    implicitHeight: 650

    function resetSlides() {
        crossfade.stop()
        currentIndex = 0
        pendingIndex = -1
        blend = 0
        activeBuffer = 0
        layerA.dataPhoto = slides.length ? slides[0].photo : ({})
        preloadNext()
    }
    function preloadNext() {
        if (pendingIndex >= 0 || crossfade.running) return
        const next = slides.length > 1 ? slides[(currentIndex + 1) % slides.length].photo : ({})
        if (activeBuffer === 0) layerB.dataPhoto = next
        else layerA.dataPhoto = next
    }
    function selectSlide(index, manual = true) {
        if (manual) restartInterval()
        if (!slides.length || pendingIndex >= 0 || crossfade.running) return
        const selected = (index + slides.length) % slides.length
        if (selected === currentIndex) return
        pendingIndex = selected
        if (activeBuffer === 0) layerB.dataPhoto = slides[selected].photo
        else layerA.dataPhoto = slides[selected].photo
        Qt.callLater(beginTransition)
    }
    function beginTransition() {
        const incoming = activeBuffer === 0 ? layerB : layerA
        if (pendingIndex >= 0 && incoming.failed) { pendingIndex = -1; return }
        if (pendingIndex < 0 || !incoming.painted || crossfade.running) return
        if (transitionDuration === 0) finishTransition()
        else crossfade.start()
    }
    function finishTransition() {
        if (pendingIndex < 0) return
        const completedIndex = pendingIndex
        crossfade.stop()
        activeBuffer = 1 - activeBuffer
        pendingIndex = -1
        blend = 0
        // Keep the outgoing photo's caption/credits until the incoming buffer
        // is fully displayed; commit the visible slide as one completed step.
        currentIndex = completedIndex
        Qt.callLater(preloadNext)
    }
    function restartInterval() {
        if (canAutoAdvance) advanceTimer.restart()
        else advanceTimer.stop()
    }
    onSlidesChanged: Qt.callLater(resetSlides)
    onCanAutoAdvanceChanged: restartInterval()
    Component.onCompleted: { resetSlides(); restartInterval() }
    Timer {
        id: advanceTimer
        objectName: "homeAdvanceTimer"
        interval: view.slideInterval
        repeat: true
        onTriggered: view.selectSlide(view.currentIndex + 1, false)
    }
    NumberAnimation {
        id: crossfade
        target: view
        property: "blend"
        from: 0; to: 1
        duration: view.transitionDuration
        paused: running && (!view.visible || !view.windowActive || credits.visible)
        onFinished: view.finishTransition()
    }
    Connections {
        target: Style.Theme
        function onMotionEnabledChanged() {
            if (!Style.Theme.motionEnabled) view.finishTransition()
        }
    }
    Keys.onLeftPressed: selectSlide(currentIndex - 1)
    Keys.onRightPressed: selectSlide(currentIndex + 1)
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Home) { selectSlide(0); event.accepted = true }
        else if (event.key === Qt.Key_End) { selectSlide(slides.length - 1); event.accepted = true }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Style.Theme.spaceLg
        Rectangle {
            id: hero
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 420
            radius: Style.Theme.cardRadius
            color: Style.Theme.canvas
            border.color: Style.Theme.border
            component SlidePhoto: Item {
                id: buffer
                property var dataPhoto: ({})
                readonly property bool painted: bitmap.status === Image.Ready
                readonly property bool failed: bitmap.status === Image.Error
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: Style.Theme.borderWidth
                width: parent.width * 0.76
                clip: true
                Image {
                    id: bitmap
                    source: buffer.dataPhoto.image || ""
                    asynchronous: true
                    cache: false
                    sourceSize: Qt.size(Math.min(1920, buffer.dataPhoto.width || 1920), Math.min(1920, buffer.dataPhoto.height || 1920))
                    // Native image rendering avoids Canvas texture invalidation
                    // while a preloaded buffer is invisible in the software backend.
                    readonly property real cropScale: implicitWidth > 0 && implicitHeight > 0
                        ? Math.max(buffer.width / implicitWidth, buffer.height / implicitHeight) : 1
                    width: implicitWidth * cropScale
                    height: implicitHeight * cropScale
                    x: Math.max(buffer.width - width, Math.min(0, buffer.width / 2 - (buffer.dataPhoto.focusX === undefined ? 0.5 : buffer.dataPhoto.focusX) * width))
                    y: Math.max(buffer.height - height, Math.min(0, buffer.height / 2 - (buffer.dataPhoto.focusY === undefined ? 0.5 : buffer.dataPhoto.focusY) * height))
                }
                onPaintedChanged: if (painted) Qt.callLater(view.beginTransition)
                onFailedChanged: if (failed) Qt.callLater(view.beginTransition)
            }
            SlidePhoto {
                id: layerA
                objectName: "homeHeroPhoto"
                opacity: view.activeBuffer === 0 ? 1 - view.blend : view.blend
                Accessible.role: Accessible.Graphic
                Accessible.name: view.photo.subject || ""
                Accessible.description: "Identificação fornecida pela fonte; consulte os créditos."
            }
            SlidePhoto {
                id: layerB
                objectName: "homeIncomingPhoto"
                opacity: view.activeBuffer === 1 ? 1 - view.blend : view.blend
            }
            Rectangle {
                anchors.fill: layerA
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: Style.Theme.canvas }
                    GradientStop { position: 0.28; color: Qt.rgba(Style.Theme.canvas.r, Style.Theme.canvas.g, Style.Theme.canvas.b, 0.56) }
                    GradientStop { position: 0.65; color: "transparent" }
                    GradientStop { position: 1; color: "transparent" }
                }
            }
            Canvas {
                objectName: "homeCaptionShade"
                anchors.right: layerA.right
                anchors.bottom: layerA.bottom
                width: layerA.width
                height: Math.min(layerA.height, caption.height + Style.Theme.spaceLg * 6)
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                // A static lower veil, feathered on both sides. Repaint only
                // when its size changes; the photographs crossfade underneath.
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    if (width <= 0 || height <= 0) return
                    const ink = Style.Theme.canvas
                    ctx.save()
                    ctx.scale(width, height)
                    const shade = ctx.createRadialGradient(0.72, 1.05, 0, 0.72, 1.05, 1.05)
                    shade.addColorStop(0, Qt.rgba(ink.r, ink.g, ink.b, 0.96))
                    shade.addColorStop(0.45, Qt.rgba(ink.r, ink.g, ink.b, 0.94))
                    shade.addColorStop(0.72, Qt.rgba(ink.r, ink.g, ink.b, 0.70))
                    shade.addColorStop(1, Qt.rgba(ink.r, ink.g, ink.b, 0))
                    ctx.fillStyle = shade
                    ctx.fillRect(0, 0, 1, 1)
                    ctx.restore()
                    ctx.globalCompositeOperation = "destination-in"
                    const vertical = ctx.createLinearGradient(0, 0, 0, height)
                    vertical.addColorStop(0, "transparent")
                    vertical.addColorStop(0.28, "black")
                    vertical.addColorStop(1, "black")
                    ctx.fillStyle = vertical
                    ctx.fillRect(0, 0, width, height)
                    const horizontal = ctx.createLinearGradient(0, 0, width, 0)
                    horizontal.addColorStop(0, "transparent")
                    horizontal.addColorStop(0.20, "black")
                    horizontal.addColorStop(1, "black")
                    ctx.fillStyle = horizontal
                    ctx.fillRect(0, 0, width, height)
                }
            }
            Canvas {
                anchors.fill: layerA
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                // Only the two corner cutouts are painted, on load/resize.
                // Photographs and crossfade are handled by native Image items.
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    const r = Style.Theme.cardRadius
                    ctx.fillStyle = Style.Theme.canvas
                    ctx.beginPath()
                    ctx.moveTo(width - r, 0); ctx.lineTo(width, 0); ctx.lineTo(width, r)
                    ctx.quadraticCurveTo(width, 0, width - r, 0)
                    ctx.closePath(); ctx.fill()
                    ctx.beginPath()
                    ctx.moveTo(width, height - r); ctx.lineTo(width, height); ctx.lineTo(width - r, height)
                    ctx.quadraticCurveTo(width, height, width, height - r)
                    ctx.closePath(); ctx.fill()
                }
            }
            ColumnLayout {
                anchors.left: parent.left
                anchors.leftMargin: Style.Theme.spaceLg
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width * 0.44
                spacing: Style.Theme.spaceMd
                Label { text: "NATUREZA EM FOCO"; color: Style.Theme.gold; font.pixelSize: Style.Theme.captionSize; font.letterSpacing: 1 }
                Label {
                    Layout.fillWidth: true
                    text: "Descubra a\nbiodiversidade"
                    color: Style.Theme.text
                    font.family: Style.Theme.fontFamily
                    font.pixelSize: Style.Theme.titleSize
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
                Label {
                    Layout.fillWidth: true
                    text: "Explore as linhagens da vida.\nObserve, aprenda e teste seus conhecimentos."
                    color: Style.Theme.textMuted
                    font.pixelSize: Style.Theme.bodySize
                    wrapMode: Text.WordWrap
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: Style.Theme.spaceSm
                    UiButton { objectName: "homeExploreButton"; text: "Explorar"; iconName: "book"; onClicked: view.app.openExplore() }
                    UiButton { objectName: "homePlayButton"; text: view.app.playing ? "Continuar partida" : "Jogar"; iconName: "play"; secondary: true; onClicked: view.app.openPlay() }
                }
                Label {
                    Layout.fillWidth: true
                    text: "Respostas e dicas salvas automaticamente."
                    color: Style.Theme.textMuted
                    font.pixelSize: Style.Theme.captionSize
                    wrapMode: Text.WordWrap
                }
            }
            ColumnLayout {
                id: caption
                objectName: "homePhotoCaption"
                HoverHandler { id: captionHover }
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Style.Theme.spaceLg * 2
                width: parent.width * 0.44
                spacing: Style.Theme.spaceXs
                Label {
                    objectName: "homeCaptionTitle"
                    Layout.fillWidth: true
                    text: view.slides.length ? view.slide.title : "Fotografias indisponíveis"
                    color: Style.Theme.text
                    font.family: Style.Theme.fontFamily
                    font.pixelSize: Style.Theme.subtitleSize
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
                Label {
                    objectName: "homeCaptionDescription"
                    Layout.fillWidth: true
                    text: view.slides.length ? view.slide.excerpt : "A exploração e os modos de jogo continuam disponíveis."
                    color: Style.Theme.text
                    font.pixelSize: Style.Theme.bodySize
                    wrapMode: Text.WordWrap
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Style.Theme.spaceSm
                    Label {
                        objectName: "homeCaptionScientificName"
                        Layout.fillWidth: true
                        text: view.photo.subject || ""
                        color: Style.Theme.textMuted
                        font: Style.Theme.scientificFont
                        wrapMode: Text.WordWrap
                    }
                    TextAction {
                        id: creditButton
                        objectName: "homeCreditsButton"
                        Layout.alignment: Qt.AlignRight
                        visible: view.slides.length > 0
                        text: "Créditos"
                        Accessible.name: "Ver autor, licença, proveniência e fonte educativa de " + (view.photo.subject || "")
                        onClicked: credits.open()
                    }
                }
                Label {
                    Layout.fillWidth: true
                    visible: view.slides.length > 0 && !view.photoReady
                    text: "Fotografia ainda não disponível."
                    color: Style.Theme.textMuted
                    font.pixelSize: Style.Theme.captionSize
                    wrapMode: Text.WordWrap
                }
            }
        }
        RowLayout {
            HoverHandler { id: controlsHover }
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            UiButton {
                objectName: "homePreviousSlide"; text: "‹"; Accessible.name: "Fotografia anterior"
                implicitWidth: 42; compact: true; secondary: true
                enabled: view.slides.length > 1 && view.pendingIndex < 0
                onClicked: view.selectSlide(view.currentIndex - 1)
            }
            Item { Layout.fillWidth: true }
            Repeater {
                model: view.slides
                Button {
                    id: dot
                    required property int index
                    objectName: "homeSlideDot" + index
                    implicitWidth: 32; implicitHeight: 36
                    activeFocusOnTab: true
                    hoverEnabled: true
                    enabled: view.pendingIndex < 0
                    Accessible.name: "Fotografia " + (index + 1) + " de " + view.slides.length + ": " + (view.slides[index] ? view.slides[index].photo.subject : "")
                    Accessible.description: view.currentIndex === index ? "Selecionada" : "Mostrar fotografia"
                    background: Rectangle {
                        color: "transparent"
                        radius: Style.Theme.controlRadius
                        border.color: dot.visualFocus ? Style.Theme.focus : "transparent"
                        border.width: Style.Theme.focusWidth
                    }
                    contentItem: Item {
                        Rectangle {
                            anchors.centerIn: parent
                            width: 8; height: 8; radius: 4
                            color: view.currentIndex === dot.index ? Style.Theme.green : dot.hovered ? Style.Theme.text : Style.Theme.border
                        }
                    }
                    onClicked: view.selectSlide(index)
                }
            }
            Item { Layout.fillWidth: true }
            UiButton {
                objectName: "homeNextSlide"; text: "›"; Accessible.name: "Próxima fotografia"
                implicitWidth: 42; compact: true; secondary: true
                enabled: view.slides.length > 1 && view.pendingIndex < 0
                onClicked: view.selectSlide(view.currentIndex + 1)
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceMd
            StatCard { Layout.fillWidth: true; title: "Espécies para descobrir"; value: String(view.backend.categories[0].count); iconName: "butterfly"; accent: Style.Theme.gold }
            StatCard { Layout.fillWidth: true; title: "Seu ritmo"; value: "Sem pressa"; iconName: "leaf"; accent: Style.Theme.green }
            StatCard { Layout.fillWidth: true; title: "Conhecimento"; value: "A cada descoberta"; iconName: "book"; accent: Style.Theme.textMuted }
        }
    }
    Dialog {
        id: credits
        objectName: "homeCreditsDialog"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(620, parent.width - 40)
        height: Math.min(600, parent.height - 40)
        title: "Fotografia e fonte educativa"
        modal: true
        footer: DialogButtonBox {
            UiButton { text: "Fechar"; DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            onRejected: credits.close()
        }
        onClosed: creditButton.forceActiveFocus(Qt.PopupFocusReason)
        contentItem: ScrollView {
            id: creditScroll
            clip: true
            contentWidth: availableWidth
            ColumnLayout {
                width: creditScroll.availableWidth
                spacing: Style.Theme.spaceMd
                Label {
                    Layout.fillWidth: true
                    text: (view.photo.subject || "") + "\n" + (view.photo.title || "") + "\n" + (view.photo.author || "") + "\n" + (view.photo.license || "")
                    wrapMode: Text.WordWrap; textFormat: Text.PlainText
                }
                TextAction { text: "Fotografia na fonte original ↗"; onClicked: Qt.openUrlExternally(view.photo.photoUrl) }
                TextAction { text: "Termos da licença ↗"; onClicked: Qt.openUrlExternally(view.photo.licenseUrl) }
                Label {
                    Layout.fillWidth: true
                    text: "Arquivo: " + (view.photo.modifications || "") + "\nApresentação: recorte proporcional, gradiente e redimensionamento; arquivo original preservado.\nIdentificação: fornecida pela fonte; revisão especializada não declarada."
                    wrapMode: Text.WordWrap; textFormat: Text.PlainText
                }
                Label {
                    Layout.fillWidth: true
                    text: "Evidência local: " + (view.photo.evidencePath || "") + "\nColeta: " + (view.photo.collectedAt || "") + "\nSHA-256: " + (view.photo.evidenceSha256 || "")
                    wrapMode: Text.WrapAnywhere; textFormat: Text.PlainText
                }
                TextAction {
                    text: "Registro público de proveniência ↗"
                    onClicked: Qt.openUrlExternally("https://github.com/kaduolliver/Projeto-Glossata/blob/main/" + view.photo.evidencePath)
                }
                Label {
                    Layout.fillWidth: true
                    text: "Texto: trecho da introdução de " + (view.slide.title || "") + ", com revisão editorial existente; refere-se ao grupo.\n" + (view.slide.introduction ? view.slide.introduction.source_title : "")
                    wrapMode: Text.WordWrap; textFormat: Text.PlainText
                }
                TextAction { text: "Fonte do texto educativo ↗"; onClicked: Qt.openUrlExternally(view.slide.introduction.source_url) }
                TextAction { text: "Todos os créditos no aplicativo"; onClicked: { credits.close(); view.app.navigate("credits") } }
            }
        }
    }
}
