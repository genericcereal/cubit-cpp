import QtQuick
import Cubit 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    
    property var canvasView
    property real zoomLevel: canvasView ? canvasView.zoomLevel : 1.0
    property var flickable: canvasView ? canvasView.flickable : null
    property var hoveredElement: null
    property var selectionManager: canvasView ? canvasView.selectionManager : null
    property var selectedElements: selectionManager ? selectionManager.selectedElements : []
    property var creationDragHandler: canvasView ? canvasView.creationDragHandler : null
    property var controller: canvasView ? canvasView.controller : null
    
    // Selection box visual
    Rectangle {
        id: selectionBox
        visible: canvasView && canvasView.selectionBoxHandler.active
        color: "transparent"
        border.color: "#0066cc"
        border.width: 1
        
        // Convert canvas coordinates to viewport coordinates
        x: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return (Math.min(handler.startPoint.x, handler.currentPoint.x) - canvasView.canvasMinX) * zoomLevel - flickable.contentX
        }
        
        y: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return (Math.min(handler.startPoint.y, handler.currentPoint.y) - canvasView.canvasMinY) * zoomLevel - flickable.contentY
        }
        
        width: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.x - handler.startPoint.x) * zoomLevel
        }
        
        height: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.y - handler.startPoint.y) * zoomLevel
        }
    }
    
    // Selection rectangles and handles
    Repeater {
        model: selectedElements
        
        Item {
            property var element: modelData
            
            // Control bars and joints
            Item {
                x: element ? ((element.x - canvasView.canvasMinX) * zoomLevel - flickable.contentX) : 0
                y: element ? ((element.y - canvasView.canvasMinY) * zoomLevel - flickable.contentY) : 0
                width: element ? (element.width * zoomLevel) : 0
                height: element ? (element.height * zoomLevel) : 0
                
                // Top bar
                Rectangle {
                    x: 0
                    y: -1
                    width: parent.width
                    height: 2
                    color: "#0066cc"
                }
                
                // Bottom bar
                Rectangle {
                    x: 0
                    y: parent.height - 1
                    width: parent.width
                    height: 2
                    color: "#0066cc"
                }
                
                // Left bar
                Rectangle {
                    x: -1
                    y: 0
                    width: 2
                    height: parent.height
                    color: "#0066cc"
                }
                
                // Right bar
                Rectangle {
                    x: parent.width - 1
                    y: 0
                    width: 2
                    height: parent.height
                    color: "#0066cc"
                }
                
                // Corner handles (joints) - only show for single selection
                Repeater {
                    model: selectedElements.length === 1 ? 4 : 0
                    Rectangle {
                        width: 8
                        height: 8
                        color: "#0066cc"
                        
                        x: {
                            switch(index) {
                                case 0: return -4 // top-left
                                case 1: return parent.width - 4 // top-right
                                case 2: return parent.width - 4 // bottom-right
                                case 3: return -4 // bottom-left
                            }
                        }
                        
                        y: {
                            switch(index) {
                                case 0: return -4 // top-left
                                case 1: return -4 // top-right
                                case 2: return parent.height - 4 // bottom-right
                                case 3: return parent.height - 4 // bottom-left
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Hover indicator
    Rectangle {
        id: hoverIndicator
        visible: hoveredElement !== null && 
                (!canvasView.selectionManager || 
                 hoveredElement && !hoveredElement.selected)
        color: "transparent"
        border.color: "#999999"
        border.width: 1
        
        x: hoveredElement ? (hoveredElement.x - canvasView.canvasMinX) * zoomLevel - flickable.contentX : 0
        y: hoveredElement ? (hoveredElement.y - canvasView.canvasMinY) * zoomLevel - flickable.contentY : 0
        width: hoveredElement ? hoveredElement.width * zoomLevel : 0
        height: hoveredElement ? hoveredElement.height * zoomLevel : 0
    }
}