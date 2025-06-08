import QtQuick

// Individual resize handle that maintains constant viewport size
Rectangle {
    id: root
    
    property string position: "top-left" // Position identifier
    property var element: null // The element being resized
    property real handleSize: 8 // Size in viewport pixels
    property real zoomLevel: 1.0
    property var flickable: null
    
    width: handleSize
    height: handleSize
    color: "#0066cc"
    border.color: "#004499"
    border.width: 1
    radius: 2
    
    // Position the handle based on element bounds and handle position
    x: {
        if (!element || !flickable) return 0
        
        var elementX = element.x * zoomLevel - flickable.contentX
        var elementWidth = element.width * zoomLevel
        
        switch(position) {
            case "top-left":
            case "middle-left":
            case "bottom-left":
                return elementX - handleSize / 2
            case "top-center":
            case "bottom-center":
                return elementX + elementWidth / 2 - handleSize / 2
            case "top-right":
            case "middle-right":
            case "bottom-right":
                return elementX + elementWidth - handleSize / 2
            default:
                return 0
        }
    }
    
    y: {
        if (!element || !flickable) return 0
        
        var elementY = element.y * zoomLevel - flickable.contentY
        var elementHeight = element.height * zoomLevel
        
        switch(position) {
            case "top-left":
            case "top-center":
            case "top-right":
                return elementY - handleSize / 2
            case "middle-left":
            case "middle-right":
                return elementY + elementHeight / 2 - handleSize / 2
            case "bottom-left":
            case "bottom-center":
            case "bottom-right":
                return elementY + elementHeight - handleSize / 2
            default:
                return 0
        }
    }
    
    // Cursor shape based on position
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        cursorShape: {
            switch(root.position) {
                case "top-left":
                case "bottom-right":
                    return Qt.SizeFDiagCursor
                case "top-right":
                case "bottom-left":
                    return Qt.SizeBDiagCursor
                case "top-center":
                case "bottom-center":
                    return Qt.SizeVerCursor
                case "middle-left":
                case "middle-right":
                    return Qt.SizeHorCursor
                default:
                    return Qt.ArrowCursor
            }
        }
        
        property point pressPoint: Qt.point(0, 0)
        property real startX: 0
        property real startY: 0
        property real startWidth: 0
        property real startHeight: 0
        
        onPressed: (mouse) => {
            if (!element) return
            
            pressPoint = Qt.point(mouse.x, mouse.y)
            startX = element.x
            startY = element.y
            startWidth = element.width
            startHeight = element.height
        }
        
        onPositionChanged: (mouse) => {
            if (!pressed || !element) return
            
            var deltaX = (mouse.x - pressPoint.x) / zoomLevel
            var deltaY = (mouse.y - pressPoint.y) / zoomLevel
            
            // Apply resize based on handle position
            switch(root.position) {
                case "top-left":
                    element.x = Math.min(startX + deltaX, startX + startWidth - 20)
                    element.y = Math.min(startY + deltaY, startY + startHeight - 20)
                    element.width = Math.max(20, startWidth - deltaX)
                    element.height = Math.max(20, startHeight - deltaY)
                    break
                case "top-center":
                    element.y = Math.min(startY + deltaY, startY + startHeight - 20)
                    element.height = Math.max(20, startHeight - deltaY)
                    break
                case "top-right":
                    element.y = Math.min(startY + deltaY, startY + startHeight - 20)
                    element.width = Math.max(20, startWidth + deltaX)
                    element.height = Math.max(20, startHeight - deltaY)
                    break
                case "middle-left":
                    element.x = Math.min(startX + deltaX, startX + startWidth - 20)
                    element.width = Math.max(20, startWidth - deltaX)
                    break
                case "middle-right":
                    element.width = Math.max(20, startWidth + deltaX)
                    break
                case "bottom-left":
                    element.x = Math.min(startX + deltaX, startX + startWidth - 20)
                    element.width = Math.max(20, startWidth - deltaX)
                    element.height = Math.max(20, startHeight + deltaY)
                    break
                case "bottom-center":
                    element.height = Math.max(20, startHeight + deltaY)
                    break
                case "bottom-right":
                    element.width = Math.max(20, startWidth + deltaX)
                    element.height = Math.max(20, startHeight + deltaY)
                    break
            }
        }
    }
}