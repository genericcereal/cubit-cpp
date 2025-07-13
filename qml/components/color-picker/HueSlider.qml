import QtQuick
import QtQuick.Controls

Item {
    id: root
    
    property real hue: 0 // 0-360
    property real saturation: 1.0 // 0-1
    property real lightness: 0.5 // 0-1
    
    signal hueUpdated(real newHue)
    
    implicitWidth: 200
    implicitHeight: 20  // Height to accommodate the handle
    
    // Hue gradient background
    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 10
        radius: height / 2
        
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.hsla(0, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 0.167; color: Qt.hsla(60/360, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 0.333; color: Qt.hsla(120/360, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 0.5; color: Qt.hsla(180/360, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 0.667; color: Qt.hsla(240/360, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 0.833; color: Qt.hsla(300/360, root.saturation, root.lightness, 1.0) }
            GradientStop { position: 1.0; color: Qt.hsla(0, root.saturation, root.lightness, 1.0) }
        }
        
        // Border
        border.width: 1
        border.color: "#d0d0d0"
    }
    
    // Slider handle
    Rectangle {
        id: handle
        width: 20
        height: 20
        radius: height / 2
        x: (root.hue / 360) * (track.width - width)
        y: (parent.height - height) / 2  // Center vertically on the track
        
        color: Qt.hsla(root.hue / 360, root.saturation, root.lightness, 1.0)
        border.width: 2
        border.color: "#ffffff"
        
        // Drop shadow effect
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: "#40000000"
            z: -1
        }
    }
    
    // Mouse interaction
    MouseArea {
        anchors.fill: track
        
        function updateHue(mouseX) {
            var normalizedX = Math.max(0, Math.min(mouseX, track.width))
            var newHue = (normalizedX / track.width) * 360
            root.hue = newHue
            root.hueUpdated(newHue)
        }
        
        onPressed: (mouse) => updateHue(mouse.x)
        onPositionChanged: (mouse) => {
            if (pressed) {
                updateHue(mouse.x)
            }
        }
    }
}