import QtQuick
import QtQuick.Controls

// Use a container Item to work with native styling
Item {
    id: root
    
    property bool panelVisible: false
    property color elementFillColor: "#e0e0e0"
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    signal panelRequested()
    
    implicitHeight: textField.implicitHeight
    implicitWidth: textField.implicitWidth
    
    // Color preview square
    Rectangle {
        id: squareBox
        width: 16
        height: 16
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        color: root.elementFillColor
        border.color: "#b0b0b0"
        border.width: 1
        radius: 0
        z: 1 // Above the TextField
    }
    
    TextField {
        id: textField
        anchors.fill: parent
        readOnly: true
        placeholderText: "Click to select..."
        leftPadding: 30 // Make room for the square box
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.panelRequested()
                root.panelVisible = !root.panelVisible
            }
        }
    }
}