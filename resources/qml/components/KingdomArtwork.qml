pragma ComponentBehavior: Bound
import QtQuick
Item {
    id: art
    property string kingdom: "Animalia"
    property color accent: kingdom === "Plantae" || kingdom === "Archaeplastida" ? "#6BAA7A" : kingdom === "Fungi" ? "#BB8068" : kingdom === "Eukaryota" || kingdom === "SAR" ? "#729BC0" : kingdom === "Bacteria" || kingdom === "Archaea" ? "#66A8AC" : "#C5A366"
    onKingdomChanged: drawing.requestPaint()
    onAccentChanged: drawing.requestPaint()
    Canvas {
        id: drawing
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const c = getContext("2d")
            c.reset(); c.translate(width / 2, height / 2)
            const scale = Math.min(width, height) / 230
            c.scale(scale, scale); c.strokeStyle = art.accent; c.fillStyle = art.accent; c.lineWidth = 3
            c.globalAlpha = 0.08; c.beginPath(); c.arc(0, 0, 105, 0, 7); c.fill(); c.globalAlpha = 0.9
            if (art.kingdom === "Animalia") {
                for (const side of [-1, 1]) {
                    c.save(); c.scale(side, 1)
                    c.beginPath(); c.moveTo(0, 10); c.bezierCurveTo(30, -95, 105, -100, 85, -25); c.bezierCurveTo(110, 55, 35, 95, 0, 10); c.fill()
                    c.strokeStyle = "#1C3633"; c.lineWidth = 2
                    for (let i = 0; i < 4; i++) { c.beginPath(); c.moveTo(6, 4); c.lineTo(68, -60 + i * 32); c.stroke() }
                    c.restore()
                }
                c.strokeStyle = "#E4D5AE"; c.lineWidth = 5; c.beginPath(); c.moveTo(0, -42); c.lineTo(0, 55); c.stroke()
                c.lineWidth = 2; c.beginPath(); c.moveTo(0, -40); c.lineTo(-17, -64); c.moveTo(0, -40); c.lineTo(17, -64); c.stroke()
            } else if (art.kingdom === "Plantae") {
                c.beginPath(); c.moveTo(-20, 85); c.quadraticCurveTo(15, 0, 24, -80); c.stroke()
                for (let i = 0; i < 4; i++) {
                    const y = 55 - i * 35, side = i % 2 ? -1 : 1
                    c.beginPath(); c.moveTo(0, y); c.quadraticCurveTo(side * 80, y - 65, side * 65, y - 5); c.quadraticCurveTo(side * 30, y + 24, 0, y); c.fill()
                }
            } else if (art.kingdom === "Fungi") {
                c.beginPath(); c.moveTo(-16, 0); c.lineTo(-24, 82); c.quadraticCurveTo(0, 92, 24, 82); c.lineTo(16, 0); c.closePath(); c.fill()
                c.fillStyle = "#C89B76"; c.beginPath(); c.moveTo(-85, 0); c.bezierCurveTo(-80, -100, 80, -100, 85, 0); c.quadraticCurveTo(0, 30, -85, 0); c.fill()
                c.strokeStyle = "#EDD8B2"; c.globalAlpha = 0.4
                for (let x = -60; x <= 60; x += 20) { c.beginPath(); c.moveTo(0, 16); c.lineTo(x, -2); c.stroke() }
            } else if (["Eukaryota", "SAR", "Amoebozoa", "Discoba"].indexOf(art.kingdom) >= 0) {
                c.beginPath(); c.ellipse(-68, -90, 136, 180); c.fill()
                c.globalAlpha = 0.5; c.strokeStyle = "#CAE0E7"
                for (let i = 0; i < 24; i++) {
                    const a = i * Math.PI / 12
                    c.beginPath(); c.moveTo(Math.cos(a) * 73, Math.sin(a) * 94); c.lineTo(Math.cos(a) * 84, Math.sin(a) * 108); c.stroke()
                }
                c.fillStyle = "#243F53"; c.beginPath(); c.ellipse(-22, -25, 44, 64); c.fill()
            } else {
                c.rotate(-0.4); c.beginPath(); c.moveTo(-32, -58); c.bezierCurveTo(-32, -102, 32, -102, 32, -58); c.lineTo(32, 58); c.bezierCurveTo(32, 102, -32, 102, -32, 58); c.closePath(); c.fill()
                c.strokeStyle = "#214344"; c.lineWidth = 4; c.beginPath(); c.moveTo(-10, -45); c.bezierCurveTo(40, -10, -40, 20, 10, 45); c.stroke()
                c.strokeStyle = art.accent; c.lineWidth = 2; c.beginPath(); c.moveTo(0, 86); c.bezierCurveTo(65, 105, -65, 120, 0, 138); c.stroke()
            }
        }
    }
}
