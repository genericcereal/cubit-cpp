import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Rectangle {
    id: borderRadiusControl
    width: 10
    height: 10
    radius: 5
    antialiasing: true
    z: 2
    
    property bool allSelectedAreComponentRelated: false
    // Remove this property - now handled by designControls context property
    property real parentWidth: 100
    property real parentHeight: 100
    property var selectedFrame: null
    
    color: allSelectedAreComponentRelated ? Config.componentControlResizeJointColor : Config.controlResizeJointColor
    
    // Drag progress along diagonal (0 = top-left corner, 1 = center)
    property real dragProgress: 0
    
    signal borderRadiusChanged(real newRadius)
    
    // Calculate position based on drag progress
    // Start position: (8, 8)
    // End position: (parent.width/2 - 5, parent.height/2 - 5) - center of controls
    x: 8 + dragProgress * ((parentWidth / 2 - 5) - 8)
    y: 8 + dragProgress * ((parentHeight / 2 - 5) - 8)
    
    // Update drag progress based on frame's current border radius
    function updateDragProgressFromFrame() {
        if (!selectedFrame || selectedFrame.elementType !== "Frame") return
        
        // Calculate progress from border radius
        // Max radius is half the smaller dimension
        var maxRadius = Math.min(selectedFrame.width, selectedFrame.height) / 2
        if (maxRadius > 0) {
            dragProgress = Math.min(1, selectedFrame.borderRadius / maxRadius)
        } else {
            dragProgress = 0
        }
    }
    
    // Update the frame's border radius based on drag progress
    function updateFrameBorderRadius() {
        if (!selectedFrame || selectedFrame.elementType !== "Frame") return
        
        // Calculate the maximum possible radius (half of the smaller dimension)
        var maxRadius = Math.min(selectedFrame.width, selectedFrame.height) / 2
        
        // Set border radius based on drag progress (0 to 1)
        var newRadius = dragProgress * maxRadius
        
        borderRadiusControl.borderRadiusChanged(newRadius)
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: {
            // BorderRadiusControl -> DesignControls
            var designControlsRoot = borderRadiusControl.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        cursorShape: dragging ? Qt.ClosedHandCursor : Qt.PointingHandCursor
        
        property bool dragging: false
        property point dragStartMouse
        property real dragStartProgress
        property real diagonalLength: Math.sqrt(Math.pow((borderRadiusControl.parentWidth / 2 - 5) - 8, 2) + 
                                               Math.pow((borderRadiusControl.parentHeight / 2 - 5) - 8, 2))
        
        onPressed: (mouse) => {
            dragging = true
            dragStartMouse = mapToItem(parent.parent, mouse.x, mouse.y)
            dragStartProgress = borderRadiusControl.dragProgress
            if (parent.canvas && parent.canvas.console) parent.canvas.console.addOutput("BorderRadiusControl drag started")
            mouse.accepted = true
            
            // Update frame border radius immediately when starting drag
            borderRadiusControl.updateFrameBorderRadius()
        }
        
        onReleased: {
            dragging = false
            if (parent.canvas && parent.canvas.console) parent.canvas.console.addOutput("BorderRadiusControl drag ended at progress: " + borderRadiusControl.dragProgress.toFixed(2))
        }
        
        onPositionChanged: (mouse) => {
            if (!dragging) return
            
            // Get current mouse position in parent coordinates
            var currentMouse = mapToItem(parent.parent, mouse.x, mouse.y)
            
            // Calculate the diagonal vector (from top-left to center)
            var diagonalX = (borderRadiusControl.parentWidth / 2 - 5) - 8
            var diagonalY = (borderRadiusControl.parentHeight / 2 - 5) - 8
            
            // Calculate mouse delta
            var deltaX = currentMouse.x - dragStartMouse.x
            var deltaY = currentMouse.y - dragStartMouse.y
            
            // Project the mouse delta onto the diagonal vector
            var dotProduct = deltaX * diagonalX + deltaY * diagonalY
            var diagonalLengthSquared = diagonalX * diagonalX + diagonalY * diagonalY
            
            if (diagonalLengthSquared > 0) {
                var projectedDistance = dotProduct / Math.sqrt(diagonalLengthSquared)
                var progressDelta = projectedDistance / diagonalLength
                
                // Update progress, clamped between 0 and 1
                borderRadiusControl.dragProgress = Math.max(0, Math.min(1, dragStartProgress + progressDelta))
                
                // Update the frame's border radius
                borderRadiusControl.updateFrameBorderRadius()
            }
        }
    }
}