import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../CanvasUtils.js" as Utils
import "."

Item {
    id: root
    
    // Required properties from subclasses
    property var controller
    property var selectionManager
    property var elementModel
    
    // Canvas bounds configuration
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    property real canvasMaxX: 10000
    property real canvasMaxY: 10000
    
    // Zoom configuration
    property real zoom: 1.0
    property real minZoom: 0.1
    property real maxZoom: 5.0
    
    // Canvas type - to be overridden by subclasses
    property string canvasType: "base"
    
    // Edge preview state - to be provided by ScriptCanvas
    property bool isEdgePreview: false
    
    // Mouse tracking
    property point lastMousePosition: Qt.point(0, 0)
    property bool isResizing: false
    
    // Expose internal components for viewport overlay
    property alias flickable: flick
    property alias canvasArea: canvasArea
    property alias zoomLevel: root.zoom  // Alias for ViewportOverlay compatibility
    
    // Expose content layer through default property
    default property alias contentData: contentLayer.data
    
    Component.onCompleted: {
        centerViewAtOrigin()
    }
    
    // Center the view at canvas origin (0,0)
    function centerViewAtOrigin() {
        var centerPos = Utils.calculateCenterPosition(
            Qt.point(0, 0),
            flick.width,
            flick.height,
            canvasMinX,
            canvasMinY,
            zoom
        )
        
        flick.contentX = centerPos.x
        flick.contentY = centerPos.y
    }
    
    // Watch for mode changes (keeping for subclasses that might need it)
    Connections {
        target: controller
        function onModeChanged() {
            // Subclasses can override to handle mode changes
        }
    }
    
    // Main canvas area
    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: (root.canvasMaxX - root.canvasMinX) * root.zoom
        contentHeight: (root.canvasMaxY - root.canvasMinY) * root.zoom
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        
        // Disable interactive dragging during edge preview mode or creation modes
        interactive: {
            // Disable for script canvas edge preview
            if (root.canvasType === "script" && root.isEdgePreview) {
                return false
            }
            // Disable for design/variant canvas creation modes
            if ((root.canvasType === "design" || root.canvasType === "variant") && 
                controller && controller.mode !== CanvasController.Select) {
                return false
            }
            return true
        }
        
        // Watch for edge preview changes to immediately stop any ongoing interaction
        onInteractiveChanged: {
            if (!interactive) {
                // Force stop any ongoing flick/drag
                cancelFlick()
                returnToBounds()
            }
        }
        
        Item {
            id: canvasArea
            width: root.canvasMaxX - root.canvasMinX
            height: root.canvasMaxY - root.canvasMinY
            scale: root.zoom
            transformOrigin: Item.TopLeft
            
            // Content layer for subclasses
            Item {
                id: contentLayer
                anchors.fill: parent
            }
        }
        
        ScrollIndicator.vertical: ScrollIndicator { active: true }
        ScrollIndicator.horizontal: ScrollIndicator { active: true }
    }
    
    // Gesture handling for pan and zoom
    Item {
        id: gestureHandler
        anchors.fill: parent
        
        // Disable gesture handling during edge preview in script canvas
        enabled: root.canvasType !== "script" || !root.isEdgePreview
        
        // Mouse area for middle-button panning
        MouseArea {
            id: panArea
            anchors.fill: parent
            acceptedButtons: Qt.MiddleButton
            
            // Disable middle-button panning during creation modes
            enabled: {
                if ((root.canvasType === "design" || root.canvasType === "variant") && 
                    controller && controller.mode !== CanvasController.Select) {
                    return false
                }
                return true
            }
            
            property bool isPanning: false
            property point lastPanPosition
            
            onPressed: (mouse) => {
                isPanning = true
                lastPanPosition = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            }
            
            onPositionChanged: (mouse) => {
                if (isPanning) {
                    var dx = mouse.x - lastPanPosition.x
                    var dy = mouse.y - lastPanPosition.y
                    lastPanPosition = Qt.point(mouse.x, mouse.y)
                    
                    // Don't pan during edge preview mode
                    if (root.canvasType === "script" && root.isEdgePreview) {
                        return
                    }
                    flick.contentX -= dx
                    flick.contentY -= dy
                }
            }
            
            onReleased: {
                isPanning = false
                cursorShape = Qt.ArrowCursor
            }
        }
        
        // Mouse area for hover tracking and wheel events
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            hoverEnabled: true
            
            property real throttleDelay: 16 // 60fps
            property point pendingMousePosition
            property bool hasPendingMouseMove: false
            
            Timer {
                id: throttleTimer
                interval: hoverArea.throttleDelay
                repeat: false
                onTriggered: {
                    if (hoverArea.hasPendingMouseMove) {
                        var canvasPt = Utils.viewportToCanvas(
                            hoverArea.pendingMousePosition,
                            flick.contentX, flick.contentY, root.zoom, 
                            root.canvasMinX, root.canvasMinY
                        )
                        root.handleHover(canvasPt)
                        hoverArea.hasPendingMouseMove = false
                    }
                }
            }
            
            onPositionChanged: (mouse) => {
                // Update last mouse position immediately for hover badge
                root.lastMousePosition = Qt.point(mouse.x, mouse.y)
                
                // Throttle hover events
                pendingMousePosition = Qt.point(mouse.x, mouse.y)
                hasPendingMouseMove = true
                if (!throttleTimer.running) {
                    throttleTimer.start()
                }
            }
            
            onExited: {
                root.handleExit()
            }
            
            // Handle wheel events for zoom
            onWheel: (wheel) => {
                if (wheel.modifiers & Qt.ControlModifier) {
                    var delta = wheel.angleDelta.y
                    if (Math.abs(delta) > 0) {
                        var scaleFactor = delta > 0 ? 1.1 : 0.9
                        var canvasPt = Utils.viewportToCanvas(
                            Qt.point(wheel.x, wheel.y),
                            flick.contentX, flick.contentY, root.zoom, 
                            root.canvasMinX, root.canvasMinY
                        )
                        
                        // Store old zoom for calculations
                        var oldZoom = root.zoom
                        
                        // Calculate new zoom level
                        root.zoom = Utils.clamp(oldZoom * scaleFactor, root.minZoom, root.maxZoom)
                        
                        // Keep the zoom point stable in viewport
                        var viewportX = (canvasPt.x - root.canvasMinX) * oldZoom - flick.contentX
                        var viewportY = (canvasPt.y - root.canvasMinY) * oldZoom - flick.contentY
                        
                        // Recalculate content position to keep zoom point fixed
                        flick.contentX = (canvasPt.x - root.canvasMinX) * root.zoom - viewportX
                        flick.contentY = (canvasPt.y - root.canvasMinY) * root.zoom - viewportY
                        
                        wheel.accepted = true
                    }
                } else {
                    wheel.accepted = false
                }
            }
        }
        
        // Pinch handler for touch zoom
        PinchHandler {
            id: pinchHandler
            target: null  // We don't want it to directly modify anything
            
            property real startZoom: 1.0
            property point startCentroid
            
            onActiveChanged: {
                if (active) {
                    startZoom = root.zoom
                    startCentroid = Utils.viewportToCanvas(
                        centroid.position,
                        flick.contentX, flick.contentY, root.zoom, 
                        root.canvasMinX, root.canvasMinY
                    )
                }
            }
            
            onScaleChanged: {
                if (active && Math.abs(scale - 1.0) > 0.01) {
                    // Store old zoom for calculations
                    var oldZoom = root.zoom
                    
                    // Calculate new zoom level
                    root.zoom = Utils.clamp(oldZoom * scale, root.minZoom, root.maxZoom)
                    
                    // Keep the zoom point stable in viewport
                    var viewportX = (startCentroid.x - root.canvasMinX) * startZoom - flick.contentX
                    var viewportY = (startCentroid.y - root.canvasMinY) * startZoom - flick.contentY
                    
                    // Recalculate content position to keep zoom point fixed
                    flick.contentX = (startCentroid.x - root.canvasMinX) * root.zoom - viewportX
                    flick.contentY = (startCentroid.y - root.canvasMinY) * root.zoom - viewportY
                }
            }
        }
    }
    
    
    // Default implementations - subclasses override these
    function handleHover(pt) {
        // Override in subclasses
    }
    
    function handleExit() {
        // Override in subclasses
    }
    
    // Get content layer for backward compatibility
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
                controller.mode = CanvasController.Select
            }
        }
    }
}