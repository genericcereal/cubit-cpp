import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 80
    height: 30
    radius: 4
    color: Qt.rgba(0, 0, 0, 0.7)
    
    property int frameCount: 0
    property real fps: 0
    
    Label {
        anchors.centerIn: parent
        text: root.fps.toFixed(1) + " FPS"
        color: "#ffffff"
        font.pixelSize: 12
    }
    
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            root.fps = root.frameCount
            root.frameCount = 0
        }
    }
    
    // Count frames
    Timer {
        interval: 16 // ~60 FPS
        running: true
        repeat: true
        onTriggered: root.frameCount++
    }
}