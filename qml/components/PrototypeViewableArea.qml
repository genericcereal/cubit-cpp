import QtQuick 2.15
import QtQuick.Controls 2.15
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    width: viewableArea.width
    height: viewableArea.height
    
    // Properties passed from ViewportOverlay
    property var designCanvas: null
    property var flickable: null
    property var prototypeController: null
    
    // Track if we're simulating scrolling
    property bool isSimulatingScroll: false
    property real frozenX: 0
    property real frozenY: 0
    
    // Get the active outer frame element
    property var activeFrameElement: {
        if (!prototypeController || !prototypeController.activeOuterFrame || !designCanvas || !designCanvas.elementModel) {
            return null
        }
        return designCanvas.elementModel.getElementById(prototypeController.activeOuterFrame)
    }
    
    // Position to follow the active outer frame
    x: {
        // When simulating scroll, use frozen position
        if (isSimulatingScroll) {
            return frozenX
        }
        
        if (!visible || !activeFrameElement || !flickable || !designCanvas) {
            return (parent.width - width) / 2
        }
        
        // Calculate the frame's viewport position
        var frameX = activeFrameElement.x
        var canvasMinX = designCanvas.canvasMinX || 0
        var zoom = designCanvas.zoomLevel || 1.0
        var contentX = flickable.contentX || 0
        
        // Convert frame canvas position to viewport position
        var frameViewportX = (frameX - canvasMinX) * zoom - contentX
        
        // Center the viewable area on the frame horizontally
        return frameViewportX + (activeFrameElement.width * zoom - width) / 2
    }
    
    y: {
        // When simulating scroll, use frozen position
        if (isSimulatingScroll) {
            return frozenY
        }
        
        if (!visible || !activeFrameElement || !flickable || !designCanvas) {
            return (parent.height - height) / 2
        }
        
        // Calculate the frame's viewport position
        var frameY = activeFrameElement.y
        var canvasMinY = designCanvas.canvasMinY || 0
        var zoom = designCanvas.zoomLevel || 1.0
        var contentY = flickable.contentY || 0
        
        // Convert frame canvas position to viewport position
        var frameViewportY = (frameY - canvasMinY) * zoom - contentY
        
        // Align the viewable area's top edge with the frame's top edge
        return frameViewportY
    }
    
    Component.onCompleted: {
    }
    
    onVisibleChanged: {
        // Reset scroll state when visibility changes
        if (!visible) {
            isSimulatingScroll = false
            frozenX = 0
            frozenY = 0
        }
    }
    
    // Reset scroll state when active frame changes
    onActiveFrameElementChanged: {
        isSimulatingScroll = false
        frozenX = 0
        frozenY = 0
    }
    
    // The actual viewable area
    Rectangle {
        id: viewableArea
        
        width: root.prototypeController ? root.prototypeController.viewableArea.width : 400
        height: root.prototypeController ? root.prototypeController.viewableArea.height : 400
        color: "transparent"
        border.color: "red"
        border.width: 3
        
        // Center in parent
        anchors.centerIn: parent
        
        // MouseArea to intercept all mouse events and prevent canvas panning
        MouseArea {
            id: blockingMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
            
            // Accept all mouse events to prevent them from reaching the canvas
            onPressed: (mouse) => {
                mouse.accepted = true
            }
            
            onReleased: (mouse) => {
                mouse.accepted = true
            }
            
            onPositionChanged: (mouse) => {
                mouse.accepted = true
            }
            
            onClicked: (mouse) => {
                mouse.accepted = true
            }
            
            onDoubleClicked: (mouse) => {
                mouse.accepted = true
            }
            
            // Most importantly, handle wheel events to prevent canvas scrolling
            onWheel: (wheel) => {
                wheel.accepted = true
                
                // Move the activeOuterFrame based on scroll to simulate device scrolling
                if (root.activeFrameElement && root.prototypeController) {
                    // Freeze position if not already simulating
                    if (!root.isSimulatingScroll) {
                        root.frozenX = root.x
                        root.frozenY = root.y
                        root.isSimulatingScroll = true
                    }
                    
                    // Invert the scroll direction to match natural scrolling behavior
                    // When scrolling down (negative angleDelta.y), content should move up (decrease y)
                    var scrollSpeed = 1.0  // Adjust this value to control scroll sensitivity
                    var deltaY = -wheel.angleDelta.y / 8 * scrollSpeed  // Divide by 8 to convert from wheel units to pixels
                    
                    // Calculate the new Y position
                    var newY = root.activeFrameElement.y + deltaY
                    
                    // Get frame and viewable area bounds in canvas coordinates
                    var frameHeight = root.activeFrameElement.height
                    var viewableHeight = viewableArea.height
                    var zoom = root.designCanvas.zoomLevel || 1.0
                    
                    // Convert viewable area position to canvas coordinates
                    var viewableTopInCanvas = (root.frozenY + root.flickable.contentY) / zoom + (root.designCanvas.canvasMinY || 0)
                    var viewableBottomInCanvas = viewableTopInCanvas + viewableHeight / zoom
                    
                    // Apply constraints:
                    // 1. Top edge of frame cannot go below top edge of viewable area
                    if (newY > viewableTopInCanvas) {
                        newY = viewableTopInCanvas
                    }
                    
                    // 2. Bottom edge of frame cannot go above bottom edge of viewable area
                    var frameBottom = newY + frameHeight
                    if (frameBottom < viewableBottomInCanvas) {
                        newY = viewableBottomInCanvas - frameHeight
                    }
                    
                    // Update the frame's y position with constraints
                    root.activeFrameElement.y = newY
                }
            }
        }
    }
}