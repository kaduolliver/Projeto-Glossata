pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

Item {
    id: view
    objectName: "homeCarousel"
    required property var app
    required property var backend
    property var slides: backend.homeSlides
    property int currentIndex: 0
    readonly property var slide: slides.length ? slides[currentIndex] : ({})
    readonly property var photo: slide.photo || ({})
    readonly property bool photoReady: heroPhoto.ready
    readonly property int transitionDuration: Style.Theme.durationPage
    implicitHeight: 650
    function selectSlide(index) {
        if (slides.length) currentIndex = (index + slides.length) % slides.length
    }
    onSlidesChanged: currentIndex = 0
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
            FeatheredPhoto {
                id: heroPhoto
                objectName: "homeHeroPhoto"
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: Style.Theme.borderWidth
                width: parent.width * 0.76
                cornerRadius: Style.Theme.cardRadius
                source: view.photo.image || ""
                focusX: view.photo.focusX === undefined ? 0.5 : view.photo.focusX
                focusY: view.photo.focusY === undefined ? 0.5 : view.photo.focusY
                opacity: ready ? 1 : 0
                Behavior on opacity { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationPage } }
                Accessible.role: Accessible.Graphic
                Accessible.name: view.photo.subject || ""
                Accessible.description: "Identificação fornecida pela fonte; consulte os créditos."
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
            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Style.Theme.spaceLg
                width: parent.width * 0.46
                height: caption.implicitHeight + Style.Theme.spaceLg * 2
                radius: Style.Theme.controlRadius
                color: Style.Theme.surface
                ColumnLayout {
                    id: caption
                    anchors.fill: parent
                    anchors.margins: Style.Theme.spaceLg
                    spacing: Style.Theme.spaceXs
                    Label {
                        Layout.fillWidth: true
                        text: view.photo.subject || "Fotografias indisponíveis"
                        color: Style.Theme.text
                        font: Style.Theme.scientificFont
                        wrapMode: Text.WordWrap
                    }
                    Label {
                        Layout.fillWidth: true
                        text: view.slides.length ? view.slide.title + " · " + view.slide.excerpt : "A exploração e os modos de jogo continuam disponíveis."
                        color: Style.Theme.textMuted
                        font.pixelSize: Style.Theme.captionSize
                        wrapMode: Text.WordWrap
                    }
                    TextAction {
                        id: creditButton
                        objectName: "homeCreditsButton"
                        Layout.fillWidth: true
                        visible: view.slides.length > 0
                        text: (view.photo.author || "") + " · " + (view.photo.license || "") + " · Créditos"
                        Accessible.name: "Ver autor, licença, proveniência e fonte educativa de " + (view.photo.subject || "")
                        onClicked: credits.open()
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
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            UiButton {
                objectName: "homePreviousSlide"; text: "‹"; Accessible.name: "Fotografia anterior"
                implicitWidth: 42; secondary: true; enabled: view.slides.length > 1
                onClicked: view.selectSlide(view.currentIndex - 1)
            }
            Repeater {
                model: view.slides
                Button {
                    id: thumbnail
                    required property int index
                    required property var modelData
                    objectName: "homeThumbnail" + index
                    Layout.fillWidth: true
                    Layout.preferredHeight: 66
                    activeFocusOnTab: true
                    hoverEnabled: true
                    Accessible.name: "Fotografia " + (index + 1) + ": " + modelData.photo.subject
                    Accessible.description: view.currentIndex === index ? "Selecionada" : "Mostrar fotografia"
                    background: Rectangle {
                        radius: Style.Theme.controlRadius
                        color: Style.Theme.surfaceRaised
                        border.width: thumbnail.activeFocus || view.currentIndex === thumbnail.index ? Style.Theme.focusWidth : Style.Theme.borderWidth
                        border.color: thumbnail.activeFocus ? Style.Theme.focus : view.currentIndex === thumbnail.index ? Style.Theme.green : Style.Theme.border
                    }
                    contentItem: Image {
                        source: thumbnail.modelData.photo.image
                        fillMode: Image.PreserveAspectCrop
                        clip: true
                        asynchronous: true
                        opacity: thumbnail.hovered || view.currentIndex === thumbnail.index ? 1 : 0.65
                        Behavior on opacity { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationFast } }
                    }
                    onClicked: view.selectSlide(index)
                    ToolTip.visible: hovered
                    ToolTip.text: modelData.photo.subject
                }
            }
            UiButton {
                objectName: "homeNextSlide"; text: "›"; Accessible.name: "Próxima fotografia"
                implicitWidth: 42; secondary: true; enabled: view.slides.length > 1
                onClicked: view.selectSlide(view.currentIndex + 1)
            }
            Label {
                objectName: "homeSlidePosition"
                text: view.slides.length ? (view.currentIndex + 1) + " / " + view.slides.length : "0 / 0"
                color: Style.Theme.textMuted
                font.pixelSize: Style.Theme.captionSize
                Accessible.name: "Posição da fotografia: " + text
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
