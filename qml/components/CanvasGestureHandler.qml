import QtQuick
import QtQuick.Controls
import "../CanvasUtils.js" as Utils

Item {
    id: root
    
    anchors.fill: parent
    
    // Inputs from the canvas
    required property real contentX
    required property real contentY
    required property real zoom
    required property real canvasMinX
    required property real canvasMinY
    
    // High-level signals for gestures only
    signal hover(point pt)
    signal exitCanvas()
    signal pan(real dx, real dy)
    signal pinch(point pt, real scaleFactor)
    
    // Mouse area for middle-button panning
    MouseArea {
        id: panArea
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton
        
        property bool isPanning: false
        property point lastPanPosition
        
        onPressed: (mouse) => {
            isPanning = true
            lastPanPosition = Qt.point(mouse.x, mouse.y)
            cursorShape = Qt.ClosedHandCursor
        }
        
        onPositionChanged: (mouse) => {
            if (isPanning) {
                var dx = mouse.x - lastPanPosition.x
                var dy = mouse.y - lastPanPosition.y
                lastPanPosition = Qt.point(mouse.x, mouse.y)
                root.pan(dx, dy)
            }
        }
        
        onReleased: {
            isPanning = false
            cursorShape = Qt.ArrowCursor
        }
    }
    
    // Mouse area for hover tracking and wheel events
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        hoverEnabled: true
        
        property real throttleDelay: 16 // 60fps
        property point pendingMousePosition
        property bool hasPendingMouseMove: false
        
        Timer {
            id: throttleTimer
            interval: hoverArea.throttleDelay
            repeat: false
            onTriggered: {
                if (hoverArea.hasPendingMouseMove) {
                    var canvasPt = Utils.viewportToCanvas(
                        hoverArea.pendingMousePosition,
                        root.contentX, root.contentY, root.zoom, 
                        root.canvasMinX, root.canvasMinY
                    )
                    root.hover(canvasPt)
                    hoverArea.hasPendingMouseMove = false
                }
            }
        }
        
        onPositionChanged: (mouse) => {
            // Throttle hover events
            pendingMousePosition = Qt.point(mouse.x, mouse.y)
            hasPendingMouseMove = true
            if (!throttleTimer.running) {
                throttleTimer.start()
            }
        }
        
        onExited: {
            root.exitCanvas()
        }
        
        // Handle wheel events for zoom
        onWheel: (wheel) => {
            if (wheel.modifiers & Qt.ControlModifier) {
                var delta = wheel.angleDelta.y
                if (Math.abs(delta) > 0) {
                    var scaleFactor = delta > 0 ? 1.1 : 0.9
                    var canvasPt = Utils.viewportToCanvas(
                        Qt.point(wheel.x, wheel.y),
                        root.contentX, root.contentY, root.zoom, 
                        root.canvasMinX, root.canvasMinY
                    )
                    root.pinch(canvasPt, scaleFactor)
                    wheel.accepted = true
                }
            } else {
                wheel.accepted = false
            }
        }
    }
    
    // Pinch handler for touch zoom
    PinchHandler {
        id: pinchHandler
        target: null  // We don't want it to directly modify anything
        
        property real startZoom: 1.0
        property point startCentroid
        
        onActiveChanged: {
            if (active) {
                startZoom = root.zoom
                startCentroid = Utils.viewportToCanvas(
                    centroid.position,
                    root.contentX, root.contentY, root.zoom, 
                    root.canvasMinX, root.canvasMinY
                )
            }
        }
        
        onScaleChanged: {
            if (active && Math.abs(scale - 1.0) > 0.01) {
                root.pinch(startCentroid, scale)
            }
        }
    }
}