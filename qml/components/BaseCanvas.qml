import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../CanvasUtils.js" as Utils
import "."

Item {
    id: root
    focus: true  // Enable keyboard focus
    
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
    property real maxZoom: 2.5
    
    // Debug zoom changes
    onZoomChanged: {
        console.log("BaseCanvas zoom changed to:", Math.round(zoom * 10) / 10)
    }
    
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
        // Delay centering until the flickable has a valid size
        if (flick.width > 0 && flick.height > 0) {
            centerViewAtOrigin()
        } else {
            // Wait for size to be available
            centerTimer.start()
        }
    }
    
    // Timer to retry centering when flickable gets sized
    Timer {
        id: centerTimer
        interval: 16  // 60fps
        repeat: true
        onTriggered: {
            if (flick.width > 0 && flick.height > 0) {
                centerViewAtOrigin()
                stop()
            }
        }
    }
    
    // Center the view so that (0,0) is at the center of the viewport
    function centerViewAtOrigin() {
        // Use the CanvasUtils function to center (0,0) in the viewport
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
        console.log("BaseCanvas.centerViewAtOrigin - contentX:", flick.contentX, "contentY:", flick.contentY,
                   "viewport size:", flick.width, "x", flick.height, "zoom:", zoom)
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
                        var scaleFactor = delta > 0 ? 1.025 : 0.975
                        
                        // Get the mouse position in viewport coordinates
                        var viewportPt = Qt.point(wheel.x, wheel.y)
                        
                        // Convert to canvas coordinates at current zoom
                        var canvasPt = Utils.viewportToCanvas(
                            viewportPt,
                            flick.contentX, flick.contentY, root.zoom, 
                            root.canvasMinX, root.canvasMinY
                        )
                        
                        // Calculate new zoom level
                        var newZoom = Utils.clamp(root.zoom * scaleFactor, root.minZoom, root.maxZoom)
                        
                        // Calculate new content position to keep cursor point stable
                        var newContentPos = Utils.calculateStableZoomPosition(
                            canvasPt, viewportPt, root.canvasMinX, root.canvasMinY, newZoom
                        )
                        
                        // Apply zoom and new position
                        root.zoom = newZoom
                        flick.contentX = Math.round(newContentPos.x)
                        flick.contentY = Math.round(newContentPos.y)
                        
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
            property real initialZoom: 1.0  // The zoom when gesture truly starts
            property point startCentroid
            property point startViewportCentroid  // Store the initial viewport position
            property point startContentPos  // Store the initial content position
            
            onActiveChanged: {
                if (active) {
                    console.log("ZOOM START - root.zoom raw value:", root.zoom, "initial scale:", scale, "activeScale:", activeScale)
                    // Round the start zoom to prevent accumulation errors
                    startZoom = Math.round(root.zoom * 100) / 100  // Round to 2 decimal places
                    startViewportCentroid = centroid.position
                    startContentPos = Qt.point(flick.contentX, flick.contentY)
                    startCentroid = Utils.viewportToCanvas(
                        startViewportCentroid,
                        startContentPos.x, startContentPos.y, root.zoom, 
                        root.canvasMinX, root.canvasMinY
                    )
                    console.log("PinchHandler started - startZoom:", startZoom, "rounded:", Math.round(startZoom * 10) / 10, "startCentroid:", Math.round(startCentroid.x), Math.round(startCentroid.y), "startContentPos:", Math.round(startContentPos.x), Math.round(startContentPos.y))
                } else {
                    // Log when gesture ends
                    console.log("ZOOM END - final zoom:", Math.round(root.zoom * 10) / 10, "final scale:", scale, "activeScale:", activeScale)
                }
            }
            
            onScaleChanged: {
                if (active && Math.abs(scale - 1.0) > 0.01) {
                    console.log("PinchHandler scale:", Math.round(scale * 10) / 10, "activeScale:", Math.round(activeScale * 10) / 10, "currentZoom:", Math.round(root.zoom * 10) / 10)
                    
                    // Calculate new zoom level based on the active scale (scale change during this gesture)
                    // activeScale represents the scale factor relative to when the gesture started
                    var newZoom = Utils.clamp(startZoom * activeScale, root.minZoom, root.maxZoom)
                    
                    // Use the stored start viewport centroid for stable zoom
                    // This prevents jumps caused by the centroid moving during the gesture
                    var newContentPos = Utils.calculateStableZoomPosition(
                        startCentroid, startViewportCentroid, root.canvasMinX, root.canvasMinY, newZoom
                    )
                    
                    // Debug the calculation
                    console.log("Zoom calculation - startCentroid:", startCentroid.x, startCentroid.y, 
                               "startViewport:", startViewportCentroid.x, startViewportCentroid.y,
                               "newZoom:", newZoom, "minX/Y:", root.canvasMinX, root.canvasMinY)
                    
                    console.log("PinchHandler applying - newZoom:", Math.round(newZoom * 10) / 10, "newContentPos:", Math.round(newContentPos.x), Math.round(newContentPos.y))
                    
                    // Apply zoom and new position
                    root.zoom = newZoom
                    flick.contentX = Math.round(newContentPos.x)
                    flick.contentY = Math.round(newContentPos.y)
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
    
    // Get current viewport state
    function getViewportState() {
        return {
            contentX: flick.contentX,
            contentY: flick.contentY,
            zoom: root.zoom
        }
    }
    
    // Set viewport state
    function setViewportState(state) {
        if (state && state.contentX !== undefined && state.contentY !== undefined && state.zoom !== undefined) {
            root.zoom = state.zoom
            flick.contentX = state.contentX
            flick.contentY = state.contentY
        }
    }
    
    // Keyboard shortcuts for Delete key
    Shortcut {
        sequence: "Delete"
        onActivated: {
            console.log("Delete key pressed in BaseCanvas")
            if (controller) {
                console.log("Controller exists, calling deleteSelectedElements")
                controller.deleteSelectedElements()
            } else {
                console.log("Controller is null!")
            }
        }
    }
    
    // Alternative shortcut for Backspace key (Mac compatibility)
    Shortcut {
        sequence: "Backspace"
        onActivated: {
            console.log("Backspace key pressed in BaseCanvas")
            if (controller) {
                console.log("Controller exists, calling deleteSelectedElements")
                controller.deleteSelectedElements()
            } else {
                console.log("Controller is null!")
            }
        }
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