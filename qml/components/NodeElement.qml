import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property var canvas  // Reference to the canvas
    property Node nodeElement: element as Node
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 150
    height: element ? element.height : 80
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Hover state - check if this element is hovered in the canvas
    property bool elementHovered: canvas && canvas.hoveredElement === element
    
    // Check if hover is specifically over the node handle
    property bool handleHovered: {
        if (!elementHovered || !canvas) return false
        
        // Get the hover point relative to this element
        var localX = canvas.hoveredPoint.x - element.x
        var localY = canvas.hoveredPoint.y - element.y
        
        // Check if point is within the handle bounds
        // Handle is anchored right/bottom with 10px margins
        var handleLeft = element.width - 40  // width - margin - handleWidth
        var handleTop = element.height - 40  // height - margin - handleHeight
        var handleRight = element.width - 10
        var handleBottom = element.height - 10
        
        return localX >= handleLeft && localX <= handleRight && 
               localY >= handleTop && localY <= handleBottom
    }
    
    // Log node properties when element changes
    onElementChanged: {
        if (element) {
            console.log("NodeElement created/updated:")
            console.log("  id:", element.id)
            console.log("  objectName:", element.objectName)
            console.log("  position:", element.x, ",", element.y)
            console.log("  size:", element.width, "x", element.height)
            console.log("  nodeTitle:", nodeElement ? nodeElement.nodeTitle : "null")
        }
    }
    
    // Simple node visual representation for debugging
    Rectangle {
        id: nodeBody
        anchors.fill: parent
        
        // Node-specific properties
        color: "#E6F3FF"  // Hardcoded light blue
        border.width: 2
        border.color: "red"  // Red border for visibility
        radius: 8
        
        Component.onCompleted: {
            console.log("NodeElement Rectangle - color:", color)
            console.log("NodeElement Rectangle - size:", width, "x", height)
            console.log("NodeElement Rectangle - visible:", visible)
            console.log("NodeElement Rectangle - opacity:", opacity)
        }
        
        // Simple title text
        Text {
            anchors.centerIn: parent
            text: nodeElement ? nodeElement.nodeTitle : "Node"
            color: "black"
            font.pixelSize: 16
        }
        
        // Node Handle
        Rectangle {
            id: nodeHandle
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            width: 30
            height: 30
            color: root.handleHovered ? "green" : "red"
        }
    }
}