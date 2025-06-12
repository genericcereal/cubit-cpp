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
        if (contentLayer) {
            edgesLoader.parent = contentLayer
            nodesLoader.parent = contentLayer
        }
        
        // Activate the loaders after setup
        activationTimer.start()
    }
    
    // Clean up on destruction
    Component.onDestruction: {
        edgesLoader.active = false
        nodesLoader.active = false
    }
    
    // Edges layer loader
    Loader {
        id: edgesLoader
        active: false
        asynchronous: false
        z: 0
        
        sourceComponent: Component {
            Item {
                id: edgesLayer
                anchors.fill: parent
                
                // Placeholder for edge rendering
                // Will be implemented when edges are added
            }
        }
        
        onLoaded: {
            if (item && parent) {
                item.anchors.fill = parent
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
            edgesLoader.active = true
            nodesLoader.active = true
            
            // Create default nodes after loaders are active
            createDefaultNodes()
        }
    }
    
    onVisibleChanged: {
        if (visible) {
            activationTimer.start()
        } else {
            edgesLoader.active = false
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
        // Will handle node hover when implemented
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
        // TODO: Add port configuration once Node port methods are exposed to QML
    }
    
    // Component definitions
    Component {
        id: nodeComponent
        NodeElement {}
    }
}