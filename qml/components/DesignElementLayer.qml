import QtQuick
import Cubit 1.0
import "."

Item {
    id: root
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    
    // Element rendering - only render root elements (no parent)
    Repeater {
        id: elementRepeater
        model: root.elementModel
        
        delegate: Loader {
            property var element: model.element
            property string elementType: model.elementType
            property string elementParentId: element ? element.parentId : ""
            
            // Only render elements without a parent (root elements)
            active: elementParentId === ""
            
            // Position elements relative to canvas origin
            x: element && active ? element.x - root.canvasMinX : 0
            y: element && active ? element.y - root.canvasMinY : 0
            
            sourceComponent: {
                if (!active || !element || !elementType) return null
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