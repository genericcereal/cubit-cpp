.pragma library

function toCanvas(point, flickable, zoom, minX, minY) {
    return {
        x: (point.x + (flickable?.contentX || 0)) / zoom + minX,
        y: (point.y + (flickable?.contentY || 0)) / zoom + minY
    }
}

function inMode(controller, mode) {
    return controller?.mode === mode
}

function distance(point1, point2) {
    const dx = point1.x - point2.x
    const dy = point1.y - point2.y
    return Math.sqrt(dx * dx + dy * dy)
}

function pointInRect(point, rect) {
    return point.x >= rect.x && 
           point.x <= rect.x + rect.width &&
           point.y >= rect.y && 
           point.y <= rect.y + rect.height
}

function expandRect(rect, margin) {
    return {
        x: rect.x - margin,
        y: rect.y - margin,
        width: rect.width + 2 * margin,
        height: rect.height + 2 * margin
    }
}

function clampToRect(point, rect) {
    return {
        x: Math.max(rect.x, Math.min(rect.x + rect.width, point.x)),
        y: Math.max(rect.y, Math.min(rect.y + rect.height, point.y))
    }
}