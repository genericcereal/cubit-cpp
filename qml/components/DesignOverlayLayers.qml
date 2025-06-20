import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    required property var controller
    required property var selectionManager
    required property var hoveredElement
    required property real canvasMinX
    required property real canvasMinY
    required property real zoom
    
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