import QtQuick

Item {
    id: root
    
    // Properties that need to be provided by parent
    property real zoomLevel: 1.0
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    property real gridSize: 50
    property color gridColor: "#e0e0e0"
    property color originColor: "#cccccc"
    property real gridLineWidth: 1
    property real originLineWidth: 2
    property real visibilityThreshold: 0.5
    
    // Internal properties
    property real scaledGridSize: gridSize * zoomLevel
    
    // Visibility based on zoom level
    visible: zoomLevel > visibilityThreshold && scaledGridSize > 10
    opacity: Math.min(1, (zoomLevel - visibilityThreshold) * 2)
    
    // Use ShaderEffectSource or simple Rectangle grid for better performance
    Repeater {
        model: {
            if (!root.visible) return []
            
            var lines = []
            var viewWidth = root.width
            var viewHeight = root.height
            
            // Calculate visible grid lines only
            var startX = Math.floor(-root.canvasMinX / gridSize) * gridSize
            var endX = startX + viewWidth / zoomLevel + gridSize
            var startY = Math.floor(-root.canvasMinY / gridSize) * gridSize
            var endY = startY + viewHeight / zoomLevel + gridSize
            
            // Vertical lines
            for (var x = startX; x < endX; x += gridSize) {
                var screenX = (x + root.canvasMinX) * zoomLevel
                if (screenX >= 0 && screenX <= viewWidth) {
                    lines.push({
                        x: screenX,
                        y: 0,
                        width: gridLineWidth,
                        height: viewHeight,
                        color: x === 0 ? originColor : gridColor,
                        lineWidth: x === 0 ? originLineWidth : gridLineWidth
                    })
                }
            }
            
            // Horizontal lines
            for (var y = startY; y < endY; y += gridSize) {
                var screenY = (y + root.canvasMinY) * zoomLevel
                if (screenY >= 0 && screenY <= viewHeight) {
                    lines.push({
                        x: 0,
                        y: screenY,
                        width: viewWidth,
                        height: gridLineWidth,
                        color: y === 0 ? originColor : gridColor,
                        lineWidth: y === 0 ? originLineWidth : gridLineWidth
                    })
                }
            }
            
            return lines
        }
        
        delegate: Rectangle {
            x: modelData.x
            y: modelData.y
            width: modelData.width
            height: modelData.height
            color: modelData.color
            opacity: 0.5
        }
    }
}