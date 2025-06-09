import QtQuick
import QtQuick.Controls
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property TextElement textElement: element as TextElement
    
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
    }
    
    // Text visual representation
    Text {
        id: textItem
        anchors.fill: parent
        
        // Text-specific properties
        text: textElement ? textElement.text : ""
        color: textElement ? textElement.color : "black"
        font: textElement ? textElement.font : Qt.font({family: "Arial", pixelSize: 14})
        
        // Text layout
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        
        // Ensure text is visible
        clip: true
    }
}