import QtQuick 2.15
import QtQuick.Shapes 1.15
import Cubit 1.0

Item {
    id: root
    
    // The C++ Element object
    property var element
    property var elementModel
    property ShapeElement shapeElement: element as ShapeElement
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Size properties bound to C++ element (position is handled by parent Loader)
    width: element ? element.width : 100
    height: element ? element.height : 100
    
    // Selection state
    property bool selected: element ? element.selected : false
    
    Component.onCompleted: {
        // Debug logging removed
    }
    
    Component.onDestruction: {
        // Debug logging removed
    }
    
    onShapeElementChanged: {
        // Debug logging removed
    }
    
    // Debug rectangle to see bounds
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "red"
        border.width: 1
        opacity: 0.5
    }
    
    // Shape rendering
    Shape {
        id: shape
        anchors.fill: parent
        antialiasing: true
        
        ShapePath {
            id: shapePath
            strokeColor: shapeElement ? shapeElement.edgeColor : "#000000"
            strokeWidth: shapeElement ? shapeElement.edgeWidth : 2
            fillColor: shapeElement ? shapeElement.fillColor : "transparent"
            strokeStyle: ShapePath.SolidLine
            
            // Start point
            startX: {
                if (!shapeElement || !shapeElement.joints || shapeElement.joints.length === 0) {
                    return 0
                }
                return shapeElement.joints[0].x * root.width
            }
            startY: {
                if (!shapeElement || !shapeElement.joints || shapeElement.joints.length === 0) {
                    return 0
                }
                return shapeElement.joints[0].y * root.height
            }
            
            // Path elements created dynamically based on joints
            PathPolyline {
                id: polyline
                path: {
                    // Check if component is being destroyed
                    if (!root || !shape) {
                        return []
                    }
                    
                    if (!shapeElement || !shapeElement.joints || shapeElement.joints.length === 0) {
                        return []
                    }
                    
                    var points = []
                    for (var i = 0; i < shapeElement.joints.length; i++) {
                        var joint = shapeElement.joints[i]
                        points.push(Qt.point(joint.x * root.width, joint.y * root.height))
                    }
                    
                    return points
                }
            }
        }
    }
}