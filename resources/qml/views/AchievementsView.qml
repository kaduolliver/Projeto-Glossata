pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: 570
    ColumnLayout {
        anchors.fill: parent; spacing: 20
        Label { text: "Conquistas"; color: "#EFF7F0"; font.pixelSize: 30; font.weight: Font.DemiBold }
        Label { text: "Pequenos desafios para grandes descobertas. Progresso nesta partida."; color: "#8EAAB1"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.bottomMargin: 8 }
        GridLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; columns: 3; columnSpacing: 18; rowSpacing: 18
            Repeater {
                model: view.backend.achievements
                delegate: Rectangle {
                    id: badge
                    required property var modelData
                    required property int index
                    readonly property color accent: modelData.unlocked ? ["#78D677", "#ECAD69", "#6BB7DD"][index % 3] : "#617A87"
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 208
                    radius: 16; color: modelData.unlocked ? "#133329" : "#132832"; border.color: modelData.unlocked ? "#315A3E" : "#243E49"
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 20; spacing: 10
                        Item { Layout.fillHeight: true }
                        Rectangle {
                            implicitWidth: 62; implicitHeight: 62; radius: 21; color: badge.modelData.unlocked ? "#23523A" : "#1D333E"; Layout.alignment: Qt.AlignHCenter
                            UiIcon { anchors.centerIn: parent; name: badge.modelData.icon; ink: badge.accent; implicitWidth: 36; implicitHeight: 36 }
                        }
                        Label { text: badge.modelData.title; color: badge.modelData.unlocked ? "#E5F2DF" : "#A5BBC2"; font.pixelSize: 15; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        Label { text: badge.modelData.description; color: "#73959A"; font.pixelSize: 11; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
                        Item { Layout.fillHeight: true }
                        RowLayout {
                            Layout.fillWidth: true
                            UiIcon { name: badge.modelData.unlocked ? "check" : "target"; ink: badge.accent; implicitWidth: 14; implicitHeight: 14 }
                            Label { text: badge.modelData.unlocked ? "Conquistado" : badge.modelData.progress + " / " + badge.modelData.target; color: badge.accent; font.pixelSize: 10; Layout.fillWidth: true }
                        }
                        Rectangle { Layout.fillWidth: true; implicitHeight: 4; radius: 2; color: "#28414A"; Rectangle { implicitHeight: 4; radius: 2; width: parent.width * badge.modelData.progress / badge.modelData.target; color: badge.accent } }
                    }
                }
            }
        }
        Label { text: "Uma nova partida é um novo voo: os desafios recomeçam junto com você."; color: "#63858F"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
