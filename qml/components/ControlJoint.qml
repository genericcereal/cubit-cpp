import QtQuick
import Cubit.UI 1.0

// ControlJoint.qml - Reusable joint component for rotation and corner resize
Rectangle {
    id: root
    
    // Joint type: "rotation" or "resize"
    property string jointType: "resize"
    
    // Position: "top-left", "top-right", "bottom-left", "bottom-right"
    property string position: "top-left"
    
    // Parent controls container reference - find it from the parent hierarchy
    property var controlsContainer: parent && parent.parent ? parent.parent : null
    property var selectedElements: []
    property real zoomLevel: 1.0
    
    // The viewport overlay root - we need to find it properly
    property Item viewportRoot: {
        var p = parent
        while (p) {
            if (p.objectName === "viewportOverlay" || (p.canvasView !== undefined && p.flickable !== undefined)) {
                return p
            }
            p = p.parent
        }
        return null
    }
    
    
    // Bounding box properties
    property real selectionBoundingX: 0
    property real selectionBoundingY: 0
    property real selectionBoundingWidth: 0
    property real selectionBoundingHeight: 0
    
    // Visual properties based on type
    width: jointType === "rotation" ? Config.controlRotationJointSize : Config.controlResizeJointSize
    height: jointType === "rotation" ? Config.controlRotationJointSize : Config.controlResizeJointSize
    color: jointType === "rotation" ? Config.controlRotationJointColor : Config.controlResizeJointColor
    antialiasing: true
    
    // Position calculation
    x: {
        if (!parent) return 0
        var overlap = jointType === "rotation" ? Config.controlJointOverlap : 0
        var offset = jointType === "rotation" && (position === "top-left" || position === "bottom-left") 
                    ? -(Config.controlRotationJointSize - overlap) : 0
        
        switch(position) {
            case "top-left":
            case "bottom-left":
                return -Config.controlBarWidth/2 + offset
            case "top-right":
            case "bottom-right":
                return parent.width - Config.controlBarWidth/2
            default:
                return 0
        }
    }
    
    y: {
        if (!parent) return 0
        var overlap = jointType === "rotation" ? Config.controlJointOverlap : 0
        var offset = jointType === "rotation" && (position === "top-left" || position === "top-right") 
                    ? -(Config.controlRotationJointSize - overlap) : 0
        
        switch(position) {
            case "top-left":
            case "top-right":
                return -Config.controlBarHeight/2 + offset
            case "bottom-left":
            case "bottom-right":
                return parent.height - Config.controlBarHeight/2
            default:
                return 0
        }
    }
    
    MouseArea {
        anchors.fill: parent
        preventStealing: true
        
        // Cursor shape
        cursorShape: {
            if (jointType === "rotation") {
                return Qt.PointingHandCursor
            } else {
                // Resize cursors based on position
                switch(root.position) {
                    case "top-left":
                    case "bottom-right":
                        return Qt.SizeFDiagCursor
                    case "top-right":
                    case "bottom-left":
                        return Qt.SizeBDiagCursor
                    default:
                        return Qt.ArrowCursor
                }
            }
        }
        
        // Rotation-specific properties
        property real jointAngleOffset: 0
        property real startControlsRotation: 0
        
        // Resize-specific properties
        property real startCanvasX: 0
        property real startCanvasY: 0
        property var startElementStates: []
        property real startBoundingX: 0
        property real startBoundingY: 0
        property real startBoundingWidth: 0
        property real startBoundingHeight: 0
        property real startLocalX: 0
        property real startLocalY: 0
        property real anchorWorldX: 0
        property real anchorWorldY: 0
        property real startControlsX: 0
        property real startControlsY: 0
        property real startControlsWidth: 0
        property real startControlsHeight: 0
        property real mouseOffsetX: 0
        property real mouseOffsetY: 0
        
        onPressed: function(mouse) {
            if (jointType === "rotation") {
                handleRotationPress(mouse)
            } else {
                handleResizePress(mouse)
            }
        }
        
        onPositionChanged: function(mouse) {
            if (!pressed) return
            
            if (jointType === "rotation") {
                handleRotationDrag(mouse)
            } else {
                handleResizeDrag(mouse)
            }
        }
        
        // Rotation handling
        function handleRotationPress(mouse) {
            if (!controlsContainer || !controlsContainer.parent) return
            
            startControlsRotation = controlsContainer.controlsRotation
            
            // Map the mouse position to the controls container's parent coordinate space
            var mouseInParent = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            
            // Get center of controls container in its parent's coordinate space
            var centerX = controlsContainer.x + controlsContainer.width / 2
            var centerY = controlsContainer.y + controlsContainer.height / 2
            
            // Calculate angle from center to mouse
            var dx = mouseInParent.x - centerX
            var dy = mouseInParent.y - centerY
            var mouseAngle = Math.atan2(dy, dx) * 180 / Math.PI
            
            // Store the offset to maintain the joint's position under the mouse
            jointAngleOffset = mouseAngle - startControlsRotation
        }
        
        function handleRotationDrag(mouse) {
            if (!controlsContainer) return
            
            // Map the mouse position to the controls container's parent coordinate space
            var mouseInParent = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            
            // Get center of controls container in its parent's coordinate space
            var centerX = controlsContainer.x + controlsContainer.width / 2
            var centerY = controlsContainer.y + controlsContainer.height / 2
            
            // Calculate current angle from center to mouse
            var dx = mouseInParent.x - centerX
            var dy = mouseInParent.y - centerY
            var currentMouseAngle = Math.atan2(dy, dx) * 180 / Math.PI
            
            // Set rotation to maintain the offset
            controlsContainer.controlsRotation = currentMouseAngle - jointAngleOffset
        }
        
        // Resize handling
        function handleResizePress(mouse) {
            if (!controlsContainer || !controlsContainer.parent) return
            
            // Pause bindings during drag
            controlsContainer.dragging = true
            
            var globalPos = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            var canvasPos = controlsContainer.viewportToCanvas(globalPos.x, globalPos.y)
            startCanvasX = canvasPos.x
            startCanvasY = canvasPos.y
            
            // Store initial mouse position in container-local coords
            var localMouse = controlsContainer.mapFromItem(controlsContainer.parent, globalPos.x, globalPos.y)
            startLocalX = localMouse.x
            startLocalY = localMouse.y
            
            // Calculate anchor point (opposite corner)
            var anchorLocal
            switch(root.position) {
                case "top-left":
                    anchorLocal = Qt.point(controlsContainer.width, controlsContainer.height)
                    break
                case "top-right":
                    anchorLocal = Qt.point(0, controlsContainer.height)
                    break
                case "bottom-right":
                    anchorLocal = Qt.point(0, 0)
                    break
                case "bottom-left":
                    anchorLocal = Qt.point(controlsContainer.width, 0)
                    break
            }
            
            // Map anchor to world coords
            var anchorWorld = controlsContainer.mapToItem(controlsContainer.parent, anchorLocal.x, anchorLocal.y)
            anchorWorldX = anchorWorld.x
            anchorWorldY = anchorWorld.y
            
            // Store initial controls state
            startControlsX = controlsContainer.x
            startControlsY = controlsContainer.y
            startControlsWidth = controlsContainer.width
            startControlsHeight = controlsContainer.height
            
            // Calculate initial offset between mouse and corner in local space
            switch(root.position) {
                case "top-left":
                    mouseOffsetX = localMouse.x - 0
                    mouseOffsetY = localMouse.y - 0
                    break
                case "top-right":
                    mouseOffsetX = localMouse.x - controlsContainer.width
                    mouseOffsetY = localMouse.y - 0
                    break
                case "bottom-right":
                    mouseOffsetX = localMouse.x - controlsContainer.width
                    mouseOffsetY = localMouse.y - controlsContainer.height
                    break
                case "bottom-left":
                    mouseOffsetX = localMouse.x - 0
                    mouseOffsetY = localMouse.y - controlsContainer.height
                    break
            }
            
            // Store initial states
            startElementStates = []
            for (var i = 0; i < selectedElements.length; i++) {
                startElementStates.push({
                    x: selectedElements[i].x,
                    y: selectedElements[i].y,
                    width: selectedElements[i].width,
                    height: selectedElements[i].height
                })
            }
            
            // Store bounding box
            startBoundingX = selectionBoundingX
            startBoundingY = selectionBoundingY
            startBoundingWidth = selectionBoundingWidth
            startBoundingHeight = selectionBoundingHeight
        }
        
        function handleResizeDrag(mouse) {
            if (!controlsContainer) return
            
            // Get current mouse position in global coordinates
            var globalPos = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            
            // Calculate the desired corner position in global space
            var desiredCornerX = globalPos.x - mouseOffsetX * zoomLevel
            var desiredCornerY = globalPos.y - mouseOffsetY * zoomLevel
            
            // Calculate new dimensions and position based on which corner is being dragged
            var newX = startControlsX
            var newY = startControlsY
            var newWidth = startControlsWidth
            var newHeight = startControlsHeight
            
            switch(root.position) {
                case "top-left":
                    // Top-left corner moves to desired position, bottom-right stays at anchor
                    newX = desiredCornerX
                    newY = desiredCornerY
                    newWidth = anchorWorldX - desiredCornerX
                    newHeight = anchorWorldY - desiredCornerY
                    break
                case "top-right":
                    // Top-right corner moves to desired position, bottom-left stays at anchor
                    newY = desiredCornerY
                    newWidth = desiredCornerX - anchorWorldX
                    newHeight = anchorWorldY - desiredCornerY
                    break
                case "bottom-right":
                    // Bottom-right corner moves to desired position, top-left stays at anchor
                    newWidth = desiredCornerX - anchorWorldX
                    newHeight = desiredCornerY - anchorWorldY
                    break
                case "bottom-left":
                    // Bottom-left corner moves to desired position, top-right stays at anchor
                    newX = desiredCornerX
                    newWidth = anchorWorldX - desiredCornerX
                    newHeight = desiredCornerY - anchorWorldY
                    break
            }
            
            // Apply minimum size constraints and handle flipping
            if (newWidth < 1) {
                // Handle horizontal flip
                switch(root.position) {
                    case "top-left":
                    case "bottom-left":
                        newX = anchorWorldX - 1
                        break
                    case "top-right":
                    case "bottom-right":
                        newX = anchorWorldX
                        break
                }
                newWidth = 1
            }
            
            if (newHeight < 1) {
                // Handle vertical flip
                switch(root.position) {
                    case "top-left":
                    case "top-right":
                        newY = anchorWorldY - 1
                        break
                    case "bottom-left":
                    case "bottom-right":
                        newY = anchorWorldY
                        break
                }
                newHeight = 1
            }
            
            // Calculate deltas for element updates
            var deltaX = (newX - startControlsX) / zoomLevel
            var deltaY = (newY - startControlsY) / zoomLevel
            var deltaWidth = (newWidth - startControlsWidth) / zoomLevel
            var deltaHeight = (newHeight - startControlsHeight) / zoomLevel
            
            // Update elements
            if (selectedElements.length === 1) {
                // For corner resize, we need to use the full position/size deltas
                var element = selectedElements[0]
                var startState = startElementStates[0]
                
                switch(root.position) {
                    case "top-left":
                        element.x = startState.x + deltaX
                        element.y = startState.y + deltaY
                        element.width = startState.width - deltaX
                        element.height = startState.height - deltaY
                        break
                    case "top-right":
                        element.y = startState.y + deltaY
                        element.width = startState.width + deltaWidth
                        element.height = startState.height - deltaY
                        break
                    case "bottom-right":
                        element.width = startState.width + deltaWidth
                        element.height = startState.height + deltaHeight
                        break
                    case "bottom-left":
                        element.x = startState.x + deltaX
                        element.width = startState.width - deltaX
                        element.height = startState.height + deltaHeight
                        break
                }
            } else {
                // For multiple selection, calculate position-based deltas in canvas space
                var canvasDeltaX = 0
                var canvasDeltaY = 0
                
                switch(root.position) {
                    case "top-left":
                        canvasDeltaX = deltaX
                        canvasDeltaY = deltaY
                        break
                    case "top-right":
                        canvasDeltaX = 0
                        canvasDeltaY = deltaY
                        break
                    case "bottom-right":
                        canvasDeltaX = 0
                        canvasDeltaY = 0
                        break
                    case "bottom-left":
                        canvasDeltaX = deltaX
                        canvasDeltaY = 0
                        break
                }
                
                handleMultipleElementResize(canvasDeltaX, canvasDeltaY)
            }
            
            // Update controls - direct assignment, no incremental changes
            controlsContainer.x = newX
            controlsContainer.y = newY
            controlsContainer.width = newWidth
            controlsContainer.height = newHeight
        }
        
        
        // Single element corner resize
        function handleSingleElementResize(deltaX, deltaY) {
            var element = selectedElements[0]
            var startState = startElementStates[0]
            
            switch(root.position) {
                case "top-left":
                    var newLeft = startState.x + deltaX
                    var newTop = startState.y + deltaY
                    var right = startState.x + startState.width
                    var bottom = startState.y + startState.height
                    
                    element.x = newLeft + 1 > right ? right - 1 : newLeft
                    element.width = newLeft + 1 > right ? 1 + (newLeft - right) : right - newLeft
                    element.y = newTop + 1 > bottom ? bottom - 1 : newTop
                    element.height = newTop + 1 > bottom ? 1 + (newTop - bottom) : bottom - newTop
                    break
                    
                case "top-right":
                    var left = startState.x
                    var newTop = startState.y + deltaY
                    var newRight = startState.x + startState.width + deltaX
                    var bottom = startState.y + startState.height
                    
                    element.x = newRight < left + 1 ? newRight : left
                    element.width = newRight < left + 1 ? 1 + (left - newRight) : newRight - left
                    element.y = newTop + 1 > bottom ? bottom - 1 : newTop
                    element.height = newTop + 1 > bottom ? 1 + (newTop - bottom) : bottom - newTop
                    break
                    
                case "bottom-right":
                    var left = startState.x
                    var top = startState.y
                    var newRight = startState.x + startState.width + deltaX
                    var newBottom = startState.y + startState.height + deltaY
                    
                    element.x = newRight < left + 1 ? newRight : left
                    element.width = newRight < left + 1 ? 1 + (left - newRight) : newRight - left
                    element.y = newBottom < top + 1 ? newBottom : top
                    element.height = newBottom < top + 1 ? 1 + (top - newBottom) : newBottom - top
                    break
                    
                case "bottom-left":
                    var newLeft = startState.x + deltaX
                    var top = startState.y
                    var right = startState.x + startState.width
                    var newBottom = startState.y + startState.height + deltaY
                    
                    element.x = newLeft + 1 > right ? right - 1 : newLeft
                    element.width = newLeft + 1 > right ? 1 + (newLeft - right) : right - newLeft
                    element.y = newBottom < top + 1 ? newBottom : top
                    element.height = newBottom < top + 1 ? 1 + (top - newBottom) : newBottom - top
                    break
            }
        }
        
        // Multiple element proportional resize
        function handleMultipleElementResize(deltaX, deltaY) {
            var scaleX = 1.0
            var scaleY = 1.0
            var anchorX, anchorY
            
            // Determine anchor point and scale factors based on corner
            switch(root.position) {
                case "top-left":
                    anchorX = startBoundingX + startBoundingWidth
                    anchorY = startBoundingY + startBoundingHeight
                    var newLeft = startBoundingX + deltaX
                    var newTop = startBoundingY + deltaY
                    
                    scaleX = newLeft >= anchorX - 1 ? (newLeft - anchorX + 1) / startBoundingWidth
                                                    : (anchorX - newLeft) / startBoundingWidth
                    scaleY = newTop >= anchorY - 1 ? (newTop - anchorY + 1) / startBoundingHeight
                                                   : (anchorY - newTop) / startBoundingHeight
                    
                    applyScaling(anchorX, anchorY, scaleX, scaleY, 
                               newLeft >= anchorX - 1, newTop >= anchorY - 1)
                    break
                    
                case "top-right":
                    anchorX = startBoundingX
                    anchorY = startBoundingY + startBoundingHeight
                    var newRight = startBoundingX + startBoundingWidth + deltaX
                    var newTop = startBoundingY + deltaY
                    
                    scaleX = newRight <= anchorX + 1 ? (anchorX - newRight + 1) / startBoundingWidth
                                                     : (newRight - anchorX) / startBoundingWidth
                    scaleY = newTop >= anchorY - 1 ? (newTop - anchorY + 1) / startBoundingHeight
                                                   : (anchorY - newTop) / startBoundingHeight
                    
                    applyScaling(anchorX, anchorY, scaleX, scaleY,
                               newRight <= anchorX + 1, newTop >= anchorY - 1)
                    break
                    
                case "bottom-right":
                    anchorX = startBoundingX
                    anchorY = startBoundingY
                    var newRight = startBoundingX + startBoundingWidth + deltaX
                    var newBottom = startBoundingY + startBoundingHeight + deltaY
                    
                    scaleX = newRight <= anchorX + 1 ? (anchorX - newRight + 1) / startBoundingWidth
                                                     : (newRight - anchorX) / startBoundingWidth
                    scaleY = newBottom <= anchorY + 1 ? (anchorY - newBottom + 1) / startBoundingHeight
                                                      : (newBottom - anchorY) / startBoundingHeight
                    
                    applyScaling(anchorX, anchorY, scaleX, scaleY,
                               newRight <= anchorX + 1, newBottom <= anchorY + 1)
                    break
                    
                case "bottom-left":
                    anchorX = startBoundingX + startBoundingWidth
                    anchorY = startBoundingY
                    var newLeft = startBoundingX + deltaX
                    var newBottom = startBoundingY + startBoundingHeight + deltaY
                    
                    scaleX = newLeft >= anchorX - 1 ? (newLeft - anchorX + 1) / startBoundingWidth
                                                    : (anchorX - newLeft) / startBoundingWidth
                    scaleY = newBottom <= anchorY + 1 ? (anchorY - newBottom + 1) / startBoundingHeight
                                                      : (newBottom - anchorY) / startBoundingHeight
                    
                    applyScaling(anchorX, anchorY, scaleX, scaleY,
                               newLeft >= anchorX - 1, newBottom <= anchorY + 1)
                    break
            }
        }
        
        // Apply scaling to all elements
        function applyScaling(anchorX, anchorY, scaleX, scaleY, flippedX, flippedY) {
            for (var i = 0; i < selectedElements.length; i++) {
                var elem = selectedElements[i]
                var state = startElementStates[i]
                
                // Calculate distances from anchor
                var distX = position.includes("left") ? anchorX - (state.x + state.width) : state.x - anchorX
                var distY = position.includes("top") ? anchorY - (state.y + state.height) : state.y - anchorY
                
                // Apply new dimensions
                elem.width = state.width * Math.abs(scaleX)
                elem.height = state.height * Math.abs(scaleY)
                
                // Calculate new position based on flip state
                if (position === "top-left") {
                    elem.x = flippedX ? anchorX + distX * Math.abs(scaleX) 
                                     : anchorX - distX * Math.abs(scaleX) - elem.width
                    elem.y = flippedY ? anchorY + distY * Math.abs(scaleY)
                                     : anchorY - distY * Math.abs(scaleY) - elem.height
                } else if (position === "top-right") {
                    elem.x = flippedX ? anchorX - distX * Math.abs(scaleX) - elem.width
                                     : anchorX + distX * Math.abs(scaleX)
                    elem.y = flippedY ? anchorY + distY * Math.abs(scaleY)
                                     : anchorY - distY * Math.abs(scaleY) - elem.height
                } else if (position === "bottom-right") {
                    elem.x = flippedX ? anchorX - distX * Math.abs(scaleX) - elem.width
                                     : anchorX + distX * Math.abs(scaleX)
                    elem.y = flippedY ? anchorY - distY * Math.abs(scaleY) - elem.height
                                     : anchorY + distY * Math.abs(scaleY)
                } else if (position === "bottom-left") {
                    elem.x = flippedX ? anchorX + distX * Math.abs(scaleX)
                                     : anchorX - distX * Math.abs(scaleX) - elem.width
                    elem.y = flippedY ? anchorY - distY * Math.abs(scaleY) - elem.height
                                     : anchorY + distY * Math.abs(scaleY)
                }
            }
        }
        
        onReleased: {
            // Resume bindings
            if (controlsContainer && jointType === "resize") {
                controlsContainer.dragging = false
            }
        }
    }
}