import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

Item {
    id: root
    
    property var controller
    property var selectionManager
    property var elementModel
    
    Component.onCompleted: {
        if (controller) {
            controller.setElementModel(elementModel)
            controller.setSelectionManager(selectionManager)
        }
        centerViewAtOrigin()
    }
    
    // Center the view at canvas origin (0,0)
    function centerViewAtOrigin() {
        // Calculate the position to center (0,0) in the viewport
        var centerX = (-canvasMinX) * zoomLevel - flickable.width / 2
        var centerY = (-canvasMinY) * zoomLevel - flickable.height / 2
        
        flickable.contentX = centerX
        flickable.contentY = centerY
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
    property var hoveredElement: null
    
    // Canvas configuration - infinite canvas with center at (0,0)
    // Using larger fixed bounds to avoid coordinate system shifts during element manipulation
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    property real canvasMaxX: 10000
    property real canvasMaxY: 10000
    
    // With fixed bounds, we don't need dynamic updates
    
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
                
                
                // Elements layer
                Item {
                    id: elementsLayer
                    anchors.fill: parent
                    
                    Repeater {
                        model: elementModel
                        
                        delegate: Loader {
                            property var element: model.element
                            property string elementType: model.elementType
                            
                            // Position elements relative to canvas origin
                            x: element ? element.x - root.canvasMinX : 0
                            y: element ? element.y - root.canvasMinY : 0
                            
                            sourceComponent: {
                                switch(elementType) {
                                    case "Frame": return frameComponent
                                    case "Text": return textComponent
                                    default: return null
                                }
                            }
                            
                            onLoaded: {
                                if (item) {
                                    item.element = element
                                    item.elementModel = root.elementModel
                                }
                            }
                        }
                    }
                }
                
                // Creation preview layer
                Item {
                    id: creationLayer
                    anchors.fill: parent
                    z: 100
                    
                    // Element creation preview
                    Rectangle {
                        id: creationPreview
                        visible: creationDragHandler.active && controller.mode !== "select"
                        color: Qt.rgba(0, 0.4, 1, 0.1)
                        border.color: "#0066cc"
                        border.width: 2
                        
                        x: Math.min(creationDragHandler.startPoint.x, creationDragHandler.currentPoint.x) - root.canvasMinX
                        y: Math.min(creationDragHandler.startPoint.y, creationDragHandler.currentPoint.y) - root.canvasMinY
                        width: Math.abs(creationDragHandler.currentPoint.x - creationDragHandler.startPoint.x)
                        height: Math.abs(creationDragHandler.currentPoint.y - creationDragHandler.startPoint.y)
                    }
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
        
        // Convert mouse position to canvas coordinates
        function toCanvasCoords(mouseX, mouseY) {
            var canvasX = (flickable.contentX + mouseX) / root.zoomLevel + root.canvasMinX
            var canvasY = (flickable.contentY + mouseY) / root.zoomLevel + root.canvasMinY
            return Qt.point(canvasX, canvasY)
        }
        
        // Middle mouse button panning
        property bool isPanning: false
        property point lastPanPoint
        
        onPressed: (mouse) => {
            var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
            
            if (mouse.button === Qt.MiddleButton) {
                isPanning = true
                lastPanPoint = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                if (controller.mode === "select") {
                    // Check if we hit an element
                    var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
                    if (element) {
                        // Start dragging element
                        controller.handleMousePress(canvasPoint.x, canvasPoint.y)
                    } else {
                        // Start selection box
                        selectionBoxHandler.startSelection(canvasPoint)
                    }
                } else {
                    // Start element creation
                   
                    controller.handleMousePress(canvasPoint.x, canvasPoint.y)
                }
            }
        }
        
        onPositionChanged: (mouse) => {
            var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
            
            if (isPanning) {
                // Pan the view
                flickable.contentX -= mouse.x - lastPanPoint.x
                flickable.contentY -= mouse.y - lastPanPoint.y
                lastPanPoint = Qt.point(mouse.x, mouse.y)
            } else {
                // Track hover when not dragging
                if (!pressed && controller.mode === "select") {
                    root.hoveredElement = controller.hitTest(canvasPoint.x, canvasPoint.y)
                }
                
                if (selectionBoxHandler.active) {
                    selectionBoxHandler.updateSelection(canvasPoint)
                } else if (creationDragHandler.active) {
                    creationDragHandler.updateCreation(canvasPoint)
                } else if (pressed) {
                    // Dragging elements or updating creation
                    controller.handleMouseMove(canvasPoint.x, canvasPoint.y)
                }
            }
        }
        
        onReleased: (mouse) => {
            console.log("MouseArea onReleased - button:", mouse.button, "mode:", controller.mode)
            console.log("  isPanning:", isPanning, "selectionBoxHandler.active:", selectionBoxHandler.active, "creationDragHandler.active:", creationDragHandler.active)
            
            if (isPanning) {
                isPanning = false
                cursorShape = Qt.ArrowCursor
            } else if (selectionBoxHandler.active) {
                selectionBoxHandler.endSelection()
            } else if (creationDragHandler.active) {
                creationDragHandler.endCreation()
            } else if (mouse.button === Qt.LeftButton) {
                var canvasPoint = toCanvasCoords(mouse.x, mouse.y)
                console.log("  Calling controller.handleMouseRelease at:", canvasPoint.x, canvasPoint.y)
                controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
            }
        }
        
        onExited: {
            root.hoveredElement = null
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
    QtObject {
        id: selectionBoxHandler
        property bool active: false
        property point startPoint: Qt.point(0, 0)
        property point currentPoint: Qt.point(0, 0)
        
        function startSelection(point) {
            active = true
            startPoint = point
            currentPoint = point
            selectionManager.clearSelection()
        }
        
        function updateSelection(point) {
            currentPoint = point
            
            // Find elements in selection box
            var rect = Qt.rect(
                Math.min(startPoint.x, currentPoint.x),
                Math.min(startPoint.y, currentPoint.y),
                Math.abs(currentPoint.x - startPoint.x),
                Math.abs(currentPoint.y - startPoint.y)
            )
            
            // Update selection through controller
            controller.selectElementsInRect(rect)
        }
        
        function endSelection() {
            active = false
        }
    }
    
    // Creation drag handler
    QtObject {
        id: creationDragHandler
        property bool active: false
        property point startPoint: Qt.point(0, 0)
        property point currentPoint: Qt.point(0, 0)
        
        function startCreation(point) {
            active = true
            startPoint = point
            currentPoint = point
        }
        
        function updateCreation(point) {
            currentPoint = point
        }
        
        function endCreation() {
            if (active) {
                var width = Math.abs(currentPoint.x - startPoint.x)
                var height = Math.abs(currentPoint.y - startPoint.y)
                
                // Minimum size threshold
                if (width < 10) width = 200
                if (height < 10) height = 150
                
                var x = Math.min(startPoint.x, currentPoint.x)
                var y = Math.min(startPoint.y, currentPoint.y)
                
                controller.createElement(controller.mode, x, y, width, height)
            }
            active = false
        }
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
            selectionManager.clearSelection()
            controller.setMode("select")
        }
    }
    
    // Component definitions
    Component {
        id: frameComponent
        FrameElement {}
    }
    
    Component {
        id: textComponent
        TextElement {}
    }
}