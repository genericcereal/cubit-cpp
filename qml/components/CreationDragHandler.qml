import QtQuick
import Cubit.UI 1.0

QtObject {
    id: root
    
    // Properties that need to be provided by the parent
    property var controller
    property real defaultWidth: Config.defaultElementWidth
    property real defaultHeight: Config.defaultElementHeight
    property int minimumSize: 10
    
    // Creation state
    property bool active: false
    property point startPoint: Qt.point(0, 0)
    property point currentPoint: Qt.point(0, 0)
    
    function startCreation(point) {
        active = true
        startPoint = point
        currentPoint = point
    }
    
    function updateCreation(point) {
        currentPoint = point
    }
    
    function endCreation() {
        if (active) {
            var width = Math.abs(currentPoint.x - startPoint.x)
            var height = Math.abs(currentPoint.y - startPoint.y)
            
            // Apply minimum size threshold
            if (width < minimumSize) width = defaultWidth
            if (height < minimumSize) height = defaultHeight
            
            var x = Math.min(startPoint.x, currentPoint.x)
            var y = Math.min(startPoint.y, currentPoint.y)
            
            if (controller) {
                controller.createElement(controller.mode, x, y, width, height)
            }
        }
        active = false
    }
    
    // Get the current creation rectangle for preview
    function getCreationRect() {
        return Qt.rect(
            Math.min(startPoint.x, currentPoint.x),
            Math.min(startPoint.y, currentPoint.y),
            Math.abs(currentPoint.x - startPoint.x),
            Math.abs(currentPoint.y - startPoint.y)
        )
    }
}