import QtQuick

// Canvas.Image supports Qt's software renderer as well as GPU backends.
// The local image is painted only on load/resize, never once per frame.
Canvas {
    id: photo
    property url source
    readonly property bool ready: bitmap.status === Image.Ready
    renderTarget: Canvas.Image
    antialiasing: true
    Image {
        id: bitmap
        source: photo.source
        visible: false
        onStatusChanged: photo.requestPaint()
    }
    onReadyChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        if (!ready || width <= 0 || height <= 0) return
        const size = bitmap.sourceSize
        if (!size.width || !size.height) return
        const scale = Math.min(width / size.width, height / size.height)
        const w = size.width * scale, h = size.height * scale
        const x = (width - w) / 2, y = (height - h) / 2
        ctx.drawImage(bitmap, x, y, w, h)
        ctx.globalCompositeOperation = "destination-in"
        const horizontal = ctx.createLinearGradient(x, y, x + w, y)
        horizontal.addColorStop(0, "transparent")
        horizontal.addColorStop(0.16, "white")
        horizontal.addColorStop(0.84, "white")
        horizontal.addColorStop(1, "transparent")
        ctx.fillStyle = horizontal
        ctx.fillRect(0, 0, width, height)
        const vertical = ctx.createLinearGradient(x, y, x, y + h)
        vertical.addColorStop(0, "transparent")
        vertical.addColorStop(0.16, "white")
        vertical.addColorStop(0.84, "white")
        vertical.addColorStop(1, "transparent")
        ctx.fillStyle = vertical
        ctx.fillRect(0, 0, width, height)
        ctx.globalCompositeOperation = "source-over"
    }
}
