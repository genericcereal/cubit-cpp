import QtQuick
import Cubit.UI 1.0

// ControlBar.qml - Reusable resize bar component
Rectangle {
    id: root
    
    // Position can be "top", "bottom", "left", "right"
    property string position: "top"
    property var controlsContainer: parent  // The parent is always controlsContainer
    property var selectedElements: []
    property real zoomLevel: 1.0
    
    // Bounding box properties passed from parent
    property real selectionBoundingX: 0
    property real selectionBoundingY: 0
    property real selectionBoundingWidth: 0
    property real selectionBoundingHeight: 0
    
    // Bar dimensions based on position
    width: (position === "left" || position === "right") ? Config.controlBarWidth : parent.width
    height: (position === "top" || position === "bottom") ? Config.controlBarHeight : parent.height
    
    // Bar positioning
    x: {
        switch(position) {
            case "left": return -Config.controlBarWidth/2
            case "right": return parent.width - Config.controlBarWidth/2
            default: return 0
        }
    }
    
    y: {
        switch(position) {
            case "top": return -Config.controlBarHeight/2
            case "bottom": return parent.height - Config.controlBarHeight/2
            default: return 0
        }
    }
    
    color: Config.controlBarColor
    antialiasing: true
    
    // Center line for visual clarity
    Rectangle {
        anchors.centerIn: parent
        width: (position === "left" || position === "right") ? Config.controlLineWidth : parent.width
        height: (position === "top" || position === "bottom") ? parent.height : Config.controlLineWidth
        color: Config.controlBarColor
        antialiasing: true
    }
    
    // Mouse handling
    MouseArea {
        anchors.fill: parent
        cursorShape: (position === "top" || position === "bottom") ? Qt.SizeVerCursor : Qt.SizeHorCursor
        preventStealing: true
        
        // State for drag operation
        property real startCanvasX: 0
        property real startCanvasY: 0
        property var startElementStates: []
        property real startBoundingX: 0
        property real startBoundingY: 0
        property real startBoundingWidth: 0
        property real startBoundingHeight: 0
        property real startRotation: 0
        property real anchorWorldX: 0
        property real anchorWorldY: 0
        property real startLocalY: 0
        property real startLocalX: 0
        property real startControlsX: 0
        property real startControlsY: 0
        property real startControlsWidth: 0
        property real startControlsHeight: 0
        property real mouseOffsetX: 0
        property real mouseOffsetY: 0
        property real startMouseGlobalX: 0
        property real startMouseGlobalY: 0
        
        // Get the anchor edge (opposite edge that stays fixed during resize)
        function getAnchorPosition() {
            switch(root.position) {
                case "top": return { x: controlsContainer.width / 2, y: controlsContainer.height }  // bottom center
                case "bottom": return { x: controlsContainer.width / 2, y: 0 }  // top center
                case "left": return { x: controlsContainer.width, y: controlsContainer.height / 2 }  // right center
                case "right": return { x: 0, y: controlsContainer.height / 2 }  // left center
            }
        }
        
        onPressed: function(mouse) {
            if (!controlsContainer || !controlsContainer.parent) return
            
            // Pause bindings during drag
            controlsContainer.dragging = true
            
            var globalPos = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            var canvasPos = controlsContainer.viewportToCanvas(globalPos.x, globalPos.y)
            startCanvasX = canvasPos.x
            startCanvasY = canvasPos.y
            startRotation = controlsContainer.controlsRotation
            startMouseGlobalX = globalPos.x
            startMouseGlobalY = globalPos.y
            
            // Calculate anchor point in world coordinates
            var anchorLocal = getAnchorPosition()
            var anchorWorld = controlsContainer.mapToItem(controlsContainer.parent, anchorLocal.x, anchorLocal.y)
            anchorWorldX = anchorWorld.x
            anchorWorldY = anchorWorld.y
            
            // Store initial mouse position in container-local coords
            // Map from the bar to the container
            var localMouse = mapToItem(controlsContainer, mouse.x, mouse.y)
            startLocalX = localMouse.x
            startLocalY = localMouse.y
            
            // Store initial controls state
            startControlsX = controlsContainer.x
            startControlsY = controlsContainer.y
            startControlsWidth = controlsContainer.width
            startControlsHeight = controlsContainer.height
            
            // Calculate initial offset between mouse and edge in local space
            switch(root.position) {
                case "top":
                    mouseOffsetY = localMouse.y - 0
                    break
                case "bottom":
                    mouseOffsetY = localMouse.y - controlsContainer.height
                    break
                case "left":
                    mouseOffsetX = localMouse.x - 0
                    break
                case "right":
                    mouseOffsetX = localMouse.x - controlsContainer.width
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
        
        onPositionChanged: function(mouse) {
            if (!pressed) return
            
            // Get current mouse position in both coordinate systems
            var globalPos = mapToItem(controlsContainer.parent, mouse.x, mouse.y)
            var localMouse = mapToItem(controlsContainer, mouse.x, mouse.y)
            
            // Calculate where the edge should be in local space
            var desiredEdgeLocal = 0
            var deltaLocal = 0
            
            if (root.position === "top") {
                // Calculate mouse movement in global coordinates
                var mouseDeltaGlobalX = globalPos.x - startMouseGlobalX
                var mouseDeltaGlobalY = globalPos.y - startMouseGlobalY
                
                // Project the mouse movement onto the bar's normal direction
                // For a top bar, the normal points up (-Y direction in local space)
                // In global space with rotation, this normal is rotated
                var angle = startRotation * Math.PI / 180
                var normalX = Math.sin(angle)
                var normalY = -Math.cos(angle)
                
                // Project mouse delta onto the normal (dot product)
                var projectedDelta = mouseDeltaGlobalX * normalX + mouseDeltaGlobalY * normalY
                
                // Update container size (the projected delta represents how much to shrink/grow)
                // When dragging down (positive delta), height should increase
                var newHeight = startControlsHeight + projectedDelta
                controlsContainer.height = newHeight
                
                // Calculate canvas delta
                // For top edge, negative projected delta means edge moves up (Y decreases)
                var canvasDeltaY = -projectedDelta / zoomLevel
                
                resizeTop(canvasDeltaY)
            } else if (root.position === "bottom") {
                // In local space, bottom edge should be at mouse minus offset
                desiredEdgeLocal = localMouse.y - mouseOffsetY
                deltaLocal = desiredEdgeLocal - startControlsHeight
                
                // Update size
                var newHeight = startControlsHeight + deltaLocal
                controlsContainer.height = newHeight
                
                // When rotated, we need to account for rotation in the canvas delta calculation
                var angle = startRotation * Math.PI / 180
                var canvasDeltaY = deltaLocal * Math.cos(angle) / zoomLevel
                var canvasDeltaX = -deltaLocal * Math.sin(angle) / zoomLevel
                
                resizeBottom(canvasDeltaY)
            } else if (root.position === "left") {
                // Calculate mouse movement in global coordinates
                var mouseDeltaGlobalX = globalPos.x - startMouseGlobalX
                var mouseDeltaGlobalY = globalPos.y - startMouseGlobalY
                
                // Project the mouse movement onto the bar's normal direction
                // For a left bar, the normal points left (-X direction in local space)
                // In global space with rotation, this normal is rotated
                var angle = startRotation * Math.PI / 180
                var normalX = -Math.cos(angle)
                var normalY = -Math.sin(angle)
                
                // Project mouse delta onto the normal (dot product)
                var projectedDelta = mouseDeltaGlobalX * normalX + mouseDeltaGlobalY * normalY
                
                // Update container size (the projected delta represents how much to shrink/grow)
                // When dragging right (positive delta), width should increase
                var newWidth = startControlsWidth + projectedDelta
                controlsContainer.width = newWidth
                
                // Calculate canvas delta
                // For left edge, negative projected delta means edge moves left (X decreases)
                var canvasDeltaX = -projectedDelta / zoomLevel
                
                resizeLeft(canvasDeltaX)
            } else if (root.position === "right") {
                // In local space, right edge should be at mouse minus offset
                desiredEdgeLocal = localMouse.x - mouseOffsetX
                deltaLocal = desiredEdgeLocal - startControlsWidth
                
                // Update size
                var newWidth = startControlsWidth + deltaLocal
                controlsContainer.width = newWidth
                
                // When rotated, we need to account for rotation in the canvas delta calculation
                var angle = startRotation * Math.PI / 180
                var canvasDeltaX = deltaLocal * Math.cos(angle) / zoomLevel
                var canvasDeltaY = deltaLocal * Math.sin(angle) / zoomLevel
                
                resizeRight(canvasDeltaX)
            }
            
            // Update position to keep anchor fixed
            var anchorLocal = getAnchorPosition()
            var currentAnchorWorld = controlsContainer.mapToItem(controlsContainer.parent, anchorLocal.x, anchorLocal.y)
            var offsetX = anchorWorldX - currentAnchorWorld.x
            var offsetY = anchorWorldY - currentAnchorWorld.y
            controlsContainer.x += offsetX
            controlsContainer.y += offsetY
            
        }
        
        
        // Resize functions for each edge
        function resizeTop(deltaY) {
            if (selectedElements.length === 1) {
                // Single selection - direct manipulation
                var element = selectedElements[0]
                var startState = startElementStates[0]
                
                var newTop = startState.y + deltaY
                var bottom = startState.y + startState.height
                
                if (newTop + 1 > bottom) {
                    // Flip
                    element.y = bottom - 1
                    element.height = 1 + (newTop - bottom)
                } else {
                    element.y = newTop
                    element.height = bottom - newTop
                }
            } else {
                // Multiple selection - proportional scaling
                resizeMultipleTop(deltaY)
            }
        }
        
        function resizeBottom(deltaY) {
            if (selectedElements.length === 1) {
                var element = selectedElements[0]
                var startState = startElementStates[0]
                
                var top = startState.y
                var newBottom = startState.y + startState.height + deltaY
                
                if (newBottom < top + 1) {
                    // Flip
                    element.y = newBottom
                    element.height = 1 + (top - newBottom)
                } else {
                    element.y = top
                    element.height = newBottom - top
                }
            } else {
                resizeMultipleBottom(deltaY)
            }
        }
        
        function resizeLeft(deltaX) {
            if (selectedElements.length === 1) {
                var element = selectedElements[0]
                var startState = startElementStates[0]
                
                var newLeft = startState.x + deltaX
                var right = startState.x + startState.width
                
                if (newLeft + 1 > right) {
                    // Flip
                    element.x = right - 1
                    element.width = 1 + (newLeft - right)
                } else {
                    element.x = newLeft
                    element.width = right - newLeft
                }
            } else {
                resizeMultipleLeft(deltaX)
            }
        }
        
        function resizeRight(deltaX) {
            if (selectedElements.length === 1) {
                var element = selectedElements[0]
                var startState = startElementStates[0]
                
                var left = startState.x
                var newRight = startState.x + startState.width + deltaX
                
                if (newRight < left + 1) {
                    // Flip
                    element.x = newRight
                    element.width = 1 + (left - newRight)
                } else {
                    element.x = left
                    element.width = newRight - left
                }
            } else {
                resizeMultipleRight(deltaX)
            }
        }
        
        // Multiple selection resize functions
        function resizeMultipleTop(deltaY) {
            var newBoundingTop = startBoundingY + deltaY
            var boundingBottom = startBoundingY + startBoundingHeight
            var isFlipped = newBoundingTop > boundingBottom - 1
            
            if (!isFlipped) {
                var scaleFactor = (boundingBottom - newBoundingTop) / startBoundingHeight
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromBottom = (startBoundingY + startBoundingHeight) - (state.y + state.height)
                    elem.height = state.height * scaleFactor
                    elem.y = boundingBottom - relativeFromBottom * scaleFactor - elem.height
                }
            } else {
                var flippedHeight = newBoundingTop - boundingBottom
                var scaleFactor = flippedHeight / startBoundingHeight
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromTop = state.y - startBoundingY
                    elem.height = state.height * scaleFactor
                    elem.y = boundingBottom + (startBoundingHeight - relativeFromTop - state.height) * scaleFactor
                }
            }
        }
        
        function resizeMultipleBottom(deltaY) {
            var boundingTop = startBoundingY
            var newBoundingBottom = startBoundingY + startBoundingHeight + deltaY
            var isFlipped = newBoundingBottom < boundingTop + 1
            
            if (!isFlipped) {
                var scaleFactor = (newBoundingBottom - boundingTop) / startBoundingHeight
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeTop = state.y - startBoundingY
                    elem.y = boundingTop + relativeTop * scaleFactor
                    elem.height = state.height * scaleFactor
                }
            } else {
                var flippedHeight = boundingTop - newBoundingBottom
                var scaleFactor = flippedHeight / startBoundingHeight
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromBottom = (startBoundingY + startBoundingHeight) - (state.y + state.height)
                    elem.height = state.height * scaleFactor
                    elem.y = newBoundingBottom + relativeFromBottom * scaleFactor
                }
            }
        }
        
        function resizeMultipleLeft(deltaX) {
            var newBoundingLeft = startBoundingX + deltaX
            var boundingRight = startBoundingX + startBoundingWidth
            var isFlipped = newBoundingLeft > boundingRight - 1
            
            if (!isFlipped) {
                var scaleFactor = (boundingRight - newBoundingLeft) / startBoundingWidth
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromRight = (startBoundingX + startBoundingWidth) - (state.x + state.width)
                    elem.width = state.width * scaleFactor
                    elem.x = boundingRight - relativeFromRight * scaleFactor - elem.width
                }
            } else {
                var flippedWidth = newBoundingLeft - boundingRight
                var scaleFactor = flippedWidth / startBoundingWidth
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromLeft = state.x - startBoundingX
                    elem.width = state.width * scaleFactor
                    elem.x = boundingRight + (startBoundingWidth - relativeFromLeft - state.width) * scaleFactor
                }
            }
        }
        
        function resizeMultipleRight(deltaX) {
            var boundingLeft = startBoundingX
            var newBoundingRight = startBoundingX + startBoundingWidth + deltaX
            var isFlipped = newBoundingRight < boundingLeft + 1
            
            if (!isFlipped) {
                var scaleFactor = (newBoundingRight - boundingLeft) / startBoundingWidth
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeLeft = state.x - startBoundingX
                    elem.x = boundingLeft + relativeLeft * scaleFactor
                    elem.width = state.width * scaleFactor
                }
            } else {
                var flippedWidth = boundingLeft - newBoundingRight
                var scaleFactor = flippedWidth / startBoundingWidth
                
                for (var i = 0; i < selectedElements.length; i++) {
                    var elem = selectedElements[i]
                    var state = startElementStates[i]
                    
                    var relativeFromRight = (startBoundingX + startBoundingWidth) - (state.x + state.width)
                    elem.width = state.width * scaleFactor
                    elem.x = newBoundingRight + relativeFromRight * scaleFactor
                }
            }
        }
        
        onReleased: {
            // Resume bindings
            if (controlsContainer) {
                controlsContainer.dragging = false
            }
        }
    }
}