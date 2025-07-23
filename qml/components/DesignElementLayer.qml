import QtQuick 2.15
import Cubit 1.0
import "."
import "platform/web"

Item {
    id: root
    
    // Element component mapper
    property ElementComponentMapper mapper: ElementComponentMapper {}
    
    required property var elementModel
    required property real canvasMinX
    required property real canvasMinY
    property var canvas: null  // Will be set by parent
    
    // Create filtered proxy model
    ElementFilterProxy {
        id: filteredModel
        sourceModel: root.elementModel || null
        viewMode: root.canvas ? root.canvas.viewMode : "design"
        editingElement: root.canvas ? root.canvas.editingElement : null
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
                if (!active || !element) return null
                var componentPath = root.mapper.getComponentPath(element)
                if (!componentPath) return null
                
                // Map paths to components
                switch(componentPath) {
                    case "FrameElement.qml": return frameComponent
                    case "TextElement.qml": return textComponent
                    case "platform/web/WebTextInputElement.qml": return webTextInputComponent
                    default: return null
                }
            }
            
            onLoaded: {
                if (item && element) {
                    item.element = element
                    item.elementModel = root.elementModel
                    item.canvasMinX = root.canvasMinX
                    item.canvasMinY = root.canvasMinY
                    if (item.hasOwnProperty("canvas")) {
                        item.canvas = root.canvas
                    }
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
    
    Component {
        id: webTextInputComponent
        WebTextInputElement {}
    }
}