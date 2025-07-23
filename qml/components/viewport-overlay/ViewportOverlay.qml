import QtQuick
import QtQml
import Cubit 1.0
import Cubit.UI 1.0

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
}