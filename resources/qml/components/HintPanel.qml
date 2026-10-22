pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
Dialog {
    id: panel
    required property var backend
    objectName: "hintPanel"
    property int selectedCategory: -1
    readonly property var labels: [0,1,2]
    function categoryTitle(key) { return key === "taxonomic" ? "Taxonômica" : key === "etymological" ? "Etimológica" : key === "morphological" ? "Morfológica" : "Dica" }
    function categoryIcon(key) { return key === "taxonomic" ? "book" : key === "etymological" ? "spark" : "shape" }
    readonly property var selectedHint: selectedCategory >= 0 ? backend.hints[selectedCategory] || ({}) : ({})
    readonly property var containingWindow: scroll.containingWindow
    parent: Overlay.overlay
    modal: true; focus: true; anchors.centerIn: parent
    width: Math.min(620,parent ? parent.width-32 : 620)
    height: Math.min(740,Overlay.overlay ? Overlay.overlay.height-40 : 740)
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    header: Label { text: "Dicas de identificação"; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; color: Style.Theme.text; padding: Style.Theme.spaceLg }
    background: Rectangle { radius: Style.Theme.cardRadius; color: Style.Theme.surface; border.color: Style.Theme.border }
    function revealFocusedControl() {
        const control = containingWindow ? containingWindow.activeFocusItem : null
        let ancestor = control
        while (ancestor && ancestor !== scroll) ancestor = ancestor.parent
        const viewport = scroll.contentItem as Flickable
        if (!ancestor || !control || !viewport) return
        const top = control.mapToItem(viewport, 0, 0).y, bottom = top + control.height
        if (top < 8) viewport.contentY = Math.max(0, viewport.contentY + top - 8)
        else if (bottom > viewport.height - 8) viewport.contentY = Math.min(viewport.contentHeight - viewport.height, viewport.contentY + bottom - viewport.height + 8)
    }
    Timer { id: revealFocus; interval: 0; onTriggered: panel.revealFocusedControl() }
    Connections { target: panel.containingWindow; function onActiveFocusItemChanged() { if (panel.opened && !report.visible) revealFocus.restart() } }
    onOpened: {
        const viewport = scroll.contentItem as Flickable
        if (viewport) viewport.contentY = 0
        if (selectedHint.revealed) hintText.forceActiveFocus(Qt.PopupFocusReason)
        else closeHints.forceActiveFocus(Qt.PopupFocusReason)
    }
    contentItem: ScrollView {
        id: scroll; objectName: "hintPanelContent"
        readonly property var containingWindow: Window.window
        clip: true; contentWidth: availableWidth
        ColumnLayout {
            width: scroll.availableWidth; spacing: 12
            Label { text: "Escolha o apoio necessário. O limite de pontos aparece antes de revelar; ao combinar dicas, vale o menor valor."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Repeater {
                model: panel.labels
                delegate: Button {
                    id: choice
                    required property var modelData
                    required property int index
                    readonly property var hint: panel.backend.hints[index] || ({})
                    objectName: "hintCategory" + index
                    enabled: !!hint.available && !panel.backend.answerPending && !panel.backend.gameFinished; hoverEnabled: true; padding: Style.Theme.spaceMd; activeFocusOnTab: true
                    Layout.fillWidth: true; implicitHeight: Math.max(100, contentItem.implicitHeight + topPadding + bottomPadding)
                    Accessible.name: panel.categoryTitle(hint.category) + ". Assistência " + hint.assistance + ". " + (hint.available ? "Revelar esta dica mantém o acerto em até " + hint.pointsText + " ponto." : hint.reason)
                    background: Rectangle {
                        radius: Style.Theme.controlRadius
                        color: !choice.enabled ? Style.Theme.disabledSurface : panel.selectedCategory===choice.index ? Style.Theme.forest : choice.down ? Style.Theme.primaryPressed : choice.hovered ? Style.Theme.surfaceHover : Style.Theme.surfaceRaised
                        border.color: choice.visualFocus ? Style.Theme.focus : panel.selectedCategory===choice.index ? Style.Theme.green : Style.Theme.border
                        border.width: Style.Theme.focusWidth
                        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                        Behavior on border.color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                    }
                    contentItem: RowLayout {
                        spacing: 12
                        UiIcon { name: panel.categoryIcon(choice.hint.category); ink: "#79CE9E" }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Label { text: (choice.index+1) + ". " + panel.categoryTitle(choice.hint.category) + (choice.hint.revealed ? " · revelada" : ""); color: choice.enabled ? Style.Theme.text : Style.Theme.textDisabled; font.pixelSize: Style.Theme.bodySize; font.weight: Font.DemiBold; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            Label { objectName: "hintAvailability" + choice.index; text: choice.hint.available ? "Avaliação pedagógica automatizada" : choice.hint.reason; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            Label {
                                text: "Assistência " + choice.hint.assistance + (choice.hint.provisional ? " · provisória, revisão pendente" : "")
                                color: choice.hint.assistance === "Leve" ? "#8CF0B1"
                                    : choice.hint.assistance === "Moderada" ? "#FFD166"
                                    : choice.hint.assistance === "Forte" ? "#FF9A91" : "#C6D8DC"
                                font.pixelSize: 13; font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap; Layout.fillWidth: true
                            }
                            Label { objectName: "hintCeiling" + choice.index; visible: !!choice.hint.available; text: "Revelar esta dica mantém o acerto em até " + choice.hint.pointsText + " ponto."; color: "#71DCA0"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        }
                    }
                    onClicked: {
                        if (panel.backend.revealHintCategory(hint.id)) {
                            panel.selectedCategory=index
                            Qt.callLater(function() { hintText.forceActiveFocus(Qt.PopupFocusReason) })
                        }
                    }
                }
            }
            Label { id: hintText; objectName: "selectedHintText"; visible: !!panel.selectedHint.revealed; text: panel.selectedHint.text || ""; color: Style.Theme.gold; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.Wrap; textFormat: Text.PlainText; Layout.fillWidth: true }
            Label { visible: !!panel.selectedHint.revealed; text: (panel.selectedHint.rationale || "") + "\nEstado de revisão: " + (panel.selectedHint.assessmentState || "Não informado"); textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.fillWidth: true; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
            Label {
                visible: !!panel.selectedHint.revealed
                text: "Avaliação automática · " + (panel.selectedHint.evaluationPolicy || "") + " · " + (panel.selectedHint.evaluationDate || "").substring(0,10)
                color: "#96AFB3"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
            }
            TextAction { objectName: "hintSourceLink"; visible: !!panel.selectedHint.sourceUrl; text: "Consultar fonte ↗"; Accessible.description: panel.selectedHint.sourceUrl || ""; onClicked: Qt.openUrlExternally(panel.selectedHint.sourceUrl) }
        }
    }
    Dialog {
        id: report; objectName: "hintReportDialog"; parent: Overlay.overlay; modal: true; anchors.centerIn: parent
        title: "Relatar problema nesta dica"; width: Math.min(540,parent ? parent.width-32 : 540)
        onOpened: reportProblem.forceActiveFocus(Qt.PopupFocusReason)
        onClosed: reportHint.forceActiveFocus(Qt.PopupFocusReason)
        height: Math.min(600,parent ? parent.height-40 : 600)
        contentItem: ColumnLayout {
            Label { text: "Rascunho local. Nada é enviado automaticamente. Não inclua dados pessoais. Sem internet, copie e guarde para depois."; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            TextArea { id: reportProblem; objectName: "hintReportProblem"; placeholderText: "Descreva o problema"; color: "#E1EDE7"; placeholderTextColor: "#91ACB4"; background: Rectangle { color: "#132832"; radius: 8; border.color: "#2B424D" } wrapMode: TextEdit.Wrap; Layout.fillWidth: true; Layout.preferredHeight: 80 }
            UiButton { objectName: "prepareHintReportButton"; text: "Preparar relato"; onClicked: reportPreview.text=panel.backend.hintReport(panel.selectedHint.id,reportProblem.text).text || "" }
            ScrollView {
                Layout.fillWidth: true; Layout.fillHeight: true
                TextArea { id: reportPreview; objectName: "hintReportPreview"; readOnly: true; color: "#E1EDE7"; wrapMode: TextEdit.Wrap; textFormat: TextEdit.PlainText }
            }
            RowLayout {
                UiButton { text: "Copiar"; secondary: true; enabled: !!reportPreview.text; onClicked: { reportPreview.selectAll(); reportPreview.copy() } }
                UiButton { text: "Abrir formulário no GitHub ↗"; enabled: !!reportPreview.text; onClicked: Qt.openUrlExternally(panel.backend.hintReport(panel.selectedHint.id,reportProblem.text).url) }
            }
            UiButton { text: "Voltar"; secondary: true; onClicked: report.close() }
        }
    }
    footer: ColumnLayout {
        spacing: 8
        TextAction {
            id: reportHint
            Layout.leftMargin: 20
            objectName: "reportHintButton"; visible: !!panel.selectedHint.revealed
            text: "Relatar problema nesta dica"
            onClicked: { reportProblem.text=""; reportPreview.text=""; report.open() }
        }

        Label { text: "Pontuação possível: " + panel.backend.possibleScoreText; color: "#71DCA0"; font.pixelSize: 13; Layout.leftMargin: 20 }
        UiButton { id: closeHints; objectName: "closeHintPanelButton"; text: "Voltar à pergunta"; secondary: true; Layout.fillWidth: true; Layout.margins: 20; onClicked: panel.close() }
    }
}
