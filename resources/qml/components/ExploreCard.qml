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
    readonly property var photo: entry.photo || ({})
    readonly property string photoCredit: photo.image ? (photo.kind === "micrograph" ? "Micrografia: " : "Fotografia: ") + photo.subject + ". " + photo.author + " · " + photo.license + ". " + (photo.identificationNote || "Representante do grupo; identificação indicada pela fonte.") + " Créditos completos em Perfis e ajustes → Créditos e transparência." : "Ilustração temporária. " + (photo.pendingReason || "Fotografia adequada ainda não disponível.")
    property color accent: entry.kingdom === "Plantae" || entry.kingdom === "Archaeplastida" ? "#7DA88A" : entry.kingdom === "Fungi" ? "#C29580" : entry.kingdom === "Eukaryota" || entry.kingdom === "SAR" ? "#86A9C9" : entry.kingdom === "Bacteria" || entry.kingdom === "Archaea" ? "#7BBABE" : "#D1B17B"
    implicitHeight: encyclopedic ? modernBody.implicitHeight + topPadding + bottomPadding : wide ? 132 : 185
    padding: encyclopedic ? Style.Theme.spaceMd : 20; hoverEnabled: true
    activeFocusOnTab: true
    Accessible.name: encyclopedic ? entry.title + ", " + (entry.kind === "organism" ? "organismo cadastrado" : entry.rankLabel + ", " + entry.count + " organismos no catálogo") : entry.title + ", " + entry.count + " espécies"
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
            source: card.photo.image || ""
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
            Label { text: card.entry.count === 0 ? "Ainda sem organismos no catálogo" : card.entry.count + " cadastradas · " + (card.wide ? card.entry.playableCount + " disponíveis neste modo" : (card.entry.supportingCount || 0) > 0 ? card.entry.supportingCount + " com imagem de apoio" : (card.entry.visualCount || 0) + " com foto aprovada"); color: "#91ABB0"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        ColumnLayout {
            id: modernBody
            visible: card.encyclopedic
            anchors.fill: parent
            spacing: Style.Theme.spaceXs
            Rectangle {
                Layout.fillWidth: true; implicitHeight: Math.max(card.entry.kind === "organism" ? 148 : 126, Math.min(220, card.availableWidth * 0.55))
                color: Style.Theme.surfaceRaised
                Image {
                    id: modernPhoto
                    objectName: "encyclopediaCardPhoto"
                    anchors.fill: parent
                    source: card.encyclopedic ? card.photo.image || "" : ""
                    fillMode: Image.PreserveAspectCrop; asynchronous: true; smooth: true; mipmap: true
                }
                Label { visible: modernPhoto.status !== Image.Ready; anchors.centerIn: parent; width: parent.width - 20; text: modernPhoto.status === Image.Loading ? "Carregando fotografia…" : "Fotografia ainda indisponível"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter }
            }
            Label { text: card.entry.kind === "organism" ? "ORGANISMO · " + (card.entry.rankLabel || "") : card.entry.rankLabel || ""; textFormat: Text.PlainText; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 0.6; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.topMargin: 4 }
            Label { objectName: "encyclopediaCardTitle"; text: card.entry.title; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: card.entry.kind === "organism" ? 18 : Style.Theme.subtitleSize; font.italic: card.entry.rank === "species" || card.entry.rank === "genus"; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.entry.commonName; text: card.entry.commonName || ""; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.entry.kind === "organism" ? card.entry.pathText || "" : card.entry.count === 0 ? "Sem organismos no catálogo" : card.entry.count + " organismos no catálogo"; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { visible: !!card.photo.subject && card.entry.kind !== "organism"; text: "Na foto: " + (card.photo.subject || ""); textFormat: Text.PlainText; font.italic: true; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { text: card.photo.image ? (card.photo.author || "Autoria não informada") + " · " + (card.photo.license || "Licença não informada") : card.photo.pendingReason || "Fotografia adequada ainda não disponível."; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Item { Layout.fillHeight: true; Layout.minimumHeight: 0 }
            RowLayout {
                Layout.fillWidth: true; Layout.topMargin: 4
                Label { text: card.entry.kind === "organism" ? "Ver na taxonomia" : "Explorar grupo"; color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize }
                Item { Layout.fillWidth: true }
                UiIcon { name: "arrow"; ink: Style.Theme.green; implicitWidth: 16; implicitHeight: 16 }
            }
        }
    }
}
