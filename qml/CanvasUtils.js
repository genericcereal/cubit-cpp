.pragma library

// Convert viewport coordinates to canvas coordinates
function viewportToCanvas(pt, contentX, contentY, zoom, minX, minY) {
    return Qt.point(
        (contentX + pt.x) / zoom + minX,
        (contentY + pt.y) / zoom + minY
    );
}

// Convert canvas coordinates to viewport coordinates
function canvasToViewport(pt, minX, minY, zoom) {
    return Qt.point(
        (pt.x - minX) * zoom,
        (pt.y - minY) * zoom
    );
}

// Clamp a value between min and max
function clamp(val, min, max) {
    return val < min ? min : (val > max ? max : val);
}

// Additional utility functions based on BaseCanvas needs

// Calculate content position to keep a point stable during zoom
function calculateStableZoomPosition(canvasPoint, viewportPoint, minX, minY, newZoom) {
    return {
        x: (canvasPoint.x - minX) * newZoom - viewportPoint.x,
        y: (canvasPoint.y - minY) * newZoom - viewportPoint.y
    };
}

// Calculate content position to center a canvas point in viewport
function calculateCenterPosition(canvasPoint, viewportWidth, viewportHeight, minX, minY, zoom) {
    return {
        x: (canvasPoint.x - minX) * zoom - viewportWidth / 2,
        y: (canvasPoint.y - minY) * zoom - viewportHeight / 2
    };
}

// Get viewport bounds in canvas coordinates
function getViewportBounds(contentX, contentY, viewportWidth, viewportHeight, zoom, minX, minY) {
    return {
        left: contentX / zoom + minX,
        top: contentY / zoom + minY,
        right: (contentX + viewportWidth) / zoom + minX,
        bottom: (contentY + viewportHeight) / zoom + minY,
        width: viewportWidth / zoom,
        height: viewportHeight / zoom
    };
}