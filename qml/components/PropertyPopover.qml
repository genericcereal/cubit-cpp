import QtQuick
import QtQuick.Controls
import Cubit

// Use a container Item to work with native styling
Item {
    id: root
    
    property bool panelVisible: false
    property color elementFillColor: "#e0e0e0"
    property string text: ""
    property string placeholderText: "Click to select..."
    signal panelRequested()
    
    implicitHeight: 32
    implicitWidth: 200
    
    // Color preview square
    Rectangle {
        id: squareBox
        width: 16
        height: 16
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        color: root.elementFillColor
        border.color: ConfigObject.darkMode ? "#505050" : "#b0b0b0"
        border.width: 1
        radius: 0
        z: 1 // Above the TextField
    }
    
    // Background rectangle that looks like a TextField
    Rectangle {
        id: textFieldBackground
        anchors.fill: parent
        color: ConfigObject.darkMode ? "#2a2a2a" : "#ffffff"
        border.color: ConfigObject.darkMode ? "#404040" : "#d0d0d0"
        border.width: 1
        radius: 4
        
        // Text display
        Text {
            id: textDisplay
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 30 // Make room for the square box
            anchors.rightMargin: 8
            text: root.text || root.placeholderText
            color: root.text ? ConfigObject.textColor : ConfigObject.secondaryTextColor
            elide: Text.ElideRight
            font.pixelSize: 14
        }
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            
            onEntered: {
                textFieldBackground.border.color = ConfigObject.darkMode ? "#505050" : "#a0a0a0"
            }
            
            onExited: {
                textFieldBackground.border.color = ConfigObject.darkMode ? "#404040" : "#d0d0d0"
            }
            
            onClicked: {
                root.panelRequested()
                root.panelVisible = !root.panelVisible
            }
        }
    }
}