import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."
import "../CanvasUtils.js" as Utils

BaseCanvas {
    id: root
    
    canvasType: "script"
    // isEdgePreview is defined in this component and passed to BaseCanvas
    
    // BezierEdge component is now directly available
    property Component bezierEdgeComponent: BezierEdge {}
    
    // Node and edge management
    property var nodes: []
    property var edges: []
    
    // Hover state
    property var hoveredElement: null
    property point hoveredPoint: Qt.point(0, 0)
    
    // Click state for text input focus
    property bool clickedThisFrame: false
    property point clickPoint: Qt.point(0, 0)
    
    // Signal to notify when a click occurs
    signal canvasClicked(point clickPoint)
    
    // Signal to notify when dragging starts
    signal canvasDragStarted()
    
    // Signal to notify when clicked outside any node
    signal canvasClickedOutside()
    
    // Node catalog instance
    NodeCatalog {
        id: nodeCatalog
    }
    
    // Track if any node is being dragged
    property bool isNodeDragging: false
    
    // Force edge updates when node dragging state changes
    onIsNodeDraggingChanged: {
        if (!isNodeDragging) {
            // When dragging stops, force a final update of all edges
            updateAllEdges()
        }
    }
    
    // Timer to update edges during node dragging
    Timer {
        id: edgeUpdateTimer
        interval: 16  // 60 FPS
        repeat: true
        running: isNodeDragging
        onTriggered: {
            // Force re-evaluation of all edge bindings
            edgesLayer.visible = false
            edgesLayer.visible = true
        }
    }
    
    // Function to force update all edges
    function updateAllEdges() {
        // Force all edge delegates to update by triggering a model refresh
        if (elementModel) {
            // This will cause all delegates to re-evaluate their bindings
            elementModel.dataChanged(elementModel.index(0, 0), 
                                   elementModel.index(elementModel.rowCount() - 1, 0))
        }
    }
    
    
    // Handle drag state
    property bool isDraggingHandle: false
    property bool isEdgePreview: false  // True when dragging to create edge preview
    property var dragSourceNode: null
    property string dragSourceHandleType: ""  // "left" or "right"
    property int dragSourcePortIndex: -1
    property string dragSourcePortType: "Flow"  // "Flow", "Boolean", "String", or "Number"
    property point dragCurrentPoint: Qt.point(0, 0)
    
    // Node catalog state
    property bool showNodeCatalog: false
    property point nodeCatalogPosition: Qt.point(0, 0)
    
    // Port drag handling functions for nodes
    function startPortDrag(node, handleType, portIndex, portType) {
        isDraggingHandle = true
        isEdgePreview = true  // Enable edge preview mode
        dragSourceNode = node
        dragSourceHandleType = handleType
        dragSourcePortIndex = portIndex
        dragSourcePortType = portType
        console.log("Started port drag from node:", node.nodeTitle, "handle:", handleType, "port:", portIndex, "type:", portType)
        console.log("Edge preview mode enabled - all node dragging disabled")
    }
    
    function updatePortDrag(canvasPos) {
        dragCurrentPoint = canvasPos
        console.log("Updating port drag to canvas position:", canvasPos.x.toFixed(2), canvasPos.y.toFixed(2))
    }
    
    function endPortDrag(canvasPos) {
        if (!isDraggingHandle) return
        
        // Check if we're over a target handle
        var targetHandleInfo = getHandleAtPoint(canvasPos)
        if (targetHandleInfo && targetHandleInfo.node !== dragSourceNode) {
            // Create edge between source and target
            console.log("Creating edge from", dragSourceNode.nodeTitle, "to", targetHandleInfo.node.nodeTitle)
            controller.createEdge(
                dragSourceNode.elementId,
                targetHandleInfo.node.elementId,
                dragSourceHandleType,
                targetHandleInfo.handleType,
                dragSourcePortIndex,
                targetHandleInfo.portIndex
            )
        } else {
            // Show node catalog at release position
            showNodeCatalog = true
            nodeCatalogPosition = canvasPos
            console.log("Showing node catalog at", canvasPos.x, canvasPos.y)
        }
        
        // Reset drag state, but keep source info when showing catalog
        isDraggingHandle = false
        isEdgePreview = false  // Disable edge preview mode
        if (!showNodeCatalog) {
            dragSourceNode = null
            dragSourceHandleType = ""
            dragSourcePortIndex = -1
            dragSourcePortType = "Flow"
        }
        console.log("Edge preview mode disabled - node dragging re-enabled")
    }
    
    
    // Add content into the default contentData
    contentData: [
        // Canvas background - handles background clicks
        CanvasBackground {
            id: canvasBackground
            anchors.fill: parent
            canvas: root
            z: -1  // Ensure it's behind all other elements
            
            onClicked: (canvasPoint) => {
                // Notify canvas about the click
                root.canvasClicked(canvasPoint)
                root.clickedThisFrame = true
                root.clickPoint = canvasPoint
                
                // Check if clicked outside any node
                var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
                if (!element || element.objectName !== "Node") {
                    root.canvasClickedOutside()
                }
            }
        },
        
        // Edges layer
        Item {
            id: edgesLayer
            anchors.fill: parent
            z: 10  // Very high z-order
            
            Repeater {
                id: edgesRepeater
                model: root.elementModel
                
                delegate: Item {
                    id: edgeDelegate
                    visible: model.elementType === "Edge"
                    property var edge: model.element
                    property var edgeObj: edge as Edge
                    
                    // Get source and target nodes - dynamically look them up
                    property var sourceNode: {
                        if (model.elementType !== "Edge" || !edgeObj || !root.elementModel) return null
                        // Force re-evaluation when dragging
                        var dummy = root.isNodeDragging
                        return root.elementModel.getElementById(edgeObj.sourceNodeId)
                    }
                    property var targetNode: {
                        if (model.elementType !== "Edge" || !edgeObj || !root.elementModel) return null
                        // Force re-evaluation when dragging
                        var dummy = root.isNodeDragging
                        return root.elementModel.getElementById(edgeObj.targetNodeId)
                    }
                    
                    // Watch for source/target node changes and update positions
                    onSourceNodeChanged: {
                        if (sourceNode) {
                            Qt.callLater(updateEdgePositions)
                        } else {
                        }
                    }
                    
                    onTargetNodeChanged: {
                        if (targetNode) {
                            Qt.callLater(updateEdgePositions)
                        } else {
                        }
                    }
                    
                    // Connect to node position changes
                    Connections {
                        target: sourceNode
                        enabled: sourceNode !== null && model.elementType === "Edge"
                        function onXChanged() {
                            updateEdgePositions()
                        }
                        function onYChanged() {
                            updateEdgePositions()
                        }
                        function onWidthChanged() {
                            updateEdgePositions()
                        }
                        function onHeightChanged() {
                            updateEdgePositions()
                        }
                    }
                    
                    Connections {
                        target: targetNode
                        enabled: targetNode !== null && model.elementType === "Edge"
                        function onXChanged() {
                            updateEdgePositions()
                        }
                        function onYChanged() {
                            updateEdgePositions()
                        }
                        function onWidthChanged() {
                            updateEdgePositions()
                        }
                        function onHeightChanged() {
                            updateEdgePositions()
                        }
                    }
                    
                    function updateEdgePositions() {
                        // Always refresh node references when updating
                        var currentSourceNode = sourceNode
                        var currentTargetNode = targetNode
                        
                        if (!edgeObj) {
                            console.log("updateEdgePositions: edgeObj is null!")
                            return
                        }
                        
                        if (!currentSourceNode) {
                            console.log("updateEdgePositions: sourceNode is null for edge", edgeObj.elementId, 
                                       "sourceNodeId:", edgeObj.sourceNodeId)
                            return
                        }
                        
                        if (!currentTargetNode) {
                            console.log("updateEdgePositions: targetNode is null for edge", edgeObj.elementId,
                                       "targetNodeId:", edgeObj.targetNodeId)
                            return
                        }
                        
                        
                        var sourceX, sourceY
                        
                        // Special handling for Variable nodes as source
                        if (currentSourceNode.nodeType === "Variable") {
                            sourceX = currentSourceNode.x + currentSourceNode.width - 15  // 5px margin + 10px to center
                            sourceY = currentSourceNode.y + 15  // Center of 30px header
                        } else {
                            var titleAndMargins = 30 + 15  // Title height + margins
                            var columnsMargin = 10  // Left/right margin of columns container
                            var columnSpacing = 20  // Spacing between columns
                            
                            // Check if this is a flow port (index -1)
                            if (edgeObj.sourcePortIndex === -1) {
                                // Flow ports are positioned at the top right, in the first row of the sources column
                                // Handle is 20x20, anchored to right edge, vertically centered in 30px row
                                sourceX = currentSourceNode.x + currentSourceNode.width - 10 - 10  // right edge - right margin - half handle width
                                sourceY = currentSourceNode.y + 30 + 15 + 15  // header + top margin + half row height
                            } else {
                                // Find the source port index in the sources column
                                var sourceIndex = 0
                                var foundSource = false
                                for (var i = 0; i < currentSourceNode.rowConfigurations.length; i++) {
                                    var config = currentSourceNode.rowConfigurations[i]
                                    if (config.hasSource) {
                                        if (config.sourcePortIndex === edgeObj.sourcePortIndex) {
                                            foundSource = true
                                            break
                                        }
                                        sourceIndex++
                                    }
                                }
                                
                                if (!foundSource) {
                                    console.log("updateEdgePositions: Could not find source port", edgeObj.sourcePortIndex, "in node", currentSourceNode.nodeTitle)
                                    return
                                }
                                
                                // Calculate source point (right column, right side)
                                var columnWidth = (currentSourceNode.width - 2 * columnsMargin - columnSpacing) / 2
                                var rightColumnX = currentSourceNode.x + columnsMargin + columnWidth + columnSpacing
                                sourceX = rightColumnX + columnWidth - 10  // 10px from right edge to center
                                sourceY = currentSourceNode.y + titleAndMargins + sourceIndex * 40 + 15  // center of 30px item
                            }
                        }
                        
                        // Find the target port index in the targets column
                        var titleAndMargins = 30 + 15  // Title height + margins
                        var columnsMargin = 10  // Left/right margin of columns container
                        
                        // Check if this is a flow port (index -1)
                        if (edgeObj.targetPortIndex === -1) {
                            // Flow ports are positioned at the top left, in the first row of the targets column
                            // Handle is 20x20, anchored to left edge, vertically centered in 30px row
                            var targetX = currentTargetNode.x + 10 + 10  // left edge + left margin + half handle width
                            var targetY = currentTargetNode.y + 30 + 15 + 15  // header + top margin + half row height
                        } else {
                            var targetIndex = 0
                            var foundTarget = false
                            for (var j = 0; j < currentTargetNode.rowConfigurations.length; j++) {
                                var targetConfig = currentTargetNode.rowConfigurations[j]
                                if (targetConfig.hasTarget) {
                                    if (targetConfig.targetPortIndex === edgeObj.targetPortIndex) {
                                        foundTarget = true
                                        break
                                    }
                                    targetIndex++
                                }
                            }
                            
                            if (!foundTarget) {
                                console.log("updateEdgePositions: Could not find target port", edgeObj.targetPortIndex, "in node", currentTargetNode.nodeTitle)
                                return
                            }
                            
                            // Calculate target point (left column, left side)
                            var targetX = currentTargetNode.x + columnsMargin + 10  // 10px to center of 20px handle
                            var targetY = currentTargetNode.y + titleAndMargins + targetIndex * 40 + 15
                        }
                        
                        // Update the edge points
                        edgeObj.sourcePoint = Qt.point(sourceX, sourceY)
                        edgeObj.targetPoint = Qt.point(targetX, targetY)
                    }
                    
                    Component.onCompleted: {
                        if (model.elementType === "Edge") {
                            console.log("Edge delegate created for edge:", edgeObj ? edgeObj.elementId : "unknown",
                                       "sourceNodeId:", edgeObj ? edgeObj.sourceNodeId : "unknown",
                                       "targetNodeId:", edgeObj ? edgeObj.targetNodeId : "unknown")
                            console.log("  sourceNode found:", !!sourceNode, sourceNode ? sourceNode.nodeTitle : "null")
                            console.log("  targetNode found:", !!targetNode, targetNode ? targetNode.nodeTitle : "null")
                            updateEdgePositions()
                        }
                    }
                    
                    // Bezier curve using BezierEdge component
                    BezierEdge {
                        id: bezierEdge
                        anchors.fill: parent
                        z: -1
                        edge: edgeObj
                        canvasMinX: root.canvasMinX
                        canvasMinY: root.canvasMinY
                        visible: !root.isNodeDragging || true
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
        },
        
        // Nodes layer
        Repeater {
            id: nodeRepeater
            model: root.elementModel
            
            delegate: Loader {
                property var element: model.element
                property string elementType: model.elementType
                
                // Position elements relative to canvas origin
                x: element ? element.x - root.canvasMinX : 0
                y: element ? element.y - root.canvasMinY : 0
                z: 1
                
                sourceComponent: {
                    if (!element || !elementType) return null
                    switch(elementType) {
                        case "Node": return nodeComponent
                        case "Edge": return null  // Edges are handled separately
                        default: return null
                    }
                }
                
                onLoaded: {
                    if (item && element) {
                        item.element = element
                        item.elementModel = root.elementModel
                        item.canvas = root  // Pass canvas reference
                        console.log("ScriptCanvas: Loaded node", element.nodeTitle, "at", element.x, element.y, "view pos:", x, y)
                    }
                }
                
                Component.onCompleted: {
                    if (element && elementType === "Node") {
                        console.log("ScriptCanvas: Node delegate created for", element.nodeTitle, "at", element.x, element.y)
                    }
                }
            }
        },
        
        // Temporary edge preview during handle drag
        Item {
            id: tempEdgeContainer
            visible: (root.isDraggingHandle || root.showNodeCatalog) && root.dragSourceNode
            anchors.fill: parent
            z: 11  // Above edges layer
            
            onVisibleChanged: {
                console.log("Temp edge container visibility changed to:", visible,
                           "isDraggingHandle:", root.isDraggingHandle,
                           "showNodeCatalog:", root.showNodeCatalog,
                           "dragSourceNode:", root.dragSourceNode ? root.dragSourceNode.nodeTitle : "null")
            }
            
            property point sourcePoint: {
                if (!root.dragSourceNode) return Qt.point(0, 0)
                
                // Special handling for Variable nodes
                if (root.dragSourceNode.nodeType === "Variable") {
                    var handleX = root.dragSourceNode.x + root.dragSourceNode.width - 15  // 5px margin + 10px to center
                    var handleY = root.dragSourceNode.y + 15  // Center of 30px header
                    return Qt.point(handleX, handleY)
                }
                
                var titleAndMargins = 30 + 15
                var columnsMargin = 10
                var columnSpacing = 20
                
                if (root.dragSourceHandleType === "right") {
                    // Check if this is a flow port (index -1)
                    if (root.dragSourcePortIndex === -1) {
                        // Flow ports are positioned at the top right, in the first row of the sources column
                        // Handle is 20x20, anchored to right edge, vertically centered in 30px row
                        var handleX = root.dragSourceNode.x + root.dragSourceNode.width - 10 - 10  // right edge - right margin - half handle width
                        var handleY = root.dragSourceNode.y + 30 + 15 + 15  // header + top margin + half row height
                        return Qt.point(handleX, handleY)
                    } else {
                        // Find source port index in sources column
                        var sourceIndex = 0
                        var foundSource = false
                        for (var i = 0; i < root.dragSourceNode.rowConfigurations.length; i++) {
                            var config = root.dragSourceNode.rowConfigurations[i]
                            if (config.hasSource) {
                                if (config.sourcePortIndex === root.dragSourcePortIndex) {
                                    foundSource = true
                                    break
                                }
                                sourceIndex++
                            }
                        }
                        
                        if (!foundSource) {
                            console.log("Warning: Could not find source port", root.dragSourcePortIndex)
                            return Qt.point(0, 0)
                        }
                        
                        var columnWidth = (root.dragSourceNode.width - 2 * columnsMargin - columnSpacing) / 2
                        var rightColumnX = root.dragSourceNode.x + columnsMargin + columnWidth + columnSpacing
                        var handleX = rightColumnX + columnWidth - 10
                        var handleY = root.dragSourceNode.y + titleAndMargins + sourceIndex * 40 + 15
                        return Qt.point(handleX, handleY)
                    }
                } else {
                    // Check if this is a flow port (index -1)
                    if (root.dragSourcePortIndex === -1) {
                        // Flow ports are positioned at the top left, in the first row of the targets column
                        // Handle is 20x20, anchored to left edge, vertically centered in 30px row
                        var targetHandleX = root.dragSourceNode.x + 10 + 10  // left edge + left margin + half handle width
                        var targetHandleY = root.dragSourceNode.y + 30 + 15 + 15  // header + top margin + half row height
                        return Qt.point(targetHandleX, targetHandleY)
                    } else {
                        // Find target port index in targets column
                        var targetIndex = 0
                        var foundTarget = false
                        for (var j = 0; j < root.dragSourceNode.rowConfigurations.length; j++) {
                            var targetConfig = root.dragSourceNode.rowConfigurations[j]
                            if (targetConfig.hasTarget) {
                                if (targetConfig.targetPortIndex === root.dragSourcePortIndex) {
                                    foundTarget = true
                                    break
                                }
                                targetIndex++
                            }
                        }
                        
                        if (!foundTarget) {
                            console.log("Warning: Could not find target port", root.dragSourcePortIndex)
                            return Qt.point(0, 0)
                        }
                        
                        var targetHandleX = root.dragSourceNode.x + columnsMargin + 10
                        var targetHandleY = root.dragSourceNode.y + titleAndMargins + targetIndex * 40 + 15
                        return Qt.point(targetHandleX, targetHandleY)
                    }
                }
            }
            
            // Create temporary edge object for bezier rendering
            QtObject {
                id: tempEdge
                property point sourcePoint: tempEdgeContainer.sourcePoint
                property point targetPoint: root.dragCurrentPoint
                property point controlPoint1: Qt.point(
                    sourcePoint.x + (targetPoint.x - sourcePoint.x) * 0.5,
                    sourcePoint.y
                )
                property point controlPoint2: Qt.point(
                    sourcePoint.x + (targetPoint.x - sourcePoint.x) * 0.5,
                    targetPoint.y
                )
                property string sourcePortType: root.dragSourcePortType
                property bool selected: false
            }
            
            // Bezier curve preview
            BezierEdge {
                anchors.fill: parent
                edge: tempEdge
                canvasMinX: root.canvasMinX
                canvasMinY: root.canvasMinY
                isPreview: true
            }
            
            // Source handle circle (visual feedback)
            Rectangle {
                x: tempEdgeContainer.sourcePoint.x - root.canvasMinX - 10
                y: tempEdgeContainer.sourcePoint.y - root.canvasMinY - 10
                width: 20
                height: 20
                radius: 10
                color: root.dragSourceHandleType === "right" ? "blue" : "green"
                opacity: 0.8
            }
            
            // Target position circle
            Rectangle {
                x: root.dragCurrentPoint.x - root.canvasMinX - 10
                y: root.dragCurrentPoint.y - root.canvasMinY - 10
                width: 20
                height: 20
                radius: 10
                color: "orange"
                opacity: 0.8
            }
        },
        
        // Drop area for elements dragged from ElementList - moved inside contentData
        DropArea {
            anchors.fill: parent
            z: 100  // High z-order to ensure it's above other elements
            
            onEntered: (drag) => {
                console.log("Drag entered DropArea - elementType:", drag.source ? drag.source.elementType : "no source", "elementName:", drag.source ? drag.source.elementName : "no source")
            }
            
            onExited: {
                console.log("Drag exited DropArea")
            }
            
            onDropped: (drag) => {
                console.log("DropArea.onDropped called")
                // Get element information from drag source
                var elementType = drag.source.elementType
                var elementName = drag.source.elementName
                var elementId = drag.source.elementId
                
                console.log("Dragged element - type:", elementType, "name:", elementName, "id:", elementId)
                
                console.log("DropArea.onDropped - drag.x:", drag.x, "drag.y:", drag.y)
                console.log("Current zoom level:", root.zoom)
                console.log("Flickable contentX:", root.flickable.contentX, "contentY:", root.flickable.contentY)
                
                // The drag.x and drag.y are relative to the DropArea which fills the contentLayer
                // The contentLayer is at the canvas coordinate space (before zoom is applied)
                // So drag.x and drag.y are already in canvas coordinates, just offset by canvasMinX/Y
                
                // The drag preview is 200x100 with hotspot at center (100, 50)
                // So we need to offset by half the node size to align with where the preview appeared
                var nodeX = drag.x + root.canvasMinX - 100  // Subtract half width
                var nodeY = drag.y + root.canvasMinY - 50   // Subtract half height
                
                console.log("Calculated node position:", nodeX, nodeY)
                
                console.log("Creating node at position:", nodeX, nodeY)
                
                // Create Variable node for ALL element types (Frame, Text, Variable)
                var nodeData = {
                    x: nodeX,
                    y: nodeY,
                    width: 150,  // Narrower since it's just a header
                    name: elementName,
                    type: "Variable",
                    sourceElementId: elementId,  // Store reference to source element
                    targets: [],
                    sources: [{
                        id: "value",
                        label: "Value",
                        type: "String"
                    }]
                }
                console.log("Creating node with JSON:", JSON.stringify(nodeData))
                var nodeId = controller.createNodeFromJson(JSON.stringify(nodeData))
                console.log("Created Variable node for", elementType, "element with ID:", nodeId)
                
                // Check if the node was actually created
                if (nodeId && elementModel) {
                    var createdNode = elementModel.getElementById(nodeId)
                    if (createdNode) {
                        console.log("Node successfully added to model at", createdNode.x, createdNode.y)
                    } else {
                        console.log("ERROR: Node was not found in model after creation")
                    }
                }
            }
        }
    ]
    
    // Node catalog popup - Must be outside content layer to receive mouse events
    NodeCatalogPopup {
        id: nodeCatalogPopup
        parent: root
        canvas: root
        nodeCatalog: nodeCatalog
        showCatalog: root.showNodeCatalog
        position: root.nodeCatalogPosition
        dragSourceNode: root.dragSourceNode
        dragSourceHandleType: root.dragSourceHandleType
        dragSourcePortIndex: root.dragSourcePortIndex
        dragSourcePortType: root.dragSourcePortType
        z: 1000  // Ensure it's above everything
        
        onNodeSelected: {
            // Clear catalog and drag state
            root.showNodeCatalog = false
            root.isDraggingHandle = false
            root.isEdgePreview = false
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
            }
        
        onDismissed: {
            // Clear catalog and drag state
            root.showNodeCatalog = false
            root.isDraggingHandle = false
            root.isEdgePreview = false
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
            console.log("Node catalog dismissed - edge preview mode disabled")
        }
    }
    
    // Connect to canvasClickedOutside to dismiss the node catalog
    Connections {
        target: root
        function onCanvasClickedOutside() {
            if (root.showNodeCatalog) {
                console.log("Canvas clicked outside - dismissing node catalog")
                nodeCatalogPopup.dismissed()
            }
        }
    }
    
    Component.onCompleted: {
        // Call base implementation
        centerViewAtOrigin()
        
        // Check if we're editing a design element
        if (root.canvas && root.canvas.editingElement) {
            console.log("ScriptCanvas: Editing element", root.canvas.editingElement.name, 
                       "ID:", root.canvas.editingElement.elementId)
        } else {
            console.log("ScriptCanvas: No element being edited")
        }
    }
    
    
    function handleHover(pt) {
        // Check if hovering over a node
        if (controller) {
            var element = controller.hitTest(pt.x, pt.y)
            hoveredPoint = pt
            if (element !== hoveredElement) {
                hoveredElement = element
                if (element && element.objectName === "Node") {
                } else if (!element && hoveredElement) {
                    console.log("Stopped hovering")
                }
            }
        }
    }
    
    function handleExit() {
        hoveredElement = null
    }
    
    
    // Pan to show a node at the given canvas position
    function panToNode(canvasPos) {
        // Calculate the content position to center the node in the viewport
        var centerPos = Utils.calculateCenterPosition(
            canvasPos,
            flickable.width,
            flickable.height,
            canvasMinX,
            canvasMinY,
            zoom
        )
        
        // Animate to the new position
        flickable.contentX = centerPos.x
        flickable.contentY = centerPos.y
    }
    
    
    // Helper function to check if a point is over a handle
    function getHandleAtPoint(point) {
        if (!elementModel) return null
        
        var elements = elementModel.getAllElements()
        for (var i = 0; i < elements.length; i++) {
            var element = elements[i]
            if (element && element.objectName === "Node") {
                // Special handling for Variable nodes
                if (element.nodeType === "Variable") {
                    // Variable nodes have a single source handle on the right edge of the header
                    var handleX = element.x + element.width - 15  // 5px margin + 10px to center
                    var handleY = element.y + 15  // Center of 30px header
                    
                    if (Math.abs(point.x - handleX) <= 10 && Math.abs(point.y - handleY) <= 10) {
                        return {
                            node: element,
                            handleType: "right",
                            portIndex: 0,  // Variable nodes have only one source port
                            portType: "String"  // Variables are always String type
                        }
                    }
                    continue  // Skip normal port checking for Variable nodes
                }
                
                var titleAndMargins = 30 + 15  // Title height + margins
                var columnsMargin = 10  // Left/right margin of columns container
                var columnSpacing = 20  // Spacing between columns
                
                // Get row configurations from the node
                var rowConfigs = element.rowConfigurations
                if (!rowConfigs) continue
                
                // Check target handles (left column)
                var targetIndex = 0
                for (var j = 0; j < rowConfigs.length; j++) {
                    var config = rowConfigs[j]
                    if (config.hasTarget) {
                        var handleY = element.y + titleAndMargins + targetIndex * 40 + 15  // center of 30px item
                        var handleX = element.x + columnsMargin + 10  // 10px to center of 20px handle
                        
                        if (Math.abs(point.x - handleX) <= 10 && Math.abs(point.y - handleY) <= 10) {
                            return {
                                node: element,
                                handleType: "left",
                                portIndex: config.targetPortIndex,
                                portType: config.targetType || "Flow"
                            }
                        }
                        targetIndex++
                    }
                }
                
                // Check source handles (right column)
                var sourceIndex = 0
                var columnWidth = (element.width - 2 * columnsMargin - columnSpacing) / 2
                var rightColumnX = element.x + columnsMargin + columnWidth + columnSpacing
                
                for (var k = 0; k < rowConfigs.length; k++) {
                    var sourceConfig = rowConfigs[k]
                    if (sourceConfig.hasSource) {
                        var sourceHandleY = element.y + titleAndMargins + sourceIndex * 40 + 15
                        var sourceHandleX = rightColumnX + columnWidth - 10  // 10px from right edge to center
                        
                        if (Math.abs(point.x - sourceHandleX) <= 10 && Math.abs(point.y - sourceHandleY) <= 10) {
                            return {
                                node: element,
                                handleType: "right",
                                portIndex: sourceConfig.sourcePortIndex,
                                portType: sourceConfig.sourceType || "Flow"
                            }
                        }
                        sourceIndex++
                    }
                }
            }
        }
        
        return null
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