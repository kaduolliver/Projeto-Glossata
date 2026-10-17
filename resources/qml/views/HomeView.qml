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
        anchors.fill: parent
        spacing: 20
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 440
            radius: 22
            color: "#122F2B"
            border.color: "#355943"
            clip: true
            LifeArtwork { anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom; width: parent.width * 0.61 }
            Rectangle {
                anchors.fill: parent; radius: 22
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "#FF102A28" }
                    GradientStop { position: 0.48; color: "#E6102A28" }
                    GradientStop { position: 0.85; color: "#00102A28" }
                }
            }
            ColumnLayout {
                anchors.left: parent.left; anchors.leftMargin: 38
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width * 0.54
                spacing: 20
                Rectangle {
                    implicitWidth: 198; implicitHeight: 28; radius: 14; color: "#253E30"; border.color: "#496043"
                    Label { anchors.centerIn: parent; text: "UM MUNDO DE DESCOBERTAS"; color: "#B6D29B"; font.pixelSize: 9; font.letterSpacing: 1.6 }
                }
                ColumnLayout {
                    spacing: 3; Layout.fillWidth: true
                    Label { text: "Um mundo de vida."; color: "#F5F8EF"; font.pixelSize: 38; font.weight: Font.DemiBold; font.letterSpacing: -1.3; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { text: "Infinitas descobertas."; color: "#5DDE8D"; font.pixelSize: 38; font.weight: Font.DemiBold; font.letterSpacing: -1.3; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                Label { text: "Explore Bacteria, Archaea e Eukaryota e descubra suas linhagens evolutivas. Observe, identifique e aprenda sobre a diversidade da vida, uma espécie de cada vez."; color: "#B8CAC0"; font.pixelSize: 15; lineHeight: 1.4; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.maximumWidth: 345 }
                UiButton { objectName: "homePlayButton"; text: view.app.playing ? "Continuar partida" : "Começar a explorar"; iconName: "play"; Layout.topMargin: 8; implicitWidth: 230; onClicked: view.app.openPlay() }
                Label { text: "Respostas e dicas salvas automaticamente."; color: "#71998A"; font.pixelSize: 11 }
            }
            Label { anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 18; text: "Ilustração decorativa"; color: "#9AAF97"; font.pixelSize: 9 }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            StatCard { Layout.fillWidth: true; title: "Espécies para descobrir"; value: String(view.backend.categories[0].count); iconName: "butterfly"; accent: "#E6B366" }
            StatCard { Layout.fillWidth: true; title: "Seu ritmo"; value: "Sem pressa"; iconName: "leaf"; accent: "#52D18B" }
            StatCard { Layout.fillWidth: true; title: "Conhecimento"; value: "A cada descoberta"; iconName: "book"; accent: "#74AEDD" }
        }
    }
}
