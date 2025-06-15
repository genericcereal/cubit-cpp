import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

Item {
    id: root
    
    property var controller
    property var selectionManager
    property var elementModel
    
    // Watch for mode changes to ensure selection box is deactivated
    Connections {
        target: controller
        function onModeChanged() {
            if (controller.mode !== "select" && selectionBoxHandler.active) {
                selectionBoxHandler.endSelection()
            }
        }
    }
    
    // Canvas properties
    property real zoomLevel: 1.0
    property real minZoom: 0.1
    property real maxZoom: 5.0
    property real zoomStep: 0.1
    
    // Expose internal components for viewport overlay
    property alias flickable: flickable
    property alias canvasArea: canvasArea
    property alias selectionBoxHandler: selectionBoxHandler
    
    // Canvas configuration - infinite canvas with center at (0,0)
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    property real canvasMaxX: 10000
    property real canvasMaxY: 10000
    
    // Canvas type - to be overridden by subclasses
    property string canvasType: "base"
    
    Component.onCompleted: {
        centerViewAtOrigin()
    }
    
    // Center the view at canvas origin (0,0)
    function centerViewAtOrigin() {
        var centerX = (-canvasMinX) * zoomLevel - flickable.width / 2
        var centerY = (-canvasMinY) * zoomLevel - flickable.height / 2
        
        flickable.contentX = centerX
        flickable.contentY = centerY
    }
    
    // Background
    Rectangle {
        anchors.fill: parent
        color: "#f5f5f5"
        antialiasing: true
    }
    
    // Main canvas flickable area
    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: canvasContent.width
        contentHeight: canvasContent.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        
        // Canvas content container
        Item {
            id: canvasContent
            width: (root.canvasMaxX - root.canvasMinX) * root.zoomLevel
            height: (root.canvasMaxY - root.canvasMinY) * root.zoomLevel
            
            // Transform origin for zooming
            transformOrigin: Item.TopLeft
            
            // Canvas area where elements are placed
            Item {
                id: canvasArea
                x: 0
                y: 0
                width: root.canvasMaxX - root.canvasMinX
                height: root.canvasMaxY - root.canvasMinY
                scale: root.zoomLevel
                transformOrigin: Item.TopLeft
                
                // Content layer to be defined by subclasses
                Item {
                    id: contentLayer
                    anchors.fill: parent
                }
            }
        }
        
        // Scroll indicators
        ScrollIndicator.vertical: ScrollIndicator { active: true }
        ScrollIndicator.horizontal: ScrollIndicator { active: true }
    }
    
    // Mouse/touch interaction area
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        hoverEnabled: true
        propagateComposedEvents: true
        
        // Convert mouse position to canvas coordinates
        function toCanvasCoords(mouseX, mouseY) {
            var canvasX = (flickable.contentX + mouseX) / root.zoomLevel + root.canvasMinX
            var canvasY = (flickable.contentY + mouseY) / root.zoomLevel + root.canvasMinY
            return Qt.point(canvasX, canvasY)
        }
        
        // Middle mouse button panning
        property bool isPanning: false
        property point lastPanPoint
        
        // Track if mouse moved during press (to distinguish click from drag)
        property bool mouseMoved: false
        property point pressPoint
        
        onPressed: (mouse) => {
            var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
            mouseMoved = false
            pressPoint = Qt.point(mouse.x, mouse.y)
            
            if (mouse.button === Qt.MiddleButton) {
                isPanning = true
                lastPanPoint = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                // Always let subclasses handle the press first
                handleLeftButtonPress(canvasPoint)
            }
        }
        
        onPositionChanged: (mouse) => {
            var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
            
            // Check if mouse moved significantly (more than 3 pixels)
            if (pressed && !mouseMoved) {
                var dx = Math.abs(mouse.x - pressPoint.x)
                var dy = Math.abs(mouse.y - pressPoint.y)
                if (dx > 3 || dy > 3) {
                    mouseMoved = true
                    
                    // If in select mode and mouse moved, start selection box
                    if (controller && controller.mode === "select" && !selectionBoxHandler.active) {
                        console.log("BaseCanvas: Checking if should start selection box")
                        console.log("  controller.mode:", controller.mode)
                        console.log("  selectionBoxHandler.active:", selectionBoxHandler.active)
                        // Check if we're not dragging an element
                        var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
                        console.log("  hit test result:", element)
                        if (!element) {
                            console.log("  Starting selection box at:", JSON.stringify(toCanvasCoords(pressPoint.x, pressPoint.y)))
                            selectionBoxHandler.startSelection(toCanvasCoords(pressPoint.x, pressPoint.y))
                        }
                    }
                }
            }
            
            if (isPanning) {
                // Pan the view
                flickable.contentX -= mouse.x - lastPanPoint.x
                flickable.contentY -= mouse.y - lastPanPoint.y
                lastPanPoint = Qt.point(mouse.x, mouse.y)
            } else {
                if (selectionBoxHandler.active) {
                    selectionBoxHandler.updateSelection(canvasPoint)
                } else if (pressed) {
                    // Let subclasses handle drag
                    handleMouseDrag(canvasPoint)
                } else {
                    // Let subclasses handle hover
                    handleMouseHover(canvasPoint)
                }
            }
        }
        
        onReleased: (mouse) => {
            if (isPanning) {
                isPanning = false
                cursorShape = Qt.ArrowCursor
            } else if (selectionBoxHandler.active) {
                console.log("BaseCanvas: Ending selection box")
                selectionBoxHandler.endSelection()
            } else if (mouse.button === Qt.LeftButton) {
                var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
                handleLeftButtonRelease(canvasPoint)
            }
        }
        
        onExited: {
            handleMouseExit()
        }
        
        onWheel: (wheel) => {
            // Zoom with Ctrl+Wheel
            if (wheel.modifiers & Qt.ControlModifier) {
                var zoomDelta = wheel.angleDelta.y > 0 ? 1.1 : 0.9
                var newZoom = Math.max(root.minZoom, Math.min(root.maxZoom, root.zoomLevel * zoomDelta))
                
                // Get mouse position in canvas coordinates before zoom
                var canvasPoint = toCanvasCoords(wheel.x, wheel.y)
                
                // Apply zoom
                root.zoomLevel = newZoom
                
                // Adjust content position to keep mouse point stable
                flickable.contentX = (canvasPoint.x - root.canvasMinX) * root.zoomLevel - wheel.x
                flickable.contentY = (canvasPoint.y - root.canvasMinY) * root.zoomLevel - wheel.y
                
                wheel.accepted = true
            } else {
                // Regular scrolling
                wheel.accepted = false
            }
        }
    }
    
    // Selection box handler
    SelectionBoxHandler {
        id: selectionBoxHandler
        selectionManager: root.selectionManager
        onSelectionRectChanged: function(rect) {
            console.log("BaseCanvas: onSelectionRectChanged called with rect:", JSON.stringify({x: rect.x, y: rect.y, width: rect.width, height: rect.height}))
            // Let subclasses handle what to select
            handleSelectionRect(rect)
        }
    }
    
    // Virtual functions to be implemented by subclasses
    function handleLeftButtonPress(canvasPoint) {
        // Override in subclasses
    }
    
    function handleMouseDrag(canvasPoint) {
        // Override in subclasses
    }
    
    function handleMouseHover(canvasPoint) {
        // Override in subclasses
    }
    
    function handleLeftButtonRelease(canvasPoint) {
        // Override in subclasses
    }
    
    function handleMouseExit() {
        // Override in subclasses
    }
    
    function handleSelectionRect(rect) {
        // Override in subclasses
    }
    
    // Get content layer for subclasses to add their content
    function getContentLayer() {
        return contentLayer
    }
    
    // Keyboard shortcuts
    Shortcut {
        sequence: "Delete"
        onActivated: controller.deleteSelectedElements()
    }
    
    Shortcut {
        sequence: "Ctrl+A"
        onActivated: controller.selectAll()
    }
    
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (selectionManager) {
                selectionManager.clearSelection()
            }
            if (controller) {
                controller.setMode("select")
            }
        }
    }
}