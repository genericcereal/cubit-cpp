import QtQuick
import Cubit 1.0

Item {
    id: root
    
    // The C++ Edge object
    property var element
    property var elementModel
    property Edge edgeElement: element as Edge
    
    // Canvas reference for getting node positions
    property var canvas
    
    // Edge fills the entire parent (which is the edges layer)
    anchors.fill: parent
    
    // Make sure edge is visible
    visible: true
    opacity: 1.0
    z: 100  // Very high z-order for debugging
    
    // Source and target nodes
    property var sourceNode: {
        if (!elementModel || !edgeElement) return null
        return elementModel.getElementById(edgeElement.sourceNodeId)
    }
    
    property var targetNode: {
        if (!elementModel || !edgeElement) return null
        return elementModel.getElementById(edgeElement.targetNodeId)
    }
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Debug background to see if edge element is visible
    Rectangle {
        anchors.fill: parent
        color: "yellow"
        opacity: 0.2
        z: -1
    }
    
    // Draw the edge using Canvas - simplified to just draw points for debugging
    Canvas {
        id: edgeCanvas
        anchors.fill: parent
        z: 10  // Ensure canvas is above the debug rectangle
        
        onPaint: {
            console.log("EdgeElement onPaint called")
            console.log("  Canvas size:", width, "x", height)
            
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (!edgeElement || !sourceNode || !targetNode) {
                console.log("  Missing data - edgeElement:", !!edgeElement, "sourceNode:", !!sourceNode, "targetNode:", !!targetNode)
                return
            }
            
            // Get points in canvas coordinates
            var sp = edgeElement.sourcePoint
            var tp = edgeElement.targetPoint
            
            // Adjust for canvas offset (parent Loader is already offset by canvasMin)
            var canvasOffsetX = canvas ? canvas.canvasMinX : 0
            var canvasOffsetY = canvas ? canvas.canvasMinY : 0
            
            var localSp = Qt.point(sp.x - canvasOffsetX, sp.y - canvasOffsetY)
            var localTp = Qt.point(tp.x - canvasOffsetX, tp.y - canvasOffsetY)
            
            console.log("  Source point (world):", sp.x, sp.y)
            console.log("  Target point (world):", tp.x, tp.y)
            console.log("  Canvas offset:", canvasOffsetX, canvasOffsetY)
            console.log("  Source point (local):", localSp.x, localSp.y)
            console.log("  Target point (local):", localTp.x, localTp.y)
            
            // Draw circles at the handle positions
            ctx.fillStyle = "#FF0000"  // Red for source
            ctx.beginPath()
            ctx.arc(localSp.x, localSp.y, 10, 0, 2 * Math.PI)
            ctx.fill()
            
            ctx.fillStyle = "#0000FF"  // Blue for target
            ctx.beginPath()
            ctx.arc(localTp.x, localTp.y, 10, 0, 2 * Math.PI)
            ctx.fill()
            
            console.log("  Points drawn")
        }
    }
    
    // Repaint when selection changes
    onSelectedChanged: edgeCanvas.requestPaint()
    
    // Watch for element changes
    Connections {
        target: edgeElement
        enabled: edgeElement !== null
        
        function onSourcePointChanged() { edgeCanvas.requestPaint() }
        function onTargetPointChanged() { edgeCanvas.requestPaint() }
        function onControlPoint1Changed() { edgeCanvas.requestPaint() }
        function onControlPoint2Changed() { edgeCanvas.requestPaint() }
        function onGeometryChanged() { edgeCanvas.requestPaint() }
    }
    
    // Watch for node movements
    Connections {
        target: sourceNode
        enabled: sourceNode !== null
        
        function onXChanged() { updateEdgePosition() }
        function onYChanged() { updateEdgePosition() }
        function onWidthChanged() { updateEdgePosition() }
        function onHeightChanged() { updateEdgePosition() }
    }
    
    Connections {
        target: targetNode
        enabled: targetNode !== null
        
        function onXChanged() { updateEdgePosition() }
        function onYChanged() { updateEdgePosition() }
        function onWidthChanged() { updateEdgePosition() }
        function onHeightChanged() { updateEdgePosition() }
    }
    
    function updateEdgePosition() {
        if (!edgeElement || !sourceNode || !targetNode) {
            return
        }
        
        console.log("EdgeElement.updateEdgePosition called")
        console.log("  Source node position:", sourceNode.x, sourceNode.y)
        console.log("  Target node position:", targetNode.x, targetNode.y)
        
        // Recalculate edge endpoints based on current node positions
        var sourceX, sourceY, targetX, targetY
        
        // Source point calculation
        if (edgeElement.sourceHandleType === "right") {
            sourceX = sourceNode.x + sourceNode.width
        } else {
            sourceX = sourceNode.x
        }
        sourceY = sourceNode.y + 60 + 15 + (edgeElement.sourcePortIndex * 40)
        
        // Target point calculation  
        if (edgeElement.targetHandleType === "left") {
            targetX = targetNode.x
        } else {
            targetX = targetNode.x + targetNode.width
        }
        targetY = targetNode.y + 60 + 15 + (edgeElement.targetPortIndex * 40)
        
        console.log("  New source point:", sourceX, sourceY)
        console.log("  New target point:", targetX, targetY)
        
        // Update the edge points - this will trigger updateControlPoints and updateGeometry in C++
        edgeElement.sourcePoint = Qt.point(sourceX, sourceY)
        edgeElement.targetPoint = Qt.point(targetX, targetY)
    }
    
    Component.onCompleted: {
        console.log("EdgeElement created for edge:", element ? element.elementId : "null")
        console.log("  EdgeElement dimensions:", width, "x", height)
        console.log("  Parent dimensions:", parent ? parent.width + "x" + parent.height : "no parent")
        console.log("  element:", element)
        console.log("  edgeElement:", edgeElement)
        if (edgeElement) {
            console.log("  Source node ID:", edgeElement.sourceNodeId)
            console.log("  Target node ID:", edgeElement.targetNodeId)
            console.log("  Source point:", edgeElement.sourcePoint)
            console.log("  Target point:", edgeElement.targetPoint)
            console.log("  Control point 1:", edgeElement.controlPoint1)
            console.log("  Control point 2:", edgeElement.controlPoint2)
            console.log("  Edge color:", edgeElement.edgeColor)
            console.log("  Edge width:", edgeElement.edgeWidth)
        }
        console.log("  Element position (x,y):", element.x, element.y)
        console.log("  EdgeElement position:", x, y, "Size:", width, "x", height)
        console.log("  Parent:", parent)
        console.log("  Visible:", visible)
        console.log("  Z-order:", z)
        console.log("  Opacity:", opacity)
        
        // Log canvas info
        if (canvas) {
            console.log("  Canvas minX:", canvas.canvasMinX, "minY:", canvas.canvasMinY)
        }
        
        updateEdgePosition()
        edgeCanvas.requestPaint()
    }
}