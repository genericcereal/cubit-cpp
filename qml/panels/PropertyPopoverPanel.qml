import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    width: 250
    height: dragArea.height + (contentLoader.item ? contentLoader.item.implicitHeight : 0) + 20 // header + content + margins
    color: "#ffffff"
    border.color: "#d0d0d0"
    border.width: 1
    
    // Property to accept a source component
    property alias contentComponent: contentLoader.sourceComponent
    
    // Signal to notify when close is requested
    signal closeRequested()
    
    // Draggable header area
    MouseArea {
        id: dragArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        
        property point clickPos: "0,0"
        
        onPressed: (mouse) => {
            clickPos = Qt.point(mouse.x, mouse.y)
        }
        
        onPositionChanged: (mouse) => {
            if (pressed) {
                var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                root.x += delta.x
                root.y += delta.y
                
                // Mark as manually positioned when dragged
                if (root.parent && root.parent.manuallyPositioned !== undefined) {
                    root.parent.manuallyPositioned = true
                }
            }
        }
        
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
    }
    
    // Close button in top right corner
    Rectangle {
        id: closeButton
        width: 24
        height: 24
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        color: closeButtonArea.containsMouse ? "#e0e0e0" : "transparent"
        radius: 2
        
        Text {
            anchors.centerIn: parent
            text: "✕"
            font.pixelSize: 16
            font.weight: Font.Medium
            color: closeButtonArea.containsMouse ? "#333333" : "#666666"
        }
        
        MouseArea {
            id: closeButtonArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.closeRequested()
            }
        }
    }
    
    // Content loader for dynamic content
    Loader {
        id: contentLoader
        anchors.top: dragArea.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        
        // Ensure loaded content has proper implicit height
        onLoaded: {
            if (item && !item.implicitHeight) {
                console.warn("PropertyPopoverPanel: Loaded content has no implicitHeight")
            }
        }
    }
}