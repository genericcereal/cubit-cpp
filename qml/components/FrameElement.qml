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
    
    // Check if we need rounded corner clipping
    property bool needsClipping: frameElement && frameElement.borderRadius > 0 && frameElement.overflow !== 2
    
    // Frame visual representation (background)
    Rectangle {
        id: frameRect
        anchors.fill: parent
        
        // Frame-specific properties - use the fill property
        color: frameElement ? frameElement.fill : Config.elementBackgroundColor
        border.width: 0
        radius: frameElement ? frameElement.borderRadius : 0
        antialiasing: true
    }
    
    // Content that needs to be masked
    Item {
        id: contentWrapper
        anchors.fill: parent
        visible: !needsClipping
        
        // Content container for child elements
        Item {
            id: contentContainer
            anchors.fill: parent
            anchors.margins: frameRect.border.width
            
            // Apply clipping based on overflow mode
            clip: frameElement ? frameElement.overflow !== 2 : false  // clip for Hidden (0) and Scroll (1) modes
            
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
    
    // Mask shape for rounded corners
    Rectangle {
        id: mask
        anchors.fill: parent
        radius: frameElement ? frameElement.borderRadius : 0
        visible: false
        antialiasing: true
        smooth: true
        color: "white"
    }
    
    // Shader effect sources
    ShaderEffectSource {
        id: contentSource
        sourceItem: contentWrapper
        hideSource: needsClipping
        visible: false
        live: true
        recursive: true
    }
    
    ShaderEffectSource {
        id: maskSource
        sourceItem: mask
        hideSource: true
        visible: false
    }
    
    // Apply the mask using shader effect
    ShaderEffect {
        anchors.fill: parent
        visible: needsClipping
        
        property variant source: contentSource
        property variant maskSource: maskSource
        
        fragmentShader: "qrc:/shaders/roundedmask.frag.qsb"
    }
}