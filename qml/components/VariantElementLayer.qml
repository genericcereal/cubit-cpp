import QtQuick
import Cubit 1.0
import "."

Item {
    id: root
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    
    Component.onCompleted: {
        console.log("VariantElementLayer loaded")
        console.log("ElementModel:", elementModel)
        console.log("ElementModel row count:", elementModel ? elementModel.rowCount() : 0)
        
        // Debug: List all elements to see what's in the model
        if (elementModel) {
            for (var i = 0; i < elementModel.rowCount(); i++) {
                var idx = elementModel.index(i, 0)
                var element = elementModel.data(idx, 256) // ElementRole
                var elementType = elementModel.data(idx, 257) // ElementTypeRole
                if (element) {
                    console.log("Element", i, ":", element.elementId, "type:", elementType, "parentId:", element.parentId)
                }
            }
        }
    }
    
    // Element rendering - show ALL ComponentVariants for now
    Repeater {
        id: elementRepeater
        model: root.elementModel
        
        delegate: Loader {
            property var element: model.element
            property string elementType: model.elementType
            
            
            // Only render ComponentVariants as root elements for now
            active: elementType === "ComponentVariant" && element.parentId === ""
            
            // Position elements relative to canvas origin
            x: element && active ? element.x - root.canvasMinX : 0
            y: element && active ? element.y - root.canvasMinY : 0
            
            sourceComponent: {
                if (!active || !element || !elementType) return null
                
                // ComponentVariant inherits from Frame, so use frameComponent
                if (elementType === "ComponentVariant") {
                    return frameComponent
                }
                
                return null
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