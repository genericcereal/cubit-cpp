import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../CanvasUtils.js" as Utils
import "."

BaseCanvas {
    id: root
    
    canvasType: root.canvas && root.canvas.viewMode === "variant" ? "variant" : "design"
    
    // Property to access the viewport overlay's controls
    property var viewportControls: null
    
    // Expose the element layer for viewport overlay
    property alias elementLayer: elementLayer
    
    // Signal emitted when move animation completes
    signal moveAnimationCompleted()
    
    // Update parentId of selected elements when hovering ONLY during controls drag
    onHoveredElementChanged: {
        // Only update parentId if the viewport controls are actively dragging
        if (!viewportControls || !viewportControls.dragging) {
            return
        }
        
        // Use the C++ method to handle parenting logic
        if (controller && controller.updateParentingDuringDrag) {
            controller.updateParentingDuringDrag()
        }
    }
    
    // Additional properties for design canvas
    property var hoveredElement: controller && controller.hoveredElement ? controller.hoveredElement : null
    
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
            canvas: root.canvas
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
            canvas: root.canvas
        }
    ]
    
    
    // Implement behavior by overriding handler functions
    function handleDragStart(pt) {
        // Handle drag start
        if (controller && controller.mode !== CanvasController.Select) {
            // In creation modes, start creating element at drag start position
            // Calling controller.handleMousePress
            controller.handleMousePress(pt.x, pt.y)
            // Set isResizing to true during creation drag
            root.isResizing = true
        } else {
            // Skipping, in select mode or no controller
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
            // Exit shape editing mode if clicking on background
            if (root.controller.isEditingShape) {
                root.controller.isEditingShape = false
            }
            
            // In select mode, handle click for selection
            root.controller.handleMousePress(pt.x, pt.y)
            root.controller.handleMouseRelease(pt.x, pt.y)
        } else if (root.controller.mode === CanvasController.ShapeLine) {
            // Line creation mode - handle clicks to add joints
            root.controller.handleMousePress(pt.x, pt.y)
        }
        // Other creation modes don't create on click - require drag
    }
    
    function handleHover(pt) {
        // Let the C++ controller handle hover tracking
        if (root.controller && root.controller.updateHover) {
            root.controller.updateHover(pt.x, pt.y)
        }
    }
    
    function handleExit() {
        // Clear hover through the controller
        if (root.controller && root.controller.setHoveredElement) {
            root.controller.setHoveredElement(null)
        }
    }
    
    function handleSelectionRect(rect) {
        // Select design elements in rect
        controller.selectElementsInRect(rect)
    }
    
    // Move viewport to center on a specific canvas point
    function moveToPoint(canvasPoint, isAnimated = false) {
        var targetPos = Utils.calculateCenterPosition(
            canvasPoint,
            flickable.width,
            flickable.height,
            canvasMinX,
            canvasMinY,
            zoom
        )
        
        if (isAnimated) {
            // Set isAnimating to true when starting animation
            if (root.controller) {
                root.controller.isAnimating = true
            }
            // Create animation for smooth movement
            moveAnimation.toX = targetPos.x
            moveAnimation.toY = targetPos.y
            moveAnimation.start()
        } else {
            // Immediate movement
            flickable.contentX = targetPos.x
            flickable.contentY = targetPos.y
        }
    }
    
    // Animation for smooth viewport movement
    ParallelAnimation {
        id: moveAnimation
        property real toX: 0
        property real toY: 0
        
        NumberAnimation {
            target: flickable
            property: "contentX"
            to: moveAnimation.toX
            duration: 300
            easing.type: Easing.InOutQuad
        }
        
        NumberAnimation {
            target: flickable
            property: "contentY"
            to: moveAnimation.toY
            duration: 300
            easing.type: Easing.InOutQuad
        }
        
        onStopped: {
            root.moveAnimationCompleted()
            // Reset isAnimating when the move animation completes
            if (root.controller) {
                root.controller.isAnimating = false
            }
        }
    }
}