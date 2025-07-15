import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Rectangle {
    id: edgeBar
    
    property int edgeIndex: 0  // 0=top, 1=right, 2=bottom, 3=left
    property bool allSelectedAreComponentRelated: false
    // Remove these properties - now handled by designControls context property
    property real controlRotation: 0
    property bool flexHeightFitContent: false
    property bool flexWidthFitContent: false
    
    color: allSelectedAreComponentRelated ? Config.componentControlBarColor : Config.controlBarColor
    antialiasing: true
    
    signal resizeStarted(int edgeIndex, var anchorEdgePoint1, var anchorEdgePoint2, var draggedEdgePoint1, var draggedEdgePoint2)
    signal resizePositionChanged(var mouseInParent)
    signal resizeEnded()
    
    // Helper function to get rotation-adjusted cursor
    function getEdgeCursor(edgeIndex) {
        // Adjust edge index based on rotation (90 degree increments)
        var rotationSteps = Math.round(controlRotation / 90) % 4
        if (rotationSteps < 0) rotationSteps += 4
        var adjustedIndex = (edgeIndex + rotationSteps) % 4
        
        // Return cursor based on adjusted edge
        return (adjustedIndex % 2 === 0) ? Qt.SizeVerCursor : Qt.SizeHorCursor
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: !designControls.isAnyTextEditing
        cursorShape: {
            // Check if resize is allowed for this edge
            var resizeAllowed = true
            if (edgeIndex === 0 || edgeIndex === 2) {
                // Horizontal bars (top/bottom) - check height resize
                resizeAllowed = !flexHeightFitContent
            } else {
                // Vertical bars (left/right) - check width resize
                resizeAllowed = !flexWidthFitContent
            }
            return resizeAllowed ? edgeBar.getEdgeCursor(edgeIndex) : Qt.ArrowCursor
        }
        
        property point anchorEdgePoint1  // First point of anchor edge in parent coords
        property point anchorEdgePoint2  // Second point of anchor edge in parent coords
        property point draggedEdgePoint1 // First point of dragged edge in parent coords
        property point draggedEdgePoint2 // Second point of dragged edge in parent coords
        
        onPressed: (mouse) => {
            // Check if resizing is globally disabled
            if (!designControls.isResizingEnabled) {
                mouse.accepted = false
                return
            }
            
            // Check if resize is allowed for this edge
            var resizeAllowed = true
            if (edgeIndex === 0 || edgeIndex === 2) {
                // Horizontal bars (top/bottom) - check height resize
                resizeAllowed = !flexHeightFitContent
            } else {
                // Vertical bars (left/right) - check width resize
                resizeAllowed = !flexWidthFitContent
            }
            
            if (!resizeAllowed) {
                mouse.accepted = false
                return
            }
            
            // Get the corners of the control in local space (same as CornerResizeJoint)
            var corners = [
                Qt.point(0, 0),           // top-left
                Qt.point(parent.parent.width, 0),  // top-right
                Qt.point(parent.parent.width, parent.parent.height), // bottom-right
                Qt.point(0, parent.parent.height)  // bottom-left
            ]
            
            // Map corners to parent coordinates (same as CornerResizeJoint)
            var parentCorners = []
            for (var i = 0; i < 4; i++) {
                parentCorners.push(parent.parent.mapToItem(parent.parent.parent, corners[i].x, corners[i].y))
            }
            
            // Store both the anchor edge and dragged edge positions
            switch(edgeIndex) {
                case 0: // dragging top edge
                    anchorEdgePoint1 = parentCorners[3] // bottom-left
                    anchorEdgePoint2 = parentCorners[2] // bottom-right
                    draggedEdgePoint1 = parentCorners[0] // top-left
                    draggedEdgePoint2 = parentCorners[1] // top-right
                    break
                case 1: // dragging right edge
                    anchorEdgePoint1 = parentCorners[0] // top-left
                    anchorEdgePoint2 = parentCorners[3] // bottom-left
                    draggedEdgePoint1 = parentCorners[1] // top-right
                    draggedEdgePoint2 = parentCorners[2] // bottom-right
                    break
                case 2: // dragging bottom edge
                    anchorEdgePoint1 = parentCorners[0] // top-left
                    anchorEdgePoint2 = parentCorners[1] // top-right
                    draggedEdgePoint1 = parentCorners[3] // bottom-left
                    draggedEdgePoint2 = parentCorners[2] // bottom-right
                    break
                case 3: // dragging left edge
                    anchorEdgePoint1 = parentCorners[1] // top-right
                    anchorEdgePoint2 = parentCorners[2] // bottom-right
                    draggedEdgePoint1 = parentCorners[0] // top-left
                    draggedEdgePoint2 = parentCorners[3] // bottom-left
                    break
            }
            
            edgeBar.resizeStarted(edgeIndex, anchorEdgePoint1, anchorEdgePoint2, draggedEdgePoint1, draggedEdgePoint2)
            mouse.accepted = true
        }
        
        onReleased: {
            edgeBar.resizeEnded()
        }
        
        onPositionChanged: (mouse) => {
            // Get mouse position in parent coordinates (same as CornerResizeJoint)
            var mouseInParent = mapToItem(parent.parent.parent, mouse.x, mouse.y)
            edgeBar.resizePositionChanged(mouseInParent)
        }
    }
}