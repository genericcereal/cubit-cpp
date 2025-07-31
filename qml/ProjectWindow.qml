import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit

ApplicationWindow {
    id: projectWindow
    width: 1280
    height: 800
    visible: true
    title: qsTr("Cubit - %1").arg(canvas ? canvas.name : "Untitled")
    
    property string canvasId: ""
    property var canvas: null
    property alias dragOverlay: dragOverlay
    
    onCanvasIdChanged: {
        // Canvas ID changed
        // Update canvas when canvasId changes
        if (canvasId) {
            canvas = Application.getProject(canvasId)
            // Canvas updated
            
            // Update the controller's project when canvas changes
            if (canvas && designControlsController) {
                designControlsController.project = canvas
            }
        }
    }
    
    // Listen for canvas ID updates from the Application
    Connections {
        target: Application
        
        function onCanvasIdUpdated(oldId, newId) {
            // Update our canvasId if it matches the old ID
            if (projectWindow.canvasId === oldId) {
                projectWindow.canvasId = newId
            }
        }
    }
    
    // Create window-specific panels instance
    property var windowPanels: Panels {}
    
    // Window-specific DesignControlsController
    property var designControlsController: DesignControlsController {
        project: canvas
        
        Component.onCompleted: {
            // Design controls controller initialized
        }
    }
    
    Component.onCompleted: {
        // Project window initialized
        
        // Test if we can create a controller dynamically
        try {
            var testController = Qt.createQmlObject('import Cubit 1.0; DesignControlsController {}', projectWindow)
            // Test controller created
            testController.destroy()
        } catch (error) {
            // Error creating test controller
        }
        
        // Canvas will be set by onCanvasIdChanged
    }
    
    // Use CanvasScreen for the window content
    CanvasScreen {
        id: canvasScreen
        anchors.fill: parent
        canvas: projectWindow.canvas
        panels: projectWindow.windowPanels
        designControlsController: projectWindow.designControlsController
        
        Component.onCompleted: {
            // Canvas screen initialized
        }
    }
    
    // Drag overlay for ElementList
    Item {
        id: dragOverlay
        anchors.fill: parent
        visible: false
        z: 10000
        focus: true  // Enable keyboard focus for ESC handling
        
        // Make sure this overlay captures all mouse events when visible
        enabled: visible
        
        property string draggedElementName: ""
        property string draggedElementType: ""
        property var draggedElement: null
        property point ghostPosition: Qt.point(0, 0)
        property string dropTarget: "none"  // "none", "canvas", "properties"
        
        // Watch for ghost position changes to update drop target
        onGhostPositionChanged: {
            updateDropTarget()
        }
        
        // Update canvas draggedVariableType when visibility changes
        onVisibleChanged: {
            // DragOverlay visibility changed
            
            if (canvas) {
                if (visible && draggedElementType === "Variable" && draggedElement) {
                    // Set canvas dragged variable type
                    canvas.draggedVariableType = draggedElement.variableType || "string"
                } else {
                    // Clear canvas dragged variable type
                    canvas.draggedVariableType = ""
                    canvas.hoveredVariableTarget = {}  // Clear hover target when not dragging
                }
            } else {
                // No canvas available
            }
            
            // Ensure the overlay gets keyboard focus when shown
            if (visible) {
                dragOverlay.forceActiveFocus()
            }
        }
        
        // Signal emitted when an element is dropped
        signal elementDropped(var element, point dropPoint)
        
        // Function to handle drop at current position
        function handleDrop() {
            // Handle drop
            
            // Check if we're dropping on the script canvas
            if (dropTarget === "script" && draggedElement && canvas && canvas.controller) {
                console.log("Dropping element on script canvas:", draggedElementName, draggedElementType)
                
                // Get the canvas coordinates from the ghost position
                if (canvasScreen && canvasScreen.canvasContainer) {
                    var canvasPoint = canvasScreen.canvasContainer.mapFromItem(dragOverlay, ghostPosition.x, ghostPosition.y)
                    
                    // Get the actual canvas component through the loader
                    var scriptCanvas = canvasScreen.canvasContainer.children[0] ? canvasScreen.canvasContainer.children[0].item : null
                    if (scriptCanvas && scriptCanvas.flickable) {
                        // Convert to canvas coordinates accounting for scroll and zoom
                        var canvasX = (scriptCanvas.flickable.contentX + canvasPoint.x) / scriptCanvas.zoom + scriptCanvas.canvasMinX
                        var canvasY = (scriptCanvas.flickable.contentY + canvasPoint.y) / scriptCanvas.zoom + scriptCanvas.canvasMinY
                        
                        // Adjust for ghost center (150x30 for Variable node preview)
                        canvasX -= 75  // Half width
                        canvasY -= 15  // Half height
                        
                        console.log("Creating Variable node at canvas position:", canvasX, canvasY)
                        
                        // Create Variable node for the dropped element
                        var nodeData = {
                            x: canvasX,
                            y: canvasY,
                            width: 150,
                            name: draggedElementName,
                            type: "Variable",
                            sourceElementId: draggedElement.elementId,
                            targets: [],
                            sources: [{
                                id: "value",
                                label: "Value",
                                type: "String"
                            }]
                        }
                        
                        var nodeId = canvas.controller.createNodeFromJson(JSON.stringify(nodeData))
                        console.log("Created Variable node with ID:", nodeId)
                    }
                }
            } else if (draggedElement && draggedElementType === "Variable" && canvas) {
                // Original Variable binding logic for design canvas
                if (canvas.hoveredVariableTarget && 
                    canvas.hoveredVariableTarget.elementId && 
                    canvas.hoveredVariableTarget.propertyName) {
                    
                    if (canvas.controller) {
                        canvas.controller.assignVariable(
                            draggedElement.elementId, 
                            canvas.hoveredVariableTarget.elementId, 
                            canvas.hoveredVariableTarget.propertyName
                        )
                    }
                }
            }
            
            // Emit the drop signal
            if (draggedElement) {
                elementDropped(draggedElement, ghostPosition)
            }
            
            // Hide the overlay and reset state
            visible = false
            draggedElement = null
            draggedElementName = ""
            draggedElementType = ""
        }
        
        // Function to update drop target based on current ghost position
        function updateDropTarget() {
            var mouse = ghostPosition
            var target = "none"
            
            // Check if we're over the canvas
            if (canvasScreen && canvasScreen.canvasContainer) {
                var canvasPoint = canvasScreen.canvasContainer.mapFromItem(dragOverlay, mouse.x, mouse.y)
                
                if (canvasPoint.x >= 0 && canvasPoint.x <= canvasScreen.canvasContainer.width &&
                    canvasPoint.y >= 0 && canvasPoint.y <= canvasScreen.canvasContainer.height) {
                    // Check if it's a script canvas
                    if (canvas && canvas.viewMode === "script") {
                        target = "script"
                    } else {
                        target = "canvas"
                    }
                }
            }
            
            // Check if we're over the detail panel (only if not already over canvas)
            if (target === "none" && canvasScreen && canvasScreen.detailPanel && canvasScreen.detailPanel.visible) {
                var detailPoint = canvasScreen.detailPanel.mapFromItem(dragOverlay, mouse.x, mouse.y)
                
                if (detailPoint.x >= 0 && detailPoint.x <= canvasScreen.detailPanel.width &&
                    detailPoint.y >= 0 && detailPoint.y <= canvasScreen.detailPanel.height) {
                    // We're in the detail panel - now check if we're in the properties section
                    // The detail panel has tabs and the properties section is in the bottom part
                    if (detailPoint.y > canvasScreen.detailPanel.height * 0.5) {
                        target = "properties"
                    }
                }
            }
            
            dropTarget = target
        }
        
        // Full screen mouse area to capture drop events
        MouseArea {
            id: dropArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            
            onEntered: {
                console.log("DragOverlay MouseArea - Mouse entered")
            }
            
            onExited: {
                console.log("DragOverlay MouseArea - Mouse exited")
            }
            
            onPositionChanged: (mouse) => {
                console.log("DragOverlay MouseArea - Mouse moved to:", mouse.x, mouse.y)
            }
            
            onPressed: (mouse) => {
                console.log("DragOverlay MouseArea - Mouse pressed at:", mouse.x, mouse.y)
                mouse.accepted = true  // Accept the press to get the release
            }
            
            onReleased: (mouse) => {
                console.log("DragOverlay MouseArea - Mouse released at:", mouse.x, mouse.y)
                
                // Check if we're dropping a Variable
                if (dragOverlay.draggedElement && dragOverlay.draggedElementType === "Variable") {
                    console.log("DragOverlay - Dropping variable:", dragOverlay.draggedElement.elementId)
                    
                    // Find what's under the mouse using childAt
                    var item = projectWindow.childAt(mouse.x, mouse.y)
                    console.log("DragOverlay - Item under mouse:", item)
                    
                    // Walk up the parent chain to find a VariableAwareSpinBox
                    while (item) {
                        if (item.objectName === "VariableAwareSpinBox" || 
                            (item.propertyName !== undefined && item.elementId !== undefined && item.acceptsDraggedType !== undefined)) {
                            console.log("DragOverlay - Found VariableAwareSpinBox:", item.propertyName)
                            
                            // Manually trigger the binding creation
                            if (item.acceptsDraggedType && item.elementId && item.propertyName) {
                                if (canvas && canvas.bindingManager) {
                                    console.log("DragOverlay - Creating binding")
                                    canvas.bindingManager.createBinding(
                                        dragOverlay.draggedElement.elementId, 
                                        item.elementId, 
                                        item.propertyName
                                    )
                                }
                            }
                            break
                        }
                        item = item.parent
                    }
                }
                
                // Emit the drop signal with the element and position
                if (dragOverlay.draggedElement) {
                    dragOverlay.elementDropped(dragOverlay.draggedElement, Qt.point(mouse.x, mouse.y))
                }
                
                // Hide the overlay and reset state
                dragOverlay.visible = false
                dragOverlay.draggedElement = null
                dragOverlay.draggedElementName = ""
                dragOverlay.draggedElementType = ""
            }
        }
        
        // Handle ESC key to cancel drag
        Keys.onEscapePressed: {
            // Cancel the drag operation
            dragOverlay.visible = false
            dragOverlay.draggedElement = null
            dragOverlay.draggedElementName = ""
            dragOverlay.draggedElementType = ""
        }
        
        
        // Container that switches between regular drag ghost and node preview
        Item {
            id: ghostContainer
            x: dragOverlay.ghostPosition.x - width / 2
            y: dragOverlay.ghostPosition.y - height / 2
            width: dragOverlay.dropTarget === "script" ? 150 : 200
            height: dragOverlay.dropTarget === "script" ? 30 : 28
            
            // Regular drag ghost for non-script targets
            Rectangle {
                id: dragGhost
                anchors.fill: parent
                visible: dragOverlay.dropTarget !== "script"
                color: {
                    switch(dragOverlay.dropTarget) {
                        case "canvas": return "#2196F3"  // Blue when over design canvas
                        case "properties": return "#4CAF50"  // Green when over properties
                        default: return "#f5f5f5"  // Default gray
                    }
                }
                border.color: {
                    switch(dragOverlay.dropTarget) {
                        case "canvas": return "#1976D2"  // Darker blue
                        case "properties": return "#388E3C"  // Darker green
                        default: return "#2196F3"  // Default blue
                    }
                }
                border.width: 2
                radius: 4
                opacity: 0.8
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    anchors.topMargin: 4
                    anchors.bottomMargin: 4
                    spacing: 4
                    
                    Label {
                        Layout.fillWidth: true
                        text: dragOverlay.draggedElementName
                        elide: Text.ElideRight
                        font.pixelSize: 12
                        color: (dragOverlay.dropTarget === "none") ? "#000000" : "#FFFFFF"
                    }
                    
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: dragOverlay.draggedElementType
                        color: (dragOverlay.dropTarget === "none") ? "#666666" : "#EEEEEE"
                        font.pixelSize: 10
                    }
                }
            }
            
            // Variable Node preview when over script canvas
            Rectangle {
                id: nodePreview
                anchors.fill: parent
                visible: dragOverlay.dropTarget === "script"
                color: "#FFFFFF"
                border.color: "#CCCCCC"
                border.width: 1
                radius: 4
                opacity: 0.9
                
                // Purple header like Variable nodes
                Rectangle {
                    id: nodeHeader
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 30
                    color: "#9C27B0"  // Purple for Variable nodes
                    radius: 4
                    
                    // Clip bottom corners
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 4
                        color: parent.color
                    }
                    
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: dragOverlay.draggedElementName
                        color: "#FFFFFF"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                    
                    // Source port handle on the right
                    Rectangle {
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        color: "#FFB74D"  // Orange color for String type
                        border.color: "#F57C00"
                        border.width: 1
                    }
                }
            }
        }
    }
    
    // Handle window closing
    onClosing: {
        // Future: Handle unsaved changes, cleanup, etc.
        // Project window closing
        
        // Window closing
        
        // Close the project window without deleting the project data
        if (canvasId) {
            Application.closeProjectWindow(canvasId)
        }
    }
}