import QtQuick
import QtQuick.Controls

// A generic object input component that displays a preview and text representation
// Based on the Frame's Fill input implementation
Item {
    id: root
    
    // Properties
    property var value: null // The object value (e.g., a color, gradient, etc.)
    property string displayText: "" // Text to display in the field
    property string placeholderText: "Click to select..."
    property int previewType: PreviewType.Color // Type of preview to show
    property alias readOnly: textField.readOnly
    
    // Helper property to check if value is effectively set
    property bool hasValue: {
        if (value === null || value === undefined) return false
        // Always show clear button if there's a value
        return true
    }
    
    // Signals
    signal clicked()
    signal cleared()
    
    // Enums
    enum PreviewType {
        Color,
        None,
        Custom
    }
    
    // Preview component property for custom preview types
    property Component customPreview: null
    
    implicitHeight: textField.implicitHeight
    implicitWidth: textField.implicitWidth
    
    // Preview area
    Loader {
        id: previewLoader
        width: 16
        height: 16
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        z: 1 // Above the TextField
        
        sourceComponent: {
            if (root.previewType === ObjectInput.PreviewType.Color) {
                return colorPreviewComponent
            } else if (root.previewType === ObjectInput.PreviewType.Custom && root.customPreview) {
                return root.customPreview
            }
            return null
        }
    }
    
    // Default color preview component
    Component {
        id: colorPreviewComponent
        
        Item {
            width: 16
            height: 16
            
            // Checkerboard background for transparent colors
            Rectangle {
                anchors.fill: parent
                color: "#ffffff"
                visible: {
                    if (!root.value || root.value.a === undefined) return false
                    return root.value.a < 1
                }
            }
            
            Rectangle {
                width: 8
                height: 8
                color: "#cccccc"
                visible: {
                    if (!root.value || root.value.a === undefined) return false
                    return root.value.a < 1
                }
            }
            
            Rectangle {
                x: 8
                y: 8
                width: 8
                height: 8
                color: "#cccccc"
                visible: {
                    if (!root.value || root.value.a === undefined) return false
                    return root.value.a < 1
                }
            }
            
            // Color overlay
            Rectangle {
                anchors.fill: parent
                color: root.value || "#e0e0e0"
                border.color: "#b0b0b0"
                border.width: 1
            }
        }
    }
    
    TextField {
        id: textField
        anchors.fill: parent
        text: root.displayText
        readOnly: true
        placeholderText: root.placeholderText
        leftPadding: root.previewType !== ObjectInput.PreviewType.None ? 30 : 8 // Make room for preview if needed
        rightPadding: root.hasValue ? 30 : 8 // Make room for clear button if value exists
        
        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: root.hasValue ? 24 : 0 // Don't capture clicks on clear button
            cursorShape: root.readOnly ? Qt.PointingHandCursor : Qt.IBeamCursor
            enabled: root.readOnly
            onClicked: root.clicked()
        }
        
        // Clear button
        Rectangle {
            id: clearButton
            visible: root.hasValue
            width: 16
            height: 16
            anchors.right: parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            color: clearButtonArea.containsMouse ? "#e0e0e0" : "transparent"
            radius: 8
            
            Text {
                anchors.centerIn: parent
                text: "✕"
                font.pixelSize: 12
                font.weight: Font.Medium
                color: clearButtonArea.containsMouse ? "#333333" : "#666666"
            }
            
            MouseArea {
                id: clearButtonArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.cleared()
                }
            }
        }
    }
    
    // Helper function to format color for display
    function formatColor(color, format) {
        if (!color) return ""
        
        format = format !== undefined ? format : 1 // Default to hex
        
        switch (format) {
            case 0: // RGB
                var r = Math.round(color.r * 255)
                var g = Math.round(color.g * 255)
                var b = Math.round(color.b * 255)
                return "rgb(" + r + ", " + g + ", " + b + ")"
                
            case 1: // Hex
                var rh = Math.round(color.r * 255).toString(16).padStart(2, '0')
                var gh = Math.round(color.g * 255).toString(16).padStart(2, '0')
                var bh = Math.round(color.b * 255).toString(16).padStart(2, '0')
                return "#" + rh + gh + bh
                
            case 2: // HSL
                var h = Math.round(color.hslHue * 360)
                var s = Math.round(color.hslSaturation * 100)
                var l = Math.round(color.hslLightness * 100)
                return "hsl(" + h + ", " + s + "%, " + l + "%)"
                
            default:
                return color.toString()
        }
    }
}