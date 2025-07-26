import QtQuick
import QtQml
import Cubit 1.0
import Cubit.UI 1.0
import "OverlayUtils.js" as OverlayUtils

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    objectName: "viewportOverlay"
    
    // Core properties
    property var canvasView
    property var canvas: canvasView?.canvas ?? null
    property real zoomLevel: canvasView?.zoomLevel ?? 1.0
    property var flickable: canvasView?.flickable ?? null
    property var hoveredElement: null
    property var selectionManager: canvas?.selectionManager ?? null
    property var selectedElements: selectionManager?.selectedElements ?? []
    property var controller: canvas?.controller ?? null
    property string canvasType: canvasView?.canvasType ?? "design"
    property var designControlsController: null
    
    Component.onCompleted: {
        // ViewportOverlay initialized
    }
    
    onCanvasChanged: {
        // Canvas changed
    }
    
    // Canvas bounds from canvasView
    property real canvasMinX: canvasView?.canvasMinX ?? 0
    property real canvasMinY: canvasView?.canvasMinY ?? 0
    
    // Expose the selection controls (from DesignControlsOverlay)
    property alias selectionControls: designControlsOverlay.selectionControls
    
    // Drag-to-select functionality
    SelectionArea {
        id: selectionArea
        anchors.fill: parent
        controller: root.controller
        selectionManager: root.selectionManager
        flickable: root.flickable
        zoomLevel: root.zoomLevel
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        selectionControls: root.selectionControls
    }
    
    // Selection bounding boxes (design vs script)
    SelectionBounds {
        id: selectionBounds
        selectionManager: root.selectionManager
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        canvasType: root.canvasType
    }
    
    // Design controls overlay (design/variant canvases only)
    DesignControlsOverlay {
        id: designControlsOverlay
        anchors.fill: parent
        canvasView: root.canvasView
        controller: root.controller
        selectionManager: root.selectionManager
        flickable: root.flickable
        zoomLevel: root.zoomLevel
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        canvasType: root.canvasType
        designControlsController: root.designControlsController
    }
    
    // Prototype controls (viewable area, play button)
    PrototypeControls {
        id: prototypeControls
        anchors.fill: parent
        canvasView: root.canvasView
        controller: root.controller
        flickable: root.flickable
        canvas: root.canvas
    }
    
    // Unified mouse tracking for the entire viewport overlay
    MouseArea {
        id: unifiedMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton  // Don't capture any button events
        propagateComposedEvents: true
        z: -1  // Below all other components
        
        property var currentHandler: null
        
        onEntered: {
            if (root.canvasView) {
                root.canvasView.cursorInCanvas = true
            }
        }
        
        onExited: {
            if (root.canvasView) {
                root.canvasView.cursorInCanvas = false
                if (root.controller && root.controller.hasOwnProperty('hoveredElement')) {
                    root.controller.hoveredElement = null
                }
            }
        }
        
        onPositionChanged: (mouse) => {
            // Update last mouse position
            if (root.canvasView) {
                root.canvasView.lastMousePosition = Qt.point(mouse.x, mouse.y)
            }
            
            // Convert to canvas coordinates
            var canvasPoint = OverlayUtils.toCanvas(Qt.point(mouse.x, mouse.y), 
                flickable, zoomLevel, canvasMinX, canvasMinY)
            
            // Check what's under the mouse
            var overControls = false
            if (designControlsOverlay.shapeControls && designControlsOverlay.shapeControls.visible && 
                designControlsOverlay.shapeControls.contains(
                    designControlsOverlay.shapeControls.mapFromItem(root, mouse.x, mouse.y))) {
                overControls = true
            } else if (designControlsOverlay.selectionControls.visible && 
                       designControlsOverlay.selectionControls.contains(
                           designControlsOverlay.selectionControls.mapFromItem(root, mouse.x, mouse.y))) {
                overControls = true
            }
            
            // Only update hover when not over controls and in select mode
            if (!overControls && controller && controller.mode === CanvasController.Select) {
                // Directly use HitTestService to find hovered element
                if (controller.hitTestService) {
                    var element = controller.hitTestService.hitTestForHover(canvasPoint.x, canvasPoint.y)
                    if (controller && controller.hasOwnProperty('hoveredElement')) {
                        controller.hoveredElement = element
                    }
                }
            } else if (overControls && controller && controller.hasOwnProperty('hoveredElement')) {
                // Clear hover when over controls
                controller.hoveredElement = null
            }
        }
    }
}