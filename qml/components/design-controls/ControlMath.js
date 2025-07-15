// ControlMath.js
.pragma library

function rotationSteps(angle) {
    return (Math.round(angle/90)%4 + 4) % 4;
}

function edgeCursorFor(angle, edgeIndex) {
    const steps = rotationSteps(angle);
    const effectiveIndex = (edgeIndex + steps) % 4;
    
    switch (effectiveIndex) {
        case 0: return Qt.SizeVerCursor;   // top edge
        case 1: return Qt.SizeHorCursor;   // right edge
        case 2: return Qt.SizeVerCursor;   // bottom edge
        case 3: return Qt.SizeHorCursor;   // left edge
        default: return Qt.ArrowCursor;
    }
}

function cornerCursorFor(angle, cornerIndex) {
    const steps = rotationSteps(angle);
    const effectiveIndex = (cornerIndex + steps) % 4;
    
    switch (effectiveIndex) {
        case 0: return Qt.SizeFDiagCursor;  // top-left corner
        case 1: return Qt.SizeBDiagCursor;  // top-right corner
        case 2: return Qt.SizeFDiagCursor;  // bottom-right corner
        case 3: return Qt.SizeBDiagCursor;  // bottom-left corner
        default: return Qt.ArrowCursor;
    }
}

function mapToCanvas(parent, point, zoom, flickable, canvasMin) {
    const globalPoint = parent.mapToItem(flickable, point.x, point.y);
    return Qt.point(
        (flickable.contentX + globalPoint.x) / zoom + canvasMin.x,
        (flickable.contentY + globalPoint.y) / zoom + canvasMin.y
    );
}