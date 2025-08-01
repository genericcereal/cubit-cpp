import QtQuick
import QtQuick.Controls
import Cubit 1.0
import ".."
import "../CanvasUtils.js" as Utils

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
        
        // Variable nodes only have header
        if (nodeElement.nodeType === "Variable") {
            return ConfigObject.nodeHeaderHeight
        }
        
        var titleHeight = ConfigObject.nodeHeaderHeight  // Node header height from Config
        var topMargin = 10    // Title top margin
        var titleBottomMargin = 15  // Space between title and columns
        var containerBottomMargin = ConfigObject.nodeBottomMargin   // Bottom margin from Config
        var rowHeight = 30   // Height of each row
        var rowSpacing = 10  // Spacing between rows
        var addButtonHeight = nodeElement.isDynamicNode() ? 30 : 0  // Height of add button
        var addButtonMargin = nodeElement.isDynamicNode() ? 10 : 0  // Bottom margin for add button
        
        var numRows = nodeElement.rowConfigurations.length
        var contentHeight = numRows * rowHeight + (numRows > 0 ? (numRows - 1) * rowSpacing : 0)
        
        var totalHeight = topMargin + titleHeight + titleBottomMargin + contentHeight + containerBottomMargin + addButtonHeight + addButtonMargin
        
        // Ensure minimum height from Config
        return Math.max(totalHeight, ConfigObject.nodeMinHeight)
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
        function onElementChanged() {
            root.modelUpdateCount++
        }
    }
    
    
    // Handle element changes
    onElementChanged: {
        // Element properties updated
    }
    
    // Handle click to open combo boxes
    function handleClick(clickPoint) {
        // Get the click position relative to the node
        var localX = clickPoint.x - element.x
        var localY = clickPoint.y - element.y
        
        
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
        
        // MouseArea to handle node body interactions
        MouseArea {
            id: nodeBodyMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            z: -1  // Below all other elements so port handles can intercept events
            preventStealing: true  // Prevent parent from stealing mouse events during drag
            enabled: !root.canvas || !root.canvas.isEdgePreview  // Disable when edge preview is active
            
            
            
            onEntered: {
            }
            
            onExited: {
            }
            
            property bool isDragging: false
            property point dragStartPoint
            property int dragFrameCount: 0
            property point lastLoggedPos: Qt.point(0, 0)
            property point startElementPos: Qt.point(0, 0)
            property point dragOffset: Qt.point(0, 0)  // Offset from node origin to mouse click point
            
            onPressed: (mouse) => {
                // Accept the event to prevent canvas from handling it
                mouse.accepted = true
                isDragging = false
                dragStartPoint = Qt.point(mouse.x, mouse.y)
                dragFrameCount = 0
                lastLoggedPos = Qt.point(mouse.x, mouse.y)
                startElementPos = Qt.point(root.element.x, root.element.y)
                
                // Calculate the offset from where the mouse clicked to the node's origin
                // This keeps the node from jumping when drag starts
                dragOffset = Qt.point(mouse.x, mouse.y)
                
                
                // Let the controller handle selection
                // We need to convert to canvas coordinates, not just viewport coordinates
                var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                var flickable = root.canvas.flickable
                var zoom = root.canvas.zoom || 1.0
                var canvasMinX = root.canvas.canvasMinX
                var canvasMinY = root.canvas.canvasMinY
                
                // Convert viewport position to canvas coordinates
                var canvasPos = Utils.viewportToCanvas(
                    mouseInViewport,
                    flickable.contentX,
                    flickable.contentY,
                    zoom,
                    canvasMinX,
                    canvasMinY
                )
                
                
                root.canvas.controller.handleMousePress(canvasPos.x, canvasPos.y)
                
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    // Check if we've moved enough to start dragging
                    var dx = Math.abs(mouse.x - dragStartPoint.x)
                    var dy = Math.abs(mouse.y - dragStartPoint.y)
                    if (!isDragging && (dx > 3 || dy > 3)) {
                        isDragging = true
                        // Notify canvas that node dragging has started
                        if (root.canvas && root.canvas.canvasType === "script") {
                            root.canvas.isNodeDragging = true
                        }
                    }
                    
                    if (isDragging) {
                        dragFrameCount++
                        
                        // Map current mouse position to the canvas viewport
                        var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                        
                        // Convert viewport coordinates to canvas coordinates
                        // We need the flickable's content position and zoom level
                        var flickable = root.canvas.flickable
                        var zoom = root.canvas.zoom || 1.0
                        var canvasMinX = root.canvas.canvasMinX
                        var canvasMinY = root.canvas.canvasMinY
                        
                        // Convert viewport position to canvas coordinates
                        var canvasPos = Utils.viewportToCanvas(
                            mouseInViewport,
                            flickable.contentX,
                            flickable.contentY,
                            zoom,
                            canvasMinX,
                            canvasMinY
                        )
                        
                        // Update element position, accounting for where the mouse grabbed the node
                        if (root.element) {
                            root.element.x = canvasPos.x - dragOffset.x
                            root.element.y = canvasPos.y - dragOffset.y
                        }
                        
                        
                        lastLoggedPos = Qt.point(mouse.x, mouse.y)
                    }
                }
            }
            
            onReleased: (mouse) => {
                // Convert to canvas coordinates
                var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                var flickable = root.canvas.flickable
                var zoom = root.canvas.zoom || 1.0
                var canvasMinX = root.canvas.canvasMinX
                var canvasMinY = root.canvas.canvasMinY
                
                var canvasPos = Utils.viewportToCanvas(
                    mouseInViewport,
                    flickable.contentX,
                    flickable.contentY,
                    zoom,
                    canvasMinX,
                    canvasMinY
                )
                
                if (isDragging) {
                    var totalDeltaX = root.element.x - startElementPos.x
                    var totalDeltaY = root.element.y - startElementPos.y
                    
                } else {
                }
                
                // Always notify the controller about mouse release for proper selection handling
                // This ensures clicks (press + release without drag) properly select the node
                root.canvas.controller.handleMouseRelease(canvasPos.x, canvasPos.y)
                
                // Notify canvas that node dragging has stopped
                if (root.canvas && root.canvas.canvasType === "script") {
                    root.canvas.isNodeDragging = false
                }
                
                isDragging = false
                dragFrameCount = 0
            }
            
            onContainsMouseChanged: {
                if (containsMouse) {
                    // Use mapToItem without mouse parameter for hover position
                    var canvasPos = mapToItem(root.canvas, 0, 0)
                    root.canvas.hoveredElement = root.element
                    root.canvas.hoveredPoint = canvasPos
                } else if (root.canvas.hoveredElement === root.element) {
                    root.canvas.hoveredElement = null
                }
            }
        }
        
        
        // Node header
        Rectangle {
            id: nodeHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: ConfigObject.nodeHeaderHeight
            z: 2  // Ensure header is above the node body MouseArea
            
            // Background color based on node type
            color: {
                var type = nodeElement && nodeElement.nodeType ? nodeElement.nodeType : "Operation"
                switch(type) {
                    case "Event":
                        return ConfigObject.nodeHeaderEventColor
                    case "Operation":
                        return ConfigObject.nodeHeaderOperationColor
                    case "Param":
                        return ConfigObject.nodeHeaderParamColor
                    case "Variable":
                        return ConfigObject.nodeHeaderVariableColor
                    default:
                        return ConfigObject.nodeHeaderOperationColor
                }
            }
            
            Text {
                id: headerText
                text: nodeElement ? nodeElement.nodeTitle : "Node"
                color: ConfigObject.nodeHeaderTextColor
                font.pixelSize: ConfigObject.nodeHeaderTextSize
                anchors {
                    left: parent.left
                    leftMargin: ConfigObject.nodeHeaderPadding
                    verticalCenter: parent.verticalCenter
                }
                elide: Text.ElideRight
                width: parent.width - (2 * ConfigObject.nodeHeaderPadding)
            }
        }
        
        // Two column layout for targets and sources (not for Variable nodes)
        Row {
            id: columnsContainer
            visible: nodeElement ? nodeElement.nodeType !== "Variable" : true
            anchors.top: nodeHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            anchors.topMargin: 15
            spacing: 20
            z: 3  // Above MouseArea and header
            
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
                        property int targetPortIndex: hasTarget ? (targetConfig.targetPortIndex !== undefined ? targetConfig.targetPortIndex : -1) : -1
                        property string targetType: hasTarget ? (targetConfig.targetType || "Flow") : ""
                        property string targetLabel: hasTarget ? (targetConfig.targetLabel || "") : ""
                        
                        // Check if this target port has an incoming edge
                        property bool hasIncomingEdge: {
                            if (!hasTarget || !root.elementModel || !root.element) {
                                return false
                            }
                            
                            var dummy = root.modelUpdateCount
                            var edges = root.elementModel.getAllElements()
                            
                            var incomingFound = false
                            for (var i = 0; i < edges.length; i++) {
                                var edge = edges[i]
                                if (edge && edge.objectName === "Edge") {
                                    if (edge.targetNodeId === root.element.elementId && 
                                        edge.targetPortIndex === targetPortIndex) {
                                        incomingFound = true
                                    }
                                }
                            }
                            return incomingFound
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
                        
                        // Row layout for target handle, input/label, and delete button
                        Row {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 5
                            
                            // Target handle
                            Rectangle {
                                id: targetHandle
                                visible: hasTarget
                                width: 20
                                height: 20
                                radius: targetType === "Flow" ? 10 : 0
                                z: 100  // Higher than node body
                                color: {
                                    if (handleHovered || targetHandleMouseArea.containsMouse) {
                                        return targetType === "Flow" ? "#4CAF50" : "#FF9800"
                                    } else {
                                        return targetType === "Flow" ? "#666666" : "#795548"
                                    }
                                }
                                border.width: 2
                                border.color: {
                                    if (handleHovered || targetHandleMouseArea.containsMouse) {
                                        return targetType === "Flow" ? "#2E7D32" : "#E65100"
                                    } else {
                                        return targetType === "Flow" ? "#333333" : "#5D4037"
                                    }
                                }
                                
                                MouseArea {
                                    id: targetHandleMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.CrossCursor
                                    propagateComposedEvents: false  // Don't let events bubble to node body
                                    preventStealing: true  // Prevent parent from stealing events
                                    
                                    property bool isDragging: false
                                    
                                    onPressed: (mouse) => {
                                        // Don't allow dragging from target handles
                                        mouse.accepted = true
                                    }
                                }
                            }
                            
                            // Port input using Loader to ensure proper re-creation
                            Loader {
                                id: targetInputLoader
                                visible: hasTarget && targetType !== "Flow" && parent && !parent.hasIncomingEdge
                                width: 80
                                height: 24
                                
                                sourceComponent: PortInput {
                                    portType: targetType
                                    portIndex: targetPortIndex
                                    hasIncomingEdge: {
                                        // Safely access the parent's hasIncomingEdge property
                                        if (!parent || !parent.parent) return false;
                                        return parent.parent.hasIncomingEdge || false;
                                    }
                                    isHovered: comboBoxHovered
                                    canvas: root.canvas
                                    value: nodeElement ? (nodeElement.isDynamicNode() ? nodeElement.getPortValue(targetPortIndex) : nodeElement.value) : ""
                                    
                                    
                                    onPortValueChanged: function(newValue) {
                                        // Update the node's value property
                                        if (nodeElement) {
                                            if (nodeElement.isDynamicNode()) {
                                                nodeElement.setPortValue(targetPortIndex, String(newValue))
                                            } else {
                                                nodeElement.value = String(newValue)
                                            }
                                        }
                                    }
                                }
                            }
                            
                            Text {
                                visible: hasTarget && targetLabel !== "" && (targetType === "Flow" || hasIncomingEdge)
                                text: targetLabel
                                color: "#333333"
                                font.pixelSize: 12
                            }
                            
                            // Delete button for dynamic nodes
                            Rectangle {
                                visible: nodeElement && nodeElement.isDynamicNode() && hasTarget && targetType === "Number" && targetPortIndex > 0
                                width: 20
                                height: 20
                                color: deleteMouseArea.containsMouse ? "#FF5252" : "#F44336"
                                radius: 2
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: "−"
                                    color: "white"
                                    font.pixelSize: 16
                                    font.bold: true
                                }
                                
                                MouseArea {
                                    id: deleteMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    
                                    onClicked: {
                                        nodeElement.removeDynamicRow(targetPortIndex)
                                    }
                                }
                            }
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
                        property int sourcePortIndex: hasSource ? (sourceConfig.sourcePortIndex !== undefined ? sourceConfig.sourcePortIndex : -1) : -1
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
                            z: 100  // Higher than node body
                            color: {
                                if (handleHovered || sourceHandleMouseArea.containsMouse) {
                                    return sourceType === "Flow" ? "#2196F3" : "#FF9800"
                                } else {
                                    return sourceType === "Flow" ? "#666666" : "#795548"
                                }
                            }
                            border.width: 2
                            border.color: {
                                if (handleHovered || sourceHandleMouseArea.containsMouse) {
                                    return sourceType === "Flow" ? "#1565C0" : "#E65100"
                                } else {
                                    return sourceType === "Flow" ? "#333333" : "#5D4037"
                                }
                            }
                            
                            MouseArea {
                                id: sourceHandleMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.CrossCursor
                                propagateComposedEvents: false  // Don't let events bubble to node body
                                preventStealing: true  // Prevent parent from stealing events
                                
                                property bool isDragging: false
                                
                                onPressed: (mouse) => {
                                    // Start dragging from this source port
                                    isDragging = true
                                    root.canvas.startPortDrag(root.element, "right", sourcePortIndex, sourceType)
                                    mouse.accepted = true
                                }
                                
                                onPositionChanged: (mouse) => {
                                    if (isDragging && pressed) {
                                        // Convert to canvas coordinates
                                        var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                                        var flickable = root.canvas.flickable
                                        var zoom = root.canvas.zoom || 1.0
                                        var canvasMinX = root.canvas.canvasMinX
                                        var canvasMinY = root.canvas.canvasMinY
                                        
                                        var canvasPos = Utils.viewportToCanvas(
                                            mouseInViewport,
                                            flickable.contentX,
                                            flickable.contentY,
                                            zoom,
                                            canvasMinX,
                                            canvasMinY
                                        )
                                        root.canvas.updatePortDrag(canvasPos)
                                    }
                                }
                                
                                onReleased: (mouse) => {
                                    if (isDragging) {
                                        isDragging = false
                                        // Convert to canvas coordinates
                                        var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                                        var flickable = root.canvas.flickable
                                        var zoom = root.canvas.zoom || 1.0
                                        var canvasMinX = root.canvas.canvasMinX
                                        var canvasMinY = root.canvas.canvasMinY
                                        
                                        var canvasPos = Utils.viewportToCanvas(
                                            mouseInViewport,
                                            flickable.contentX,
                                            flickable.contentY,
                                            zoom,
                                            canvasMinX,
                                            canvasMinY
                                        )
                                        root.canvas.endPortDrag(canvasPos)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Add row button for dynamic nodes
        Rectangle {
            visible: nodeElement && nodeElement.isDynamicNode()
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.bottomMargin: 10
            height: 30
            color: addRowMouseArea.containsMouse ? "#2196F3" : "#1976D2"
            radius: 4
            
            Text {
                anchors.centerIn: parent
                text: "+ Add Row"
                color: "white"
                font.pixelSize: 13
                font.bold: true
            }
            
            MouseArea {
                id: addRowMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                
                onClicked: {
                    nodeElement.addDynamicRow()
                }
            }
        }
        
        // Variable node source handle (appears on right edge of header)
        Rectangle {
            id: variableSourceHandle
            visible: nodeElement && nodeElement.nodeType === "Variable"
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.verticalCenter: nodeHeader.verticalCenter
            width: 20
            height: 20
            radius: 0  // Square for data ports
            z: 100  // Ensure it's above all other elements including MouseArea
            
            property bool isHovered: variableHandleMouseArea.containsMouse
            
            color: isHovered ? "#FF9800" : "#795548"  // Orange when hovered, brown otherwise
            border.width: 2
            border.color: isHovered ? "#E65100" : "#5D4037"  // Dark orange when hovered, dark brown otherwise
            
            MouseArea {
                id: variableHandleMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.CrossCursor
                propagateComposedEvents: false  // Don't let events bubble to node body
                preventStealing: true  // Prevent parent from stealing events
                
                property bool isDragging: false
                
                onPressed: (mouse) => {
                    // Start dragging from variable source port
                    isDragging = true
                    root.canvas.startPortDrag(root.element, "right", 0, "String")
                    mouse.accepted = true
                }
                
                onPositionChanged: (mouse) => {
                    if (isDragging && pressed) {
                        // Convert to canvas coordinates
                        var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                        var flickable = root.canvas.flickable
                        var zoom = root.canvas.zoom || 1.0
                        var canvasMinX = root.canvas.canvasMinX
                        var canvasMinY = root.canvas.canvasMinY
                        
                        var canvasPos = Utils.viewportToCanvas(
                            mouseInViewport,
                            flickable.contentX,
                            flickable.contentY,
                            zoom,
                            canvasMinX,
                            canvasMinY
                        )
                        root.canvas.updatePortDrag(canvasPos)
                    }
                }
                
                onReleased: (mouse) => {
                    if (isDragging) {
                        isDragging = false
                        // Convert to canvas coordinates
                        var mouseInViewport = mapToItem(root.canvas, mouse.x, mouse.y)
                        var flickable = root.canvas.flickable
                        var zoom = root.canvas.zoom || 1.0
                        var canvasMinX = root.canvas.canvasMinX
                        var canvasMinY = root.canvas.canvasMinY
                        
                        var canvasPos = Utils.viewportToCanvas(
                            mouseInViewport,
                            flickable.contentX,
                            flickable.contentY,
                            zoom,
                            canvasMinX,
                            canvasMinY
                        )
                        root.canvas.endPortDrag(canvasPos)
                    }
                }
            }
            
        }
    }
}