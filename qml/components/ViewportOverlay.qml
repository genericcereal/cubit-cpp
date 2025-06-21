import QtQuick
import QtQml
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
    property var controller: canvasView?.controller ?? null
    property string canvasType: canvasView?.canvasType ?? "design"
    
    // Canvas bounds from canvasView
    property real canvasMinX: canvasView?.canvasMinX ?? 0
    property real canvasMinY: canvasView?.canvasMinY ?? 0
    
    // Expose the selection controls
    property alias selectionControls: selectionControls
    
    
    
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
            if (controller?.mode !== 0) { // 0 is CanvasController.Select
                mouse.accepted = false
                return
            }
            
            // Check if controls are visible and under mouse
            if (selectionControls.visible && selectionControls.contains(selectionControls.mapFromItem(root, mouse.x, mouse.y))) {
                mouse.accepted = false
                return
            }
            
            // Convert to canvas coordinates
            var canvasX = (mouse.x + (flickable?.contentX ?? 0)) / zoomLevel + canvasMinX
            var canvasY = (mouse.y + (flickable?.contentY ?? 0)) / zoomLevel + canvasMinY
            var canvasPoint = Qt.point(canvasX, canvasY)
            
            // Check if clicking on an element
            var element = controller?.hitTest(canvasX, canvasY) ?? null
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
            var canvasX = (mouse.x + (flickable?.contentX ?? 0)) / zoomLevel + canvasMinX
            var canvasY = (mouse.y + (flickable?.contentY ?? 0)) / zoomLevel + canvasMinY
            var canvasPoint = Qt.point(canvasX, canvasY)
            
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
    
    // Node selection bounds (only for script canvas)
    NodeSelectionBounds {
        id: nodeSelectionBounds
        visible: canvasType === "script"
        selectionManager: root.selectionManager
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
    
    // Use bounding box properties computed in C++ for better performance
    // Use readonly to ensure we always get the latest value from C++
    readonly property real selectionBoundingX: selectionManager?.boundingX ?? 0
    readonly property real selectionBoundingY: selectionManager?.boundingY ?? 0
    readonly property real selectionBoundingWidth: selectionManager?.boundingWidth ?? 0
    readonly property real selectionBoundingHeight: selectionManager?.boundingHeight ?? 0
    
    // Controls that follow selected elements (only for design canvas)
    DesignControls {
        id: selectionControls
        visible: {
            // Controls only visible on design canvas
            if (canvasType !== "design") {
                return false
            }
            // Only show controls if there are visual elements selected
            if (selectionManager && selectionManager.hasVisualSelection) {
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
        
        // Additional connection for creation mode - always update during element creation
        Connections {
            target: selectionManager
            enabled: canvasView && canvasView.controller && 
                    canvasView.controller.mode !== CanvasController.Select && 
                    selectionManager !== null
            
            function onSelectionChanged() {
                if (selectedElements.length > 0 && selectionControls.visible) {
                    // Always update control properties during creation
                    selectionControls.controlX = selectionBoundingX
                    selectionControls.controlY = selectionBoundingY
                    selectionControls.controlWidth = selectionBoundingWidth
                    selectionControls.controlHeight = selectionBoundingHeight
                    // Force visual update during creation
                    selectionControls.updateViewportPosition()
                    
                    // During creation, don't override the mouse position here
                    // Let the creationMouseTracker handle it so the hover badge follows the actual cursor
                }
            }
        }
        
        // Ensure controls are positioned when they become visible
        onVisibleChanged: {
            if (visible && !dragging) {
                Qt.callLater(initializeFromSelection)
            }
        }
        
        
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
        
        // Watch for dragMode changes
        onDragModeChanged: {
            // Update isResizing when dragMode changes during active drag
            if (dragging && canvasView) {
                if (dragMode.startsWith("resize-") || dragMode === "resize") {
                    canvasView.isResizing = true
                } else {
                    canvasView.isResizing = false
                }
            }
        }
        
        // Handle mouse drag for hover detection
        onMouseDragged: (viewportPos) => {
            // Update the canvas's lastMousePosition for hover badge
            if (canvasView) {
                canvasView.lastMousePosition = viewportPos
                
                // Also handle hover detection
                if (canvasView.handleHover) {
                    // Convert viewport position to canvas coordinates
                    var canvasX = (viewportPos.x + (flickable?.contentX ?? 0)) / zoomLevel + canvasMinX
                    var canvasY = (viewportPos.y + (flickable?.contentY ?? 0)) / zoomLevel + canvasMinY
                    var canvasPoint = Qt.point(canvasX, canvasY)
                    canvasView.handleHover(canvasPoint)
                }
            }
        }
        
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
                // Set isResizing on the canvas when starting resize drag
                if (canvasView && (dragMode.startsWith("resize-") || dragMode === "resize")) {
                    canvasView.isResizing = true
                }
            } else {
                initialElementStates = []
                updateViewportPosition() // Ensure position is correct after drag
                // Clear isResizing on the canvas when ending drag
                if (canvasView) {
                    canvasView.isResizing = false
                }
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
    
    // Hover badge that shows dimensions during resize or rotation angle during rotate (only for design canvas)
    Loader {
        id: hoverBadgeLoader
        parent: root
        active: canvasType === "design"
        sourceComponent: selectionControls.hoverBadge
        
        onLoaded: {
            item.parent = root
            item.visible = Qt.binding(function() { 
                // Show during resize (both standard resize and creation resize)
                if (canvasView && canvasView.isResizing) {
                    return true
                }
                // Show during rotation
                if (selectionControls.dragging && selectionControls.dragMode === "rotate") {
                    return true
                }
                return false
            })
            item.text = Qt.binding(function() {
                if (selectionControls.dragMode === "rotate") {
                    return Math.round(selectionControls.controlRotation) + "°"
                } else {
                    return Math.round(Math.abs(selectionControls.controlWidth)) + " x " + Math.round(Math.abs(selectionControls.controlHeight))
                }
            })
            item.mousePosition = Qt.binding(function() { 
                // Use the mouse position from BaseCanvas
                return canvasView ? canvasView.lastMousePosition : Qt.point(0, 0)
            })
            item.z = Config.zHoverBadge
        }
    }
}