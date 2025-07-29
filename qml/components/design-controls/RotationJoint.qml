import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../.."

Rectangle {
    id: rotationJoint
    width: 20
    height: 20
    antialiasing: true
    z: 1
    visible: true
    
    property int cornerIndex: 0  // 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left
    property bool allSelectedAreComponentRelated: false
    // Remove these properties - now handled by designControls context property
    property real controlRotation: 0
    
    color: allSelectedAreComponentRelated ? ConfigObject.componentControlRotationJointColor : ConfigObject.controlRotationJointColor
    
    signal rotationStarted(var startMousePos, real startAngle)
    signal rotationPositionChanged(var currentMousePos, real currentAngle)
    signal rotationEnded()
    
    MouseArea {
        anchors.fill: parent
        enabled: {
            // RotationJoint -> Repeater -> DesignControls
            var designControlsRoot = rotationJoint.parent?.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        cursorShape: Qt.CrossCursor
        
        property point startMousePos
        property real startAngle
        
        onPressed: (mouse) => {
            // Check if resizing is globally disabled
            // RotationJoint -> Repeater -> DesignControls
            var designControlsItem = parent.parent
            var controller = designControlsItem.getDesignControlsController()
            if (!controller.isResizingEnabled) {
                mouse.accepted = false
                return
            }
            
            // Calculate center of the control in parent coordinates
            var centerPoint = Qt.point(
                parent.x + parent.width / 2,
                parent.y + parent.height / 2
            )
            startMousePos = mapToItem(parent.parent, mouse.x, mouse.y)
            startAngle = Math.atan2(startMousePos.y - centerPoint.y, startMousePos.x - centerPoint.x) * 180 / Math.PI
            
            rotationJoint.rotationStarted(startMousePos, startAngle)
            mouse.accepted = true
        }
        
        onReleased: {
            rotationJoint.rotationEnded()
        }
        
        onPositionChanged: (mouse) => {
            // Calculate center of the control in parent coordinates
            var centerPoint = Qt.point(
                parent.x + parent.width / 2,
                parent.y + parent.height / 2
            )
            var currentMousePos = mapToItem(parent.parent, mouse.x, mouse.y)
            var currentAngle = Math.atan2(currentMousePos.y - centerPoint.y, currentMousePos.x - centerPoint.x) * 180 / Math.PI
            
            rotationJoint.rotationPositionChanged(currentMousePos, currentAngle)
        }
    }
}