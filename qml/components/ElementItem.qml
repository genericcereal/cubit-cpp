import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel  // The model containing all elements
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Selection visual feedback
    Rectangle {
        id: selectionBorder
        anchors.fill: parent
        color: "transparent"
        border.color: "#0066cc"
        border.width: selected ? 2 : 0
        visible: selected
        z: 1000
        antialiasing: true
    }
    
    // Clipping support
    clip: {
        if (element && element.elementType === "Frame") {
            return element.clipContent
        }
        return false
    }
    
    // Common mouse handling
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton // Let CanvasView handle mouse events
        hoverEnabled: true
        
        onEntered: {
            if (canvasController && canvasController.mode === "select") {
                cursorShape = Qt.PointingHandCursor
            }
        }
        
        onExited: {
            cursorShape = Qt.ArrowCursor
        }
    }
}