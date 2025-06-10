import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    objectName: "viewportOverlay"
    
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
        if (!selectedElements || selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
        }
        return minX
    }
    
    property real selectionBoundingY: {
        if (!selectedElements || selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
        }
        return minY
    }
    
    property real selectionBoundingWidth: {
        if (!selectedElements || selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        var maxX = -Infinity  
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
            maxX = Math.max(maxX, selectedElements[i].x + selectedElements[i].width)
        }
        return maxX - minX
    }
    
    property real selectionBoundingHeight: {
        if (!selectedElements || selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        var maxY = -Infinity
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
            maxY = Math.max(maxY, selectedElements[i].y + selectedElements[i].height)
        }
        return maxY - minY
    }
    
    // Controls that follow selected elements
    Controls {
        id: selectionControls
        visible: {
            if (selectedElements && selectedElements.length > 0) {
                return true
            }
            if (creationDragHandler && creationDragHandler.active) {
                return true
            }
            return false
        }
        
        onVisibleChanged: {
            console.log("Controls visibility changed to:", visible)
            console.log("  selectedElements.length:", selectedElements ? selectedElements.length : "null")
            console.log("  creationDragHandler.active:", creationDragHandler ? creationDragHandler.active : "null")
        }
        
        // Initialize from selection bounding box
        Component.onCompleted: {
            console.log("Controls onCompleted - selectedElements:", selectedElements)
            console.log("Controls onCompleted - selectedElements.length:", selectedElements ? selectedElements.length : "null")
            console.log("Controls onCompleted - creationDragHandler:", creationDragHandler)
            console.log("Controls onCompleted - creationDragHandler.active:", creationDragHandler ? creationDragHandler.active : "null")
            initializeFromSelection()
        }
        
        // Re-initialize when selection changes
        Connections {
            target: root
            function onSelectedElementsChanged() {
                if (selectedElements.length > 0 && !selectionControls.dragging) {
                    selectionControls.initializeFromSelection()
                }
            }
        }
        
        // Update during creation drag
        property bool creationActive: creationDragHandler ? creationDragHandler.active : false
        onCreationActiveChanged: {
            if (creationActive) {
                updateFromCreationDrag()
            }
        }
        
        // Watch for creation drag updates
        property point creationStart: creationDragHandler ? creationDragHandler.startPoint : Qt.point(0, 0)
        property point creationCurrent: creationDragHandler ? creationDragHandler.currentPoint : Qt.point(0, 0)
        onCreationStartChanged: if (creationActive) updateFromCreationDrag()
        onCreationCurrentChanged: if (creationActive) updateFromCreationDrag()
        
        function initializeFromSelection() {
            console.log("initializeFromSelection called, selectedElements.length:", selectedElements ? selectedElements.length : "null")
            if (!selectedElements || selectedElements.length === 0) {
                // Reset controls to zero size when no selection
                width = 0
                height = 0
                return
            }
            
            // Set control properties from selection bounds
            controlX = selectionBoundingX
            controlY = selectionBoundingY
            controlWidth = selectionBoundingWidth
            controlHeight = selectionBoundingHeight
            controlRotation = 0
            
            // Convert to viewport coordinates for display
            updateViewportPosition()
        }
        
        function updateFromCreationDrag() {
            if (!creationDragHandler || !creationDragHandler.active) return
            
            var startX = Math.min(creationDragHandler.startPoint.x, creationDragHandler.currentPoint.x)
            var startY = Math.min(creationDragHandler.startPoint.y, creationDragHandler.currentPoint.y)
            var width = Math.abs(creationDragHandler.currentPoint.x - creationDragHandler.startPoint.x)
            var height = Math.abs(creationDragHandler.currentPoint.y - creationDragHandler.startPoint.y)
            
            // Set control properties from creation bounds
            controlX = startX
            controlY = startY
            controlWidth = width
            controlHeight = height
            controlRotation = 0
            
            // Convert to viewport coordinates for display
            updateViewportPosition()
        }
        
        function updateViewportPosition() {
            x = (controlX - canvasMinX) * zoomLevel - flickable.contentX
            y = (controlY - canvasMinY) * zoomLevel - flickable.contentY
            width = Math.abs(controlWidth) * zoomLevel
            height = Math.abs(controlHeight) * zoomLevel
        }
        
        // Update viewport position when view changes
        Connections {
            target: root
            function onZoomLevelChanged() { selectionControls.updateViewportPosition() }
        }
        
        Connections {
            target: flickable
            function onContentXChanged() { selectionControls.updateViewportPosition() }
            function onContentYChanged() { selectionControls.updateViewportPosition() }
        }
        
        // Track initial states when drag starts
        property var initialElementStates: []
        property real initialControlX: 0
        property real initialControlY: 0
        property real initialControlWidth: 0
        property real initialControlHeight: 0
        
        onDraggingChanged: {
            if (dragging) {
                captureInitialStates()
            } else {
                initialElementStates = []
                updateViewportPosition() // Ensure position is correct after drag
            }
        }
        
        // Update elements when controls change during drag
        onControlXChanged: if (dragging) updateElements()
        onControlYChanged: if (dragging) updateElements()
        onControlWidthChanged: if (dragging) updateElements()
        onControlHeightChanged: if (dragging) updateElements()
        onControlRotationChanged: if (dragging) updateElements()
        
        function captureInitialStates() {
            // Store initial control bounds
            initialControlX = controlX
            initialControlY = controlY
            initialControlWidth = controlWidth
            initialControlHeight = controlHeight
            
            var states = []
            for (var i = 0; i < selectedElements.length; i++) {
                var element = selectedElements[i]
                states.push({
                    element: element,
                    x: element.x,
                    y: element.y,
                    width: element.width,
                    height: element.height
                })
            }
            initialElementStates = states
        }
        
        function updateElements() {
            if (initialElementStates.length === 0) return
            
            if (dragMode === "move") {
                // Simple translation
                var deltaX = controlX - initialControlX
                var deltaY = controlY - initialControlY
                
                for (var i = 0; i < initialElementStates.length; i++) {
                    var state = initialElementStates[i]
                    state.element.x = state.x + deltaX
                    state.element.y = state.y + deltaY
                }
            } else if (dragMode.startsWith("resize")) {
                // Calculate scale factors from initial control size
                var scaleX = controlWidth / initialControlWidth
                var scaleY = controlHeight / initialControlHeight
                
                // Use absolute values for scaling calculations
                var absScaleX = Math.abs(scaleX)
                var absScaleY = Math.abs(scaleY)
                
                // Determine if we need to flip
                var flipX = scaleX < 0
                var flipY = scaleY < 0
                
                for (var i = 0; i < initialElementStates.length; i++) {
                    var state = initialElementStates[i]
                    
                    // Calculate relative position within initial control bounds
                    var relX = state.x - initialControlX
                    var relY = state.y - initialControlY
                    
                    // Apply scaling with minimum size constraint
                    var newWidth = Math.max(1, state.width * absScaleX)
                    var newHeight = Math.max(1, state.height * absScaleY)
                    
                    // Calculate new position
                    var newX, newY
                    
                    if (flipX) {
                        // Flip horizontally: mirror around the center
                        newX = controlX + Math.abs(controlWidth) - (relX + state.width) * absScaleX
                    } else {
                        // Normal positioning
                        newX = controlX + relX * absScaleX
                    }
                    
                    if (flipY) {
                        // Flip vertically: mirror around the center
                        newY = controlY + Math.abs(controlHeight) - (relY + state.height) * absScaleY
                    } else {
                        // Normal positioning
                        newY = controlY + relY * absScaleY
                    }
                    
                    // Update element
                    state.element.x = newX
                    state.element.y = newY
                    state.element.width = newWidth
                    state.element.height = newHeight
                }
            }
            
            // Update viewport position during drag
            updateViewportPosition()
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