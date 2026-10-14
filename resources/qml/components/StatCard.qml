import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    property string title: ""
    property string value: "0"
    property string iconName: "chart"
    property color accent: "#39CE87"
    implicitHeight: 118
    implicitWidth: 200
    radius: 16
    color: "#122630"
    border.color: "#223C45"
    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        Rectangle {
            implicitWidth: 42; implicitHeight: 42; radius: 14
            color: Qt.rgba(card.accent.r, card.accent.g, card.accent.b, 0.13)
            UiIcon { anchors.centerIn: parent; name: card.iconName; ink: card.accent; implicitWidth: 23; implicitHeight: 23 }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            Label { text: card.title; color: "#91AAB2"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.value; color: "#F4FAF7"; font.pixelSize: 28; font.weight: Font.DemiBold }
        }
    }
}
