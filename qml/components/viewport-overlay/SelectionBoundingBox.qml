import QtQuick
import Cubit 1.0

// Visual bounding box that surrounds all selected elements
Rectangle {
    id: root
    
    // Properties for canvas coordinate system
    property real canvasMinX: 0
    property real canvasMinY: 0
    property real zoomLevel: 1.0
    property var flickable: null
    
    // Bounding box in canvas coordinates
    property real boundingX: 0
    property real boundingY: 0
    property real boundingWidth: 0
    property real boundingHeight: 0
    
    // Visual properties
    color: "transparent"
    border.color: Qt.rgba(0, 0, 0, 1)  // Fully opaque black
    border.width: 1
    visible: boundingWidth > 0 && boundingHeight > 0
    
    // Convert canvas coordinates to viewport coordinates
    x: (boundingX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
    y: (boundingY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
    width: boundingWidth * zoomLevel
    height: boundingHeight * zoomLevel
}