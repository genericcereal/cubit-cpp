import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property Frame frameElement: element as Frame
    property alias contentContainer: contentContainer
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Frame visual representation
    Rectangle {
        id: frameRect
        anchors.fill: parent
        
        // Frame-specific properties - use the fill property
        color: frameElement ? frameElement.fill : Config.elementBackgroundColor
        border.width: 0
        radius: frameElement ? frameElement.borderRadius : 0
        antialiasing: true
        
        // Content container for child elements
        Item {
            id: contentContainer
            anchors.fill: parent
            anchors.margins: frameRect.border.width
            
            // Apply clipping based on overflow mode
            clip: {
                if (element && element.elementType === "ComponentInstance") {
                    console.log("ComponentInstance clipping check - frameElement:", frameElement, 
                               "overflow:", frameElement ? frameElement.overflow : "null",
                               "should clip:", frameElement ? frameElement.overflow !== 2 : false)
                }
                return frameElement ? frameElement.overflow !== 2 : false  // clip for Hidden (0) and Scroll (1) modes
            }
            
            // Render child elements
            Repeater {
                model: root.elementModel
                
                delegate: Loader {
                    property var childElement: model.element
                    property string childElementType: model.elementType
                    property string childParentId: childElement ? childElement.parentId : ""
                    
                    // Only render elements that are children of this frame
                    active: childParentId === root.element.elementId
                    
                    // Position child elements relative to parent
                    x: childElement && active ? childElement.x - root.element.x : 0
                    y: childElement && active ? childElement.y - root.element.y : 0
                    
                    Component.onCompleted: {
                        if (active && root.element.elementType === "ComponentInstance") {
                            console.log("ComponentInstance child - parentId:", childParentId, 
                                       "parent elementId:", root.element.elementId,
                                       "child type:", childElementType,
                                       "child showInList:", childElement.showInElementList)
                        }
                    }
                    
                    source: {
                        if (!active || !childElement || !childElementType) return ""
                        switch(childElementType) {
                            case "Frame": return "FrameElement.qml"
                            case "ComponentVariant": return "FrameElement.qml"
                            case "ComponentInstance": return "FrameElement.qml"
                            case "Text": return "TextElement.qml"
                            default: return ""
                        }
                    }
                    
                    onLoaded: {
                        if (item && childElement) {
                            item.element = childElement
                            item.elementModel = root.elementModel
                            if (item.hasOwnProperty("canvasMinX")) {
                                item.canvasMinX = root.canvasMinX
                                item.canvasMinY = root.canvasMinY
                            }
                        }
                    }
                }
            }
        }
    }
}