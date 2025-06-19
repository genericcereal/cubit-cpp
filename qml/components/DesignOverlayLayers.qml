import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    required property var controller
    required property var selectionManager
    required property var creationDragHandler
    required property var hoveredElement
    required property real canvasMinX
    required property real canvasMinY
    required property real zoom
    
    // Creation preview rectangle
    Rectangle {
        visible: creationDragHandler.active
        x: Math.min(creationDragHandler.startPoint.x, creationDragHandler.currentPoint.x) - root.canvasMinX
        y: Math.min(creationDragHandler.startPoint.y, creationDragHandler.currentPoint.y) - root.canvasMinY
        width: Math.abs(creationDragHandler.currentPoint.x - creationDragHandler.startPoint.x)
        height: Math.abs(creationDragHandler.currentPoint.y - creationDragHandler.startPoint.y)
        color: "transparent"
        border.color: "#0066cc"
        border.width: 1 / root.zoom
        opacity: 0.7
    }
    
    // Hover highlight
    Rectangle {
        id: hoverRect
        visible: hoveredElement !== null && controller.mode === CanvasController.Select
        x: hoveredElement ? hoveredElement.x - root.canvasMinX : 0
        y: hoveredElement ? hoveredElement.y - root.canvasMinY : 0
        width: hoveredElement ? hoveredElement.width : 0
        height: hoveredElement ? hoveredElement.height : 0
        color: "transparent"
        border.color: "#ffaa00"
        border.width: 2 / root.zoom
        opacity: 0.5
    }
}