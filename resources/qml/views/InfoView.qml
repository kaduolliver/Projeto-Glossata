pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: 580
    ColumnLayout {
        anchors.fill: parent
        spacing: 24
        RowLayout {
            UiButton { text: "Voltar"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate("feedback") }
            Label { text: "Conheça a espécie"; color: "#EEF7F0"; font.pixelSize: 26; font.weight: Font.DemiBold; Layout.leftMargin: 14 }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 32
            SpeciesArtwork { Layout.preferredWidth: view.width * 0.4; Layout.fillHeight: true; Layout.minimumHeight: 450 }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 18
                Label { text: view.backend.answerDetails.name || ""; color: "#F2F8EE"; font.pixelSize: 29; font.weight: Font.DemiBold; font.italic: true; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Família " + (view.backend.answerDetails.family || "Não informada"); color: "#86A5AA"; font.pixelSize: 13; Layout.bottomMargin: 6 }
                Repeater {
                    model: [
                        {title: "Classificação", body: "Lepidoptera · Glossata", icon: "butterfly", color: "#72D585"},
                        {title: "Distribuição", body: view.backend.answerDetails.region || "Não informada", icon: "pin", color: "#67ABEE"},
                        {title: "Curiosidade", body: view.backend.answerDetails.fact || "", icon: "spark", color: "#E0B75C"},
                        {title: "Mais uma pista", body: view.backend.answerDetails.hint || "", icon: "leaf", color: "#A4CE9E"}
                    ]
                    delegate: RowLayout {
                        id: detail
                        required property var modelData
                        Layout.fillWidth: true; spacing: 16
                        Rectangle { implicitWidth: 38; implicitHeight: 38; radius: 12; color: "#172E36"; Layout.alignment: Qt.AlignTop; UiIcon { anchors.centerIn: parent; name: detail.modelData.icon; ink: detail.modelData.color; implicitWidth: 22; implicitHeight: 22 } }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 5
                            Label { text: detail.modelData.title; color: "#EDF4EE"; font.pixelSize: 13; font.weight: Font.DemiBold }
                            Label { text: detail.modelData.body; color: "#91ADB2"; font.pixelSize: 13; lineHeight: 1.3; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                UiButton { text: "Continuar explorando"; iconName: "arrow"; Layout.fillWidth: true; onClicked: view.app.nextQuestion() }
            }
        }
    }
}
