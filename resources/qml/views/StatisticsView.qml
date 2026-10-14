pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: 600
    ColumnLayout {
        anchors.fill: parent; spacing: 22
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true; spacing: 8
                Label { text: "Suas estatísticas"; color: "#EFF7F1"; font.pixelSize: 30; font.weight: Font.DemiBold }
                Label { text: "Cada descoberta conta. Veja como foi seu voo."; color: "#8CA9B1"; font.pixelSize: 13 }
            }
            Item { Layout.fillWidth: true }
            Rectangle { implicitWidth: 118; implicitHeight: 32; radius: 9; color: "#182E38"; border.color: "#2A444F"; Label { anchors.centerIn: parent; text: "Nesta partida"; color: "#AECDD0"; font.pixelSize: 11 } }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 16
            StatCard { Layout.fillWidth: true; title: "Perguntas respondidas"; value: String(view.backend.answeredCount); iconName: "book"; accent: "#6CABEE" }
            StatCard { Layout.fillWidth: true; title: "Acertos"; value: String(view.backend.score); iconName: "check"; accent: "#45D080" }
            StatCard { Layout.fillWidth: true; title: "Taxa de acerto"; value: view.app.accuracy + "%"; iconName: "target"; accent: "#E7B857" }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; radius: 18; color: "#10242D"; border.color: "#233E48"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 24; spacing: 18
                    Label { text: "Desempenho por família"; color: "#E0EDE6"; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Label { visible: view.backend.categoryStats.length === 0; text: "Seu próximo voo começa com uma pergunta. Jogue para acompanhar seu desempenho aqui."; color: "#73939B"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 13; lineHeight: 1.5 }
                    ListView {
                        id: familyList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 18
                        ScrollBar.vertical: ScrollBar { }
                        model: view.backend.categoryStats
                        delegate: ColumnLayout {
                            id: familyRow
                            required property var modelData
                            required property int index
                            readonly property color accent: ["#3CCD83", "#61A8E8", "#E5B858", "#B590E0"][index % 4]
                            width: familyList.width; spacing: 8
                            RowLayout {
                                Layout.fillWidth: true
                                UiIcon { name: "butterfly"; ink: familyRow.accent; implicitWidth: 18; implicitHeight: 18 }
                                Label { text: familyRow.modelData.family; color: "#A7C0C5"; font.pixelSize: 12; Layout.fillWidth: true }
                                Label { text: familyRow.modelData.accuracy + "%"; color: "#DAEAE1"; font.pixelSize: 12 }
                            }
                            Rectangle {
                                Layout.fillWidth: true; implicitHeight: 7; radius: 4; color: "#203943"
                                Rectangle { implicitHeight: 7; radius: 4; width: parent.width * familyRow.modelData.accuracy / 100; color: familyRow.accent }
                            }
                            Label { text: familyRow.modelData.correct + " acertos em " + familyRow.modelData.answered + " respostas"; color: "#63858F"; font.pixelSize: 10 }
                        }
                    }
                    UiButton { text: view.app.playing ? "Continuar partida" : "Começar uma partida"; iconName: "play"; compact: true; onClicked: view.app.openPlay() }
                }
            }
            Rectangle {
                Layout.preferredWidth: view.width * 0.4; Layout.fillHeight: true; radius: 18; color: "#10242D"; border.color: "#233E48"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 22; spacing: 14
                    RowLayout { UiIcon { name: "trophy"; ink: "#E4BE69"; implicitWidth: 21; implicitHeight: 21 } Label { text: "Ranking local"; color: "#E5EEE6"; font.pixelSize: 17; font.weight: Font.DemiBold } }
                    Label { text: "Última partida de cada jogador"; color: "#72959E"; font.pixelSize: 11 }
                    Label { visible: view.backend.ranking.length === 0; text: "Encerre uma partida para entrar no ranking."; color: "#8FAAB0"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    ListView {
                        id: rankingList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8
                        clip: true
                        ScrollBar.vertical: ScrollBar { }
                        model: view.backend.ranking
                        delegate: Rectangle {
                            id: rankRow
                            required property var modelData
                            required property int index
                            width: rankingList.width; implicitHeight: 58; radius: 11; color: index === 0 ? "#1D3B31" : "#152D36"; border.color: index === 0 ? "#3E6348" : "#203B44"
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 10
                                Label { text: String(rankRow.index + 1).padStart(2, "0"); color: rankRow.index === 0 ? "#E8C878" : "#648B94"; font.pixelSize: 15; font.weight: Font.DemiBold }
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 3
                                    Label { text: rankRow.modelData.playerName; color: "#D5E6DB"; elide: Text.ElideRight; font.pixelSize: 12; Layout.fillWidth: true }
                                    Label { text: rankRow.modelData.playedAt; color: "#688B92"; font.pixelSize: 9 }
                                }
                                Label { text: rankRow.modelData.score + " pts"; color: "#71DCA0"; font.pixelSize: 13; font.weight: Font.DemiBold }
                            }
                        }
                    }
                }
            }
        }
    }
}
