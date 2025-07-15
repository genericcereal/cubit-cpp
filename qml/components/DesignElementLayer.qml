import QtQuick 2.15
import Cubit 1.0
import "."

Item {
    id: root
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    
    // Create filtered proxy model
    ElementFilterProxy {
        id: filteredModel
        sourceModel: root.elementModel
        viewMode: Application.activeCanvas ? Application.activeCanvas.viewMode : "design"
        editingElement: Application.activeCanvas ? Application.activeCanvas.editingElement : null
    }
    
    // Element rendering - only render root elements (no parent)
    Repeater {
        id: elementRepeater
        model: filteredModel
        
        delegate: Loader {
            property var element: model.element
            property string elementType: model.elementType
            property string elementParentId: element ? element.parentId : ""
            
            // Only render root elements - children are handled by their parents
            active: elementParentId === ""
            
            // Position elements relative to canvas origin
            x: element && active ? element.x - root.canvasMinX : 0
            y: element && active ? element.y - root.canvasMinY : 0
            
            sourceComponent: {
                if (!active || !element || !elementType) return null
                switch(elementType) {
                    case "Frame": return frameComponent
                    case "FrameComponentVariant": return frameComponent
                    case "FrameComponentInstance": 
                        // Check if it's a text-based instance by checking for content property
                        return element.hasOwnProperty('content') ? textComponent : frameComponent
                    case "Text": return textComponent
                    case "TextComponentVariant": return textComponent
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