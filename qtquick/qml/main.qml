import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Cubit 1.0
import "panels"
import "components"

Window {
    id: mainWindow
    visible: true
    width: 1280
    height: 720
    title: qsTr("Cubit")

    // Controllers
    CanvasController {
        id: canvasController
        onModeChanged: {
            console.log("Main.qml: CanvasController mode changed to:", mode)
        }
    }

    SelectionManager {
        id: selectionManager
    }

    ElementModel {
        id: elementModel
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left Panel - Canvas Area
        Item {
            id: canvasContainer
            SplitView.preferredWidth: parent.width * 0.7
            SplitView.minimumWidth: 400

            CanvasView {
                id: canvasView
                anchors.fill: parent
                controller: canvasController
                selectionManager: selectionManager
                elementModel: elementModel
            }
            
            // Viewport overlay for non-scaling UI elements
            ViewportOverlay {
                anchors.fill: parent
                canvasView: canvasView
                hoveredElement: canvasView.hoveredElement
            }

            // Overlay panels
            ActionsPanel {
                id: actionsPanel
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                currentMode: canvasController.mode
                onModeChanged: (mode) => canvasController.setMode(mode)
                
                Connections {
                    target: canvasController
                    function onModeChanged() {
                        console.log("ActionsPanel Connections: mode changed to", canvasController.mode)
                        actionsPanel.currentMode = canvasController.mode
                    }
                }
            }

            FPSMonitor {
                id: fpsMonitor
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
            }
        }

        // Right Panel - Detail Panel
        DetailPanel {
            id: detailPanel
            SplitView.preferredWidth: parent.width * 0.3
            SplitView.minimumWidth: 250
            elementModel: elementModel
            selectionManager: selectionManager
        }
    }
}