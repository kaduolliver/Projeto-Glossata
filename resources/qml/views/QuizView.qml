pragma ComponentBehavior: Bound
import QtQuick
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
    readonly property bool compact: width < 850
    readonly property bool sideBySide: width >= 700
    readonly property bool questionActive: !backend.gameFinished && !backend.answerPending && !submitting
    readonly property bool canSelect: questionActive && portrait.hasPhoto && !finishDialog.visible && !hintPanel.visible
    readonly property int cycleIndex: (backend.round - 1) % Math.max(1, backend.catalogSize)
    implicitHeight: quizLayout.implicitHeight
    focus: true
    Keys.onPressed: function(event) {
        if (!canSelect) return
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_4) {
            const index = event.key - Qt.Key_1
            if (index < backend.answerOptions.length) selectedOption = backend.answerOptions[index]
            event.accepted = true
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
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
    ColumnLayout {
        id: quizLayout
        width: parent.width; spacing: Style.Theme.spaceMd
        RowLayout {
            Layout.fillWidth: true; spacing: Style.Theme.spaceMd
            UiButton { id: backButton; objectName: "backFromQuizButton"; text: "Encerrar"; iconName: "back"; secondary: true; compact: true; onClicked: finishDialog.open() }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 3
                Label { objectName: "quizModeLabel"; text: view.backend.gameMode === "competitive" ? "MODO COMPETITIVO" : "MODO LIVRE"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1 }
                Label { objectName: "quizGroupLabel"; text: view.backend.categoryName + " · " + view.backend.identificationRankLabel; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: Style.Theme.labelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            }
            ColumnLayout {
                Layout.preferredWidth: view.compact ? 155 : 190; Layout.maximumWidth: view.compact ? 155 : 190; spacing: 5
                Label { objectName: "quizQuestionNumber"; text: "Pergunta " + view.backend.round + (view.backend.gameMode === "competitive" ? " / " + view.backend.questionLimit : ""); color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
                ProgressBar {
                    id: progress; objectName: "quizProgress"; Layout.fillWidth: true
                    from: 0; to: Math.max(1, view.backend.gameMode === "competitive" ? view.backend.questionLimit : view.backend.catalogSize)
                    value: view.backend.gameMode === "competitive" ? view.backend.answeredCount : view.cycleIndex
                    Accessible.name: view.backend.gameMode === "competitive" ? "Questões respondidas na partida" : "Progresso do ciclo Livre"
                    background: Rectangle { implicitHeight: 5; radius: 3; color: Style.Theme.surfaceRaised }
                    contentItem: Item { implicitHeight: 5; Rectangle { width: parent.width * progress.visualPosition; height: 5; radius: 3; color: Style.Theme.green } }
                }
                Label { objectName: "quizCycleLabel"; visible: view.backend.gameMode !== "competitive"; text: "Ciclo · " + (view.cycleIndex + 1) + " de " + view.backend.catalogSize; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; Layout.fillWidth: true }
            }
        }
        GridLayout {
            objectName: "quizQuestionLayout"
            Layout.fillWidth: true; columns: view.sideBySide ? 2 : 1
            columnSpacing: Style.Theme.spaceLg; rowSpacing: Style.Theme.spaceMd
            SpeciesPortrait {
                id: portrait; objectName: "questionPortrait"
                quizPresentation: true; info: view.backend.questionPhoto
                Layout.fillWidth: true; Layout.preferredWidth: view.sideBySide ? view.width * 0.44 : view.width
                Layout.preferredHeight: view.sideBySide ? Math.max(view.compact ? 440 : 520, answers.implicitHeight) : 400
                Layout.alignment: Qt.AlignTop
            }
            ColumnLayout {
                id: answers; objectName: "quizAnswers"
                Layout.fillWidth: true; Layout.preferredWidth: view.sideBySide ? view.width * 0.56 - Style.Theme.spaceLg : view.width
                Layout.alignment: Qt.AlignTop; spacing: Style.Theme.spaceSm
                Label { text: "OBSERVE E IDENTIFIQUE"; color: Style.Theme.green; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.2 }
                Label { objectName: "quizPrompt"; text: view.backend.questionText; textFormat: Text.PlainText; color: Style.Theme.text; font.pixelSize: view.compact ? 22 : 26; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                Label { visible: !!text; text: view.backend.factText; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
                ColumnLayout {
                    id: choices; objectName: "quizChoices"
                    readonly property real cardHeight: Math.max(60, ...Array.from(children, child => child.implicitHeight))
                    Layout.fillWidth: true; spacing: Style.Theme.spaceSm
                    Repeater {
                        model: view.backend.answerOptions
                        delegate: Button {
                            id: option
                            required property string modelData
                            required property int index
                            readonly property bool selected: view.selectedOption === modelData
                            objectName: "answerOption" + index
                            Layout.fillWidth: true; Layout.preferredWidth: 1; Layout.preferredHeight: choices.cardHeight
                            implicitHeight: Math.max(60, optionBody.implicitHeight + topPadding + bottomPadding)
                            padding: Style.Theme.spaceMd; hoverEnabled: true; activeFocusOnTab: true; enabled: view.canSelect
                            Accessible.role: Accessible.RadioButton
                            Accessible.name: String.fromCharCode(65 + index) + ". " + modelData
                            Accessible.checkable: true; Accessible.checked: selected
                            Accessible.description: (selected ? "Selecionada. " : "") + "Atalho " + (index + 1) + ". Confirme para registrar a resposta."
                            background: Rectangle {
                                objectName: "quizOptionBackground"
                                radius: Style.Theme.controlRadius; border.width: Style.Theme.focusWidth
                                color: !option.enabled ? Style.Theme.disabledSurface : option.selected ? Style.Theme.forest : option.down ? Style.Theme.primaryPressed : option.hovered ? Style.Theme.surfaceHover : Style.Theme.surface
                                border.color: option.visualFocus ? Style.Theme.focus : option.selected ? Style.Theme.green : Style.Theme.border
                                Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                                Behavior on border.color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                            }
                            contentItem: RowLayout {
                                id: optionBody; spacing: Style.Theme.spaceSm
                                Rectangle {
                                    implicitWidth: 30; implicitHeight: 30; radius: 15
                                    color: option.selected ? Style.Theme.primary : Style.Theme.surfaceRaised
                                    border.color: option.selected ? Style.Theme.green : Style.Theme.border
                                    Label { anchors.centerIn: parent; text: String.fromCharCode(65 + option.index); color: option.enabled ? Style.Theme.text : Style.Theme.textDisabled; font.pixelSize: Style.Theme.labelSize; font.weight: Font.DemiBold }
                                }
                                Label { objectName: "quizOptionText"; text: option.modelData; textFormat: Text.PlainText; color: option.enabled ? Style.Theme.text : Style.Theme.textDisabled; font.pixelSize: Style.Theme.bodySize; font.italic: true; wrapMode: Text.Wrap; Layout.fillWidth: true }
                                Item { implicitWidth: 18; implicitHeight: 18; UiIcon { visible: option.selected; anchors.fill: parent; name: "target"; ink: Style.Theme.gold } }
                            }
                            Keys.onReturnPressed: view.confirmAnswer()
                            Keys.onEnterPressed: view.confirmAnswer()
                            onClicked: view.selectedOption = modelData
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: Style.Theme.spaceSm
                    Label { objectName: "possibleScoreLabel"; text: "Nesta pergunta: " + view.backend.possibleScoreText + " ponto"; color: Style.Theme.green; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    Label { objectName: "quizScoreLabel"; text: "Total: " + view.backend.scoreText; color: Style.Theme.gold; font.pixelSize: Style.Theme.captionSize; Accessible.name: "Pontuação acumulada: " + view.backend.scoreText }
                }
                Flow {
                    Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: Style.Theme.spaceXs
                    Repeater {
                        model: view.backend.hints
                        delegate: TextAction {
                            required property var modelData
                            required property int index
                            visible: !!modelData.revealed
                            text: hintPanel.categoryTitle(modelData.category) + " · ver dica"
                            Accessible.description: "Consultar novamente a dica revelada; não altera sua pontuação."
                            onClicked: { hintPanel.selectedCategory = index; hintPanel.open() }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: Style.Theme.spaceSm
                    UiButton { id: hintButton; objectName: "hintButton"; text: "Dica"; iconName: "hint"; secondary: true; compact: view.compact; enabled: view.questionActive; onClicked: hintPanel.open() }
                    Item { Layout.fillWidth: true }
                    UiButton { objectName: "confirmAnswerButton"; text: "Confirmar resposta"; iconName: "arrow"; compact: view.compact; enabled: view.canSelect && view.selectedOption !== ""; onClicked: view.confirmAnswer() }
                }
                Label { text: "1–4 para selecionar · Enter para confirmar"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: Style.Theme.spaceSm
            UiIcon { name: "user"; ink: Style.Theme.green; implicitWidth: 16; implicitHeight: 16 }
            Label { objectName: "quizPlayerLabel"; text: view.backend.sessionPlayerName; textFormat: Text.PlainText; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
        }
    }
    EndGameDialog { id: finishDialog; onAccepted: view.app.leaveGame(); onClosed: backButton.forceActiveFocus(Qt.PopupFocusReason) }
    HintPanel { id: hintPanel; backend: view.backend; onClosed: hintButton.forceActiveFocus(Qt.PopupFocusReason) }
}
