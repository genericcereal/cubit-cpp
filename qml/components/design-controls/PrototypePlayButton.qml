import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../.."

Rectangle {
    id: inlinePrototypePlay
    width: 20
    height: 20
    antialiasing: true
    z: 2
    
    property bool allSelectedAreComponentRelated: false
    // Remove this property - now handled by designControls context property
    property real parentWidth: 100
    property real parentHeight: 100
    property var selectedFrame: null
    property var viewportOverlay: null
    
    color: allSelectedAreComponentRelated ? ConfigObject.componentControlResizeJointColor : ConfigObject.controlResizeJointColor
    
    // Position outside the right control bar
    x: Math.round(parentWidth) + 5  // 5px outside the right edge
    y: Math.round(parentHeight / 2) - 10  // Centered vertically
    
    signal prototypePlayClicked(var selectedFrame)
    
    // Display "P" for Play
    Text {
        anchors.centerIn: parent
        text: "P"
        font.pixelSize: 12
        font.family: "Arial"
        font.bold: true
        color: "white"
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: {
            // PrototypePlayButton -> DesignControls
            var designControlsRoot = inlinePrototypePlay.parent
            return designControlsRoot ? !designControlsRoot.isAnyTextEditing : true
        }
        cursorShape: Qt.PointingHandCursor
        
        onClicked: (mouse) => {
            if (selectedFrame && selectedFrame.elementType === "Frame") {
                inlinePrototypePlay.prototypePlayClicked(selectedFrame)
            }
            mouse.accepted = true
        }
    }
}