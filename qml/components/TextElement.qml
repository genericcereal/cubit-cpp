import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property var textElement: {
        // Handle Text, TextComponentVariant, and FrameComponentInstance that inherits from Text
        if (element) {
            // Check if it's a text-based element by checking if it has content property
            if (element.hasOwnProperty('content')) {
                return element
            }
        }
        return null
    }
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Font loader for Google Fonts
    GoogleFontLoader {
        id: fontLoader
        fontFamily: textElement ? textElement.font.family : ""
        fontWeight: {
            if (!textElement || !textElement.font) return "regular"
            
            // Map Qt font weights to Google font weights
            switch (textElement.font.weight) {
                case Font.Thin: return "100"
                case Font.ExtraLight: return "200"
                case Font.Light: return "300"
                case Font.Normal: return "regular"
                case Font.Medium: return "500"
                case Font.DemiBold: return "600"
                case Font.Bold: return "700"
                case Font.ExtraBold: return "800"
                case Font.Black: return "900"
                default: return "regular"
            }
        }
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
        
        // Apply shadow effect if enabled
        layer.enabled: element && element.boxShadow && element.boxShadow.enabled
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: element && element.boxShadow ? element.boxShadow.color : "#444444"
            shadowHorizontalOffset: element && element.boxShadow ? element.boxShadow.offsetX : 0
            shadowVerticalOffset: element && element.boxShadow ? element.boxShadow.offsetY : 0
            shadowBlur: element && element.boxShadow ? element.boxShadow.blurRadius / 10.0 : 0
            shadowScale: element && element.boxShadow ? 1.0 + (element.boxShadow.spreadRadius / 100.0) : 1.0
        }
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
        
        // Apply shadow effect if enabled (same as textItem)
        layer.enabled: element && element.boxShadow && element.boxShadow.enabled
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: element && element.boxShadow ? element.boxShadow.color : "#444444"
            shadowHorizontalOffset: element && element.boxShadow ? element.boxShadow.offsetX : 0
            shadowVerticalOffset: element && element.boxShadow ? element.boxShadow.offsetY : 0
            shadowBlur: element && element.boxShadow ? element.boxShadow.blurRadius / 10.0 : 0
            shadowScale: element && element.boxShadow ? 1.0 + (element.boxShadow.spreadRadius / 100.0) : 1.0
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