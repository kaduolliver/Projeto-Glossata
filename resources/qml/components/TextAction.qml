pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
Button {
    id: control
    implicitHeight: 36
    implicitWidth: label.implicitWidth + 24
    leftPadding: 12; rightPadding: 12; topPadding: 8; bottomPadding: 8
    hoverEnabled: true
    opacity: enabled ? 1 : 0.4
    font.pixelSize: 12
    Accessible.name: text
    background: Rectangle {
        objectName: "textActionBackground"
        radius: 10
        color: control.down ? control.palette.highlight : (control.hovered || control.activeFocus ? control.palette.button : "transparent")
        border.color: control.activeFocus ? control.palette.light : "transparent"
        border.width: control.activeFocus ? 2 : 0
    }
    contentItem: Text {
        id: label
        text: control.text; font: control.font; color: control.palette.text
        textFormat: Text.PlainText; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
    }
}
