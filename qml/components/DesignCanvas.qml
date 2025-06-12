import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "design"
    
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
    
    // Override content layer with design elements
    Component.onCompleted: {
        // Call base implementation
        centerViewAtOrigin()
        
        // Add design-specific initialization
        var contentLayer = getContentLayer()
        elementsLayer.parent = contentLayer
        elementsLayer.anchors.fill = contentLayer
    }
    
    // Elements layer
    Item {
        id: elementsLayer
        // Don't set anchors here - will be set when parent is assigned
        
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
                    switch(elementType) {
                        case "Frame": return frameComponent
                        case "Text": return textComponent
                        default: return null
                    }
                }
                
                onLoaded: {
                    if (item) {
                        item.element = element
                        item.elementModel = root.elementModel
                    }
                }
            }
        }
    }
    
    // Creation preview layer (removed - no longer needed)
    
    // Creation drag handler
    QtObject {
        id: creationDragHandler
        property bool active: false
        property point startPoint: Qt.point(0, 0)
        property point currentPoint: Qt.point(0, 0)
        
        function startCreation(point) {
            active = true
            startPoint = point
            currentPoint = point
        }
        
        function updateCreation(point) {
            currentPoint = point
        }
        
        function endCreation() {
            // The C++ controller already handles element creation
            // We just need to reset our state
            active = false
        }
    }
    
    // Override virtual functions for design canvas behavior
    function handleLeftButtonPress(canvasPoint) {
        if (controller.mode === "select") {
            // Always pass the press to the controller
            // It will handle both element clicks and empty space clicks
            controller.handleMousePress(canvasPoint.x, canvasPoint.y)
        } else {
            // Start element creation - let C++ controller handle it
            creationDragHandler.startCreation(canvasPoint)
            controller.handleMousePress(canvasPoint.x, canvasPoint.y)
        }
    }
    
    function handleMouseDrag(canvasPoint) {
        if (creationDragHandler.active) {
            creationDragHandler.updateCreation(canvasPoint)
        }
        // Always pass mouse move to controller
        controller.handleMouseMove(canvasPoint.x, canvasPoint.y)
    }
    
    function handleMouseHover(canvasPoint) {
        // Track hover for design elements
        if (controller.mode === "select") {
            root.hoveredElement = controller.hitTest(canvasPoint.x, canvasPoint.y)
        } else {
            root.hoveredElement = null
        }
    }
    
    function handleLeftButtonRelease(canvasPoint) {
        if (creationDragHandler.active) {
            creationDragHandler.endCreation()
        }
        // Always call controller.handleMouseRelease to finalize creation
        controller.handleMouseRelease(canvasPoint.x, canvasPoint.y)
    }
    
    function handleMouseExit() {
        root.hoveredElement = null
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