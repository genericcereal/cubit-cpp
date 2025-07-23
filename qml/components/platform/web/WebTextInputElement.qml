import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0

FocusScope {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property var canvas: null  // Will be set by parent
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 200
    height: element ? element.height : 40
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Track if we're in prototype mode
    property bool isPrototyping: root.canvas && root.canvas.prototypeController ? root.canvas.prototypeController.isPrototyping : false
    
    // Force focus to be cleared when isEditing becomes false
    Connections {
        target: element
        function onIsEditingChanged() {
            if (element && !element.isEditing && textField.activeFocus) {
                textField.focus = false
                root.focus = false
            }
        }
    }
    
    // Web-style text input visual representation
    Rectangle {
        id: inputBackground
        anchors.fill: parent
        color: "white"
        border.color: element ? element.borderColor : "#aaaaaa"
        border.width: element ? element.borderWidth : 1
        radius: element ? element.borderRadius : 4
        
        // Inner text field
        TextField {
            id: textField
            anchors.fill: parent
            anchors.margins: 8
            
            text: element ? element.value : ""
            placeholderText: element ? element.placeholder : "Enter text..."
            color: "#333333"
            font.pixelSize: 14
            font.family: "system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
            
            background: Rectangle {
                color: "transparent"
            }
            
            // Handle editing mode - enabled when editing OR in prototype mode
            enabled: element ? (element.isEditing || root.isPrototyping) : false
            
            // Update value on text change (when editing or prototyping)
            onTextChanged: {
                if (element && text !== element.value) {
                    if (element.isEditing || root.isPrototyping) {
                        element.value = text
                        
                        // Fire onChange event in prototype mode
                        if (root.isPrototyping) {
                            element.executeScriptEvent("On Change")
                        }
                    }
                }
            }
            
            // Handle enter key
            onAccepted: {
                if (element) {
                    element.isEditing = false
                }
            }
            
            // Handle escape key
            Keys.onEscapePressed: {
                if (element) {
                    text = element.value  // Restore original value
                    element.isEditing = false
                }
            }
            
            // Handle focus changes
            onActiveFocusChanged: {
                if (element) {
                    if (activeFocus) {
                        // Fire onFocus event in prototype mode
                        if (root.isPrototyping) {
                            element.executeScriptEvent("On Focus")
                        }
                    } else {
                        // Handle focus loss
                        if (element.isEditing) {
                            element.isEditing = false
                        }
                        
                        // Fire onBlur event in prototype mode
                        if (root.isPrototyping) {
                            element.executeScriptEvent("On Blur")
                        }
                    }
                }
            }
            
            // Start with focus when editing starts
            Connections {
                target: element
                function onIsEditingChanged() {
                    if (element.isEditing) {
                        textField.forceActiveFocus()
                        textField.selectAll()
                    }
                }
            }
        }
    }
    
    // Hover effect - removed for now since we don't have a MouseArea
    // The hover effect should be handled by the canvas hover system
}