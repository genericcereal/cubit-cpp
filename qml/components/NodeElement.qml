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
    width: element ? element.width : 200
    height: element ? element.height : 180
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    // Hover state - check if this element is hovered in the canvas
    property bool elementHovered: canvas && canvas.hoveredElement === element
    
    // Log node properties when element changes
    onElementChanged: {
        if (element) {
            console.log("NodeElement created/updated:")
            console.log("  elementId:", element.elementId)
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
        
        // Title header
        Text {
            id: titleText
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 10
            text: nodeElement ? nodeElement.nodeTitle : "Node"
            color: "black"
            font.pixelSize: 16
            font.bold: true
        }
        
        // Container for NodeRow components
        Column {
            id: rowContainer
            anchors.top: titleText.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            anchors.topMargin: 15
            spacing: 10
            
            // NodeRow component definition
            component NodeRow: Item {
                id: nodeRow
                width: parent.width
                height: 30
                
                property string label: "Port"
                property int rowIndex: 0
                
                // Check if left handle is hovered
                property bool leftHandleHovered: {
                    if (!root.elementHovered || !root.canvas) return false
                    
                    // Get the hover point relative to this nodeRow
                    var localX = root.canvas.hoveredPoint.x - root.element.x - rowContainer.anchors.leftMargin - nodeRow.x
                    var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - rowContainer.anchors.topMargin - nodeRow.y
                    
                    // Check if point is within the left handle bounds
                    return localX >= 0 && localX <= 20 && 
                           localY >= 5 && localY <= 25  // vertically centered, 30px tall row
                }
                
                // Check if right handle is hovered
                property bool rightHandleHovered: {
                    if (!root.elementHovered || !root.canvas) return false
                    
                    // Get the hover point relative to this nodeRow
                    var localX = root.canvas.hoveredPoint.x - root.element.x - rowContainer.anchors.leftMargin - nodeRow.x
                    var localY = root.canvas.hoveredPoint.y - root.element.y - titleText.height - titleText.anchors.topMargin - rowContainer.anchors.topMargin - nodeRow.y
                    
                    // Check if point is within the right handle bounds
                    var rightHandleX = nodeRow.width - 20
                    return localX >= rightHandleX && localX <= nodeRow.width && 
                           localY >= 5 && localY <= 25  // vertically centered, 30px tall row
                }
                
                // Left handle
                Rectangle {
                    id: leftHandle
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    radius: 10
                    color: nodeRow.leftHandleHovered ? "#4CAF50" : "#666666"
                    border.width: 2
                    border.color: nodeRow.leftHandleHovered ? "#2E7D32" : "#333333"
                }
                
                // Label in the middle
                Text {
                    anchors.centerIn: parent
                    text: nodeRow.label
                    color: "#333333"
                    font.pixelSize: 12
                }
                
                // Right handle
                Rectangle {
                    id: rightHandle
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    radius: 10
                    color: nodeRow.rightHandleHovered ? "#2196F3" : "#666666"
                    border.width: 2
                    border.color: nodeRow.rightHandleHovered ? "#1565C0" : "#333333"
                }
            }
            
            // Example NodeRow instances
            NodeRow {
                label: "Input 1"
                rowIndex: 0
            }
            
            NodeRow {
                label: "Input 2"
                rowIndex: 1
            }
            
            NodeRow {
                label: "Output"
                rowIndex: 2
            }
        }
    }
}