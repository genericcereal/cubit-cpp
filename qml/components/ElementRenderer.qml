import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    property var elementModel
    property int parentId: -1  // -1 means render root elements
    
    Repeater {
        model: elementModel
        
        Loader {
            id: elementLoader
            
            property var element: model.element
            property int elementId: model.elementId
            property string elementType: model.elementType
            property int elementParentId: element ? element.parentId : -1
            
            // Only load if this element's parent matches our parentId
            active: elementParentId === root.parentId
            
            source: {
                if (!active) return ""
                switch(elementType) {
                    case "Frame": return "FrameElement.qml"
                    case "ComponentInstance": return "FrameElement.qml"
                    case "Text": return "TextElement.qml"  
                    case "Html": return "HtmlElement.qml"
                    default: return ""
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