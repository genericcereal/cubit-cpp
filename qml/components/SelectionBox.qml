import QtQuick
import QtQml
import Cubit.UI 1.0

// SelectionBox.qml - Visual selection box that appears during drag selection
Rectangle {
    id: root
    
    // Direct properties for selection box control
    property bool active: false
    property point startPoint: Qt.point(0, 0)
    property point currentPoint: Qt.point(0, 0)
    
    // Parent properties
    property real zoomLevel: parent.zoomLevel
    property var flickable: parent.flickable
    property real canvasMinX: parent.canvasMinX
    property real canvasMinY: parent.canvasMinY
    
    visible: active
    color: "transparent"
    border.color: Config.selectionColor
    border.width: 1
    antialiasing: true
    
    // Cached canvas coordinates for better performance
    property real canvasX: 0
    property real canvasY: 0
    property real canvasWidth: 0
    property real canvasHeight: 0
    
    // Update coordinates when points change
    onStartPointChanged: updateCanvasCoordinates()
    onCurrentPointChanged: updateCanvasCoordinates()
    
    function updateCanvasCoordinates() {
        if (!active) {
            canvasX = 0
            canvasY = 0
            canvasWidth = 0
            canvasHeight = 0
            return
        }
        
        canvasX = Math.min(startPoint.x, currentPoint.x)
        canvasY = Math.min(startPoint.y, currentPoint.y)
        canvasWidth = Math.abs(currentPoint.x - startPoint.x)
        canvasHeight = Math.abs(currentPoint.y - startPoint.y)
    }
    
    // Use Binding objects for efficient position updates
    Binding {
        target: root
        property: "x"
        value: (root.canvasX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
        when: root.active
        restoreMode: Binding.RestoreBinding
    }
    
    Binding {
        target: root
        property: "y"
        value: (root.canvasY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
        when: root.active
        restoreMode: Binding.RestoreBinding
    }
    
    Binding {
        target: root
        property: "width"
        value: root.canvasWidth * zoomLevel
        when: root.active
        restoreMode: Binding.RestoreBinding
    }
    
    Binding {
        target: root
        property: "height"
        value: root.canvasHeight * zoomLevel
        when: root.active
        restoreMode: Binding.RestoreBinding
    }
}