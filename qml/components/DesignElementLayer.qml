import QtQuick
import Cubit 1.0
import "."

Item {
    id: root
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    
    // Element rendering
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