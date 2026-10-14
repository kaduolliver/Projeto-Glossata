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
        anchors.fill: parent
        spacing: 20
        Label { text: "Escolha seu próximo voo"; color: "#F0F7F3"; font.pixelSize: 30; font.weight: Font.DemiBold }
        Label { text: "Explore o catálogo completo ou conheça uma família de perto."; color: "#92ADB4"; font.pixelSize: 14; Layout.bottomMargin: 8 }
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            columnSpacing: 20
            rowSpacing: 20
            Repeater {
                model: view.backend.categories
                delegate: Button {
                    id: category
                    required property var modelData
                    required property int index
                    objectName: "categoryButton" + index
                    readonly property color accent: index === 1 ? "#91A6DA" : (index === 2 ? "#D5B886" : "#69D79C")
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.columnSpan: index === 0 ? 2 : 1
                    Layout.minimumHeight: index === 0 ? 210 : 240
                    hoverEnabled: true
                    Accessible.name: modelData.title + ", " + modelData.count + " espécies"
                    background: Rectangle { radius: 18; color: "#122B2B"; border.width: category.hovered || category.activeFocus ? 2 : 1; border.color: category.hovered || category.activeFocus ? category.accent : "#355249" }
                    contentItem: Item {
                        SpeciesArtwork { anchors.fill: parent; anchors.margins: 1; accent: category.accent; moth: category.modelData.id === "Saturniidae"; showCaption: false; butterflyScale: 0.8 }
                        Rectangle {
                            anchors.fill: parent; radius: 18
                            gradient: Gradient { GradientStop { position: 0.15; color: "#00102A25" } GradientStop { position: 1; color: "#F2102425" } }
                        }
                        ColumnLayout {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 24; spacing: 7
                            RowLayout {
                                UiIcon { name: "butterfly"; ink: category.accent; implicitWidth: 26; implicitHeight: 26 }
                                Label { text: category.modelData.title; color: "#F4FAF4"; font.pixelSize: 22; font.weight: Font.DemiBold; Layout.fillWidth: true }
                                UiIcon { name: "arrow"; ink: category.accent; implicitWidth: 21; implicitHeight: 21 }
                            }
                            Label { text: category.modelData.count + " espécies · Partida contínua"; color: "#BCD0C6"; font.pixelSize: 12 }
                        }
                    }
                    onClicked: view.app.beginGame(modelData.id)
                }
            }
        }
        Label { text: "As espécies são embaralhadas a cada ciclo. Você escolhe quando encerrar."; color: "#6F9098"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
