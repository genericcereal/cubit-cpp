import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property Frame frameElement: element as Frame
    property alias contentContainer: contentContainer
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Frame visual representation
    Rectangle {
        id: frameRect
        anchors.fill: parent
        
        // Frame-specific properties - light blue box with no border
        color: "#E6F3FF"  // Light blue
        border.width: 0
        radius: frameElement ? frameElement.borderRadius : 0
        antialiasing: true
        
        // Content container for child elements
        Item {
            id: contentContainer
            anchors.fill: parent
            anchors.margins: frameRect.border.width
            
            // Child elements will be added here in a future update
        }
    }
}