import QtQuick

// Canvas.Image supports Qt's software renderer as well as GPU backends.
// The local image is painted only on load/resize, never once per frame.
Canvas {
    id: photo
    property url source
    property real cornerRadius: 17
    property real focusX: 0.5
    property real focusY: 0.5
    readonly property bool ready: bitmap.status === Image.Ready
    renderTarget: Canvas.Image
    antialiasing: true
    Image {
        id: bitmap
        source: photo.source
        visible: false
        asynchronous: true
        onStatusChanged: photo.requestPaint()
    }
    onReadyChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onCornerRadiusChanged: requestPaint()
    onFocusXChanged: requestPaint()
    onFocusYChanged: requestPaint()
    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        if (!ready || width <= 0 || height <= 0) return
        const size = bitmap.sourceSize
        if (!size.width || !size.height) return
        // Equivalent to PreserveAspectCrop, with an editable focal point.
        const scale = Math.max(width / size.width, height / size.height)
        const w = size.width * scale, h = size.height * scale
        const x = Math.max(width - w, Math.min(0, width / 2 - focusX * w))
        const y = Math.max(height - h, Math.min(0, height / 2 - focusY * h))
        const r = Math.min(cornerRadius, width / 2, height / 2)
        ctx.save()
        ctx.beginPath()
        ctx.moveTo(0, 0)
        ctx.lineTo(width - r, 0)
        ctx.quadraticCurveTo(width, 0, width, r)
        ctx.lineTo(width, height - r)
        ctx.quadraticCurveTo(width, height, width - r, height)
        ctx.lineTo(0, height)
        ctx.closePath()
        ctx.clip()
        ctx.drawImage(bitmap, x, y, w, h)
        ctx.globalCompositeOperation = "destination-in"
        const horizontal = ctx.createLinearGradient(0, 0, width, 0)
        horizontal.addColorStop(0, "transparent")
        horizontal.addColorStop(0.28, "#70FFFFFF")
        horizontal.addColorStop(0.65, "white")
        horizontal.addColorStop(1, "white")
        ctx.fillStyle = horizontal
        ctx.fillRect(0, 0, width, height)
        ctx.globalCompositeOperation = "source-over"
        ctx.restore()
    }
}
