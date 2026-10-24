pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import "../style" as Style
import "AchievementBadgePaths.js" as BadgePaths

Item {
    id: layer
    required property var app
    required property var backend
    required property real headerBottom
    property bool surfaceAvailable: true
    readonly property bool presentationAllowed: app.active && app.visible && app.visibility !== Window.Minimized
        && surfaceAvailable && backend.localUserReady && !backend.localUserChoiceRequired
        && !backend.localUserError && !backend.progressError && !backend.achievementsError && !backend.resultPending
    readonly property string ownerId: backend.currentPlayerId
    readonly property int activeCount: active.count
    readonly property int pendingCount: pending.length
    property var pending: []
    property var seen: ({})
    property var activityFocus: null
    property bool initialized: false
    property int discardedCount: 0
    property string lastDiscardReason: ""
    readonly property int suspensionExpiryMs: 60000
    signal notificationPresented(string achievementId)
    signal notificationCompleted(string achievementId, string reason)
    signal notificationDiscarded(string achievementId, string reason)
    objectName: "achievementNotificationLayer"
    onNotificationDiscarded: (achievementId, reason) => { discardedCount += 1; lastDiscardReason = reason }

    function containsControl(control) {
        while (control && control !== layer) control = control.parent
        return control === layer
    }
    function restoreActivityFocus() {
        if (presentationAllowed && containsControl(app.activeFocusItem) && activityFocus && activityFocus.visible && activityFocus.enabled)
            activityFocus.forceActiveFocus(Qt.TabFocusReason)
    }
    function valid(event) {
        if (!event || event.unlocked !== true || event.retrospective !== false || !backend.localUserReady) return false
        const fields = ["id", "title", "description", "badge", "visualLevel", "visualLevelName", "playerId", "unlockedAt"]
        for (const field of fields) if (typeof event[field] !== "string" || !event[field].trim()) return false
        return event.playerId === ownerId && Number.isFinite(Date.parse(event.unlockedAt))
            && Object.prototype.hasOwnProperty.call(BadgePaths.levels, event.visualLevel)
            && Object.prototype.hasOwnProperty.call(BadgePaths.drawings, event.badge)
    }
    function enqueue(event) {
        if (!valid(event)) { notificationDiscarded(event && typeof event.id === "string" ? event.id : "", "invalid-event"); return }
        const key = event.playerId + ":" + event.id
        if (seen[key]) { notificationDiscarded(event.id, "duplicate"); return }
        const nextSeen = Object.assign({}, seen); nextSeen[key] = true; seen = nextSeen
        pending = pending.concat([{token: key, achievement: JSON.parse(JSON.stringify(event)), receivedAt: Date.now()}])
        pump()
    }
    function pump() {
        if (!presentationAllowed) return
        while (active.count < 2 && pending.length) {
            const entry = pending[0]; pending = pending.slice(1)
            active.append(entry)
            notificationPresented(entry.achievement.id)
        }
    }
    function complete(token, reason) {
        for (let i = 0; i < active.count; ++i) {
            if (active.get(i).token !== token) continue
            const id = active.get(i).achievement.id
            restoreActivityFocus(); active.remove(i)
            notificationCompleted(id, reason); pump(); return
        }
    }
    function clear(reason) {
        restoreActivityFocus()
        for (let i = 0; i < active.count; ++i) notificationDiscarded(active.get(i).achievement.id, reason)
        for (const entry of pending) notificationDiscarded(entry.achievement.id, reason)
        active.clear(); pending = []
    }
    function expireSuspended(now) {
        if (now === undefined) now = Date.now()
        for (let i = active.count - 1; i >= 0; --i) {
            if (now - active.get(i).receivedAt < suspensionExpiryMs) continue
            notificationDiscarded(active.get(i).achievement.id, "suspension-expired")
            restoreActivityFocus(); active.remove(i)
        }
        const retained = []
        for (const entry of pending) {
            if (now - entry.receivedAt >= suspensionExpiryMs) notificationDiscarded(entry.achievement.id, "suspension-expired")
            else retained.push(entry)
        }
        pending = retained
    }
    onPresentationAllowedChanged: {
        if (!initialized) return
        // Also prune on resume: Qt timers may not run while the OS suspends the app.
        expireSuspended()
        if (presentationAllowed) pump()
    }
    onOwnerIdChanged: if (initialized) clear("identity-changed")
    onBackendChanged: if (initialized) { clear("backend-changed"); seen = ({}) }
    Component.onCompleted: { initialized = true; pump() }
    Connections {
        target: layer.backend
        function onAchievementUnlocked(achievement) { layer.enqueue(achievement) }
    }
    Connections {
        target: layer.app
        function onActiveFocusItemChanged() {
            const control = layer.app.activeFocusItem
            if (control && !layer.containsControl(control)) layer.activityFocus = control
        }
    }
    Timer {
        interval: 1000; repeat: true
        running: !layer.presentationAllowed && (layer.activeCount > 0 || layer.pendingCount > 0)
        onTriggered: layer.expireSuspended()
    }
    ListModel { id: active; dynamicRoles: true }
    Column {
        id: notices
        anchors {
            right: parent.right; top: parent.top
            rightMargin: Style.Theme.spaceLg
            topMargin: layer.headerBottom + Style.Theme.spaceLg
        }
        width: Math.min(440, Math.max(0, layer.width - 2 * Style.Theme.spaceLg))
        spacing: Style.Theme.spaceSm
        Repeater {
            model: active
            delegate: AchievementNotification {
                required property string token
                width: notices.width
                presentationAllowed: layer.presentationAllowed
                onFinished: reason => layer.complete(token, reason)
            }
        }
    }
}
