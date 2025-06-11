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
    property real zoomLevel: canvasView?.zoomLevel ?? 1.0
    property var flickable: canvasView?.flickable ?? null
    property var hoveredElement: null
    property var selectionManager: canvasView?.selectionManager ?? null
    property var selectedElements: selectionManager?.selectedElements ?? []
    property var creationDragHandler: canvasView?.creationDragHandler ?? null
    property var controller: canvasView?.controller ?? null
    
    // Canvas bounds from canvasView
    property real canvasMinX: canvasView?.canvasMinX ?? 0
    property real canvasMinY: canvasView?.canvasMinY ?? 0
    
    // Selection box visual during drag selection
    SelectionBox {
        id: selectionBox
        selectionBoxHandler: canvasView?.selectionBoxHandler ?? null
    }
    
    // Use bounding box properties computed in C++ for better performance
    // Use readonly to ensure we always get the latest value from C++
    readonly property real selectionBoundingX: selectionManager?.boundingX ?? 0
    readonly property real selectionBoundingY: selectionManager?.boundingY ?? 0
    readonly property real selectionBoundingWidth: selectionManager?.boundingWidth ?? 0
    readonly property real selectionBoundingHeight: selectionManager?.boundingHeight ?? 0
    
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
        
        // Initialize from selection bounding box
        Component.onCompleted: {
            initializeFromSelection()
        }
        
        // Consolidated connections for selection updates
        Connections {
            target: root
            enabled: !selectionControls.dragging  // Disable during drag for performance
            
            function onSelectedElementsChanged() {
                if (selectedElements.length > 0) {
                    // Defer initialization to ensure bounding box is updated
                    Qt.callLater(selectionControls.initializeFromSelection)
                }
            }
        }
        
        // Direct connection to selection manager for bounding box updates
        Connections {
            target: selectionManager
            enabled: !selectionControls.dragging && selectionManager !== null
            
            function onSelectionChanged() {
                if (selectedElements.length > 0 && selectionControls.visible) {
                    // Update control properties when selection bounds change
                    selectionControls.controlX = selectionBoundingX
                    selectionControls.controlY = selectionBoundingY
                    selectionControls.controlWidth = selectionBoundingWidth
                    selectionControls.controlHeight = selectionBoundingHeight
                }
            }
        }
        
        // Ensure controls are positioned when they become visible
        onVisibleChanged: {
            if (visible && !dragging) {
                Qt.callLater(initializeFromSelection)
            }
        }
        
        // Update during creation drag
        property bool creationActive: creationDragHandler?.active ?? false
        onCreationActiveChanged: {
            if (creationActive) {
                updateFromCreationDrag()
            }
        }
        
        // Watch for creation drag updates with conditional execution
        property point creationStart: creationDragHandler?.startPoint ?? Qt.point(0, 0)
        property point creationCurrent: creationDragHandler?.currentPoint ?? Qt.point(0, 0)
        onCreationStartChanged: if (creationActive) updateFromCreationDrag()
        onCreationCurrentChanged: if (creationActive) updateFromCreationDrag()
        
        function initializeFromSelection() {
            if (!selectedElements || selectedElements.length === 0) {
                // Reset controls to zero size when no selection
                controlX = 0
                controlY = 0
                controlWidth = 0
                controlHeight = 0
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
        
        // Use Binding objects for efficient position updates
        Binding {
            target: selectionControls
            property: "x"
            value: (selectionControls.controlX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
            when: !selectionControls.dragging && selectionControls.visible
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "y"
            value: (selectionControls.controlY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
            when: !selectionControls.dragging && selectionControls.visible
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "width"
            value: Math.abs(selectionControls.controlWidth) * zoomLevel
            when: !selectionControls.dragging && selectionControls.visible
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "height"
            value: Math.abs(selectionControls.controlHeight) * zoomLevel
            when: !selectionControls.dragging && selectionControls.visible
            restoreMode: Binding.RestoreBinding
        }
        
        function updateViewportPosition() {
            // Manually update position when needed (e.g., during drag)
            x = (controlX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
            y = (controlY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
            width = Math.abs(controlWidth) * zoomLevel
            height = Math.abs(controlHeight) * zoomLevel
        }
        
        // Track initial states when drag starts
        property var initialElementStates: []
        property real initialControlX: 0
        property real initialControlY: 0
        property real initialControlWidth: 0
        property real initialControlHeight: 0
        
        // Frame throttling for drag updates
        property bool updateFramePending: false
        
        function scheduleUpdateFrame() {
            if (!updateFramePending) {
                updateFramePending = true
                Qt.callLater(function() {
                    updateElements()
                    updateFramePending = false
                })
            }
        }
        
        onDraggingChanged: {
            if (dragging) {
                captureInitialStates()
            } else {
                initialElementStates = []
                updateViewportPosition() // Ensure position is correct after drag
            }
        }
        
        // Update elements when controls change during drag - throttled to once per frame
        onControlXChanged: if (dragging) scheduleUpdateFrame()
        onControlYChanged: if (dragging) scheduleUpdateFrame()
        onControlWidthChanged: if (dragging) scheduleUpdateFrame()
        onControlHeightChanged: if (dragging) scheduleUpdateFrame()
        onControlRotationChanged: if (dragging) scheduleUpdateFrame()
        
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
    
    // Hover badge that shows dimensions during resize or rotation angle during rotate
    HoverBadge {
        id: hoverBadge
        parent: root
        visible: selectionControls.dragging && (selectionControls.dragMode.startsWith("resize-") || selectionControls.dragMode === "rotate")
        text: {
            if (selectionControls.dragMode === "rotate") {
                return Math.round(selectionControls.controlRotation) + "°"
            } else {
                return Math.round(Math.abs(selectionControls.controlWidth)) + " x " + Math.round(Math.abs(selectionControls.controlHeight))
            }
        }
        mousePosition: selectionControls.lastMousePosition
        z: Config.zHoverBadge
    }
}