pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    objectName: "settingsView"
    implicitHeight: settingsLayout.implicitHeight
    Connections {
        target: view.backend
        function onCurrentPlayerNameChanged() { profileName.text = view.backend.currentPlayerName }
    }
    ColumnLayout {
        id: settingsLayout
        width: parent.width; spacing: 22
        Label { text: "Seu espaço"; color: "#EDF7F0"; font.pixelSize: 30; font.weight: Font.DemiBold }
        Label { text: "Seu usuário neste computador e as configurações do aplicativo."; color: "#8EAAB1"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: 300; radius: 18; color: "#112831"; border.color: "#28424C"
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 28; spacing: 16
                RowLayout { UiIcon { name: "user"; ink: "#6BD49A"; implicitWidth: 23; implicitHeight: 23 } Label { text: "Meu perfil"; color: "#DDEDE3"; font.pixelSize: 20; font.weight: Font.DemiBold } }
                Label { text: "Uso local e offline. Este perfil não é uma conta autenticada. Alterar o nome mantém sua identidade e seus resultados."; color: "#7F9FA6"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: "Nome de exibição · Usuário local"; color: "#A9C2C7"; font.pixelSize: 13 }
                TextField {
                    id: profileName
                    objectName: "profileNameField"
                    Layout.fillWidth: true
                    implicitHeight: 46
                    text: view.backend.currentPlayerName
                    maximumLength: 32
                    placeholderText: "Nome de exibição"
                    placeholderTextColor: "#61828C"
                    color: "#DEF0E3"
                    padding: 14
                    selectByMouse: true
                    background: Rectangle { radius: 10; color: "#0E222B"; border.color: profileName.activeFocus ? "#53C68B" : "#2B454F" }
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: 12
                    UiButton { objectName: "renameLocalUserButton"; text: "Alterar nome"; secondary: true; compact: true; onClicked: { if (view.backend.renameCurrentPlayerProfile(profileName.text)) profileName.text = view.backend.currentPlayerName } }
                    Item { Layout.fillWidth: true }
                }
                Label { text: view.backend.feedbackText; color: "#A9C5AF"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
        }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: 108; radius: 18; color: "#112831"; border.color: "#28424C"
            RowLayout {
                anchors.fill: parent; anchors.margins: 26; spacing: 16
                UiIcon { name: "spark"; ink: "#A5BCE3"; implicitWidth: 26; implicitHeight: 26 }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 7
                    Label { text: "Transições suaves"; color: "#DDEAE2"; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Label { text: "Ative ou reduza a animação entre telas nesta sessão."; color: "#7E9CA5"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                Switch { checked: view.app.motionEnabled; Accessible.name: "Transições suaves"; onToggled: view.app.motionEnabled = checked }
            }
        }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: 108; radius: 18; color: "#112831"; border.color: "#28424C"
            RowLayout {
                anchors.fill: parent; anchors.margins: 26; spacing: 16
                UiIcon { name: "butterfly"; ink: "#A5BCE3"; implicitWidth: 26; implicitHeight: 26 }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 7
                    Label { text: "Atividade no Discord"; color: "#DDEAE2"; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Label { text: view.app.presence.status; color: "#7E9CA5"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                Switch { checked: view.app.presence.enabled; Accessible.name: "Compartilhar atividade no Discord"; onToggled: view.app.presence.enabled = checked }
            }
        }
        Label { text: "Projeto Glossata · Aprender sobre a natureza, uma espécie por vez."; color: "#527782"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        TextAction { objectName: "creditsButton"; text: "Créditos, fontes e transparência"; onClicked: view.app.navigate("credits") }
        Item { Layout.fillHeight: true }
    }
}
