import QtQuick
import QtQuick.Controls
import "../CanvasUtils.js" as Utils

MouseArea {
    id: root
    
    anchors.fill: parent
    acceptedButtons: Qt.AllButtons
    hoverEnabled: true
    
    // Inputs from the canvas
    required property real contentX
    required property real contentY
    required property real zoom
    required property real canvasMinX
    required property real canvasMinY
    
    // High-level signals in canvas coords
    signal press(point pt)
    signal click(point pt)
    signal move(point pt)
    signal release(point pt)
    signal hover(point pt)
    signal exitCanvas()
    signal pan(real dx, real dy)
    signal pinch(point pt, real scaleFactor)
    
    // Internal state
    property bool isPanning: false
    property point lastPanPosition
    property point pressPosition
    property bool hasMoved: false
    
    // Throttling for performance
    property real throttleDelay: 16 // 60fps
    property point pendingMousePosition
    property bool hasPendingMouseMove: false
    
    // Single throttle timer for all throttled events
    Timer {
        id: throttleTimer
        interval: root.throttleDelay
        repeat: false
        onTriggered: {
            if (hasPendingMouseMove) {
                processMouseMove(pendingMousePosition.x, pendingMousePosition.y)
                hasPendingMouseMove = false
            }
        }
    }
    
    onPressed: (mouse) => {
        var canvasPt = Utils.viewportToCanvas(
            Qt.point(mouse.x, mouse.y),
            contentX, contentY, zoom, canvasMinX, canvasMinY
        )
        
        pressPosition = Qt.point(mouse.x, mouse.y)
        hasMoved = false
        
        if (mouse.button === Qt.MiddleButton) {
            // Start panning
            isPanning = true
            lastPanPosition = Qt.point(mouse.x, mouse.y)
            cursorShape = Qt.ClosedHandCursor
            mouse.accepted = true
        } else if (mouse.button === Qt.LeftButton) {
            // Emit press for left button
            root.press(canvasPt)
            mouse.accepted = true
        }
    }
    
    onPositionChanged: (mouse) => {
        // Track if mouse has moved significantly (more than 3 pixels)
        if (!hasMoved && pressed) {
            var dx = Math.abs(mouse.x - pressPosition.x)
            var dy = Math.abs(mouse.y - pressPosition.y)
            if (dx > 3 || dy > 3) {
                hasMoved = true
            }
        }
        
        if (isPanning) {
            // Panning is not throttled for smooth experience
            var dx = mouse.x - lastPanPosition.x
            var dy = mouse.y - lastPanPosition.y
            lastPanPosition = Qt.point(mouse.x, mouse.y)
            root.pan(dx, dy)
        } else {
            // Throttle other mouse moves
            pendingMousePosition = Qt.point(mouse.x, mouse.y)
            hasPendingMouseMove = true
            if (!throttleTimer.running) {
                throttleTimer.start()
            }
        }
    }
    
    function processMouseMove(mouseX, mouseY) {
        var canvasPt = Utils.viewportToCanvas(
            Qt.point(mouseX, mouseY),
            contentX, contentY, zoom, canvasMinX, canvasMinY
        )
        
        if (pressed && !isPanning) {
            root.move(canvasPt)
        } else if (!pressed) {
            root.hover(canvasPt)
        }
    }
    
    onReleased: (mouse) => {
        var canvasPt = Utils.viewportToCanvas(
            Qt.point(mouse.x, mouse.y),
            contentX, contentY, zoom, canvasMinX, canvasMinY
        )
        
        if (isPanning) {
            isPanning = false
            cursorShape = Qt.ArrowCursor
            mouse.accepted = true
        } else if (mouse.button === Qt.LeftButton) {
            root.release(canvasPt)
            
            // Detect click (minimal movement)
            if (!hasMoved || (Math.abs(mouse.x - pressPosition.x) < 5 && 
                             Math.abs(mouse.y - pressPosition.y) < 5)) {
                root.click(canvasPt)
            }
            
            mouse.accepted = true
        }
    }
    
    onExited: {
        root.exitCanvas()
    }
    
    // Override wheel event in MouseArea
    onWheel: (wheel) => {
        if (wheel.modifiers & Qt.ControlModifier) {
            var delta = wheel.angleDelta.y
            if (Math.abs(delta) > 0) {
                var scaleFactor = delta > 0 ? 1.1 : 0.9
                var canvasPt = Utils.viewportToCanvas(
                    Qt.point(wheel.x, wheel.y),
                    contentX, contentY, zoom, canvasMinX, canvasMinY
                )
                root.pinch(canvasPt, scaleFactor)
                wheel.accepted = true
            }
        } else {
            wheel.accepted = false
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