pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
    id: card
    required property var entry
    property bool wide: false
    property color accent: entry.kingdom === "Plantae" ? "#7DA88A" : entry.kingdom === "Fungi" ? "#C29580" : entry.kingdom === "Protista" ? "#86A9C9" : entry.kingdom === "Monera" ? "#7BBABE" : "#D1B17B"
    implicitHeight: wide ? 132 : 185
    padding: 20; hoverEnabled: true
    Accessible.name: entry.title + ", " + entry.count + " espécies"
    background: Rectangle { radius: 17; color: card.hovered ? "#1C3439" : "#142B33"; border.color: card.activeFocus ? card.accent : (card.hovered ? "#526565" : "#2D444A"); border.width: card.activeFocus ? 2 : 1 }
    contentItem: Item {
        KingdomArtwork { visible: !card.wide; kingdom: card.entry.kingdom || "Animalia"; width: 96; height: 96; anchors.right: parent.right; anchors.top: parent.top; opacity: 0.7 }
        LifeArtwork { visible: card.wide; width: 250; height: 115; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; opacity: 0.6 }
        ColumnLayout {
            anchors.left: parent.left; anchors.bottom: parent.bottom
            width: card.wide ? parent.width * 0.65 : parent.width; spacing: 6
            Label { text: card.entry.rankLabel || "JOGO MISTO"; color: card.accent; font.pixelSize: 9; font.letterSpacing: 1 }
            Label { text: card.entry.title; color: "#E8F1E9"; font.pixelSize: card.wide ? 24 : 21; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.entry.count + " espécies disponíveis"; color: "#91ABB0"; font.pixelSize: 12 }
        }
    }
}
