import QtQuick
import QtQuick.Controls

Rectangle {
    id: artwork
    property color accent: "#E7AE61"
    property bool moth: false
    property bool showCaption: true
    property real butterflyScale: 1
    radius: 20
    color: "#142E2B"
    border.color: "#345546"
    clip: true
    Canvas {
        id: illustration
        anchors.fill: parent
        anchors.margins: 1
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Connections {
            target: artwork
            function onAccentChanged() { illustration.requestPaint() }
            function onMothChanged() { illustration.requestPaint() }
            function onButterflyScaleChanged() { illustration.requestPaint() }
        }
        onPaint: {
            const c = getContext("2d")
            function ellipse(x, y, rx, ry) {
                c.save(); c.translate(x, y); c.scale(rx, ry)
                c.beginPath(); c.arc(0, 0, 1, 0, Math.PI * 2)
                c.restore(); c.fill()
            }
            c.clearRect(0, 0, width, height)
            c.save()
            const r = 19
            c.beginPath(); c.moveTo(r, 0); c.lineTo(width - r, 0)
            c.quadraticCurveTo(width, 0, width, r); c.lineTo(width, height - r)
            c.quadraticCurveTo(width, height, width - r, height); c.lineTo(r, height)
            c.quadraticCurveTo(0, height, 0, height - r); c.lineTo(0, r)
            c.quadraticCurveTo(0, 0, r, 0); c.closePath(); c.clip()
            const bg = c.createRadialGradient(width * 0.65, height * 0.38, 0, width * 0.65, height * 0.38, width * 0.9)
            bg.addColorStop(0, "#456643"); bg.addColorStop(0.5, "#1E4035"); bg.addColorStop(1, "#11272A")
            c.fillStyle = bg; c.fillRect(0, 0, width, height)
            for (let i = 0; i < 14; ++i) {
                c.beginPath(); c.arc(width * ((i * 0.173) % 1), height * ((i * 0.237) % 1), 12 + (i % 4) * 15, 0, Math.PI * 2)
                c.fillStyle = i % 2 ? "rgba(141,170,79,0.035)" : "rgba(191,207,139,0.05)"; c.fill()
            }
            for (let k = 0; k < 6; ++k) {
                const x = width * (0.08 + k * 0.18), y = height * (0.78 + (k % 2) * 0.12)
                c.save(); c.translate(x, y); c.rotate(-0.6 + k * 0.3)
                c.strokeStyle = "#547857"; c.lineWidth = 1
                c.beginPath(); c.moveTo(0, 100); c.quadraticCurveTo(-15, 20, 5, -60); c.stroke()
                for (let l = 0; l < 4; ++l) {
                    c.fillStyle = l % 2 ? "#284F3D" : "#365E43"
                    c.beginPath(); c.moveTo(0, 30 - l * 22)
                    c.quadraticCurveTo(-65, -8 - l * 22, -38, -30 - l * 22)
                    c.quadraticCurveTo(-5, -20 - l * 22, 0, 30 - l * 22); c.fill()
                }
                c.restore()
            }
            const scale = Math.min(width / 360, height / 380) * artwork.butterflyScale
            c.translate(width * 0.53, height * 0.45)
            c.rotate(artwork.moth ? -0.13 : -0.22)
            c.scale(scale, scale)
            for (let side = -1; side <= 1; side += 2) {
                c.save(); c.scale(side, 1)
                const wing = c.createLinearGradient(5, -100, 130, 100)
                wing.addColorStop(0, artwork.accent); wing.addColorStop(1, artwork.moth ? "#646853" : "#B8682F")
                c.fillStyle = wing; c.strokeStyle = "#0D2021"; c.lineWidth = 9
                c.beginPath(); c.moveTo(5, 0)
                c.bezierCurveTo(35, -64, 92, -140, 145, -110)
                c.bezierCurveTo(172, -89, 138, -7, 75, 30)
                c.bezierCurveTo(141, 43, 113, 136, 62, 119)
                c.bezierCurveTo(23, 111, 18, 52, 5, 0)
                c.fill(); c.stroke()
                c.lineWidth = 3
                for (let v = 0; v < 5; ++v) {
                    c.beginPath(); c.moveTo(9, 1); c.quadraticCurveTo(62, -17 - v * 12, 138 - v * 13, -95 + v * 23); c.stroke()
                }
                for (let v = 0; v < 3; ++v) {
                    c.beginPath(); c.moveTo(11, 15); c.quadraticCurveTo(37, 74, 46 + v * 24, 108 - v * 13); c.stroke()
                }
                c.fillStyle = "#EADFC0"
                for (let d = 0; d < 7; ++d) {
                    ellipse(134 - d * 8, -91 + d * 17, 3.5, 5)
                }
                c.fillStyle = "#183536"; ellipse(70, 70, 19, 17)
                c.fillStyle = "#D7C486"; ellipse(70, 70, 9, 8)
                c.fillStyle = "#183536"; ellipse(70, 70, 4, 4)
                c.restore()
            }
            c.fillStyle = "#0D2021"
            ellipse(0, 17, 7, 52)
            c.beginPath(); c.arc(0, -32, 7, 0, Math.PI * 2); c.fill()
            c.lineWidth = 2; c.strokeStyle = "#0D2021"
            c.beginPath(); c.moveTo(-2, -32); c.quadraticCurveTo(-7, -62, -22, -65); c.moveTo(2, -32); c.quadraticCurveTo(7, -62, 22, -65); c.stroke()
            c.restore()
        }
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 14
        width: caption.implicitWidth + 24
        implicitHeight: 26
        radius: 13
        color: "#AA112A28"
        visible: artwork.showCaption
        Label { id: caption; anchors.centerIn: parent; text: "Ilustração decorativa"; font.pixelSize: 10; color: "#B5C8B9" }
    }
}
