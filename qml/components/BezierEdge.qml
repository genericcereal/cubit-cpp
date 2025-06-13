import QtQuick
import QtQuick.Shapes
import Cubit 1.0

Shape {
    id: root
    
    property var edge: null
    property real canvasMinX: 0
    property real canvasMinY: 0
    
    // Convert edge points to local coordinates
    property point sourcePoint: edge ? Qt.point(edge.sourcePoint.x - canvasMinX, 
                                               edge.sourcePoint.y - canvasMinY) : Qt.point(0, 0)
    property point targetPoint: edge ? Qt.point(edge.targetPoint.x - canvasMinX, 
                                               edge.targetPoint.y - canvasMinY) : Qt.point(0, 0)
    property point controlPoint1: edge ? Qt.point(edge.controlPoint1.x - canvasMinX, 
                                                 edge.controlPoint1.y - canvasMinY) : Qt.point(0, 0)
    property point controlPoint2: edge ? Qt.point(edge.controlPoint2.x - canvasMinX, 
                                                 edge.controlPoint2.y - canvasMinY) : Qt.point(0, 0)
    
    ShapePath {
        strokeColor: edge && edge.selected ? "#2196F3" : "#666666"
        strokeWidth: edge && edge.selected ? 3 : 2
        fillColor: "transparent"
        
        // Start at source point
        startX: sourcePoint.x
        startY: sourcePoint.y
        
        // Bezier curve to target point
        PathCubic {
            x: targetPoint.x
            y: targetPoint.y
            control1X: controlPoint1.x
            control1Y: controlPoint1.y
            control2X: controlPoint2.x
            control2Y: controlPoint2.y
        }
    }
}