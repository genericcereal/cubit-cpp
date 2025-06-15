import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0
import ".."

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property var canvas  // Reference to the canvas
    property Node nodeElement: element as Node
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 200
    height: element ? element.height : 180
    
    // Track row configurations changes
    property int rowConfigCount: nodeElement ? nodeElement.rowConfigurations.length : 0
    
    // Calculate the required height based on content
    property real calculatedHeight: {
        // Force re-evaluation when rowConfigCount changes
        var dummy = rowConfigCount
        
        if (!nodeElement || !nodeElement.rowConfigurations) return 180
        
        var titleHeight = Config.nodeHeaderHeight  // Node header height from Config
        var topMargin = 10    // Title top margin
        var titleBottomMargin = 15  // Space between title and columns
        var containerBottomMargin = Config.nodeBottomMargin   // Bottom margin from Config
        var rowHeight = 30   // Height of each row
        var rowSpacing = 10  // Spacing between rows
        
        var numRows = nodeElement.rowConfigurations.length
        var contentHeight = numRows * rowHeight + (numRows > 0 ? (numRows - 1) * rowSpacing : 0)
        
        var totalHeight = topMargin + titleHeight + titleBottomMargin + contentHeight + containerBottomMargin
        
        // Ensure minimum height from Config
        return Math.max(totalHeight, Config.nodeMinHeight)
    }
    
    // Update element height when calculated height changes
    onCalculatedHeightChanged: {
        if (element && Math.abs(element.height - calculatedHeight) > 1) {
            element.height = calculatedHeight
        }
    }
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Hover state - check if this element is hovered in the canvas
    property bool elementHovered: canvas && canvas.hoveredElement === element
    
    // Track the currently active ComboBox (no longer needed with PortInput)
    // property var activeComboBox: null
    
    // Force re-evaluation of bindings when model changes
    property int modelUpdateCount: 0
    
    Connections {
        target: elementModel
        function onRowsInserted() {
            root.modelUpdateCount++
        }
        function onRowsRemoved() {
            root.modelUpdateCount++
        }
        function onDataChanged() {
            root.modelUpdateCount++
        }
    }
    
    // Listen for canvas clicks to focus text inputs
    Connections {
        target: canvas
        function onCanvasClicked(clickPoint) {
            console.log("Canvas clicked at:", clickPoint.x, clickPoint.y)
            console.log("Node bounds:", element.x, element.y, "to", element.x + element.width, element.y + element.height)
            
            // Check if click is within this node
            if (clickPoint.x >= element.x && clickPoint.x <= element.x + element.width &&
                clickPoint.y >= element.y && clickPoint.y <= element.y + element.height) {
                // Node was clicked, check if it's on a text input
                console.log("Click is within node bounds")
                root.handleClick(clickPoint)
            }
        }
        
        function onCanvasClickedOutside() {
            console.log("Canvas clicked outside - clearing focus if any input has focus")
            // Simply clear focus - the focus system will handle it properly
            root.forceActiveFocus()
            
            // Also close any open ComboBox popups
            for (var i = 0; i < targetsColumn.children.length; i++) {
                var targetItem = targetsColumn.children[i]
                if (targetItem && targetItem.children.length > 1) {
                    var portInput = targetItem.children[1]
                    if (portInput && typeof portInput.blur === 'function') {
                        portInput.blur()
                    }
                }
            }
        }
        
        function onCanvasDragStarted() {
            // PortInput components will handle their own state
        }
    }
    
    // Log node properties when element changes
    onElementChanged: {
        if (element) {
            console.log("NodeElement created/updated:")
            console.log("  elementId:", element.elementId)
            console.log("  objectName:", element.objectName)
            console.log("  position:", element.x, ",", element.y)
            console.log("  size:", element.width, "x", element.height)
            console.log("  nodeTitle:", nodeElement ? nodeElement.nodeTitle : "null")
        }
    }
    
    // Handle click to open combo boxes
    function handleClick(clickPoint) {
        // Get the click position relative to the node
        var localX = clickPoint.x - element.x
        var localY = clickPoint.y - element.y
        
        console.log("Node clicked at local position:", localX, localY)
        console.log("  Node position:", element.x, element.y)
        console.log("  Click point:", clickPoint.x, clickPoint.y)
        
        // Check targets column
        var titleAndMargins = nodeHeader.height + columnsContainer.anchors.topMargin
        
        // Check if click is within the columns area
        if (localY > titleAndMargins) {
            var columnsLocalY = localY - titleAndMargins
            var columnsLocalX = localX - columnsContainer.anchors.leftMargin
            
            // Check if click is in the targets column
            if (columnsLocalX >= 0 && columnsLocalX <= targetsColumn.width) {
                // Find which target item was clicked
                var targetIndex = Math.floor(columnsLocalY / 40) // 30px height + 10px spacing
                
                if (targetIndex >= 0 && targetIndex < targetsColumn.children.length) {
                    var targetItem = targetsColumn.children[targetIndex]
                    if (targetItem && targetItem.children.length > 1) {
                        var portInput = targetItem.children[1] // PortInput is the second child
                        if (portInput && portInput.visible) {
                            // Check if click is on the port input
                            var inputX = 25 // handle width + margin
                            var inputWidth = 80
                            
                            if (columnsLocalX >= inputX && columnsLocalX <= inputX + inputWidth) {
                                // Call the handleClick method on the PortInput
                                portInput.handleClick()
                                console.log("Clicked on port input for target port:", targetItem.targetPortIndex)
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Simple node visual representation for debugging
    Rectangle {
        id: nodeBody
        anchors.fill: parent
        
        // Node-specific properties
        color: "#E6F3FF"  // Hardcoded light blue
        border.width: 2
        border.color: "#CCCCCC"  // Light gray border
        radius: 0  // No radius for sharp corners to match header
        
        Component.onCompleted: {
            console.log("NodeElement Rectangle - color:", color)
            console.log("NodeElement Rectangle - size:", width, "x", height)
            console.log("NodeElement Rectangle - visible:", visible)
            console.log("NodeElement Rectangle - opacity:", opacity)
        }
        
        // Node header
        NodeHeader {
            id: nodeHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            nodeName: nodeElement ? nodeElement.nodeTitle : "Node"
            nodeType: nodeElement && nodeElement.nodeType ? nodeElement.nodeType : "Operation"
            z: 1  // Ensure header is above the node body
        }
        
        // Two column layout for targets and sources
        Row {
            id: columnsContainer
            anchors.top: nodeHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            anchors.topMargin: 15
            spacing: 20
            
            // Left column - Target handles
            Column {
                id: targetsColumn
                width: (parent.width - parent.spacing) / 2
                spacing: 10
                
                Repeater {
                    model: {
                        if (!nodeElement || !nodeElement.rowConfigurations) return []
                        // Use all row configurations, not just those with targets
                        return nodeElement.rowConfigurations
                    }
                    
                    delegate: Item {
                        width: parent.width
                        height: 30
                        
                        property var targetConfig: modelData
                        property bool hasTarget: targetConfig.hasTarget || false
                        property int targetPortIndex: hasTarget ? (targetConfig.targetPortIndex || -1) : -1
                        property string targetType: hasTarget ? (targetConfig.targetType || "Flow") : ""
                        property string targetLabel: hasTarget ? (targetConfig.targetLabel || "") : ""
                        
                        // Check if this target port has an incoming edge
                        property bool hasIncomingEdge: {
                            if (!hasTarget || !root.elementModel || !root.element) return false
                            
                            var dummy = root.modelUpdateCount
                            var edges = root.elementModel.getAllElements()
                            for (var i = 0; i < edges.length; i++) {
                                var edge = edges[i]
                                if (edge && edge.objectName === "Edge") {
                                    if (edge.targetNodeId === root.element.elementId && 
                                        edge.targetPortIndex === targetPortIndex) {
                                        return true
                                    }
                                }
                            }
                            return false
                        }
                        
                        // Check if handle is hovered
                        property bool handleHovered: {
                            if (!hasTarget || !root.elementHovered || !root.canvas) return false
                            
                            var localX = root.canvas.hoveredPoint.x - root.element.x - columnsContainer.anchors.leftMargin
                            var localY = root.canvas.hoveredPoint.y - root.element.y - nodeHeader.height - columnsContainer.anchors.topMargin - y
                            
                            return localX >= 0 && localX <= 20 && 
                                   localY >= 5 && localY <= 25
                        }
                        
                        // Check if combo box is hovered
                        property bool comboBoxHovered: {
                            if (!hasTarget || targetType === "Flow" || hasIncomingEdge) return false
                            if (!root.elementHovered || !root.canvas) return false
                            
                            var localX = root.canvas.hoveredPoint.x - root.element.x - columnsContainer.anchors.leftMargin
                            var localY = root.canvas.hoveredPoint.y - root.element.y - nodeHeader.height - columnsContainer.anchors.topMargin - y
                            
                            var inputX = 25
                            var inputWidth = 80
                            var inputY = 3
                            var inputHeight = 24
                            
                            return localX >= inputX && localX <= inputX + inputWidth && 
                                   localY >= inputY && localY <= inputY + inputHeight
                        }
                        
                        // Target handle
                        Rectangle {
                            id: targetHandle
                            visible: hasTarget
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            width: 20
                            height: 20
                            radius: targetType === "Flow" ? 10 : 0
                            color: {
                                if (handleHovered) {
                                    return targetType === "Flow" ? "#4CAF50" : "#FF9800"
                                } else {
                                    return targetType === "Flow" ? "#666666" : "#795548"
                                }
                            }
                            border.width: 2
                            border.color: {
                                if (handleHovered) {
                                    return targetType === "Flow" ? "#2E7D32" : "#E65100"
                                } else {
                                    return targetType === "Flow" ? "#333333" : "#5D4037"
                                }
                            }
                        }
                        
                        // Port input using PortInput component
                        PortInput {
                            id: targetInput
                            visible: hasTarget && targetType !== "Flow" && !hasIncomingEdge
                            anchors.left: targetHandle.right
                            anchors.leftMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            
                            portType: targetType
                            portIndex: targetPortIndex
                            hasIncomingEdge: hasIncomingEdge
                            isHovered: comboBoxHovered
                            canvas: root.canvas
                            
                            onPortValueChanged: function(newValue) {
                                console.log("Target port", targetPortIndex, "value changed to:", newValue)
                                // TODO: Connect this to the node's data model
                            }
                        }
                        
                        Text {
                            visible: hasTarget && targetLabel !== "" && (targetType === "Flow" || hasIncomingEdge)
                            anchors.left: targetHandle.right
                            anchors.leftMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            text: targetLabel
                            color: "#333333"
                            font.pixelSize: 12
                        }
                    }
                }
            }
            
            // Right column - Source handles
            Column {
                id: sourcesColumn
                width: (parent.width - parent.spacing) / 2
                spacing: 10
                
                Repeater {
                    model: {
                        if (!nodeElement || !nodeElement.rowConfigurations) return []
                        // Use all row configurations, not just those with sources
                        return nodeElement.rowConfigurations
                    }
                    
                    delegate: Item {
                        width: parent.width
                        height: 30
                        
                        property var sourceConfig: modelData
                        property bool hasSource: sourceConfig.hasSource || false
                        property int sourcePortIndex: hasSource ? (sourceConfig.sourcePortIndex || -1) : -1
                        property string sourceType: hasSource ? (sourceConfig.sourceType || "Flow") : ""
                        property string sourceLabel: hasSource ? (sourceConfig.sourceLabel || "") : ""
                        
                        // Check if handle is hovered
                        property bool handleHovered: {
                            if (!hasSource || !root.elementHovered || !root.canvas) return false
                            
                            var localX = root.canvas.hoveredPoint.x - root.element.x - columnsContainer.anchors.leftMargin - sourcesColumn.x
                            var localY = root.canvas.hoveredPoint.y - root.element.y - nodeHeader.height - columnsContainer.anchors.topMargin - y
                            
                            var handleX = parent.width - 20
                            return localX >= handleX && localX <= parent.width && 
                                   localY >= 5 && localY <= 25
                        }
                        
                        // Source label
                        Text {
                            visible: hasSource && sourceLabel !== ""
                            anchors.right: sourceHandle.left
                            anchors.rightMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            text: sourceLabel
                            color: "#333333"
                            font.pixelSize: 12
                        }
                        
                        // Source handle
                        Rectangle {
                            id: sourceHandle
                            visible: hasSource
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            width: 20
                            height: 20
                            radius: sourceType === "Flow" ? 10 : 0
                            color: {
                                if (handleHovered) {
                                    return sourceType === "Flow" ? "#2196F3" : "#FF9800"
                                } else {
                                    return sourceType === "Flow" ? "#666666" : "#795548"
                                }
                            }
                            border.width: 2
                            border.color: {
                                if (handleHovered) {
                                    return sourceType === "Flow" ? "#1565C0" : "#E65100"
                                } else {
                                    return sourceType === "Flow" ? "#333333" : "#5D4037"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}