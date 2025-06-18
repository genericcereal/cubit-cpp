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
        var centerPos = viewportCache.calculateContentPositionForCenter(0, 0)
        flickable.contentX = centerPos.x + viewportCache.viewportWidth / 2
        flickable.contentY = centerPos.y + viewportCache.viewportHeight / 2
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
    property alias hoveredElement: inputController.hoveredElement
    
    // Viewport cache for efficient coordinate transformations and visibility
    ViewportCache {
        id: viewportCache
        contentX: flickable.contentX
        contentY: flickable.contentY
        viewportWidth: flickable.width
        viewportHeight: flickable.height
        zoomLevel: root.zoomLevel
    }
    
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
        interactive: !pinchHandler.active // Disable flickable dragging during pinch
        
        // Canvas content container
        Item {
            id: canvasContent
            width: ViewportCache.CANVAS_WIDTH
            height: ViewportCache.CANVAS_HEIGHT
            
            // Transform origin for zooming
            transformOrigin: Item.TopLeft
            scale: root.zoomLevel
            
            // Canvas area where elements are placed
            Item {
                id: canvasArea
                x: 0
                y: 0
                width: ViewportCache.CANVAS_WIDTH
                height: ViewportCache.CANVAS_HEIGHT
                
                
                // Elements layer
                ElementLayer {
                    id: elementsLayer
                    anchors.fill: parent
                    elementModel: root.elementModel
                    viewportCache: viewportCache
                    zoomLevel: root.zoomLevel
                    frameComponent: frameComponent
                    textComponent: textComponent
                }
                
                // Overlay layers (creation preview, etc.)
                OverlayLayers {
                    id: overlayLayers
                    anchors.fill: parent
                    z: 100
                    creationDragHandler: creationDragHandler
                    controller: root.controller
                    viewportCache: viewportCache
                }
            }
        }
        
        // Scroll indicators
        ScrollIndicator.vertical: ScrollIndicator { active: true }
        ScrollIndicator.horizontal: ScrollIndicator { active: true }
    }
    
    // Touch pinch gesture handler
    PinchHandler {
        id: pinchHandler
        target: null // We'll handle the transformation manually
        
        property point pinchCenterStart
        property real initialZoom
        
        onActiveChanged: {
            if (active) {
                // Store initial state when pinch starts
                pinchCenterStart = centroid.position
                initialZoom = root.zoomLevel
            }
        }
        
        onScaleChanged: {
            // Calculate new zoom level
            var newZoom = Math.max(root.minZoom, Math.min(root.maxZoom, initialZoom * activeScale))
            
            if (viewportCache && newZoom !== root.zoomLevel) {
                // Get pinch center in canvas coordinates
                var canvasPoint = viewportCache.viewportToCanvas(pinchCenterStart.x, pinchCenterStart.y)
                
                // Apply zoom
                root.zoomLevel = newZoom
                
                // Adjust content position to keep pinch center stable
                var newContentPos = viewportCache.calculateContentPositionForCenter(canvasPoint.x, canvasPoint.y)
                flickable.contentX = newContentPos.x + viewportCache.viewportWidth / 2 - pinchCenterStart.x
                flickable.contentY = newContentPos.y + viewportCache.viewportHeight / 2 - pinchCenterStart.y
            }
        }
    }
    
    // Mouse wheel handler for zooming
    WheelHandler {
        id: wheelHandler
        acceptedModifiers: Qt.ControlModifier
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        target: null // We'll handle the transformation manually
        
        property real wheelThrottleDelay: 16 // 60fps
        property point pendingWheelPoint
        property real pendingRotation: 0
        property bool hasPendingWheel: false
        
        Timer {
            id: wheelTimer
            interval: wheelHandler.wheelThrottleDelay
            repeat: false
            onTriggered: {
                if (wheelHandler.hasPendingWheel) {
                    processWheel(wheelHandler.pendingWheelPoint, wheelHandler.pendingRotation)
                    wheelHandler.hasPendingWheel = false
                    wheelHandler.pendingRotation = 0
                }
            }
        }
        
        onWheel: (event) => {
            // Accumulate wheel events for throttling
            pendingWheelPoint = event.point.position
            pendingRotation += event.angleDelta.y
            hasPendingWheel = true
            
            if (!wheelTimer.running) {
                wheelTimer.start()
            }
        }
        
        function processWheel(wheelPoint, angleDelta) {
            var zoomDelta = angleDelta > 0 ? 1.1 : 0.9
            var newZoom = Math.max(root.minZoom, Math.min(root.maxZoom, root.zoomLevel * zoomDelta))
            
            if (viewportCache && newZoom !== root.zoomLevel) {
                // Get mouse position in canvas coordinates before zoom
                var canvasPoint = viewportCache.viewportToCanvas(wheelPoint.x, wheelPoint.y)
                
                // Apply zoom
                root.zoomLevel = newZoom
                
                // Adjust content position to keep mouse point stable
                var newContentPos = viewportCache.calculateContentPositionForCenter(canvasPoint.x, canvasPoint.y)
                flickable.contentX = newContentPos.x + viewportCache.viewportWidth / 2 - wheelPoint.x
                flickable.contentY = newContentPos.y + viewportCache.viewportHeight / 2 - wheelPoint.y
            }
        }
    }
    
    // Input controller for mouse/touch interactions
    InputController {
        id: inputController
        controller: root.controller
        selectionManager: root.selectionManager
        flickable: flickable
        selectionBoxHandler: selectionBoxHandler
        creationDragHandler: creationDragHandler
        viewportCache: viewportCache
    }
    
    // Update parentId of selected elements when hovering
    Connections {
        target: inputController
        function onHoveredElementChanged() {
            if (!selectionManager || selectionManager.selectionCount === 0) {
                return
            }
            
            // Get all selected elements
            var selectedElements = selectionManager.selectedElements
            
            // Update parentId for each selected element
            for (var i = 0; i < selectedElements.length; i++) {
                var element = selectedElements[i]
                if (element) {
                    if (inputController.hoveredElement) {
                        // Don't set an element as its own parent
                        if (inputController.hoveredElement.elementId !== element.elementId) {
                            // Set parentId to the hovered element's ID
                            console.log("Setting parentId of", element.elementId, "to", inputController.hoveredElement.elementId)
                            element.parentId = inputController.hoveredElement.elementId
                        }
                    } else {
                        // Clear parentId when no element is hovered
                        console.log("Clearing parentId of", element.elementId)
                        element.parentId = ""
                    }
                }
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