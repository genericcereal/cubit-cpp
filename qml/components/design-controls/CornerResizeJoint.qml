import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../.."

Item {
    id: resizeJointContainer
    width: 10
    height: 10
    z: 2
    
    property int cornerIndex: 0  // 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left
    property bool allSelectedAreComponentRelated: false
    // Remove these properties - now handled by designControls context property
    property real controlRotation: 0
    property bool showResizeJoints: true
    property bool dragging: false
    property string dragMode: ""
    property bool flexWidthFitContent: false
    property bool flexHeightFitContent: false
    
    signal resizeStarted(int cornerIdx, var anchorCorner, var adjacentCorner1, var adjacentCorner2)
    signal resizePositionChanged(var mouseInParent)
    signal resizeEnded()
    
    // Helper function to get rotation-adjusted corner cursor
    function getCornerCursor(cornerIndex) {
        // Adjust corner index based on rotation
        var rotationSteps = Math.round(controlRotation / 90) % 4
        if (rotationSteps < 0) rotationSteps += 4
        var adjustedIndex = (cornerIndex + rotationSteps) % 4
        
        // Diagonal cursors alternate based on adjusted corner
        return (adjustedIndex === 0 || adjustedIndex === 2) ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor
    }
    
    // Visual representation of the resize joint
    Rectangle {
        id: resizeJoint
        anchors.fill: parent
        color: allSelectedAreComponentRelated ? ConfigObject.componentControlResizeJointColor : ConfigObject.controlResizeJointColor
        antialiasing: true
        visible: resizeJointContainer.showResizeJoints && !(resizeJointContainer.dragging && resizeJointContainer.dragMode.startsWith("resize-corner-"))
        
        // Decorative circle inside the resize joint
        Rectangle {
            anchors.centerIn: parent
            width: 10
            height: 10
            radius: 5
            color: ConfigObject.controlJointCircleFill
            border.color: allSelectedAreComponentRelated ? ConfigObject.componentControlJointCircleBorder : ConfigObject.controlJointCircleBorder
            border.width: 1
            antialiasing: true
        }
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: {
            // CornerResizeJoint -> Item -> Repeater -> DesignControls
            var designControlsRoot = resizeJointContainer.parent?.parent?.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        cursorShape: {
            if (flexWidthFitContent && !flexHeightFitContent) {
                // Width fit content - vertical resize only
                return Qt.SizeVerCursor
            } else if (flexHeightFitContent && !flexWidthFitContent) {
                // Height fit content - horizontal resize only
                return Qt.SizeHorCursor
            } else {
                // Normal corner resize
                return resizeJointContainer.getCornerCursor(resizeJointContainer.cornerIndex)
            }
        }
        
        // When in fit content mode, don't handle events - let them pass through to the bar
        propagateComposedEvents: flexWidthFitContent || flexHeightFitContent
        
        property bool shouldPropagateEvents: flexWidthFitContent || flexHeightFitContent
        
        property int cornerIdx: resizeJointContainer.cornerIndex
        property point anchorCorner  // Opposite corner in parent coords
        property point adjacentCorner1  // Adjacent corner 1 in parent coords
        property point adjacentCorner2  // Adjacent corner 2 in parent coords
        
        onPressed: (mouse) => {
            // Check if resizing is globally disabled
            // CornerResizeJoint -> Item -> Repeater -> DesignControls
            var designControlsItem = parent.parent.parent
            var controller = designControlsItem.getDesignControlsController()
            if (!controller.isResizingEnabled) {
                mouse.accepted = false
                return
            }
            
            // In fit content mode, don't handle the event
            if (shouldPropagateEvents) {
                mouse.accepted = false
                return
            }
            
            // Get the corners of the control in local space
            var corners = [
                Qt.point(0, 0),           // top-left
                Qt.point(parent.parent.width, 0),  // top-right
                Qt.point(parent.parent.width, parent.parent.height), // bottom-right
                Qt.point(0, parent.parent.height)  // bottom-left
            ]
            
            // Map corners to parent coordinates
            var parentCorners = []
            for (var i = 0; i < 4; i++) {
                parentCorners.push(parent.parent.mapToItem(parent.parent.parent, corners[i].x, corners[i].y))
            }
            
            // Store the anchor corner (opposite) and adjacent corners
            switch(cornerIdx) {
                case 0: // dragging top-left
                    anchorCorner = parentCorners[2]     // bottom-right is anchor
                    adjacentCorner1 = parentCorners[1]  // top-right
                    adjacentCorner2 = parentCorners[3]  // bottom-left
                    break
                case 1: // dragging top-right
                    anchorCorner = parentCorners[3]     // bottom-left is anchor
                    adjacentCorner1 = parentCorners[0]  // top-left
                    adjacentCorner2 = parentCorners[2]  // bottom-right
                    break
                case 2: // dragging bottom-right
                    anchorCorner = parentCorners[0]     // top-left is anchor
                    adjacentCorner1 = parentCorners[1]  // top-right
                    adjacentCorner2 = parentCorners[3]  // bottom-left
                    break
                case 3: // dragging bottom-left
                    anchorCorner = parentCorners[1]     // top-right is anchor
                    adjacentCorner1 = parentCorners[0]  // top-left
                    adjacentCorner2 = parentCorners[2]  // bottom-right
                    break
            }
            
            resizeJointContainer.resizeStarted(cornerIdx, anchorCorner, adjacentCorner1, adjacentCorner2)
            mouse.accepted = true
        }
        
        onReleased: (mouse) => {
            if (shouldPropagateEvents) {
                mouse.accepted = false
                return
            }
            
            resizeJointContainer.resizeEnded()
        }
        
        onPositionChanged: (mouse) => {
            if (shouldPropagateEvents) {
                mouse.accepted = false
                return
            }
            
            // Get mouse position in parent coordinates
            var mouseInParent = mapToItem(parent.parent.parent, mouse.x, mouse.y)
            resizeJointContainer.resizePositionChanged(mouseInParent)
        }
    }
}