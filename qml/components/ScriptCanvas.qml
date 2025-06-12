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
        // Let the controller handle release
        controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
    }
    
    function handleMouseExit() {
        // Will clear hover state when implemented
    }
    
    function handleSelectionRect(rect) {
        // Will select nodes in rect when implemented
        // For now, no elements to select
    }
    
    // Create default nodes
    function createDefaultNodes() {
        if (!controller || !elementModel) return
        
        // Create first node - use light blue from Config
        controller.createNode(-150, -50, "Start Node", "")
        
        // Create second node - also light blue
        controller.createNode(50, -50, "Process Node", "")
        
        // TODO: Add port configuration once Node port methods are exposed to QML
    }
    
    // Component definitions
    Component {
        id: nodeComponent
        NodeElement {}
    }
}