import QtQuick

Canvas {
    id: icon
    property string name: "butterfly"
    property color ink: "#A9BEC4"
    implicitWidth: 22
    implicitHeight: 22
    onNameChanged: requestPaint()
    onInkChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        const c = getContext("2d")
        c.clearRect(0, 0, width, height)
        c.save()
        c.scale(width / 24, height / 24)
        c.strokeStyle = ink
        c.fillStyle = ink
        c.lineWidth = 1.7
        c.lineCap = "round"
        c.lineJoin = "round"
        function line(points) {
            c.beginPath(); c.moveTo(points[0], points[1])
            for (let i = 2; i < points.length; i += 2) c.lineTo(points[i], points[i + 1])
            c.stroke()
        }
        function circle(x, y, r) { c.beginPath(); c.arc(x, y, r, 0, Math.PI * 2); c.stroke() }
        switch (name) {
        case "home":
            line([3, 11, 12, 3, 21, 11]); line([5, 10, 5, 21, 10, 21, 10, 15, 14, 15, 14, 21, 19, 21, 19, 10]); break
        case "play":
            c.beginPath(); c.moveTo(7, 4); c.lineTo(20, 12); c.lineTo(7, 20); c.closePath(); c.fill(); break
        case "chart":
            line([4, 20, 4, 12, 8, 12, 8, 20]); line([10, 20, 10, 5, 14, 5, 14, 20]); line([16, 20, 16, 9, 20, 9, 20, 20]); break
        case "trophy":
            line([7, 3, 17, 3, 17, 10, 15, 14, 9, 14, 7, 10, 7, 3]); line([7, 5, 3, 5, 3, 9, 7, 11]); line([17, 5, 21, 5, 21, 9, 17, 11]); line([12, 14, 12, 20]); line([8, 21, 16, 21]); break
        case "settings":
            circle(12, 12, 7); circle(12, 12, 2.5)
            for (let j = 0; j < 8; ++j) { const a = j * Math.PI / 4; line([12 + 7 * Math.cos(a), 12 + 7 * Math.sin(a), 12 + 10 * Math.cos(a), 12 + 10 * Math.sin(a)]) }
            break
        case "arrow": line([4, 12, 20, 12]); line([14, 6, 20, 12, 14, 18]); break
        case "back": line([20, 12, 4, 12]); line([10, 6, 4, 12, 10, 18]); break
        case "check": line([5, 12, 10, 17, 20, 6]); break
        case "cross": line([6, 6, 18, 18]); line([18, 6, 6, 18]); break
        case "book": line([12, 6, 8, 4, 3, 4, 3, 19, 8, 19, 12, 21, 16, 19, 21, 19, 21, 4, 16, 4, 12, 6, 12, 21]); break
        case "leaf":
            c.beginPath(); c.moveTo(5, 19); c.bezierCurveTo(0, 10, 7, 3, 21, 3); c.bezierCurveTo(20, 16, 15, 23, 5, 19); c.stroke(); line([4, 21, 15, 10]); break
        case "target": circle(12, 12, 9); circle(12, 12, 5); circle(12, 12, 1); break
        case "pin": circle(12, 9, 2); c.beginPath(); c.moveTo(12, 22); c.bezierCurveTo(-2, 7, 5, 2, 12, 2); c.bezierCurveTo(19, 2, 26, 7, 12, 22); c.stroke(); break
        case "hint": circle(12, 9, 6); line([9, 14, 9, 18, 15, 18, 15, 14]); line([10, 21, 14, 21]); break
        case "user": circle(12, 7, 4); c.beginPath(); c.arc(12, 22, 8, Math.PI, Math.PI * 2); c.stroke(); break
        case "grid": c.strokeRect(4, 4, 6, 6); c.strokeRect(14, 4, 6, 6); c.strokeRect(4, 14, 6, 6); c.strokeRect(14, 14, 6, 6); break
        case "spark": line([12, 2, 14, 9, 21, 12, 14, 15, 12, 22, 10, 15, 3, 12, 10, 9, 12, 2]); break
        default:
            c.beginPath(); c.moveTo(11, 11); c.bezierCurveTo(1, -1, 0, 3, 3, 12); c.bezierCurveTo(0, 22, 8, 22, 11, 14); c.moveTo(13, 11); c.bezierCurveTo(23, -1, 24, 3, 21, 12); c.bezierCurveTo(24, 22, 16, 22, 13, 14); c.stroke(); line([12, 6, 12, 20]); line([12, 7, 9, 3]); line([12, 7, 15, 3]); break
        }
        c.restore()
    }
}
