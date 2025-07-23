import QtQuick
import QtQuick.Controls
import "../CanvasUtils.js" as Utils

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
    
    onCanvasChanged: {
        // Canvas changed
    }
    
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
                    // Convert mouse position to canvas coordinates using CanvasUtils
                    // First map to viewport coordinates relative to the canvas
                    var viewportPos = mapToItem(root.canvas, root.dragStartPos.x, root.dragStartPos.y)
                    var canvasPos = Utils.viewportToCanvas(
                        viewportPos,
                        root.canvas.flickable.contentX,
                        root.canvas.flickable.contentY,
                        root.canvas.zoom,
                        root.canvas.canvasMinX,
                        root.canvas.canvasMinY
                    )
                    // Drag started
                    root.dragStarted(canvasPos)
                }
            }
            
            // If dragging, emit drag moved
            if (root.isDragging) {
                // Convert mouse position to canvas coordinates using CanvasUtils
                var viewportPos = mapToItem(root.canvas, mouse.x, mouse.y)
                var canvasPos = Utils.viewportToCanvas(
                    viewportPos,
                    root.canvas.flickable.contentX,
                    root.canvas.flickable.contentY,
                    root.canvas.zoom,
                    root.canvas.canvasMinX,
                    root.canvas.canvasMinY
                )
                root.dragMoved(canvasPos)
            }
        }
        
        onReleased: (mouse) => {
            if (root.canvas) {
                // Convert mouse position to canvas coordinates using CanvasUtils
                var viewportPos = mapToItem(root.canvas, mouse.x, mouse.y)
                var canvasPoint = Utils.viewportToCanvas(
                    viewportPos,
                    root.canvas.flickable.contentX,
                    root.canvas.flickable.contentY,
                    root.canvas.zoom,
                    root.canvas.canvasMinX,
                    root.canvas.canvasMinY
                )
                
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