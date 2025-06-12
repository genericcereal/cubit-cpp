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
        active = true
        startPoint = point
        currentPoint = point
        if (selectionManager) {
            selectionManager.clearSelection()
        }
    }
    
    function updateSelection(point) {
        currentPoint = point
        
        // Calculate selection rectangle
        var rect = Qt.rect(
            Math.min(startPoint.x, currentPoint.x),
            Math.min(startPoint.y, currentPoint.y),
            Math.abs(currentPoint.x - startPoint.x),
            Math.abs(currentPoint.y - startPoint.y)
        )
        
        // Notify parent of selection rect change
        if (onSelectionRectChanged) {
            onSelectionRectChanged(rect)
        }
    }
    
    function endSelection() {
        active = false
    }
}