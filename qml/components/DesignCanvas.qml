import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: Application.activeCanvas && Application.activeCanvas.viewMode === "variant" ? "variant" : "design"
    
    // Property to access the viewport overlay's controls
    property var viewportControls: null
    
    // Helper function to check if elementA is a descendant of elementB
    function isDescendantOf(elementA, elementB) {
        if (!elementA || !elementB) return false
        
        var current = elementA
        var maxDepth = 100 // Prevent infinite loops
        var depth = 0
        
        while (current && current.parentId && depth < maxDepth) {
            if (current.parentId === elementB.elementId) {
                return true
            }
            // Find the parent element
            current = elementModel.getElementById(current.parentId)
            depth++
        }
        
        return false
    }
    
    // Helper function to check if an element is a child of any selected element
    function isChildOfSelected(element, selectedElements) {
        if (!element) return false
        
        for (var i = 0; i < selectedElements.length; i++) {
            if (isDescendantOf(element, selectedElements[i])) {
                return true
            }
        }
        return false
    }
    
    // Update parentId of selected elements when hovering ONLY during controls drag
    onHoveredElementChanged: {
        if (!selectionManager || selectionManager.selectionCount === 0) {
            return
        }
        
        // Removed variant mode restriction - parenting is now allowed in variant mode
        
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
                // Guard 4: Don't change parentId of children of selected elements
                // (they should move with their parent, not be reparented)
                if (isChildOfSelected(element, selectedElements)) {
                    continue
                }
                
                // Guard 5: ComponentVariants shouldn't have parents
                if (element.elementType === "ComponentVariant") {
                    continue
                }
                
                if (hoveredElement) {
                    // Guard 1: Don't set an element as its own parent
                    if (hoveredElement.elementId === element.elementId) {
                        continue
                    }
                    
                    // Guard 2: Don't set parent if it would create a circular dependency (element can't be parented to its own child)
                    if (isDescendantOf(hoveredElement, element)) {
                        continue
                    }
                    
                    // Guard 3: Don't parent to children of any selected element
                    if (isChildOfSelected(hoveredElement, selectedElements)) {
                        continue
                    }
                    
                    // Guard 4: Check if the hovered element accepts children
                    if (hoveredElement.acceptsChildren === false) {
                        continue
                    }
                    
                    // Guard 5: Don't parent relatively positioned elements to other relatively positioned elements
                    if (element.elementType === "Frame" && hoveredElement.elementType === "Frame") {
                        if (element.position === Frame.Relative && hoveredElement.position === Frame.Relative) {
                            continue
                        }
                    }
                    
                    // Only update if parentId actually changes
                    if (element.parentId !== hoveredElement.elementId) {
                        // Set parentId to the hovered element's ID
                        // console.log("Setting parentId of", element.elementId, "to", hoveredElement.elementId)
                        element.parentId = hoveredElement.elementId
                    }
                } else {
                    // Clear parentId when no element is hovered (only if not already empty)
                    if (element.parentId !== "") {
                        // console.log("Clearing parentId of", element.elementId)
                        element.parentId = ""
                    }
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
        
        // Element layer
        DesignElementLayer {
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
        if (root.controller.mode === CanvasController.Select) {
            // In select mode, handle click for selection
            root.controller.handleMousePress(pt.x, pt.y)
            root.controller.handleMouseRelease(pt.x, pt.y)
        }
        // In creation modes, don't create on click - require drag
    }
    
    function handleHover(pt) {
        // Track hover for design elements (use hitTestForHover to exclude selected elements)
        if (root.controller.mode === CanvasController.Select) {
            hoveredElement = root.controller.hitTestForHover(pt.x, pt.y)
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