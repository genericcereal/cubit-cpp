import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Cubit

ApplicationWindow {
    id: projectWindow
    width: 1280
    height: 800
    visible: true
    title: qsTr("Cubit - %1").arg(canvas ? canvas.name : "Untitled")
    
    property string canvasId: ""
    property var canvas: null
    
    Component.onCompleted: {
        // Get the canvas from Application by ID
        if (canvasId) {
            canvas = Application.getCanvas(canvasId)
            if (!canvas) {
                console.error("Canvas not found:", canvasId)
                close()
            }
        }
    }
    
    // Use CanvasScreen for the window content
    CanvasScreen {
        anchors.fill: parent
        canvas: projectWindow.canvas
    }
    
    // Handle window closing
    onClosing: {
        // Future: Handle unsaved changes, cleanup, etc.
        console.log("Project window closing for canvas:", canvasId)
        
        // Remove the canvas from Application
        if (canvasId) {
            Application.removeCanvas(canvasId)
        }
    }
}