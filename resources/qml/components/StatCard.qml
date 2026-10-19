import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style

Rectangle {
    id: card
    property string title: ""
    property string value: "0"
    property string iconName: "chart"
    property color accent: Style.Theme.green
    implicitHeight: 118
    implicitWidth: 200
    radius: Style.Theme.cardRadius
    color: Style.Theme.surface
    border.color: Style.Theme.border
    RowLayout {
        anchors.fill: parent
        anchors.margins: Style.Theme.spaceLg
        spacing: Style.Theme.spaceMd
        Rectangle {
            implicitWidth: 42; implicitHeight: 42; radius: 14
            color: Qt.rgba(card.accent.r, card.accent.g, card.accent.b, 0.13)
            UiIcon { anchors.centerIn: parent; name: card.iconName; ink: card.accent; implicitWidth: 23; implicitHeight: 23 }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceXs
            Label { text: card.title; color: Style.Theme.textMuted; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.value; color: Style.Theme.text; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.metricSize; font.weight: Font.DemiBold; fontSizeMode: Text.Fit; minimumPixelSize: Style.Theme.bodySize; Layout.fillWidth: true }
        }
    }
}
