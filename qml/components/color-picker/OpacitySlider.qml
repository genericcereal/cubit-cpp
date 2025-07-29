import QtQuick
import QtQuick.Controls
import "../design-controls"

Item {
    id: root
    
    property color color: "#000000" // The color to show opacity for
    property real alpha: 1.0 // 0-1
    
    signal alphaUpdated(real newAlpha)
    
    implicitWidth: 200
    implicitHeight: 20  // Same as HueSlider
    
    // Throttled update for drag operations
    ThrottledUpdate {
        id: alphaThrottle
        active: mouseArea.pressed
        onUpdate: (data) => {
            root.alpha = data.alpha
            root.alphaUpdated(data.alpha)
        }
    }
    
    // Checkered pattern background
    Rectangle {
        id: checkerBackground
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 10
        radius: height / 2
        clip: true
        
        // White background
        color: "#ffffff"
        
        // Checkered pattern using a repeating grid
        Row {
            anchors.fill: parent
            Repeater {
                model: Math.ceil(checkerBackground.width / 5) // 5px squares
                Row {
                    Repeater {
                        model: 2
                        Rectangle {
                            width: 5
                            height: 5
                            color: (index + modelData) % 2 === 0 ? "#cccccc" : "#ffffff"
                            y: modelData * 5
                        }
                    }
                }
            }
        }
    }
    
    // Gradient overlay from transparent to opaque color
    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 10
        radius: height / 2
        
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0) }
            GradientStop { position: 1.0; color: Qt.rgba(root.color.r, root.color.g, root.color.b, 1) }
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
        x: root.alpha * (track.width - width)
        y: (parent.height - height) / 2  // Center vertically
        
        color: Qt.rgba(root.color.r, root.color.g, root.color.b, root.alpha)
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
        id: mouseArea
        anchors.fill: track
        
        function updateAlpha(mouseX) {
            var normalizedX = Math.max(0, Math.min(mouseX, track.width))
            var newAlpha = normalizedX / track.width
            
            // Use throttled update during drag
            alphaThrottle.requestUpdate({
                alpha: newAlpha
            })
        }
        
        onPressed: (mouse) => updateAlpha(mouse.x)
        onPositionChanged: (mouse) => {
            if (pressed) {
                updateAlpha(mouse.x)
            }
        }
        
        onReleased: {
            // Force final update when drag ends
            alphaThrottle.forceUpdate()
        }
    }
}