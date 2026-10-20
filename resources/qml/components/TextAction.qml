pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import "../style" as Style
Button {
    id: control
    implicitHeight: Style.Theme.actionHeight
    implicitWidth: label.implicitWidth + 24
    leftPadding: Style.Theme.spaceMd; rightPadding: Style.Theme.spaceMd; topPadding: 8; bottomPadding: 8
    hoverEnabled: true
    opacity: 1
    activeFocusOnTab: true
    font.family: Style.Theme.fontFamily
    font.pixelSize: Style.Theme.captionSize
    Accessible.name: text
    background: Rectangle {
        objectName: "textActionBackground"
        radius: Style.Theme.controlRadius
        color: !control.enabled ? Style.Theme.disabledSurface : control.down ? Style.Theme.primaryPressed : (control.hovered || control.visualFocus ? Style.Theme.surfaceHover : "transparent")
        border.color: control.visualFocus ? Style.Theme.focus : "transparent"
        border.width: control.visualFocus ? Style.Theme.focusWidth : 0
        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
    }
    contentItem: Text {
        id: label
        text: control.text; font: control.font; color: control.enabled ? Style.Theme.text : Style.Theme.textDisabled
        textFormat: Text.PlainText; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
    }
}
