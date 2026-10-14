pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: Math.max(580, infoLayout.implicitHeight)
    ColumnLayout {
        id: infoLayout
        anchors.fill: parent
        spacing: 24
        RowLayout {
            UiButton { text: "Voltar"; iconName: "back"; secondary: true; compact: true; onClicked: view.app.navigate("feedback") }
            Label { text: "Conheça a espécie"; color: "#EEF7F0"; font.pixelSize: 26; font.weight: Font.DemiBold; Layout.leftMargin: 14 }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 32
            SpeciesPortrait { Layout.preferredWidth: view.width * 0.4; Layout.preferredHeight: view.width < 850 ? 450 : 520; Layout.minimumHeight: 350; Layout.maximumHeight: 520; Layout.alignment: Qt.AlignTop; info: view.backend.answerDetails }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 18
                Label { text: view.backend.answerDetails.speciesName || ""; color: "#F2F8EE"; font.pixelSize: 29; font.weight: Font.DemiBold; font.italic: true; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { visible: !!view.backend.answerDetails.commonName; text: view.backend.answerDetails.commonName || ""; color: "#B6CBBF"; font.pixelSize: 15; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Reino didático: " + (view.backend.answerDetails.kingdom || "Não informado"); color: "#86A5AA"; font.pixelSize: 13; Layout.bottomMargin: 6 }
                Repeater {
                    model: [
                        {title: "Classificação", body: view.backend.answerDetails.classification || "Não informada", icon: "book", color: "#72D585"},
                        {title: "Distribuição", body: view.backend.answerDetails.region || "Não informada", icon: "pin", color: "#67ABEE"},
                        {title: "Curiosidade" + (view.backend.answerDetails.curiosityScope ? " · " + view.backend.answerDetails.curiosityScope : ""), body: view.backend.answerDetails.fact || "", icon: "spark", color: "#E0B75C"},
                        {title: "Mais uma pista", body: view.backend.answerDetails.hint || "Ainda não há dica validada para esta questão.", icon: "leaf", color: "#A4CE9E"}
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
                TextAction { visible: !!view.backend.answerDetails.sourceUrl; text: "Consultar classificação no iNaturalist ↗"; onClicked: Qt.openUrlExternally(view.backend.answerDetails.sourceUrl) }
                TextAction { visible: !!view.backend.answerDetails.curiositySource; text: "Fonte da curiosidade"; onClicked: Qt.openUrlExternally(view.backend.answerDetails.curiositySource) }
                TextAction { visible: !!view.backend.answerDetails.hintSource; text: "Fonte da dica"; onClicked: Qt.openUrlExternally(view.backend.answerDetails.hintSource) }
                Item { Layout.fillHeight: true }
                UiButton { text: "Continuar explorando"; iconName: "arrow"; Layout.fillWidth: true; onClicked: view.app.nextQuestion() }
            }
        }
    }
}
