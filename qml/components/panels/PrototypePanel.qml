import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Cubit 1.0
import Cubit.UI 1.0
import ".."

Rectangle {
    id: root
    
    property var canvasView: null
    property var canvas: null
    
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
                enabled: false  // Disabled - mode is dictated by the platform of the launching element
                
                model: Config.platformOptions
                currentIndex: {
                    if (root.canvas && 
                        root.canvas.prototypeController) {
                        var mode = root.canvas.prototypeController.prototypeMode.toLowerCase()
                        if (mode === "ios") return 1
                        if (mode === "android") return 2
                    }
                    return 0
                }
                
                // Removed onCurrentTextChanged handler since dropdown is disabled
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
                if (!root.canvas || !root.canvas.prototypeController) {
                    return
                }
                
                if (!root.canvasView) {
                    return
                }
                
                var prototypeController = root.canvas.prototypeController
                
                // Canvas restoration is disabled for now
                // var startPos = prototypeController.getSnapshotCanvasPosition()
                // var startZoom = prototypeController.getSnapshotCanvasZoom()
                
                // Restore element positions to their original state (including scroll positions)
                prototypeController.restoreElementPositionsFromSnapshot()
                
                // Stop prototyping mode through the controller
                prototypeController.stopPrototyping()
                
                // Canvas position restoration is disabled for now
                // if (startPos !== null && startPos !== undefined) {
                //     // First restore the zoom level
                //     root.canvasView.zoom = startZoom
                //     
                //     // Then move to the original position without animation
                //     // The moveToPoint function will set isAnimating = false
                //     root.canvasView.moveToPoint(startPos, false)
                // }
            }
        }
    }
}