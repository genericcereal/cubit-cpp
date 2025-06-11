import QtQuick
import Cubit.UI 1.0

// SelectionBox.qml - Visual selection box that appears during drag selection
Rectangle {
    id: root
    
    property var selectionBoxHandler  // Handler from canvasView
    property real zoomLevel: parent.zoomLevel
    property var flickable: parent.flickable
    property real canvasMinX: parent.canvasMinX
    property real canvasMinY: parent.canvasMinY
    
    visible: selectionBoxHandler && selectionBoxHandler.active
    color: "transparent"
    border.color: Config.selectionColor
    border.width: 1
    antialiasing: true
    
    // Convert canvas coordinates to viewport coordinates
    x: {
        if (!selectionBoxHandler || !selectionBoxHandler.active || !flickable) return 0
        return (Math.min(selectionBoxHandler.startPoint.x, selectionBoxHandler.currentPoint.x) - canvasMinX) * zoomLevel - flickable.contentX
    }
    
    y: {
        if (!selectionBoxHandler || !selectionBoxHandler.active || !flickable) return 0
        return (Math.min(selectionBoxHandler.startPoint.y, selectionBoxHandler.currentPoint.y) - canvasMinY) * zoomLevel - flickable.contentY
    }
    
    width: {
        if (!selectionBoxHandler || !selectionBoxHandler.active) return 0
        return Math.abs(selectionBoxHandler.currentPoint.x - selectionBoxHandler.startPoint.x) * zoomLevel
    }
    
    height: {
        if (!selectionBoxHandler || !selectionBoxHandler.active) return 0
        return Math.abs(selectionBoxHandler.currentPoint.y - selectionBoxHandler.startPoint.y) * zoomLevel
    }
}