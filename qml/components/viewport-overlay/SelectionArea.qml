import QtQuick
import Cubit 1.0
import "OverlayUtils.js" as OverlayUtils

// SelectionArea handles drag-to-select functionality
Item {
    id: root
    
    // Properties passed from ViewportOverlay
    property var controller
    property var selectionManager
    property var flickable
    property real zoomLevel: 1.0
    property real canvasMinX: 0
    property real canvasMinY: 0
    property var selectionControls
    
    // Direct mouse handling for selection box
    MouseArea {
        id: selectionMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        propagateComposedEvents: true
        z: -1  // Behind other overlays but above canvas
        
        property bool isSelecting: false
        property point selectionStartPoint: Qt.point(0, 0)
        
        onPressed: (mouse) => {
            // Only start selection if:
            // 1. We're in select mode
            // 2. Click is not on a control
            // 3. Click is not on an element
            if (!OverlayUtils.inMode(controller, 0)) { // 0 is CanvasController.Select
                mouse.accepted = false
                return
            }
            
            // Check if controls are visible and under mouse
            if (selectionControls && selectionControls.visible && 
                selectionControls.contains(selectionControls.mapFromItem(root, mouse.x, mouse.y))) {
                mouse.accepted = false
                return
            }
            
            // Convert to canvas coordinates
            var canvasPoint = OverlayUtils.toCanvas(Qt.point(mouse.x, mouse.y), flickable, zoomLevel, canvasMinX, canvasMinY)
            
            // Check if clicking on an element
            var element = controller?.hitTest(canvasPoint.x, canvasPoint.y) ?? null
            if (element) {
                mouse.accepted = false
                return
            }
            
            // Start selection
            isSelecting = true
            selectionStartPoint = canvasPoint
            selectionBox.startPoint = canvasPoint
            selectionBox.currentPoint = canvasPoint
            selectionBox.active = true
            
            // Clear existing selection
            if (selectionManager) {
                selectionManager.clearSelection()
            }
            
            mouse.accepted = true
        }
        
        onPositionChanged: (mouse) => {
            if (!isSelecting) {
                mouse.accepted = false
                return
            }
            
            // Convert to canvas coordinates
            var canvasPoint = OverlayUtils.toCanvas(Qt.point(mouse.x, mouse.y), flickable, zoomLevel, canvasMinX, canvasMinY)
            
            // Update selection box
            selectionBox.currentPoint = canvasPoint
            
            // Calculate selection rectangle
            var rect = Qt.rect(
                Math.min(selectionStartPoint.x, canvasPoint.x),
                Math.min(selectionStartPoint.y, canvasPoint.y),
                Math.abs(canvasPoint.x - selectionStartPoint.x),
                Math.abs(canvasPoint.y - selectionStartPoint.y)
            )
            
            // Select elements in rect through the controller
            if (controller?.selectElementsInRect) {
                controller.selectElementsInRect(rect)
            }
            
            mouse.accepted = true
        }
        
        onReleased: (mouse) => {
            if (!isSelecting) {
                mouse.accepted = false
                return
            }
            
            isSelecting = false
            selectionBox.active = false
            mouse.accepted = true
        }
    }
    
    // Selection box visual during drag selection
    SelectionBox {
        id: selectionBox
    }
}