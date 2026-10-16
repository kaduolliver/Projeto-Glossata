pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
    id: card
    required property var entry
    property bool wide: false
    readonly property var photo: entry.photo || ({})
    readonly property string photoCredit: photo.image ? (photo.kind === "micrograph" ? "Micrografia: " : "Fotografia: ") + photo.subject + ". " + photo.author + " · " + photo.license + ". " + (photo.identificationNote || "Representante do grupo; identificação indicada pela fonte.") + " Créditos completos em Perfis e ajustes → Créditos e transparência." : "Ilustração temporária. " + (photo.pendingReason || "Fotografia adequada ainda não disponível.")
    property color accent: entry.kingdom === "Plantae" || entry.kingdom === "Archaeplastida" ? "#7DA88A" : entry.kingdom === "Fungi" ? "#C29580" : entry.kingdom === "Eukaryota" || entry.kingdom === "SAR" ? "#86A9C9" : entry.kingdom === "Bacteria" || entry.kingdom === "Archaea" ? "#7BBABE" : "#D1B17B"
    implicitHeight: wide ? 132 : 185
    padding: 20; hoverEnabled: true
    Accessible.name: entry.title + ", " + entry.count + " espécies"
    Accessible.description: (entry.pathText || entry.description || "") + ". " + photoCredit
    ToolTip {
        visible: card.hovered || card.visualFocus
        delay: 600
        timeout: -1
        width: Math.min(360, card.width)
        contentItem: Text { text: card.photoCredit; textFormat: Text.PlainText; wrapMode: Text.WordWrap; color: "#EDF4EE"; font.pixelSize: 12 }
        background: Rectangle { color: "#203A43"; radius: 8; border.color: "#4B666C" }
    }
    background: Rectangle {
        radius: 17; color: card.hovered ? "#1C3439" : "#142B33"
        FeatheredPhoto {
            id: photograph
            objectName: "explorationPhoto"
            source: card.photo.image || ""
            width: parent.width * 0.42
            height: parent.height
            anchors.right: parent.right
            cornerRadius: parent.radius
            focusX: card.photo.focusX === undefined ? 0.5 : card.photo.focusX
            focusY: card.photo.focusY === undefined ? 0.5 : card.photo.focusY
        }
        Rectangle { anchors.fill: parent; radius: parent.radius; color: "transparent"; border.color: card.activeFocus ? card.accent : (card.hovered ? "#526565" : "#2D444A"); border.width: card.activeFocus ? 2 : 1 }
    }
    contentItem: Item {
        KingdomArtwork { visible: !card.wide && !photograph.ready; kingdom: card.entry.kingdom || "Animalia"; width: 96; height: 96; anchors.right: parent.right; anchors.top: parent.top; opacity: 0.7 }
        LifeArtwork { visible: card.wide && !photograph.ready; width: 250; height: 115; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; opacity: 0.6 }
        ColumnLayout {
            anchors.left: parent.left; anchors.bottom: parent.bottom
            width: card.width * 0.60 - card.leftPadding - 8; spacing: 6
            Label { text: card.wide ? "JOGO MISTO · GÊNEROS" : card.entry.rankLabel; color: card.accent; font.pixelSize: 9; font.letterSpacing: 1; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.entry.title; color: "#E8F1E9"; font.pixelSize: card.wide ? 24 : 21; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { text: card.entry.count === 0 ? "Ainda sem organismos no catálogo" : card.entry.count + " cadastradas · " + (card.wide ? card.entry.playableCount + " disponíveis neste modo" : (card.entry.supportingCount || 0) > 0 ? card.entry.supportingCount + " com imagem de apoio" : (card.entry.visualCount || 0) + " com foto aprovada"); color: "#91ABB0"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
}
