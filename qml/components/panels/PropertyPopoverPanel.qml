import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

Rectangle {
    id: root
    
    width: 250
    height: dragArea.height + (contentLoader.item ? contentLoader.item.implicitHeight : 0) + 20 // header + content + margins
    color: ConfigObject.darkMode ? "#2a2a2a" : "#ffffff"
    border.color: ConfigObject.darkMode ? "#404040" : "#d0d0d0"
    border.width: 1
    
    // Property to accept a source component
    property alias contentComponent: contentLoader.sourceComponent
    
    // Property for the title text
    property string title: ""
    
    // Property to control back button visibility
    property bool showBackButton: false
    
    // Signal to notify when close is requested
    signal closeRequested()
    
    // Signal to notify when back button is clicked
    signal backRequested()
    
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
                if (root.manuallyPositioned !== undefined) {
                    root.manuallyPositioned = true
                }
            }
        }
        
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
    }
    
    // Header content row
    Row {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: dragArea.verticalCenter
        spacing: 8
        
        // Back button
        Rectangle {
            id: backButton
            width: 24
            height: 24
            color: backButtonArea.containsMouse ? (ConfigObject.darkMode ? "#3e3e42" : "#e0e0e0") : "transparent"
            radius: 2
            visible: root.showBackButton
            
            Text {
                anchors.centerIn: parent
                text: "←"
                font.pixelSize: 16
                font.weight: Font.Medium
                color: backButtonArea.containsMouse ? ConfigObject.textColor : ConfigObject.secondaryTextColor
            }
            
            MouseArea {
                id: backButtonArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.backRequested()
                }
            }
        }
        
        // Title text
        Text {
            id: titleText
            text: root.title
            font.pixelSize: 14
            font.weight: Font.Medium
            color: ConfigObject.textColor
            visible: root.title !== ""
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    // Close button in top right corner
    Rectangle {
        id: closeButton
        width: 24
        height: 24
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        color: closeButtonArea.containsMouse ? (ConfigObject.darkMode ? "#3e3e42" : "#e0e0e0") : "transparent"
        radius: 2
        
        Text {
            anchors.centerIn: parent
            text: "✕"
            font.pixelSize: 16
            font.weight: Font.Medium
            color: closeButtonArea.containsMouse ? ConfigObject.textColor : ConfigObject.secondaryTextColor
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