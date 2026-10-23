// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import "../style" as Style

Item {
    id: view
    required property var app
    required property var backend
    readonly property var all: backend.achievements
    readonly property int unlockedCount: all.filter(a => a.unlocked).length
    readonly property int percent: all.length ? Math.round(100 * unlockedCount / all.length) : 0
    readonly property var categories: ["Todas as categorias"].concat(all.map(a => a.category).filter((c, i, a) => a.indexOf(c) === i))
    property string stateFilter: app.achievementsPresentation.state || "Todas"
    property string categoryFilter: app.achievementsPresentation.category || "Todas as categorias"
    readonly property var filtered: all.filter(a => (stateFilter === "Todas" || a.unlocked === (stateFilter === "Desbloqueadas")) && (categoryFilter === "Todas as categorias" || a.category === categoryFilter))
    property string selectedId: ""
    readonly property var selected: all.find(a => a.id === selectedId) || ({})
    property var previousFocus: null
    implicitHeight: content.implicitHeight
    function remember() { app.achievementsPresentation = {state: stateFilter, category: categoryFilter} }
    function openDetails(achievement, control) { selectedId = achievement.id; previousFocus = control; details.open() }
    function dateText(a) {
        return a.unlockedAt ? (a.retrospective ? "Reconhecida em " : "Desbloqueada em ") + new Date(a.unlockedAt).toLocaleDateString(Qt.locale("pt_BR"), "dd/MM/yyyy") : "Ainda bloqueada"
    }
    ColumnLayout {
        id: content
        width: parent.width
        spacing: Style.Theme.spaceLg
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: overview.implicitHeight + 40
            radius: Style.Theme.cardRadius
            color: Style.Theme.surface
            border.color: Style.Theme.border
            ColumnLayout {
                id: overview
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: 20 }
                spacing: 12
                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: "SEU PERCURSO DE DESCOBERTAS"; color: Style.Theme.gold; font.pixelSize: 11; font.letterSpacing: 1.3; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        Label { text: "Conquistas"; color: Style.Theme.text; font.pixelSize: Style.Theme.titleSize; font.weight: Font.DemiBold; Accessible.role: Accessible.Heading }
                    }
                    Label { objectName: "achievementSummary"; text: view.unlockedCount + " / " + view.all.length; color: Style.Theme.green; font.pixelSize: 30; font.weight: Font.DemiBold; Accessible.name: view.unlockedCount + " conquistas desbloqueadas de " + view.all.length }
                }
                Label { Layout.fillWidth: true; text: "Cada descoberta faz parte da sua história. Explore, aprenda e acompanhe seu progresso."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.bodySize; wrapMode: Text.WordWrap }
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "Progresso geral"; color: Style.Theme.textMuted; Layout.fillWidth: true }
                    Label { text: view.percent + "% concluído"; color: Style.Theme.green }
                }
                Rectangle {
                    Layout.fillWidth: true; implicitHeight: 7; radius: 4; color: Style.Theme.surfaceRaised
                    Rectangle { width: parent.width * view.percent / 100; height: parent.height; radius: 4; color: Style.Theme.green; Behavior on width { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationPage } } }
                }
            }
        }
        Label { visible: view.backend.achievementsError !== ""; text: view.backend.achievementsError; color: Style.Theme.gold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Flow {
            Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: 8
            Repeater {
                model: ["Todas", "Desbloqueadas", "Bloqueadas"]
                delegate: UiButton {
                    required property string modelData
                    objectName: "achievementState-" + modelData
                    text: modelData; compact: true; checkable: true; checked: view.stateFilter === modelData; secondary: !checked
                    onClicked: { view.stateFilter = modelData; view.remember() }
                }
            }
        }
        Flow {
            Layout.fillWidth: true; Layout.preferredHeight: implicitHeight; spacing: 8
            Repeater {
                model: view.categories
                delegate: UiButton {
                    required property string modelData
                    objectName: "achievementCategory-" + modelData
                    text: modelData; compact: true; checkable: true; checked: view.categoryFilter === modelData; secondary: !checked
                    onClicked: { view.categoryFilter = modelData; view.remember() }
                }
            }
        }
        Label { objectName: "achievementFilterCount"; text: view.filtered.length + (view.filtered.length === 1 ? " conquista nesta seleção" : " conquistas nesta seleção"); color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
        GridLayout {
            id: grid
            Layout.fillWidth: true
            columns: view.width >= 1300 ? 4 : view.width >= 930 ? 3 : 2
            columnSpacing: 16; rowSpacing: 16
            Repeater {
                model: view.filtered
                delegate: AbstractButton {
                    id: card
                    required property var modelData
                    objectName: "achievementCard-" + modelData.id
                    Layout.fillWidth: true
                    Layout.preferredWidth: (view.width - (grid.columns - 1) * grid.columnSpacing) / grid.columns
                    implicitHeight: cardContent.implicitHeight + 36
                    Layout.fillHeight: true
                    padding: 18
                    hoverEnabled: true; focusPolicy: Qt.StrongFocus
                    Accessible.name: modelData.title + ". " + (modelData.unlocked ? "Desbloqueada. " : "Bloqueada. ") + modelData.description + " Progresso " + modelData.progress + " de " + modelData.target
                    onClicked: view.openDetails(modelData, card)
                    Keys.onReturnPressed: clicked()
                    background: Rectangle {
                        radius: Style.Theme.cardRadius
                        color: card.down ? Style.Theme.forest : card.hovered ? Style.Theme.surfaceHover : Style.Theme.surface
                        border.width: card.visualFocus ? 2 : 1
                        border.color: card.visualFocus ? Style.Theme.focus : card.modelData.unlocked ? "#588570" : Style.Theme.border
                        Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
                    }
                    contentItem: ColumnLayout {
                        id: cardContent
                        spacing: 10
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: card.modelData.category; color: Style.Theme.textMuted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            UiIcon { name: card.modelData.unlocked ? "check" : "target"; ink: card.modelData.unlocked ? Style.Theme.green : Style.Theme.textDisabled; implicitWidth: 18; implicitHeight: 18 }
                        }
                        AchievementBadge {
                            objectName: "achievementBadge-" + card.modelData.id
                            glyph: card.modelData.badge
                            Layout.preferredWidth: 96; Layout.preferredHeight: 106; Layout.alignment: Qt.AlignHCenter
                            opacity: card.modelData.unlocked ? 1 : 0.4
                            scale: card.down && Style.Theme.motionEnabled ? 0.96 : 1
                            Behavior on opacity { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationPage } }
                            Behavior on scale { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationFast } }
                        }
                        Label { Layout.fillWidth: true; text: card.modelData.title; color: Style.Theme.text; font.pixelSize: 17; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
                        Label { Layout.fillWidth: true; text: card.modelData.description; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
                        Item { Layout.fillHeight: true; implicitHeight: 4 }
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: card.modelData.unlocked ? "Desbloqueada" : "Bloqueada"; color: card.modelData.unlocked ? Style.Theme.green : Style.Theme.textDisabled; font.pixelSize: Style.Theme.captionSize; Layout.fillWidth: true }
                            Label { text: card.modelData.progress + " / " + card.modelData.target; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize }
                        }
                        Rectangle {
                            Layout.fillWidth: true; implicitHeight: 4; radius: 2; color: Style.Theme.surfaceRaised
                            Rectangle { height: parent.height; width: parent.width * Math.min(1, card.modelData.progress / card.modelData.target); radius: 2; color: card.modelData.unlocked ? Style.Theme.green : Style.Theme.textDisabled; Behavior on width { enabled: Style.Theme.motionEnabled; NumberAnimation { duration: Style.Theme.durationPage } } }
                        }
                        Label { text: card.modelData.unlocked ? view.dateText(card.modelData) : "Selecionar para ver detalhes"; color: Style.Theme.textMuted; font.pixelSize: 11; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                    }
                }
            }
        }
        Label { visible: view.filtered.length === 0; text: "Nenhuma conquista nesta seleção. Experimente outra categoria ou estado."; color: Style.Theme.textMuted; Layout.fillWidth: true; wrapMode: Text.WordWrap; Layout.topMargin: 24; Layout.bottomMargin: 24 }
        Label { text: "Seu progresso é salvo neste computador e permanece entre partidas."; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; Layout.fillWidth: true; wrapMode: Text.WordWrap }
    }
    Dialog {
        id: details
        objectName: "achievementDetails"
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(560, view.app.width - 48)
        height: Math.min(detailContent.implicitHeight + topPadding + bottomPadding
                         + implicitHeaderHeight + implicitFooterHeight + 2 * spacing, view.app.height - 64)
        modal: true; focus: true
        title: view.selected.title || "Conquista"
        background: Rectangle { color: Style.Theme.surface; radius: Style.Theme.cardRadius; border.color: Style.Theme.border }
        onOpened: closeDetails.forceActiveFocus(Qt.TabFocusReason)
        onClosed: if (view.previousFocus) view.previousFocus.forceActiveFocus(Qt.TabFocusReason)
        contentItem: ScrollView {
            objectName: "achievementDetailScroll"
            clip: true; contentWidth: availableWidth; contentHeight: detailContent.implicitHeight
            ColumnLayout {
                id: detailContent
                width: parent.width; spacing: 16
                AchievementBadge { glyph: view.selected.badge || "leaf"; Layout.preferredWidth: 144; Layout.preferredHeight: 158; Layout.alignment: Qt.AlignHCenter; opacity: view.selected.unlocked ? 1 : 0.4 }
                Label { text: view.selected.category || ""; color: Style.Theme.gold; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: view.selected.description || ""; color: Style.Theme.text; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 16 }
                Label { text: (view.selected.unlocked ? "Desbloqueada" : "Bloqueada") + " · " + (view.selected.progress || 0) + " / " + (view.selected.target || 1); color: Style.Theme.green; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                Label { objectName: "achievementDetailDate"; text: view.dateText(view.selected); color: Style.Theme.textMuted; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                Label { visible: !!view.selected.retrospective; text: "Reconhecida a partir de registros anteriores verificáveis. A data indica o reconhecimento, não a data original do feito."; color: Style.Theme.textMuted; Layout.fillWidth: true; wrapMode: Text.WordWrap }
            }
        }
        footer: DialogButtonBox {
            background: Item {}
            UiButton { id: closeDetails; objectName: "closeAchievementDetails"; text: "Fechar detalhes"; compact: true; DialogButtonBox.buttonRole: DialogButtonBox.RejectRole; onClicked: details.close() }
        }
    }
}
