import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style

Rectangle {
    id: card
    property string title: ""
    property string value: "0"
    property string description: ""
    property string iconName: "chart"
    property real iconSize: 23
    property real iconContainerSize: 42
    property color accent: Style.Theme.green
    implicitHeight: description.length ? Math.max(154, labels.implicitHeight + 2 * Style.Theme.spaceLg) : 118
    implicitWidth: 200
    radius: Style.Theme.cardRadius
    color: Style.Theme.surface
    border.color: Style.Theme.border
    Accessible.role: Accessible.StaticText
    Accessible.name: title + ": " + value + (description.length ? ". " + description : "")
    RowLayout {
        anchors.fill: parent
        anchors.margins: Style.Theme.spaceLg
        spacing: Style.Theme.spaceMd
        Rectangle {
            objectName: "statCardIconContainer"
            implicitWidth: card.iconContainerSize; implicitHeight: card.iconContainerSize
            radius: card.iconContainerSize / 3
            Layout.alignment: Qt.AlignVCenter
            color: Qt.rgba(card.accent.r, card.accent.g, card.accent.b, 0.13)
            UiIcon { objectName: "statCardIcon"; anchors.centerIn: parent; name: card.iconName; ink: card.accent; implicitWidth: card.iconSize; implicitHeight: card.iconSize }
        }
        ColumnLayout {
            id: labels
            Layout.fillWidth: true
            spacing: Style.Theme.spaceXs
            Label { text: card.title; color: Style.Theme.textMuted; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: card.value; color: Style.Theme.text; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.metricSize; font.weight: Font.DemiBold; fontSizeMode: Text.Fit; minimumPixelSize: Style.Theme.bodySize; Layout.fillWidth: true }
            Label { visible: !!text; text: card.description; color: Style.Theme.textMuted; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.captionSize; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
}
