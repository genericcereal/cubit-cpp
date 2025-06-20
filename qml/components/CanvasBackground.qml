import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    
    // Background styling
    color: "#f5f5f5"
    antialiasing: true
    
    // Signal emitted when background is clicked
    signal clicked(point canvasPoint)
    
    // Canvas reference for coordinate conversion
    property var canvas
    
    MouseArea {
        id: backgroundMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        onClicked: (mouse) => {
            // Only emit signal if canvas reference is set
            if (root.canvas) {
                // The background fills the content layer, so mouse position is already in canvas space
                // Just need to add the canvas minimum bounds
                var canvasX = mouse.x + (root.canvas.canvasMinX || 0)
                var canvasY = mouse.y + (root.canvas.canvasMinY || 0)
                var canvasPoint = Qt.point(canvasX, canvasY)
                
                console.log("CanvasBackground clicked at canvas position:", canvasPoint.x, canvasPoint.y)
                root.clicked(canvasPoint)
            }
        }
    }
}