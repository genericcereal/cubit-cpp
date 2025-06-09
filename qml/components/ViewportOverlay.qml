import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    
    // Core properties from CanvasView
    property var canvasView
    property real zoomLevel: canvasView ? canvasView.zoomLevel : 1.0
    property var flickable: canvasView ? canvasView.flickable : null
    property var hoveredElement: null
    property var selectionManager: canvasView ? canvasView.selectionManager : null
    property var selectedElements: selectionManager ? selectionManager.selectedElements : []
    property var creationDragHandler: canvasView ? canvasView.creationDragHandler : null
    property var controller: canvasView ? canvasView.controller : null
    
    // Canvas bounds from canvasView
    property real canvasMinX: canvasView ? canvasView.canvasMinX : 0
    property real canvasMinY: canvasView ? canvasView.canvasMinY : 0
    
    // Selection box visual during drag selection
    SelectionBox {
        id: selectionBox
        selectionBoxHandler: canvasView ? canvasView.selectionBoxHandler : null
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
    
    // Calculate bounding box for all selected elements
    // These properties are used by the controls to determine their size and position
    property real selectionBoundingX: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
        }
        return minX
    }
    
    property real selectionBoundingY: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
        }
        return minY
    }
    
    property real selectionBoundingWidth: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        var maxX = -Infinity  
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
            maxX = Math.max(maxX, selectedElements[i].x + selectedElements[i].width)
        }
        return maxX - minX
    }
    
    property real selectionBoundingHeight: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        var maxY = -Infinity
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
            maxY = Math.max(maxY, selectedElements[i].y + selectedElements[i].height)
        }
        return maxY - minY
    }
    
    // Controls container for selected elements
    Item {
        id: controlsContainer
        visible: selectedElements.length > 0
        
        // Controls rotation angle
        property real controlsRotation: 0
        
        // Position and size based on selection bounding box
        property real controlX: (selectionBoundingX - root.canvasMinX) * zoomLevel - (flickable ? flickable.contentX : 0)
        property real controlY: (selectionBoundingY - root.canvasMinY) * zoomLevel - (flickable ? flickable.contentY : 0)
        property real controlWidth: selectionBoundingWidth * zoomLevel
        property real controlHeight: selectionBoundingHeight * zoomLevel
        
        x: controlX
        y: controlY
        width: controlWidth
        height: controlHeight
        
        // Apply rotation around center
        transform: Rotation {
            origin.x: controlsContainer.width / 2
            origin.y: controlsContainer.height / 2
            angle: controlsContainer.controlsRotation
        }
        
        // Helper function to convert viewport coordinates to canvas coordinates
        function viewportToCanvas(viewX, viewY) {
            var canvasX = (flickable.contentX + viewX) / zoomLevel + root.canvasMinX
            var canvasY = (flickable.contentY + viewY) / zoomLevel + root.canvasMinY
            return Qt.point(canvasX, canvasY)
        }
        
        // Inner rectangle (yellow with 5% opacity) - visual feedback for selection bounds
        Rectangle {
            anchors.fill: parent
            color: Config.controlInnerRectColor
            z: -1  // Behind other controls
            antialiasing: true
        }
        
        // Edge resize bars
        ControlBar {
            position: "top"
            controlsContainer: parent
            selectedElements: root.selectedElements
            zoomLevel: root.zoomLevel
            selectionBoundingX: root.selectionBoundingX
            selectionBoundingY: root.selectionBoundingY
            selectionBoundingWidth: root.selectionBoundingWidth
            selectionBoundingHeight: root.selectionBoundingHeight
        }
        
        ControlBar {
            position: "bottom"
            controlsContainer: parent
            selectedElements: root.selectedElements
            zoomLevel: root.zoomLevel
            selectionBoundingX: root.selectionBoundingX
            selectionBoundingY: root.selectionBoundingY
            selectionBoundingWidth: root.selectionBoundingWidth
            selectionBoundingHeight: root.selectionBoundingHeight
        }
        
        ControlBar {
            position: "left"
            controlsContainer: parent
            selectedElements: root.selectedElements
            zoomLevel: root.zoomLevel
            selectionBoundingX: root.selectionBoundingX
            selectionBoundingY: root.selectionBoundingY
            selectionBoundingWidth: root.selectionBoundingWidth
            selectionBoundingHeight: root.selectionBoundingHeight
        }
        
        ControlBar {
            position: "right"
            controlsContainer: parent
            selectedElements: root.selectedElements
            zoomLevel: root.zoomLevel
            selectionBoundingX: root.selectionBoundingX
            selectionBoundingY: root.selectionBoundingY
            selectionBoundingWidth: root.selectionBoundingWidth
            selectionBoundingHeight: root.selectionBoundingHeight
        }
        
        // Joints container
        Item {
            anchors.fill: parent
            
            // Rotation joints (blue, larger) - for future rotation functionality
            Repeater {
                model: 4
                ControlJoint {
                    jointType: "rotation"
                    position: {
                        switch(index) {
                            case 0: return "top-left"
                            case 1: return "top-right"
                            case 2: return "bottom-right"
                            case 3: return "bottom-left"
                        }
                    }
                    controlsContainer: controlsContainer
                    selectedElements: root.selectedElements
                    zoomLevel: root.zoomLevel
                    selectionBoundingX: root.selectionBoundingX
                    selectionBoundingY: root.selectionBoundingY
                    selectionBoundingWidth: root.selectionBoundingWidth
                    selectionBoundingHeight: root.selectionBoundingHeight
                }
            }
            
            // Resize joints (yellow, smaller) - for corner resizing
            Repeater {
                model: 4
                ControlJoint {
                    jointType: "resize"
                    position: {
                        switch(index) {
                            case 0: return "top-left"
                            case 1: return "top-right"
                            case 2: return "bottom-right"
                            case 3: return "bottom-left"
                        }
                    }
                    controlsContainer: controlsContainer
                    selectedElements: root.selectedElements
                    zoomLevel: root.zoomLevel
                    selectionBoundingX: root.selectionBoundingX
                    selectionBoundingY: root.selectionBoundingY
                    selectionBoundingWidth: root.selectionBoundingWidth
                    selectionBoundingHeight: root.selectionBoundingHeight
                }
            }
        }
    }
    
    // Hover indicator for elements under mouse
    HoverIndicator {
        hoveredElement: root.hoveredElement
        selectionManager: root.selectionManager
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
}