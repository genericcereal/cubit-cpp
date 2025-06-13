pragma Singleton
import QtQuick

QtObject {
    // Canvas bounds
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    property real canvasMaxX: 10000
    property real canvasMaxY: 10000
    
    // Canvas dimensions
    property real canvasWidth: canvasMaxX - canvasMinX
    property real canvasHeight: canvasMaxY - canvasMinY
    
    // Convert canvas coordinates to relative coordinates (for positioning elements)
    function canvasToRelative(canvasX, canvasY) {
        return Qt.point(canvasX - canvasMinX, canvasY - canvasMinY)
    }
    
    function canvasXToRelative(canvasX) {
        return canvasX - canvasMinX
    }
    
    function canvasYToRelative(canvasY) {
        return canvasY - canvasMinY
    }
    
    // Convert relative coordinates back to canvas coordinates
    function relativeToCanvas(relativeX, relativeY) {
        return Qt.point(relativeX + canvasMinX, relativeY + canvasMinY)
    }
    
    function relativeXToCanvas(relativeX) {
        return relativeX + canvasMinX
    }
    
    function relativeYToCanvas(relativeY) {
        return relativeY + canvasMinY
    }
    
    // Convert viewport coordinates to canvas coordinates
    function viewportToCanvas(viewportX, viewportY, contentX, contentY, zoomLevel) {
        var canvasX = (contentX + viewportX) / zoomLevel + canvasMinX
        var canvasY = (contentY + viewportY) / zoomLevel + canvasMinY
        return Qt.point(canvasX, canvasY)
    }
    
    function viewportXToCanvas(viewportX, contentX, zoomLevel) {
        return (contentX + viewportX) / zoomLevel + canvasMinX
    }
    
    function viewportYToCanvas(viewportY, contentY, zoomLevel) {
        return (contentY + viewportY) / zoomLevel + canvasMinY
    }
    
    // Convert canvas coordinates to viewport coordinates
    function canvasToViewport(canvasX, canvasY, contentX, contentY, zoomLevel) {
        var viewportX = (canvasX - canvasMinX) * zoomLevel - contentX
        var viewportY = (canvasY - canvasMinY) * zoomLevel - contentY
        return Qt.point(viewportX, viewportY)
    }
    
    function canvasXToViewport(canvasX, contentX, zoomLevel) {
        return (canvasX - canvasMinX) * zoomLevel - contentX
    }
    
    function canvasYToViewport(canvasY, contentY, zoomLevel) {
        return (canvasY - canvasMinY) * zoomLevel - contentY
    }
    
    // Calculate content position to center a canvas point in viewport
    function calculateContentPositionForCenter(canvasX, canvasY, viewportWidth, viewportHeight, zoomLevel) {
        var contentX = (canvasX - canvasMinX) * zoomLevel - viewportWidth / 2
        var contentY = (canvasY - canvasMinY) * zoomLevel - viewportHeight / 2
        return Qt.point(contentX, contentY)
    }
    
    // Calculate viewport bounds in canvas coordinates
    function getViewportBounds(contentX, contentY, viewportWidth, viewportHeight, zoomLevel) {
        return {
            left: contentX / zoomLevel + canvasMinX,
            top: contentY / zoomLevel + canvasMinY,
            right: (contentX + viewportWidth) / zoomLevel + canvasMinX,
            bottom: (contentY + viewportHeight) / zoomLevel + canvasMinY,
            width: viewportWidth / zoomLevel,
            height: viewportHeight / zoomLevel
        }
    }
    
    // Check if an element is visible in viewport
    function isElementInViewport(elementBounds, viewportBounds, margin) {
        margin = margin || 0
        return elementBounds.right >= (viewportBounds.left - margin) &&
               elementBounds.left <= (viewportBounds.right + margin) &&
               elementBounds.bottom >= (viewportBounds.top - margin) &&
               elementBounds.top <= (viewportBounds.bottom + margin)
    }
}