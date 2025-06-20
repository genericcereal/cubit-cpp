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
    
    // Frame throttling for better performance
    property bool updatePending: false
    property point pendingPoint: Qt.point(0, 0)
    
    function startSelection(point) {
        active = true
        startPoint = point
        currentPoint = point
        if (selectionManager) {
            selectionManager.clearSelection()
        }
    }
    
    function updateSelection(point) {
        pendingPoint = point
        
        if (!updatePending) {
            updatePending = true
            Qt.callLater(performUpdate)
        }
    }
    
    function performUpdate() {
        updatePending = false
        currentPoint = pendingPoint
        
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