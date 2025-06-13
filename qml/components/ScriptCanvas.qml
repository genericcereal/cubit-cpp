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
    
    // Hover state
    property var hoveredElement: null
    property point hoveredPoint: Qt.point(0, 0)
    
    // Override content layer with script elements
    Component.onCompleted: {
        // Call base implementation
        centerViewAtOrigin()
        
        // Add script-specific initialization
        var contentLayer = getContentLayer()
        if (contentLayer) {
            nodesLoader.parent = contentLayer
        }
        
        // Activate the loaders after setup
        activationTimer.start()
    }
    
    // Clean up on destruction
    Component.onDestruction: {
        nodesLoader.active = false
    }
    
    // Simple edge test - draw circles at handle positions
    Item {
        id: edgesLayer
        parent: getContentLayer()
        anchors.fill: parent
        z: 10  // Very high z-order
        
        Repeater {
            model: root.elementModel
            
            delegate: Item {
                visible: model.elementType === "Edge"
                property var edge: model.element
                property var edgeObj: edge as Edge
                
                // Get source and target nodes
                property var sourceNode: edgeObj && root.elementModel ? root.elementModel.getElementById(edgeObj.sourceNodeId) : null
                property var targetNode: edgeObj && root.elementModel ? root.elementModel.getElementById(edgeObj.targetNodeId) : null
                
                // Update edge positions when nodes move
                Connections {
                    target: sourceNode
                    enabled: sourceNode !== null
                    function onXChanged() { updateEdgePositions() }
                    function onYChanged() { updateEdgePositions() }
                    function onWidthChanged() { updateEdgePositions() }
                }
                
                Connections {
                    target: targetNode
                    enabled: targetNode !== null
                    function onXChanged() { updateEdgePositions() }
                    function onYChanged() { updateEdgePositions() }
                    function onHeightChanged() { updateEdgePositions() }
                }
                
                function updateEdgePositions() {
                    if (!edgeObj || !sourceNode || !targetNode) return
                    
                    // NodeRow calculation from NodeElement.qml:
                    // - Title height: 10 (top margin) + ~20 (text height) = 30
                    // - Row container top margin: 15
                    // - Each row height: 30, spacing: 10
                    // - Handle is vertically centered in row (30/2 = 15)
                    
                    var titleAndMargins = 30 + 15  // 45
                    var rowOffset = edgeObj.sourcePortIndex * 40  // row height (30) + spacing (10)
                    var handleCenterY = 15  // center of 30px row
                    
                    // Calculate source point (right side of source node, center of handle)
                    var sourceX = sourceNode.x + sourceNode.width - 20  // Right handle center
                    var sourceY = sourceNode.y + titleAndMargins + rowOffset + handleCenterY
                    
                    // Calculate target point (left side of target node, center of handle)
                    rowOffset = edgeObj.targetPortIndex * 40
                    var targetX = targetNode.x + 20  // Left handle center
                    var targetY = targetNode.y + titleAndMargins + rowOffset + handleCenterY
                    
                    // Update the edge points
                    edgeObj.sourcePoint = Qt.point(sourceX, sourceY)
                    edgeObj.targetPoint = Qt.point(targetX, targetY)
                }
                
                Component.onCompleted: {
                    updateEdgePositions()
                }
                
                // Bezier curve using Shape and PathSVG
                BezierEdge {
                    anchors.fill: parent
                    edge: edgeObj
                    canvasMinX: root.canvasMinX
                    canvasMinY: root.canvasMinY
                    z: -1
                }
                
                // Source handle circle
                Rectangle {
                    x: edgeObj && edgeObj.sourcePoint ? edgeObj.sourcePoint.x - root.canvasMinX - 10 : 0
                    y: edgeObj && edgeObj.sourcePoint ? edgeObj.sourcePoint.y - root.canvasMinY - 10 : 0
                    width: 20
                    height: 20
                    radius: 10
                    color: "red"
                    opacity: 0.8
                }
                
                // Target handle circle
                Rectangle {
                    x: edgeObj && edgeObj.targetPoint ? edgeObj.targetPoint.x - root.canvasMinX - 10 : 0
                    y: edgeObj && edgeObj.targetPoint ? edgeObj.targetPoint.y - root.canvasMinY - 10 : 0
                    width: 20
                    height: 20
                    radius: 10
                    color: "blue"
                    opacity: 0.8
                }
            }
        }
    }
    
    // Nodes layer loader
    Loader {
        id: nodesLoader
        active: false
        asynchronous: false
        z: 1
        
        sourceComponent: Component {
            Item {
                id: nodesLayer
                anchors.fill: parent
                
                Repeater {
                    id: nodeRepeater
                    model: root.elementModel
                    
                    delegate: Loader {
                        property var element: model.element
                        property string elementType: model.elementType
                        
                        // Position elements relative to canvas origin
                        x: element ? element.x - root.canvasMinX : 0
                        y: element ? element.y - root.canvasMinY : 0
                        
                        sourceComponent: {
                            if (!element || !elementType) return null
                            switch(elementType) {
                                case "Node": return nodeComponent
                                case "Edge": return null  // Edges will be handled separately
                                default: return null
                            }
                        }
                        
                        onLoaded: {
                            if (item && element) {
                                item.element = element
                                item.elementModel = root.elementModel
                                item.canvas = root  // Pass canvas reference
                            }
                        }
                    }
                }
            }
        }
        
        onLoaded: {
            if (item && parent) {
                item.anchors.fill = parent
            }
        }
    }
    
    // Activation timer to prevent race conditions
    Timer {
        id: activationTimer
        interval: 100
        onTriggered: {
            nodesLoader.active = true
            
            // Create default nodes after loaders are active
            createDefaultNodes()
        }
    }
    
    onVisibleChanged: {
        if (visible) {
            activationTimer.start()
        } else {
            nodesLoader.active = false
        }
    }
    
    // Override virtual functions for script canvas behavior
    function handleLeftButtonPress(canvasPoint) {
        if (controller.mode === "select") {
            // Let the controller handle the press for node selection/dragging
            controller.handleMousePress(canvasPoint.x, canvasPoint.y)
        }
        // Other modes will be handled when node creation is implemented
    }
    
    function handleMouseDrag(canvasPoint) {
        // Let the controller handle drag
        controller.handleMouseMove(canvasPoint.x, canvasPoint.y)
    }
    
    function handleMouseHover(canvasPoint) {
        // Check if hovering over a node
        if (controller) {
            var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
            hoveredPoint = canvasPoint
            if (element !== hoveredElement) {
                hoveredElement = element
                if (element && element.objectName === "Node") {
                    console.log("Started hovering over node:", element.nodeTitle)
                } else if (!element && hoveredElement) {
                    console.log("Stopped hovering")
                }
            }
        }
    }
    
    function handleLeftButtonRelease(canvasPoint) {
        // Check if this was a click on empty canvas (no selection box was drawn)
        var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
        if (!element && !selectionBoxHandler.active && selectionManager && selectionManager.hasSelection) {
            // Clear selection when clicking on empty canvas
            selectionManager.clearSelection()
        }
        
        // Let the controller handle release
        controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
    }
    
    function handleMouseExit() {
        // Will clear hover state when implemented
    }
    
    function handleSelectionRect(rect) {
        console.log("=== ScriptCanvas.handleSelectionRect called ===")
        console.log("Selection rect:", JSON.stringify({x: rect.x, y: rect.y, width: rect.width, height: rect.height}))
        
        // Select all nodes that intersect with the selection rectangle
        if (!controller || !elementModel || !selectionManager) {
            console.log("ERROR: Missing required components:")
            console.log("  controller:", !!controller)
            console.log("  elementModel:", !!elementModel)
            console.log("  selectionManager:", !!selectionManager)
            return
        }
        
        var elements = elementModel.getAllElements()
        console.log("Total elements found:", elements.length)
        
        var selectedNodes = []
        
        for (var i = 0; i < elements.length; i++) {
            var element = elements[i]
            console.log("Element", i, ":")
            console.log("  type:", element ? element.objectName : "null")
            console.log("  element exists:", !!element)
            
            // Only consider Node elements
            if (element && element.objectName === "Node") {
                // Check if element intersects with selection rect
                var elementRect = Qt.rect(element.x, element.y, element.width, element.height)
                console.log("  Node position:", JSON.stringify({x: element.x, y: element.y}))
                console.log("  Node size:", JSON.stringify({width: element.width, height: element.height}))
                console.log("  Node rect:", JSON.stringify({x: elementRect.x, y: elementRect.y, width: elementRect.width, height: elementRect.height}))
                
                // Check for intersection
                var intersects = rect.x < elementRect.x + elementRect.width &&
                    rect.x + rect.width > elementRect.x &&
                    rect.y < elementRect.y + elementRect.height &&
                    rect.y + rect.height > elementRect.y
                    
                console.log("  Intersection test:")
                console.log("    rect.x (" + rect.x + ") < elementRect.x + width (" + (elementRect.x + elementRect.width) + "):", rect.x < elementRect.x + elementRect.width)
                console.log("    rect.x + width (" + (rect.x + rect.width) + ") > elementRect.x (" + elementRect.x + "):", rect.x + rect.width > elementRect.x)
                console.log("    rect.y (" + rect.y + ") < elementRect.y + height (" + (elementRect.y + elementRect.height) + "):", rect.y < elementRect.y + elementRect.height)
                console.log("    rect.y + height (" + (rect.y + rect.height) + ") > elementRect.y (" + elementRect.y + "):", rect.y + rect.height > elementRect.y)
                console.log("  Intersects:", intersects)
                
                if (intersects) {
                    selectedNodes.push(element)
                    console.log("  -> Node added to selection")
                }
            }
        }
        
        console.log("Total nodes selected:", selectedNodes.length)
        
        // Update selection
        if (selectedNodes.length > 0) {
            console.log("Updating selection with", selectedNodes.length, "nodes")
            selectionManager.clearSelection()
            for (var j = 0; j < selectedNodes.length; j++) {
                console.log("Selecting node", j, ":", selectedNodes[j])
                selectionManager.selectElement(selectedNodes[j])
            }
        } else {
            console.log("No nodes selected - not updating selection")
        }
        
        console.log("=== handleSelectionRect complete ===")
    }
    
    // Create default nodes
    function createDefaultNodes() {
        console.log("ScriptCanvas.createDefaultNodes called")
        if (!controller || !elementModel) {
            console.log("  Missing controller or elementModel")
            return
        }
        
        // Create first node - use light blue from Config
        console.log("  Creating first node at (-150, -50)")
        controller.createNode(-150, -50, "Start Node", "")
        
        // Create second node - also light blue
        console.log("  Creating second node at (50, -50)")
        controller.createNode(50, -50, "Process Node", "")
        
        console.log("  Default nodes created")
        
        // Create a default edge between the nodes
        Qt.callLater(function() {
            createDefaultEdge()
        })
    }
    
    // Create default edge between the two nodes
    function createDefaultEdge() {
        console.log("ScriptCanvas.createDefaultEdge called")
        var elements = elementModel.getAllElements()
        var nodes = []
        
        // Find the two nodes
        for (var i = 0; i < elements.length; i++) {
            if (elements[i].objectName === "Node") {
                nodes.push(elements[i])
            }
        }
        
        if (nodes.length >= 2) {
            console.log("  Found", nodes.length, "nodes, creating edge")
            console.log("  Node 0 ID:", nodes[0].elementId)
            console.log("  Node 1 ID:", nodes[1].elementId)
            
            // Create edge using controller method
            // Connect from first node's output (right) to second node's input (left)
            controller.createEdge(
                nodes[0].elementId,     // sourceNodeId
                nodes[1].elementId,     // targetNodeId
                "right",                // sourceHandleType
                "left",                 // targetHandleType
                2,                      // sourcePortIndex (Output row)
                0                       // targetPortIndex (Input 1 row)
            )
            
            console.log("  Edge creation requested")
        } else {
            console.log("  Not enough nodes found:", nodes.length)
        }
    }
    
    // Component definitions
    Component {
        id: nodeComponent
        NodeElement {}
    }
    
    Component {
        id: edgeComponent
        EdgeElement {}
    }
}