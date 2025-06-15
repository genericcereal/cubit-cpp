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
    
    // Expose internal components for viewport overlay
    property alias flickable: flick
    property alias canvasArea: canvasArea
    property alias selectionBoxHandler: selectionBoxHandler
    
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
    
    // Watch for mode changes to ensure selection box is deactivated
    Connections {
        target: controller
        function onModeChanged() {
            if (controller.mode !== "select" && selectionBoxHandler.active) {
                selectionBoxHandler.endSelection()
            }
        }
    }
    
    // Background
    Rectangle {
        anchors.fill: parent
        color: "#f5f5f5"
        antialiasing: true
    }
    
    // Main canvas area
    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: (root.canvasMaxX - root.canvasMinX) * root.zoom
        contentHeight: (root.canvasMaxY - root.canvasMinY) * root.zoom
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        
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
    
    // Input handling
    CanvasInputHandler {
        id: inputHandler
        anchors.fill: parent
        
        contentX: flick.contentX
        contentY: flick.contentY
        zoom: root.zoom
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        
        property point dragStartPoint
        
        onPanned: (dx, dy) => {
            flick.contentX -= dx
            flick.contentY -= dy
        }
        
        onClicked: (pt) => {
            if (!selectionBoxHandler.active) {
                root.handleClick(pt)
            }
        }
        
        onDragStarted: (pt) => {
            dragStartPoint = pt
            root.handleDragStart(pt)
        }
        
        onDragMoved: (pt) => {
            // Start selection box if in select mode and not already dragging an element
            if (controller && controller.mode === "select" && !selectionBoxHandler.active && inputHandler.isDragging() && !controller.isDragging) {
                var element = controller.hitTest(dragStartPoint.x, dragStartPoint.y)
                if (!element) {
                    selectionBoxHandler.startSelection(dragStartPoint)
                }
            }
            
            if (selectionBoxHandler.active) {
                selectionBoxHandler.updateSelection(pt)
            } else {
                root.handleDragMove(pt)
            }
        }
        
        onDragEnded: (pt) => {
            if (selectionBoxHandler.active) {
                selectionBoxHandler.endSelection()
            } else {
                root.handleDragEnd(pt)
            }
        }
        
        onHovered: (pt) => {
            root.handleHover(pt)
        }
        
        onExited: {
            root.handleExit()
        }
        
        onZoomed: (pt, scaleFactor) => {
            // Store old zoom for calculations
            var oldZoom = root.zoom
            
            // Calculate new zoom level
            root.zoom = Utils.clamp(oldZoom * scaleFactor, root.minZoom, root.maxZoom)
            
            // Keep the zoom point stable in viewport
            // The point 'pt' is in canvas coordinates, we need to find where it is in the viewport
            // before zoom, then adjust content position to keep it at the same viewport location
            var viewportX = (pt.x - root.canvasMinX) * oldZoom - flick.contentX
            var viewportY = (pt.y - root.canvasMinY) * oldZoom - flick.contentY
            
            // Recalculate content position to keep zoom point fixed
            flick.contentX = (pt.x - root.canvasMinX) * root.zoom - viewportX
            flick.contentY = (pt.y - root.canvasMinY) * root.zoom - viewportY
        }
    }
    
    // Selection box handler
    SelectionBoxHandler {
        id: selectionBoxHandler
        selectionManager: root.selectionManager
        onSelectionRectChanged: (rect) => {
            root.handleSelectionRect(rect)
        }
    }
    
    // Default implementations - subclasses override these
    function handleClick(pt) {
        // Override in subclasses
    }
    
    function handleDragStart(pt) {
        // Override in subclasses
    }
    
    function handleDragMove(pt) {
        // Override in subclasses
    }
    
    function handleDragEnd(pt) {
        // Override in subclasses
    }
    
    function handleHover(pt) {
        // Override in subclasses
    }
    
    function handleExit() {
        // Override in subclasses
    }
    
    function handleSelectionRect(rect) {
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
                controller.setMode("select")
            }
        }
    }
}