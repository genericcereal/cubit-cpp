import QtQuick

Canvas {
    id: root
    
    property real borderWidth: 1
    property color borderColor: "black"
    property string borderStyle: "Solid" // "Solid", "Dashed", "Dotted"
    property real borderRadius: 0
    
    onBorderWidthChanged: requestPaint()
    onBorderColorChanged: requestPaint()
    onBorderStyleChanged: requestPaint()
    onBorderRadiusChanged: requestPaint()
    
    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        
        if (borderWidth <= 0) return
        
        ctx.strokeStyle = borderColor
        ctx.lineWidth = borderWidth
        
        // Set line style
        if (borderStyle === "Dashed") {
            ctx.setLineDash([borderWidth * 3, borderWidth * 2])
        } else if (borderStyle === "Dotted") {
            ctx.setLineDash([borderWidth, borderWidth])
        } else {
            ctx.setLineDash([])
        }
        
        // Calculate inset to center the border on the edge
        var inset = borderWidth / 2
        
        if (borderRadius > 0) {
            // Draw rounded rectangle
            var x = inset
            var y = inset
            var w = width - borderWidth
            var h = height - borderWidth
            var r = Math.min(borderRadius, Math.min(w, h) / 2)
            
            ctx.beginPath()
            ctx.moveTo(x + r, y)
            ctx.lineTo(x + w - r, y)
            ctx.quadraticCurveTo(x + w, y, x + w, y + r)
            ctx.lineTo(x + w, y + h - r)
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h)
            ctx.lineTo(x + r, y + h)
            ctx.quadraticCurveTo(x, y + h, x, y + h - r)
            ctx.lineTo(x, y + r)
            ctx.quadraticCurveTo(x, y, x + r, y)
            ctx.closePath()
            ctx.stroke()
        } else {
            // Draw regular rectangle
            ctx.strokeRect(inset, inset, width - borderWidth, height - borderWidth)
        }
    }
}