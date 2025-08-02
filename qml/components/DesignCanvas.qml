import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../CanvasUtils.js" as Utils
import "."
import "design-controls"

BaseCanvas {
    id: root
    
    canvasType: {
        if (!root.canvas) return "design"
        if (root.canvas.viewMode === "variant") return "variant"
        if (root.canvas.viewMode === "globalElements") return "variant" // Treat globalElements like variant for controls
        return "design"
    }
    
    // Property to access the viewport overlay's controls
    property var viewportControls: null
    
    // Property to access the shape controls controller
    property var shapeControlsController: null
    
    onShapeControlsControllerChanged: {
        if (root.controller && root.shapeControlsController) {
            root.controller.shapeControlsController = root.shapeControlsController
        }
    }
    
    onControllerChanged: {
        if (root.controller && root.shapeControlsController) {
            root.controller.shapeControlsController = root.shapeControlsController
        }
    }
    
    Component.onDestruction: {
        // Clear controller references to prevent access during destruction
        if (root.controller) {
            root.controller.shapeControlsController = null
        }
    }
    
    // Expose the element layer for viewport overlay
    property alias elementLayer: elementLayer
    
    // Signal emitted when move animation completes
    signal moveAnimationCompleted()
    
    // Additional properties for design canvas
    property var hoveredElement: controller && controller.hoveredElement ? controller.hoveredElement : null
    
    // Track element creation state
    property bool isCreatingElement: false
    property point creationStartPoint: Qt.point(0, 0)
    
    
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
    
    // Add elements into the default contentData
    contentData: [
        // Connection to clear shape editing mode when selection changes
        Connections {
            target: root.selectionManager
            enabled: root.selectionManager !== null
            
            function onSelectionChanged() {
                // Don't clear shape editing mode while dragging
                if (root.canvas && root.canvas.shapeControlsController && 
                    root.canvas.shapeControlsController.isShapeControlDragging) {
                    return
                }
                
                // Clear shape editing mode when:
                // 1. Selection is empty
                // 2. Selected element is not a shape
                if (root.canvas && root.canvas.shapeControlsController) {
                    if (root.selectionManager.selectedElements.length === 0) {
                        // No selection - clear shape editing mode
                        root.canvas.shapeControlsController.isEditingShape = false
                        root.canvas.shapeControlsController.isShapeControlDragging = false
                    } else if (root.selectionManager.selectedElements.length === 1) {
                        var element = root.selectionManager.selectedElements[0]
                        if (!element || element.elementType !== "Shape") {
                            // Single selection but not a shape - clear shape editing mode
                            root.canvas.shapeControlsController.isEditingShape = false
                            root.canvas.shapeControlsController.isShapeControlDragging = false
                        }
                    } else {
                        // Multiple selection - clear shape editing mode
                        root.canvas.shapeControlsController.isEditingShape = false
                        root.canvas.shapeControlsController.isShapeControlDragging = false
                    }
                }
            }
        },
        
        // Throttled update for element creation
        ThrottledUpdate {
            id: creationThrottle
            // interval uses default from ConfigObject.throttleInterval
            active: root.isCreatingElement
            
            onUpdate: (data) => {
                if (controller && controller.mode !== CanvasController.Select) {
                    controller.handleMouseMove(data.x, data.y)
                }
            }
        },
        
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
            root.isCreatingElement = true
            root.creationStartPoint = pt
        } else {
            // Skipping, in select mode or no controller
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleDragMove(pt) {
        if (controller.mode !== CanvasController.Select) {
            // In creation modes, use throttled update for better performance
            creationThrottle.requestUpdate({x: pt.x, y: pt.y})
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleDragEnd(pt) {
        if (controller.mode !== CanvasController.Select) {
            // Force final update to ensure accurate final position
            creationThrottle.forceUpdate()
            
            // In creation modes, finish creating element
            controller.handleMouseRelease(pt.x, pt.y)
            // Switch back to select mode after creation
            controller.mode = CanvasController.Select
            // Clear creation state
            root.isResizing = false
            root.isCreatingElement = false
        }
        // In select mode, we don't handle drag - elements can only be moved via controls
    }
    
    function handleClick(pt) {
        if (root.controller.mode === CanvasController.Select) {
            // Check if clicking on empty space (no element under cursor)
            var element = root.controller.hitTest(pt.x, pt.y)
            
            // Always clear shape editing states when clicking on canvas
            if (root.canvas && root.canvas.shapeControlsController) {
                // Clear selected joint index
                root.canvas.shapeControlsController.setSelectedJointIndex(-1)
                
                // Exit shape editing mode
                root.canvas.shapeControlsController.isEditingShape = false
                
                // Clear shape control dragging
                root.canvas.shapeControlsController.isShapeControlDragging = false
            }
            
            // If clicking on empty space, clear the selection after a small delay
            if (!element && root.selectionManager) {
                Qt.callLater(function() {
                    if (root.selectionManager) {
                        root.selectionManager.clearSelection()
                    }
                })
                return // Don't process further
            }
            
            if (!element) {
                // Clear selection when clicking on empty canvas
                if (root.selectionManager) {
                    root.selectionManager.clearSelection()
                }
            } else {
                // In select mode, handle click for selection
                root.controller.handleMousePress(pt.x, pt.y)
                root.controller.handleMouseRelease(pt.x, pt.y)
            }
        } else if (root.controller.mode === CanvasController.ShapePen) {
            // Pen creation mode - handle clicks to add joints
            root.controller.handleMousePress(pt.x, pt.y)
        }
        // Other creation modes don't create on click - require drag
    }
    
    // Watch for selection changes to exit shape editing mode
    Connections {
        target: root.selectionManager
        enabled: root.selectionManager !== null
        
        function onSelectionChanged() {
            // Exit shape editing mode when selection changes
            if (root.controller && root.controller.isEditingShape) {
                root.controller.isEditingShape = false
                if (root.controller.isShapeControlDragging) {
                    root.controller.isShapeControlDragging = false
                }
                // Clear selected joint index in shape controls controller
                // Add extra safety check for the method existence
                if (root.canvas && root.canvas.shapeControlsController && 
                    typeof root.canvas.shapeControlsController.setSelectedJointIndex === 'function') {
                    root.canvas.shapeControlsController.setSelectedJointIndex(-1)
                }
            }
        }
    }
    
    // Watch for mode changes to clear preview
    Connections {
        target: root.controller
        enabled: root.controller !== null
        function onModeChanged() {
            if (root.controller.mode !== CanvasController.ShapePen && root.shapeControlsController &&
                typeof root.shapeControlsController.showLinePreview !== 'undefined') {
                // Clear the preview when leaving pen mode
                root.shapeControlsController.showLinePreview = false
            }
        }
    }
    
    function handleHover(pt) {
        
        // Let the C++ controller handle hover tracking
        if (root.controller && root.controller.updateHover) {
            root.controller.updateHover(pt.x, pt.y)
        }
        
        
        // Update shape controls preview during pen creation
        if (root.controller && root.controller.mode === CanvasController.ShapePen && root.shapeControlsController) {
            if (typeof root.shapeControlsController.linePreviewPoint !== 'undefined') {
                root.shapeControlsController.linePreviewPoint = pt
            }
            if (typeof root.shapeControlsController.showLinePreview !== 'undefined') {
                root.shapeControlsController.showLinePreview = true
            }
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