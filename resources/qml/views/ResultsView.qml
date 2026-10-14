pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: 540
    Rectangle {
        anchors.fill: parent; radius: 24; color: "#102E28"; border.color: "#30563D"
        LifeArtwork { anchors.fill: parent; opacity: 0.18 }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 42; spacing: 16
            Item { Layout.fillHeight: true }
            UiIcon { name: "trophy"; ink: "#F1C656"; implicitWidth: 64; implicitHeight: 64; Layout.alignment: Qt.AlignHCenter }
            Label { text: view.backend.answeredCount > 0 ? "Partida concluída!" : "Pronto para descobrir?"; color: "#F0F7ED"; font.pixelSize: 34; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true; wrapMode: Text.WordWrap }
            Label { text: view.backend.sessionPlayerName + " · " + view.backend.categoryName; color: "#91B7A1"; font.pixelSize: 13; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
            RowLayout {
                Layout.fillWidth: true; Layout.topMargin: 16; Layout.bottomMargin: 12; spacing: 16
                StatCard { Layout.fillWidth: true; title: "Acertos"; value: String(view.backend.score); iconName: "check"; accent: "#43D189" }
                StatCard { Layout.fillWidth: true; title: "Erros"; value: String(view.backend.answeredCount - view.backend.score); iconName: "cross"; accent: "#E97B6C" }
                StatCard { Layout.fillWidth: true; title: "Aproveitamento"; value: view.app.accuracy + "%"; iconName: "target"; accent: "#B996EC" }
            }
            Label { text: view.backend.feedbackText; color: view.backend.resultPending ? "#F0BBA8" : "#A7C5B1"; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 12 }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 16; spacing: 16
                UiButton { objectName: "returnExplorationButton"; text: "Voltar ? explora??o"; iconName: "play"; secondary: true; enabled: !view.backend.resultPending; onClicked: view.app.returnToExploration() }
                UiButton { text: "Ver estatísticas"; iconName: "chart"; onClicked: view.app.navigate("statistics") }
            }
            Item { Layout.fillHeight: true }
            Label { text: "Cada pergunta é uma oportunidade de aprender."; color: "#608D76"; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
        }
    }
}
