pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "views"

ApplicationWindow {
    id: root
    objectName: "mainWindow"
    visible: true
    width: 1240
    height: 820
    minimumWidth: 1000
    minimumHeight: 700
    title: "Projeto Glossata"
    color: "#091A22"
    font.family: "Segoe UI"
    font.pixelSize: 14
    palette.window: "#132B35"
    palette.windowText: "#E6F1E9"
    palette.text: "#E6F1E9"
    palette.base: "#11262F"
    palette.button: "#203943"
    palette.buttonText: "#E6F1E9"
    palette.highlight: "#218653"
    palette.highlightedText: "#FFFFFF"
    palette.mid: "#2A424C"
    palette.light: "#BCF6D9"
    property var backend: gameController
    property var presence: discordPresence
    property string page: "home"
    property bool motionEnabled: true
    property bool sessionStarted: false
    property bool forceClose: false
    property string abandonedSessionId: ""
    readonly property real accuracy: backend.answeredCount > 0 ? Math.round(100 * backend.correctAnswers / backend.answeredCount) : 0
    readonly property bool playing: sessionStarted && !backend.gameFinished
    readonly property var pageTitles: ({home: "Seu próximo voo começa aqui", categories: "Explorar", taxonomy: "Explorar um grupo", quiz: "Em jogo", feedback: "Continue descobrindo", info: "Ficha da espécie", results: "Resultado da partida", statistics: "Seu desempenho", achievements: "Suas descobertas", settings: "Seu espaço"})

    function navigate(destination) { page = destination }
    function openPlay() {
        page = backend.resultPending ? "results" : (playing ? (backend.answerPending ? "feedback" : "quiz") : "categories")
    }
    function beginGame(family) {
        if (!backend.startCategoryGame(family)) return
        sessionStarted = !backend.gameFinished
        page = backend.gameFinished ? "results" : "quiz"
    }
    function openTaxon(id) { backend.exploreTaxon(id); page = "taxonomy" }
    function beginTaxonGame(id, rank) {
        if (!backend.startTaxonGame(id, rank)) return
        sessionStarted = !backend.gameFinished
        page = backend.gameFinished ? "taxonomy" : "quiz"
    }
    function returnToExploration() {
        if (backend.sessionTaxonId) openTaxon(backend.sessionTaxonId)
        else page = "categories"
    }
    function beginCompetitiveGame() {
        if (!backend.startCompetitiveGame()) return
        sessionStarted = !backend.gameFinished
        page = backend.gameFinished ? "results" : "quiz"
    }
    function nextQuestion() { backend.nextQuestion(); page = backend.gameFinished ? "results" : "quiz" }
    function finishGame() { backend.stopGame(); if (backend.gameFinished) page = "results" }
    function leaveGame() {
        backend.stopGame()
        if (!backend.gameFinished) return
        if (backend.resultPending || backend.gameMode === "competitive") page = "results"
        else returnToExploration()
    }
    onClosing: function(close) {
        if (!forceClose && !backend.prepareToClose()) { close.accepted = false; closeWarning.open() }
    }
    Dialog {
        id: closeWarning
        title: "Não foi possível confirmar o salvamento"
        width: 470
        implicitWidth: 470
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.NoButton
        contentItem: ColumnLayout {
            implicitWidth: 430
            Label { text: "O último checkpoint confirmado permanece no banco. Tente novamente ou feche preservando somente esse estado."; wrapMode: Text.WordWrap; Layout.maximumWidth: 420; color: "#E6F1E9" }
            RowLayout {
                UiButton { text: "Tentar novamente"; onClicked: { closeWarning.close(); root.close() } }
                UiButton { text: "Fechar mesmo assim"; secondary: true; onClicked: { root.forceClose = true; root.close() } }
            }
        }
    }
    Dialog {
        id: abandonDialog
        objectName: "abandonRecoveryDialog"
        title: "Abandonar sessão recuperável?"
        width: 450
        implicitWidth: 450
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        contentItem: Label { text: "A sessão não poderá ser continuada e não entrará no ranking. O checkpoint será conservado para inspeção."; wrapMode: Text.WordWrap; width: 410; color: "#E6F1E9" }
        onAccepted: root.backend.abandonSession(root.abandonedSessionId)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.preferredWidth: 208
            Layout.fillHeight: true
            color: "#10242D"
            border.color: "#203741"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 7
                RowLayout {
                    Layout.topMargin: 15
                    Layout.bottomMargin: 38
                    spacing: 10
                    Rectangle {
                        implicitWidth: 36; implicitHeight: 36; radius: 12; color: "#174D3A"
                        Image { anchors.fill: parent; anchors.margins: 2; source: "qrc:/resources/branding/glossata.png"; fillMode: Image.PreserveAspectFit; smooth: true; mipmap: true }
                    }
                    ColumnLayout {
                        spacing: 0
                        Label { text: "Projeto Glossata"; color: "#F5FAF7"; font.pixelSize: 16; font.weight: Font.DemiBold }
                        Label { text: "APRENDA. EXPLORE."; color: "#739399"; font.pixelSize: 8; font.letterSpacing: 1.5 }
                    }
                }
                Label { text: "DESCOBRIR"; color: "#62838D"; font.pixelSize: 10; font.letterSpacing: 1.6; Layout.leftMargin: 14; Layout.bottomMargin: 8 }
                NavItem { Layout.fillWidth: true; text: "Início"; iconName: "home"; selected: root.page === "home"; onClicked: root.navigate("home") }
                NavItem { Layout.fillWidth: true; text: "Jogar"; iconName: "play"; selected: ["categories", "taxonomy", "quiz", "feedback", "info", "results"].indexOf(root.page) >= 0; onClicked: root.openPlay() }
                NavItem { Layout.fillWidth: true; text: "Estatísticas"; iconName: "chart"; selected: root.page === "statistics"; onClicked: root.navigate("statistics") }
                NavItem { Layout.fillWidth: true; text: "Conquistas"; iconName: "trophy"; selected: root.page === "achievements"; onClicked: root.navigate("achievements") }
                Item { Layout.preferredHeight: 18 }
                Label { text: "SEU ESPAÇO"; color: "#62838D"; font.pixelSize: 10; font.letterSpacing: 1.6; Layout.leftMargin: 14; Layout.bottomMargin: 8 }
                NavItem { Layout.fillWidth: true; text: "Perfis e ajustes"; iconName: "settings"; selected: root.page === "settings"; onClicked: root.navigate("settings") }
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 80
                    radius: 12
                    color: "#162E37"
                    border.color: "#28424A"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 10
                        Rectangle { implicitWidth: 32; implicitHeight: 32; radius: 16; color: "#284C43"; UiIcon { anchors.centerIn: parent; name: "user"; ink: "#8CD7AC"; implicitWidth: 18; implicitHeight: 18 } }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Label { text: root.backend.currentPlayerName || "Sem perfil"; color: "#DCEBE5"; font.pixelSize: 12; font.weight: Font.DemiBold; elide: Text.ElideRight; Layout.fillWidth: true }
                            Label { text: "Perfil selecionado"; color: "#789AA1"; font.pixelSize: 10 }
                        }
                    }
                }
                Label { text: "Natureza em cada descoberta."; color: "#54757E"; font.pixelSize: 9; Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 12; Layout.bottomMargin: 8 }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 32
            spacing: 22
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                spacing: 10
                UiIcon { name: "leaf"; ink: "#65C994"; implicitWidth: 17; implicitHeight: 17 }
                Label { text: root.pageTitles[root.page] || "Projeto Glossata"; color: "#9AB2B8"; font.pixelSize: 12; Layout.fillWidth: true }
                Rectangle {
                    implicitWidth: 116; implicitHeight: 28; radius: 14; color: "#15372D"; border.color: "#285441"
                    Row { anchors.centerIn: parent; spacing: 6; Rectangle { implicitWidth: 5; implicitHeight: 5; radius: 3; color: "#56D596"; anchors.verticalCenter: parent.verticalCenter } Label { text: "Ranking local"; color: "#86CBA5"; font.pixelSize: 10 } }
                }
            }
            Rectangle {
                visible: root.backend.recoverableSessions.length > 0
                objectName: "sessionRecoveryBanner"
                Layout.fillWidth: true
                implicitHeight: recoveryControls.implicitHeight + 28
                radius: 12; color: "#17382F"; border.color: "#3C7055"
                ColumnLayout {
                    id: recoveryControls
                    anchors.fill: parent; anchors.margins: 14
                    Label { text: "Há uma sessão salva. Continue ou abandone antes de iniciar outra."; color: "#D8EFDE"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RowLayout {
                        Layout.fillWidth: true
                        ComboBox {
                            id: recoveryChoice
                            objectName: "recoverySessionChoice"
                            model: root.backend.recoverableSessions
                            textRole: "label"
                            valueRole: "id"
                            Layout.fillWidth: true
                            Accessible.name: "Sessões recuperáveis"
                        }
                        UiButton { objectName: "resumeSessionButton"; text: "Continuar sessão"; compact: true; enabled: recoveryChoice.currentIndex >= 0; onClicked: {
                            if (root.backend.resumeSession(String(recoveryChoice.currentValue))) { root.sessionStarted = true; root.page = root.backend.gameFinished ? "results" : "quiz" }
                        } }
                        UiButton { objectName: "abandonSessionButton"; text: "Abandonar"; secondary: true; compact: true; enabled: recoveryChoice.currentIndex >= 0; onClicked: { root.abandonedSessionId = String(recoveryChoice.currentValue); abandonDialog.open() } }
                    }
                    Label { text: root.backend.feedbackText; color: "#B5D0C0"; textFormat: Text.PlainText; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }
            Rectangle {
                visible: root.backend.progressError.length > 0
                objectName: "checkpointErrorBanner"
                Layout.fillWidth: true
                implicitHeight: progressControls.implicitHeight + 28
                radius: 12; color: "#442C28"; border.color: "#925246"
                RowLayout {
                    id: progressControls
                    anchors.fill: parent; anchors.margins: 14
                    Label { text: root.backend.progressError; textFormat: Text.PlainText; color: "#F3C8BA"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    UiButton { text: "Tentar checkpoint"; compact: true; visible: root.backend.canRetryCheckpoint; onClicked: root.backend.retryCheckpoint() }
                }
            }
            Rectangle {
                visible: root.backend.resultPending
                Layout.fillWidth: true
                implicitHeight: 70
                radius: 12; color: "#442C28"; border.color: "#925246"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 14
                    Label { text: "Finalização pendente. Tente salvar novamente; o checkpoint confirmado é preservado."; color: "#F3C8BA"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 12 }
                    UiButton { text: "Tentar salvar"; compact: true; onClicked: root.backend.retrySaveResult() }
                }
            }
            Flickable {
                id: scroll
                objectName: "pageScroll"
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick
                contentWidth: width
                contentHeight: screen.height
                ScrollBar.vertical: ScrollBar { }
                Loader {
                    id: screen
                    readonly property Item currentView: item as Item
                    width: scroll.width
                    height: Math.max(scroll.height, currentView ? currentView.implicitHeight : 0)
                    onLoaded: { scroll.contentY = 0; screenFade.restart() }
                    sourceComponent: {
                        switch (root.page) {
                        case "categories": return categoriesPage
                        case "taxonomy": return taxonomyPage
                        case "quiz": return quizPage
                        case "feedback": return feedbackPage
                        case "info": return infoPage
                        case "results": return resultsPage
                        case "statistics": return statisticsPage
                        case "achievements": return achievementsPage
                        case "settings": return settingsPage
                        case "credits": return creditsPage
                        default: return homePage
                        }
                    }
                }
            }
        }
    }
    NumberAnimation { id: screenFade; target: screen; property: "opacity"; from: 0; to: 1; duration: root.motionEnabled ? 180 : 0 }
    Component { id: homePage; HomeView { app: root; backend: root.backend } }
    Component { id: categoriesPage; CategoryView { app: root; backend: root.backend } }
    Component { id: taxonomyPage; TaxonomyView { app: root; backend: root.backend } }
    Component { id: quizPage; QuizView { app: root; backend: root.backend } }
    Component { id: feedbackPage; FeedbackView { app: root; backend: root.backend } }
    Component { id: infoPage; InfoView { app: root; backend: root.backend } }
    Component { id: resultsPage; ResultsView { app: root; backend: root.backend } }
    Component { id: statisticsPage; StatisticsView { app: root; backend: root.backend } }
    Component { id: achievementsPage; AchievementsView { app: root; backend: root.backend } }
    Component { id: settingsPage; SettingsView { app: root; backend: root.backend } }
    Component { id: creditsPage; CreditsView { app: root; backend: root.backend } }
}
