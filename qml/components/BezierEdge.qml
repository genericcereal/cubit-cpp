import QtQuick
import QtQuick.Shapes
import Cubit 1.0
import ".."

Shape {
    id: root
    
    property var edge: null
    property real canvasMinX: 0
    property real canvasMinY: 0
    property bool isPreview: false  // For edge drag preview
    
    // Convert edge points to local coordinates
    property point sourcePoint: edge ? Qt.point(edge.sourcePoint.x - canvasMinX, 
                                               edge.sourcePoint.y - canvasMinY) : Qt.point(0, 0)
    property point targetPoint: edge ? Qt.point(edge.targetPoint.x - canvasMinX, 
                                               edge.targetPoint.y - canvasMinY) : Qt.point(0, 0)
    property point controlPoint1: edge ? Qt.point(edge.controlPoint1.x - canvasMinX, 
                                                 edge.controlPoint1.y - canvasMinY) : Qt.point(0, 0)
    property point controlPoint2: edge ? Qt.point(edge.controlPoint2.x - canvasMinX, 
                                                 edge.controlPoint2.y - canvasMinY) : Qt.point(0, 0)
    
    // Determine edge type and styling
    property string edgeType: {
        if (!edge) return "Flow"
        // Check source port type (both should be the same type)
        return edge.sourcePortType || "Flow"
    }
    
    property string strokeColor: {
        if (!edge) return Config.edgeFlowColor
        
        if (edgeType === "Flow") {
            return edge.selected ? Config.edgeFlowSelectedColor : Config.edgeFlowColor
        } else {
            return edge.selected ? Config.edgeVariableSelectedColor : Config.edgeVariableColor
        }
    }
    
    property int strokeWidth: {
        if (!edge) return Config.edgeFlowWidth
        
        if (edgeType === "Flow") {
            return edge.selected ? Config.edgeFlowSelectedWidth : Config.edgeFlowWidth
        } else {
            return edge.selected ? Config.edgeVariableSelectedWidth : Config.edgeVariableWidth
        }
    }
    
    ShapePath {
        strokeColor: root.strokeColor
        strokeWidth: root.strokeWidth
        fillColor: "transparent"
        
        // Add dashed style for data edges (non-Flow)
        strokeStyle: root.edgeType !== "Flow" ? ShapePath.DashLine : ShapePath.SolidLine
        dashPattern: root.edgeType !== "Flow" ? [5, 3] : []
        
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