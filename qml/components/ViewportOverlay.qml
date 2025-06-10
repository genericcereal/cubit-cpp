import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

// ViewportOverlay provides a layer for UI elements that should maintain
// their visual size regardless of canvas zoom level
Item {
    id: root
    objectName: "viewportOverlay"
    
    // Core properties from CanvasView
    property var canvasView
    property real zoomLevel: canvasView ? canvasView.zoomLevel : 1.0
    property var flickable: canvasView ? canvasView.flickable : null
    property var hoveredElement: null
    property var selectionManager: canvasView ? canvasView.selectionManager : null
    property var selectedElements: selectionManager ? selectionManager.selectedElements : []
    property var creationDragHandler: canvasView ? canvasView.creationDragHandler : null
    property var controller: canvasView ? canvasView.controller : null
    
    // Canvas bounds from canvasView
    property real canvasMinX: canvasView ? canvasView.canvasMinX : 0
    property real canvasMinY: canvasView ? canvasView.canvasMinY : 0
    
    // Selection box visual during drag selection
    SelectionBox {
        id: selectionBox
        selectionBoxHandler: canvasView ? canvasView.selectionBoxHandler : null
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
    
    // Calculate bounding box for all selected elements
    // These properties are used by the controls to determine their size and position
    property real selectionBoundingX: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
        }
        return minX
    }
    
    property real selectionBoundingY: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
        }
        return minY
    }
    
    property real selectionBoundingWidth: {
        if (selectedElements.length === 0) return 0
        var minX = Number.MAX_VALUE
        var maxX = -Infinity  
        for (var i = 0; i < selectedElements.length; i++) {
            minX = Math.min(minX, selectedElements[i].x)
            maxX = Math.max(maxX, selectedElements[i].x + selectedElements[i].width)
        }
        return maxX - minX
    }
    
    property real selectionBoundingHeight: {
        if (selectedElements.length === 0) return 0
        var minY = Number.MAX_VALUE
        var maxY = -Infinity
        for (var i = 0; i < selectedElements.length; i++) {
            minY = Math.min(minY, selectedElements[i].y)
            maxY = Math.max(maxY, selectedElements[i].y + selectedElements[i].height)
        }
        return maxY - minY
    }
    
    // Test Controls instance - 100x100 at position (200, 200)
    Controls {
        id: testControls
        controlX: 200
        controlY: 200
        controlWidth: 100
        controlHeight: 100
    }
    
    // Hover indicator for elements under mouse
    HoverIndicator {
        hoveredElement: root.hoveredElement
        selectionManager: root.selectionManager
        zoomLevel: root.zoomLevel
        flickable: root.flickable
        canvasMinX: root.canvasMinX
        canvasMinY: root.canvasMinY
    }
}