import QtQuick
import Cubit 1.0
import "../design-controls"
import "../shape-controls"
import "OverlayUtils.js" as OverlayUtils

// DesignControlsOverlay contains the complex DesignControls block
Item {
    id: root
    
    // Properties passed from ViewportOverlay
    property var canvasView
    property var controller
    property var selectionManager
    property var selectedElements: selectionManager?.selectedElements ?? []
    property var flickable
    property real zoomLevel: 1.0
    property real canvasMinX: 0
    property real canvasMinY: 0
    property string canvasType: "design"
    property var designControlsController: null
    property var shapeControlsController: null
    
    Component.onCompleted: {
        // DesignControlsOverlay initialized
    }
    
    onSelectionManagerChanged: {
        // SelectionManager changed
    }
    
    // Bounding box properties
    readonly property real selectionBoundingX: selectionManager?.boundingX ?? 0
    readonly property real selectionBoundingY: selectionManager?.boundingY ?? 0
    readonly property real selectionBoundingWidth: selectionManager?.boundingWidth ?? 0
    readonly property real selectionBoundingHeight: selectionManager?.boundingHeight ?? 0
    
    // Expose the selection controls for external access
    property alias selectionControls: selectionControls
    property alias shapeControls: shapeControls
    
    // Controls that follow selected elements (for design and variant canvases)
    DesignControls {
        id: selectionControls
        designControlsController: root.designControlsController
        canvas: root.canvasView ? root.canvasView.canvas : null
        visible: {
            // Controls visible on design and variant canvases
            if (canvasType !== "design" && canvasType !== "variant") {
                return false
            }
            // Hide during animations
            if (controller && controller.isAnimating) {
                return false
            }
            // IMPORTANT: Hide whenever ShapeControls are visible to prevent mouse event conflicts
            if (shapeControls.visible) {
                return false
            }
            // Hide when editing shapes - this is when ShapeControls are active
            if (controller && controller.isEditingShape) {
                return false
            }
            // Hide when shape controls are actively dragging (prevents reactivation during drag)
            if (controller && controller.isShapeControlDragging) {
                return false
            }
            // Hide during pen creation mode
            if (controller && controller.mode === CanvasController.ShapePen) {
                return false
            }
            // Also hide if ShapeControls are currently dragging
            if (shapeControls.visible && shapeControls.dragging) {
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
            enabled: !selectionControls.dragging && !(shapeControls.visible && shapeControls.dragging)  // Disable during drag for performance and during shape editing
            
            function onSelectedElementsChanged() {
                // Handle selection change
                // Exit shape editing mode if selection changes
                if (controller && controller.isEditingShape) {
                    controller.isEditingShape = false
                }
                
                // Exit edit mode for any text elements that were being edited
                if (canvasView && canvasView.elementModel) {
                    var allElements = canvasView.elementModel.getAllElements()
                    for (var i = 0; i < allElements.length; i++) {
                        var element = allElements[i]
                        // Check for all text-based elements that can be edited
                        if ((element.elementType === "Text" || 
                             element.elementType === "TextComponentVariant" ||
                             element.elementType === "WebTextInput" ||
                             (element.elementType === "FrameComponentInstance" && element.hasOwnProperty('content'))) 
                            && element.isEditing) {
                            // This will trigger the save through the Connections in TextElement.qml
                            element.isEditing = false
                        }
                    }
                }
                
                if (selectedElements.length > 0) {
                    // Defer initialization to ensure bounding box is updated
                    Qt.callLater(selectionControls.initializeFromSelection)
                }
            }
        }
        
        // Direct connection to selection manager for bounding box updates
        Connections {
            target: selectionManager
            enabled: !selectionControls.dragging && selectionManager !== null && !(shapeControls.visible && shapeControls.dragging)
            
            function onSelectionChanged() {
                // Handle selection change from SelectionManager
                if (selectedElements.length > 0 && selectionControls.visible) {
                    // Use regular selection bounds
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
                    selectionManager !== null &&
                    !(shapeControls.visible && shapeControls.dragging)
            
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
            when: !selectionControls.dragging && selectionControls.visible && !(shapeControls.visible && shapeControls.dragging)
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "y"
            value: (selectionControls.controlY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
            when: !selectionControls.dragging && selectionControls.visible && !(shapeControls.visible && shapeControls.dragging)
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "width"
            value: Math.abs(selectionControls.controlWidth) * zoomLevel
            when: !selectionControls.dragging && selectionControls.visible && !(shapeControls.visible && shapeControls.dragging)
            restoreMode: Binding.RestoreBinding
        }
        
        Binding {
            target: selectionControls
            property: "height"
            value: Math.abs(selectionControls.controlHeight) * zoomLevel
            when: !selectionControls.dragging && selectionControls.visible && !(shapeControls.visible && shapeControls.dragging)
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
                    var canvasPoint = OverlayUtils.toCanvas(viewportPos, flickable, zoomLevel, canvasMinX, canvasMinY)
                    canvasView.handleHover(canvasPoint)
                }
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
                // When dragging ends, sync controls with selection bounding box
                if (selectionManager && selectionManager.hasVisualSelection) {
                    controlX = selectionBoundingX
                    controlY = selectionBoundingY
                    controlWidth = selectionBoundingWidth
                    controlHeight = selectionBoundingHeight
                }
                
                initialElementStates = []
                updateViewportPosition() // Ensure position is correct after drag
                // Clear isResizing on the canvas when ending drag
                if (canvasView) {
                    canvasView.isResizing = false
                }
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
            var parentElementsToCapture = {} // Track parent elements we need to capture
            
            for (var i = 0; i < selectedElements.length; i++) {
                var element = selectedElements[i]
                states.push({
                    element: element,
                    x: element.x,
                    y: element.y,
                    width: element.width,
                    height: element.height
                })
                
                // Check if this is a relatively positioned child in a flex parent
                // Only capture parent state during resize operations, not move operations
                if (dragMode.startsWith("resize") && element.elementType === "Frame" && element.position === 0 && element.parentId) { // 0 = Relative
                    // We'll need to capture the parent's state too for resize operations
                    parentElementsToCapture[element.parentId] = true
                }
                
                // Also capture children of this element
                if (canvasView && canvasView.elementModel) {
                    var children = canvasView.elementModel.getChildrenRecursive(element.elementId)
                    for (var j = 0; j < children.length; j++) {
                        var child = children[j]
                        if (child && child.isVisual) {
                            states.push({
                                element: child,
                                x: child.x,
                                y: child.y,
                                width: child.width,
                                height: child.height
                            })
                        }
                    }
                }
            }
            
            // Now capture parent elements that we identified
            if (canvasView && canvasView.elementModel) {
                for (var parentId in parentElementsToCapture) {
                    var parentElement = canvasView.elementModel.getElementById(parentId)
                    if (parentElement && parentElement.elementType === "Frame" && parentElement.flex) {
                        // Check if we haven't already captured this parent
                        var alreadyCaptured = false
                        for (var k = 0; k < states.length; k++) {
                            if (states[k].element.elementId === parentId) {
                                alreadyCaptured = true
                                break
                            }
                        }
                        
                        if (!alreadyCaptured) {
                            states.push({
                                element: parentElement,
                                x: parentElement.x,
                                y: parentElement.y,
                                width: parentElement.width,
                                height: parentElement.height
                            })
                        }
                    }
                }
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
                    // Check if element is controlled
                    if (state.element.elementType === "Frame" && !state.element.controlled) {
                        continue // Skip uncontrolled frames
                    }
                    
                    // Check if element is a relatively positioned child in a flex parent
                    var isRelativeChildInFlexParent = false
                    if (state.element.elementType === "Frame" && state.element.position === 0) { // 0 = Relative
                        // Check if parent has flex enabled
                        if (canvasView && canvasView.elementModel) {
                            var parentElement = canvasView.elementModel.getElementById(state.element.parentId)
                            if (parentElement && parentElement.elementType === "Frame" && parentElement.flex) {
                                isRelativeChildInFlexParent = true
                            }
                        }
                    }
                    
                    // Skip moving relatively positioned children in flex parents
                    if (isRelativeChildInFlexParent) {
                        continue
                    }
                    
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
                
                // Create a set of selected element IDs for quick lookup
                var selectedIds = {}
                for (var j = 0; j < selectedElements.length; j++) {
                    selectedIds[selectedElements[j].elementId] = true
                }
                
                for (var i = 0; i < initialElementStates.length; i++) {
                    var state = initialElementStates[i]
                    var isDirectlySelected = selectedIds[state.element.elementId] === true
                    
                    // Check if element is a Frame with controlled=false
                    var isFrame = state.element.elementType === "Frame"
                    if (isFrame && !state.element.controlled) {
                        continue // Skip uncontrolled frames
                    }
                    
                    // Check if element is a relatively positioned child in a flex parent
                    var isRelativeChildInFlexParent = false
                    var parentElement = null
                    if (isFrame && state.element.position === 0) { // 0 = Relative
                        // Check if parent has flex enabled
                        if (canvasView && canvasView.elementModel) {
                            parentElement = canvasView.elementModel.getElementById(state.element.parentId)
                        }
                        if (parentElement && parentElement.elementType === "Frame" && parentElement.flex) {
                            isRelativeChildInFlexParent = true
                        }
                    }
                    
                    // Calculate relative position within initial control bounds
                    var relX = state.x - initialControlX
                    var relY = state.y - initialControlY
                    
                    if (isDirectlySelected && !isRelativeChildInFlexParent) {
                        // For directly selected elements, apply scaling
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
                    } else if (isDirectlySelected && isRelativeChildInFlexParent && parentElement) {
                        // For relatively positioned children in flex parents, resize the child
                        // Only resize the parent if its width/height type is set to "fit content"
                        
                        // Calculate the drag distance (difference between new and old control size)
                        var widthDelta = controlWidth - initialControlWidth
                        var heightDelta = controlHeight - initialControlHeight
                        
                        // First, resize the child by the drag distance
                        var newChildWidth = Math.max(1, state.width + widthDelta)
                        var newChildHeight = Math.max(1, state.height + heightDelta)
                        state.element.width = newChildWidth
                        state.element.height = newChildHeight
                        
                        // Find the parent's initial state
                        var parentInitialState = null
                        for (var k = 0; k < initialElementStates.length; k++) {
                            if (initialElementStates[k].element.elementId === parentElement.elementId) {
                                parentInitialState = initialElementStates[k]
                                break
                            }
                        }
                        
                        if (parentInitialState) {
                            // Only resize parent dimensions that are set to "fit content" (SizeFitContent = 3)
                            var shouldResizeWidth = parentElement.widthType === 3 // SizeFitContent
                            var shouldResizeHeight = parentElement.heightType === 3 // SizeFitContent
                            
                            if (shouldResizeWidth) {
                                var newParentWidth = Math.max(1, parentInitialState.width + widthDelta)
                                parentElement.width = newParentWidth
                            }
                            
                            if (shouldResizeHeight) {
                                var newParentHeight = Math.max(1, parentInitialState.height + heightDelta)
                                parentElement.height = newParentHeight
                            }
                        }
                        
                        // The flex layout engine will handle repositioning
                    } else {
                        // For child elements, maintain size and adjust position to stay relative to parent's left edge
                        // Only update position if parent moved (due to resize from left/top)
                        var deltaX = controlX - initialControlX
                        var deltaY = controlY - initialControlY
                        
                        // Don't move elements with controlled=false
                        if (!isRelativeChildInFlexParent) {
                            state.element.x = state.x + deltaX
                            state.element.y = state.y + deltaY
                        }
                        // Don't change width/height for children
                    }
                }
            }
            
            // Update viewport position during drag
            updateViewportPosition()
        }
    }
    
    // Hover badge that shows dimensions during resize or rotation angle during rotate (for design and variant canvases)
    Loader {
        id: hoverBadgeLoader
        parent: root
        active: canvasType === "design" || canvasType === "variant"
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
                return canvasView ? canvasView.lastMousePosition : {x: 0, y: 0}
            })
            item.z = ConfigObject.zHoverBadge
        }
    }
    
    // Shape controls - shown when editing shapes or during line creation
    ShapeControls {
        id: shapeControls
        
        // Store the last known visibility state to prevent flickering when controller temporarily becomes null
        property bool lastVisibleState: false
        
        visible: {
            // During dragging, maintain the last visible state
            if (shapeControls.isDragging) {
                return lastVisibleState
            }
            
            // Use root.controller (the canvas controller) for visibility check
            if (!root.controller) {
                lastVisibleState = false
                return false
            }
            
            // Show during pen creation mode (ShapePen) even without selection
            if (root.controller.mode === CanvasController.ShapePen) {
                // During line creation, we need at least one line shape being created
                if (selectedElements.length === 1) {
                    var selectedElement = selectedElements[0]
                    if (selectedElement && selectedElement.elementType === "Shape") {
                        lastVisibleState = true
                        return true
                    }
                }
            }
            
            // Normal shape editing mode
            if (selectedElements.length !== 1) {
                lastVisibleState = false
                return false
            }
            
            var selectedElement = selectedElements[0]
            if (!selectedElement || selectedElement.elementType !== "Shape") {
                lastVisibleState = false
                return false
            }
            
            // Show when explicitly editing shapes
            if (root.controller.isEditingShape) {
                lastVisibleState = true
                return true
            }
            
            lastVisibleState = false
            return false
        }
        selectedElement: selectedElements.length === 1 ? selectedElements[0] : null
        controller: root.shapeControlsController
        canvasController: root.controller
        
        // Bind position and size to viewport coordinates
        // Add padding for joint controls that extend beyond shape bounds
        property real jointPadding: 15 // Half of controlSize (30/2)
        
        x: {
            if (!selectedElement) return 0
            return (selectedElement.x - canvasMinX) * zoomLevel - (flickable ? flickable.contentX : 0) - jointPadding
        }
        y: {
            if (!selectedElement) return 0
            return (selectedElement.y - canvasMinY) * zoomLevel - (flickable ? flickable.contentY : 0) - jointPadding
        }
        width: {
            if (!selectedElement) return 0
            return selectedElement.width * zoomLevel + (jointPadding * 2)
        }
        height: {
            if (!selectedElement) return 0
            return selectedElement.height * zoomLevel + (jointPadding * 2)
        }
    }
}