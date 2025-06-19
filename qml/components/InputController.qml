import QtQuick
import QtQuick.Controls
import Cubit 1.0

MouseArea {
    id: root
    
    property var controller
    property var selectionManager
    property var flickable
    property var selectionBoxHandler
    property var creationDragHandler
    property ViewportCache viewportCache
    
    // Expose hovered element
    property var hoveredElement: null
    
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    hoverEnabled: true
    
    // Throttling properties
    property real throttleDelay: 16 // 60fps
    property point pendingMousePosition
    property bool hasPendingMouseMove: false
    
    // Throttle timer for mouse movement
    Timer {
        id: mouseMoveTimer
        interval: root.throttleDelay
        repeat: false
        onTriggered: {
            if (hasPendingMouseMove) {
                processMouseMove(pendingMousePosition.x, pendingMousePosition.y)
                hasPendingMouseMove = false
            }
        }
    }
    
    onPressed: (mouse) => {
        if (!viewportCache) return
        
        // Get canvas coordinates using cached conversion
        var canvasPoint = viewportCache.viewportToCanvas(mouse.x, mouse.y)
        
        if (mouse.button === Qt.LeftButton) {
            if (controller.mode === CanvasController.Select) {
                // Check if we hit an element
                var element = controller.hitTest(canvasPoint.x, canvasPoint.y)
                if (element) {
                    // Start dragging element
                    controller.handleMousePress(canvasPoint.x, canvasPoint.y)
                } else {
                    // Start selection box
                    selectionBoxHandler.startSelection(canvasPoint)
                }
            } else {
                // Start element creation
                creationDragHandler.startCreation(canvasPoint)
                controller.handleMousePress(canvasPoint.x, canvasPoint.y)
            }
        }
    }
    
    onPositionChanged: (mouse) => {
        // Throttle mouse move events
        pendingMousePosition = Qt.point(mouse.x, mouse.y)
        hasPendingMouseMove = true
        if (!mouseMoveTimer.running) {
            mouseMoveTimer.start()
        }
    }
    
    function processMouseMove(mouseX, mouseY) {
        if (!viewportCache) return
        
        // Get canvas coordinates using cached conversion
        var canvasPoint = viewportCache.viewportToCanvas(mouseX, mouseY)
        
        // Track hover even when dragging controls (use hitTestForHover to exclude selected elements)
        if (controller.mode === CanvasController.Select) {
            hoveredElement = controller.hitTestForHover(canvasPoint.x, canvasPoint.y)
        }
        
        if (selectionBoxHandler.active) {
            selectionBoxHandler.updateSelection(canvasPoint)
        } else if (creationDragHandler.active) {
            creationDragHandler.updateCreation(canvasPoint)
        } else if (pressed) {
            // Dragging elements or updating creation
            controller.handleMouseMove(canvasPoint.x, canvasPoint.y)
        }
    }
    
    onReleased: (mouse) => {
        if (selectionBoxHandler.active) {
            selectionBoxHandler.endSelection()
        } else if (creationDragHandler.active) {
            creationDragHandler.endCreation()
        } else if (mouse.button === Qt.LeftButton && viewportCache) {
            // Get final canvas position
            var canvasPoint = viewportCache.viewportToCanvas(mouse.x, mouse.y)
            controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
        }
    }
    
    onExited: {
        hoveredElement = null
    }
    
    // Keyboard shortcuts
    Shortcut {
        sequence: StandardKey.Undo
        enabled: controller && controller.canUndo
        onActivated: controller.undo()
    }
    
    Shortcut {
        sequence: StandardKey.Redo
        enabled: controller && controller.canRedo
        onActivated: controller.redo()
    }
}