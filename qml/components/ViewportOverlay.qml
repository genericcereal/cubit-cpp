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
        visible: selectedElements.length > 0
        
        // Always use bounding box calculations for consistency
        property real controlX: (selectionBoundingX - root.canvasMinX) * zoomLevel - (flickable ? flickable.contentX : 0)
        property real controlY: (selectionBoundingY - root.canvasMinY) * zoomLevel - (flickable ? flickable.contentY : 0)
        property real controlWidth: selectionBoundingWidth * zoomLevel
        property real controlHeight: selectionBoundingHeight * zoomLevel
        
        x: controlX
        y: controlY
        width: controlWidth
        height: controlHeight
        
        
        
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
                
                property real startCanvasY: 0
                property var startElementStates: []
                property real startBoundingY: 0
                property real startBoundingHeight: 0
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasY = canvasPos.y
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            y: selectedElements[i].y,
                            height: selectedElements[i].height
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingY = selectionBoundingY
                    startBoundingHeight = selectionBoundingHeight
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        var deltaY = canvasPos.y - startCanvasY
                        
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
                                selectedElements[0].y = bottom - 1
                                selectedElements[0].height = 1 + (newTop - bottom)
                            } else {
                                selectedElements[0].y = newTop
                                selectedElements[0].height = bottom - newTop
                            }
                            console.log("Top bar resize: element y =", selectedElements[0].y, "height =", selectedElements[0].height)
                        } else {
                            // Multiple selection - proportional scaling
                            var newBoundingTop = startBoundingY + deltaY
                            var boundingBottom = startBoundingY + startBoundingHeight
                            
                            // Calculate scale factor (maintain bottom edge)
                            var scaleFactor = 1.0
                            if (newBoundingTop < boundingBottom - 1) {
                                scaleFactor = (boundingBottom - newBoundingTop) / startBoundingHeight
                            } else {
                                // Minimum height constraint
                                newBoundingTop = boundingBottom - 1
                                scaleFactor = 1 / startBoundingHeight
                            }
                            
                            // Apply proportional changes to all elements
                            for (var i = 0; i < selectedElements.length; i++) {
                                var elem = selectedElements[i]
                                var state = startElementStates[i]
                                
                                // Calculate relative position from bottom edge
                                var relativeFromBottom = (startBoundingY + startBoundingHeight) - (state.y + state.height)
                                
                                // New height scaled proportionally
                                elem.height = state.height * scaleFactor
                                
                                // Position calculated from the new bottom edge, scaled properly
                                elem.y = boundingBottom - relativeFromBottom * scaleFactor - elem.height
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
                
                property real startCanvasY: 0
                property var startElementStates: []
                property real startBoundingY: 0
                property real startBoundingHeight: 0
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasY = canvasPos.y
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            y: selectedElements[i].y,
                            height: selectedElements[i].height
                        })
                    }
                    
                    // Store bounding box for proportional scaling
                    startBoundingY = selectionBoundingY
                    startBoundingHeight = selectionBoundingHeight
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var globalPos = mapToItem(root, mouse.x, mouse.y)
                        var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                        var deltaY = canvasPos.y - startCanvasY
                        
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
                        } else {
                            // Multiple selection - proportional scaling
                            var boundingTop = startBoundingY
                            var newBoundingBottom = startBoundingY + startBoundingHeight + deltaY
                            
                            // Calculate scale factor (maintain top edge)
                            var scaleFactor = 1.0
                            if (newBoundingBottom > boundingTop + 1) {
                                scaleFactor = (newBoundingBottom - boundingTop) / startBoundingHeight
                            } else {
                                // Minimum height constraint
                                newBoundingBottom = boundingTop + 1
                                scaleFactor = 20 / startBoundingHeight
                            }
                            
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
                property var startElementStates: []
                property real startBoundingX: 0
                property real startBoundingWidth: 0
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
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
                        var deltaX = canvasPos.x - startCanvasX
                        
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
                        } else {
                            // Multiple selection - proportional scaling
                            var newBoundingLeft = startBoundingX + deltaX
                            var boundingRight = startBoundingX + startBoundingWidth
                            
                            // Calculate scale factor (maintain right edge)
                            var scaleFactor = 1.0
                            if (newBoundingLeft < boundingRight - 1) {
                                scaleFactor = (boundingRight - newBoundingLeft) / startBoundingWidth
                            } else {
                                // Minimum width constraint
                                newBoundingLeft = boundingRight - 1
                                scaleFactor = 1 / startBoundingWidth
                            }
                            
                            // Apply proportional changes to all elements
                            for (var i = 0; i < selectedElements.length; i++) {
                                var elem = selectedElements[i]
                                var state = startElementStates[i]
                                
                                // Calculate relative position from right edge
                                var relativeFromRight = (startBoundingX + startBoundingWidth) - (state.x + state.width)
                                
                                // New width scaled proportionally
                                elem.width = state.width * scaleFactor
                                
                                // Position calculated from the new right edge, scaled properly
                                elem.x = boundingRight - relativeFromRight * scaleFactor - elem.width
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
                property var startElementStates: []
                property real startBoundingX: 0
                property real startBoundingWidth: 0
                
                onPressed: function(mouse) {
                    var globalPos = mapToItem(root, mouse.x, mouse.y)
                    var canvasPos = parent.parent.viewportToCanvas(globalPos.x, globalPos.y)
                    startCanvasX = canvasPos.x
                    
                    // Store initial states
                    startElementStates = []
                    for (var i = 0; i < selectedElements.length; i++) {
                        startElementStates.push({
                            x: selectedElements[i].x,
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
                        var deltaX = canvasPos.x - startCanvasX
                        
                        if (selectedElements.length === 1) {
                            // Single selection - direct manipulation
                            var element = selectedElements[0]
                            var startState = startElementStates[0]
                            
                            // Calculate new right edge position
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
                        } else {
                            // Multiple selection - proportional scaling
                            var boundingLeft = startBoundingX
                            var newBoundingRight = startBoundingX + startBoundingWidth + deltaX
                            
                            // Calculate scale factor (maintain left edge)
                            var scaleFactor = 1.0
                            if (newBoundingRight > boundingLeft + 1) {
                                scaleFactor = (newBoundingRight - boundingLeft) / startBoundingWidth
                            } else {
                                // Minimum width constraint
                                newBoundingRight = boundingLeft + 1
                                scaleFactor = 1 / startBoundingWidth
                            }
                            
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
                }
            }
            
            // Resize joints (yellow, smaller) - positioned at bar intersections
            Repeater {
                model: 4
                Rectangle {
                    width: Config.controlResizeJointSize
                    height: Config.controlResizeJointSize
                    color: Config.controlResizeJointColor
                    
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