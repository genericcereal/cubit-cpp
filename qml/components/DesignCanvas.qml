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
        // Canvas background - handles background clicks
        CanvasBackground {
            id: canvasBackground
            anchors.fill: parent
            canvas: root
            z: -1  // Ensure it's behind all other elements
            
            onClicked: (canvasPoint) => {
                // Forward the click to the canvas handler
                root.handleClick(canvasPoint)
            }
        },
        
        // Element layer
        DesignElementLayer {
            id: elementLayer
            elementModel: root.elementModel
            canvasMinX: root.canvasMinX
            canvasMinY: root.canvasMinY
        },
        
        // Overlay layer (hover highlights)
        DesignOverlayLayers {
            id: overlayLayer
            controller: root.controller
            selectionManager: root.selectionManager
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
    
    
    // Implement behavior by overriding handler functions
    function handleDragStart(pt) {
        console.log("DesignCanvas.handleDragStart:", pt.x, pt.y, "mode:", controller.mode)
        // In design canvas, we don't handle drag start - elements can only be moved via controls
        // This prevents direct element dragging
    }
    
    function handleDragMove(pt) {
        // In design canvas, we don't handle drag move - elements can only be moved via controls
    }
    
    function handleDragEnd(pt) {
        // In design canvas, we don't handle drag end - elements can only be moved via controls
    }
    
    function handleClick(pt) {
        console.log("DesignCanvas.handleClick:", pt.x, pt.y, "mode:", controller.mode)
        if (controller.mode === CanvasController.Select) {
            // In select mode, handle click for selection
            controller.handleMousePress(pt.x, pt.y)
            controller.handleMouseRelease(pt.x, pt.y)
        } else {
            // In creation modes, create element with default size at click position
            var defaultSize = 100
            var x = pt.x - defaultSize/2
            var y = pt.y - defaultSize/2
            
            // Create element through controller - it will automatically select the new element
            controller.handleMousePress(x, y)
            controller.handleMouseMove(x + defaultSize, y + defaultSize)
            controller.handleMouseRelease(x + defaultSize, y + defaultSize)
            
            // Switch back to select mode so user can immediately resize with controls
            controller.mode = CanvasController.Select
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