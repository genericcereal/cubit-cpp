import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    // Scale the viewable area by zoom level, just like DesignControls
    width: (prototypeController ? prototypeController.viewableArea.width : 400) * canvasZoom
    height: (prototypeController ? prototypeController.viewableArea.height : 400) * canvasZoom
    
    // Properties passed from ViewportOverlay
    property var designCanvas: null
    property var flickable: null
    property var prototypeController: null
    
    // Explicit bindings for canvas properties we need
    property real canvasZoom: designCanvas ? (designCanvas.zoomLevel || 1.0) : 1.0
    property real canvasContentX: flickable ? (flickable.contentX || 0) : 0
    property real canvasContentY: flickable ? (flickable.contentY || 0) : 0
    property real canvasMinX: designCanvas ? (designCanvas.canvasMinX || 0) : 0
    property real canvasMinY: designCanvas ? (designCanvas.canvasMinY || 0) : 0
    
    // Track if we're simulating scrolling
    property bool isSimulatingScroll: false
    property real frozenX: 0
    property real frozenY: 0
    
    // Get the active outer frame element
    property var activeFrameElement: {
        if (!prototypeController || !prototypeController.activeOuterFrame) {
            return null
        }
        
        // designCanvas is actually the canvasView, so we need to get the canvas from it
        var canvas = designCanvas?.canvas
        if (!canvas || !canvas.elementModel) {
            return null
        }
        
        var element = canvas.elementModel.getElementById(prototypeController.activeOuterFrame)
        return element
    }
    
    // Use Binding objects for position, following the same pattern as DesignControls
    Binding {
        target: root
        property: "x"
        value: {
            if (root.isSimulatingScroll) {
                return root.frozenX
            }
            
            if (!root.visible || !root.activeFrameElement || !root.flickable || !root.designCanvas) {
                return (parent.width - root.width) / 2
            }
            
            // Use the same formula as DesignControls
            var frameX = root.activeFrameElement.x
            var frameViewportX = (frameX - root.canvasMinX) * root.canvasZoom - root.canvasContentX
            
            // Center the viewable area on the frame horizontally
            return frameViewportX + (root.activeFrameElement.width * root.canvasZoom - root.width) / 2
        }
        when: root.visible
        restoreMode: Binding.RestoreBinding
    }
    
    Binding {
        target: root
        property: "y"
        value: {
            if (root.isSimulatingScroll) {
                return root.frozenY
            }
            
            if (!root.visible || !root.activeFrameElement || !root.flickable || !root.designCanvas) {
                return (parent.height - root.height) / 2
            }
            
            // Use the same formula as DesignControls
            var frameY = root.activeFrameElement.y
            var frameViewportY = (frameY - root.canvasMinY) * root.canvasZoom - root.canvasContentY
            
            // Now that the Rectangle is at (0,0) in the parent, we can directly align the tops
            return frameViewportY
        }
        when: root.visible
        restoreMode: Binding.RestoreBinding
    }
    
    Component.onCompleted: {
        // PrototypeViewableArea created
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
        
        // Fill the parent which is already scaled
        anchors.fill: parent
        color: "transparent"
        border.color: "red"
        border.width: 3
        
        // MouseArea to intercept all mouse events and prevent canvas panning
        MouseArea {
            id: blockingMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
            
            // Accept all mouse events to prevent them from reaching the canvas
            onPressed: (mouse) => {
                mouse.accepted = true
                
                // First, always clear any active input when clicking anywhere
                if (root.prototypeController) {
                    root.prototypeController.clearActiveInput()
                }
                
                // Then handle the click to potentially activate a new input
                if (root.prototypeController && root.designCanvas && root.activeFrameElement) {
                    // Calculate canvas coordinates same as click handler
                    var frameRelativeX = mouse.x
                    var frameRelativeY = mouse.y
                    
                    var canvasX = root.activeFrameElement.x + frameRelativeX
                    var canvasY = root.activeFrameElement.y + frameRelativeY
                    
                    var canvasPoint = Qt.point(canvasX, canvasY)
                    
                    // Let the prototype controller handle the click
                    root.prototypeController.handlePrototypeClick(canvasPoint)
                }
            }
            
            onReleased: (mouse) => {
                mouse.accepted = true
            }
            
            onPositionChanged: (mouse) => {
                mouse.accepted = true
                
                // Track hovered element
                if (root.prototypeController && root.designCanvas && root.activeFrameElement) {
                    // The mouse position is relative to the viewableArea rectangle
                    // We need to convert this to canvas coordinates
                    
                    // First, get the mouse position relative to the active frame element
                    // Since the viewableArea is positioned to align with the active frame,
                    // we can calculate the position within the frame
                    var frameRelativeX = mouse.x
                    var frameRelativeY = mouse.y
                    
                    // The active frame element's canvas position gives us the offset
                    var canvasX = root.activeFrameElement.x + frameRelativeX
                    var canvasY = root.activeFrameElement.y + frameRelativeY
                    
                    var canvasPoint = Qt.point(canvasX, canvasY)
                    
                    // Update hovered element in prototype controller
                    root.prototypeController.updateHoveredElement(canvasPoint)
                }
            }
            
            onClicked: (mouse) => {
                mouse.accepted = true
                // Click handling is now done in onPressed to ensure focus clearing happens immediately
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
                        // Get the current computed x and y values
                        var currentX = root.x
                        var currentY = root.y
                        root.frozenX = currentX
                        root.frozenY = currentY
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
                    var viewableHeight = root.height  // Use root.height instead of viewableArea.height
                    var zoom = root.canvasZoom
                    
                    // Convert viewable area position to canvas coordinates
                    var viewableTopInCanvas = (root.frozenY + root.canvasContentY) / zoom + root.canvasMinY
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
            
            onExited: {
                // Reset hover when mouse leaves the viewable area
                if (root.prototypeController) {
                    root.prototypeController.updateHoveredElement(Qt.point(-1, -1))
                }
                
                // Reset scroll simulation state when mouse leaves
                if (root.isSimulatingScroll) {
                    root.isSimulatingScroll = false
                    root.frozenX = 0
                    root.frozenY = 0
                    // Console log removed - Reset scroll simulation on mouse exit
                }
            }
        }
    }
}