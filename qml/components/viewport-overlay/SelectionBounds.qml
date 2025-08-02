import QtQuick
import Cubit 1.0

// SelectionBounds wraps SelectionBoundingBox and NodeSelectionBounds
Item {
    id: root
    
    // Properties passed from ViewportOverlay
    property var selectionManager
    property real zoomLevel: 1.0
    property var flickable
    property real canvasMinX: 0
    property real canvasMinY: 0
    property string canvasType: "design"
    property var shapeControlsController: null
    
    // Bounding box properties from selection manager
    readonly property real selectionBoundingX: selectionManager?.boundingX ?? 0
    readonly property real selectionBoundingY: selectionManager?.boundingY ?? 0
    readonly property real selectionBoundingWidth: selectionManager?.boundingWidth ?? 0
    readonly property real selectionBoundingHeight: selectionManager?.boundingHeight ?? 0
    
    // Selection bounding box for all selected elements (design and variant canvases)
    SelectionBoundingBox {
        id: selectionBoundingBox
        visible: (canvasType === "design" || canvasType === "variant") && 
                 selectionManager && 
                 selectionManager.hasVisualSelection &&
                 selectionBoundingWidth > 0 && 
                 selectionBoundingHeight > 0 &&
                 !(root.shapeControlsController && root.shapeControlsController.isEditingShape) &&
                 !(root.shapeControlsController && root.shapeControlsController.isShapeControlDragging)
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        // Use selection bounds directly
        boundingX: root.selectionBoundingX
        boundingY: root.selectionBoundingY
        boundingWidth: root.selectionBoundingWidth
        boundingHeight: root.selectionBoundingHeight
        z: -1  // Behind controls but above canvas
    }
    
    // Node selection bounds (only for script canvas)
    NodeSelectionBounds {
        id: nodeSelectionBounds
        visible: canvasType === "script"
        selectionManager: root.selectionManager
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
}