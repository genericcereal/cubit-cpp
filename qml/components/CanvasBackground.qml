import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    
    // Background styling
    color: "#f5f5f5"
    antialiasing: true
    
    // Signal emitted when background is clicked
    signal clicked(point canvasPoint)
    
    // Signals for drag operations
    signal dragStarted(point canvasPoint)
    signal dragMoved(point canvasPoint)
    signal dragEnded(point canvasPoint)
    
    // Canvas reference for coordinate conversion
    property var canvas
    
    // Track if we're dragging
    property bool isDragging: false
    property point dragStartPos
    
    MouseArea {
        id: backgroundMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        property real clickThreshold: 5 // pixels
        
        onPressed: (mouse) => {
            if (root.canvas) {
                root.dragStartPos = Qt.point(mouse.x, mouse.y)
                root.isDragging = false
            }
        }
        
        onPositionChanged: (mouse) => {
            if (!root.canvas || !mouse.buttons) return
            
            // Update the canvas's lastMousePosition during drag
            // mouse.x, mouse.y are in background coordinates, need to map to viewport
            if (root.canvas.parent) {
                var viewportPos = mapToItem(root.canvas.parent, mouse.x, mouse.y)
                root.canvas.lastMousePosition = viewportPos
            }
            
            // Check if we've moved enough to start dragging
            if (!root.isDragging) {
                var dx = Math.abs(mouse.x - root.dragStartPos.x)
                var dy = Math.abs(mouse.y - root.dragStartPos.y)
                if (dx > clickThreshold || dy > clickThreshold) {
                    root.isDragging = true
                    // Emit drag started with the start position
                    var startCanvasX = root.dragStartPos.x + (root.canvas.canvasMinX || 0)
                    var startCanvasY = root.dragStartPos.y + (root.canvas.canvasMinY || 0)
                    root.dragStarted(Qt.point(startCanvasX, startCanvasY))
                }
            }
            
            // If dragging, emit drag moved
            if (root.isDragging) {
                var canvasX = mouse.x + (root.canvas.canvasMinX || 0)
                var canvasY = mouse.y + (root.canvas.canvasMinY || 0)
                root.dragMoved(Qt.point(canvasX, canvasY))
            }
        }
        
        onReleased: (mouse) => {
            if (root.canvas) {
                var canvasX = mouse.x + (root.canvas.canvasMinX || 0)
                var canvasY = mouse.y + (root.canvas.canvasMinY || 0)
                var canvasPoint = Qt.point(canvasX, canvasY)
                
                if (root.isDragging) {
                    // Emit drag ended
                    root.dragEnded(canvasPoint)
                } else {
                    // It was a click, not a drag
                    root.clicked(canvasPoint)
                }
                
                root.isDragging = false
            }
        }
    }
}