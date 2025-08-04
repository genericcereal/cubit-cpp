import QtQuick 2.15

QtObject {
    // Map element types to their QML component paths
    readonly property var elementTypeToComponent: ({
        // Frame types
        "Frame": "FrameElement.qml",
        
        // Text types
        "Text": "TextElement.qml",
        
        // WebTextInput types
        "WebTextInput": "platform/web/WebTextInputElement.qml",
        
        // Shape types
        "Shape": "ShapeElement.qml"
    })
    
    // Get the component path for an element
    function getComponentPath(element) {
        if (!element || !element.elementType) return ""
        
        // Special case: Frame instance might be a text-based instance
        if (element.elementType === "Frame" && element.isInstance && element.isInstance() && element.hasOwnProperty('content')) {
            return "TextElement.qml"
        }
        
        return elementTypeToComponent[element.elementType] || ""
    }
    
    // Check if an element type is supported
    function isSupported(elementType) {
        return elementType in elementTypeToComponent
    }
}