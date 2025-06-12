import QtQuick

QtObject {
    id: root
    
    // Properties that need to be provided by the parent
    property var selectionManager
    
    // Selection state
    property bool active: false
    property point startPoint: Qt.point(0, 0)
    property point currentPoint: Qt.point(0, 0)
    
    // Callback function to be implemented by parent
    property var onSelectionRectChanged: null
    
    function startSelection(point) {
        console.log("SelectionBoxHandler.startSelection called with point:", JSON.stringify({x: point.x, y: point.y}))
        active = true
        startPoint = point
        currentPoint = point
        if (selectionManager) {
            console.log("Clearing selection")
            selectionManager.clearSelection()
        } else {
            console.log("WARNING: No selectionManager available")
        }
    }
    
    function updateSelection(point) {
        console.log("SelectionBoxHandler.updateSelection called with point:", JSON.stringify({x: point.x, y: point.y}))
        currentPoint = point
        
        // Calculate selection rectangle
        var rect = Qt.rect(
            Math.min(startPoint.x, currentPoint.x),
            Math.min(startPoint.y, currentPoint.y),
            Math.abs(currentPoint.x - startPoint.x),
            Math.abs(currentPoint.y - startPoint.y)
        )
        
        console.log("Calculated selection rect:", JSON.stringify({x: rect.x, y: rect.y, width: rect.width, height: rect.height}))
        
        // Notify parent of selection rect change
        if (onSelectionRectChanged) {
            console.log("Calling onSelectionRectChanged callback")
            onSelectionRectChanged(rect)
        } else {
            console.log("WARNING: No onSelectionRectChanged callback set")
        }
    }
    
    function endSelection() {
        console.log("SelectionBoxHandler.endSelection called")
        console.log("Final selection rect:", JSON.stringify({
            start: {x: startPoint.x, y: startPoint.y},
            end: {x: currentPoint.x, y: currentPoint.y},
            rect: {
                x: Math.min(startPoint.x, currentPoint.x),
                y: Math.min(startPoint.y, currentPoint.y),
                width: Math.abs(currentPoint.x - startPoint.x),
                height: Math.abs(currentPoint.y - startPoint.y)
            }
        }))
        active = false
    }
}