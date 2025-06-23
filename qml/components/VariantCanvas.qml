import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "variant"
    
    // Property to access the viewport overlay's controls
    property var viewportControls: null
    
    // Variants should not be allowed to have parents - removed parentId logic
    // This was causing a recursive binding loop when dragging variants
    
    // Additional properties for variant canvas
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
        // Canvas background - handles background clicks and drags
        CanvasBackground {
            id: canvasBackground
            anchors.fill: parent
            canvas: root
            z: -1  // Ensure it's behind all other elements
            
            onClicked: (canvasPoint) => {
                // Forward the click to the canvas handler
                root.handleClick(canvasPoint)
            }
            
            onDragStarted: (canvasPoint) => {
                // Forward drag start to the canvas handler
                root.handleDragStart(canvasPoint)
            }
            
            onDragMoved: (canvasPoint) => {
                // Forward drag move to the canvas handler
                root.handleDragMove(canvasPoint)
            }
            
            onDragEnded: (canvasPoint) => {
                // Forward drag end to the canvas handler
                root.handleDragEnd(canvasPoint)
            }
        },
        
        // Element layer - shows only variants and their descendants
        VariantElementLayer {
            id: elementLayer
            elementModel: root.elementModel
            canvasMinX: root.canvasMinX
            canvasMinY: root.canvasMinY
        },
        
        // Hover highlight overlay
        Rectangle {
            id: hoverRect
            visible: root.hoveredElement !== null && root.controller.mode === CanvasController.Select
            x: root.hoveredElement ? root.hoveredElement.x - root.canvasMinX : 0
            y: root.hoveredElement ? root.hoveredElement.y - root.canvasMinY : 0
            width: root.hoveredElement ? root.hoveredElement.width : 0
            height: root.hoveredElement ? root.hoveredElement.height : 0
            color: "transparent"
            border.color: "#ffaa00"
            border.width: 2 / root.zoom
            opacity: 0.5
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
        console.log("VariantCanvas.handleDragStart:", pt.x, pt.y, "mode:", controller.mode)
        if (controller.mode !== CanvasController.Select) {
            // In creation modes, start creating element at drag start position
            controller.handleMousePress(pt.x, pt.y)
            // Set isResizing to true during creation drag
            root.isResizing = true
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleDragMove(pt) {
        if (controller.mode !== CanvasController.Select) {
            // In creation modes, update element size as if dragging bottom-right resize joint
            controller.handleMouseMove(pt.x, pt.y)
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleDragEnd(pt) {
        if (controller.mode !== CanvasController.Select) {
            // In creation modes, finish creating element
            controller.handleMouseRelease(pt.x, pt.y)
            // Switch back to select mode after creation
            controller.mode = CanvasController.Select
            // Clear isResizing when creation drag ends
            root.isResizing = false
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleClick(pt) {
        console.log("VariantCanvas.handleClick:", pt.x, pt.y, "mode:", controller.mode)
        if (controller.mode === CanvasController.Select) {
            // In select mode, handle click for selection
            controller.handleMousePress(pt.x, pt.y)
            controller.handleMouseRelease(pt.x, pt.y)
        }
        // In creation modes, don't create on click - require drag
    }
    
    function handleHover(pt) {
        // Track hover for variant elements (use hitTestForHover to exclude selected elements)
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
        // Select variant elements in rect
        controller.selectElementsInRect(rect)
    }
}