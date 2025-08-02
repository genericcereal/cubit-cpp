import QtQuick 2.15
import QtQuick.Shapes 1.15
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property ShapeElement shapeElement: element as ShapeElement
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    Component.onCompleted: {
        // Debug logging removed
    }
    
    Component.onDestruction: {
        // Debug logging removed
    }
    
    // Function to find closed loops in the edge graph
    function findClosedLoops(edges, jointCount) {
        // Build adjacency list
        var adjacencyList = []
        for (var i = 0; i < jointCount; i++) {
            adjacencyList[i] = []
        }
        
        for (var e = 0; e < edges.length; e++) {
            var edge = edges[e]
            adjacencyList[edge.from].push(edge.to)
            adjacencyList[edge.to].push(edge.from) // Undirected graph
        }
        
        var visited = []
        var loops = []
        
        // DFS to find cycles
        function findCycles(node, path, start) {
            if (visited[node] && node === start && path.length >= 3) {
                // Found a cycle, add it to loops
                var cycle = path.slice()
                cycle.push(node) // Close the cycle
                loops.push(cycle)
                return
            }
            
            if (visited[node]) {
                return // Already processed
            }
            
            visited[node] = true
            path.push(node)
            
            var neighbors = adjacencyList[node]
            for (var n = 0; n < neighbors.length; n++) {
                var neighbor = neighbors[n]
                if (path.length === 1 || neighbor !== path[path.length - 2]) { // Avoid immediate backtrack
                    findCycles(neighbor, path, start)
                }
            }
            
            path.pop()
            visited[node] = false
        }
        
        // Try starting from each joint
        for (var startNode = 0; startNode < jointCount; startNode++) {
            visited = []
            for (var v = 0; v < jointCount; v++) {
                visited[v] = false
            }
            findCycles(startNode, [], startNode)
        }
        
        // Remove duplicate cycles and return unique ones
        var uniqueLoops = []
        for (var l = 0; l < loops.length; l++) {
            var loop = loops[l]
            var isUnique = true
            
            for (var ul = 0; ul < uniqueLoops.length; ul++) {
                if (areLoopsEqual(loop, uniqueLoops[ul])) {
                    isUnique = false
                    break
                }
            }
            
            if (isUnique) {
                uniqueLoops.push(loop.slice(0, -1)) // Remove the duplicate closing node
            }
        }
        
        return uniqueLoops
    }
    
    // Helper function to check if two loops are the same (considering rotation and direction)
    function areLoopsEqual(loop1, loop2) {
        if (loop1.length !== loop2.length) return false
        
        var len = loop1.length - 1 // Exclude duplicate closing node
        for (var offset = 0; offset < len; offset++) {
            var match = true
            for (var i = 0; i < len; i++) {
                if (loop1[i] !== loop2[(i + offset) % len]) {
                    match = false
                    break
                }
            }
            if (match) return true
            
            // Check reverse direction
            match = true
            for (var j = 0; j < len; j++) {
                if (loop1[j] !== loop2[(len - 1 - j + offset) % len]) {
                    match = false
                    break
                }
            }
            if (match) return true
        }
        
        return false
    }
    
    onShapeElementChanged: {
        // Debug logging removed
    }
    
    // Shape rendering  
    Canvas {
        id: shapeCanvas
        // Add padding to prevent edge clipping
        anchors.centerIn: parent
        width: parent.width + (shapeElement ? shapeElement.edgeWidth * 2 : 0)
        height: parent.height + (shapeElement ? shapeElement.edgeWidth * 2 : 0)
        antialiasing: true
        
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (!shapeElement || !shapeElement.joints || shapeElement.joints.length === 0) {
                return
            }
            
            var joints = shapeElement.joints
            var shapeType = shapeElement.shapeType
            var padding = shapeElement.edgeWidth
            var drawWidth = width - padding * 2
            var drawHeight = height - padding * 2
            
            // Set stroke properties
            ctx.strokeStyle = shapeElement.edgeColor
            ctx.lineWidth = shapeElement.edgeWidth
            ctx.fillStyle = shapeElement.fillColor
            ctx.lineJoin = shapeElement.lineJoin || "miter"
            ctx.lineCap = shapeElement.lineCap || "round"
            
            // For closed shapes (Square, Triangle)
            if (shapeType !== 2) { // 2 = Pen
                ctx.beginPath()
                ctx.moveTo(joints[0].x * drawWidth + padding, joints[0].y * drawHeight + padding)
                
                for (var i = 1; i < joints.length; i++) {
                    ctx.lineTo(joints[i].x * drawWidth + padding, joints[i].y * drawHeight + padding)
                }
                
                ctx.closePath()
                
                // Fill first, then stroke
                if (shapeElement.fillColor && shapeElement.fillColor.a > 0) {
                    ctx.fill()
                }
                ctx.stroke()
            } else {
                // For Pen shapes - use edges list if available, otherwise consecutive joints
                var edges = shapeElement.edges || []
                
                if (edges.length > 0) {
                    // First, detect and fill closed loops
                    var closedLoops = findClosedLoops(edges, joints.length)
                    for (var loopIndex = 0; loopIndex < closedLoops.length; loopIndex++) {
                        var loop = closedLoops[loopIndex]
                        if (loop.length >= 3) { // Need at least 3 joints for a fillable area
                            ctx.beginPath()
                            ctx.moveTo(joints[loop[0]].x * drawWidth + padding, joints[loop[0]].y * drawHeight + padding)
                            for (var k = 1; k < loop.length; k++) {
                                ctx.lineTo(joints[loop[k]].x * drawWidth + padding, joints[loop[k]].y * drawHeight + padding)
                            }
                            ctx.closePath()
                            
                            // Fill the closed loop
                            if (shapeElement.fillColor && shapeElement.fillColor.a > 0) {
                                ctx.fill()
                            }
                        }
                    }
                    
                    // Set line join style for proper corners
                    ctx.lineJoin = shapeElement.lineJoin || "miter"
                    ctx.lineCap = shapeElement.lineCap || "round"
                    
                    // Build adjacency information to find connected paths
                    var adjacency = {}
                    var edgeUsed = []
                    
                    for (var i = 0; i < edges.length; i++) {
                        edgeUsed[i] = false
                        var edge = edges[i]
                        if (!adjacency[edge.from]) adjacency[edge.from] = []
                        if (!adjacency[edge.to]) adjacency[edge.to] = []
                        adjacency[edge.from].push({to: edge.to, edgeIndex: i})
                        adjacency[edge.to].push({to: edge.from, edgeIndex: i})
                    }
                    
                    // Draw connected paths
                    for (var i = 0; i < edges.length; i++) {
                        if (edgeUsed[i]) continue
                        
                        var edge = edges[i]
                        var path = [edge.from]
                        var current = edge.to
                        edgeUsed[i] = true
                        
                        // Follow the path forward
                        while (current !== undefined && current !== edge.from) {
                            path.push(current)
                            var found = false
                            
                            if (adjacency[current]) {
                                for (var j = 0; j < adjacency[current].length; j++) {
                                    var neighbor = adjacency[current][j]
                                    if (!edgeUsed[neighbor.edgeIndex]) {
                                        edgeUsed[neighbor.edgeIndex] = true
                                        current = neighbor.to
                                        found = true
                                        break
                                    }
                                }
                            }
                            
                            if (!found) break
                        }
                        
                        // Draw the path
                        if (path.length >= 2) {
                            ctx.beginPath()
                            ctx.moveTo(joints[path[0]].x * drawWidth + padding, joints[path[0]].y * drawHeight + padding)
                            for (var k = 1; k < path.length; k++) {
                                ctx.lineTo(joints[path[k]].x * drawWidth + padding, joints[path[k]].y * drawHeight + padding)
                            }
                            // Check if this path forms a closed loop
                            var isClosedLoop = false
                            if (adjacency[path[path.length - 1]]) {
                                for (var j = 0; j < adjacency[path[path.length - 1]].length; j++) {
                                    if (adjacency[path[path.length - 1]][j].to === path[0]) {
                                        isClosedLoop = true
                                        break
                                    }
                                }
                            }
                            // Close the path to draw the final edge back to start
                            if (isClosedLoop) {
                                ctx.closePath()
                            }
                            ctx.stroke()
                        }
                    }
                } else {
                    // Fallback: handle multiple paths with consecutive joints
                    var pathStarted = false
                    
                    for (var j = 0; j < joints.length; j++) {
                        var joint = joints[j]
                        
                        if (joint.isPathStart || j === 0) {
                            // Start a new path
                            if (pathStarted) {
                                ctx.stroke() // Finish previous path
                            }
                            ctx.beginPath()
                            ctx.moveTo(joint.x * drawWidth + padding, joint.y * drawHeight + padding)
                            pathStarted = true
                        } else {
                            // Continue current path
                            ctx.lineTo(joint.x * drawWidth + padding, joint.y * drawHeight + padding)
                        }
                    }
                    
                    // Stroke the last path
                    if (pathStarted) {
                        ctx.stroke()
                    }
                }
            }
        }
        
        // Redraw when joints change
        Connections {
            target: shapeElement
            enabled: shapeElement !== null
            ignoreUnknownSignals: true
            function onJointsChanged() {
                shapeCanvas.requestPaint()
            }
            function onEdgesChanged() {
                shapeCanvas.requestPaint()
            }
            function onEdgeColorChanged() {
                shapeCanvas.requestPaint()
            }
            function onEdgeWidthChanged() {
                shapeCanvas.requestPaint()
            }
            function onFillColorChanged() {
                shapeCanvas.requestPaint()
            }
            function onLineJoinChanged() {
                shapeCanvas.requestPaint()
            }
            function onLineCapChanged() {
                shapeCanvas.requestPaint()
            }
        }
        
        Component.onCompleted: {
            if (shapeElement) {
                shapeCanvas.requestPaint()
            }
        }
    }
}