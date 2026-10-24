pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../style" as Style
import "AchievementBadgePaths.js" as BadgePaths

Item {
    id: notice
    required property var achievement
    property bool presentationAllowed: true
    readonly property color metal: BadgePaths.levels[achievement.visualLevel].rim
    property real reveal: 0
    property bool entered: false
    property bool leaving: false
    property string closeReason: ""
    property real remainingMs: 5000
    property double readingStartedAt: 0
    signal finished(string reason)
    objectName: "achievementNotification-" + achievement.id
    implicitHeight: Math.max(94, contents.implicitHeight + 24)
    visible: presentationAllowed
    opacity: reveal
    transform: Translate { x: Style.Theme.motionEnabled ? 26 * (1 - notice.reveal) : 0 }
    Accessible.role: Accessible.AlertMessage
    Accessible.name: "Conquista desbloqueada! " + achievement.title + ". " + achievement.visualLevelName
    Accessible.description: achievement.description

    function startReading() {
        if (!presentationAllowed || leaving || reading.running) return
        readingStartedAt = Date.now()
        reading.start()
    }
    function pauseReading() {
        if (!reading.running) return
        remainingMs = Math.max(1, remainingMs - (Date.now() - readingStartedAt))
        reading.stop()
    }
    function enter() {
        if (!presentationAllowed || leaving) return
        if (entered) { startReading(); return }
        if (Style.Theme.motionEnabled) entrance.start()
        else { reveal = 1; entered = true; startReading() }
    }
    function dismiss(reason) {
        if (leaving) return
        leaving = true; closeReason = reason
        reading.stop(); entrance.stop()
        if (Style.Theme.motionEnabled && presentationAllowed) departure.start()
        else { reveal = 0; finished(closeReason) }
    }
    onPresentationAllowedChanged: {
        if (presentationAllowed) enter()
        else {
            pauseReading()
            if (entrance.running) { entrance.stop(); reveal = 1; entered = true }
        }
    }
    Component.onCompleted: enter()
    Connections {
        target: Style.Theme
        function onMotionEnabledChanged() {
            if (Style.Theme.motionEnabled) return
            if (departure.running) { departure.stop(); notice.reveal = 0; notice.finished(notice.closeReason) }
            else if (entrance.running) { entrance.stop(); notice.reveal = 1; notice.entered = true; notice.startReading() }
        }
    }
    NumberAnimation {
        id: entrance; target: notice; property: "reveal"; from: 0; to: 1
        duration: 280; easing.type: Easing.OutCubic
        onFinished: { notice.entered = true; notice.startReading() }
    }
    NumberAnimation {
        id: departure; target: notice; property: "reveal"; to: 0
        duration: 200; easing.type: Easing.InCubic
        onFinished: notice.finished(notice.closeReason)
    }
    Timer { id: reading; interval: Math.max(1, notice.remainingMs); onTriggered: notice.dismiss("timeout") }
    Rectangle {
        x: 0; y: 3; width: parent.width; height: parent.height
        radius: Style.Theme.cardRadius; color: "#40000000"
    }
    Rectangle {
        anchors.fill: parent
        radius: Style.Theme.cardRadius; color: Style.Theme.surface
        border.color: notice.metal
        border.width: notice.achievement.visualLevel === "special" ? 2 : 1
        Rectangle {
            x: 16; y: 1; width: parent.width - 32
            height: notice.achievement.visualLevel === "special" ? 3 : 2
            radius: 1; color: notice.metal
            opacity: notice.achievement.visualLevel === "initial" ? 0.45 : 0.8
        }
        Rectangle {
            anchors.fill: parent; anchors.margins: 4
            radius: Style.Theme.cardRadius - 4; color: "#d7edfa"
            opacity: Style.Theme.motionEnabled && notice.achievement.visualLevel === "special" && !notice.entered
                     ? Math.sin(Math.PI * notice.reveal) * 0.12 : 0
        }
    }
    RowLayout {
        id: contents
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
        spacing: 12
        AchievementBadge {
            glyph: notice.achievement.badge; visualLevel: notice.achievement.visualLevel; unlocked: true
            Layout.preferredWidth: 64; Layout.preferredHeight: 70
            Accessible.ignored: true
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 4
            Label { text: "Conquista desbloqueada!"; color: Style.Theme.green; font.pixelSize: Style.Theme.captionSize; Layout.fillWidth: true; wrapMode: Text.WordWrap; Accessible.ignored: true }
            Label { objectName: "notificationTitle-" + notice.achievement.id; text: notice.achievement.title; color: Style.Theme.text; font.pixelSize: Style.Theme.bodySize; font.weight: Font.DemiBold; Layout.fillWidth: true; wrapMode: Text.WordWrap; Accessible.ignored: true }
            Label { text: notice.achievement.visualLevelName; color: Style.Theme.textMuted; font.pixelSize: Style.Theme.captionSize; Layout.fillWidth: true; wrapMode: Text.WordWrap; Accessible.ignored: true }
        }
        ToolButton {
            id: closeNotice
            objectName: "dismissAchievement-" + notice.achievement.id
            text: "×"; font.pixelSize: 22
            Layout.preferredWidth: 36; Layout.preferredHeight: 36; Layout.alignment: Qt.AlignTop
            focusPolicy: Qt.TabFocus
            Accessible.name: "Dispensar conquista: " + notice.achievement.title
            onClicked: notice.dismiss("manual")
            contentItem: Text { text: closeNotice.text; font: closeNotice.font; color: Style.Theme.textMuted; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle {
                radius: Style.Theme.controlRadius
                color: closeNotice.down ? Style.Theme.surfaceRaised : closeNotice.hovered ? Style.Theme.surfaceHover : "transparent"
                border.color: closeNotice.visualFocus ? Style.Theme.focus : "transparent"
                border.width: Style.Theme.focusWidth
                Behavior on color { enabled: Style.Theme.motionEnabled; ColorAnimation { duration: Style.Theme.durationFast } }
            }
        }
    }
}
