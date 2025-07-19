import QtQuick 2.15

QtObject {
    // Map element types to their QML component paths
    readonly property var elementTypeToComponent: ({
        // Frame types
        "Frame": "FrameElement.qml",
        "FrameComponentVariant": "FrameElement.qml",
        "FrameComponentInstance": "FrameElement.qml",
        
        // Text types
        "Text": "TextElement.qml",
        "TextComponentVariant": "TextElement.qml",
        "TextComponentInstance": "TextElement.qml",
        
        // WebTextInput types
        "WebTextInput": "platform/web/WebTextInputElement.qml",
        "WebTextInputComponentVariant": "platform/web/WebTextInputElement.qml",
        "WebTextInputComponentInstance": "platform/web/WebTextInputElement.qml"
    })
    
    // Get the component path for an element
    function getComponentPath(element) {
        if (!element || !element.elementType) return ""
        
        // Special case: FrameComponentInstance might be a text-based instance
        if (element.elementType === "FrameComponentInstance" && element.hasOwnProperty('content')) {
            return "TextElement.qml"
        }
        
        return elementTypeToComponent[element.elementType] || ""
    }
    
    // Check if an element type is supported
    function isSupported(elementType) {
        return elementType in elementTypeToComponent
    }
}