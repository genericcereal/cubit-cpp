import QtQuick
import Cubit 1.0
import "."

Item {
    id: root
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    
    // Check if we're in variant mode
    property bool isVariantMode: Application.activeCanvas && Application.activeCanvas.viewMode === "variant"
    
    // Get the editing component if in variant mode
    property var editingComponent: {
        if (isVariantMode && Application.activeCanvas && Application.activeCanvas.editingElement) {
            var element = Application.activeCanvas.editingElement
            // Check if it's a Component type
            if (element && element.variants !== undefined) {
                return element
            }
        }
        return null
    }
    
    // Helper function to check if an element is in the component's variants array
    function isInComponentVariants(element) {
        if (!editingComponent || !element) return false
        
        var variants = editingComponent.variants
        for (var i = 0; i < variants.length; i++) {
            if (variants[i] === element) {
                return true
            }
        }
        return false
    }
    
    // Helper function to check if an element is in ANY component's variants array
    function isInAnyComponentVariants(element) {
        if (!element || !elementModel) return false
        
        // Iterate through all elements in the model
        var allElements = elementModel.getAllElements()
        for (var i = 0; i < allElements.length; i++) {
            var el = allElements[i]
            // Check if this element is a Component (has variants property)
            if (el && el.variants !== undefined) {
                // Check if our element is in this component's variants
                var variants = el.variants
                for (var j = 0; j < variants.length; j++) {
                    if (variants[j] === element) {
                        return true
                    }
                }
            }
        }
        return false
    }
    
    // Element rendering - only render root elements (no parent)
    Repeater {
        id: elementRepeater
        model: root.elementModel
        
        delegate: Loader {
            property var element: model.element
            property string elementType: model.elementType
            property string elementParentId: element ? element.parentId : ""
            
            // Determine if element should be rendered based on variant mode
            active: {
                if (elementParentId !== "") return false  // Only render root elements
                
                if (root.isVariantMode) {
                    // In variant mode, only show elements that are in the component's variants array
                    return root.isInComponentVariants(element)
                } else {
                    // In design mode, exclude all elements that are in any component's variants array
                    return !root.isInAnyComponentVariants(element)
                }
            }
            
            // Position elements relative to canvas origin
            x: element && active ? element.x - root.canvasMinX : 0
            y: element && active ? element.y - root.canvasMinY : 0
            
            sourceComponent: {
                if (!active || !element || !elementType) return null
                switch(elementType) {
                    case "Frame": return frameComponent
                    case "ComponentVariant": return frameComponent
                    case "ComponentInstance": return frameComponent
                    case "Text": return textComponent
                    default: return null
                }
            }
            
            onLoaded: {
                if (item && element) {
                    item.element = element
                    item.elementModel = root.elementModel
                    item.canvasMinX = root.canvasMinX
                    item.canvasMinY = root.canvasMinY
                }
            }
        }
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