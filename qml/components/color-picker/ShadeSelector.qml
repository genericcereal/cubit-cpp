import QtQuick
import QtQuick.Controls
import Cubit.UI 1.0
import "../design-controls"

Item {
    id: root
    
    property real hue: 0 // 0-360
    property real saturation: 1.0 // 0-1
    property real lightness: 0.5 // 0-1
    
    signal shadeUpdated(real saturation, real lightness)
    
    implicitWidth: 200
    implicitHeight: 100
    
    // Throttled update for drag operations
    ThrottledUpdate {
        id: shadeThrottle
        active: mouseArea.pressed
        onUpdate: (data) => {
            root.saturation = data.saturation
            root.lightness = data.lightness
            root.shadeUpdated(data.saturation, data.lightness)
        }
    }
    
    // Background with the base hue color
    Rectangle {
        id: background
        anchors.fill: parent
        anchors.margins: 2
        color: Qt.hsla(root.hue / 360, 1.0, 0.5, 1.0)
    }
    
    // White gradient overlay (left to right)
    Rectangle {
        anchors.fill: background
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#ffffff" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
    
    // Black gradient overlay (top to bottom)
    Rectangle {
        anchors.fill: background
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: "#000000" }
        }
    }
    
    // Border
    Rectangle {
        anchors.fill: background
        color: "transparent"
        border.width: 1
        border.color: "#d0d0d0"
    }
    
    // Selector handle
    Rectangle {
        id: handle
        width: 16
        height: 16
        radius: 8
        x: root.saturation * (background.width - width) + 2
        y: (1 - root.lightness) * (background.height - height) + 2 // Simple linear mapping
        
        color: "transparent"
        border.width: 2
        border.color: "#ffffff"
        
        // Inner circle for visibility
        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: parent.radius - 2
            color: "transparent"
            border.width: 1
            border.color: "#000000"
        }
    }
    
    // Mouse interaction
    MouseArea {
        id: mouseArea
        anchors.fill: background
        
        function updateShade(mouseX, mouseY) {
            // Calculate saturation (0-1) from X position
            var normalizedX = Math.max(0, Math.min(mouseX - 2, background.width))
            var newSaturation = normalizedX / background.width
            
            // Calculate lightness (0-1) from Y position
            // Simple linear mapping: Y=0 is lightness=1 (white), Y=height is lightness=0 (black)
            var normalizedY = Math.max(0, Math.min(mouseY - 2, background.height))
            var newLightness = 1 - (normalizedY / background.height)
            
            // Use throttled update during drag
            shadeThrottle.requestUpdate({
                saturation: newSaturation,
                lightness: newLightness
            })
        }
        
        onPressed: (mouse) => updateShade(mouse.x, mouse.y)
        onPositionChanged: (mouse) => {
            if (pressed) {
                updateShade(mouse.x, mouse.y)
            }
        }
        
        onReleased: {
            // Force final update when drag ends
            shadeThrottle.forceUpdate()
        }
    }
}