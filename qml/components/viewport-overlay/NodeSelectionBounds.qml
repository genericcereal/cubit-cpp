import QtQuick
import Cubit 1.0

// Visual bounding box that appears around selected nodes
Item {
    id: root
    
    property var selectionManager
    property real zoomLevel: 1.0
    property var flickable
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Only visible when multiple nodes are selected
    visible: {
        if (!selectionManager) return false
        var elements = selectionManager.selectedElements
        if (!elements || elements.length < 2) return false
        
        // Check if all selected elements are nodes
        for (var i = 0; i < elements.length; i++) {
            if (elements[i].objectName !== "Node") return false
        }
        return true
    }
    
    // Position and size from selection bounds
    x: selectionManager ? ((selectionManager.boundingX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)) : 0
    y: selectionManager ? ((selectionManager.boundingY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)) : 0
    width: selectionManager ? (selectionManager.boundingWidth * zoomLevel) : 0
    height: selectionManager ? (selectionManager.boundingHeight * zoomLevel) : 0
    
    // Blue border rectangle
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: ConfigObject.nodeSelectionBoundsColor  // Blue color from Config
        border.width: 1 / zoomLevel  // 1px border width constant regardless of zoom
        radius: 4
    }
}