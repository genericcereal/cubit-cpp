import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."
import "../CanvasUtils.js" as Utils

BaseCanvas {
    id: root
    
    canvasType: "script"
    
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
    
    // Default nodes creator
    DefaultNodesCreator {
        id: defaultNodesCreator
        controller: root.controller
    }
    
    // Handle drag state
    property bool isDraggingHandle: false
    property var dragSourceNode: null
    property string dragSourceHandleType: ""  // "left" or "right"
    property int dragSourcePortIndex: -1
    property string dragSourcePortType: "Flow"  // "Flow", "Boolean", "String", or "Number"
    property point dragCurrentPoint: Qt.point(0, 0)
    
    // Node catalog state
    property bool showNodeCatalog: false
    property point nodeCatalogPosition: Qt.point(0, 0)
    
    // Add content into the default contentData
    contentData: [
        // Edges layer
        Item {
            id: edgesLayer
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
                        function onXChanged() { 
                            console.log("Edge source node X changed:", sourceNode.x)
                            updateEdgePositions() 
                        }
                        function onYChanged() { 
                            console.log("Edge source node Y changed:", sourceNode.y)
                            updateEdgePositions() 
                        }
                        function onWidthChanged() { updateEdgePositions() }
                    }
                    
                    Connections {
                        target: targetNode
                        enabled: targetNode !== null
                        function onXChanged() { 
                            console.log("Edge target node X changed:", targetNode.x)
                            updateEdgePositions() 
                        }
                        function onYChanged() { 
                            console.log("Edge target node Y changed:", targetNode.y)
                            updateEdgePositions() 
                        }
                        function onHeightChanged() { updateEdgePositions() }
                    }
                    
                    function updateEdgePositions() {
                        if (!edgeObj || !sourceNode || !targetNode) {
                            console.log("updateEdgePositions early return - edgeObj:", !!edgeObj, "sourceNode:", !!sourceNode, "targetNode:", !!targetNode)
                            return
                        }
                        console.log("updateEdgePositions called for edge", edgeObj.elementId)
                        
                        var sourceX, sourceY
                        
                        // Special handling for Variable nodes as source
                        if (sourceNode.nodeType === "Variable") {
                            sourceX = sourceNode.x + sourceNode.width - 15  // 5px margin + 10px to center
                            sourceY = sourceNode.y + 15  // Center of 30px header
                        } else {
                            var titleAndMargins = 30 + 15  // Title height + margins
                            var columnsMargin = 10  // Left/right margin of columns container
                            var columnSpacing = 20  // Spacing between columns
                            
                            // Find the source port index in the sources column
                            var sourceIndex = 0
                            var foundSource = false
                            for (var i = 0; i < sourceNode.rowConfigurations.length; i++) {
                                var config = sourceNode.rowConfigurations[i]
                                if (config.hasSource) {
                                    if (config.sourcePortIndex === edgeObj.sourcePortIndex) {
                                        foundSource = true
                                        break
                                    }
                                    sourceIndex++
                                }
                            }
                            
                            if (!foundSource) {
                                console.log("Warning: Could not find source port", edgeObj.sourcePortIndex)
                                return
                            }
                            
                            // Calculate source point (right column, right side)
                            var columnWidth = (sourceNode.width - 2 * columnsMargin - columnSpacing) / 2
                            var rightColumnX = sourceNode.x + columnsMargin + columnWidth + columnSpacing
                            sourceX = rightColumnX + columnWidth - 10  // 10px from right edge to center
                            sourceY = sourceNode.y + titleAndMargins + sourceIndex * 40 + 15  // center of 30px item
                        }
                        
                        // Find the target port index in the targets column
                        var titleAndMargins = 30 + 15  // Title height + margins
                        var columnsMargin = 10  // Left/right margin of columns container
                        
                        var targetIndex = 0
                        var foundTarget = false
                        for (var j = 0; j < targetNode.rowConfigurations.length; j++) {
                            var targetConfig = targetNode.rowConfigurations[j]
                            if (targetConfig.hasTarget) {
                                if (targetConfig.targetPortIndex === edgeObj.targetPortIndex) {
                                    foundTarget = true
                                    break
                                }
                                targetIndex++
                            }
                        }
                        
                        if (!foundTarget) {
                            console.log("Warning: Could not find target port", edgeObj.targetPortIndex)
                            return
                        }
                        
                        // Calculate target point (left column, left side)
                        var targetX = targetNode.x + columnsMargin + 10  // 10px to center of 20px handle
                        var targetY = targetNode.y + titleAndMargins + targetIndex * 40 + 15
                        
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
                
                // Create Variable node for ALL element types (Frame, Text, Html, Variable)
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
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
        }
        
        onDismissed: {
            // Clear catalog and drag state
            root.showNodeCatalog = false
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
        }
    }
    
    Component.onCompleted: {
        // Call base implementation
        centerViewAtOrigin()
        
        // Check if we're editing a design element
        if (Application.activeCanvas && Application.activeCanvas.editingElement) {
            console.log("ScriptCanvas: Editing element", Application.activeCanvas.editingElement.name, 
                       "ID:", Application.activeCanvas.editingElement.elementId)
        } else {
            console.log("ScriptCanvas: No element being edited")
        }
        
        // Create default nodes after a delay
        delayTimer.start()
    }
    
    Timer {
        id: delayTimer
        interval: 100
        repeat: false
        onTriggered: createDefaultNodes()
    }
    
    // Implement behavior by overriding handler functions
    function handleDragStart(pt) {
        // Set click state but don't emit signal yet - wait for release to confirm it's a click
        clickedThisFrame = true
        clickPoint = pt
        
        console.log("ScriptCanvas.handleDragStart called at", pt.x, pt.y)
        
        if (controller.mode === "select") {
            // Check if we're over a handle first
            var handleInfo = getHandleAtPoint(pt)
            console.log("getHandleAtPoint returned:", handleInfo ? "handle found" : "no handle")
            if (handleInfo) {
                // Start handle drag immediately
                isDraggingHandle = true
                dragSourceNode = handleInfo.node
                dragSourceHandleType = handleInfo.handleType
                dragSourcePortIndex = handleInfo.portIndex
                dragSourcePortType = handleInfo.portType
                dragCurrentPoint = pt
                console.log("Started dragging handle:", handleInfo.handleType, "port:", handleInfo.portIndex, "type:", handleInfo.portType, "from node:", handleInfo.node.nodeTitle)
            } else {
                // Let the controller handle the press
                console.log("No handle found, calling controller.handleMousePress")
                controller.handleMousePress(pt.x, pt.y)
            }
        }
        // Other modes will be handled when node creation is implemented
    }
    
    function handleDragMove(pt) {
        // Emit drag started signal on first drag movement
        if (!isDraggingHandle) {
            canvasDragStarted()
        }
        
        if (isDraggingHandle) {
            // Update drag position for edge preview
            dragCurrentPoint = pt
        } else {
            // Let the controller handle drag
            controller.handleMouseMove(pt.x, pt.y)
        }
    }
    
    function handleDragEnd(pt) {
        // Check if this was a click (not a drag)
        var wasClick = !isDraggingHandle && !selectionBoxHandler.active && 
                      Math.abs(pt.x - clickPoint.x) < 5 && 
                      Math.abs(pt.y - clickPoint.y) < 5
        
        if (wasClick) {
            // If node catalog is showing, dismiss it
            if (root.showNodeCatalog) {
                console.log("Click detected in handleDragEnd - dismissing node catalog")
                root.showNodeCatalog = false
                root.dragSourceNode = null
                root.dragSourceHandleType = ""
                root.dragSourcePortIndex = -1
                root.dragSourcePortType = "Flow"
                return
            }
            
            // Check if clicking on a node or empty space
            var element = controller.hitTest(pt.x, pt.y)
            if (element && element.objectName === "Node") {
                // Clicked on a node - emit click signal for input handling
                canvasClicked(pt)
            } else {
                // Clicked outside any node - emit outside click signal
                canvasClickedOutside()
            }
        }
        
        if (isDraggingHandle) {
            // End handle drag
            var targetHandleInfo = getHandleAtPoint(pt)
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
                // Reset drag state
                isDraggingHandle = false
                dragSourceNode = null
                dragSourceHandleType = ""
                dragSourcePortIndex = -1
                dragSourcePortType = "Flow"
            } else {
                // Show node catalog at release position
                showNodeCatalog = true
                nodeCatalogPosition = pt
                isDraggingHandle = false
                console.log("Showing node catalog at", pt.x, pt.y)
            }
        } else {
            // Check if this was a click on empty canvas (no selection box was drawn)
            var element = controller.hitTest(pt.x, pt.y)
            if (!element && !selectionBoxHandler.active && selectionManager && selectionManager.hasSelection) {
                // Clear selection when clicking on empty canvas
                selectionManager.clearSelection()
            }
            
            // Let the controller handle release
            controller.handleMouseRelease(pt.x, pt.y)
        }
    }
    
    function handleClick(pt) {
        // Dismiss node catalog if it's showing
        if (root.showNodeCatalog) {
            console.log("Click detected - dismissing node catalog")
            root.showNodeCatalog = false
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
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
                    console.log("Started hovering over node:", element.nodeTitle)
                } else if (!element && hoveredElement) {
                    console.log("Stopped hovering")
                }
            }
        }
    }
    
    function handleExit() {
        hoveredElement = null
    }
    
    function handleSelectionRect(rect) {
        console.log("=== ScriptCanvas.handleSelectionRect called ===")
        console.log("Selection rect:", JSON.stringify({x: rect.x, y: rect.y, width: rect.width, height: rect.height}))
        
        // Select all nodes that intersect with the selection rectangle
        if (!controller || !elementModel || !selectionManager) {
            console.log("ERROR: Missing required components")
            return
        }
        
        var elements = elementModel.getAllElements()
        console.log("Total elements found:", elements.length)
        
        var selectedNodes = []
        
        for (var i = 0; i < elements.length; i++) {
            var element = elements[i]
            
            // Only consider Node elements
            if (element && element.objectName === "Node") {
                // Check if element intersects with selection rect
                var elementRect = Qt.rect(element.x, element.y, element.width, element.height)
                
                // Check for intersection
                var intersects = rect.x < elementRect.x + elementRect.width &&
                    rect.x + rect.width > elementRect.x &&
                    rect.y < elementRect.y + elementRect.height &&
                    rect.y + rect.height > elementRect.y
                
                if (intersects) {
                    selectedNodes.push(element)
                }
            }
        }
        
        console.log("Total nodes selected:", selectedNodes.length)
        
        // Update selection
        if (selectedNodes.length > 0) {
            selectionManager.clearSelection()
            for (var j = 0; j < selectedNodes.length; j++) {
                selectionManager.selectElement(selectedNodes[j])
            }
        }
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
    
    // Create default nodes
    function createDefaultNodes() {
        // Check if nodes already exist (not just any elements)
        if (elementModel) {
            var elements = elementModel.getAllElements()
            for (var i = 0; i < elements.length; i++) {
                if (elements[i] && elements[i].objectName === "Node") {
                    console.log("ScriptCanvas.createDefaultNodes: Nodes already exist, skipping creation")
                    return
                }
            }
        }
        
        defaultNodesCreator.createDefaultNodes()
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