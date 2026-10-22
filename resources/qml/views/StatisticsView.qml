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
    readonly property string rankingContext: backend.rankingMode === "competitive" ? "Competitivo atual" : "Livre"
    readonly property int mainColumns: width >= 900 ? 4 : 2
    readonly property int metricColumns: width >= 690 ? 3 : 2
    readonly property bool sideBySide: width >= 900
    implicitHeight: statisticsLayout.implicitHeight

    function count(value, singular, plural) { return value + " " + (value === 1 ? singular : plural) }

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
            Layout.fillWidth: true
            spacing: Style.Theme.spaceXs
            Heading { text: "Suas estatísticas"; font.pixelSize: Style.Theme.titleSize }
            Note { text: "Acompanhe a partida e consulte o histórico do perfil, sem misturar modalidades." }
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
                    text: view.count(view.session.hintedQuestions, "questão com dicas", "questões com dicas") + " · "
                          + view.count(view.session.revealedCategories, "dica revelada", "dicas reveladas")
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
                            readonly property int count: view.session[modelData.key]
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
                Note { text: "Categorias das dicas usadas em respostas confirmadas; não indicam níveis de assistência." }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Style.Theme.spaceSm
            Heading { text: "Histórico do perfil" }
            Note { objectName: "statisticsHistoryContext"; text: view.backend.currentPlayerName + " · " + view.rankingContext + ". Partidas concluídas com respostas detalhadas disponíveis." }
            Note { text: "Registros antigos podem não ter detalhamento. Estes indicadores não representam necessariamente toda a atividade do perfil." }
            Note { visible: !!view.backend.identityNotice; text: view.backend.identityNotice; color: Style.Theme.gold }
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

        GridLayout {
            Layout.fillWidth: true
            columns: view.sideBySide ? 2 : 1
            columnSpacing: Style.Theme.spaceLg
            rowSpacing: Style.Theme.spaceLg
            Rectangle {
                objectName: "statisticsTaxonomyPanel"
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
                            Layout.fillWidth: true
                            spacing: Style.Theme.spaceSm
                            RowLayout {
                                Layout.fillWidth: true
                                KingdomArtwork { objectName: "performanceIcon" + group.index; kingdom: group.modelData.kingdom; implicitWidth: 38; implicitHeight: 38 }
                                Heading { text: group.modelData.title; font.pixelSize: Style.Theme.bodySize }
                                Label { text: group.modelData.answered ? Math.floor(100 * group.modelData.correct / group.modelData.answered) + "%" : "—"; color: Style.Theme.green; font.pixelSize: Style.Theme.bodySize }
                            }
                            Note { text: view.count(group.modelData.correct, "acerto", "acertos") + " em " + view.count(group.modelData.answered, "resposta", "respostas") }
                            AccuracyBar { ratio: group.accuracy }
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
                                    AccuracyBar { ratio: phylum.modelData.answered > 0 ? phylum.modelData.correct / phylum.modelData.answered : 0 }
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
                objectName: "statisticsRankingPanel"
                Layout.fillWidth: true
                Layout.preferredWidth: view.sideBySide ? view.width * 0.44 : view.width
                Layout.alignment: Qt.AlignTop
                implicitHeight: rankingLayout.implicitHeight + 2 * Style.Theme.spaceLg
                radius: Style.Theme.cardRadius
                color: Style.Theme.surface
                border.color: Style.Theme.border
                ColumnLayout {
                    id: rankingLayout
                    anchors.fill: parent
                    anchors.margins: Style.Theme.spaceLg
                    spacing: Style.Theme.spaceMd
                    RowLayout {
                        Layout.fillWidth: true
                        UiIcon { name: "trophy"; ink: Style.Theme.gold; implicitWidth: 22; implicitHeight: 22 }
                        Heading { text: "Ranking · " + view.rankingContext }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        UiButton { objectName: "freeRankingButton"; text: "Livre"; compact: true; checkable: true; checked: view.backend.rankingMode === "free"; secondary: !checked; Accessible.description: "Histórico e ranking do modo Livre"; onClicked: view.backend.rankingMode = "free" }
                        UiButton { objectName: "competitiveRankingButton"; text: "Competitivo"; compact: true; checkable: true; checked: view.backend.rankingMode === "competitive"; secondary: !checked; Accessible.description: "Histórico e ranking competitivo da configuração atual"; onClicked: view.backend.rankingMode = "competitive" }
                    }
                    Note { text: "Melhor pontuação de cada jogador · até cinco posições." }
                    Note { text: view.backend.rankingMode === "competitive" ? view.backend.competitiveQuestionCount + " questões · gênero · equilibrado · conteúdo atual" : "Treinamento contínuo; grupos e durações variáveis." }
                    Note {
                        objectName: "statisticsPersonalRecord"
                        visible: !!view.backend.personalRecord.scoreText
                        text: "Seu recorde: " + (view.backend.personalRecord.scoreText || "0,00") + " pts · Última: " + (view.backend.personalRecord.lastScoreText || "0,00") + " pts"
                        color: Style.Theme.green
                    }
                    Note { objectName: "statisticsRankingEmpty"; visible: view.backend.ranking.length === 0; text: "Nenhuma partida classificada nesta modalidade. Encerre uma partida elegível para entrar no ranking." }
                    // The backend bounds this model to five rows. Let the page scroll
                    // handle their full intrinsic height, including long player names.
                    Repeater {
                        model: view.backend.ranking
                        delegate: FocusScope {
                            id: rankRow
                            required property var modelData
                            required property int index
                            objectName: "statisticsRank" + index
                            Layout.fillWidth: true
                            implicitHeight: rankContent.implicitHeight + 2 * Style.Theme.spaceMd
                            activeFocusOnTab: true
                            Accessible.role: Accessible.ListItem
                            Accessible.name: (index + 1) + ". " + modelData.playerName + ", " + modelData.scoreText + " pontos"
                            Accessible.description: rankDetail.text + ". Nome na partida mais recente: " + modelData.snapshotName
                            Rectangle {
                                anchors.fill: parent
                                radius: Style.Theme.controlRadius
                                color: rankRow.index === 0 ? Style.Theme.forest : Style.Theme.surfaceRaised
                                border.width: rankRow.activeFocus ? Style.Theme.focusWidth : Style.Theme.borderWidth
                                border.color: rankRow.activeFocus ? Style.Theme.focus : rankRow.index < 3 ? Style.Theme.gold : Style.Theme.border
                            }
                            RowLayout {
                                id: rankContent
                                anchors.fill: parent
                                anchors.margins: Style.Theme.spaceMd
                                spacing: Style.Theme.spaceSm
                                Label { text: String(rankRow.index + 1).padStart(2, "0"); color: rankRow.index < 3 ? Style.Theme.gold : Style.Theme.textMuted; font.pixelSize: Style.Theme.subtitleSize; font.weight: Font.DemiBold }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: Style.Theme.spaceXs
                                    Label { text: rankRow.modelData.playerName; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WrapAnywhere; textFormat: Text.PlainText; Layout.fillWidth: true }
                                    Note { id: rankDetail; text: (rankRow.modelData.legacyIdentity ? "Legado, vínculo pendente · " : rankRow.modelData.deletedProfile ? "Perfil excluído · " : "") + "Última: " + rankRow.modelData.lastScoreText + " pts · " + rankRow.modelData.lastPlayedAt }
                                    Label { text: rankRow.modelData.scoreText + " pts"; color: Style.Theme.green; font.pixelSize: Style.Theme.bodySize; font.weight: Font.DemiBold }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
