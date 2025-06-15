import QtQuick
import QtQuick.Controls
import "../CanvasUtils.js" as Utils

Item {
    id: ih
    
    // Properties for coordinate transformation
    property real contentX: 0
    property real contentY: 0
    property real zoom: 1.0
    property real canvasMinX: -10000
    property real canvasMinY: -10000
    
    // Signals for canvas interactions
    signal clicked(point canvasPoint)
    signal dragStarted(point canvasPoint)
    signal dragMoved(point canvasPoint)
    signal dragEnded(point canvasPoint)
    signal hovered(point canvasPoint)
    signal exited()
    signal panned(real dx, real dy)
    signal zoomed(point canvasPoint, real scaleFactor)
    
    // Internal state
    property bool _panning: false
    property point _lastPan
    property bool _isDragging: false
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        
        property point pressPt
        property bool mouseMoved: false
        
        onPressed: (mouse) => {
            pressPt = Qt.point(mouse.x, mouse.y)
            mouseMoved = false
            
            if (mouse.button === Qt.MiddleButton) {
                ih._panning = true
                ih._lastPan = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                var canvasPoint = Utils.viewportToCanvas(
                    Qt.point(mouse.x, mouse.y),
                    ih.contentX,
                    ih.contentY,
                    ih.zoom,
                    ih.canvasMinX,
                    ih.canvasMinY
                )
                ih.dragStarted(canvasPoint)
            }
        }
        
        onPositionChanged: (mouse) => {
            // Check if mouse moved significantly (more than 3 pixels)
            if (pressed && !mouseMoved) {
                var dx = Math.abs(mouse.x - pressPt.x)
                var dy = Math.abs(mouse.y - pressPt.y)
                if (dx > 3 || dy > 3) {
                    mouseMoved = true
                    ih._isDragging = true
                }
            }
            
            if (ih._panning) {
                var dx = mouse.x - ih._lastPan.x
                var dy = mouse.y - ih._lastPan.y
                ih._lastPan = Qt.point(mouse.x, mouse.y)
                ih.panned(dx, dy)
            } else if (pressed) {
                var canvasPoint = Utils.viewportToCanvas(
                    Qt.point(mouse.x, mouse.y),
                    ih.contentX,
                    ih.contentY,
                    ih.zoom,
                    ih.canvasMinX,
                    ih.canvasMinY
                )
                ih.dragMoved(canvasPoint)
            } else {
                var canvasPoint = Utils.viewportToCanvas(
                    Qt.point(mouse.x, mouse.y),
                    ih.contentX,
                    ih.contentY,
                    ih.zoom,
                    ih.canvasMinX,
                    ih.canvasMinY
                )
                ih.hovered(canvasPoint)
            }
        }
        
        onReleased: (mouse) => {
            if (ih._panning) {
                ih._panning = false
                cursorShape = Qt.ArrowCursor
            } else if (mouse.button === Qt.LeftButton) {
                var canvasPoint = Utils.viewportToCanvas(
                    Qt.point(mouse.x, mouse.y),
                    ih.contentX,
                    ih.contentY,
                    ih.zoom,
                    ih.canvasMinX,
                    ih.canvasMinY
                )
                ih.dragEnded(canvasPoint)
                
                // Was it a click? (less than 5 pixels movement)
                if (Math.sqrt(Math.pow(mouse.x - pressPt.x, 2) + Math.pow(mouse.y - pressPt.y, 2)) < 5) {
                    ih.clicked(canvasPoint)
                }
                
                ih._isDragging = false
            }
        }
        
        onExited: {
            ih.exited()
        }
        
        onWheel: (wheel) => {
            if (wheel.modifiers & Qt.ControlModifier) {
                var canvasPoint = Utils.viewportToCanvas(
                    Qt.point(wheel.x, wheel.y),
                    ih.contentX,
                    ih.contentY,
                    ih.zoom,
                    ih.canvasMinX,
                    ih.canvasMinY
                )
                ih.zoomed(canvasPoint, wheel.angleDelta.y > 0 ? 1.1 : 0.9)
                wheel.accepted = true
            } else {
                wheel.accepted = false
            }
        }
    }
    
    // Helper function to convert viewport coordinates to canvas coordinates
    function toCanvasCoords(x, y) {
        return Utils.viewportToCanvas(
            Qt.point(x, y),
            contentX,
            contentY,
            zoom,
            canvasMinX,
            canvasMinY
        )
    }
    
    // Check if currently dragging
    function isDragging() {
        return _isDragging
    }
    
    // Check if currently panning
    function isPanning() {
        return _panning
    }
}