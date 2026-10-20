import QtQuick
import QtQuick.Controls
import "../style" as Style

Button {
    id: control
    property string iconName: ""
    property bool secondary: false
    property bool danger: false
    property bool compact: false
    implicitHeight: compact ? Style.Theme.compactHeight : Style.Theme.controlHeight
    implicitWidth: Math.max(110, caption.implicitWidth + (iconName ? 66 : 44))
    padding: Style.Theme.spaceMd
    hoverEnabled: true
    opacity: 1
    activeFocusOnTab: true
    font.family: Style.Theme.fontFamily
    font.pixelSize: Style.Theme.bodySize
    font.weight: Font.DemiBold
    Accessible.name: text
    background: Rectangle {
        radius: Style.Theme.controlRadius
        border.width: control.visualFocus ? Style.Theme.focusWidth : Style.Theme.borderWidth
        border.color: control.visualFocus ? Style.Theme.focus : Style.Theme.border
        color: !control.enabled ? Style.Theme.disabledSurface
             : control.secondary ? (control.down ? Style.Theme.surface : control.hovered ? Style.Theme.surfaceHover : Style.Theme.surfaceRaised)
             : control.danger ? (control.down ? Style.Theme.dangerPressed : control.hovered ? Style.Theme.dangerHover : Style.Theme.danger)
             : control.down ? Style.Theme.primaryPressed : control.hovered ? Style.Theme.primaryHover : Style.Theme.primary
        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
    }
    contentItem: Item {
        implicitWidth: caption.implicitWidth + (control.iconName ? 26 : 0)
        implicitHeight: 22
        Row {
            anchors.centerIn: parent
            spacing: Style.Theme.spaceSm
            UiIcon { visible: control.iconName !== ""; name: control.iconName; ink: control.enabled ? Style.Theme.text : Style.Theme.textDisabled; implicitWidth: 18; implicitHeight: 18; anchors.verticalCenter: parent.verticalCenter }
            Text { id: caption; text: control.text; font: control.font; color: control.enabled ? Style.Theme.text : Style.Theme.textDisabled; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
