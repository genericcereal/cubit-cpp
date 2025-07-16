import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Cubit 1.0
import ".."

Rectangle {
    id: root
    
    property var canvasView: null
    
    width: 250
    height: 100
    color: Config.panelBackground
    border.color: Config.controlsBorderColor
    border.width: 1
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Text {
                text: "Prototype Mode:"
                color: Config.textColor
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter
            }
            
            ComboBox {
                id: prototypingModeDropdown
                Layout.preferredWidth: 100
                Layout.preferredHeight: 30
                
                model: ["web", "ios", "android"]
                currentIndex: {
                    if (Application.activeCanvas && 
                        Application.activeCanvas.prototypeController) {
                        var mode = Application.activeCanvas.prototypeController.prototypeMode
                        if (mode === "ios") return 1
                        if (mode === "android") return 2
                    }
                    return 0
                }
                
                onCurrentTextChanged: {
                    if (Application.activeCanvas && 
                        Application.activeCanvas.prototypeController && 
                        currentText !== Application.activeCanvas.prototypeController.prototypeMode) {
                        Application.activeCanvas.prototypeController.prototypeMode = currentText
                    }
                }
            }
            
            Item {
                Layout.fillWidth: true
            }
        }
        
        // Stop button
        Button {
            id: stopButton
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            text: "Stop Prototype"
            
            onClicked: {
                if (!Application.activeCanvas || !Application.activeCanvas.prototypeController) {
                    return
                }
                
                if (!root.canvasView) {
                    return
                }
                
                var prototypeController = Application.activeCanvas.prototypeController
                
                // Get the snapshot values from prototype controller BEFORE stopping
                var startPos = prototypeController.getSnapshotCanvasPosition()
                var startZoom = prototypeController.getSnapshotCanvasZoom()
                
                // First restore all element positions (without animation)
                prototypeController.restoreElementPositionsFromSnapshot()
                
                // Stop prototyping mode through the controller BEFORE animating
                prototypeController.stopPrototyping()
                
                // Then animate back to the canvas position if we have one
                if (startPos !== null && startPos !== undefined) {
                    // First restore the zoom level
                    root.canvasView.zoom = startZoom
                    
                    // Then move to the original position without animation
                    // The moveToPoint function will set isAnimating = false
                    root.canvasView.moveToPoint(startPos, false)
                }
            }
        }
    }
}