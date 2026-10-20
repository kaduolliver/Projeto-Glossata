import QtQuick
import QtQuick.Controls
import "../style" as Style

Button {
    id: control
    property string iconName: "home"
    property bool selected: false
    implicitHeight: Style.Theme.navHeight
    activeFocusOnTab: true
    Accessible.name: text
    Accessible.description: selected ? "Selecionado" : ""
    hoverEnabled: true
    padding: Style.Theme.navPadding
    background: Rectangle {
        radius: Style.Theme.controlRadius
        color: !control.enabled ? Style.Theme.disabledSurface : control.selected ? Style.Theme.forest : (control.hovered || control.down ? Style.Theme.surfaceHover : "transparent")
        border.color: control.visualFocus ? Style.Theme.focus : "transparent"
        border.width: control.visualFocus ? Style.Theme.focusWidth : Style.Theme.borderWidth
        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
        Rectangle { implicitWidth: 3; implicitHeight: 22; radius: 2; color: Style.Theme.gold; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; visible: control.selected }
    }
    contentItem: Row {
        spacing: Style.Theme.spaceMd
        UiIcon { name: control.iconName; ink: !control.enabled ? Style.Theme.textDisabled : control.selected ? Style.Theme.gold : Style.Theme.textMuted; implicitWidth: 19; implicitHeight: 19; anchors.verticalCenter: parent.verticalCenter }
        Text { text: control.text; color: !control.enabled ? Style.Theme.textDisabled : control.selected ? Style.Theme.text : Style.Theme.textMuted; font.family: Style.Theme.fontFamily; font.pixelSize: Style.Theme.labelSize; font.weight: control.selected ? Font.DemiBold : Font.Normal; anchors.verticalCenter: parent.verticalCenter }
    }
}
