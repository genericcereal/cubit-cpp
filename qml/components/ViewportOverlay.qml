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
    
    // Selection bounding box for all selected elements (design and variant canvases)
    SelectionBoundingBox {
        id: selectionBoundingBox
        visible: (canvasType === "design" || canvasType === "variant") && 
                 selectionManager && 
                 selectionManager.hasVisualSelection
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        // Use selection bounds directly
        boundingX: root.selectionBoundingX
        boundingY: root.selectionBoundingY
        boundingWidth: root.selectionBoundingWidth
        boundingHeight: root.selectionBoundingHeight
        z: -1  // Behind controls but above canvas
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
    
    // Controls that follow selected elements (for design and variant canvases)
    DesignControls {
        id: selectionControls
        visible: {
            // Controls visible on design and variant canvases
            if (canvasType !== "design" && canvasType !== "variant") {
                return false
            }
            // Hide during animations
            if (controller && controller.isAnimating) {
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
                // Exit edit mode for any text elements that were being edited
                if (canvasView && canvasView.elementModel) {
                    var allElements = canvasView.elementModel.getAllElements()
                    for (var i = 0; i < allElements.length; i++) {
                        var element = allElements[i]
                        if (element.elementType === "Text" && element.isEditing) {
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
            enabled: !selectionControls.dragging && selectionManager !== null
            
            function onSelectionChanged() {
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
                            // Capturing parent element state
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
                        console.log("Resizing relative child:", state.element.elementId, "Parent:", parentElement.elementId)
                        
                        // Calculate the drag distance (difference between new and old control size)
                        var widthDelta = controlWidth - initialControlWidth
                        var heightDelta = controlHeight - initialControlHeight
                        console.log("Resize deltas - width:", widthDelta, "height:", heightDelta)
                        
                        // First, resize the child by the drag distance
                        var newChildWidth = Math.max(1, state.width + widthDelta)
                        var newChildHeight = Math.max(1, state.height + heightDelta)
                        console.log("Setting child size to:", newChildWidth, "x", newChildHeight)
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
                            console.log("Found parent initial state - width:", parentInitialState.width, "height:", parentInitialState.height)
                            console.log("Parent widthType:", parentElement.widthType, "heightType:", parentElement.heightType)
                            
                            // Only resize parent dimensions that are set to "fit content" (SizeFitContent = 3)
                            var shouldResizeWidth = parentElement.widthType === 3 // SizeFitContent
                            var shouldResizeHeight = parentElement.heightType === 3 // SizeFitContent
                            
                            if (shouldResizeWidth) {
                                var newParentWidth = Math.max(1, parentInitialState.width + widthDelta)
                                console.log("Setting parent width to:", newParentWidth, "(widthType is fit-content)")
                                parentElement.width = newParentWidth
                            } else {
                                console.log("Not resizing parent width (widthType is not fit-content)")
                            }
                            
                            if (shouldResizeHeight) {
                                var newParentHeight = Math.max(1, parentInitialState.height + heightDelta)
                                console.log("Setting parent height to:", newParentHeight, "(heightType is fit-content)")
                                parentElement.height = newParentHeight
                            } else {
                                console.log("Not resizing parent height (heightType is not fit-content)")
                            }
                        } else {
                            console.log("WARNING: Could not find parent initial state!")
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
                return canvasView ? canvasView.lastMousePosition : Qt.point(0, 0)
            })
            item.z = Config.zHoverBadge
        }
    }
    
    // Access the prototype controller
    property var prototypeController: Application.activeCanvas ? Application.activeCanvas.prototypeController : null
    
    onPrototypeControllerChanged: {
        ConsoleMessageRepository.addOutput("ViewportOverlay: prototypeController changed to " + (prototypeController ? "valid controller" : "null"))
        
        // Connect requestCanvasMove signal to the canvas moveToPoint function
        if (prototypeController) {
            prototypeController.requestCanvasMove.connect(function(canvasPoint, animated) {
                if (canvasView && canvasView.moveToPoint) {
                    // Adjust the Y coordinate to account for the viewable area's position
                    // The prototype viewable area is positioned at a specific Y in the viewport
                    // We need to offset the canvas point so the frame aligns with the viewable area
                    var adjustedPoint = Qt.point(canvasPoint.x, canvasPoint.y)
                    
                    // Get the viewable area's actual position in viewport
                    if (prototypeViewableArea && prototypeViewableArea.visible) {
                        // The viewable area is now vertically centered in the viewport
                        var viewportHeight = root.flickable.height
                        var viewableAreaHeight = prototypeController.viewableArea.height
                        var viewableAreaTopInViewport = (viewportHeight - viewableAreaHeight) / 2
                        
                        // We need to find what canvas point to pass to moveToPoint
                        // so that the frame top appears at viewableAreaTopInViewport
                        
                        // Using the standard viewport formula: viewportY = (canvasY - canvasMinY) * zoom - contentY
                        // We want: viewableAreaTopInViewport = (frameTopY - canvasMinY) * zoom - contentY
                        // Solving for contentY: contentY = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport
                        
                        // But moveToPoint sets contentY based on centering a canvas point
                        // When point P is centered: contentY = (P - canvasMinY) * zoom - viewportHeight/2
                        
                        // Setting these equal:
                        // (P - canvasMinY) * zoom - viewportHeight/2 = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport
                        // (P - canvasMinY) * zoom = (frameTopY - canvasMinY) * zoom - viewableAreaTopInViewport + viewportHeight/2
                        // P = frameTopY + (viewportHeight/2 - viewableAreaTopInViewport) / zoom
                        
                        var frameTopY = canvasPoint.y  // This is the frame's top edge
                        var offset = (viewportHeight/2 - viewableAreaTopInViewport) / canvasView.zoom
                        
                        // Calculate the adjustment to align frame top with viewable area top
                        adjustedPoint.y = frameTopY + offset
                    }
                    
                    canvasView.moveToPoint(adjustedPoint, animated)
                    
                }
            })
        }
    }
    
    // Prototype viewable area - visible only when prototyping
    PrototypeViewableArea {
        id: prototypeViewableArea
        visible: {
            var hasController = prototypeController !== null
            var isPrototyping = hasController && prototypeController.isPrototyping
            var isAnimating = controller && controller.isAnimating
            var shouldShow = hasController && isPrototyping && !isAnimating
            
            return shouldShow
        }
        designCanvas: canvasView
        flickable: root.flickable
        prototypeController: root.prototypeController
    }
}