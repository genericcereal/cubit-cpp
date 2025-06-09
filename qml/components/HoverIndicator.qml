import QtQuick
import Cubit.UI 1.0

// HoverIndicator.qml - Visual indicator that appears when hovering over an element
Rectangle {
    id: root
    
    property var hoveredElement: null
    property var selectionManager: null
    property real zoomLevel: 1.0
    property var flickable: null
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    visible: hoveredElement !== null && 
            (!selectionManager || 
             (hoveredElement && !hoveredElement.selected))
    color: "transparent"
    border.color: Config.hoverColor
    border.width: 1
    antialiasing: true
    
    x: hoveredElement ? (hoveredElement.x - canvasMinX) * zoomLevel - (flickable ? flickable.contentX : 0) : 0
    y: hoveredElement ? (hoveredElement.y - canvasMinY) * zoomLevel - (flickable ? flickable.contentY : 0) : 0
    width: hoveredElement ? hoveredElement.width * zoomLevel : 0
    height: hoveredElement ? hoveredElement.height * zoomLevel : 0
}