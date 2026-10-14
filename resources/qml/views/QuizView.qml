pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    objectName: "quizView"
    required property var app
    required property var backend
    property string selectedOption: ""
    implicitHeight: Math.max(620,quizLayout.implicitHeight)
    focus: true
    Keys.onPressed: function(event) {
        if (finishDialog.visible || !portrait.hasPhoto) return
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_4) {
            const index = event.key - Qt.Key_1
            if (index < backend.answerOptions.length) selectedOption = backend.answerOptions[index]
            event.accepted = true
        } else if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && selectedOption) {
            confirmAnswer(); event.accepted = true
        }
    }
    function confirmAnswer() {
        if (!selectedOption || backend.gameFinished || backend.answerPending || !portrait.hasPhoto) return
        backend.answerSelected(selectedOption)
        if (backend.answerPending) app.navigate("feedback")
    }
    ColumnLayout {
        id: quizLayout
        anchors.fill: parent
        spacing: 22
        RowLayout {
            Layout.fillWidth: true
            UiButton { objectName: "backFromQuizButton"; text: "Voltar"; iconName: "back"; secondary: true; compact: true; onClicked: finishDialog.open() }
            Item { Layout.fillWidth: true }
            UiIcon { name: view.backend.questionPhoto.kingdom === "Animalia" ? "butterfly" : (view.backend.questionPhoto.kingdom === "Plantae" ? "leaf" : "book"); ink: "#E2B969"; implicitWidth: 20; implicitHeight: 20 }
            Label { text: view.backend.categoryName + " · " + view.backend.identificationRankLabel; color: "#C9DCD2"; font.pixelSize: 13 }
            Item { Layout.fillWidth: true }
            Label { text: "Pergunta " + view.backend.round; color: "#D3E4DD"; font.pixelSize: 13; font.weight: Font.DemiBold }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 28
            ColumnLayout {
                Layout.preferredWidth: view.width * 0.37
                Layout.fillHeight: true
                spacing: 12
                SpeciesPortrait { id: portrait; objectName: "questionPortrait"; Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 360; info: view.backend.questionPhoto }
                Label { text: view.backend.questionPhoto.supporting ? "Use as pistas. A fotografia é apenas apoio." : "Observe a fotografia. Identifique o grupo solicitado."; color: "#718F93"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                Rectangle {
                    Layout.fillWidth: true; implicitHeight: 54; radius: 12; color: "#102B2B"; border.color: "#28433C"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14
                        UiIcon { name: "user"; ink: "#73AD94"; implicitWidth: 18; implicitHeight: 18 }
                        Label { text: view.backend.sessionPlayerName; color: "#A5C4B5"; elide: Text.ElideRight; Layout.fillWidth: true; font.pixelSize: 12 }
                        Label { text: view.backend.score + " pts"; color: "#6AE09D"; font.pixelSize: 13; font.weight: Font.DemiBold }
                    }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12
                Label { text: "IDENTIFIQUE E DESCUBRA"; color: "#69C590"; font.pixelSize: 9; font.letterSpacing: 1.6 }
                Label { text: view.backend.questionText; color: "#F2F8F4"; font.pixelSize: 25; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: view.backend.factText; color: "#B1C8CC"; font.pixelSize: 15; lineHeight: 1.35; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.topMargin: 2; Layout.bottomMargin: 10 }
                Repeater {
                    model: view.backend.answerOptions
                    delegate: Button {
                        id: option
                        required property string modelData
                        required property int index
                        readonly property bool selected: view.selectedOption === modelData
                        objectName: "answerOption" + index
                        Layout.fillWidth: true
                        implicitHeight: 58
                        hoverEnabled: true
                        padding: 14
                        Accessible.name: String.fromCharCode(65 + index) + ". " + modelData
                        background: Rectangle {
                            radius: 12; color: option.selected ? "#134C36" : (option.hovered ? "#1B3440" : "#132832")
                            border.width: option.selected || option.activeFocus ? 2 : 1
                            border.color: option.selected ? "#35D480" : (option.activeFocus ? "#8ECCAD" : "#2B424D")
                        }
                        contentItem: RowLayout {
                            spacing: 12
                            Rectangle {
                                implicitWidth: 28; implicitHeight: 28; radius: 14; color: option.selected ? "#29BF70" : "#223C48"; border.color: option.selected ? "#6EDC9A" : "#3D5762"
                                Label { anchors.centerIn: parent; text: String.fromCharCode(65 + option.index); color: "#E4F0E9"; font.pixelSize: 12 }
                            }
                            Label { text: option.modelData; color: "#E1EDE7"; font.pixelSize: 14; font.italic: true; Layout.fillWidth: true; elide: Text.ElideRight }
                            UiIcon { visible: option.selected; name: "check"; ink: "#7DE8A8"; implicitWidth: 18; implicitHeight: 18 }
                        }
                        onClicked: view.selectedOption = modelData
                    }
                }
                Rectangle {
                    visible: view.backend.hintRevealed
                    Layout.fillWidth: true
                    implicitHeight: hintLabel.implicitHeight + 24
                    radius: 12; color: "#2B3325"; border.color: "#4D5538"
                    Label { id: hintLabel; anchors.fill: parent; anchors.margins: 12; text: view.backend.extraHint; color: "#DDD3A4"; font.pixelSize: 12; wrapMode: Text.WordWrap }
                }
                Item { Layout.fillHeight: true }
                RowLayout {
                    Layout.fillWidth: true
                    UiButton { objectName: "hintButton"; text: !view.backend.hasHint ? "Sem dica disponível" : (view.backend.hintRevealed ? "Dica revelada" : "Dica"); iconName: "hint"; secondary: true; enabled: view.backend.hasHint && !view.backend.hintRevealed; onClicked: view.backend.revealHint() }
                    Item { Layout.fillWidth: true }
                    UiButton { objectName: "confirmAnswerButton"; text: "Confirmar"; iconName: "arrow"; enabled: view.selectedOption !== "" && portrait.hasPhoto; onClicked: view.confirmAnswer() }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label { text: "Ciclo atual · " + (((view.backend.round - 1) % Math.max(1, view.backend.catalogSize)) + 1) + " de " + view.backend.catalogSize + " espécies"; color: "#688B94"; font.pixelSize: 11 }
            ProgressBar {
                Layout.preferredWidth: 160
                from: 0; to: Math.max(1, view.backend.catalogSize); value: (view.backend.round - 1) % Math.max(1, view.backend.catalogSize)
                background: Rectangle { implicitHeight: 5; color: "#203943"; radius: 3 }
                contentItem: Item { implicitHeight: 5; Rectangle { width: parent.width * ((view.backend.round - 1) % Math.max(1, view.backend.catalogSize)) / Math.max(1, view.backend.catalogSize); implicitHeight: 5; radius: 3; color: "#3ACA82" } }
            }
            Item { Layout.fillWidth: true }
        }
    }
    EndGameDialog { id: finishDialog; onAccepted: view.app.leaveGame() }
}
