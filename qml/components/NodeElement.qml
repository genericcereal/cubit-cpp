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
        
        var titleHeight = 30  // Title text height estimate
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
    
    // Track the currently active ComboBox
    property var activeComboBox: null
    
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
            } else if (root.activeComboBox) {
                // Click outside of node, close active ComboBox
                root.activeComboBox.popup.close()
                root.activeComboBox = null
                console.log("Closed active ComboBox due to click outside node")
            }
        }
        
        function onCanvasDragStarted() {
            // Close active ComboBox when dragging starts
            if (root.activeComboBox) {
                root.activeComboBox.popup.close()
                root.activeComboBox = null
                console.log("Closed active ComboBox due to drag start")
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
    
    // Handle click to open combo boxes
    function handleClick(clickPoint) {
        // Get the click position relative to the node
        var localX = clickPoint.x - element.x
        var localY = clickPoint.y - element.y
        
        var clickedOnComboBox = false
        
        // Check targets column
        var titleAndMargins = titleText.height + titleText.anchors.topMargin + columnsContainer.anchors.topMargin
        
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
                        var comboBox = targetItem.children[1] // ComboBox is the second child
                        if (comboBox && comboBox.visible) {
                            // Check if click is on the combo box
                            var comboX = 25 // handle width + margin
                            var comboWidth = 80
                            
                            if (columnsLocalX >= comboX && columnsLocalX <= comboX + comboWidth) {
                                // Close any previously active combo box
                                if (activeComboBox && activeComboBox !== comboBox) {
                                    activeComboBox.popup.close()
                                }
                                
                                comboBox.popup.open()
                                activeComboBox = comboBox
                                clickedOnComboBox = true
                                console.log("Opened ComboBox for target port:", targetItem.targetPortIndex)
                            }
                        }
                    }
                }
            }
        }
        
        // If we didn't click on a combo box, close the active one
        if (!clickedOnComboBox && activeComboBox) {
            activeComboBox.popup.close()
            activeComboBox = null
            console.log("Closed active ComboBox")
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
        
        // Two column layout for targets and sources
        Row {
            id: columnsContainer
            anchors.top: titleText.bottom
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
                            var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - columnsContainer.anchors.topMargin - y
                            
                            return localX >= 0 && localX <= 20 && 
                                   localY >= 5 && localY <= 25
                        }
                        
                        // Check if combo box is hovered
                        property bool comboBoxHovered: {
                            if (!hasTarget || targetType === "Flow" || hasIncomingEdge) return false
                            if (!root.elementHovered || !root.canvas) return false
                            
                            var localX = root.canvas.hoveredPoint.x - root.element.x - columnsContainer.anchors.leftMargin
                            var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - columnsContainer.anchors.topMargin - y
                            
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
                        
                        // Variable input or label
                        ComboBox {
                            id: targetComboBox
                            visible: hasTarget && targetType !== "Flow" && !hasIncomingEdge
                            anchors.left: targetHandle.right
                            anchors.leftMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            width: 80
                            height: 24
                            font.pixelSize: 11
                            currentIndex: 0
                            model: ["Option 1", "Option 2", "Option 3", "Option 4", "Option 5"]
                            
                            
                            background: Rectangle {
                                color: comboBoxHovered ? "#E8F4FD" : "#F5F5F5"
                                border.color: targetComboBox.activeFocus ? "#2196F3" : (comboBoxHovered ? "#90CAF9" : "#CCCCCC")
                                border.width: comboBoxHovered ? 2 : 1
                                radius: 3
                                
                                Behavior on color {
                                    ColorAnimation { duration: 150 }
                                }
                                Behavior on border.color {
                                    ColorAnimation { duration: 150 }
                                }
                            }
                            
                            // Custom popup that also scales
                            popup: Popup {
                                y: targetComboBox.height
                                width: targetComboBox.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 1
                                
                                // Apply zoom scale to match the canvas (not inverse)
                                scale: root.canvas ? root.canvas.zoomLevel : 1.0
                                transformOrigin: Item.TopLeft
                                
                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: targetComboBox.popup.visible ? targetComboBox.delegateModel : null
                                    currentIndex: targetComboBox.highlightedIndex
                                    
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }
                                
                                background: Rectangle {
                                    border.color: "#CCCCCC"
                                    radius: 2
                                }
                            }
                            
                            onCurrentTextChanged: {
                                console.log("Target input changed to:", currentText, "for port:", targetPortIndex)
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
                            var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - columnsContainer.anchors.topMargin - y
                            
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