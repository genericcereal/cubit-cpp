import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

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
    
    // Check if all selected elements are component-related
    property bool allSelectedAreComponentRelated: false
    
    // Check if resizing is disabled
    property bool isResizingDisabled: {
        var viewportOverlay = root.parent
        if (viewportOverlay && viewportOverlay.canvasView && viewportOverlay.canvasView.controller) {
            return viewportOverlay.canvasView.controller.isDesignControlsResizingDisabled || false
        }
        return false
    }
    
    // Check if movement is disabled
    property bool isMovementDisabled: {
        var viewportOverlay = root.parent
        if (viewportOverlay && viewportOverlay.canvasView && viewportOverlay.canvasView.controller) {
            return viewportOverlay.canvasView.controller.isDesignControlsMovementDisabled || false
        }
        return false
    }
    
    // Control state
    property bool dragging: false
    property string dragMode: "" // "move", "resize-edge-0", "resize-edge-1", etc.
    property bool showResizeJoints: {
        // Hide resize joints when dragging or when in prototyping mode
        if (dragging) {
            ConsoleMessageRepository.addOutput("showResizeJoints: false (dragging)")
            return false
        }
        
        // Check if we're in prototyping mode
        var viewportOverlay = root.parent
        if (viewportOverlay && viewportOverlay.prototypeController && viewportOverlay.prototypeController.isPrototyping) {
            ConsoleMessageRepository.addOutput("showResizeJoints: false (prototyping)")
            return false
        }
        
        // Check if animations are currently playing
        if (viewportOverlay && viewportOverlay.canvasView) {
            var controller = viewportOverlay.canvasView.controller
            if (controller && controller.isAnimating) {
                ConsoleMessageRepository.addOutput("showResizeJoints: false (animating)")
                return false
            }
        }
        
        ConsoleMessageRepository.addOutput("showResizeJoints: true")
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
        // Adjust edge index based on rotation (90 degree increments)
        var rotationSteps = Math.round(controlRotation / 90) % 4
        if (rotationSteps < 0) rotationSteps += 4
        var adjustedIndex = (edgeIndex + rotationSteps) % 4
        
        // Return cursor based on adjusted edge
        return (adjustedIndex % 2 === 0) ? Qt.SizeVerCursor : Qt.SizeHorCursor
    }
    
    // Helper function to get rotation-adjusted corner cursor
    function getCornerCursor(cornerIndex) {
        // Adjust corner index based on rotation
        var rotationSteps = Math.round(controlRotation / 90) % 4
        if (rotationSteps < 0) rotationSteps += 4
        var adjustedIndex = (cornerIndex + rotationSteps) % 4
        
        // Diagonal cursors alternate based on adjusted corner
        return (adjustedIndex === 0 || adjustedIndex === 2) ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor
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
    
    // Control Surface - yellow transparent fill
    Rectangle {
        id: controlSurface
        anchors.fill: parent
        color: Config.controlInnerRectColor
        antialiasing: true
        
        MouseArea {
            id: surfaceMouseArea
            anchors.fill: parent
            cursorShape: dragging ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            propagateComposedEvents: true
            
            property bool isDragThresholdExceeded: false
            property real dragThreshold: 3 // pixels
            
            onPressed: (mouse) => {
                // Store mouse position in parent coordinates to handle rotation correctly
                var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                root.updateMousePosition(mouseInParent)
                root.dragStartPoint = mouseInParent
                root.dragStartControlPos = Qt.point(root.controlX, root.controlY)
                isDragThresholdExceeded = false
                mouse.accepted = true
            }
            
            onReleased: (mouse) => {
                if (!isDragThresholdExceeded && root.parent && root.parent.selectedElements) {
                    // This was a click, not a drag
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    
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
                
                root.dragging = false
                root.dragMode = ""
                root.updateMousePosition(Qt.point(0, 0))
            }
            
            onDoubleClicked: (mouse) => {
                ConsoleMessageRepository.addOutput("Controls double-clicked")
                
                // Access ViewportOverlay through root.parent
                var viewportOverlay = root.parent
                
                // Check if we have a single element selected
                if (viewportOverlay && viewportOverlay.selectedElements && viewportOverlay.selectedElements.length === 1) {
                    var element = viewportOverlay.selectedElements[0]
                    ConsoleMessageRepository.addOutput("Selected element type: " + element.elementType + " (ID: " + element.elementId + ")")
                    
                    // Check if it's a Text element directly selected
                    if (element && element.elementType === "Text") {
                        ConsoleMessageRepository.addOutput("Text element directly selected - triggering edit mode")
                        element.isEditing = true
                        mouse.accepted = true
                        return
                    } else if (element && element.elementType === "Frame") {
                        ConsoleMessageRepository.addOutput("Frame selected, checking for Text children...")
                        
                        // Check if this frame has any text children
                        if (viewportOverlay.canvasView && viewportOverlay.canvasView.elementModel) {
                            var allElements = viewportOverlay.canvasView.elementModel.getAllElements()
                            var textChildrenFound = 0
                            
                            for (var i = 0; i < allElements.length; i++) {
                                var child = allElements[i]
                                if (child.parentId === element.elementId && child.elementType === "Text") {
                                    textChildrenFound++
                                    ConsoleMessageRepository.addOutput("Found Text child: " + child.name + " (ID: " + child.elementId + ")")
                                    // Trigger edit mode for the first text child found
                                    child.isEditing = true
                                    mouse.accepted = true
                                    return
                                }
                            }
                            
                            if (textChildrenFound === 0) {
                                ConsoleMessageRepository.addOutput("No Text children found in this Frame")
                            }
                        } else {
                            ConsoleMessageRepository.addOutput("Unable to access elementModel")
                        }
                    } else {
                        ConsoleMessageRepository.addOutput("Selected element is neither Frame nor Text")
                    }
                } else if (viewportOverlay && viewportOverlay.selectedElements) {
                    ConsoleMessageRepository.addOutput("Multiple elements selected: " + viewportOverlay.selectedElements.length)
                } else {
                    ConsoleMessageRepository.addOutput("No elements selected or unable to access selection")
                }
                
                mouse.accepted = true
            }
            
            onPositionChanged: (mouse) => {
                var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                
                // Check if drag threshold exceeded
                if (!isDragThresholdExceeded) {
                    var distance = Math.sqrt(
                        Math.pow(mouseInParent.x - root.dragStartPoint.x, 2) + 
                        Math.pow(mouseInParent.y - root.dragStartPoint.y, 2)
                    )
                    
                    if (distance > dragThreshold) {
                        isDragThresholdExceeded = true
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
                    
                    // Update control position in canvas coordinates
                    root.controlX = root.dragStartControlPos.x + deltaX / root.parent.zoomLevel
                    root.controlY = root.dragStartControlPos.y + deltaY / root.parent.zoomLevel
                    
                    // Check for reordering if dragging a position relative child in a flex parent
                    checkForReordering(mouseInParent)
                    
                    // Emit signal for hover detection
                    root.mouseDragged(mouseInParent)
                }
            }
        }
    }
    
    // Edge Resize Bars - using repeater for rotation-aware behavior
    Repeater {
        model: 4 // 0=top, 1=right, 2=bottom, 3=left (before rotation)
        
        Rectangle {
            id: edgeBar
            color: allSelectedAreComponentRelated ? Config.componentControlBarColor : Config.controlBarColor
            antialiasing: true
            
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
            
            
            MouseArea {
                anchors.fill: parent
                cursorShape: {
                    // Check if resize is allowed for this edge
                    var resizeAllowed = true
                    if (index === 0 || index === 2) {
                        // Horizontal bars (top/bottom) - check height resize
                        resizeAllowed = !flexHeightFitContent
                    } else {
                        // Vertical bars (left/right) - check width resize
                        resizeAllowed = !flexWidthFitContent
                    }
                    return resizeAllowed ? root.getEdgeCursor(index) : Qt.ArrowCursor
                }
                
                property int edgeIndex: index
                property point anchorEdgePoint1  // First point of anchor edge in parent coords
                property point anchorEdgePoint2  // Second point of anchor edge in parent coords
                property point draggedEdgePoint1 // First point of dragged edge in parent coords
                property point draggedEdgePoint2 // Second point of dragged edge in parent coords
                
                onPressed: (mouse) => {
                    // Check if resizing is globally disabled
                    if (root.isResizingDisabled) {
                        mouse.accepted = false
                        return
                    }
                    
                    // Check if resize is allowed for this edge
                    var resizeAllowed = true
                    if (edgeIndex === 0 || edgeIndex === 2) {
                        // Horizontal bars (top/bottom) - check height resize
                        resizeAllowed = !flexHeightFitContent
                    } else {
                        // Vertical bars (left/right) - check width resize
                        resizeAllowed = !flexWidthFitContent
                    }
                    
                    if (!resizeAllowed) {
                        mouse.accepted = false
                        return
                    }
                    
                    root.dragging = true
                    root.dragMode = "resize-edge-" + edgeIndex
                    root.resizeStarted()
                    
                    // Initialize mouse position
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.updateMousePosition(mouseInParent)
                    
                    // Get the corners of the control in local space
                    var corners = [
                        Qt.point(0, 0),           // top-left
                        Qt.point(root.width, 0),  // top-right
                        Qt.point(root.width, root.height), // bottom-right
                        Qt.point(0, root.height)  // bottom-left
                    ]
                    
                    // Map corners to parent coordinates
                    var parentCorners = []
                    for (var i = 0; i < 4; i++) {
                        parentCorners.push(root.mapToItem(root.parent, corners[i].x, corners[i].y))
                    }
                    
                    // Store both the anchor edge and dragged edge positions
                    switch(edgeIndex) {
                        case 0: // dragging top edge
                            anchorEdgePoint1 = parentCorners[3] // bottom-left
                            anchorEdgePoint2 = parentCorners[2] // bottom-right
                            draggedEdgePoint1 = parentCorners[0] // top-left
                            draggedEdgePoint2 = parentCorners[1] // top-right
                            break
                        case 1: // dragging right edge
                            anchorEdgePoint1 = parentCorners[0] // top-left
                            anchorEdgePoint2 = parentCorners[3] // bottom-left
                            draggedEdgePoint1 = parentCorners[1] // top-right
                            draggedEdgePoint2 = parentCorners[2] // bottom-right
                            break
                        case 2: // dragging bottom edge
                            anchorEdgePoint1 = parentCorners[0] // top-left
                            anchorEdgePoint2 = parentCorners[1] // top-right
                            draggedEdgePoint1 = parentCorners[3] // bottom-left
                            draggedEdgePoint2 = parentCorners[2] // bottom-right
                            break
                        case 3: // dragging left edge
                            anchorEdgePoint1 = parentCorners[1] // top-right
                            anchorEdgePoint2 = parentCorners[2] // bottom-right
                            draggedEdgePoint1 = parentCorners[0] // top-left
                            draggedEdgePoint2 = parentCorners[3] // bottom-left
                            break
                    }
                    
                    mouse.accepted = true
                }
                
                onReleased: {
                    root.dragging = false
                    root.dragMode = ""
                    root.updateMousePosition(Qt.point(0, 0))
                }
                
                onPositionChanged: (mouse) => {
                    if (!root.dragMode.startsWith("resize-edge-")) return
                    
                    // Get mouse position in parent coordinates
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.updateMousePosition(mouseInParent)
                    
                    // For constraint-based approach:
                    // 1. The anchor edge stays fixed at anchorEdgePoint1 and anchorEdgePoint2
                    // 2. The dragged edge should be at the mouse position
                    
                    // Calculate the vector along the anchor edge
                    var anchorVector = Qt.point(
                        anchorEdgePoint2.x - anchorEdgePoint1.x,
                        anchorEdgePoint2.y - anchorEdgePoint1.y
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
                        (anchorEdgePoint1.x + anchorEdgePoint2.x) / 2,
                        (anchorEdgePoint1.y + anchorEdgePoint2.y) / 2
                    )
                    var draggedCenter = Qt.point(
                        (draggedEdgePoint1.x + draggedEdgePoint2.x) / 2,
                        (draggedEdgePoint1.y + draggedEdgePoint2.y) / 2
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
                        mouseInParent.x - anchorEdgePoint1.x,
                        mouseInParent.y - anchorEdgePoint1.y
                    )
                    var distance = toMouse.x * perpVector.x + toMouse.y * perpVector.y
                    
                    // Calculate the dragged edge points
                    var draggedPoint1 = Qt.point(
                        anchorEdgePoint1.x + distance * perpVector.x,
                        anchorEdgePoint1.y + distance * perpVector.y
                    )
                    var draggedPoint2 = Qt.point(
                        anchorEdgePoint2.x + distance * perpVector.x,
                        anchorEdgePoint2.y + distance * perpVector.y
                    )
                    
                    // Now we have the four corners of the new rectangle in parent coordinates
                    var newCorners = []
                    switch(edgeIndex) {
                        case 0: // dragged top edge
                            newCorners = [draggedPoint1, draggedPoint2, anchorEdgePoint2, anchorEdgePoint1]
                            break
                        case 1: // dragged right edge
                            newCorners = [anchorEdgePoint1, draggedPoint1, draggedPoint2, anchorEdgePoint2]
                            break
                        case 2: // dragged bottom edge
                            newCorners = [anchorEdgePoint1, anchorEdgePoint2, draggedPoint2, draggedPoint1]
                            break
                        case 3: // dragged left edge
                            newCorners = [draggedPoint1, anchorEdgePoint1, anchorEdgePoint2, draggedPoint2]
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
                    switch(edgeIndex) {
                        case 0: // top edge
                        case 2: // bottom edge
                            if (distance < 0) newHeight = -newHeight
                            break
                        case 1: // right edge
                        case 3: // left edge
                            if (distance < 0) newWidth = -newWidth
                            break
                    }
                    
                    // Convert from viewport to canvas coordinates
                    var viewportX = centerX - Math.abs(newWidth) / 2
                    var viewportY = centerY - Math.abs(newHeight) / 2
                    
                    // Update control properties in canvas coordinates
                    root.controlWidth = newWidth / root.parent.zoomLevel
                    root.controlHeight = newHeight / root.parent.zoomLevel
                    root.controlX = viewportX / root.parent.zoomLevel + root.parent.canvasMinX + root.parent.flickable.contentX / root.parent.zoomLevel
                    root.controlY = viewportY / root.parent.zoomLevel + root.parent.canvasMinY + root.parent.flickable.contentY / root.parent.zoomLevel
                }
            }
        }
    }
    
    // Rotation Joints (red, 20x20, behind resize joints)
    Repeater {
        model: 4
        
        Rectangle {
            id: rotationJoint
            width: 20
            height: 20
            color: allSelectedAreComponentRelated ? Config.componentControlRotationJointColor : Config.controlRotationJointColor
            antialiasing: true
            z: 1
            visible: true
            
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
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
                
                property point startMousePos
                property real startAngle
                
                onPressed: (mouse) => {
                    // Check if resizing is globally disabled
                    if (root.isResizingDisabled) {
                        mouse.accepted = false
                        return
                    }
                    
                    root.dragging = true
                    root.dragMode = "rotate"
                    // Calculate center of the control in parent coordinates
                    var centerPoint = Qt.point(
                        root.x + root.width / 2,
                        root.y + root.height / 2
                    )
                    startMousePos = mapToItem(root.parent, mouse.x, mouse.y)
                    root.updateMousePosition(startMousePos)
                    startAngle = Math.atan2(startMousePos.y - centerPoint.y, startMousePos.x - centerPoint.x) * 180 / Math.PI
                    root.dragStartRotation = root.controlRotation
                    mouse.accepted = true
                }
                
                onReleased: {
                    root.dragging = false
                    root.dragMode = ""
                }
                
                onPositionChanged: (mouse) => {
                    if (root.dragMode === "rotate") {
                        // Calculate center of the control in parent coordinates
                        var centerPoint = Qt.point(
                            root.x + root.width / 2,
                            root.y + root.height / 2
                        )
                        var currentMousePos = mapToItem(root.parent, mouse.x, mouse.y)
                        root.updateMousePosition(currentMousePos)
                        var currentAngle = Math.atan2(currentMousePos.y - centerPoint.y, currentMousePos.x - centerPoint.x) * 180 / Math.PI
                        var deltaAngle = currentAngle - startAngle
                        
                        // Normalize the angle difference to handle wrap-around
                        while (deltaAngle > 180) deltaAngle -= 360
                        while (deltaAngle < -180) deltaAngle += 360
                        
                        root.controlRotation = root.dragStartRotation + deltaAngle
                    }
                }
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
        
        Item {
            id: resizeJointContainer
            width: 10
            height: 10
            z: 2
            
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
            
            property int cornerIndex: index
            
            // Visual representation of the resize joint
            Rectangle {
                id: resizeJoint
                anchors.fill: parent
                color: allSelectedAreComponentRelated ? Config.componentControlResizeJointColor : Config.controlResizeJointColor
                antialiasing: true
                visible: root.showResizeJoints && !(root.dragging && root.dragMode.startsWith("resize-corner-"))
                
                // Decorative circle inside the resize joint
                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    radius: 5
                    color: Config.controlJointCircleFill
                    border.color: allSelectedAreComponentRelated ? Config.componentControlJointCircleBorder : Config.controlJointCircleBorder
                    border.width: 1
                    antialiasing: true
                }
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: {
                    if (flexWidthFitContent && !flexHeightFitContent) {
                        // Width fit content - vertical resize only
                        return Qt.SizeVerCursor
                    } else if (flexHeightFitContent && !flexWidthFitContent) {
                        // Height fit content - horizontal resize only
                        return Qt.SizeHorCursor
                    } else {
                        // Normal corner resize
                        return root.getCornerCursor(parent.cornerIndex)
                    }
                }
                
                // When in fit content mode, don't handle events - let them pass through to the bar
                propagateComposedEvents: flexWidthFitContent || flexHeightFitContent
                
                property bool shouldPropagateEvents: flexWidthFitContent || flexHeightFitContent
                
                property int cornerIdx: resizeJointContainer.cornerIndex
                property point anchorCorner  // Opposite corner in parent coords
                property point adjacentCorner1  // Adjacent corner 1 in parent coords
                property point adjacentCorner2  // Adjacent corner 2 in parent coords
                
                onPressed: (mouse) => {
                    // Check if resizing is globally disabled
                    if (root.isResizingDisabled) {
                        mouse.accepted = false
                        return
                    }
                    
                    // In fit content mode, don't handle the event
                    if (shouldPropagateEvents) {
                        mouse.accepted = false
                        return
                    }
                    
                    root.dragging = true
                    root.dragMode = "resize-corner-" + cornerIdx
                    root.resizeStarted()
                    
                    // Initialize mouse position
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.updateMousePosition(mouseInParent)
                    
                    // Get the corners of the control in local space
                    var corners = [
                        Qt.point(0, 0),           // top-left
                        Qt.point(root.width, 0),  // top-right
                        Qt.point(root.width, root.height), // bottom-right
                        Qt.point(0, root.height)  // bottom-left
                    ]
                    
                    // Map corners to parent coordinates
                    var parentCorners = []
                    for (var i = 0; i < 4; i++) {
                        parentCorners.push(root.mapToItem(root.parent, corners[i].x, corners[i].y))
                    }
                    
                    // If we're in bar mode, we don't need corner logic
                    if (!root.dragMode.startsWith("resize-corner-")) {
                        return
                    }
                    
                    // Store the anchor corner (opposite) and adjacent corners
                    switch(cornerIdx) {
                        case 0: // dragging top-left
                            anchorCorner = parentCorners[2]     // bottom-right is anchor
                            adjacentCorner1 = parentCorners[1]  // top-right
                            adjacentCorner2 = parentCorners[3]  // bottom-left
                            break
                        case 1: // dragging top-right
                            anchorCorner = parentCorners[3]     // bottom-left is anchor
                            adjacentCorner1 = parentCorners[0]  // top-left
                            adjacentCorner2 = parentCorners[2]  // bottom-right
                            break
                        case 2: // dragging bottom-right
                            anchorCorner = parentCorners[0]     // top-left is anchor
                            adjacentCorner1 = parentCorners[1]  // top-right
                            adjacentCorner2 = parentCorners[3]  // bottom-left
                            break
                        case 3: // dragging bottom-left
                            anchorCorner = parentCorners[1]     // top-right is anchor
                            adjacentCorner1 = parentCorners[0]  // top-left
                            adjacentCorner2 = parentCorners[2]  // bottom-right
                            break
                    }
                    
                    mouse.accepted = true
                }
                
                onReleased: (mouse) => {
                    if (shouldPropagateEvents) {
                        mouse.accepted = false
                        return
                    }
                    
                    root.dragging = false
                    root.dragMode = ""
                }
                
                onPositionChanged: (mouse) => {
                    if (shouldPropagateEvents) {
                        mouse.accepted = false
                        return
                    }
                    
                    if (!root.dragMode.startsWith("resize-corner-")) return
                    
                    // Get mouse position in parent coordinates
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.updateMousePosition(mouseInParent)
                    
                    // For constraint-based approach:
                    // 1. The anchor corner stays fixed
                    // 2. The dragged corner should be at the mouse position
                    // 3. The rectangle maintains its rotation
                    
                    // Calculate the original rotation from the existing edges
                    var originalEdge1 = Qt.point(
                        adjacentCorner1.x - anchorCorner.x,
                        adjacentCorner1.y - anchorCorner.y
                    )
                    var originalEdge2 = Qt.point(
                        adjacentCorner2.x - anchorCorner.x,
                        adjacentCorner2.y - anchorCorner.y
                    )
                    
                    // Calculate new edges from anchor to mouse
                    var newDiagonal = Qt.point(
                        mouseInParent.x - anchorCorner.x,
                        mouseInParent.y - anchorCorner.y
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
                        anchorCorner.x + proj1 * edge1Unit.x,
                        anchorCorner.y + proj1 * edge1Unit.y
                    )
                    var newAdjacent2 = Qt.point(
                        anchorCorner.x + proj2 * edge2Unit.x,
                        anchorCorner.y + proj2 * edge2Unit.y
                    )
                    
                    // Calculate the new dragged corner
                    var newDraggedCorner = Qt.point(
                        newAdjacent1.x + proj2 * edge2Unit.x,
                        newAdjacent1.y + proj2 * edge2Unit.y
                    )
                    
                    // Build the new corners array in order
                    var newCorners = []
                    switch(cornerIdx) {
                        case 0: // dragged top-left
                            newCorners = [newDraggedCorner, newAdjacent1, anchorCorner, newAdjacent2]
                            break
                        case 1: // dragged top-right
                            newCorners = [newAdjacent1, newDraggedCorner, newAdjacent2, anchorCorner]
                            break
                        case 2: // dragged bottom-right
                            newCorners = [anchorCorner, newAdjacent1, newDraggedCorner, newAdjacent2]
                            break
                        case 3: // dragged bottom-left
                            newCorners = [newAdjacent1, anchorCorner, newAdjacent2, newDraggedCorner]
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
                    
                    // Convert from viewport to canvas coordinates
                    var viewportX = centerX - Math.abs(newWidth) / 2
                    var viewportY = centerY - Math.abs(newHeight) / 2
                    
                    // Update control properties in canvas coordinates
                    root.controlWidth = newWidth / root.parent.zoomLevel
                    root.controlHeight = newHeight / root.parent.zoomLevel
                    root.controlX = viewportX / root.parent.zoomLevel + root.parent.canvasMinX + root.parent.flickable.contentX / root.parent.zoomLevel
                    root.controlY = viewportY / root.parent.zoomLevel + root.parent.canvasMinY + root.parent.flickable.contentY / root.parent.zoomLevel
                }
            }
        }
    }
    
    // BorderRadiusControl - circular control inside near top left resize joint
    Rectangle {
        id: borderRadiusControl
        width: 10
        height: 10
        radius: 5
        color: allSelectedAreComponentRelated ? Config.componentControlResizeJointColor : Config.controlResizeJointColor
        antialiasing: true
        z: 2
        
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
        
        // Drag progress along diagonal (0 = top-left corner, 1 = center)
        property real dragProgress: 0
        
        // Initialize drag progress based on frame's current border radius
        Component.onCompleted: updateDragProgressFromFrame()
        
        Connections {
            target: parent ? parent.parent : null
            function onSelectedElementsChanged() {
                updateDragProgressFromFrame()
            }
        }
        
        // Also watch for changes to the selected frame's properties
        Connections {
            target: {
                if (!parent || !parent.parent || !parent.parent.selectedElements) return null
                if (parent.parent.selectedElements.length !== 1) return null
                var element = parent.parent.selectedElements[0]
                if (!element || element.elementType !== "Frame") return null
                return element
            }
            function onBorderRadiusChanged() {
                updateDragProgressFromFrame()
            }
            function onWidthChanged() {
                updateDragProgressFromFrame()
            }
            function onHeightChanged() {
                updateDragProgressFromFrame()
            }
        }
        
        function updateDragProgressFromFrame() {
            if (!parent || !parent.parent || !parent.parent.selectedElements) return
            if (parent.parent.selectedElements.length !== 1) return
            var element = parent.parent.selectedElements[0]
            if (!element || element.elementType !== "Frame") return
            
            // Calculate progress from border radius
            // Max radius is half the smaller dimension
            var maxRadius = Math.min(element.width, element.height) / 2
            if (maxRadius > 0) {
                dragProgress = Math.min(1, element.borderRadius / maxRadius)
            } else {
                dragProgress = 0
            }
        }
        
        // Calculate position based on drag progress
        // Start position: (8, 8)
        // End position: (parent.width/2 - 5, parent.height/2 - 5) - center of controls
        x: 8 + dragProgress * ((parent.width / 2 - 5) - 8)
        y: 8 + dragProgress * ((parent.height / 2 - 5) - 8)
        
        MouseArea {
            anchors.fill: parent
            cursorShape: dragging ? Qt.ClosedHandCursor : Qt.PointingHandCursor
            
            property bool dragging: false
            property point dragStartMouse
            property real dragStartProgress
            property real diagonalLength: Math.sqrt(Math.pow((parent.parent.width / 2 - 5) - 8, 2) + 
                                                   Math.pow((parent.parent.height / 2 - 5) - 8, 2))
            
            onPressed: (mouse) => {
                dragging = true
                dragStartMouse = mapToItem(root, mouse.x, mouse.y)
                dragStartProgress = borderRadiusControl.dragProgress
                ConsoleMessageRepository.addOutput("BorderRadiusControl drag started")
                mouse.accepted = true
                
                // Update frame border radius immediately when starting drag
                updateFrameBorderRadius()
            }
            
            onReleased: {
                dragging = false
                ConsoleMessageRepository.addOutput("BorderRadiusControl drag ended at progress: " + borderRadiusControl.dragProgress.toFixed(2))
            }
            
            onPositionChanged: (mouse) => {
                if (!dragging) return
                
                // Get current mouse position in parent coordinates
                var currentMouse = mapToItem(root, mouse.x, mouse.y)
                
                // Calculate the diagonal vector (from top-left to center)
                var diagonalX = (parent.parent.width / 2 - 5) - 8
                var diagonalY = (parent.parent.height / 2 - 5) - 8
                
                // Calculate mouse delta
                var deltaX = currentMouse.x - dragStartMouse.x
                var deltaY = currentMouse.y - dragStartMouse.y
                
                // Project the mouse delta onto the diagonal vector
                var dotProduct = deltaX * diagonalX + deltaY * diagonalY
                var diagonalLengthSquared = diagonalX * diagonalX + diagonalY * diagonalY
                
                if (diagonalLengthSquared > 0) {
                    var projectedDistance = dotProduct / Math.sqrt(diagonalLengthSquared)
                    var progressDelta = projectedDistance / diagonalLength
                    
                    // Update progress, clamped between 0 and 1
                    borderRadiusControl.dragProgress = Math.max(0, Math.min(1, dragStartProgress + progressDelta))
                    
                    // Update the frame's border radius
                    updateFrameBorderRadius()
                }
            }
        }
    }
    
    // Square control outside the right control bar
    Rectangle {
        id: inlinePrototypePlay
        width: 20
        height: 20
        color: allSelectedAreComponentRelated ? Config.componentControlResizeJointColor : Config.controlResizeJointColor
        antialiasing: true
        z: 2
        
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
            var viewportOverlay = root.parent
            if (viewportOverlay && viewportOverlay.prototypeController && viewportOverlay.prototypeController.isPrototyping) {
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
        
        // Position outside the right control bar
        x: Math.round(parent.width) + 5  // 5px outside the right edge
        y: Math.round(parent.height / 2) - 10  // Centered vertically
        
        // Display "P" for Play
        Text {
            anchors.centerIn: parent
            text: "P"
            font.pixelSize: 12
            font.family: "Arial"
            font.bold: true
            color: "white"
        }
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            
            onClicked: (mouse) => {
                // Access the DesignCanvas through the viewport overlay
                var viewportOverlay = root.parent
                if (!viewportOverlay || !viewportOverlay.canvasView) {
                    ConsoleMessageRepository.addError("Cannot access DesignCanvas from controls")
                    return
                }
                
                var designCanvas = viewportOverlay.canvasView
                var prototypeController = viewportOverlay.prototypeController
                
                if (!prototypeController) {
                    ConsoleMessageRepository.addError("PrototypeController not available")
                    ConsoleMessageRepository.addOutput("viewportOverlay.prototypeController is: " + viewportOverlay.prototypeController)
                    ConsoleMessageRepository.addOutput("Application.activeCanvas is: " + Application.activeCanvas)
                    if (Application.activeCanvas) {
                        ConsoleMessageRepository.addOutput("Application.activeCanvas.prototypeController is: " + Application.activeCanvas.prototypeController)
                    }
                    return
                }
                
                // Check if we have a selected element (should be a frame)
                if (!viewportOverlay.selectedElements || viewportOverlay.selectedElements.length !== 1) {
                    ConsoleMessageRepository.addError("No single frame selected for prototyping")
                    return
                }
                
                var selectedFrame = viewportOverlay.selectedElements[0]
                if (selectedFrame.elementType !== "Frame") {
                    ConsoleMessageRepository.addError("Selected element is not a frame")
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
                
                // 3) Position the viewport so the PrototypeViewableArea top aligns with frame top
                // Wait for frames to be positioned, then get the selected frame's new position
                if (prototypeController.selectedFrameX >= 0) {
                    // The selected frame is now positioned at (selectedFrameX, -350) - top edge at y=-350
                    // The PrototypeViewableArea has a fixed top position
                    
                    // Calculate where the viewable area's top edge will be in viewport coordinates
                    // Using fixed position calculation from PrototypeViewableArea
                    var dropdownHeight = 50  // 30px dropdown + margins
                    var viewableAreaTopInViewport = (viewportOverlay.flickable.height - 450) / 2 + dropdownHeight
                    
                    // We want the frame top (-350) to appear at viewableAreaTopInViewport
                    // So we need to position the canvas such that y=-350 maps to that viewport position
                    var targetCanvasY = -350 - (viewableAreaTopInViewport / designCanvas.zoom) + (viewportOverlay.flickable.height / 2 / designCanvas.zoom)
                    
                    var targetPoint = Qt.point(
                        prototypeController.selectedFrameX,
                        targetCanvasY
                    )
                    
                    designCanvas.moveToPoint(targetPoint, false)
                }
                
                mouse.accepted = true
            }
        }
    }
    
    // HoverBadge component definition
    Component {
        id: hoverBadgeComponent
        
        Item {
            id: hoverBadgeRoot
            
            property alias text: hoverBadgeLabel.text
            property point mousePosition: Qt.point(0, 0)
            
            width: hoverBadgeBackground.width
            height: hoverBadgeBackground.height
            
            // Position the badge 5px right and 10px down from cursor
            x: Math.round(Math.min(mousePosition.x + 5, parent.width - width - 5))
            y: Math.round(Math.min(mousePosition.y + 10, parent.height - height - 5))
            
            // Use visible property directly from Item - no animation for instant appearance
            opacity: visible ? 1.0 : 0.0
            
            Rectangle {
                id: hoverBadgeBackground
                width: hoverBadgeLabel.width + 16
                height: hoverBadgeLabel.height + 8
                radius: 4
                color: root.allSelectedAreComponentRelated ? Config.componentHoverBadgeBackgroundColor : Config.hoverBadgeBackgroundColor
                border.color: root.allSelectedAreComponentRelated ? Config.componentHoverBadgeBorderColor : Config.hoverBadgeBorderColor
                border.width: 1
                antialiasing: true
                
                Text {
                    id: hoverBadgeLabel
                    anchors.centerIn: parent
                    font.pixelSize: 12
                    font.family: "Arial"
                    color: Config.hoverBadgeTextColor
                    text: "0 x 0"
                }
            }
        }
    }
    
    // Export the HoverBadge component for external use
    property alias hoverBadge: hoverBadgeComponent
    
    // Function to check if all selected elements are component-related
    function updateComponentRelatedStatus() {
        if (!parent || !parent.selectedElements || parent.selectedElements.length === 0) {
            allSelectedAreComponentRelated = false
            return
        }
        
        var allComponent = true
        for (var i = 0; i < parent.selectedElements.length; i++) {
            var element = parent.selectedElements[i]
            if (!element) {
                allComponent = false
                break
            }
            
            // Check if element is a ComponentInstance or ComponentVariant
            if (element.elementType !== "ComponentInstance" && element.elementType !== "ComponentVariant") {
                // Check if it's a descendant of a ComponentInstance or ComponentVariant
                var isDescendant = false
                if (parent.canvasView && parent.canvasView.elementModel) {
                    var currentId = element.parentId
                    while (currentId && currentId !== "") {
                        var parentElement = parent.canvasView.elementModel.getElementById(currentId)
                        if (parentElement && (parentElement.elementType === "ComponentInstance" || parentElement.elementType === "ComponentVariant")) {
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
    
    // Function to update frame border radius based on BorderRadiusControl position
    function updateFrameBorderRadius() {
        if (!parent || !parent.selectedElements || parent.selectedElements.length !== 1) return
        var element = parent.selectedElements[0]
        if (!element || element.elementType !== "Frame") return
        
        // Calculate the maximum possible radius (half of the smaller dimension)
        var maxRadius = Math.min(element.width, element.height) / 2
        
        // Set border radius based on drag progress (0 to 1)
        var newRadius = borderRadiusControl.dragProgress * maxRadius
        
        // Update the frame's border radius
        element.borderRadius = newRadius
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