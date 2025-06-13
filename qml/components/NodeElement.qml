import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

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
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Hover state - check if this element is hovered in the canvas
    property bool elementHovered: canvas && canvas.hoveredElement === element
    
    // Track the currently active TextField
    property var activeTextField: null
    
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
            // Check if click is within this node
            if (clickPoint.x >= element.x && clickPoint.x <= element.x + element.width &&
                clickPoint.y >= element.y && clickPoint.y <= element.y + element.height) {
                // Node was clicked, check if it's on a text input
                root.handleClick(clickPoint)
            } else if (root.activeTextField) {
                // Click outside of node, blur active TextField
                root.activeTextField.focus = false
                root.activeTextField = null
                console.log("Blurred active TextField due to click outside node")
            }
        }
        
        function onCanvasDragStarted() {
            // Blur active TextField when dragging starts
            if (root.activeTextField) {
                root.activeTextField.focus = false
                root.activeTextField = null
                console.log("Blurred active TextField due to drag start")
            }
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
    
    // Handle click to focus text inputs
    function handleClick(clickPoint) {
        // Get the click position relative to the node
        var localX = clickPoint.x - element.x
        var localY = clickPoint.y - element.y
        
        var clickedOnTextField = false
        
        // Check each row in the rowContainer
        var titleAndMargins = titleText.height + titleText.anchors.topMargin + rowContainer.anchors.topMargin
        
        // Check if click is within the rows area
        if (localY > titleAndMargins) {
            var rowLocalY = localY - titleAndMargins
            var rowIndex = Math.floor(rowLocalY / 40) // 30px row height + 10px spacing
            
            // Get the row item if it exists through the repeater
            if (rowIndex >= 0 && rowIndex < rowRepeater.count) {
                var row = rowRepeater.itemAt(rowIndex)
                if (row && row.hasTarget && row.targetType === "Variable" && !row.hasIncomingEdge) {
                    // Check if click is on the text input
                    var inputX = 20 + (row.targetLabel !== "" ? 50 : 25) // targetHandle width + margins
                    var inputWidth = 80
                    var rowLocalX = localX - rowContainer.anchors.leftMargin
                    
                    if (rowLocalX >= inputX && rowLocalX <= inputX + inputWidth) {
                        // Focus the text input using the alias
                        if (row.variableInput) {
                            // Blur any previously active field
                            if (activeTextField && activeTextField !== row.variableInput) {
                                activeTextField.focus = false
                            }
                            
                            row.variableInput.forceActiveFocus()
                            activeTextField = row.variableInput
                            clickedOnTextField = true
                            console.log("Focused TextField at row", rowIndex, "port:", row.targetPortIndex)
                        }
                    }
                }
            }
        }
        
        // If we didn't click on a text field, blur the active one
        if (!clickedOnTextField && activeTextField) {
            activeTextField.focus = false
            activeTextField = null
            console.log("Blurred active TextField")
        }
    }
    
    // Simple node visual representation for debugging
    Rectangle {
        id: nodeBody
        anchors.fill: parent
        
        // Node-specific properties
        color: "#E6F3FF"  // Hardcoded light blue
        border.width: 2
        border.color: "red"  // Red border for visibility
        radius: 8
        
        Component.onCompleted: {
            console.log("NodeElement Rectangle - color:", color)
            console.log("NodeElement Rectangle - size:", width, "x", height)
            console.log("NodeElement Rectangle - visible:", visible)
            console.log("NodeElement Rectangle - opacity:", opacity)
        }
        
        // Title header
        Text {
            id: titleText
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 10
            text: nodeElement ? nodeElement.nodeTitle : "Node"
            color: "black"
            font.pixelSize: 16
            font.bold: true
        }
        
        // Container for NodeRow components
        Column {
            id: rowContainer
            anchors.top: titleText.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            anchors.topMargin: 15
            spacing: 10
            
            // NodeRow component definition
            component NodeRow: Item {
                id: nodeRow
                width: parent.width
                height: 30
                
                property int rowIndex: 0
                property alias variableInput: variableInput
                
                
                // Target (input) properties
                property bool hasTarget: false
                property string targetLabel: ""
                property string targetType: "Flow"  // "Flow" or "Variable"
                property int targetPortIndex: -1
                
                // Source (output) properties  
                property bool hasSource: false
                property string sourceLabel: ""
                property string sourceType: "Flow"  // "Flow" or "Variable"
                property int sourcePortIndex: -1
                
                // Check if this target port has an incoming edge
                property bool hasIncomingEdge: {
                    if (!nodeRow.hasTarget || !root.elementModel || !root.element) return false
                    
                    // Use modelUpdateCount to trigger re-evaluation when model changes
                    var dummy = root.modelUpdateCount
                    
                    var edges = root.elementModel.getAllElements()
                    for (var i = 0; i < edges.length; i++) {
                        var edge = edges[i]
                        if (edge && edge.objectName === "Edge") {
                            // Check if this edge connects to our target port
                            if (edge.targetNodeId === root.element.elementId && 
                                edge.targetPortIndex === nodeRow.targetPortIndex) {
                                console.log("Found incoming edge for port", nodeRow.targetPortIndex, "on node", root.element.nodeTitle)
                                return true
                            }
                        }
                    }
                    return false
                }
                
                // Check if target (left) handle is hovered
                property bool targetHandleHovered: {
                    if (!nodeRow.hasTarget || !root.elementHovered || !root.canvas) return false
                    
                    // Get the hover point relative to this nodeRow
                    var localX = root.canvas.hoveredPoint.x - root.element.x - rowContainer.anchors.leftMargin - nodeRow.x
                    var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - rowContainer.anchors.topMargin - nodeRow.y
                    
                    // Check if point is within the left handle bounds
                    return localX >= 0 && localX <= 20 && 
                           localY >= 5 && localY <= 25  // vertically centered, 30px tall row
                }
                
                // Check if source (right) handle is hovered
                property bool sourceHandleHovered: {
                    if (!nodeRow.hasSource || !root.elementHovered || !root.canvas) return false
                    
                    // Get the hover point relative to this nodeRow
                    var localX = root.canvas.hoveredPoint.x - root.element.x - rowContainer.anchors.leftMargin - nodeRow.x
                    var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - rowContainer.anchors.topMargin - nodeRow.y
                    
                    // Check if point is within the right handle bounds
                    var rightHandleX = nodeRow.width - 20
                    return localX >= rightHandleX && localX <= nodeRow.width && 
                           localY >= 5 && localY <= 25  // vertically centered, 30px tall row
                }
                
                // Check if the text input is hovered
                property bool textInputHovered: {
                    if (!nodeRow.hasTarget || nodeRow.targetType !== "Variable" || nodeRow.hasIncomingEdge) return false
                    if (!root.elementHovered || !root.canvas) return false
                    
                    // Get the hover point relative to this nodeRow
                    var localX = root.canvas.hoveredPoint.x - root.element.x - rowContainer.anchors.leftMargin - nodeRow.x
                    var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - rowContainer.anchors.topMargin - nodeRow.y
                    
                    // Calculate text input bounds
                    var inputX = targetHandle.width + (nodeRow.targetLabel !== "" ? 50 : 25)
                    var inputWidth = 80
                    var inputY = (nodeRow.height - 24) / 2  // vertically centered, 24px tall input
                    var inputHeight = 24
                    
                    // Check if point is within the text input bounds
                    return localX >= inputX && localX <= inputX + inputWidth && 
                           localY >= inputY && localY <= inputY + inputHeight
                }
                
                // Target (left) handle
                Rectangle {
                    id: targetHandle
                    visible: nodeRow.hasTarget
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    radius: nodeRow.targetType === "Flow" ? 10 : 0  // Circle for Flow, Square for Variable
                    color: {
                        if (nodeRow.targetHandleHovered) {
                            return nodeRow.targetType === "Flow" ? "#4CAF50" : "#FF9800"  // Green for Flow, Orange for Variable
                        } else {
                            return nodeRow.targetType === "Flow" ? "#666666" : "#795548"  // Gray for Flow, Brown for Variable
                        }
                    }
                    border.width: 2
                    border.color: {
                        if (nodeRow.targetHandleHovered) {
                            return nodeRow.targetType === "Flow" ? "#2E7D32" : "#E65100"
                        } else {
                            return nodeRow.targetType === "Flow" ? "#333333" : "#5D4037"
                        }
                    }
                }
                
                // Target label (left side)
                Text {
                    visible: nodeRow.hasTarget && nodeRow.targetLabel !== ""
                    anchors.left: targetHandle.right
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    text: nodeRow.targetLabel
                    color: "#333333"
                    font.pixelSize: 12
                }
                
                // Variable input field (only for Variable type targets without edges)
                TextField {
                    id: variableInput
                    visible: nodeRow.hasTarget && nodeRow.targetType === "Variable" && !nodeRow.hasIncomingEdge
                    anchors.left: targetHandle.right
                    anchors.leftMargin: nodeRow.targetLabel !== "" ? 50 : 25  // More space if there's a label
                    anchors.verticalCenter: parent.verticalCenter
                    width: 80
                    height: 24
                    text: "0"  // Default value
                    font.pixelSize: 11
                    horizontalAlignment: TextInput.AlignHCenter
                    
                    onVisibleChanged: {
                        console.log("TextField visibility changed to:", visible, 
                                   "for port", nodeRow.targetPortIndex,
                                   "hasIncomingEdge:", nodeRow.hasIncomingEdge)
                    }
                    
                    background: Rectangle {
                        color: nodeRow.textInputHovered ? "#E8F4FD" : "#F5F5F5"
                        border.color: variableInput.activeFocus ? "#2196F3" : (nodeRow.textInputHovered ? "#90CAF9" : "#CCCCCC")
                        border.width: nodeRow.textInputHovered ? 2 : 1
                        radius: 3
                        
                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                        Behavior on border.color {
                            ColorAnimation { duration: 150 }
                        }
                    }
                    
                    onEditingFinished: {
                        console.log("Variable input value changed to:", text, "for port:", nodeRow.targetPortIndex)
                        // TODO: Store this value in the node's data
                    }
                }
                
                // Source label (right side)
                Text {
                    visible: nodeRow.hasSource && nodeRow.sourceLabel !== ""
                    anchors.right: sourceHandle.left
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    text: nodeRow.sourceLabel
                    color: "#333333"
                    font.pixelSize: 12
                }
                
                // Source (right) handle
                Rectangle {
                    id: sourceHandle
                    visible: nodeRow.hasSource
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    radius: nodeRow.sourceType === "Flow" ? 10 : 0  // Circle for Flow, Square for Variable
                    color: {
                        if (nodeRow.sourceHandleHovered) {
                            return nodeRow.sourceType === "Flow" ? "#2196F3" : "#FF9800"  // Blue for Flow, Orange for Variable
                        } else {
                            return nodeRow.sourceType === "Flow" ? "#666666" : "#795548"  // Gray for Flow, Brown for Variable
                        }
                    }
                    border.width: 2
                    border.color: {
                        if (nodeRow.sourceHandleHovered) {
                            return nodeRow.sourceType === "Flow" ? "#1565C0" : "#E65100"
                        } else {
                            return nodeRow.sourceType === "Flow" ? "#333333" : "#5D4037"
                        }
                    }
                }
            }
            
            // Dynamic row generation from node configuration
            Repeater {
                id: rowRepeater
                model: nodeElement ? nodeElement.rowConfigurations : []
                
                NodeRow {
                    rowIndex: index
                    hasTarget: modelData.hasTarget || false
                    targetLabel: modelData.targetLabel || ""
                    targetType: modelData.targetType || "Flow"
                    targetPortIndex: modelData.targetPortIndex || -1
                    hasSource: modelData.hasSource || false
                    sourceLabel: modelData.sourceLabel || ""
                    sourceType: modelData.sourceType || "Flow"
                    sourcePortIndex: modelData.sourcePortIndex || -1
                }
            }
        }
    }
}