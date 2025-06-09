import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    
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
    
    // Selection box visual
    Rectangle {
        id: selectionBox
        visible: canvasView && canvasView.selectionBoxHandler.active
        color: "transparent"
        border.color: Config.selectionColor
        border.width: 1
        
        // Convert canvas coordinates to viewport coordinates
        x: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return (Math.min(handler.startPoint.x, handler.currentPoint.x) - root.canvasMinX) * zoomLevel - flickable.contentX
        }
        
        y: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return (Math.min(handler.startPoint.y, handler.currentPoint.y) - root.canvasMinY) * zoomLevel - flickable.contentY
        }
        
        width: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.x - handler.startPoint.x) * zoomLevel
        }
        
        height: {
            if (!canvasView || !canvasView.selectionBoxHandler.active) return 0
            var handler = canvasView.selectionBoxHandler
            return Math.abs(handler.currentPoint.y - handler.startPoint.y) * zoomLevel
        }
    }
    
    // Calculate bounding box for all selected elements
    property real selectionBoundingX: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
        }
        return minX
    }
    
    property real selectionBoundingY: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
        }
        return minY
    }
    
    property real selectionBoundingWidth: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        var maxX = -Infinity  
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
            maxX = Math.max(maxX, selectedElements[i].x + selectedElements[i].width)
        }
        return maxX - minX
    }
    
    property real selectionBoundingHeight: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        var maxY = -Infinity
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
            maxY = Math.max(maxY, selectedElements[i].y + selectedElements[i].height)
        }
        return maxY - minY
    }
    
    // Controls for selected elements (single or multiple)
    Item {
        id: controlsContainer
        visible: selectedElements.length > 0
        
        // Controls rotation angle
        property real controlsRotation: 0
        
        // Always use bounding box calculations for consistency
        property real controlX: (selectionBoundingX - root.canvasMinX) * zoomLevel - (flickable ? flickable.contentX : 0)
        property real controlY: (selectionBoundingY - root.canvasMinY) * zoomLevel - (flickable ? flickable.contentY : 0)
        property real controlWidth: selectionBoundingWidth * zoomLevel
        property real controlHeight: selectionBoundingHeight * zoomLevel
        
        x: controlX
        y: controlY
        width: controlWidth
        height: controlHeight
        
        // Apply rotation transform around center
        transform: Rotation {
            origin.x: controlsContainer.width / 2
            origin.y: controlsContainer.height / 2
            angle: controlsContainer.controlsRotation
        }
        
        
        
        // Helper function to convert viewport coordinates to canvas coordinates
        function viewportToCanvas(viewX, viewY) {
            var canvasX = (flickable.contentX + viewX) / zoomLevel + root.canvasMinX
            var canvasY = (flickable.contentY + viewY) / zoomLevel + root.canvasMinY
            return Qt.point(canvasX, canvasY)
        }
        
        
        // Inner rectangle (yellow with 5% opacity)
        Rectangle {
            anchors.fill: parent
            color: Config.controlInnerRectColor
            z: -1  // Behind other controls
        }
                
        // Top bar
        Rectangle {
            x: 0
            y: -Config.controlBarHeight/2
            width: parent.width
            height: Config.controlBarHeight
            color: Config.controlBarColor
            
            // Center line
            Rectangle {
                anchors.centerIn: parent
                width: parent.width
                height: Config.controlLineWidth
                color: Config.controlBarColor
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                preventStealing: true
                
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
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    startCanvasY = canvasPos.y
                    startRotation = controlsContainer.controlsRotation
                    
                    // Calculate the anchor point (bottom edge center) in world coordinates
                    var angle = controlsContainer.controlsRotation * Math.PI / 180
                    var cos = Math.cos(angle)
                    var sin = Math.sin(angle)
                    
                    // Bottom edge center in local control coordinates
                    var localAnchorX = controlsContainer.width / 2
                    var localAnchorY = controlsContainer.height
                    
                    // Transform to world coordinates
                    var centerX = controlsContainer.x + controlsContainer.width / 2
                    var centerY = controlsContainer.y + controlsContainer.height / 2
                    
                    // Rotate around center
                    var relX = localAnchorX - controlsContainer.width / 2
                    var relY = localAnchorY - controlsContainer.height / 2
                    
                    anchorWorldX = centerX + relX * cos - relY * sin
                    anchorWorldY = centerY + relX * sin + relY * cos
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
                            y: selectedElements[i].y,
                            height: selectedElements[i].height
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingX = selectionBoundingX
                    startBoundingY = selectionBoundingY
                    startBoundingWidth = selectionBoundingWidth
                    startBoundingHeight = selectionBoundingHeight
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        
                        // Calculate raw delta
                        var rawDeltaX = canvasPos.x - startCanvasX
                        var rawDeltaY = canvasPos.y - startCanvasY
                        
                        // If controls are rotated, transform the delta into control space
                        var deltaY = rawDeltaY
                        if (controlsContainer.controlsRotation !== 0) {
                            var angle = -controlsContainer.controlsRotation * Math.PI / 180
                            var cos = Math.cos(angle)
                            var sin = Math.sin(angle)
                            
                            // Transform mouse movement into control's local coordinate system
                            deltaY = rawDeltaX * sin + rawDeltaY * cos
                        }
                        
                        if (selectedElements.length === 1) {
                            // Single selection - direct manipulation
                            var element = selectedElements[0]
                            var startState = startElementStates[0]
                            
                            // Calculate new top edge position
                            var newTop = startState.y + deltaY
                            var bottom = startState.y + startState.height
                            
                            // Ensure minimum height and handle flipping
                            if (newTop + 1 > bottom) {
                                // Flip: top edge is being dragged past bottom edge
                                element.y = bottom - 1
                                element.height = 1 + (newTop - bottom)
                            } else {
                                element.y = newTop
                                element.height = bottom - newTop
                            }
                            
                            // If controls are rotated, we need to compensate for the anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // After resize, the controls have moved. Calculate where our anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                // Get the new control center position
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                // Calculate where the bottom edge center is now
                                var localAnchorX = controlsContainer.width / 2
                                var localAnchorY = controlsContainer.height
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                // Calculate the offset needed to restore the anchor to its original position
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                // Convert viewport offset to canvas offset
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply the offset to maintain anchor position
                                element.x += canvasOffsetX
                                element.y += canvasOffsetY
                            }
                        } else {
                            // Multiple selection - proportional scaling with flipping
                            var newBoundingTop = startBoundingY + deltaY
                            var boundingBottom = startBoundingY + startBoundingHeight
                            
                            // Check if we need to flip (top edge dragged past bottom edge)
                            var isFlipped = newBoundingTop > boundingBottom - 1
                            
                            if (!isFlipped) {
                                // Normal resize - maintain bottom edge
                                var scaleFactor = (boundingBottom - newBoundingTop) / startBoundingHeight
                                
                                // Apply proportional changes to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from bottom edge
                                    var relativeFromBottom = (startBoundingY + startBoundingHeight) - (state.y + state.height)
                                    
                                    // New height scaled proportionally
                                    elem.height = state.height * scaleFactor
                                    
                                    // Position calculated from the bottom edge, scaled properly
                                    elem.y = boundingBottom - relativeFromBottom * scaleFactor - elem.height
                                }
                            } else {
                                // Flipped state - elements flip around the original bottom edge
                                var flippedHeight = newBoundingTop - boundingBottom
                                var scaleFactor = flippedHeight / startBoundingHeight
                                
                                // Apply flipped transformation to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from top edge (since we're flipping)
                                    var relativeFromTop = state.y - startBoundingY
                                    
                                    // New height scaled proportionally
                                    elem.height = state.height * scaleFactor
                                    
                                    // Position calculated from the flipped position
                                    // Elements that were on the top are now on the bottom
                                    elem.y = boundingBottom + (startBoundingHeight - relativeFromTop - state.height) * scaleFactor
                                }
                            }
                            
                            // If controls are rotated, compensate for anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // Calculate where the anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                var localAnchorX = controlsContainer.width / 2
                                var localAnchorY = controlsContainer.height
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply offset to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    selectedElements[i].x += canvasOffsetX
                                    selectedElements[i].y += canvasOffsetY
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Bottom bar
        Rectangle {
            x: 0
            y: parent.height - Config.controlBarHeight/2
            width: parent.width
            height: Config.controlBarHeight
            color: Config.controlBarColor
            
            // Center line
            Rectangle {
                anchors.centerIn: parent
                width: parent.width
                height: Config.controlLineWidth
                color: Config.controlBarColor
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                preventStealing: true
                
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
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    startCanvasY = canvasPos.y
                    startRotation = controlsContainer.controlsRotation
                    
                    // Calculate the anchor point (top edge center) in world coordinates
                    var angle = controlsContainer.controlsRotation * Math.PI / 180
                    var cos = Math.cos(angle)
                    var sin = Math.sin(angle)
                    
                    // Top edge center in local control coordinates
                    var localAnchorX = controlsContainer.width / 2
                    var localAnchorY = 0
                    
                    // Transform to world coordinates
                    var centerX = controlsContainer.x + controlsContainer.width / 2
                    var centerY = controlsContainer.y + controlsContainer.height / 2
                    
                    // Rotate around center
                    var relX = localAnchorX - controlsContainer.width / 2
                    var relY = localAnchorY - controlsContainer.height / 2
                    
                    anchorWorldX = centerX + relX * cos - relY * sin
                    anchorWorldY = centerY + relX * sin + relY * cos
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
                            y: selectedElements[i].y,
                            height: selectedElements[i].height
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingX = selectionBoundingX
                    startBoundingY = selectionBoundingY
                    startBoundingWidth = selectionBoundingWidth
                    startBoundingHeight = selectionBoundingHeight
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        
                        // Calculate raw delta
                        var rawDeltaX = canvasPos.x - startCanvasX
                        var rawDeltaY = canvasPos.y - startCanvasY
                        
                        // If controls are rotated, transform the delta into control space
                        var deltaY = rawDeltaY
                        if (controlsContainer.controlsRotation !== 0) {
                            var angle = -controlsContainer.controlsRotation * Math.PI / 180
                            var cos = Math.cos(angle)
                            var sin = Math.sin(angle)
                            
                            // Transform mouse movement into control's local coordinate system
                            deltaY = rawDeltaX * sin + rawDeltaY * cos
                        }
                        
                        if (selectedElements.length === 1) {
                            // Single selection - direct manipulation
                            var element = selectedElements[0]
                            var startState = startElementStates[0]
                            
                            // Calculate new bottom edge position
                            var top = startState.y
                            var newBottom = startState.y + startState.height + deltaY
                            
                            // Ensure minimum height and handle flipping
                            if (newBottom < top + 1) {
                                // Flip: bottom edge is being dragged past top edge
                                element.y = newBottom
                                element.height = 1 + (top - newBottom)
                            } else {
                                element.y = top
                                element.height = newBottom - top
                            }
                            
                            // If controls are rotated, we need to compensate for the anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // After resize, the controls have moved. Calculate where our anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                // Get the new control center position
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                // Calculate where the top edge center is now
                                var localAnchorX = controlsContainer.width / 2
                                var localAnchorY = 0
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                // Calculate the offset needed to restore the anchor to its original position
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                // Convert viewport offset to canvas offset
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply the offset to maintain anchor position
                                element.x += canvasOffsetX
                                element.y += canvasOffsetY
                            }
                        } else {
                            // Multiple selection - proportional scaling with flipping
                            var boundingTop = startBoundingY
                            var newBoundingBottom = startBoundingY + startBoundingHeight + deltaY
                            
                            // Check if we need to flip (bottom edge dragged past top edge)
                            var isFlipped = newBoundingBottom < boundingTop + 1
                            
                            if (!isFlipped) {
                                // Normal resize - maintain top edge
                                var scaleFactor = (newBoundingBottom - boundingTop) / startBoundingHeight
                                
                                // Apply proportional changes to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from top edge
                                    var relativeTop = state.y - startBoundingY
                                    
                                    // Position remains relative to top edge
                                    elem.y = boundingTop + relativeTop * scaleFactor
                                    
                                    // Height scaled proportionally
                                    elem.height = state.height * scaleFactor
                                }
                            } else {
                                // Flipped state - elements flip around the original top edge
                                var flippedHeight = boundingTop - newBoundingBottom
                                var scaleFactor = flippedHeight / startBoundingHeight
                                
                                // Apply flipped transformation to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from bottom edge (since we're flipping)
                                    var relativeFromBottom = (startBoundingY + startBoundingHeight) - (state.y + state.height)
                                    
                                    // New height scaled proportionally
                                    elem.height = state.height * scaleFactor
                                    
                                    // Position calculated from the flipped position
                                    // Elements that were on the bottom are now on the top
                                    elem.y = newBoundingBottom + relativeFromBottom * scaleFactor
                                }
                            }
                            
                            // If controls are rotated, compensate for anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // Calculate where the anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                var localAnchorX = controlsContainer.width / 2
                                var localAnchorY = 0
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply offset to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    selectedElements[i].x += canvasOffsetX
                                    selectedElements[i].y += canvasOffsetY
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Left bar
        Rectangle {
            x: -Config.controlBarWidth/2
            y: 0
            width: Config.controlBarWidth
            height: parent.height
            color: Config.controlBarColor
            
            // Center line
            Rectangle {
                anchors.centerIn: parent
                width: Config.controlLineWidth
                height: parent.height
                color: Config.controlBarColor
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                
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
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    startCanvasY = canvasPos.y
                    startRotation = controlsContainer.controlsRotation
                    
                    // Calculate the anchor point (right edge center) in world coordinates
                    var angle = controlsContainer.controlsRotation * Math.PI / 180
                    var cos = Math.cos(angle)
                    var sin = Math.sin(angle)
                    
                    // Right edge center in local control coordinates
                    var localAnchorX = controlsContainer.width
                    var localAnchorY = controlsContainer.height / 2
                    
                    // Transform to world coordinates
                    var centerX = controlsContainer.x + controlsContainer.width / 2
                    var centerY = controlsContainer.y + controlsContainer.height / 2
                    
                    // Rotate around center
                    var relX = localAnchorX - controlsContainer.width / 2
                    var relY = localAnchorY - controlsContainer.height / 2
                    
                    anchorWorldX = centerX + relX * cos - relY * sin
                    anchorWorldY = centerY + relX * sin + relY * cos
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
                            y: selectedElements[i].y,
                            width: selectedElements[i].width
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingX = selectionBoundingX
                    startBoundingWidth = selectionBoundingWidth
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        
                        // Calculate raw delta
                        var rawDeltaX = canvasPos.x - startCanvasX
                        var rawDeltaY = canvasPos.y - startCanvasY
                        
                        // If controls are rotated, transform the delta into control space
                        var deltaX = rawDeltaX
                        if (controlsContainer.controlsRotation !== 0) {
                            var angle = -controlsContainer.controlsRotation * Math.PI / 180
                            var cos = Math.cos(angle)
                            var sin = Math.sin(angle)
                            
                            // Transform mouse movement into control's local coordinate system
                            deltaX = rawDeltaX * cos - rawDeltaY * sin
                        }
                        
                        if (selectedElements.length === 1) {
                            // Single selection - direct manipulation
                            var element = selectedElements[0]
                            var startState = startElementStates[0]
                            
                            // Calculate new left edge position
                            var newLeft = startState.x + deltaX
                            var right = startState.x + startState.width
                            
                            // Ensure minimum width and handle flipping
                            if (newLeft + 1 > right) {
                                // Flip: left edge is being dragged past right edge
                                element.x = right - 1
                                element.width = 1 + (newLeft - right)
                            } else {
                                element.x = newLeft
                                element.width = right - newLeft
                            }
                            
                            // If controls are rotated, we need to compensate for the anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // After resize, the controls have moved. Calculate where our anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                // Get the new control center position
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                // Calculate where the right edge center is now
                                var localAnchorX = controlsContainer.width
                                var localAnchorY = controlsContainer.height / 2
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                // Calculate the offset needed to restore the anchor to its original position
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                // Convert viewport offset to canvas offset
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply the offset to maintain anchor position
                                element.x += canvasOffsetX
                                element.y += canvasOffsetY
                            }
                        } else {
                            // Multiple selection - proportional scaling with flipping
                            var newBoundingLeft = startBoundingX + deltaX
                            var boundingRight = startBoundingX + startBoundingWidth
                            
                            // Check if we need to flip (left edge dragged past right edge)
                            var isFlipped = newBoundingLeft > boundingRight - 1
                            
                            if (!isFlipped) {
                                // Normal resize - maintain right edge
                                var scaleFactor = (boundingRight - newBoundingLeft) / startBoundingWidth
                                
                                // Apply proportional changes to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from right edge
                                    var relativeFromRight = (startBoundingX + startBoundingWidth) - (state.x + state.width)
                                    
                                    // New width scaled proportionally
                                    elem.width = state.width * scaleFactor
                                    
                                    // Position calculated from the right edge, scaled properly
                                    elem.x = boundingRight - relativeFromRight * scaleFactor - elem.width
                                }
                            } else {
                                // Flipped state - elements flip around the original right edge
                                var flippedWidth = newBoundingLeft - boundingRight
                                var scaleFactor = flippedWidth / startBoundingWidth
                                
                                // Apply flipped transformation to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from left edge (since we're flipping)
                                    var relativeFromLeft = state.x - startBoundingX
                                    
                                    // New width scaled proportionally
                                    elem.width = state.width * scaleFactor
                                    
                                    // Position calculated from the flipped position
                                    // Elements that were on the left are now on the right
                                    elem.x = boundingRight + (startBoundingWidth - relativeFromLeft - state.width) * scaleFactor
                                }
                            }
                            
                            // If controls are rotated, compensate for anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // Calculate where the anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                var localAnchorX = controlsContainer.width
                                var localAnchorY = controlsContainer.height / 2
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply offset to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    selectedElements[i].x += canvasOffsetX
                                    selectedElements[i].y += canvasOffsetY
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Right bar
        Rectangle {
            x: parent.width - Config.controlBarWidth/2
            y: 0
            width: Config.controlBarWidth
            height: parent.height
            color: Config.controlBarColor
            
            // Center line
            Rectangle {
                anchors.centerIn: parent
                width: Config.controlLineWidth
                height: parent.height
                color: Config.controlBarColor
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                
                property real startCanvasX: 0
                property real startCanvasY: 0
                property var startElementStates: []
                property real startBoundingX: 0
                property real startBoundingWidth: 0
                property real startRotation: 0
                property real anchorWorldX: 0
                property real anchorWorldY: 0
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    startCanvasY = canvasPos.y
                    startRotation = controlsContainer.controlsRotation
                    
                    // Calculate the anchor point (left edge center) in world coordinates
                    var angle = controlsContainer.controlsRotation * Math.PI / 180
                    var cos = Math.cos(angle)
                    var sin = Math.sin(angle)
                    
                    // Left edge center in local control coordinates
                    var localAnchorX = 0
                    var localAnchorY = controlsContainer.height / 2
                    
                    // Transform to world coordinates
                    var centerX = controlsContainer.x + controlsContainer.width / 2
                    var centerY = controlsContainer.y + controlsContainer.height / 2
                    
                    // Rotate around center
                    var relX = localAnchorX - controlsContainer.width / 2
                    var relY = localAnchorY - controlsContainer.height / 2
                    
                    anchorWorldX = centerX + relX * cos - relY * sin
                    anchorWorldY = centerY + relX * sin + relY * cos
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
                            y: selectedElements[i].y,
                            width: selectedElements[i].width
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingX = selectionBoundingX
                    startBoundingWidth = selectionBoundingWidth
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        
                        // Calculate raw delta
                        var rawDeltaX = canvasPos.x - startCanvasX
                        var rawDeltaY = canvasPos.y - startCanvasY
                        
                        // If controls are rotated, transform the delta into control space
                        var deltaX = rawDeltaX
                        if (controlsContainer.controlsRotation !== 0) {
                            var angle = -controlsContainer.controlsRotation * Math.PI / 180
                            var cos = Math.cos(angle)
                            var sin = Math.sin(angle)
                            
                            // Transform mouse movement into control's local coordinate system
                            deltaX = rawDeltaX * cos - rawDeltaY * sin
                        }
                        
                        if (selectedElements.length === 1) {
                            // Single selection - direct manipulation
                            var element = selectedElements[0]
                            var startState = startElementStates[0]
                            
                            // First, apply the resize normally
                            var left = startState.x
                            var newRight = startState.x + startState.width + deltaX
                            
                            // Ensure minimum width and handle flipping
                            if (newRight < left + 1) {
                                // Flip: right edge is being dragged past left edge
                                element.x = newRight
                                element.width = 1 + (left - newRight)
                            } else {
                                element.x = left
                                element.width = newRight - left
                            }
                            
                            // If controls are rotated, we need to compensate for the anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // After resize, the controls have moved. Calculate where our anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                // Get the new control center position
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                // Calculate where the left edge center is now
                                var localAnchorX = 0
                                var localAnchorY = controlsContainer.height / 2
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                // Calculate the offset needed to restore the anchor to its original position
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                // Convert viewport offset to canvas offset
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply the offset to maintain anchor position
                                element.x += canvasOffsetX
                                element.y += canvasOffsetY
                            }
                        } else {
                            // Multiple selection - proportional scaling with flipping
                            var boundingLeft = startBoundingX
                            var newBoundingRight = startBoundingX + startBoundingWidth + deltaX
                            
                            // Check if we need to flip (right edge dragged past left edge)
                            var isFlipped = newBoundingRight < boundingLeft + 1
                            
                            if (!isFlipped) {
                                // Normal resize - maintain left edge
                                var scaleFactor = (newBoundingRight - boundingLeft) / startBoundingWidth
                                
                                // Apply proportional changes to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from left edge
                                    var relativeLeft = state.x - startBoundingX
                                    
                                    // Position remains relative to left edge
                                    elem.x = boundingLeft + relativeLeft * scaleFactor
                                    
                                    // Width scaled proportionally
                                    elem.width = state.width * scaleFactor
                                }
                            } else {
                                // Flipped state - elements flip around the original left edge
                                var flippedWidth = boundingLeft - newBoundingRight
                                var scaleFactor = flippedWidth / startBoundingWidth
                                
                                // Apply flipped transformation to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    var elem = selectedElements[i]
                                    var state = startElementStates[i]
                                    
                                    // Calculate relative position from right edge (since we're flipping)
                                    var relativeFromRight = (startBoundingX + startBoundingWidth) - (state.x + state.width)
                                    
                                    // New width scaled proportionally
                                    elem.width = state.width * scaleFactor
                                    
                                    // Position calculated from the flipped position
                                    // Elements that were on the right are now on the left
                                    elem.x = newBoundingRight + relativeFromRight * scaleFactor
                                }
                            }
                            
                            // If controls are rotated, compensate for anchor movement
                            if (controlsContainer.controlsRotation !== 0 && startRotation !== 0) {
                                // Calculate where the anchor point is now
                                var angle = controlsContainer.controlsRotation * Math.PI / 180
                                var cos = Math.cos(angle)
                                var sin = Math.sin(angle)
                                
                                var newCenterX = controlsContainer.x + controlsContainer.width / 2
                                var newCenterY = controlsContainer.y + controlsContainer.height / 2
                                
                                var localAnchorX = 0
                                var localAnchorY = controlsContainer.height / 2
                                var relX = localAnchorX - controlsContainer.width / 2
                                var relY = localAnchorY - controlsContainer.height / 2
                                
                                var currentAnchorX = newCenterX + relX * cos - relY * sin
                                var currentAnchorY = newCenterY + relX * sin + relY * cos
                                
                                var offsetX = anchorWorldX - currentAnchorX
                                var offsetY = anchorWorldY - currentAnchorY
                                
                                var canvasOffsetX = offsetX / zoomLevel
                                var canvasOffsetY = offsetY / zoomLevel
                                
                                // Apply offset to all elements
                                for (var i = 0; i < selectedElements.length; i++) {
                                    selectedElements[i].x += canvasOffsetX
                                    selectedElements[i].y += canvasOffsetY
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Joints container
        Item {
            anchors.fill: parent
            
            // Rotation joints (blue, larger) - positioned with overlap
            Repeater {
                model: 4
                Rectangle {
                    width: Config.controlRotationJointSize
                    height: Config.controlRotationJointSize
                    color: Config.controlRotationJointColor
                    
                    x: {
                        var overlap = Config.controlJointOverlap
                        switch(index) {
                            case 0: return -Config.controlBarWidth/2 - (Config.controlRotationJointSize - overlap) // top-left
                            case 1: return parent.width - Config.controlBarWidth/2 // top-right
                            case 2: return parent.width - Config.controlBarWidth/2 // bottom-right
                            case 3: return -Config.controlBarWidth/2 - (Config.controlRotationJointSize - overlap) // bottom-left
                        }
                    }
                    
                    y: {
                        var overlap = Config.controlJointOverlap
                        switch(index) {
                            case 0: return -Config.controlBarHeight/2 - (Config.controlRotationJointSize - overlap) // top-left
                            case 1: return -Config.controlBarHeight/2 - (Config.controlRotationJointSize - overlap) // top-right
                            case 2: return parent.height - Config.controlBarHeight/2 // bottom-right
                            case 3: return parent.height - Config.controlBarHeight/2 // bottom-left
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        preventStealing: true
                        
                        property real jointAngleOffset: 0
                        property real startControlsRotation: 0
                        
                        onPressed: function(mouse) {
                            // Store the starting rotation
                            startControlsRotation = controlsContainer.controlsRotation
                            
                            // Get the center of the controls in viewport coordinates
                            var centerX = controlsContainer.x + controlsContainer.width / 2
                            var centerY = controlsContainer.y + controlsContainer.height / 2
                            
                            // Get mouse position in viewport coordinates
                            var mouseGlobal = mapToItem(root, mouse.x, mouse.y)
                            
                            // Calculate angle from center to mouse
                            var dx = mouseGlobal.x - centerX
                            var dy = mouseGlobal.y - centerY
                            var mouseAngle = Math.atan2(dy, dx) * 180 / Math.PI
                            
                            // Store the angle offset to maintain the joint's position under the mouse
                            jointAngleOffset = mouseAngle - startControlsRotation
                        }
                        
                        onPositionChanged: function(mouse) {
                            if (!pressed) return
                            
                            // Get the center of the controls in viewport coordinates
                            var centerX = controlsContainer.x + controlsContainer.width / 2
                            var centerY = controlsContainer.y + controlsContainer.height / 2
                            
                            // Get mouse position in viewport coordinates
                            var mouseGlobal = mapToItem(root, mouse.x, mouse.y)
                            
                            // Calculate current angle from center to mouse
                            var dx = mouseGlobal.x - centerX
                            var dy = mouseGlobal.y - centerY
                            var currentMouseAngle = Math.atan2(dy, dx) * 180 / Math.PI
                            
                            // Set rotation to maintain the offset, keeping the joint under the mouse
                            controlsContainer.controlsRotation = currentMouseAngle - jointAngleOffset
                        }
                    }
                }
            }
            
            // Resize joints (yellow, smaller) - positioned at bar intersections
            Repeater {
                model: 4
                Rectangle {
                    width: Config.controlResizeJointSize
                    height: Config.controlResizeJointSize
                    color: Config.controlResizeJointColor
                    
                    property string position: {
                        switch(index) {
                            case 0: return "top-left"
                            case 1: return "top-right"
                            case 2: return "bottom-right"
                            case 3: return "bottom-left"
                        }
                    }
                    
                    x: {
                        switch(index) {
                            case 0: return -Config.controlBarWidth/2 // top-left
                            case 1: return parent.width - Config.controlBarWidth/2 // top-right
                            case 2: return parent.width - Config.controlBarWidth/2 // bottom-right
                            case 3: return -Config.controlBarWidth/2 // bottom-left
                        }
                    }
                    
                    y: {
                        switch(index) {
                            case 0: return -Config.controlBarHeight/2 // top-left
                            case 1: return -Config.controlBarHeight/2 // top-right
                            case 2: return parent.height - Config.controlBarHeight/2 // bottom-right
                            case 3: return parent.height - Config.controlBarHeight/2 // bottom-left
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: {
                            switch(parent.position) {
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
                        preventStealing: true
                        
                        property real startCanvasX: 0
                        property real startCanvasY: 0
                        property var startElementStates: []
                        property real startBoundingX: 0
                        property real startBoundingY: 0
                        property real startBoundingWidth: 0
                        property real startBoundingHeight: 0
                        
                        onPressed: function(mouse) {
                            var globalPos = mapToItem(root, mouse.x, mouse.y)
                            var canvasPos = parent.parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                            startCanvasX = canvasPos.x
                            startCanvasY = canvasPos.y
                            
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
                            
                            // Store bounding box for proportional scaling
                            startBoundingX = selectionBoundingX
                            startBoundingY = selectionBoundingY
                            startBoundingWidth = selectionBoundingWidth
                            startBoundingHeight = selectionBoundingHeight
                        }
                        
                        onPositionChanged: function(mouse) {
                            if (!pressed) return
                            
                            var globalPos = mapToItem(root, mouse.x, mouse.y)
                            var canvasPos = parent.parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                            var deltaX = canvasPos.x - startCanvasX
                            var deltaY = canvasPos.y - startCanvasY
                            
                            if (selectedElements.length === 1) {
                                // Single selection - direct manipulation
                                var element = selectedElements[0]
                                var startState = startElementStates[0]
                                
                                switch(parent.position) {
                                    case "top-left":
                                        // Move top-left corner
                                        var newLeft = startState.x + deltaX
                                        var newTop = startState.y + deltaY
                                        var right = startState.x + startState.width
                                        var bottom = startState.y + startState.height
                                        
                                        // Handle horizontal flipping
                                        if (newLeft + 1 > right) {
                                            element.x = right - 1
                                            element.width = 1 + (newLeft - right)
                                        } else {
                                            element.x = newLeft
                                            element.width = right - newLeft
                                        }
                                        
                                        // Handle vertical flipping
                                        if (newTop + 1 > bottom) {
                                            element.y = bottom - 1
                                            element.height = 1 + (newTop - bottom)
                                        } else {
                                            element.y = newTop
                                            element.height = bottom - newTop
                                        }
                                        break
                                        
                                    case "top-right":
                                        // Move top-right corner
                                        var left = startState.x
                                        var newTop = startState.y + deltaY
                                        var newRight = startState.x + startState.width + deltaX
                                        var bottom = startState.y + startState.height
                                        
                                        // Handle horizontal flipping
                                        if (newRight < left + 1) {
                                            element.x = newRight
                                            element.width = 1 + (left - newRight)
                                        } else {
                                            element.x = left
                                            element.width = newRight - left
                                        }
                                        
                                        // Handle vertical flipping
                                        if (newTop + 1 > bottom) {
                                            element.y = bottom - 1
                                            element.height = 1 + (newTop - bottom)
                                        } else {
                                            element.y = newTop
                                            element.height = bottom - newTop
                                        }
                                        break
                                        
                                    case "bottom-right":
                                        // Move bottom-right corner
                                        var left = startState.x
                                        var top = startState.y
                                        var newRight = startState.x + startState.width + deltaX
                                        var newBottom = startState.y + startState.height + deltaY
                                        
                                        // Handle horizontal flipping
                                        if (newRight < left + 1) {
                                            element.x = newRight
                                            element.width = 1 + (left - newRight)
                                        } else {
                                            element.x = left
                                            element.width = newRight - left
                                        }
                                        
                                        // Handle vertical flipping
                                        if (newBottom < top + 1) {
                                            element.y = newBottom
                                            element.height = 1 + (top - newBottom)
                                        } else {
                                            element.y = top
                                            element.height = newBottom - top
                                        }
                                        break
                                        
                                    case "bottom-left":
                                        // Move bottom-left corner
                                        var newLeft = startState.x + deltaX
                                        var top = startState.y
                                        var right = startState.x + startState.width
                                        var newBottom = startState.y + startState.height + deltaY
                                        
                                        // Handle horizontal flipping
                                        if (newLeft + 1 > right) {
                                            element.x = right - 1
                                            element.width = 1 + (newLeft - right)
                                        } else {
                                            element.x = newLeft
                                            element.width = right - newLeft
                                        }
                                        
                                        // Handle vertical flipping
                                        if (newBottom < top + 1) {
                                            element.y = newBottom
                                            element.height = 1 + (top - newBottom)
                                        } else {
                                            element.y = top
                                            element.height = newBottom - top
                                        }
                                        break
                                }
                            } else {
                                // Multiple selection - proportional scaling
                                // Calculate scale factors based on corner movement
                                var scaleX = 1.0
                                var scaleY = 1.0
                                var newBoundingX = startBoundingX
                                var newBoundingY = startBoundingY
                                
                                switch(parent.position) {
                                    case "top-left":
                                        // Scale from bottom-right anchor
                                        var newLeft = startBoundingX + deltaX
                                        var newTop = startBoundingY + deltaY
                                        var anchorX = startBoundingX + startBoundingWidth
                                        var anchorY = startBoundingY + startBoundingHeight
                                        
                                        // Calculate scales (with flip handling)
                                        if (newLeft >= anchorX - 1) {
                                            scaleX = (newLeft - anchorX + 1) / startBoundingWidth
                                            newBoundingX = anchorX - 1
                                        } else {
                                            scaleX = (anchorX - newLeft) / startBoundingWidth
                                            newBoundingX = newLeft
                                        }
                                        
                                        if (newTop >= anchorY - 1) {
                                            scaleY = (newTop - anchorY + 1) / startBoundingHeight
                                            newBoundingY = anchorY - 1
                                        } else {
                                            scaleY = (anchorY - newTop) / startBoundingHeight
                                            newBoundingY = newTop
                                        }
                                        
                                        // Apply scaling to all elements
                                        for (var i = 0; i < selectedElements.length; i++) {
                                            var elem = selectedElements[i]
                                            var state = startElementStates[i]
                                            
                                            // Calculate distance from anchor point
                                            var distX = anchorX - (state.x + state.width)
                                            var distY = anchorY - (state.y + state.height)
                                            
                                            elem.width = state.width * Math.abs(scaleX)
                                            elem.height = state.height * Math.abs(scaleY)
                                            
                                            if (newLeft >= anchorX - 1) {
                                                // Flipped horizontally
                                                elem.x = anchorX + distX * Math.abs(scaleX)
                                            } else {
                                                elem.x = anchorX - distX * Math.abs(scaleX) - elem.width
                                            }
                                            
                                            if (newTop >= anchorY - 1) {
                                                // Flipped vertically
                                                elem.y = anchorY + distY * Math.abs(scaleY)
                                            } else {
                                                elem.y = anchorY - distY * Math.abs(scaleY) - elem.height
                                            }
                                        }
                                        break
                                        
                                    case "top-right":
                                        // Scale from bottom-left anchor
                                        var newRight = startBoundingX + startBoundingWidth + deltaX
                                        var newTop = startBoundingY + deltaY
                                        var anchorX = startBoundingX
                                        var anchorY = startBoundingY + startBoundingHeight
                                        
                                        // Calculate scales (with flip handling)
                                        if (newRight <= anchorX + 1) {
                                            scaleX = (anchorX - newRight + 1) / startBoundingWidth
                                            newBoundingX = newRight
                                        } else {
                                            scaleX = (newRight - anchorX) / startBoundingWidth
                                            newBoundingX = anchorX
                                        }
                                        
                                        if (newTop >= anchorY - 1) {
                                            scaleY = (newTop - anchorY + 1) / startBoundingHeight
                                            newBoundingY = anchorY - 1
                                        } else {
                                            scaleY = (anchorY - newTop) / startBoundingHeight
                                            newBoundingY = newTop
                                        }
                                        
                                        // Apply scaling to all elements
                                        for (var i = 0; i < selectedElements.length; i++) {
                                            var elem = selectedElements[i]
                                            var state = startElementStates[i]
                                            
                                            // Calculate distance from anchor point
                                            var distX = state.x - anchorX
                                            var distY = anchorY - (state.y + state.height)
                                            
                                            elem.width = state.width * Math.abs(scaleX)
                                            elem.height = state.height * Math.abs(scaleY)
                                            
                                            if (newRight <= anchorX + 1) {
                                                // Flipped horizontally
                                                elem.x = anchorX - distX * Math.abs(scaleX) - elem.width
                                            } else {
                                                elem.x = anchorX + distX * Math.abs(scaleX)
                                            }
                                            
                                            if (newTop >= anchorY - 1) {
                                                // Flipped vertically
                                                elem.y = anchorY + distY * Math.abs(scaleY)
                                            } else {
                                                elem.y = anchorY - distY * Math.abs(scaleY) - elem.height
                                            }
                                        }
                                        break
                                        
                                    case "bottom-right":
                                        // Scale from top-left anchor
                                        var newRight = startBoundingX + startBoundingWidth + deltaX
                                        var newBottom = startBoundingY + startBoundingHeight + deltaY
                                        var anchorX = startBoundingX
                                        var anchorY = startBoundingY
                                        
                                        // Calculate scales (with flip handling)
                                        if (newRight <= anchorX + 1) {
                                            scaleX = (anchorX - newRight + 1) / startBoundingWidth
                                            newBoundingX = newRight
                                        } else {
                                            scaleX = (newRight - anchorX) / startBoundingWidth
                                            newBoundingX = anchorX
                                        }
                                        
                                        if (newBottom <= anchorY + 1) {
                                            scaleY = (anchorY - newBottom + 1) / startBoundingHeight
                                            newBoundingY = newBottom
                                        } else {
                                            scaleY = (newBottom - anchorY) / startBoundingHeight
                                            newBoundingY = anchorY
                                        }
                                        
                                        // Apply scaling to all elements
                                        for (var i = 0; i < selectedElements.length; i++) {
                                            var elem = selectedElements[i]
                                            var state = startElementStates[i]
                                            
                                            // Calculate distance from anchor point
                                            var distX = state.x - anchorX
                                            var distY = state.y - anchorY
                                            
                                            elem.width = state.width * Math.abs(scaleX)
                                            elem.height = state.height * Math.abs(scaleY)
                                            
                                            if (newRight <= anchorX + 1) {
                                                // Flipped horizontally
                                                elem.x = anchorX - distX * Math.abs(scaleX) - elem.width
                                            } else {
                                                elem.x = anchorX + distX * Math.abs(scaleX)
                                            }
                                            
                                            if (newBottom <= anchorY + 1) {
                                                // Flipped vertically
                                                elem.y = anchorY - distY * Math.abs(scaleY) - elem.height
                                            } else {
                                                elem.y = anchorY + distY * Math.abs(scaleY)
                                            }
                                        }
                                        break
                                        
                                    case "bottom-left":
                                        // Scale from top-right anchor
                                        var newLeft = startBoundingX + deltaX
                                        var newBottom = startBoundingY + startBoundingHeight + deltaY
                                        var anchorX = startBoundingX + startBoundingWidth
                                        var anchorY = startBoundingY
                                        
                                        // Calculate scales (with flip handling)
                                        if (newLeft >= anchorX - 1) {
                                            scaleX = (newLeft - anchorX + 1) / startBoundingWidth
                                            newBoundingX = anchorX - 1
                                        } else {
                                            scaleX = (anchorX - newLeft) / startBoundingWidth
                                            newBoundingX = newLeft
                                        }
                                        
                                        if (newBottom <= anchorY + 1) {
                                            scaleY = (anchorY - newBottom + 1) / startBoundingHeight
                                            newBoundingY = newBottom
                                        } else {
                                            scaleY = (newBottom - anchorY) / startBoundingHeight
                                            newBoundingY = anchorY
                                        }
                                        
                                        // Apply scaling to all elements
                                        for (var i = 0; i < selectedElements.length; i++) {
                                            var elem = selectedElements[i]
                                            var state = startElementStates[i]
                                            
                                            // Calculate distance from anchor point
                                            var distX = anchorX - (state.x + state.width)
                                            var distY = state.y - anchorY
                                            
                                            elem.width = state.width * Math.abs(scaleX)
                                            elem.height = state.height * Math.abs(scaleY)
                                            
                                            if (newLeft >= anchorX - 1) {
                                                // Flipped horizontally
                                                elem.x = anchorX + distX * Math.abs(scaleX)
                                            } else {
                                                elem.x = anchorX - distX * Math.abs(scaleX) - elem.width
                                            }
                                            
                                            if (newBottom <= anchorY + 1) {
                                                // Flipped vertically
                                                elem.y = anchorY - distY * Math.abs(scaleY) - elem.height
                                            } else {
                                                elem.y = anchorY + distY * Math.abs(scaleY)
                                            }
                                        }
                                        break
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Hover indicator
    Rectangle {
        id: hoverIndicator
        visible: hoveredElement !== null && 
                (!canvasView.selectionManager || 
                 hoveredElement && !hoveredElement.selected)
        color: "transparent"
        border.color: Config.hoverColor
        border.width: 1
        
        x: hoveredElement ? (hoveredElement.x - root.canvasMinX) * zoomLevel - flickable.contentX : 0
        y: hoveredElement ? (hoveredElement.y - root.canvasMinY) * zoomLevel - flickable.contentY : 0
        width: hoveredElement ? hoveredElement.width * zoomLevel : 0
        height: hoveredElement ? hoveredElement.height * zoomLevel : 0
    }
}