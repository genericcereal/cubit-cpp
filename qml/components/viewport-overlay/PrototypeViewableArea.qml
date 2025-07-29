import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../design-controls"

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
    
    // Use scroll state from the controller
    property bool isSimulatingScroll: prototypeController ? prototypeController.isSimulatingScroll : false
    property real scrollOffsetX: prototypeController ? prototypeController.scrollOffsetX : 0
    property real scrollOffsetY: prototypeController ? prototypeController.scrollOffsetY : 0
    
    // Force final update when scroll simulation ends
    onIsSimulatingScrollChanged: {
        if (!isSimulatingScroll && scrollThrottle.active) {
            scrollThrottle.forceUpdate()
        }
    }
    
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
    
    // Function to update position based on active frame
    function updatePosition() {
        if (!root.visible || !root.activeFrameElement || !root.flickable || !root.designCanvas) {
            root.x = (parent.width - root.width) / 2
            root.y = (parent.height - root.height) / 2
            return
        }
        
        // Use the same formula as DesignControls
        var frameX = root.activeFrameElement.x
        var frameY = root.activeFrameElement.y
        var frameViewportX = (frameX - root.canvasMinX) * root.canvasZoom - root.canvasContentX
        var frameViewportY = (frameY - root.canvasMinY) * root.canvasZoom - root.canvasContentY
        
        // Center the viewable area on the frame horizontally
        root.x = frameViewportX + (root.activeFrameElement.width * root.canvasZoom - root.width) / 2
        
        // For Y position, apply scroll offset if simulating scroll
        if (root.isSimulatingScroll) {
            // Apply the scroll offset in viewport space
            root.y = frameViewportY - root.scrollOffsetY * root.canvasZoom
        } else {
            // Normal alignment to frame top
            root.y = frameViewportY
        }
        
        // Update width and height based on zoom
        root.width = (prototypeController ? prototypeController.viewableArea.width : 400) * root.canvasZoom
        root.height = (prototypeController ? prototypeController.viewableArea.height : 400) * root.canvasZoom
    }
    
    // Update position when relevant properties change
    onCanvasZoomChanged: updatePosition()
    onCanvasContentXChanged: updatePosition()
    onCanvasContentYChanged: updatePosition()
    
    Component.onCompleted: {
        // PrototypeViewableArea created
        
        // Initialize position
        updatePosition()
        
        // Connect to prototyping stopped signal
        if (prototypeController) {
            prototypeController.prototypingStopped.connect(function() {
                // Update position after prototyping stops
                updatePosition()
            })
        }
    }
    
    onVisibleChanged: {
        if (!visible) {
            // Controller will handle resetting frame positions
        } else {
            // Update position when becoming visible
            updatePosition()
        }
    }
    
    // Reset scroll state when active frame changes
    onActiveFrameElementChanged: {
        // Controller now handles resetting previous frame position
        // Just update position for the new active frame
        updatePosition()
    }
    
    // Throttled update component for scroll operations
    ThrottledUpdate {
        id: scrollThrottle
        // interval uses default from ConfigObject.throttleInterval
        active: root.isSimulatingScroll
        
        onUpdate: (data) => {
            if (root.activeFrameElement && root.prototypeController) {
                // Update the frame position
                root.activeFrameElement.y += data.deltaY
                
                // Update the scroll offset
                root.prototypeController.scrollOffsetY = data.scrollOffset
            }
        }
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
                    if (!root.prototypeController.isSimulatingScroll) {
                        root.prototypeController.isSimulatingScroll = true
                        root.prototypeController.scrollOffsetY = 0  // Initialize scroll offset
                    }
                    
                    // Invert the scroll direction to match natural scrolling behavior
                    // When scrolling down (negative angleDelta.y), content should move up (decrease y)
                    var scrollSpeed = 1.0  // Adjust this value to control scroll sensitivity
                    var deltaY = -wheel.angleDelta.y / 8 * scrollSpeed  // Divide by 8 to convert from wheel units to pixels
                    
                    // Calculate the proposed scroll offset
                    var proposedScrollOffset = root.prototypeController.scrollOffsetY + deltaY
                    
                    // Get frame and viewable area dimensions
                    var frameHeight = root.activeFrameElement.height
                    var viewableHeightInCanvas = root.height / root.canvasZoom
                    
                    // Apply constraints to the scroll offset:
                    // 1. When scrolling down (positive deltaY), the frame moves down relative to viewable area
                    //    The top of the frame cannot go below the top of the viewable area (scrollOffset cannot be positive)
                    if (proposedScrollOffset > 0) {
                        proposedScrollOffset = 0
                    }
                    
                    // 2. When scrolling up (negative deltaY), the frame moves up relative to viewable area
                    //    The bottom of the frame cannot go above the bottom of the viewable area
                    var maxNegativeOffset = -(frameHeight - viewableHeightInCanvas)
                    if (proposedScrollOffset < maxNegativeOffset) {
                        proposedScrollOffset = maxNegativeOffset
                    }
                    
                    // Only update if the offset actually changed
                    if (proposedScrollOffset !== root.prototypeController.scrollOffsetY) {
                        // Calculate the actual delta that will be applied
                        var actualDelta = proposedScrollOffset - root.prototypeController.scrollOffsetY
                        
                        // Request throttled update instead of updating immediately
                        scrollThrottle.requestUpdate({
                            deltaY: actualDelta,
                            scrollOffset: proposedScrollOffset
                        })
                    }
                }
            }
            
            onExited: {
                // Reset hover when mouse leaves the viewable area
                if (root.prototypeController) {
                    root.prototypeController.updateHoveredElement(Qt.point(-1, -1))
                }
                
                // Note: We don't reset scroll simulation when mouse leaves
                // Scroll position persists until prototyping stops, viewableArea becomes invisible, or active frame changes
            }
        }
    }
}