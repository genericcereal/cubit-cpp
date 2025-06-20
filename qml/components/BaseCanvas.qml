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
        
        // Disable interactive dragging during edge preview mode
        interactive: root.canvasType !== "script" || !root.isEdgePreview
        
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
    CanvasGestureHandler {
        id: gestureHandler
        anchors.fill: parent
        
        contentX: flick.contentX
        contentY: flick.contentY
        zoom: root.zoom
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        
        // Disable gesture handling during edge preview in script canvas
        enabled: root.canvasType !== "script" || !root.isEdgePreview
        
        onHover: (pt) => {
            root.handleHover(pt)
        }
        
        onExitCanvas: {
            root.handleExit()
        }
        
        onPan: (dx, dy) => {
            // Don't pan during edge preview mode
            if (root.canvasType === "script" && root.isEdgePreview) {
                return
            }
            flick.contentX -= dx
            flick.contentY -= dy
        }
        
        onPinch: (center, scaleFactor) => {
            // Store old zoom for calculations
            var oldZoom = root.zoom
            
            // Calculate new zoom level
            root.zoom = Utils.clamp(oldZoom * scaleFactor, root.minZoom, root.maxZoom)
            
            // Keep the zoom point stable in viewport
            // The point 'center' is in canvas coordinates, we need to find where it is in the viewport
            // before zoom, then adjust content position to keep it at the same viewport location
            var viewportX = (center.x - root.canvasMinX) * oldZoom - flick.contentX
            var viewportY = (center.y - root.canvasMinY) * oldZoom - flick.contentY
            
            // Recalculate content position to keep zoom point fixed
            flick.contentX = (center.x - root.canvasMinX) * root.zoom - viewportX
            flick.contentY = (center.y - root.canvasMinY) * root.zoom - viewportY
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