import QtQuick
import QtQuick.Controls
import Cubit 1.0
import Cubit.UI 1.0

Rectangle {
    id: controlSurface
    
    color: Config.controlInnerRectColor
    antialiasing: true
    
    property alias mouseArea: surfaceMouseArea
    // Remove this property - now handled by designControls context property
    property bool dragging: false
    property string dragMode: ""
    property point dragStartPoint
    property point dragStartControlPos
    property alias isDragThresholdExceeded: surfaceMouseArea.isDragThresholdExceeded
    
    signal pressed(var mouse)
    signal released(var mouse)
    signal positionChanged(var mouse)
    signal doubleClicked(var mouse)
    signal clickedOnElement(var element)
    
    MouseArea {
        id: surfaceMouseArea
        anchors.fill: parent
        cursorShape: dragging ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        propagateComposedEvents: true
        
        property bool isDragThresholdExceeded: false
        property real dragThreshold: 3 // pixels
        
        // Disable mouse handling when text is being edited
        // Get the DesignControls root item
        enabled: {
            var designControlsRoot = controlSurface.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        
        onPressed: (mouse) => {
            isDragThresholdExceeded = false
            mouse.accepted = true
            controlSurface.pressed(mouse)
        }
        
        onReleased: (mouse) => {
            controlSurface.released(mouse)
        }
        
        onDoubleClicked: (mouse) => {
            controlSurface.doubleClicked(mouse)
        }
        
        onPositionChanged: (mouse) => {
            controlSurface.positionChanged(mouse)
        }
    }
}