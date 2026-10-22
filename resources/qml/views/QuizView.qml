pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

Item {
    id: view
    objectName: "quizView"
    required property var app
    required property var backend
    property string selectedOption: ""
    property bool submitting: false
    readonly property bool questionActive: !backend.gameFinished && !backend.answerPending && !submitting
    readonly property bool canSelect: questionActive && portrait.hasPhoto && !finishDialog.visible && !hintPanel.visible
    readonly property var containingWindow: Window.window
    implicitHeight: quizLayout.implicitHeight
    focus: true
    Keys.onPressed: function(event) {
        if (!canSelect) return
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_4) {
            const index = event.key - Qt.Key_1
            if (index < backend.answerOptions.length) selectedOption = backend.answerOptions[index]
            event.accepted = true
        } else if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && selectedOption) {
            confirmAnswer(); event.accepted = true
        }
    }
    function confirmAnswer() {
        if (!canSelect || backend.answerOptions.indexOf(selectedOption) < 0) return
        submitting = true
        backend.answerSelected(selectedOption)
        submitting = false
        if (backend.answerPending) app.navigate("feedback")
    }
    Connections { target: view.backend; function onAnswerOptionsChanged() { view.selectedOption = "" } }
    function revealFocusedAnswer() {
        const control = containingWindow ? containingWindow.activeFocusItem : null
        let ancestor = control
        while (ancestor && ancestor !== answerScroll) ancestor = ancestor.parent
        const viewport = answerScroll.contentItem as Flickable
        if (!ancestor || !control || !viewport) return
        const top = control.mapToItem(viewport, 0, 0).y, bottom = top + control.height
        if (top < 8) viewport.contentY = Math.max(0, viewport.contentY + top - 8)
        else if (bottom > viewport.height - 8) viewport.contentY = Math.min(viewport.contentHeight - viewport.height, viewport.contentY + bottom - viewport.height + 8)
    }
    Timer { id: revealFocus; interval: 0; onTriggered: view.revealFocusedAnswer() }
    Connections { target: view.containingWindow; function onActiveFocusItemChanged() { if (view.canSelect) revealFocus.restart() } }
    ColumnLayout {
        id: quizLayout
        anchors.fill: parent
        spacing: 22
        RowLayout {
            Layout.fillWidth: true
            UiButton { id: backButton; objectName: "backFromQuizButton"; text: "Voltar"; iconName: "back"; secondary: true; compact: true; onClicked: finishDialog.open() }
            Item { Layout.fillWidth: true }
            UiIcon { name: view.backend.questionPhoto.kingdom === "Animalia" ? "butterfly" : (view.backend.questionPhoto.kingdom === "Plantae" ? "leaf" : "book"); ink: "#E2B969"; implicitWidth: 20; implicitHeight: 20 }
            Label { objectName: "quizGroupLabel"; text: view.backend.categoryName + " · " + view.backend.identificationRankLabel; textFormat: Text.PlainText; color: "#C9DCD2"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.minimumWidth: 0; Layout.maximumWidth: view.width * 0.5 }
            Item { Layout.fillWidth: true }
            Label { objectName: "quizQuestionNumber"; text: "Pergunta " + view.backend.round + (view.backend.gameMode === "competitive" ? " / " + view.backend.questionLimit : ""); color: "#D3E4DD"; font.pixelSize: 13; font.weight: Font.DemiBold }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 28
            ColumnLayout {
                objectName: "quizPortraitColumn"
                Layout.preferredWidth: view.width * 0.37
                Layout.minimumWidth: view.width * 0.37
                Layout.maximumWidth: view.width * 0.37
                Layout.fillHeight: true
                spacing: 12
                SpeciesPortrait { id: portrait; objectName: "questionPortrait"; Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 360; info: view.backend.questionPhoto }
                Label { text: view.backend.questionPhoto.supporting ? "Use as pistas. A fotografia é apenas apoio." : "Observe a fotografia. Identifique o grupo solicitado."; color: "#718F93"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                Rectangle {
                    objectName: "quizPlayerPanel"
                    Layout.fillWidth: true; implicitHeight: Math.max(54, playerInfo.implicitHeight + 28); radius: 12; color: "#102B2B"; border.color: "#28433C"
                    ColumnLayout {
                        id: playerInfo
                        anchors.fill: parent; anchors.margins: 14
                        spacing: 4
                        RowLayout {
                            Layout.fillWidth: true
                            UiIcon { name: "user"; ink: "#73AD94"; implicitWidth: 18; implicitHeight: 18 }
                            Label { objectName: "quizPlayerLabel"; text: view.backend.sessionPlayerName; textFormat: Text.PlainText; color: "#A5C4B5"; elide: Text.ElideRight; Layout.fillWidth: true; Layout.minimumWidth: 0; font.pixelSize: 12 }
                            Label { objectName: "quizScoreLabel"; text: view.backend.scoreText + " pts"; color: "#6AE09D"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        }
                        Label {
                            objectName: "quizAnswerCounts"
                            readonly property int correct: view.backend.correctAnswers
                            readonly property int incorrect: view.backend.answeredCount - correct
                            text: correct + (correct === 1 ? " acerto" : " acertos") + " · " + incorrect + (incorrect === 1 ? " erro" : " erros")
                            color: "#A5C4B5"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
            ColumnLayout {
                objectName: "quizAnswers"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.fillHeight: true
                spacing: 12
                ScrollView {
                    id: answerScroll; objectName: "quizAnswersScroll"
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumWidth: 0; Layout.minimumHeight: 0; Layout.preferredHeight: 1
                    clip: true; contentWidth: availableWidth; rightPadding: 14
                    ColumnLayout {
                        width: answerScroll.availableWidth; spacing: 12
                        Label { text: "IDENTIFIQUE E DESCUBRA"; color: "#69C590"; font.pixelSize: 9; font.letterSpacing: 1.6 }
                        Label { objectName: "quizPrompt"; text: view.backend.questionText; textFormat: Text.PlainText; color: "#F2F8F4"; font.pixelSize: 25; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                                implicitHeight: Math.max(58, contentItem.implicitHeight + topPadding + bottomPadding)
                                hoverEnabled: true
                                activeFocusOnTab: true; enabled: view.canSelect
                                padding: 14
                                Accessible.name: String.fromCharCode(65 + index) + ". " + modelData
                                Accessible.role: Accessible.RadioButton; Accessible.checkable: true; Accessible.checked: selected
                                Accessible.description: "Atalho " + (index + 1) + ". " + (selected ? "Selecionada." : "Confirme para registrar a resposta.")
                                background: Rectangle {
                                    objectName: "quizOptionBackground"
                                    radius: 12; color: !option.enabled ? Style.Theme.disabledSurface : option.selected ? "#134C36" : (option.hovered ? "#1B3440" : "#132832")
                                    border.width: option.selected || option.activeFocus ? 2 : 1
                                    border.color: option.selected ? "#35D480" : (option.activeFocus ? "#8ECCAD" : "#2B424D")
                                }
                                contentItem: RowLayout {
                                    spacing: 12
                                    Rectangle {
                                        implicitWidth: 28; implicitHeight: 28; radius: 14; color: option.selected ? "#29BF70" : "#223C48"; border.color: option.selected ? "#6EDC9A" : "#3D5762"
                                        Label { anchors.centerIn: parent; text: String.fromCharCode(65 + option.index); color: "#E4F0E9"; font.pixelSize: 12 }
                                    }
                                    Label { objectName: "quizOptionText"; text: option.modelData; textFormat: Text.PlainText; color: "#E1EDE7"; font.pixelSize: 14; font.italic: true; Layout.fillWidth: true; Layout.minimumWidth: 0; wrapMode: Text.WordWrap }
                                    UiIcon { visible: option.selected; name: "check"; ink: "#7DE8A8"; implicitWidth: 18; implicitHeight: 18 }
                                }
                                Keys.onReturnPressed: view.confirmAnswer()
                                Keys.onEnterPressed: view.confirmAnswer()
                                onClicked: view.selectedOption = modelData
                            }
                        }
                        Label { objectName: "possibleScoreLabel"; text: "Pontuação possível: " + view.backend.possibleScoreText; color: "#71DCA0"; font.pixelSize: 13 }
                        Repeater {
                            model: view.backend.hints
                            delegate: ColumnLayout {
                                id: hintBlock
                                required property var modelData
                                required property int index
                                objectName: "revealedHint" + index
                                visible: !!modelData.revealed; Layout.fillWidth: true; Layout.minimumWidth: 0; spacing: 4
                                Label {
                                    objectName: "revealedHintTitle" + hintBlock.index
                                    text: (hintBlock.modelData.assistance ? "Dica " + hintBlock.modelData.assistance : "Dica revelada") + " · " + hintPanel.categoryTitle(hintBlock.modelData.category)
                                    color: hintBlock.modelData.assistance === "Leve" ? Style.Theme.green : hintBlock.modelData.assistance === "Moderada" ? Style.Theme.gold : hintBlock.modelData.assistance === "Forte" ? "#FF9A91" : Style.Theme.textMuted
                                    font.pixelSize: 12; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; textFormat: Text.PlainText
                                }
                                Label { objectName: "revealedHintText" + hintBlock.index; text: hintBlock.modelData.text || ""; Layout.fillWidth: true; Layout.minimumWidth: 0; wrapMode: Text.WordWrap; textFormat: Text.PlainText; color: "#DDD3A4"; font.pixelSize: 12 }
                            }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    UiButton { id: hintButton; objectName: "hintButton"; text: "Dica"; iconName: "hint"; secondary: true; enabled: view.questionActive; onClicked: hintPanel.open() }
                    Item { Layout.fillWidth: true }
                    UiButton { objectName: "confirmAnswerButton"; text: "Confirmar"; iconName: "arrow"; enabled: view.selectedOption !== "" && view.canSelect; onClicked: view.confirmAnswer() }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label { objectName: "quizCycleLabel"; visible: view.backend.gameMode !== "competitive"; text: "Ciclo atual · " + (((view.backend.round - 1) % Math.max(1, view.backend.catalogSize)) + 1) + " de " + view.backend.catalogSize + " espécies"; color: "#688B94"; font.pixelSize: 11 }
            ProgressBar {
                objectName: "quizProgress"
                visible: view.backend.gameMode !== "competitive"
                Layout.preferredWidth: 160
                from: 0; to: Math.max(1, view.backend.catalogSize); value: (view.backend.round - 1) % Math.max(1, view.backend.catalogSize)
                background: Rectangle { implicitHeight: 5; color: "#203943"; radius: 3 }
                contentItem: Item { implicitHeight: 5; Rectangle { width: parent.width * ((view.backend.round - 1) % Math.max(1, view.backend.catalogSize)) / Math.max(1, view.backend.catalogSize); implicitHeight: 5; radius: 3; color: "#3ACA82" } }
            }
            Item { Layout.fillWidth: true }
        }
    }
    EndGameDialog { id: finishDialog; onAccepted: view.app.leaveGame(); onClosed: backButton.forceActiveFocus(Qt.PopupFocusReason) }
    HintPanel { id: hintPanel; backend: view.backend; onClosed: hintButton.forceActiveFocus(Qt.PopupFocusReason) }
}
