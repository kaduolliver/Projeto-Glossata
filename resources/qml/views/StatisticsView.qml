pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: view
    required property var app
    required property var backend
    implicitHeight: Math.max(950,statisticsLayout.implicitHeight)
    ColumnLayout {
        id: statisticsLayout
        anchors.fill: parent; spacing: 22
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true; spacing: 8
                Label { text: "Suas estatísticas"; color: "#EFF7F1"; font.pixelSize: 30; font.weight: Font.DemiBold }
                Label { text: "Cada descoberta conta. Veja como foi seu voo."; color: "#8CA9B1"; font.pixelSize: 13 }
            }
            Item { Layout.fillWidth: true }
            Rectangle { implicitWidth: 118; implicitHeight: 32; radius: 9; color: "#182E38"; border.color: "#2A444F"; Label { anchors.centerIn: parent; text: "Nesta partida"; color: "#AECDD0"; font.pixelSize: 11 } }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 16
            StatCard { Layout.fillWidth: true; title: "Perguntas respondidas"; value: String(view.backend.answeredCount); iconName: "book"; accent: "#6CABEE" }
            StatCard { Layout.fillWidth: true; title: "Acertos"; value: String(view.backend.correctAnswers); iconName: "check"; accent: "#45D080" }
            StatCard { Layout.fillWidth: true; title: "Taxa de acerto"; value: view.app.accuracy + "%"; iconName: "target"; accent: "#E7B857" }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 16
            StatCard { Layout.fillWidth: true; title: "Média por resposta"; value: view.backend.sessionMetrics.average; iconName: "chart"; accent: "#6CABEE" }
            StatCard { Layout.fillWidth: true; title: "Acertos sem dicas"; value: String(view.backend.sessionMetrics.unassistedCorrect); iconName: "check"; accent: "#45D080" }
            StatCard { Layout.fillWidth: true; title: "Eficiência em pontos"; value: view.backend.sessionMetrics.efficiency + "%"; iconName: "target"; accent: "#E7B857" }
        }
        Label { objectName: "hintMetricsLabel"; text: view.backend.sessionMetrics.hintedQuestions + " questões com dicas · " + view.backend.sessionMetrics.revealedCategories + " categorias reveladas\nTaxonômica: " + view.backend.sessionMetrics.taxonomic + " · Etimológica: " + view.backend.sessionMetrics.etymological + " · Morfológica: " + view.backend.sessionMetrics.morphological; color: "#91ACB4"; font.pixelSize: 12; lineHeight: 1.4; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Label { text: "Histórico do perfil · " + (view.backend.rankingMode === "competitive" ? "Competitivo atual" : "Livre"); color: "#E0EDE6"; font.pixelSize: 17; font.weight: Font.DemiBold }
        RowLayout {
            Layout.fillWidth: true; spacing: 16
            StatCard { Layout.fillWidth: true; title: "Precisão no histórico"; value: view.backend.playerMetrics.accuracy + "%"; iconName: "target"; accent: "#6CABEE" }
            StatCard { Layout.fillWidth: true; title: "Média por resposta"; value: view.backend.playerMetrics.average; iconName: "chart"; accent: "#45D080" }
            StatCard { Layout.fillWidth: true; title: "Eficiência em pontos"; value: view.backend.playerMetrics.efficiency + "%"; iconName: "spark"; accent: "#E7B857" }
        }
        Label { text: (view.backend.playerMetrics.answered || 0) + " respostas · " + (view.backend.playerMetrics.unassistedCorrect || 0) + " acertos sem dicas · " + (view.backend.playerMetrics.hintedQuestions || 0) + " questões com dicas\nCategorias: taxonômica " + (view.backend.playerMetrics.taxonomic || 0) + ", etimológica " + (view.backend.playerMetrics.etymological || 0) + ", morfológica " + (view.backend.playerMetrics.morphological || 0) + ". Inclui partidas concluídas com histórico detalhado disponível."; color: "#91ACB4"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; implicitHeight: performanceLayout.implicitHeight+48; radius: 18; color: "#10242D"; border.color: "#233E48"
                ColumnLayout {
                    id: performanceLayout
                    anchors.fill: parent; anchors.margins: 24; spacing: 18
                    Label { text: "Desempenho taxonômico"; color: "#E0EDE6"; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Label { visible: view.backend.categoryStats.length === 0; text: "Seu próximo voo começa com uma pergunta. Jogue para acompanhar seu desempenho aqui."; color: "#73939B"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 13; lineHeight: 1.5 }
                    ColumnLayout {
                        id: performanceGroups
                        Layout.fillWidth: true
                        spacing: 18
                        Repeater {
                        model: view.backend.taxonomicStats
                        delegate: ColumnLayout {
                            id: familyRow
                            required property var modelData
                            required property int index
                            readonly property color accent: ["#3CCD83", "#61A8E8", "#E5B858", "#B590E0"][index % 4]
                            Layout.fillWidth: true; spacing: 8
                            RowLayout {
                                Layout.fillWidth: true
                                KingdomArtwork { objectName: "performanceIcon" + familyRow.index; kingdom: familyRow.modelData.kingdom; implicitWidth: 38; implicitHeight: 38 }
                                Label { text: familyRow.modelData.title; color: "#D5E4DA"; font.pixelSize: 14; font.weight: Font.DemiBold; Layout.fillWidth: true }
                                Label { text: familyRow.modelData.correct + "/" + familyRow.modelData.answered; color: "#DAEAE1"; font.pixelSize: 12 }
                            }
                            Repeater {
                                model: familyRow.modelData.phyla
                                delegate: ColumnLayout {
                                    id: phylum
                                    required property var modelData
                                    Layout.fillWidth: true; spacing: 7
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: phylum.modelData.title; color: "#A7C0C5"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                        Label { text: phylum.modelData.accuracy + "%"; color: "#DAEAE1"; font.pixelSize: 12 }
                                    }
                                    Rectangle {
                                        Layout.fillWidth: true; implicitHeight: 7; radius: 4; color: "#203943"
                                        Rectangle { implicitHeight: 7; radius: 4; width: parent.width * phylum.modelData.accuracy / 100; color: familyRow.accent }
                                    }
                                    Label { text: phylum.modelData.correct + " acertos em " + phylum.modelData.answered + " respostas"; color: "#63858F"; font.pixelSize: 10 }
                                }
                            }
                        }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    UiButton { text: view.app.playing ? "Continuar partida" : "Começar uma partida"; iconName: "play"; compact: true; onClicked: view.app.openPlay() }
                }
            }
            Rectangle {
                Layout.preferredWidth: view.width * 0.4; Layout.fillHeight: true; Layout.minimumHeight: 400; radius: 18; color: "#10242D"; border.color: "#233E48"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 22; spacing: 14
                    RowLayout { UiIcon { name: "trophy"; ink: "#E4BE69"; implicitWidth: 21; implicitHeight: 21 } Label { text: view.backend.rankingMode === "competitive" ? "Ranking competitivo" : "Recordes no modo Livre"; color: "#E5EEE6"; font.pixelSize: 17; font.weight: Font.DemiBold } }
                    RowLayout {
                        Layout.fillWidth: true
                        UiButton { objectName: "freeRankingButton"; text: "Livre"; compact: true; secondary: view.backend.rankingMode !== "free"; onClicked: view.backend.rankingMode = "free" }
                        UiButton { objectName: "competitiveRankingButton"; text: "Competitivo"; compact: true; secondary: view.backend.rankingMode !== "competitive"; onClicked: view.backend.rankingMode = "competitive" }
                    }
                    Label { text: "Melhor pontuação de cada jogador"; color: "#72959E"; font.pixelSize: 11 }
                    Label { text: view.backend.rankingMode === "competitive" ? view.backend.competitiveQuestionCount + " questões · gênero · equilibrado · conteúdo atual" : "Treinamento contínuo; grupos e durações variáveis."; color: "#72959E"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { visible: !!view.backend.personalRecord.scoreText; text: "Seu recorde: " + (view.backend.personalRecord.scoreText || "0,00") + " · Última: " + (view.backend.personalRecord.lastScoreText || "0,00"); color: "#71DCA0"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { visible: view.backend.ranking.length === 0; text: "Encerre uma partida para entrar no ranking."; color: "#8FAAB0"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    ListView {
                        id: rankingList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8
                        clip: true
                        ScrollBar.vertical: ScrollBar { }
                        model: view.backend.ranking
                        delegate: Rectangle {
                            id: rankRow
                            required property var modelData
                            required property int index
                            width: rankingList.width; implicitHeight: 58; radius: 11; color: index === 0 ? "#1D3B31" : "#152D36"; border.color: index === 0 ? "#3E6348" : "#203B44"
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 10
                                Label { text: String(rankRow.index + 1).padStart(2, "0"); color: rankRow.index === 0 ? "#E8C878" : "#648B94"; font.pixelSize: 15; font.weight: Font.DemiBold }
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 3
                                    Label { text: rankRow.modelData.playerName; color: "#D5E6DB"; elide: Text.ElideRight; font.pixelSize: 12; Layout.fillWidth: true }
                                    Label { text: "Última: " + rankRow.modelData.lastScoreText + " · " + rankRow.modelData.lastPlayedAt; color: "#688B92"; font.pixelSize: 9 }
                                }
                                Label { text: rankRow.modelData.scoreText + " pts"; color: "#71DCA0"; font.pixelSize: 13; font.weight: Font.DemiBold }
                            }
                        }
                    }
                }
            }
        }
    }
}
