pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    readonly property bool correct: !!backend.answerDetails.correct
    implicitHeight: 560
    Rectangle {
        anchors.fill: parent
        radius: 22
        border.color: view.correct ? "#2B6747" : "#77453C"
        gradient: Gradient { orientation: Gradient.Horizontal; GradientStop { position: 0; color: "#17362B" } GradientStop { position: 1; color: view.correct ? "#123D29" : "#402623" } }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 26
            spacing: 34
            SpeciesArtwork { Layout.preferredWidth: view.width * 0.37; Layout.fillHeight: true; accent: view.correct ? "#D5B777" : "#CB9D6A" }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 18
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: 64; implicitHeight: 64; radius: 32
                    color: view.correct ? "#23AD5B" : "#CD5B4E"
                    border.color: view.correct ? "#6EDE8F" : "#F8997D"
                    UiIcon { anchors.centerIn: parent; name: view.correct ? "check" : "cross"; ink: "#EBFFE4"; implicitWidth: 32; implicitHeight: 32 }
                }
                Label { text: view.correct ? "Muito bem!" : "Mais uma descoberta!"; color: "#F4F8EE"; font.pixelSize: 31; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: view.correct ? "Você identificou a espécie corretamente." : "Desta vez, sua resposta não foi a correta."; color: view.correct ? "#AFCAB4" : "#D7B5AB"; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; font.pixelSize: 13; Layout.fillWidth: true }
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 64
                    radius: 13; color: "#1B3930"; border.color: "#38634A"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 12
                        UiIcon { name: "check"; ink: "#64DA8C"; implicitWidth: 22; implicitHeight: 22 }
                        ColumnLayout {
                            spacing: 3; Layout.fillWidth: true
                            Label { text: "RESPOSTA CORRETA"; color: "#7DB28C"; font.pixelSize: 9; font.letterSpacing: 1 }
                            Label { objectName: "correctAnswerLabel"; text: view.backend.answerDetails.name || ""; color: "#F2F7E9"; font.pixelSize: 17; font.italic: true; elide: Text.ElideRight; Layout.fillWidth: true }
                        }
                    }
                }
                Label { text: view.backend.answerDetails.fact || ""; color: "#C2D3C1"; font.pixelSize: 15; lineHeight: 1.4; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Item { Layout.fillHeight: true }
                UiButton { text: "Ver mais informações"; iconName: "book"; secondary: true; Layout.fillWidth: true; onClicked: view.app.navigate("info") }
                UiButton { objectName: "nextQuestionButton"; text: "Próxima pergunta"; iconName: "arrow"; danger: !view.correct; Layout.fillWidth: true; onClicked: view.app.nextQuestion() }
                UiButton { text: "Encerrar partida"; secondary: true; compact: true; Layout.fillWidth: true; onClicked: view.app.finishGame() }
            }
        }
    }
}
