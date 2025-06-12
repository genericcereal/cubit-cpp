import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "script"
    
    // Node and edge management will be added later
    property var nodes: []
    property var edges: []
    
    // Override content layer with script elements
    Component.onCompleted: {
        // Call base implementation
        centerViewAtOrigin()
        
        // Add script-specific initialization
        var contentLayer = getContentLayer()
        nodesLayer.parent = contentLayer
        edgesLayer.parent = contentLayer
    }
    
    // Edges layer (rendered below nodes)
    Item {
        id: edgesLayer
        anchors.fill: parent ? parent : undefined
        z: 0
        
        // Placeholder for edge rendering
        // Will be implemented when edges are added
    }
    
    // Nodes layer
    Item {
        id: nodesLayer
        anchors.fill: parent ? parent : undefined
        z: 1
        
        // Placeholder for node rendering
        // Will be implemented when nodes are added
    }
    
    // Override virtual functions for script canvas behavior
    function handleLeftButtonPress(canvasPoint) {
        if (controller.mode === "select") {
            // For now, just handle selection box (no nodes to select yet)
            // When nodes are added, we'll check for node hits here
        }
        // Other modes will be handled when node creation is implemented
    }
    
    function handleMouseDrag(canvasPoint) {
        // Will handle node dragging when implemented
    }
    
    function handleMouseHover(canvasPoint) {
        // Will handle node hover when implemented
    }
    
    function handleLeftButtonRelease(canvasPoint) {
        // Will handle node creation/connection when implemented
    }
    
    function handleMouseExit() {
        // Will clear hover state when implemented
    }
    
    function handleSelectionRect(rect) {
        // Will select nodes in rect when implemented
        // For now, no elements to select
    }
    
    // Grid overlay for script canvas (optional visual aid)
    Item {
        id: gridOverlay
        parent: canvasArea
        anchors.fill: parent
        z: -1
        visible: root.zoomLevel > 0.5
        opacity: Math.min(1, (root.zoomLevel - 0.5) * 2)
        
        Canvas {
            anchors.fill: parent
            
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                
                var gridSize = 50
                var offsetX = -root.canvasMinX % gridSize
                var offsetY = -root.canvasMinY % gridSize
                
                ctx.strokeStyle = "#e0e0e0"
                ctx.lineWidth = 1
                
                // Draw vertical lines
                for (var x = offsetX; x < width; x += gridSize) {
                    ctx.beginPath()
                    ctx.moveTo(x, 0)
                    ctx.lineTo(x, height)
                    ctx.stroke()
                }
                
                // Draw horizontal lines
                for (var y = offsetY; y < height; y += gridSize) {
                    ctx.beginPath()
                    ctx.moveTo(0, y)
                    ctx.lineTo(width, y)
                    ctx.stroke()
                }
                
                // Draw origin lines
                ctx.strokeStyle = "#cccccc"
                ctx.lineWidth = 2
                
                // Origin X axis
                var originY = -root.canvasMinY
                if (originY >= 0 && originY <= height) {
                    ctx.beginPath()
                    ctx.moveTo(0, originY)
                    ctx.lineTo(width, originY)
                    ctx.stroke()
                }
                
                // Origin Y axis
                var originX = -root.canvasMinX
                if (originX >= 0 && originX <= width) {
                    ctx.beginPath()
                    ctx.moveTo(originX, 0)
                    ctx.lineTo(originX, height)
                    ctx.stroke()
                }
            }
            
            // Redraw when zoom changes
            Connections {
                target: root
                function onZoomLevelChanged() {
                    gridOverlay.Canvas.requestPaint()
                }
            }
        }
    }
}