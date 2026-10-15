pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    readonly property var legal: backend.legalInformation
    implicitHeight: content.implicitHeight
    ColumnLayout {
        id: content
        width: parent.width
        spacing: 18
        UiButton { text: "Voltar"; secondary: true; compact: true; onClicked: view.app.navigate("settings") }
        Label { text: "Créditos e transparência"; font.pixelSize: 26; font.weight: Font.DemiBold; color: "#EEF7F0" }
        Label { text: "Aplicativo " + (view.legal.applicationVersion || "em teste") + " · Catálogo " + (view.legal.catalogVersion || "não identificado"); wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#91ADB2" }
        Label { text: "Catálogo SHA256: " + (view.legal.catalogHash || "não identificado"); wrapMode: Text.WrapAnywhere; Layout.fillWidth: true; color: "#91ADB2"; font.pixelSize: 10 }
        Label { text: view.legal.notice || ""; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#91ADB2"; textFormat: Text.PlainText }
        Label { text: view.legal.limitations || ""; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#D9BA7D" }
        Label { text: "Qt e SQLite: componentes de terceiros. Licenças individuais, inventário de bibliotecas e pendências de distribuição ficam na pasta compliance do pacote. Nenhuma licença global dos materiais próprios foi escolhida pelo mantenedor nesta etapa."; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#91ADB2" }
        Repeater {
            model: view.legal.photos || []
            delegate: ColumnLayout {
                id: credit
                required property var modelData
                Layout.fillWidth: true
                spacing: 6
                Label { text: "Foto " + credit.modelData.photo_id + " · " + (credit.modelData.author || "Autoria não identificada"); textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#EDF4EE"; font.weight: Font.DemiBold }
                Label { visible: !!text; text: credit.modelData.title || ""; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#91ADB2" }
                Label { text: credit.modelData.modifications || "Alterações não documentadas."; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#91ADB2" }
                TextAction { text: (credit.modelData.license || "Licença não informada").toUpperCase() + " · " + (credit.modelData.license_version || "versão não demonstrada") + " ↗"; onClicked: Qt.openUrlExternally(credit.modelData.license_url) }
                TextAction { text: "Fotografia e origem específica ↗"; onClicked: Qt.openUrlExternally(credit.modelData.photo_url) }
                TextAction { text: "Observação associada ↗"; onClicked: Qt.openUrlExternally(credit.modelData.source) }
                Label { visible: (credit.modelData.rights_issues || []).length > 0; text: "Permissões com pendências documentadas; distribuição bloqueada até resolução."; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: "#D9BA7D"; font.pixelSize: 11 }
                Rectangle { implicitHeight: 1; color: "#28424A"; Layout.fillWidth: true; Layout.topMargin: 10 }
            }
        }
    }
}
