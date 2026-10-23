pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

Item {
    id: view
    objectName: "statisticsView"
    required property var app
    required property var backend
    readonly property var session: backend.sessionMetrics
    readonly property var history: backend.playerMetrics
    // Read-only presentation: confirmed metrics plus the active unanswered question.
    // Feedback still shows that question's hints after they have been consolidated.
    readonly property var visualHints: {
        const totals = { hintedQuestions: session.hintedQuestions, revealedCategories: session.revealedCategories,
                         taxonomic: session.taxonomic, etymological: session.etymological, morphological: session.morphological }
        if (backend.gameFinished || backend.answerPending) return totals
        const seen = new Set()
        for (const hint of backend.hints || []) {
            if (!hint.available || !hint.revealed || seen.has(hint.id)) continue
            seen.add(hint.id)
            ++totals.revealedCategories
            if (["taxonomic", "etymological", "morphological"].includes(hint.category)) ++totals[hint.category]
        }
        if (seen.size > 0) ++totals.hintedQuestions
        return totals
    }
    readonly property var groupPalette: [Style.Theme.green, "#8ABCF0", "#74D3CB", Style.Theme.gold, "#E4A77D", "#BDA5DE"]
    readonly property string rankingContext: backend.rankingMode === "competitive" ? "Competitivo atual" : "Livre"
    readonly property int mainColumns: width >= 900 ? 4 : 2
    readonly property int metricColumns: width >= 690 ? 3 : 2
    readonly property bool sideBySide: width >= 900
    readonly property string selectedPanel: app.statisticsPresentation.panel || "taxonomy"
    implicitHeight: statisticsLayout.implicitHeight

    function count(value, singular, plural) { return value + " " + (value === 1 ? singular : plural) }
    function groupColor(id) {
        // Catalog IDs, independent of names, ranking, counts and delegate order.
        const mapped = { "taxon:56956496cf600779dc1f": groupPalette[0], // Plantae
                         "domain:Bacteria": groupPalette[1],
                         "clade:SAR": groupPalette[2],
                         "taxon:31fdb10b7300d45a9dcf": groupPalette[3], // Fungi
                         "taxon:cac47417164883d60794": groupPalette[4], // Animalia
                         "clade:Amoebozoa": groupPalette[5],
                         "clade:Discoba": "#A6C4E8" }
        if (Object.prototype.hasOwnProperty.call(mapped, id)) return mapped[id]
        let hash = 0
        for (let i = 0; i < id.length; ++i) hash = (hash * 31 + id.charCodeAt(i)) >>> 0
        return groupPalette[hash % groupPalette.length]
    }
    function podiumColor(position) {
        return position === 0 ? Style.Theme.gold : position === 1 ? "#C6D0DA" : position === 2 ? "#D6A27B" : Style.Theme.border
    }
    function selectPanel(panel) { app.statisticsPresentation = {panel: panel} }
    function revealPanelFocus() {
        if (sideBySide) return
        let focused = app.activeFocusItem
        while (focused && focused !== view) {
            if (selectedPanel === "ranking" && focused === taxonomyPanel) { rankingTab.forceActiveFocus(Qt.TabFocusReason); return }
            if (selectedPanel === "taxonomy" && focused === rankingPanel) { taxonomyTab.forceActiveFocus(Qt.TabFocusReason); return }
            focused = focused.parent
        }
    }
    onSideBySideChanged: Qt.callLater(revealPanelFocus)
    onSelectedPanelChanged: Qt.callLater(revealPanelFocus)

    component Heading: Label {
        color: Style.Theme.text
        font.pixelSize: Style.Theme.subtitleSize
        font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Accessible.role: Accessible.Heading
    }
    component Note: Label {
        color: Style.Theme.textMuted
        font.pixelSize: Style.Theme.captionSize
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
        Layout.fillWidth: true
    }
    component AccuracyBar: Rectangle {
        property real ratio: 0
        property color accent: Style.Theme.green
        Layout.fillWidth: true
        implicitHeight: 6
        radius: 3
        color: Style.Theme.surfaceRaised
        Accessible.ignored: true // The adjacent text includes the actual counts and rate.
        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, parent.ratio))
            height: parent.height
            radius: parent.radius
            color: parent.accent
        }
    }

    ColumnLayout {
        id: statisticsLayout
        width: parent.width
        spacing: Style.Theme.spaceLg
        ColumnLayout {
            objectName: "statisticsHeader"
            Layout.fillWidth: true
            spacing: Style.Theme.spaceXs
            Heading { text: "Suas estatísticas"; font.pixelSize: Style.Theme.titleSize }
            Note { text: "Acompanhe a partida e consulte o histórico do perfil, sem misturar modalidades." }
        }

        RowLayout {
            objectName: "statisticsPanelSelector"
            visible: !view.sideBySide
            Layout.fillWidth: true
            spacing: Style.Theme.spaceMd
            UiButton {
                id: taxonomyTab
                objectName: "statisticsTaxonomyTab"
                text: "Desempenho taxonômico"
                compact: true
                checkable: true
                autoExclusive: true
                checked: view.selectedPanel === "taxonomy"
                secondary: !checked
                Accessible.role: Accessible.PageTab
                Accessible.description: "Mostrar os grupos da partida atual"
                onClicked: view.selectPanel("taxonomy")
            }
            UiButton {
                id: rankingTab
                objectName: "statisticsRankingTab"
                text: "Recordes pessoais"
                compact: true
                checkable: true
                autoExclusive: true
                checked: view.selectedPanel === "ranking"
                secondary: !checked
                Accessible.role: Accessible.PageTab
                Accessible.description: "Mostrar recordes pessoais locais da modalidade selecionada"
                onClicked: view.selectPanel("ranking")
            }
            Item { Layout.fillWidth: true }
        }

        GridLayout {
            objectName: "statisticsTopPanels"
            Layout.fillWidth: true
            columns: view.sideBySide ? 2 : 1
            columnSpacing: Style.Theme.spaceLg
            rowSpacing: Style.Theme.spaceLg
            Rectangle {
                id: taxonomyPanel
                objectName: "statisticsTaxonomyPanel"
                visible: view.sideBySide || view.selectedPanel === "taxonomy"
                Layout.fillWidth: true
                Layout.preferredWidth: view.sideBySide ? view.width * 0.56 : view.width
                Layout.alignment: Qt.AlignTop
                implicitHeight: performanceLayout.implicitHeight + 2 * Style.Theme.spaceLg
                color: Style.Theme.surface
                radius: Style.Theme.cardRadius
                border.color: Style.Theme.border
                ColumnLayout {
                    id: performanceLayout
                    anchors.fill: parent
                    anchors.margins: Style.Theme.spaceLg
                    spacing: Style.Theme.spaceMd
                    Heading { text: "Desempenho taxonômico" }
                    Note { text: "Somente a partida atual. Grupos e subgrupos das respostas confirmadas." }
                    Note { objectName: "statisticsTaxonomyEmpty"; visible: view.backend.taxonomicStats.length === 0; text: "Nenhum grupo com respostas nesta partida. Jogue para acompanhar seu desempenho aqui." }
                    Repeater {
                        model: view.backend.taxonomicStats
                        delegate: ColumnLayout {
                            id: group
                            required property var modelData
                            required property int index
                            objectName: "statisticsTaxon-" + modelData.id
                            readonly property real accuracy: modelData.answered > 0 ? modelData.correct / modelData.answered : 0
                            readonly property color accent: view.groupColor(modelData.id)
                            Layout.fillWidth: true
                            spacing: Style.Theme.spaceSm
                            RowLayout {
                                Layout.fillWidth: true
                                KingdomArtwork { objectName: "performanceIcon" + group.index; kingdom: group.modelData.kingdom; implicitWidth: 38; implicitHeight: 38 }
                                Heading { text: group.modelData.title; font.pixelSize: Style.Theme.bodySize }
                                Label { text: group.modelData.answered ? Math.floor(100 * group.modelData.correct / group.modelData.answered) + "%" : "—"; color: group.accent; font.pixelSize: Style.Theme.bodySize }
                            }
                            Note { text: view.count(group.modelData.correct, "acerto", "acertos") + " em " + view.count(group.modelData.answered, "resposta", "respostas") }
                            AccuracyBar { objectName: "statisticsTaxonBar-" + group.modelData.id; ratio: group.accuracy; accent: group.accent }
                            Note { visible: group.modelData.answered === 0; text: "Sem respostas neste grupo." }
                            Repeater {
                                model: group.modelData.phyla
                                delegate: ColumnLayout {
                                    id: phylum
                                    required property var modelData
                                    objectName: "statisticsSubtaxon-" + modelData.id
                                    Layout.fillWidth: true
                                    Layout.leftMargin: Style.Theme.spaceMd
                                    spacing: Style.Theme.spaceXs
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Note { text: phylum.modelData.title }
                                        Label { text: phylum.modelData.answered ? phylum.modelData.accuracy + "%" : "—"; color: Style.Theme.text; font.pixelSize: Style.Theme.captionSize }
                                    }
                                    AccuracyBar { objectName: "statisticsSubtaxonBar-" + phylum.modelData.id; ratio: phylum.modelData.answered > 0 ? phylum.modelData.correct / phylum.modelData.answered : 0; accent: Qt.tint(group.accent, "#18FFFFFF") }
                                    Note { text: phylum.modelData.answered ? view.count(phylum.modelData.correct, "acerto", "acertos") + " em " + view.count(phylum.modelData.answered, "resposta", "respostas") : "Sem respostas neste subgrupo." }
                                }
                            }
                            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Style.Theme.border; Layout.topMargin: Style.Theme.spaceXs }
                        }
                    }
                    UiButton { objectName: "statisticsPlayButton"; text: view.app.playing ? "Continuar partida" : "Começar uma partida"; iconName: "play"; compact: true; onClicked: view.app.openPlay() }
                }
            }

            Rectangle {
                id: rankingPanel
                objectName: "statisticsRankingPanel"
                visible: view.sideBySide || view.selectedPanel === "ranking"
                Layout.fillWidth: true
                Layout.preferredWidth: view.sideBySide ? view.width * 0.44 : view.width
                Layout.alignment: Qt.AlignTop
                implicitHeight: recordsLayout.implicitHeight + 2 * Style.Theme.spaceLg
                radius: Style.Theme.cardRadius
                color: Style.Theme.surface
                border.color: Style.Theme.border
                ColumnLayout {
                    id: recordsLayout
                    anchors.fill: parent
                    anchors.margins: Style.Theme.spaceLg
                    spacing: Style.Theme.spaceMd
                    RowLayout {
                        Layout.fillWidth: true
                        UiIcon { name: "trophy"; ink: Style.Theme.gold; implicitWidth: 22; implicitHeight: 22 }
                        Heading { text: "Recordes pessoais · " + view.rankingContext }
                    }
                    Note { text: "Somente seus resultados neste computador. Dados vinculados ao seu usuário local." }
                    RowLayout {
                        Layout.fillWidth: true
                        UiButton { objectName: "freeRankingButton"; text: "Livre"; compact: true; checkable: true; autoExclusive: true; checked: view.backend.rankingMode === "free"; secondary: !checked; onClicked: view.backend.rankingMode = "free" }
                        UiButton { objectName: "competitiveRankingButton"; text: "Competitivo"; compact: true; checkable: true; autoExclusive: true; checked: view.backend.rankingMode === "competitive"; secondary: !checked; onClicked: view.backend.rankingMode = "competitive" }
                    }
                    Note { text: view.backend.rankingMode === "competitive" ? view.backend.competitiveQuestionCount + " questões · gênero · equilibrado · regras e conteúdo atuais. Outras configurações não são comparadas." : "Livre: grupos e durações variáveis. O recorde não compara condições de partidas diferentes." }
                    Note { objectName: "statisticsRankingEmpty"; visible: !view.backend.personalRecord.scoreText; text: "Ainda não há recorde pessoal disponível nesta modalidade e configuração." }
                    FocusScope {
                        objectName: "statisticsPersonalRecordCard"
                        visible: !!view.backend.personalRecord.scoreText
                        Layout.fillWidth: true
                        implicitHeight: recordContent.implicitHeight + 2 * Style.Theme.spaceMd
                        activeFocusOnTab: true
                        Accessible.role: Accessible.ListItem
                        Accessible.name: "Recorde pessoal: " + (view.backend.personalRecord.scoreText || "0,00") + " pontos"
                        Rectangle { anchors.fill: parent; radius: Style.Theme.controlRadius; color: Style.Theme.surfaceRaised; border.width: parent.activeFocus ? Style.Theme.focusWidth : Style.Theme.borderWidth; border.color: parent.activeFocus ? Style.Theme.focus : Style.Theme.border }
                        ColumnLayout {
                            id: recordContent
                            anchors.fill: parent
                            anchors.margins: Style.Theme.spaceMd
                            spacing: Style.Theme.spaceSm
                            Note { text: "Melhor pontuação disponível" }
                            Label { objectName: "statisticsPersonalRecord"; text: (view.backend.personalRecord.scoreText || "0,00") + " pts"; color: Style.Theme.green; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold }
                            Note { text: (view.backend.personalRecord.correctAnswers || 0) + " acerto(s) · " + (view.backend.personalRecord.playedAt || "") }
                            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Style.Theme.border }
                            Note { objectName: "statisticsLastScore"; text: "Última pontuação: " + (view.backend.personalRecord.lastScoreText || "0,00") + " pts · " + (view.backend.personalRecord.lastPlayedAt || "") }
                        }
                    }
                    Note { text: "As pontuações são preservadas como registradas. Registros de outras identidades permanecem armazenados e não entram nos seus indicadores." }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            Heading { text: "Partida atual" }
            Note {
                objectName: "statisticsSessionContext"
                text: view.backend.answeredCount === 0 ? "Nenhuma resposta confirmada nesta partida."
                      : view.backend.sessionPlayerName + " · " + (view.backend.gameFinished ? "Partida encerrada" : "Partida em andamento")
                        + " · " + (view.backend.gameMode === "competitive" ? "Competitivo" : "Livre")
                        + ". Somente respostas confirmadas."
            }
            GridLayout {
                objectName: "statisticsMainIndicators"
                Layout.fillWidth: true
                columns: view.mainColumns
                columnSpacing: Style.Theme.spaceMd
                rowSpacing: Style.Theme.spaceMd
                StatCard { objectName: "statisticsAnswered"; Layout.fillWidth: true; title: "Perguntas respondidas"; value: String(view.backend.answeredCount); iconName: "book" }
                StatCard { objectName: "statisticsCorrect"; Layout.fillWidth: true; title: "Acertos"; value: String(view.backend.correctAnswers); iconName: "check" }
                StatCard { objectName: "statisticsIncorrect"; Layout.fillWidth: true; title: "Erros"; value: String(view.backend.answeredCount - view.backend.correctAnswers); iconName: "cross"; accent: Style.Theme.gold }
                StatCard { objectName: "statisticsAccuracy"; Layout.fillWidth: true; title: "Taxa de acerto"; value: view.session.accuracy + "%"; iconName: "target"; accent: Style.Theme.gold }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            Heading { text: "Desempenho da partida" }
            GridLayout {
                Layout.fillWidth: true
                columns: view.metricColumns
                columnSpacing: Style.Theme.spaceMd
                rowSpacing: Style.Theme.spaceMd
                StatCard { objectName: "statisticsAverage"; Layout.fillWidth: true; Layout.fillHeight: true; title: "Média por resposta"; value: view.session.average; description: "Pontos por resposta confirmada."; iconName: "chart" }
                StatCard { objectName: "statisticsEfficiency"; Layout.fillWidth: true; Layout.fillHeight: true; title: "Eficiência em pontos"; value: view.session.efficiency + "%"; description: "Pontos obtidos em relação ao máximo."; iconName: "target"; accent: Style.Theme.gold }
                StatCard { objectName: "statisticsUnassisted"; Layout.fillWidth: true; Layout.fillHeight: true; title: "Acertos sem dicas"; value: String(view.session.unassistedCorrect); description: "Respostas corretas sem ajuda."; iconName: "check" }
            }
        }

        Rectangle {
            objectName: "statisticsHintsPanel"
            Layout.fillWidth: true
            implicitHeight: hintsLayout.implicitHeight + 2 * Style.Theme.spaceLg
            color: Style.Theme.surface
            radius: Style.Theme.cardRadius
            border.color: Style.Theme.border
            ColumnLayout {
                id: hintsLayout
                anchors.fill: parent
                anchors.margins: Style.Theme.spaceLg
                spacing: Style.Theme.spaceSm
                Heading { text: "Dicas na partida" }
                Note {
                    objectName: "hintMetricsLabel"
                    text: view.count(view.visualHints.hintedQuestions, "questão com dicas", "questões com dicas") + " · "
                          + view.count(view.visualHints.revealedCategories, "dica revelada", "dicas reveladas")
                }
                GridLayout {
                    Layout.fillWidth: true
                    columns: view.metricColumns
                    columnSpacing: Style.Theme.spaceMd
                    rowSpacing: Style.Theme.spaceSm
                    Repeater {
                        model: [ {key: "taxonomic", title: "Taxonômicas"}, {key: "etymological", title: "Etimológicas"}, {key: "morphological", title: "Morfológicas"} ]
                        delegate: Rectangle {
                            id: hintCategory
                            required property var modelData
                            objectName: "statisticsHint-" + modelData.key
                            readonly property int count: view.visualHints[modelData.key]
                            Layout.fillWidth: true
                            implicitHeight: hintCategoryRow.implicitHeight + 2 * Style.Theme.spaceSm
                            color: Style.Theme.surfaceRaised
                            radius: Style.Theme.controlRadius
                            Accessible.role: Accessible.StaticText
                            Accessible.name: modelData.title + ": " + count
                            RowLayout {
                                id: hintCategoryRow
                                anchors.fill: parent
                                anchors.margins: Style.Theme.spaceSm
                                Note { text: hintCategory.modelData.title }
                                Label { text: String(hintCategory.count); color: Style.Theme.green; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold }
                            }
                        }
                    }
                }
                Note { objectName: "statisticsHintsNotice"; text: "Inclui dicas da pergunta em andamento. O histórico só registra respostas confirmadas. Categorias não indicam níveis de assistência." }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            Heading { text: "Histórico do usuário local" }
            Note { objectName: "statisticsHistoryContext"; text: view.backend.currentPlayerName + " · " + view.rankingContext + ". Partidas concluídas com respostas detalhadas disponíveis." }
            Note { text: "Registros antigos podem não ter detalhamento. Estes indicadores não representam necessariamente toda a atividade do perfil." }
            GridLayout {
                Layout.fillWidth: true
                columns: view.metricColumns
                columnSpacing: Style.Theme.spaceMd
                rowSpacing: Style.Theme.spaceMd
                StatCard { objectName: "statisticsHistoryAccuracy"; Layout.fillWidth: true; title: "Precisão no histórico"; value: view.history.accuracy + "%"; iconName: "target" }
                StatCard { objectName: "statisticsHistoryAverage"; Layout.fillWidth: true; title: "Média por resposta"; value: view.history.average; iconName: "chart" }
                StatCard { objectName: "statisticsHistoryEfficiency"; Layout.fillWidth: true; title: "Eficiência em pontos"; value: view.history.efficiency + "%"; iconName: "spark"; accent: Style.Theme.gold }
            }
            Note {
                objectName: "statisticsHistoryCounts"
                text: view.count(view.history.answered, "resposta histórica disponível", "respostas históricas disponíveis") + " · "
                      + view.count(view.history.unassistedCorrect, "acerto sem dicas", "acertos sem dicas") + " · "
                      + view.count(view.history.hintedQuestions, "questão com dicas", "questões com dicas")
            }
            Note { objectName: "statisticsHistoryHints"; text: "Dicas por categoria: taxonômicas " + view.history.taxonomic + " · etimológicas " + view.history.etymological + " · morfológicas " + view.history.morphological + "." }
            Note { objectName: "statisticsHistoryEmpty"; visible: view.history.answered === 0; text: "Sem respostas históricas detalhadas neste perfil e modalidade." }
        }
    }
}
