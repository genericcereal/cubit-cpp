import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "design"
    
    // Property to access the viewport overlay's controls
    property var viewportControls: null
    
    // Update parentId of selected elements when hovering ONLY during controls drag
    onHoveredElementChanged: {
        if (!selectionManager || selectionManager.selectionCount === 0) {
            return
        }
        
        // Only update parentId if the viewport controls are actively dragging
        if (!viewportControls || !viewportControls.dragging) {
            return
        }
        
        // Get all selected elements
        var selectedElements = selectionManager.selectedElements
        
        // Update parentId for each selected element
        for (var i = 0; i < selectedElements.length; i++) {
            var element = selectedElements[i]
            if (element) {
                if (hoveredElement) {
                    // Don't set an element as its own parent
                    if (hoveredElement.elementId !== element.elementId) {
                        // Set parentId to the hovered element's ID
                        // console.log("Setting parentId of", element.elementId, "to", hoveredElement.elementId)
                        element.parentId = hoveredElement.elementId
                    }
                } else {
                    // Clear parentId when no element is hovered
                    // console.log("Clearing parentId of", element.elementId)
                    element.parentId = ""
                }
            }
        }
    }
    
    // Additional properties for design canvas
    property var hoveredElement: null
    property alias creationDragHandler: creationDragHandler
    
    // Clear hover when selection changes
    Connections {
        target: selectionManager
        function onSelectionChanged() {
            // If the hovered element is now selected, clear the hover
            if (hoveredElement && selectionManager.isSelected(hoveredElement)) {
                hoveredElement = null
            }
        }
    }
    
    // Clear hover when mode changes
    Connections {
        target: controller
        function onModeChanged() {
            hoveredElement = null
        }
    }
    
    // Add elements into the default contentData
    contentData: [
        // Element layer
        DesignElementLayer {
            id: elementLayer
            elementModel: root.elementModel
            canvasMinX: root.canvasMinX
            canvasMinY: root.canvasMinY
        },
        
        // Overlay layer (creation previews, hover highlights)
        DesignOverlayLayers {
            id: overlayLayer
            controller: root.controller
            selectionManager: root.selectionManager
            creationDragHandler: creationDragHandler
            hoveredElement: root.hoveredElement
            canvasMinX: root.canvasMinX
            canvasMinY: root.canvasMinY
            zoom: root.zoom
        },
        
        // Script executor overlay
        DesignScriptExecutorOverlay {
            id: scriptExecutor
            elementModel: root.elementModel
            canvasVisible: root.visible
        }
    ]
    
    // Creation drag handler
    QtObject {
        id: creationDragHandler
        property bool active: false
        property point startPoint: Qt.point(0, 0)
        property point currentPoint: Qt.point(0, 0)
        
        function start(point) {
            active = true
            startPoint = point
            currentPoint = point
        }
        
        function update(point) {
            currentPoint = point
        }
        
        function end() {
            // The C++ controller already handles element creation
            // We just need to reset our state
            active = false
        }
    }
    
    // Implement behavior by overriding handler functions
    function handleDragStart(pt) {
        console.log("DesignCanvas.handleDragStart:", pt.x, pt.y, "mode:", controller.mode)
        if (controller.mode === CanvasController.Select) {
            controller.handleMousePress(pt.x, pt.y)
        } else {
            creationDragHandler.start(pt)
            controller.handleMousePress(pt.x, pt.y)
        }
    }
    
    function handleDragMove(pt) {
        if (creationDragHandler.active) {
            creationDragHandler.update(pt)
        }
        controller.handleMouseMove(pt.x, pt.y)
    }
    
    function handleDragEnd(pt) {
        if (creationDragHandler.active) {
            creationDragHandler.end()
        }
        controller.handleMouseRelease(pt.x, pt.y)
    }
    
    function handleClick(pt) {
        // Click is already handled in handleDragStart/End
        // This is called only for quick clicks without drag
        if (controller.mode === CanvasController.Select) {
            controller.handleMousePress(pt.x, pt.y)
            controller.handleMouseRelease(pt.x, pt.y)
        }
    }
    
    function handleHover(pt) {
        // Track hover for design elements (use hitTestForHover to exclude selected elements)
        if (controller.mode === CanvasController.Select) {
            hoveredElement = controller.hitTestForHover(pt.x, pt.y)
        } else {
            hoveredElement = null
        }
    }
    
    function handleExit() {
        hoveredElement = null
    }
    
    function handleSelectionRect(rect) {
        // Select design elements in rect
        controller.selectElementsInRect(rect)
    }
}