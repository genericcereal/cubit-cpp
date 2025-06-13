import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0
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
        var centerPos = CanvasUtils.calculateContentPositionForCenter(0, 0, flickable.width, flickable.height, zoomLevel)
        flickable.contentX = centerPos.x
        flickable.contentY = centerPos.y
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
    
    // Canvas configuration - use centralized bounds from CanvasUtils
    property real canvasMinX: CanvasUtils.canvasMinX
    property real canvasMinY: CanvasUtils.canvasMinY
    property real canvasMaxX: CanvasUtils.canvasMaxX
    property real canvasMaxY: CanvasUtils.canvasMaxY
    
    // With fixed bounds, we don't need dynamic updates
    
    // Background
    Rectangle {
        anchors.fill: parent
        color: Config.canvasBackground
        antialiasing: true
    }
    
    // Main canvas flickable area
    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: canvasContent.width * root.zoomLevel
        contentHeight: canvasContent.height * root.zoomLevel
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        
        // Canvas content container
        Item {
            id: canvasContent
            width: CanvasUtils.canvasWidth
            height: CanvasUtils.canvasHeight
            
            // Transform origin for zooming
            transformOrigin: Item.TopLeft
            scale: root.zoomLevel
            
            // Canvas area where elements are placed
            Item {
                id: canvasArea
                x: 0
                y: 0
                width: CanvasUtils.canvasWidth
                height: CanvasUtils.canvasHeight
                
                
                // Elements layer
                Item {
                    id: elementsLayer
                    anchors.fill: parent
                    
                    Repeater {
                        model: elementModel
                        
                        delegate: Loader {
                            id: elementLoader
                            property var element: model.element
                            property string elementType: model.elementType
                            
                            // Position elements relative to canvas origin
                            x: element ? CanvasUtils.canvasXToRelative(element.x) : 0
                            y: element ? CanvasUtils.canvasYToRelative(element.y) : 0
                            width: element ? element.width : 0
                            height: element ? element.height : 0
                            
                            // Visibility detection properties
                            property var elementBounds: {
                                "left": element ? element.x : 0,
                                "top": element ? element.y : 0,
                                "right": element ? element.x + element.width : 0,
                                "bottom": element ? element.y + element.height : 0
                            }
                            
                            property var viewportBounds: CanvasUtils.getViewportBounds(
                                flickable.contentX, 
                                flickable.contentY, 
                                flickable.width, 
                                flickable.height, 
                                root.zoomLevel
                            )
                            
                            // Check if element is visible in viewport with margin
                            property bool isInViewport: CanvasUtils.isElementInViewport(elementBounds, viewportBounds, 100)
                            
                            // Only load when visible or recently visible
                            active: isInViewport
                            asynchronous: true
                            
                            sourceComponent: {
                                if (!active) return null
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
                        color: Config.elementCreationPreviewColor
                        border.color: Config.elementCreationPreviewBorderColor
                        border.width: Config.elementCreationPreviewBorderWidth
                        
                        property var creationRect: creationDragHandler.active ? creationDragHandler.getCreationRect() : Qt.rect(0, 0, 0, 0)
                        
                        x: CanvasUtils.canvasXToRelative(creationRect.x)
                        y: CanvasUtils.canvasYToRelative(creationRect.y)
                        width: creationRect.width
                        height: creationRect.height
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
        
        // Cached canvas coordinates to avoid recomputation
        property point lastCanvasPoint: Qt.point(0, 0)
        
        // Convert mouse position to canvas coordinates
        function toCanvasCoords(mouseX, mouseY) {
            return CanvasUtils.viewportToCanvas(mouseX, mouseY, flickable.contentX, flickable.contentY, root.zoomLevel)
        }
        
        // Update cached canvas point
        function updateCanvasPoint(mouse) {
            lastCanvasPoint = CanvasUtils.viewportToCanvas(mouse.x, mouse.y, flickable.contentX, flickable.contentY, root.zoomLevel)
            return lastCanvasPoint
        }
        
        // Middle mouse button panning
        property bool isPanning: false
        property point lastPanPoint
        
        onPressed: (mouse) => {
            // Update cached canvas point
            updateCanvasPoint(mouse)
            
            if (mouse.button === Qt.MiddleButton) {
                isPanning = true
                lastPanPoint = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                if (controller.mode === "select") {
                    // Check if we hit an element
                    var element = controller.hitTest(lastCanvasPoint.x, lastCanvasPoint.y)
                    if (element) {
                        // Start dragging element
                        controller.handleMousePress(lastCanvasPoint.x, lastCanvasPoint.y)
                    } else {
                        // Start selection box
                        selectionBoxHandler.startSelection(lastCanvasPoint)
                    }
                } else {
                    // Start element creation
                    creationDragHandler.startCreation(lastCanvasPoint)
                    controller.handleMousePress(lastCanvasPoint.x, lastCanvasPoint.y)
                }
            }
        }
        
        onPositionChanged: (mouse) => {
            if (isPanning) {
                // Pan the view - no need to update canvas coords during pan
                flickable.contentX -= mouse.x - lastPanPoint.x
                flickable.contentY -= mouse.y - lastPanPoint.y
                lastPanPoint = Qt.point(mouse.x, mouse.y)
            } else {
                // Update canvas point for non-panning operations
                updateCanvasPoint(mouse)
                
                // Track hover when not dragging
                if (!pressed && controller.mode === "select") {
                    root.hoveredElement = controller.hitTest(lastCanvasPoint.x, lastCanvasPoint.y)
                }
                
                if (selectionBoxHandler.active) {
                    selectionBoxHandler.updateSelection(lastCanvasPoint)
                } else if (creationDragHandler.active) {
                    creationDragHandler.updateCreation(lastCanvasPoint)
                } else if (pressed) {
                    // Dragging elements or updating creation
                    controller.handleMouseMove(lastCanvasPoint.x, lastCanvasPoint.y)
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
                // Update canvas point one final time for release
                updateCanvasPoint(mouse)
                console.log("  Calling controller.handleMouseRelease at:", lastCanvasPoint.x, lastCanvasPoint.y)
                controller.handleMouseRelease(lastCanvasPoint.x, lastCanvasPoint.y)
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
                var newContentPos = CanvasUtils.calculateContentPositionForCenter(
                    canvasPoint.x, canvasPoint.y, 
                    wheel.x * 2, wheel.y * 2,  // Use mouse position as "viewport size" to keep it centered
                    root.zoomLevel
                )
                flickable.contentX = newContentPos.x - wheel.x
                flickable.contentY = newContentPos.y - wheel.y
                
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
            // Update selection through controller
            if (controller) {
                controller.selectElementsInRect(rect)
            }
        }
    }
    
    // Creation drag handler
    CreationDragHandler {
        id: creationDragHandler
        controller: root.controller
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