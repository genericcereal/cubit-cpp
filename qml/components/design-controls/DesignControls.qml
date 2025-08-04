import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."
import "ControlMath.js" as CM

Item {
    id: root
    
    // Start with 0 size to ensure controls are not visible until properly initialized
    width: 0
    height: 0
    
    property real controlX: 0
    property real controlY: 0
    property real controlWidth: 100
    property real controlHeight: 100
    property real controlRotation: 0
    
    // Design controls controller - passed from parent
    property var designControlsController: null
    // Canvas - passed from parent
    property var canvas: null
    
    // Helper function to get the controller (for child components)
    function getDesignControlsController() {
        return designControlsController
    }
    
    // Check if all selected elements are component-related
    property bool allSelectedAreComponentRelated: false
    property bool selectedIsComponentVariant: false
    
    // Check if resizing is disabled
    property bool isResizingDisabled: designControlsController ? !designControlsController.isResizingEnabled : true
    
    // Check if movement is disabled
    property bool isMovementDisabled: designControlsController ? !designControlsController.isMovementEnabled : true
    
    // Check if any text is being edited
    readonly property bool isAnyTextEditing: designControlsController ? designControlsController.isAnyTextEditing : false
    
    // Control state
    property bool dragging: false
    property string dragMode: "" // "move", "resize-edge-0", "resize-edge-1", etc.
    property bool showResizeJoints: {
        // Hide resize joints when dragging or when in prototyping mode
        if (dragging) {
            return false
        }
        
        // Check if we're in prototyping mode
        var viewportOverlay = root.parent
        if (viewportOverlay && viewportOverlay.prototypeController && viewportOverlay.prototypeController.isPrototyping) {
            return false
        }
        
        // Check if animations are currently playing
        if (viewportOverlay && viewportOverlay.canvasView) {
            var controller = viewportOverlay.canvasView.controller
            if (controller && controller.isAnimating) {
                return false
            }
        }
        
        return true
    }
    property point dragStartPoint
    property point dragStartControlPos
    property size dragStartControlSize
    property real dragStartRotation: 0
    property point lastMousePosition: Qt.point(0, 0)
    
    
    // Signal to notify about mouse position during drag
    signal mouseDragged(point viewportPos)
    
    // Signal to notify when resize operation starts
    signal resizeStarted()
    
    // Track if dimensions are flipped
    property bool widthFlipped: controlWidth < 0
    property bool heightFlipped: controlHeight < 0
    
    // Visual properties are set by parent, not bound to control properties
    rotation: controlRotation
    transformOrigin: Item.Center
    
    // Helper function to get rotation-adjusted cursor
    function getEdgeCursor(edgeIndex) {
        return CM.edgeCursorFor(controlRotation, edgeIndex);
    }
    
    // Helper function to get rotation-adjusted corner cursor
    function getCornerCursor(cornerIndex) {
        return CM.cornerCursorFor(controlRotation, cornerIndex);
    }
    
    // Helper function to update mouse position in both controls and canvas
    function updateMousePosition(pos) {
        root.lastMousePosition = pos
        // Also update the canvas's lastMousePosition if we can access it
        // The parent should be ViewportOverlay, which has canvasView
        if (root.parent && root.parent.canvasView) {
            root.parent.canvasView.lastMousePosition = pos
        }
    }
    
    // Throttled update component for move operations
    ThrottledUpdate {
        id: moveThrottle
        // interval uses default from ConfigObject.throttleInterval
        active: root.dragging && root.dragMode === "move"
        
        onUpdate: (data) => {
            // Update control position in canvas coordinates
            root.controlX = data.x
            root.controlY = data.y
            
            // Check for reordering if dragging a position relative child in a flex parent
            checkForReordering(data.mousePos)
            
            // Emit signal for hover detection
            root.mouseDragged(data.mousePos)
        }
    }
    
    // Throttled update component for edge resize operations
    ThrottledUpdate {
        id: edgeResizeThrottle
        // interval uses default from ConfigObject.throttleInterval
        active: root.dragging && root.dragMode.startsWith("resize-edge-")
        
        onUpdate: (data) => {
            // Update control dimensions and position
            root.controlWidth = data.width
            root.controlHeight = data.height
            root.controlX = data.x
            root.controlY = data.y
        }
    }
    
    // Throttled update component for corner resize operations
    ThrottledUpdate {
        id: cornerResizeThrottle
        // interval uses default from ConfigObject.throttleInterval
        active: root.dragging && root.dragMode.startsWith("resize-corner-")
        
        onUpdate: (data) => {
            // Update control dimensions and position
            root.controlWidth = data.width
            root.controlHeight = data.height
            root.controlX = data.x
            root.controlY = data.y
        }
    }
    
    // Control Surface - yellow transparent fill
    ControlSurface {
        id: controlSurface
        anchors.fill: parent
        dragging: root.dragging
        dragMode: root.dragMode
        dragStartPoint: root.dragStartPoint
        dragStartControlPos: root.dragStartControlPos
        
        onPressed: (mouse) => {
            // Store mouse position in parent coordinates to handle rotation correctly
            var mouseInParent = controlSurface.mapToItem(root.parent, mouse.x, mouse.y)
            root.updateMousePosition(mouseInParent)
            root.dragStartPoint = mouseInParent
            root.dragStartControlPos = Qt.point(root.controlX, root.controlY)
            
            // Store initial state for command creation via DesignControlsController
            if (root.designControlsController) {
                root.designControlsController.startDragOperation()
            }
            
            // Reset the drag threshold exceeded flag
            controlSurface.mouseArea.isDragThresholdExceeded = false
        }
        
        onReleased: (mouse) => {
            if (!controlSurface.isDragThresholdExceeded && root.parent && root.parent.selectedElements) {
                // This was a click, not a drag
                var mouseInParent = controlSurface.mapToItem(root.parent, mouse.x, mouse.y)
                
                // Convert to canvas coordinates for hit testing
                var canvasX = (mouseInParent.x + (root.parent.flickable?.contentX ?? 0)) / root.parent.zoomLevel + root.parent.canvasMinX
                var canvasY = (mouseInParent.y + (root.parent.flickable?.contentY ?? 0)) / root.parent.zoomLevel + root.parent.canvasMinY
                
                // Perform hit test
                var element = root.parent.controller?.hitTest(canvasX, canvasY) ?? null
                if (element && root.parent.selectionManager) {
                    // Check if we clicked on a different element than what's selected
                    var clickedDifferentElement = false
                    if (root.parent.selectedElements.length === 1) {
                        // Single selection - check if clicked element is different
                        clickedDifferentElement = element.elementId !== root.parent.selectedElements[0].elementId
                    } else if (root.parent.selectedElements.length > 1) {
                        // Multiple selection - always select the clicked element
                        clickedDifferentElement = true
                    }
                    
                    if (clickedDifferentElement) {
                        // Select only the element under the cursor
                        root.parent.selectionManager.selectOnly(element)
                    }
                }
            }
            
            // Fire MoveElementsCommand if this was a move operation
            if (controlSurface.isDragThresholdExceeded && root.dragMode === "move") {
                // Force final update before ending
                moveThrottle.forceUpdate()
                
                if (root.designControlsController) {
                    // Calculate total delta moved
                    var totalDelta = Qt.point(
                        (root.controlX - root.dragStartControlPos.x),
                        (root.controlY - root.dragStartControlPos.y)
                    )
                    
                    root.designControlsController.endMoveOperation(totalDelta)
                }
            }
            
            root.dragging = false
            root.dragMode = ""
            root.updateMousePosition(Qt.point(0, 0))
        }
        
        onDoubleClicked: (mouse) => {
            // Controls double-clicked
            
            // Access ViewportOverlay through root.parent
            var viewportOverlay = root.parent
            
            // Check if we have a single element selected
            if (viewportOverlay && viewportOverlay.selectedElements && viewportOverlay.selectedElements.length === 1) {
                var element = viewportOverlay.selectedElements[0]
                // Selected element type logged
                
                // Check if it's a Text element, TextComponentVariant, WebTextInput, or text-based ComponentInstance directly selected
                if (element && (element.elementType === "Text" || element.elementType === "TextComponentVariant" || 
                    element.elementType === "WebTextInput" ||
                    (element.elementType === "FrameComponentInstance" && element.hasOwnProperty("content")))) {
                    
                    // Only enable editing for WebTextInput when in prototype mode
                    if (element.elementType === "WebTextInput") {
                        var prototypeController = root.canvas ? root.canvas.prototypeController : null
                        if (!prototypeController || !prototypeController.isPrototyping) {
                            if (root.canvas && root.canvas.console) root.canvas.console.addOutput("WebTextInput editing disabled outside of prototype mode")
                            mouse.accepted = true
                            return
                        }
                    }
                    
                    if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Text-based element directly selected - triggering edit mode")
                    element.isEditing = true
                    mouse.accepted = true
                    return
                } else if (element && element.elementType === "Frame") {
                    if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Frame selected, checking for Text children...")
                    
                    // Check if this frame has any text children
                    if (viewportOverlay.canvasView && viewportOverlay.canvasView.elementModel) {
                        var allElements = viewportOverlay.canvasView.elementModel.getAllElements()
                        var textChildrenFound = 0
                        
                        for (var i = 0; i < allElements.length; i++) {
                            var child = allElements[i]
                            if (child.parentId === element.elementId && child.elementType === "Text") {
                                textChildrenFound++
                                if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Found Text child: " + child.name + " (ID: " + child.elementId + ")")
                                // Trigger edit mode for the first text child found
                                child.isEditing = true
                                mouse.accepted = true
                                return
                            }
                        }
                        
                        if (textChildrenFound === 0) {
                            if (root.canvas && root.canvas.console) root.canvas.console.addOutput("No Text children found in this Frame")
                        }
                    } else {
                        if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Unable to access elementModel")
                    }
                } else if (element && element.elementType === "Shape") {
                    // Shape selected - entering shape edit mode
                    
                    // Set isEditingShape on the shape controls controller
                    var viewportOverlay = root.parent
                    if (viewportOverlay && viewportOverlay.shapeControlsController) {
                        viewportOverlay.shapeControlsController.isEditingShape = true
                        mouse.accepted = true
                        return
                    }
                } else {
                    if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Selected element is neither Frame, Text, nor Shape")
                }
            } else if (viewportOverlay && viewportOverlay.selectedElements) {
                if (root.canvas && root.canvas.console) root.canvas.console.addOutput("Multiple elements selected: " + viewportOverlay.selectedElements.length)
            } else {
                if (root.canvas && root.canvas.console) root.canvas.console.addOutput("No elements selected or unable to access selection")
            }
            
            mouse.accepted = true
        }
        
        onPositionChanged: (mouse) => {
            var mouseInParent = controlSurface.mapToItem(root.parent, mouse.x, mouse.y)
            
            // Check if drag threshold exceeded
            if (!controlSurface.isDragThresholdExceeded) {
                var distance = Math.sqrt(
                    Math.pow(mouseInParent.x - root.dragStartPoint.x, 2) + 
                    Math.pow(mouseInParent.y - root.dragStartPoint.y, 2)
                )
                
                if (distance > controlSurface.mouseArea.dragThreshold) {
                    controlSurface.mouseArea.isDragThresholdExceeded = true
                    // Check if movement is allowed
                    if (!root.isMovementDisabled) {
                        root.dragging = true
                        root.dragMode = "move"
                    }
                }
            }
            
            if (root.dragMode === "move") {
                // Calculate delta in parent coordinates so movement follows mouse regardless of rotation
                root.updateMousePosition(mouseInParent)
                var deltaX = mouseInParent.x - root.dragStartPoint.x
                var deltaY = mouseInParent.y - root.dragStartPoint.y
                
                // Request throttled update instead of updating immediately
                moveThrottle.requestUpdate({
                    x: root.dragStartControlPos.x + deltaX / root.parent.zoomLevel,
                    y: root.dragStartControlPos.y + deltaY / root.parent.zoomLevel,
                    mousePos: mouseInParent
                })
            }
        }
    }
    
    // Edge Resize Bars - using repeater for rotation-aware behavior
    Repeater {
        model: 4 // 0=top, 1=right, 2=bottom, 3=left (before rotation)
        
        EdgeResizeBar {
            id: edgeBar
            edgeIndex: index
            allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
            controlRotation: root.controlRotation
            flexHeightFitContent: root.flexHeightFitContent
            flexWidthFitContent: root.flexWidthFitContent
            
            // Position and size based on edge index
            x: {
                switch(index) {
                    case 0: return -5 // top
                    case 1: return Math.round(parent.width) - 5 // right
                    case 2: return -5 // bottom
                    case 3: return -5 // left
                }
            }
            
            y: {
                switch(index) {
                    case 0: return -5 // top
                    case 1: return -5 // right
                    case 2: return Math.round(parent.height) - 5 // bottom
                    case 3: return -5 // left
                }
            }
            
            width: {
                switch(index) {
                    case 0: case 2: return Math.round(parent.width) + 10 // horizontal bars
                    case 1: case 3: return 10 // vertical bars
                }
            }
            
            height: {
                switch(index) {
                    case 0: case 2: return 10 // horizontal bars
                    case 1: case 3: return Math.round(parent.height) + 10 // vertical bars
                }
            }
            
            onResizeStarted: (edgeIdx, anchorEdgePoint1, anchorEdgePoint2, draggedEdgePoint1, draggedEdgePoint2) => {
                root.dragging = true
                root.dragMode = "resize-edge-" + edgeIdx
                root.resizeStarted()
                
                // Store initial state for command creation via DesignControlsController
                if (root.designControlsController) {
                    root.designControlsController.startDragOperation()
                }
                
                // Initialize mouse position
                var mouseInParent = mapToItem(root.parent, 0, 0)
                root.updateMousePosition(mouseInParent)
                
                // Store edge points for the drag operation
                edgeBar.anchorEdgePoint1 = anchorEdgePoint1
                edgeBar.anchorEdgePoint2 = anchorEdgePoint2
                edgeBar.draggedEdgePoint1 = draggedEdgePoint1
                edgeBar.draggedEdgePoint2 = draggedEdgePoint2
            }
            
            onResizePositionChanged: (mouseInParent) => {
                if (!root.dragMode.startsWith("resize-edge-")) return
                
                root.updateMousePosition(mouseInParent)
                
                // For constraint-based approach:
                // 1. The anchor edge stays fixed at anchorEdgePoint1 and anchorEdgePoint2
                // 2. The dragged edge should be at the mouse position
                
                // Calculate the vector along the anchor edge
                var anchorVector = Qt.point(
                    edgeBar.anchorEdgePoint2.x - edgeBar.anchorEdgePoint1.x,
                    edgeBar.anchorEdgePoint2.y - edgeBar.anchorEdgePoint1.y
                )
                
                // Calculate perpendicular vector (from anchor edge to dragged edge)
                var perpVector = Qt.point(-anchorVector.y, anchorVector.x)
                var perpLength = Math.sqrt(perpVector.x * perpVector.x + perpVector.y * perpVector.y)
                if (perpLength > 0) {
                    perpVector.x /= perpLength
                    perpVector.y /= perpLength
                }
                
                // Determine correct perpendicular direction
                // The perpendicular should point from anchor edge toward the original dragged edge
                var anchorCenter = Qt.point(
                    (edgeBar.anchorEdgePoint1.x + edgeBar.anchorEdgePoint2.x) / 2,
                    (edgeBar.anchorEdgePoint1.y + edgeBar.anchorEdgePoint2.y) / 2
                )
                var draggedCenter = Qt.point(
                    (edgeBar.draggedEdgePoint1.x + edgeBar.draggedEdgePoint2.x) / 2,
                    (edgeBar.draggedEdgePoint1.y + edgeBar.draggedEdgePoint2.y) / 2
                )
                
                // Vector from anchor to original dragged edge
                var originalDirection = Qt.point(
                    draggedCenter.x - anchorCenter.x,
                    draggedCenter.y - anchorCenter.y
                )
                
                // Make sure perpendicular points in the same general direction
                var dot = originalDirection.x * perpVector.x + originalDirection.y * perpVector.y
                if (dot < 0) {
                    perpVector.x = -perpVector.x
                    perpVector.y = -perpVector.y
                }
                
                // Calculate distance from mouse to anchor edge line
                var toMouse = Qt.point(
                    mouseInParent.x - edgeBar.anchorEdgePoint1.x,
                    mouseInParent.y - edgeBar.anchorEdgePoint1.y
                )
                var distance = toMouse.x * perpVector.x + toMouse.y * perpVector.y
                
                // Calculate the dragged edge points
                var draggedPoint1 = Qt.point(
                    edgeBar.anchorEdgePoint1.x + distance * perpVector.x,
                    edgeBar.anchorEdgePoint1.y + distance * perpVector.y
                )
                var draggedPoint2 = Qt.point(
                    edgeBar.anchorEdgePoint2.x + distance * perpVector.x,
                    edgeBar.anchorEdgePoint2.y + distance * perpVector.y
                )
                
                // Now we have the four corners of the new rectangle in parent coordinates
                var newCorners = []
                switch(edgeBar.edgeIndex) {
                    case 0: // dragged top edge
                        newCorners = [draggedPoint1, draggedPoint2, edgeBar.anchorEdgePoint2, edgeBar.anchorEdgePoint1]
                        break
                    case 1: // dragged right edge
                        newCorners = [edgeBar.anchorEdgePoint1, draggedPoint1, draggedPoint2, edgeBar.anchorEdgePoint2]
                        break
                    case 2: // dragged bottom edge
                        newCorners = [edgeBar.anchorEdgePoint1, edgeBar.anchorEdgePoint2, draggedPoint2, draggedPoint1]
                        break
                    case 3: // dragged left edge
                        newCorners = [draggedPoint1, edgeBar.anchorEdgePoint1, edgeBar.anchorEdgePoint2, draggedPoint2]
                        break
                }
                
                // Calculate center, dimensions, and rotation from the corners
                var centerX = (newCorners[0].x + newCorners[1].x + newCorners[2].x + newCorners[3].x) / 4
                var centerY = (newCorners[0].y + newCorners[1].y + newCorners[2].y + newCorners[3].y) / 4
                
                // Calculate width and height in local space
                var widthVector = Qt.point(newCorners[1].x - newCorners[0].x, newCorners[1].y - newCorners[0].y)
                var heightVector = Qt.point(newCorners[3].x - newCorners[0].x, newCorners[3].y - newCorners[0].y)
                
                var newWidth = Math.sqrt(widthVector.x * widthVector.x + widthVector.y * widthVector.y)
                var newHeight = Math.sqrt(heightVector.x * heightVector.x + heightVector.y * heightVector.y)
                
                // Determine sign based on distance direction
                // If distance is negative, it means we've flipped past the anchor edge
                switch(edgeBar.edgeIndex) {
                    case 0: // top edge
                    case 2: // bottom edge
                        if (distance < 0) newHeight = -newHeight
                        break
                    case 1: // right edge
                    case 3: // left edge
                        if (distance < 0) newWidth = -newWidth
                        break
                }
                
                // Calculate top-left in viewport coordinates
                // Note: centerX/centerY and the corners are already in viewport coordinates (parent.parent from EdgeResizeBar)
                var viewportX = centerX - Math.abs(newWidth) / 2
                var viewportY = centerY - Math.abs(newHeight) / 2
                
                // Request throttled update instead of updating immediately
                edgeResizeThrottle.requestUpdate({
                    width: newWidth / root.parent.zoomLevel,
                    height: newHeight / root.parent.zoomLevel,
                    x: viewportX / root.parent.zoomLevel + root.parent.canvasMinX + root.parent.flickable.contentX / root.parent.zoomLevel,
                    y: viewportY / root.parent.zoomLevel + root.parent.canvasMinY + root.parent.flickable.contentY / root.parent.zoomLevel
                })
            }
            
            onResizeEnded: {
                // Force final update before ending
                edgeResizeThrottle.forceUpdate()
                
                // Fire ResizeElementCommand via DesignControlsController
                if (root.designControlsController) {
                    root.designControlsController.endResizeOperation()
                }
                
                root.dragging = false
                root.dragMode = ""
                root.updateMousePosition(Qt.point(0, 0))
            }
            
            // Properties to store edge points during drag
            property point anchorEdgePoint1
            property point anchorEdgePoint2
            property point draggedEdgePoint1
            property point draggedEdgePoint2
        }
    }
    
    // Rotation Joints (red, 20x20, behind resize joints)
    Repeater {
        model: 4
        
        RotationJoint {
            id: rotationJoint
            cornerIndex: index
            allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
            controlRotation: root.controlRotation
            
            x: {
                switch(index) {
                    case 0: return -15 // top-left: bottom-right corner at (5, 5) to overlap resize joint
                    case 1: return Math.round(parent.width) - 5 // top-right: bottom-left corner at (parent.width - 5, 5) to overlap resize joint
                    case 2: return Math.round(parent.width) - 5 // bottom-right: top-left corner at (parent.width - 5, parent.height - 5) to overlap resize joint
                    case 3: return -15 // bottom-left: top-right corner at (5, parent.height - 5) to overlap resize joint
                }
            }
            
            y: {
                switch(index) {
                    case 0: return -15 // top-left: bottom-right corner at (5, 5) to overlap resize joint
                    case 1: return -15 // top-right: bottom-left corner at (parent.width - 5, 5) to overlap resize joint
                    case 2: return Math.round(parent.height) - 5 // bottom-right: top-left corner at (parent.width - 5, parent.height - 5) to overlap resize joint
                    case 3: return Math.round(parent.height) - 5 // bottom-left: top-right corner at (5, parent.height - 5) to overlap resize joint
                }
            }
            
            onRotationStarted: (startMousePos, startAngle) => {
                root.dragging = true
                root.dragMode = "rotate"
                root.updateMousePosition(startMousePos)
                root.dragStartRotation = root.controlRotation
            }
            
            onRotationPositionChanged: (currentMousePos, currentAngle) => {
                if (root.dragMode === "rotate") {
                    root.updateMousePosition(currentMousePos)
                    var deltaAngle = currentAngle - rotationJoint.startAngle
                    
                    // Normalize the angle difference to handle wrap-around
                    while (deltaAngle > 180) deltaAngle -= 360
                    while (deltaAngle < -180) deltaAngle += 360
                    
                    root.controlRotation = root.dragStartRotation + deltaAngle
                }
            }
            
            onRotationEnded: {
                root.dragging = false
                root.dragMode = ""
            }
            
            // Store start angle for calculation
            property real startAngle
            Component.onCompleted: {
                onRotationStarted.connect((startMousePos, angle) => {
                    startAngle = angle
                })
            }
        }
    }
    
    // Check if all selected elements are frames with flex enabled and size types set to fit content
    property bool showFlexResizeJoints: false
    property bool flexWidthFitContent: false
    property bool flexHeightFitContent: false
    
    function updateShowFlexResizeJoints() {
        if (!parent || !parent.selectedElements || parent.selectedElements.length === 0) {
            showFlexResizeJoints = false
            flexWidthFitContent = false
            flexHeightFitContent = false
            return
        }
        
        var allFlexWidthFitContent = true
        var allFlexHeightFitContent = true
        
        for (var i = 0; i < parent.selectedElements.length; i++) {
            var element = parent.selectedElements[i]
            if (!element || element.elementType !== "Frame" || !element.flex) {
                allFlexWidthFitContent = false
                allFlexHeightFitContent = false
                break
            }
            
            // Check width type (3 = SizeFitContent)
            if (element.widthType !== 3) {
                allFlexWidthFitContent = false
            }
            
            // Check height type (3 = SizeFitContent)
            if (element.heightType !== 3) {
                allFlexHeightFitContent = false
            }
        }
        
        flexWidthFitContent = allFlexWidthFitContent
        flexHeightFitContent = allFlexHeightFitContent
        showFlexResizeJoints = allFlexWidthFitContent || allFlexHeightFitContent
    }
    
    // Resize Joints (yellow, 10x10, on top)
    Repeater {
        model: {
            // If both width and height are fit content, hide all resize joints
            if (flexWidthFitContent && flexHeightFitContent) {
                return 0
            } else if (flexWidthFitContent || flexHeightFitContent) {
                return 2
            } else {
                return 4
            }
        }
        
        CornerResizeJoint {
            id: resizeJointContainer
            cornerIndex: index
            allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
            controlRotation: root.controlRotation
            showResizeJoints: root.showResizeJoints
            dragging: root.dragging
            dragMode: root.dragMode
            flexWidthFitContent: root.flexWidthFitContent
            flexHeightFitContent: root.flexHeightFitContent
            
            x: {
                if (flexWidthFitContent && !flexHeightFitContent) {
                    // Width fit content only - show top/bottom centered joints
                    return Math.round(parent.width / 2) - 5
                } else if (flexHeightFitContent && !flexWidthFitContent) {
                    // Height fit content only - show left/right centered joints
                    return index === 0 ? -5 : Math.round(parent.width) - 5
                } else {
                    // Normal corners
                    switch(index) {
                        case 0: return -5 // top-left
                        case 1: return Math.round(parent.width) - 5 // top-right
                        case 2: return Math.round(parent.width) - 5 // bottom-right
                        case 3: return -5 // bottom-left
                    }
                }
            }
            
            y: {
                if (flexWidthFitContent && !flexHeightFitContent) {
                    // Width fit content only - show top/bottom centered joints
                    return index === 0 ? -5 : Math.round(parent.height) - 5
                } else if (flexHeightFitContent && !flexWidthFitContent) {
                    // Height fit content only - show left/right centered joints
                    return Math.round(parent.height / 2) - 5
                } else {
                    // Normal corners
                    switch(index) {
                        case 0: return -5 // top-left
                        case 1: return -5 // top-right
                        case 2: return Math.round(parent.height) - 5 // bottom-right
                        case 3: return Math.round(parent.height) - 5 // bottom-left
                    }
                }
            }
            
            onResizeStarted: (cornerIdx, anchorCorner, adjacentCorner1, adjacentCorner2) => {
                root.dragging = true
                root.dragMode = "resize-corner-" + cornerIdx
                root.resizeStarted()
                
                // Store initial state for command creation via DesignControlsController
                if (root.designControlsController) {
                    root.designControlsController.startDragOperation()
                }
                
                // Initialize mouse position
                var mouseInParent = mapToItem(root.parent, 0, 0)
                root.updateMousePosition(mouseInParent)
                
                // Store corner points for the drag operation
                resizeJointContainer.anchorCorner = anchorCorner
                resizeJointContainer.adjacentCorner1 = adjacentCorner1
                resizeJointContainer.adjacentCorner2 = adjacentCorner2
            }
            
            onResizePositionChanged: (mouseInParent) => {
                if (!root.dragMode.startsWith("resize-corner-")) return
                
                root.updateMousePosition(mouseInParent)
                
                // For constraint-based approach:
                // 1. The anchor corner stays fixed
                // 2. The dragged corner should be at the mouse position
                // 3. The rectangle maintains its rotation
                
                // Calculate the original rotation from the existing edges
                var originalEdge1 = Qt.point(
                    resizeJointContainer.adjacentCorner1.x - resizeJointContainer.anchorCorner.x,
                    resizeJointContainer.adjacentCorner1.y - resizeJointContainer.anchorCorner.y
                )
                var originalEdge2 = Qt.point(
                    resizeJointContainer.adjacentCorner2.x - resizeJointContainer.anchorCorner.x,
                    resizeJointContainer.adjacentCorner2.y - resizeJointContainer.anchorCorner.y
                )
                
                // Calculate new edges from anchor to mouse
                var newDiagonal = Qt.point(
                    mouseInParent.x - resizeJointContainer.anchorCorner.x,
                    mouseInParent.y - resizeJointContainer.anchorCorner.y
                )
                
                // Project the new diagonal onto the original edge directions
                // to maintain the rectangle shape and rotation
                var edge1Length = Math.sqrt(originalEdge1.x * originalEdge1.x + originalEdge1.y * originalEdge1.y)
                var edge2Length = Math.sqrt(originalEdge2.x * originalEdge2.x + originalEdge2.y * originalEdge2.y)
                
                var edge1Unit = Qt.point(originalEdge1.x / edge1Length, originalEdge1.y / edge1Length)
                var edge2Unit = Qt.point(originalEdge2.x / edge2Length, originalEdge2.y / edge2Length)
                
                // Project diagonal onto both edge directions
                var proj1 = newDiagonal.x * edge1Unit.x + newDiagonal.y * edge1Unit.y
                var proj2 = newDiagonal.x * edge2Unit.x + newDiagonal.y * edge2Unit.y
                
                // Calculate new adjacent corners
                var newAdjacent1 = Qt.point(
                    resizeJointContainer.anchorCorner.x + proj1 * edge1Unit.x,
                    resizeJointContainer.anchorCorner.y + proj1 * edge1Unit.y
                )
                var newAdjacent2 = Qt.point(
                    resizeJointContainer.anchorCorner.x + proj2 * edge2Unit.x,
                    resizeJointContainer.anchorCorner.y + proj2 * edge2Unit.y
                )
                
                // Calculate the new dragged corner
                var newDraggedCorner = Qt.point(
                    newAdjacent1.x + proj2 * edge2Unit.x,
                    newAdjacent1.y + proj2 * edge2Unit.y
                )
                
                // Build the new corners array in order
                var newCorners = []
                switch(resizeJointContainer.cornerIndex) {
                    case 0: // dragged top-left
                        newCorners = [newDraggedCorner, newAdjacent1, resizeJointContainer.anchorCorner, newAdjacent2]
                        break
                    case 1: // dragged top-right
                        newCorners = [newAdjacent1, newDraggedCorner, newAdjacent2, resizeJointContainer.anchorCorner]
                        break
                    case 2: // dragged bottom-right
                        newCorners = [resizeJointContainer.anchorCorner, newAdjacent1, newDraggedCorner, newAdjacent2]
                        break
                    case 3: // dragged bottom-left
                        newCorners = [newAdjacent1, resizeJointContainer.anchorCorner, newAdjacent2, newDraggedCorner]
                        break
                }
                
                // Calculate center
                var centerX = (newCorners[0].x + newCorners[1].x + newCorners[2].x + newCorners[3].x) / 4
                var centerY = (newCorners[0].y + newCorners[1].y + newCorners[2].y + newCorners[3].y) / 4
                
                // Calculate width and height
                var widthVector = Qt.point(newCorners[1].x - newCorners[0].x, newCorners[1].y - newCorners[0].y)
                var heightVector = Qt.point(newCorners[3].x - newCorners[0].x, newCorners[3].y - newCorners[0].y)
                
                var newWidth = Math.sqrt(widthVector.x * widthVector.x + widthVector.y * widthVector.y)
                var newHeight = Math.sqrt(heightVector.x * heightVector.x + heightVector.y * heightVector.y)
                
                // Handle flipping by checking cross product
                var crossProduct = widthVector.x * heightVector.y - widthVector.y * heightVector.x
                if (crossProduct < 0) {
                    newWidth = -newWidth
                }
                
                // Adjust signs based on projections
                if (proj1 < 0) newWidth = -Math.abs(newWidth)
                else newWidth = Math.abs(newWidth)
                
                if (proj2 < 0) newHeight = -Math.abs(newHeight)
                else newHeight = Math.abs(newHeight)
                
                // Calculate the top-left corner from center, accounting for rotation
                // When rotated, we need to transform from the center point to find the actual top-left
                var halfWidth = Math.abs(newWidth) / 2
                var halfHeight = Math.abs(newHeight) / 2
                var angle = root.controlRotation * Math.PI / 180
                var cos = Math.cos(angle)
                var sin = Math.sin(angle)
                
                // Transform from center to top-left corner
                var viewportX = centerX - (halfWidth * cos - halfHeight * sin)
                var viewportY = centerY - (halfWidth * sin + halfHeight * cos)
                
                // Request throttled update instead of updating immediately
                cornerResizeThrottle.requestUpdate({
                    width: newWidth / root.parent.zoomLevel,
                    height: newHeight / root.parent.zoomLevel,
                    x: viewportX / root.parent.zoomLevel + root.parent.canvasMinX + root.parent.flickable.contentX / root.parent.zoomLevel,
                    y: viewportY / root.parent.zoomLevel + root.parent.canvasMinY + root.parent.flickable.contentY / root.parent.zoomLevel
                })
            }
            
            onResizeEnded: {
                // Force final update before ending
                cornerResizeThrottle.forceUpdate()
                
                // Fire ResizeElementCommand via DesignControlsController
                if (root.designControlsController) {
                    root.designControlsController.endResizeOperation()
                }
                
                root.dragging = false
                root.dragMode = ""
            }
            
            // Properties to store corner points during drag
            property point anchorCorner
            property point adjacentCorner1
            property point adjacentCorner2
        }
    }
    
    // BorderRadiusControl - circular control inside near top left resize joint
    BorderRadiusControl {
        id: borderRadiusControl
        allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
        parentWidth: root.width
        parentHeight: root.height
        selectedFrame: {
            if (!parent || !parent.parent || !parent.parent.selectedElements) return null
            if (parent.parent.selectedElements.length !== 1) return null
            var element = parent.parent.selectedElements[0]
            if (!element || element.elementType !== "Frame") return null
            return element
        }
        
        // Only visible when:
        // 1. A single element is selected
        // 2. The selected element is a frame
        // 3. Not dragging/resizing
        // 4. Frame has no parent OR parent does not have a platform
        visible: {
            if (!parent || !parent.parent || !parent.parent.selectedElements) return false
            if (parent.parent.selectedElements.length !== 1) return false
            var element = parent.parent.selectedElements[0]
            if (!element || element.elementType !== "Frame") return false
            if (!root.showResizeJoints) return false
            
            // Hide if frame has no parent (top-level) and has a platform
            if (!element.parentId || element.parentId === "") {
                if (element.platform && element.platform !== "") {
                    return false
                }
            }
            
            return true
        }
        
        Component.onCompleted: updateDragProgressFromFrame()
        
        Connections {
            target: parent ? parent.parent : null
            function onSelectedElementsChanged() {
                borderRadiusControl.updateDragProgressFromFrame()
            }
        }
        
        // Also watch for changes to the selected frame's properties
        Connections {
            target: borderRadiusControl.selectedFrame
            function onBorderRadiusChanged() {
                borderRadiusControl.updateDragProgressFromFrame()
            }
            function onWidthChanged() {
                borderRadiusControl.updateDragProgressFromFrame()
            }
            function onHeightChanged() {
                borderRadiusControl.updateDragProgressFromFrame()
            }
        }
        
        onBorderRadiusChanged: (newRadius) => {
            // Update the frame's border radius
            if (selectedFrame) {
                selectedFrame.borderRadius = newRadius
            }
        }
    }
    
    // Square control outside the right control bar
    PrototypePlayButton {
        id: inlinePrototypePlay
        allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
        parentWidth: root.width
        parentHeight: root.height
        selectedFrame: {
            if (!parent || !parent.parent || !parent.parent.selectedElements) return null
            if (parent.parent.selectedElements.length !== 1) return null
            var element = parent.parent.selectedElements[0]
            if (!element || element.elementType !== "Frame") return null
            return element
        }
        viewportOverlay: root.parent
        
        // Only visible when:
        // 1. A single element is selected
        // 2. The selected element is a frame
        // 3. Not dragging
        // 4. Not currently prototyping
        // 5. Frame has a platform defined (not empty/undefined)
        // 6. None of the selected elements' ancestors have a platform defined
        visible: {
            if (!parent || !parent.parent || !parent.parent.selectedElements) return false
            if (parent.parent.selectedElements.length !== 1) return false
            var element = parent.parent.selectedElements[0]
            if (!element || element.elementType !== "Frame") return false
            if (root.dragging) return false
            // Hide when prototyping is active
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            if (prototypeController && prototypeController.isPrototyping) {
                return false
            }
            // Hide if platform is undefined/empty
            if (!element.platform || element.platform === "") return false
            
            // Check if any ancestor has a platform defined
            if (viewportOverlay && viewportOverlay.canvasView && viewportOverlay.canvasView.elementModel) {
                var currentId = element.parentId
                while (currentId && currentId !== "") {
                    var parentElement = viewportOverlay.canvasView.elementModel.getElementById(currentId)
                    if (parentElement && parentElement.elementType === "Frame" && parentElement.platform && parentElement.platform !== "") {
                        // An ancestor has a platform defined, hide the button
                        return false
                    }
                    currentId = parentElement ? parentElement.parentId : ""
                }
            }
            
            return true
        }
        
        onPrototypePlayClicked: (selectedFrame) => {
            // Access the DesignCanvas through the viewport overlay
            var viewportOverlay = root.parent
            if (!viewportOverlay || !viewportOverlay.canvasView) {
                if (root.canvas && root.canvas.console) root.canvas.console.addError("Cannot access DesignCanvas from controls")
                return
            }
            
            var designCanvas = viewportOverlay.canvasView
            var prototypeController = root.canvas ? root.canvas.prototypeController : null
            
            if (!prototypeController) {
                if (root.canvas && root.canvas.console) root.canvas.console.addError("PrototypeController not available")
                if (root.canvas && root.canvas.console) root.canvas.console.addOutput("viewportOverlay.prototypeController is: " + viewportOverlay.prototypeController)
                if (root.canvas && root.canvas.console) root.canvas.console.addOutput("root.canvas is: " + root.canvas)
                if (root.canvas) {
                    if (root.canvas && root.canvas.console) root.canvas.console.addOutput("root.canvas.prototypeController is: " + root.canvas.prototypeController)
                }
                return
            }
            
            // Check if we have a selected element (should be a frame)
            if (!viewportOverlay.selectedElements || viewportOverlay.selectedElements.length !== 1) {
                if (root.canvas && root.canvas.console) root.canvas.console.addError("No single frame selected for prototyping")
                return
            }
            
            var selectedFrame = viewportOverlay.selectedElements[0]
            if (selectedFrame.elementType !== "Frame") {
                if (root.canvas && root.canvas.console) root.canvas.console.addError("Selected element is not a frame")
                return
            }
            
            // Set prototype mode based on frame's platform
            if (selectedFrame.platform) {
                prototypeController.prototypeMode = selectedFrame.platform
            }
            
            // Starting prototyping mode
            
            // 1) Store the current canvas position in the prototype controller
            var currentCanvasCenter = Qt.point(
                viewportOverlay.flickable.contentX / designCanvas.zoom + designCanvas.canvasMinX + viewportOverlay.flickable.width / 2 / designCanvas.zoom,
                viewportOverlay.flickable.contentY / designCanvas.zoom + designCanvas.canvasMinY + viewportOverlay.flickable.height / 2 / designCanvas.zoom
            )
            
            // 2) Start prototyping through the controller
            prototypeController.startPrototyping(currentCanvasCenter, designCanvas.zoom)
            
            // The PrototypeController will handle canvas positioning when it sets the activeOuterFrame
        }
    }
    
    // HoverBadge component definition
    Component {
        id: hoverBadgeComponent
        
        HoverBadge {
            id: hoverBadgeRoot
            allSelectedAreComponentRelated: root.allSelectedAreComponentRelated
        }
    }
    
    // Export the HoverBadge component for external use
    property alias hoverBadge: hoverBadgeComponent
    
    // Component Variant Add Button
    VariantAddButton {
        id: componentVariantAddButton
        visible: selectedIsComponentVariant
        controller: root.parent ? root.parent.controller : null
        selectedVariant: {
            if (root.parent && root.parent.selectedElements && root.parent.selectedElements.length === 1) {
                var variant = root.parent.selectedElements[0]
                if (variant && variant.isDesignElement && variant.isComponentVariant()) {
                    return variant
                }
            }
            return null
        }
        
        onAddVariantClicked: (selectedVariant) => {
            // Get the controller from the parent ViewportOverlay
            if (root.parent && root.parent.controller && selectedVariant) {
                root.parent.controller.duplicateVariant(selectedVariant.elementId)
            }
        }
    }
    
    // Function to check if all selected elements are component-related
    function updateComponentRelatedStatus() {
        if (!parent || !parent.selectedElements || parent.selectedElements.length === 0) {
            allSelectedAreComponentRelated = false
            selectedIsComponentVariant = false
            return
        }
        
        // Check if exactly one element is selected and it's a ComponentVariant
        if (parent.selectedElements.length === 1) {
            var element = parent.selectedElements[0]
            selectedIsComponentVariant = !!(element && element.isDesignElement && 
                                           typeof element.isComponentVariant === 'function' && 
                                           element.isComponentVariant())
        } else {
            selectedIsComponentVariant = false
        }
        
        var allComponent = true
        for (var i = 0; i < parent.selectedElements.length; i++) {
            var element = parent.selectedElements[i]
            if (!element) {
                allComponent = false
                break
            }
            
            // Check if element is a ComponentInstance or ComponentVariant using the generic methods
            var isComponentRelated = false
            if (element.isDesignElement) {
                isComponentRelated = (typeof element.isInstance === 'function' && element.isInstance() === true) || 
                                   (typeof element.isComponentVariant === 'function' && element.isComponentVariant() === true)
            }
            
            if (!isComponentRelated) {
                // Check if it's a descendant of a ComponentInstance or ComponentVariant
                var isDescendant = false
                if (parent.canvasView && parent.canvasView.elementModel) {
                    var currentId = element.parentId
                    while (currentId && currentId !== "") {
                        var parentElement = parent.canvasView.elementModel.getElementById(currentId)
                        if (parentElement && parentElement.isDesignElement && 
                            (parentElement.isInstance() || parentElement.isComponentVariant())) {
                            isDescendant = true
                            break
                        }
                        currentId = parentElement ? parentElement.parentId : ""
                    }
                }
                
                if (!isDescendant) {
                    allComponent = false
                    break
                }
            }
        }
        
        allSelectedAreComponentRelated = allComponent
    }
    
    // Monitor selection changes
    Connections {
        target: parent
        function onSelectedElementsChanged() {
            updateComponentRelatedStatus()
            updateShowFlexResizeJoints()
            setupElementConnections()
        }
    }
    
    // Keep track of connections to selected elements
    property var elementConnections: []
    
    function setupElementConnections() {
        // Clear existing connections
        for (var i = 0; i < elementConnections.length; i++) {
            elementConnections[i].destroy()
        }
        elementConnections = []
        
        // Create new connections for each selected element
        if (parent && parent.selectedElements) {
            for (var j = 0; j < parent.selectedElements.length; j++) {
                var element = parent.selectedElements[j]
                if (element && element.elementType === "Frame") {
                    var conn = elementConnectionComponent.createObject(root, {target: element})
                    elementConnections.push(conn)
                }
            }
        }
    }
    
    // Component for creating connections to frame elements
    Component {
        id: elementConnectionComponent
        Connections {
            function onFlexChanged() {
                root.updateShowFlexResizeJoints()
            }
            function onWidthTypeChanged() {
                root.updateShowFlexResizeJoints()
            }
            function onHeightTypeChanged() {
                root.updateShowFlexResizeJoints()
            }
        }
    }
    
    Component.onCompleted: {
        updateComponentRelatedStatus()
        updateShowFlexResizeJoints()
        setupElementConnections()
    }
    
    
    // Function to check if we should reorder elements while dragging
    function checkForReordering(mouseInParent) {
        if (!parent || !parent.selectedElements || parent.selectedElements.length !== 1) {
            return // Only handle single selection for now
        }
        
        var draggedElement = parent.selectedElements[0]
        if (!draggedElement || draggedElement.elementType !== "Frame" || draggedElement.position !== 0) {
            return // Only reorder position relative frames
        }
        
        // Check if parent has flex enabled
        if (!parent.canvasView || !parent.canvasView.elementModel) {
            return
        }
        
        var parentElement = parent.canvasView.elementModel.getElementById(draggedElement.parentId)
        if (!parentElement || parentElement.elementType !== "Frame" || !parentElement.flex) {
            return // Parent must be a flex container
        }
        
        // Convert mouse position to canvas coordinates
        var canvasX = (mouseInParent.x + (parent.flickable?.contentX ?? 0)) / parent.zoomLevel + parent.canvasMinX
        var canvasY = (mouseInParent.y + (parent.flickable?.contentY ?? 0)) / parent.zoomLevel + parent.canvasMinY
        
        // Get all siblings (children of the same parent)
        var siblings = []
        var allElements = parent.canvasView.elementModel.getAllElements()
        for (var i = 0; i < allElements.length; i++) {
            var element = allElements[i]
            if (element.parentId === draggedElement.parentId && 
                element.elementId !== draggedElement.elementId &&
                element.elementType === "Frame" &&
                element.position === 0) { // Only position relative siblings
                siblings.push(element)
            }
        }
        
        if (siblings.length === 0) {
            return // No siblings to reorder with
        }
        
        // Find which sibling we're hovering over
        var hoveredSibling = null
        var hoveredIndex = -1
        var draggedIndex = allElements.indexOf(draggedElement)
        
        for (var j = 0; j < siblings.length; j++) {
            var sibling = siblings[j]
            if (canvasX >= sibling.x && canvasX <= sibling.x + sibling.width &&
                canvasY >= sibling.y && canvasY <= sibling.y + sibling.height) {
                hoveredSibling = sibling
                hoveredIndex = allElements.indexOf(sibling)
                break
            }
        }
        
        if (!hoveredSibling || hoveredIndex === -1) {
            return // Not hovering over a sibling
        }
        
        // Determine the new index based on the layout direction and mouse position
        var newIndex = hoveredIndex
        
        if (parentElement.orientation === 0) { // Row layout
            // Check if mouse is in the right half of the hovered sibling
            if (canvasX > hoveredSibling.x + hoveredSibling.width / 2) {
                // Insert after the hovered sibling
                newIndex = hoveredIndex > draggedIndex ? hoveredIndex : hoveredIndex + 1
            } else {
                // Insert before the hovered sibling
                newIndex = hoveredIndex < draggedIndex ? hoveredIndex : hoveredIndex - 1
            }
        } else { // Column layout
            // Check if mouse is in the bottom half of the hovered sibling
            if (canvasY > hoveredSibling.y + hoveredSibling.height / 2) {
                // Insert after the hovered sibling
                newIndex = hoveredIndex > draggedIndex ? hoveredIndex : hoveredIndex + 1
            } else {
                // Insert before the hovered sibling
                newIndex = hoveredIndex < draggedIndex ? hoveredIndex : hoveredIndex - 1
            }
        }
        
        // Perform the reorder if the index changed
        if (newIndex !== draggedIndex && newIndex >= 0 && newIndex < allElements.length) {
            parent.canvasView.elementModel.reorderElement(draggedElement, newIndex)
            
            // Trigger layout update on the parent frame
            // The Frame's own layout engine will handle the update
            parentElement.triggerLayout()
        }
    }
}