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
    
    // Click state for text input focus
    property bool clickedThisFrame: false
    property point clickPoint: Qt.point(0, 0)
    
    // Signal to notify when a click occurs
    signal canvasClicked(point clickPoint)
    
    // Signal to notify when dragging starts
    signal canvasDragStarted()
    
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
    property point dragStartPoint: Qt.point(0, 0)
    
    // Node catalog state
    property bool showNodeCatalog: false
    property point nodeCatalogPosition: Qt.point(0, 0)
    
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
                    var sourceX = rightColumnX + columnWidth - 10  // 10px from right edge to center
                    var sourceY = sourceNode.y + titleAndMargins + sourceIndex * 40 + 15  // center of 30px item
                    
                    // Find the target port index in the targets column
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
        
        // Temporary edge preview during handle drag
        Item {
            id: tempEdgeContainer
            visible: (root.isDraggingHandle || root.showNodeCatalog) && root.dragSourceNode
            anchors.fill: parent
            
            property point sourcePoint: {
                if (!root.dragSourceNode) return Qt.point(0, 0)
                
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
        }
    }
    
    // Fullscreen mouse area to catch clicks outside the catalog
    MouseArea {
        anchors.fill: parent
        visible: root.showNodeCatalog
        z: 999  // Just below the catalog popup
        
        onClicked: {
            console.log("Clicked outside catalog - dismissing")
            root.showNodeCatalog = false
            root.dragSourceNode = null
            root.dragSourceHandleType = ""
            root.dragSourcePortIndex = -1
            root.dragSourcePortType = "Flow"
        }
    }
    
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
        // Set click state and emit signal for text input focus
        clickedThisFrame = true
        clickPoint = canvasPoint
        canvasClicked(canvasPoint)
        
        
        if (controller.mode === "select") {
            // Check if we're over a handle first
            var handleInfo = getHandleAtPoint(canvasPoint)
            if (handleInfo) {
                // Start handle drag immediately
                isDraggingHandle = true
                dragSourceNode = handleInfo.node
                dragSourceHandleType = handleInfo.handleType
                dragSourcePortIndex = handleInfo.portIndex
                dragSourcePortType = handleInfo.portType
                dragCurrentPoint = canvasPoint
                dragStartPoint = canvasPoint
                console.log("Started dragging handle:", handleInfo.handleType, "port:", handleInfo.portIndex, "type:", handleInfo.portType, "from node:", handleInfo.node.nodeTitle)
            } else {
                // Store the start point and let the controller handle the press
                dragStartPoint = canvasPoint
                controller.handleMousePress(canvasPoint.x, canvasPoint.y)
            }
        }
        // Other modes will be handled when node creation is implemented
    }
    
    function handleMouseDrag(canvasPoint) {
        // Emit drag started signal on first drag movement
        if (!isDraggingHandle) {
            canvasDragStarted()
        }
        
        if (isDraggingHandle) {
            // Update drag position for edge preview
            dragCurrentPoint = canvasPoint
        } else {
            // Let the controller handle drag
            controller.handleMouseMove(canvasPoint.x, canvasPoint.y)
        }
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
        if (isDraggingHandle) {
            // End handle drag
            var targetHandleInfo = getHandleAtPoint(canvasPoint)
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
                dragStartPoint = Qt.point(0, 0)
            } else {
                // Show node catalog at release position
                showNodeCatalog = true
                nodeCatalogPosition = canvasPoint
                isDraggingHandle = false
                console.log("Showing node catalog at", canvasPoint.x, canvasPoint.y)
            }
        } else {
            // Check if this was a click on empty canvas (no selection box was drawn)
            var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
            if (!element && !selectionBoxHandler.active && selectionManager && selectionManager.hasSelection) {
                // Clear selection when clicking on empty canvas
                selectionManager.clearSelection()
            }
            
            // Let the controller handle release
            controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
            
            // Reset drag start point
            dragStartPoint = Qt.point(0, 0)
        }
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
        // Check if nodes already exist
        if (elementModel && elementModel.rowCount() > 0) {
            console.log("ScriptCanvas.createDefaultNodes: Nodes already exist, skipping creation")
            return
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