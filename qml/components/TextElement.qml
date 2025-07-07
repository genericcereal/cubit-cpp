import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property TextElement textElement: element as TextElement
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Background rectangle - light blue like frames
    Rectangle {
        anchors.fill: parent
        color: Config.elementBackgroundColor
        z: 0
    }
    
    // Text visual representation
    Text {
        id: textItem
        anchors.fill: parent
        anchors.leftMargin: 4  // Match TextField's internal padding
        anchors.rightMargin: 4
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        visible: textElement ? !textElement.isEditing : true
        
        // Text-specific properties
        text: textElement ? textElement.content : ""
        color: textElement ? textElement.color : "black"
        font: textElement ? textElement.font : Qt.font({family: "Arial", pixelSize: 14})
        
        // Text layout
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        lineHeight: 1.2  // Match TextField's default line height
        
        // Ensure text is visible
        clip: true
    }
    
    // Text editing input
    TextField {
        id: textInput
        anchors.fill: parent
        visible: textElement ? textElement.isEditing : false
        
        text: textElement ? textElement.content : ""
        color: textElement ? textElement.color : "black"
        font: textElement ? textElement.font : Qt.font({family: "Arial", pixelSize: 14})
        leftPadding: 4
        rightPadding: 4
        topPadding: 4
        bottomPadding: 4
        
        background: Rectangle {
            color: "transparent"
            border.width: 2
            border.color: Config.controlBarColor
        }
        
        // Start with focus when editing starts
        onVisibleChanged: {
            if (visible) {
                forceActiveFocus()
                selectAll()
            }
        }
        
        // Save content when editing mode is exited externally
        Connections {
            target: textElement
            function onIsEditingChanged() {
                if (!textElement.isEditing && textInput.text !== textElement.content) {
                    textElement.content = textInput.text
                }
            }
        }
        
        // Handle save
        onAccepted: {
            if (textElement) {
                textElement.content = text
                textElement.isEditing = false
            }
        }
        
        // Handle cancel and save on focus loss
        Keys.onEscapePressed: {
            if (textElement) {
                text = textElement.content  // Restore original text
                textElement.isEditing = false
            }
        }
        
        onActiveFocusChanged: {
            if (!activeFocus && textElement && textElement.isEditing) {
                // Save on focus loss
                textElement.content = text
                textElement.isEditing = false
            }
        }
    }
}