import QtQuick
import QtQuick.Controls

Button {
    id: control
    property string iconName: "home"
    property bool selected: false
    implicitHeight: 46
    hoverEnabled: true
    padding: 14
    background: Rectangle {
        radius: 9
        color: control.selected ? "#14563F" : (control.hovered ? "#20333C" : "transparent")
        border.color: control.activeFocus ? "#6BE7A3" : "transparent"
        Rectangle { implicitWidth: 3; implicitHeight: 22; radius: 2; color: "#4ADE8C"; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; visible: control.selected }
    }
    contentItem: Row {
        spacing: 13
        UiIcon { name: control.iconName; ink: control.selected ? "#71E6A5" : "#7F9BA6"; implicitWidth: 19; implicitHeight: 19; anchors.verticalCenter: parent.verticalCenter }
        Text { text: control.text; color: control.selected ? "#F5FFFA" : "#A9BEC4"; font.pixelSize: 13; font.weight: control.selected ? Font.DemiBold : Font.Normal; anchors.verticalCenter: parent.verticalCenter }
    }
}
