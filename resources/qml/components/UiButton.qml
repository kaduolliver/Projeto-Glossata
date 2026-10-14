import QtQuick
import QtQuick.Controls

Button {
    id: control
    property string iconName: ""
    property bool secondary: false
    property bool danger: false
    property bool compact: false
    implicitHeight: compact ? 40 : 50
    implicitWidth: Math.max(110, caption.implicitWidth + (iconName ? 66 : 44))
    padding: 12
    hoverEnabled: true
    opacity: enabled ? 1 : 0.4
    font.pixelSize: 14
    font.weight: Font.DemiBold
    Accessible.name: text
    background: Rectangle {
        radius: 12
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? "#BCF6D9" : (control.secondary ? "#2A424C" : (control.danger ? "#EB8A7A" : "#35CF80"))
        gradient: Gradient {
            GradientStop { position: 0; color: control.secondary ? (control.hovered ? "#243C47" : "#1B303B") : (control.danger ? "#EE7469" : (control.hovered ? "#43D88A" : "#29C878")) }
            GradientStop { position: 1; color: control.secondary ? "#142731" : (control.danger ? "#CB4C47" : "#0DA662") }
        }
        scale: control.down ? 0.98 : 1
        Behavior on scale { NumberAnimation { duration: 100 } }
    }
    contentItem: Item {
        implicitWidth: caption.implicitWidth + (control.iconName ? 26 : 0)
        implicitHeight: 22
        Row {
            anchors.centerIn: parent
            spacing: 10
            UiIcon { visible: control.iconName !== ""; name: control.iconName; ink: "#F4FAF7"; implicitWidth: 18; implicitHeight: 18; anchors.verticalCenter: parent.verticalCenter }
            Text { id: caption; text: control.text; font: control.font; color: "#F4FAF7"; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
