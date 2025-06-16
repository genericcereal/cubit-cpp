import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "design"
    
    // Trigger onEditorLoad event when the canvas loads
    Component.onCompleted: {
        // Execute onEditorLoad for canvas scripts
        if (Application.activeCanvas) {
            Application.activeCanvas.executeScriptEvent("onEditorLoad")
        }
        
        // Also execute onEditorLoad for all design elements
        if (elementModel) {
            var elements = elementModel.getAllElements()
            for (var i = 0; i < elements.length; i++) {
                var element = elements[i]
                if (element && element.executeScriptEvent) {
                    element.executeScriptEvent("onEditorLoad")
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
        Repeater {
            id: elementRepeater
            model: root.elementModel
            
            delegate: Loader {
                property var element: model.element
                property string elementType: model.elementType
                
                // Position elements relative to canvas origin
                x: element ? element.x - root.canvasMinX : 0
                y: element ? element.y - root.canvasMinY : 0
                
                sourceComponent: {
                    if (!element || !elementType) return null
                    switch(elementType) {
                        case "Frame": return frameComponent
                        case "Text": return textComponent
                        default: return null
                    }
                }
                
                onLoaded: {
                    if (item && element) {
                        item.element = element
                        item.elementModel = root.elementModel
                    }
                }
            }
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
        if (controller.mode === "select") {
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
        if (controller.mode === "select") {
            controller.handleMousePress(pt.x, pt.y)
            controller.handleMouseRelease(pt.x, pt.y)
        }
    }
    
    function handleHover(pt) {
        // Track hover for design elements
        if (controller.mode === "select") {
            hoveredElement = controller.hitTest(pt.x, pt.y)
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
    
    // Component definitions
    Component {
        id: frameComponent
        FrameElement {}
    }
    
    Component {
        id: textComponent
        TextElement {}
    }
}