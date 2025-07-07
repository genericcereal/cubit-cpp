import QtQuick
import QtQuick.Controls

TextField {
    id: root
    
    property bool panelVisible: false
    property color elementFillColor: "#e0e0e0"
    signal panelRequested()
    
    readOnly: true
    placeholderText: "Click to select..."
    leftPadding: 30 // Make room for the square box (16px box + 4px margin + 10px padding)
    
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.panelRequested()
            root.panelVisible = !root.panelVisible
        }
    }
    
    // Style to match other inputs in PropertiesPanel
    background: Rectangle {
        color: root.enabled ? "#ffffff" : "#f0f0f0"
        border.color: root.panelVisible ? "#4080ff" : (root.hovered ? "#a0a0a0" : "#d0d0d0")
        border.width: 1
        radius: 2
        
        // Square box on the left side
        Rectangle {
            id: squareBox
            width: 16
            height: 16
            anchors.left: parent.left
            anchors.leftMargin: 4  // Small spacing from left edge
            anchors.verticalCenter: parent.verticalCenter
            color: root.elementFillColor
            border.color: "#b0b0b0"
            border.width: 1
            radius: 0
        }
    }
}