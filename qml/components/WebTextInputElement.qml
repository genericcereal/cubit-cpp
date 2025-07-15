import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 200
    height: element ? element.height : 40
    
    // Selection state
    property bool selected: element ? element.selected : false
    
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
            placeholderText: element && !element.isEditing ? "" : (element ? element.placeholder : "Enter text...")
            color: "#333333"
            font.pixelSize: 14
            font.family: "system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
            
            background: Rectangle {
                color: "transparent"
            }
            
            // Handle editing mode
            enabled: element ? element.isEditing : false
            
            // Update value on text change (when editing)
            onTextChanged: {
                if (element && element.isEditing && text !== element.value) {
                    element.value = text
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
            
            // Handle focus loss
            onActiveFocusChanged: {
                if (!activeFocus && element && element.isEditing) {
                    element.isEditing = false
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
        
        // Show placeholder when not editing and no value
        Text {
            anchors.fill: parent
            anchors.margins: 8
            visible: element && !element.isEditing && element.value === ""
            text: element ? element.placeholder : "Enter text..."
            color: "#999999"
            font: textField.font
            verticalAlignment: Text.AlignVCenter
        }
    }
    
    // Hover effect - removed for now since we don't have a MouseArea
    // The hover effect should be handled by the canvas hover system
}