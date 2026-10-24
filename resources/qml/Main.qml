pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "views"
import "style" as Style

ApplicationWindow {
    id: root
    objectName: "mainWindow"
    visible: true
    width: 1240
    height: 820
    minimumWidth: 1000
    minimumHeight: 700
    title: "Projeto Glossata"
    color: Style.Theme.canvas
    font.family: Style.Theme.fontFamily
    font.pixelSize: Style.Theme.bodySize
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
    property string taxonomyContext: "play"
    // Ephemeral navigation presentation only; no player data is persisted here.
    property var explorePresentation: ({})
    property var playPresentation: ({})
    property var statisticsPresentation: ({panel: "taxonomy"})
    property var achievementsPresentation: ({state: "Todas", category: "Todas as categorias"})
    readonly property bool compactNavigation: width < 1150
    readonly property string currentSection: page === "explore" || (page === "taxonomy" && taxonomyContext === "explore")
                                             ? "explore" : (["categories", "taxonomy", "quiz", "feedback", "info", "results"].indexOf(page) >= 0
                                             ? "play" : (page === "credits" ? "settings" : page))
    property bool motionEnabled: true
    onMotionEnabledChanged: Style.Theme.motionEnabled = motionEnabled
    Component.onCompleted: Style.Theme.motionEnabled = motionEnabled
    property bool sessionStarted: false
    property string lastGamePage: "quiz"
    onPageChanged: {
        if (["quiz", "feedback", "info", "results"].indexOf(page) >= 0) lastGamePage = page
    }
    property bool forceClose: false
    property string abandonedSessionId: ""
    readonly property real accuracy: backend.answeredCount > 0 ? Math.round(100 * backend.correctAnswers / backend.answeredCount) : 0
    readonly property bool playing: sessionStarted && !backend.gameFinished
    readonly property var pageTitles: ({home: "Início", explore: "Explorar", categories: "Jogar", taxonomy: taxonomyContext === "explore" ? "Explorar" : "Jogar", quiz: "Em jogo", feedback: "Continue descobrindo", info: "Ficha da espécie", results: "Resultado da partida", statistics: "Estatísticas", achievements: "Conquistas", settings: "Perfil e ajustes", credits: "Créditos e transparência"})
    readonly property string sectionContext: page === "taxonomy" ? (backend.browseTaxon.title || "Grupos taxonômicos")
                                           : currentSection === "explore" ? "Domínios e linhagens · navegação científica"
                                           : currentSection === "play" ? (playing ? "Partida em andamento · progresso salvo por checkpoint" : "Modos Livre e Competitivo")
                                           : page === "home" ? "Aprenda. Explore. Descubra." : "Dados locais · funcionamento offline"

    property string transitionPlayerId: ""
    Dialog {
        id: localUserTransition
        objectName: "localUserTransition"
        anchors.centerIn: parent
        width: Math.min(620, root.width - 48)
        modal: true
        closePolicy: Popup.NoAutoClose
        visible: root.backend.localUserChoiceRequired
        title: "Definir usuário deste computador"
        background: Rectangle { color: Style.Theme.surface; radius: Style.Theme.cardRadius; border.color: Style.Theme.border }
        contentItem: ColumnLayout {
            spacing: Style.Theme.spaceMd
            Label { Layout.fillWidth: true; text: "Esta instalação tem mais de um perfil antigo. Escolha qual identidade continuará como seu usuário local. A escolha é única e será mantida ao reabrir o aplicativo."; color: Style.Theme.text; wrapMode: Text.WordWrap }
            Label { Layout.fillWidth: true; text: "Os resultados e as sessões das outras identidades serão preservados, sem transferência para o usuário escolhido."; color: Style.Theme.textMuted; wrapMode: Text.WordWrap }
            ScrollView {
                Layout.fillWidth: true
                implicitHeight: Math.min(280, candidateList.implicitHeight)
                clip: true
                ColumnLayout {
                    id: candidateList
                    width: parent.width
                    Repeater {
                        model: root.backend.localUserCandidates
                        delegate: ColumnLayout {
                            id: candidateRow
                            required property var modelData
                            Layout.fillWidth: true
                            UiButton { objectName: "transitionChoice-" + candidateRow.modelData.playerId; text: candidateRow.modelData.name; Layout.fillWidth: true; checkable: true; checked: root.transitionPlayerId === candidateRow.modelData.playerId; secondary: !checked; onClicked: root.transitionPlayerId = candidateRow.modelData.playerId }
                            Label { Layout.fillWidth: true; text: "ID local: " + candidateRow.modelData.playerId; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WrapAnywhere }
                        }
                    }
                }
            }
            UiButton { objectName: "confirmLocalUser"; text: "Definir usuário local"; enabled: root.transitionPlayerId !== ""; onClicked: root.backend.chooseLocalUser(root.transitionPlayerId) }
        }
    }
    Dialog {
        objectName: "localUserUnavailable"
        anchors.centerIn: parent
        width: Math.min(560, root.width - 48)
        modal: true
        closePolicy: Popup.NoAutoClose
        visible: !root.backend.localUserReady && !root.backend.localUserChoiceRequired
        title: "Identidade local indisponível"
        contentItem: ColumnLayout {
            Label { Layout.fillWidth: true; text: root.backend.localUserError; color: Style.Theme.text; wrapMode: Text.WordWrap }
            UiButton { text: "Fechar aplicativo"; onClicked: { root.forceClose = true; root.close() } }
        }
    }

    function navigate(destination) { page = destination }
    // Keep keyboard focus visible inside the scrolling page without moving
    // the sidebar, header or modal dialogs.
    onActiveFocusItemChanged: Qt.callLater(revealFocusedControl)
    function revealFocusedControl() {
        const control = root.activeFocusItem
        if (!control || !control.visible) return
        let ancestor = control.parent
        while (ancestor && ancestor !== screen) ancestor = ancestor.parent
        if (!ancestor) return
        wheelScroll.stop()
        const top = control.mapToItem(scroll, 0, 0).y
        const bottom = top + control.height
        const margin = Style.Theme.spaceSm
        let offset = scroll.contentY
        if (top < margin) offset += top - margin
        else if (bottom > scroll.height - margin) offset += bottom - scroll.height + margin
        scroll.contentY = Math.max(0, Math.min(offset, Math.max(0, scroll.contentHeight - scroll.height)))
    }
    function openExplore() { taxonomyContext = "explore"; page = "explore" }
    function openPlay() {
        if (!backend.localUserReady) return
        taxonomyContext = "play"
        if (backend.resultPending) page = "results"
        else if (playing) page = backend.answerPending ? (lastGamePage === "info" ? "info" : "feedback") : "quiz"
        else if (sessionStarted && backend.gameFinished) page = ["feedback", "info", "results"].indexOf(lastGamePage) >= 0 ? lastGamePage : "results"
        else page = "categories"
    }
    function beginGame(family) {
        if (!backend.startCategoryGame(family)) return
        sessionStarted = !backend.gameFinished
        page = backend.gameFinished ? "results" : "quiz"
    }
    function openTaxon(id) {
        if (page !== "taxonomy") taxonomyContext = page === "explore" ? "explore" : "play"
        backend.exploreTaxon(id); page = "taxonomy"
    }
    function beginTaxonGame(id, rank) {
        if (!backend.startTaxonGame(id, rank)) return
        sessionStarted = !backend.gameFinished
        page = backend.gameFinished ? "taxonomy" : "quiz"
    }
    function returnToExploration() {
        if (backend.gameFinished) sessionStarted = false
        taxonomyContext = "play"
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
            objectName: "navigationSidebar"
            Layout.preferredWidth: root.compactNavigation ? Style.Theme.sidebarCompactWidth : Style.Theme.sidebarWidth
            Layout.fillHeight: true
            color: Style.Theme.surface
            border.color: Style.Theme.border
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Style.Theme.spaceMd
                spacing: Style.Theme.spaceXs
                RowLayout {
                    Layout.topMargin: Style.Theme.spaceLg
                    Layout.bottomMargin: Style.Theme.spaceLg
                    spacing: Style.Theme.spaceSm
                    Rectangle {
                        implicitWidth: 36; implicitHeight: 36; radius: 12; color: "#174D3A"
                        Image { anchors.fill: parent; anchors.margins: 2; source: "qrc:/resources/branding/glossata.png"; fillMode: Image.PreserveAspectFit; smooth: true; mipmap: true }
                    }
                    ColumnLayout {
                        spacing: 0
                        Label { text: "Projeto Glossata"; color: "#F5FAF7"; font.pixelSize: Style.Theme.brandSize; font.weight: Font.DemiBold }
                        Label { text: "APRENDA. EXPLORE."; color: "#739399"; font.pixelSize: 8; font.letterSpacing: 1.5 }
                    }
                }
                Label { text: "DESCOBRIR"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.6; Layout.leftMargin: Style.Theme.navPadding; Layout.bottomMargin: Style.Theme.spaceXs }
                NavItem { objectName: "navHome"; Layout.fillWidth: true; text: "Início"; iconName: "home"; selected: root.currentSection === "home"; onClicked: root.navigate("home") }
                NavItem { objectName: "navExplore"; Layout.fillWidth: true; text: "Explorar"; iconName: "book"; selected: root.currentSection === "explore"; onClicked: root.openExplore() }
                NavItem { objectName: "navPlay"; Layout.fillWidth: true; text: "Jogar"; iconName: "play"; selected: root.currentSection === "play"; onClicked: root.openPlay() }
                NavItem { objectName: "navStatistics"; Layout.fillWidth: true; text: "Estatísticas"; iconName: "chart"; selected: root.currentSection === "statistics"; onClicked: root.navigate("statistics") }
                NavItem { objectName: "navAchievements"; Layout.fillWidth: true; text: "Conquistas"; iconName: "trophy"; selected: root.currentSection === "achievements"; onClicked: root.navigate("achievements") }
                Item { Layout.preferredHeight: Style.Theme.spaceMd }
                Label { text: "SEU ESPAÇO"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; font.letterSpacing: 1.6; Layout.leftMargin: Style.Theme.navPadding; Layout.bottomMargin: Style.Theme.spaceXs }
                NavItem { objectName: "navSettings"; Layout.fillWidth: true; text: "Perfil e ajustes"; iconName: "settings"; selected: root.currentSection === "settings"; onClicked: root.navigate("settings") }
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 80
                    radius: Style.Theme.controlRadius
                    color: Style.Theme.surfaceRaised
                    border.color: Style.Theme.border
                    RowLayout {
                        anchors.fill: parent; anchors.margins: Style.Theme.spaceMd; spacing: Style.Theme.spaceSm
                        Rectangle { implicitWidth: 32; implicitHeight: 32; radius: Style.Theme.cardRadius; color: Style.Theme.forest; UiIcon { anchors.centerIn: parent; name: "user"; ink: Style.Theme.green; implicitWidth: 18; implicitHeight: 18 } }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Label { text: root.backend.currentPlayerName || "Definição pendente"; color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize; font.weight: Font.DemiBold; elide: Text.ElideRight; Layout.fillWidth: true }
                            Label { text: "Usuário local"; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize }
                        }
                    }
                }
                Label { text: "Natureza em cada descoberta."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.sectionLabelSize; Layout.alignment: Qt.AlignHCenter; Layout.topMargin: Style.Theme.spaceXs; Layout.bottomMargin: Style.Theme.spaceXs }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: root.compactNavigation ? Style.Theme.pageCompactMargin : Style.Theme.pageMargin
            spacing: Style.Theme.spaceLg
            RowLayout {
                objectName: "sectionHeader"
                Layout.fillWidth: true
                spacing: Style.Theme.spaceMd
                ColumnLayout {
                    Layout.fillWidth: true; spacing: Style.Theme.spaceXs
                    Label { objectName: "sectionTitle"; text: root.pageTitles[root.page] || "Projeto Glossata"; color: Style.Theme.text; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold; wrapMode: Text.WordWrap; Layout.fillWidth: true; Accessible.role: Accessible.Heading }
                    Label { objectName: "sectionContext"; text: root.sectionContext; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
                TextAction { objectName: "headerResumeGame"; visible: root.playing && root.currentSection !== "play"; text: "Retomar partida"; onClicked: root.openPlay() }
                Rectangle {
                    implicitWidth: 116; implicitHeight: 28; radius: Style.Theme.controlRadius; color: Style.Theme.forest; border.color: Style.Theme.border
                    Row { anchors.centerIn: parent; spacing: Style.Theme.spaceXs; Rectangle { implicitWidth: 5; implicitHeight: 5; radius: 3; color: Style.Theme.green; anchors.verticalCenter: parent.verticalCenter } Label { text: "Dados locais"; color: Style.Theme.text; font.pixelSize: Style.Theme.sectionLabelSize } }
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
                // Reserve the same gutter on every page, even when the bar is
                // hidden. Content and focus rings never extend under its track.
                readonly property real contentGutter: Math.max(Style.Theme.spaceSm, verticalBar.implicitWidth) + Style.Theme.spaceMd
                readonly property real wheelStep: 110
                property int wheelDirection: 0
                function conventionalWheel(delta) {
                    const direction = Math.sign(delta)
                    const start = wheelScroll.running && direction === wheelDirection ? wheelScroll.to : contentY
                    const destination = Math.max(0, Math.min(contentHeight - height, start - delta / 120 * wheelStep))
                    wheelScroll.stop(); cancelFlick(); wheelDirection = direction
                    if (Style.Theme.motionEnabled) {
                        wheelScroll.from = contentY; wheelScroll.to = destination; wheelScroll.start()
                    } else contentY = destination
                }
                onMovementStarted: wheelScroll.stop()
                ScrollBar.vertical: ScrollBar {
                    id: verticalBar; objectName: "pageScrollBar"
                    onPressedChanged: if (pressed) wheelScroll.stop()
                }
                MouseArea {
                    objectName: "pageWheelArea"
                    // A sibling before the Loader: after its controls in hit
                    // testing, but before Flickable's native wheel handling.
                    parent: scroll.contentItem
                    width: scroll.width; height: scroll.height; y: scroll.contentY
                    enabled: root.currentSection === "home" || root.currentSection === "explore" || root.currentSection === "statistics" || root.currentSection === "achievements"
                    acceptedButtons: Qt.NoButton
                    // Child controls receive events first. Synthesized gestures
                    // remain native; ordinary wheel events are accepted below
                    // only when this area actually changes the page position.
                    scrollGestureEnabled: false
                    onWheel: event => {
                        // Pixel deltas and partial/high-resolution rotations
                        // retain Flickable's native path, without quantization.
                        if (event.pixelDelta.x || event.pixelDelta.y || event.angleDelta.x || !event.angleDelta.y
                                || event.angleDelta.y % 120 !== 0 || event.modifiers !== Qt.NoModifier) {
                            wheelScroll.stop(); event.accepted = false; return
                        }
                        scroll.conventionalWheel(event.angleDelta.y)
                        event.accepted = true
                    }
                }
                NumberAnimation { id: wheelScroll; target: scroll; property: "contentY"; duration: Style.Theme.durationFast; easing.type: Easing.OutCubic }
                Loader {
                    id: screen
                    readonly property Item currentView: item as Item
                    width: Math.max(0, scroll.width - scroll.contentGutter)
                    height: Math.max(scroll.height, currentView ? currentView.implicitHeight : 0)
                    onLoaded: { wheelScroll.stop(); scroll.cancelFlick(); scroll.contentY = 0; screenFade.restart() }
                    sourceComponent: {
                        switch (root.page) {
                        case "explore": return explorePage
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
    AchievementNotificationLayer {
        anchors.fill: parent
        z: 100
        app: root; backend: root.backend
        surfaceAvailable: !root.Overlay.overlay.visible
    }
    NumberAnimation { id: screenFade; target: screen; property: "opacity"; from: 0; to: 1; duration: Style.Theme.durationPage }
    Component { id: homePage; HomeView { app: root; backend: root.backend } }
    Component { id: categoriesPage; CategoryView { app: root; backend: root.backend } }
    Component { id: explorePage; CategoryView { app: root; backend: root.backend; browsingOnly: true } }
    Component { id: taxonomyPage; TaxonomyView { app: root; backend: root.backend; browsingOnly: root.taxonomyContext === "explore" } }
    Component { id: quizPage; QuizView { app: root; backend: root.backend } }
    Component { id: feedbackPage; FeedbackView { app: root; backend: root.backend } }
    Component { id: infoPage; InfoView { app: root; backend: root.backend } }
    Component { id: resultsPage; ResultsView { app: root; backend: root.backend } }
    Component { id: statisticsPage; StatisticsView { app: root; backend: root.backend } }
    Component { id: achievementsPage; AchievementsView { app: root; backend: root.backend } }
    Component { id: settingsPage; SettingsView { app: root; backend: root.backend } }
    Component { id: creditsPage; CreditsView { app: root; backend: root.backend } }
}
