import QtQuick
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
    
    // Control state
    property bool dragging: false
    property string dragMode: "" // "move", "resize-edge-0", "resize-edge-1", etc.
    property point dragStartPoint
    property point dragStartControlPos
    property size dragStartControlSize
    property real dragStartRotation: 0
    property point lastMousePosition: Qt.point(0, 0)
    
    // Signal to notify about mouse position during drag
    signal mouseDragged(point viewportPos)
    
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
            
            onPressed: (mouse) => {
                root.dragging = true
                root.dragMode = "move"
                // Store mouse position in parent coordinates to handle rotation correctly
                var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                root.lastMousePosition = mouseInParent
                root.dragStartPoint = mouseInParent
                root.dragStartControlPos = Qt.point(root.controlX, root.controlY)
                mouse.accepted = true
            }
            
            onReleased: {
                root.dragging = false
                root.dragMode = ""
                root.lastMousePosition = Qt.point(0, 0)
            }
            
            onPositionChanged: (mouse) => {
                if (root.dragMode === "move") {
                    // Calculate delta in parent coordinates so movement follows mouse regardless of rotation
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = mouseInParent
                    var deltaX = mouseInParent.x - root.dragStartPoint.x
                    var deltaY = mouseInParent.y - root.dragStartPoint.y
                    
                    // Update control position in canvas coordinates
                    root.controlX = root.dragStartControlPos.x + deltaX / root.parent.zoomLevel
                    root.controlY = root.dragStartControlPos.y + deltaY / root.parent.zoomLevel
                    
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
            color: Config.controlBarColor
            antialiasing: true
            
            // Position and size based on edge index
            x: {
                switch(index) {
                    case 0: return -5 // top
                    case 1: return parent.width - 5 // right
                    case 2: return -5 // bottom
                    case 3: return -5 // left
                }
            }
            
            y: {
                switch(index) {
                    case 0: return -5 // top
                    case 1: return -5 // right
                    case 2: return parent.height - 5 // bottom
                    case 3: return -5 // left
                }
            }
            
            width: {
                switch(index) {
                    case 0: case 2: return parent.width + 10 // horizontal bars
                    case 1: case 3: return 10 // vertical bars
                }
            }
            
            height: {
                switch(index) {
                    case 0: case 2: return 10 // horizontal bars
                    case 1: case 3: return parent.height + 10 // vertical bars
                }
            }
            
            // Decorative line through the middle of the bar
            Rectangle {
                color: Config.controlBarLineColor
                anchors.centerIn: parent
                width: {
                    switch(index) {
                        case 0: case 2: return parent.width - 4 // horizontal bars
                        case 1: case 3: return 1 // vertical bars
                    }
                }
                height: {
                    switch(index) {
                        case 0: case 2: return 1 // horizontal bars
                        case 1: case 3: return parent.height - 4 // vertical bars
                    }
                }
                antialiasing: true
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: root.getEdgeCursor(index)
                
                property int edgeIndex: index
                property point anchorEdgePoint1  // First point of anchor edge in parent coords
                property point anchorEdgePoint2  // Second point of anchor edge in parent coords
                property point draggedEdgePoint1 // First point of dragged edge in parent coords
                property point draggedEdgePoint2 // Second point of dragged edge in parent coords
                
                onPressed: (mouse) => {
                    root.dragging = true
                    root.dragMode = "resize-edge-" + edgeIndex
                    
                    // Initialize mouse position
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = mouseInParent
                    
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
                    root.lastMousePosition = Qt.point(0, 0)
                }
                
                onPositionChanged: (mouse) => {
                    if (!root.dragMode.startsWith("resize-edge-")) return
                    
                    // Get mouse position in parent coordinates
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = mouseInParent
                    
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
            color: Config.controlRotationJointColor
            antialiasing: true
            z: 1
            
            x: {
                switch(index) {
                    case 0: return -15 // top-left: bottom-right corner at (5, 5) to overlap resize joint
                    case 1: return parent.width - 5 // top-right: bottom-left corner at (parent.width - 5, 5) to overlap resize joint
                    case 2: return parent.width - 5 // bottom-right: top-left corner at (parent.width - 5, parent.height - 5) to overlap resize joint
                    case 3: return -15 // bottom-left: top-right corner at (5, parent.height - 5) to overlap resize joint
                }
            }
            
            y: {
                switch(index) {
                    case 0: return -15 // top-left: bottom-right corner at (5, 5) to overlap resize joint
                    case 1: return -15 // top-right: bottom-left corner at (parent.width - 5, 5) to overlap resize joint
                    case 2: return parent.height - 5 // bottom-right: top-left corner at (parent.width - 5, parent.height - 5) to overlap resize joint
                    case 3: return parent.height - 5 // bottom-left: top-right corner at (5, parent.height - 5) to overlap resize joint
                }
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
                
                property point startMousePos
                property real startAngle
                
                onPressed: (mouse) => {
                    root.dragging = true
                    root.dragMode = "rotate"
                    // Calculate center of the control in parent coordinates
                    var centerPoint = Qt.point(
                        root.x + root.width / 2,
                        root.y + root.height / 2
                    )
                    startMousePos = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = startMousePos
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
                        root.lastMousePosition = currentMousePos
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
    
    // Resize Joints (yellow, 10x10, on top)
    Repeater {
        model: 4
        
        Rectangle {
            id: resizeJoint
            width: 10
            height: 10
            color: Config.controlResizeJointColor
            antialiasing: true
            z: 2
            
            x: {
                switch(index) {
                    case 0: return -5 // top-left
                    case 1: return parent.width - 5 // top-right
                    case 2: return parent.width - 5 // bottom-right
                    case 3: return -5 // bottom-left
                }
            }
            
            y: {
                switch(index) {
                    case 0: return -5 // top-left
                    case 1: return -5 // top-right
                    case 2: return parent.height - 5 // bottom-right
                    case 3: return parent.height - 5 // bottom-left
                }
            }
            
            property int cornerIndex: index
            
            // Decorative circle inside the resize joint
            Rectangle {
                anchors.centerIn: parent
                width: 10
                height: 10
                radius: 5
                color: Config.controlJointCircleFill
                border.color: Config.controlJointCircleBorder
                border.width: 1
                antialiasing: true
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: root.getCornerCursor(parent.cornerIndex)
                
                property int cornerIdx: parent.cornerIndex
                property point anchorCorner  // Opposite corner in parent coords
                property point adjacentCorner1  // Adjacent corner 1 in parent coords
                property point adjacentCorner2  // Adjacent corner 2 in parent coords
                
                onPressed: (mouse) => {
                    root.dragging = true
                    root.dragMode = "resize-corner-" + cornerIdx
                    
                    // Initialize mouse position
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = mouseInParent
                    
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
                
                onReleased: {
                    root.dragging = false
                    root.dragMode = ""
                }
                
                onPositionChanged: (mouse) => {
                    if (!root.dragMode.startsWith("resize-corner-")) return
                    
                    // Get mouse position in parent coordinates
                    var mouseInParent = mapToItem(root.parent, mouse.x, mouse.y)
                    root.lastMousePosition = mouseInParent
                    
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
            x: Math.min(mousePosition.x + 5, parent.width - width - 5)
            y: Math.min(mousePosition.y + 10, parent.height - height - 5)
            
            // Use visible property directly from Item - no animation for instant appearance
            opacity: visible ? 1.0 : 0.0
            
            Rectangle {
                id: hoverBadgeBackground
                width: hoverBadgeLabel.width + 16
                height: hoverBadgeLabel.height + 8
                radius: 4
                color: Config.hoverBadgeBackgroundColor
                border.color: Config.hoverBadgeBorderColor
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
}