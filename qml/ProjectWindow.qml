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
    
    onCanvasIdChanged: {
        // Canvas ID changed
        // Update canvas when canvasId changes
        if (canvasId) {
            canvas = Application.getProject(canvasId)
            // Canvas updated
            
            // Update the controller's project when canvas changes
            if (canvas && designControlsController) {
                designControlsController.project = canvas
            }
        }
    }
    
    // Create window-specific panels instance
    property var windowPanels: Panels {}
    
    // Window-specific DesignControlsController
    property var designControlsController: DesignControlsController {
        project: canvas
        
        Component.onCompleted: {
            // Design controls controller initialized
        }
    }
    
    Component.onCompleted: {
        // Project window initialized
        
        // Test if we can create a controller dynamically
        try {
            var testController = Qt.createQmlObject('import Cubit 1.0; DesignControlsController {}', projectWindow)
            // Test controller created
            testController.destroy()
        } catch (error) {
            // Error creating test controller
        }
        
        // Canvas will be set by onCanvasIdChanged
    }
    
    // Use CanvasScreen for the window content
    CanvasScreen {
        id: canvasScreen
        anchors.fill: parent
        canvas: projectWindow.canvas
        panels: projectWindow.windowPanels
        designControlsController: projectWindow.designControlsController
        
        Component.onCompleted: {
            // Canvas screen initialized
        }
    }
    
    // Handle window closing
    onClosing: {
        // Future: Handle unsaved changes, cleanup, etc.
        // Project window closing
        
        // Remove the canvas from Application
        if (canvasId) {
            Application.removeProject(canvasId)
        }
    }
}