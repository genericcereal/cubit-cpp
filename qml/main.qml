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

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left Panel - Canvas Area
        Item {
            id: canvasContainer
            SplitView.preferredWidth: parent.width * 0.7
            SplitView.minimumWidth: 400

            Loader {
                id: canvasLoader
                anchors.fill: parent
                sourceComponent: {
                    if (!Application.activeCanvas) return null
                    
                    switch (Application.activeCanvas.viewMode) {
                        case "design":
                            return designCanvasComponent
                        case "script":
                            return scriptCanvasComponent
                        case "variant":
                            return variantCanvasComponent
                        default:
                            return designCanvasComponent
                    }
                }
                
                property alias canvasView: canvasLoader.item
                
                onLoaded: {
                    if (item) {
                        if (Application.activeCanvas) {
                            item.controller = Application.activeCanvas.controller
                            item.selectionManager = Application.activeCanvas.selectionManager
                            item.elementModel = Application.activeCanvas.elementModel
                        }
                        // Set viewportControls reference for DesignCanvas and VariantCanvas
                        if (item.hasOwnProperty("viewportControls")) {
                            item.viewportControls = viewportOverlay.selectionControls
                        }
                    }
                }
            }
            
            Component {
                id: designCanvasComponent
                DesignCanvas {
                    // Properties will be set by Loader.onLoaded
                }
            }
            
            Component {
                id: scriptCanvasComponent
                ScriptCanvas {
                    // Properties will be set by Loader.onLoaded
                }
            }
            
            Component {
                id: variantCanvasComponent
                VariantCanvas {
                    // Properties will be set by Loader.onLoaded
                }
            }
            
            // Viewport overlay for non-scaling UI elements
            ViewportOverlay {
                id: viewportOverlay
                anchors.fill: parent
                canvasView: canvasLoader.item
                hoveredElement: canvasLoader.item?.hoveredElement ?? null
            }

            // Overlay panels
            ActionsPanel {
                id: actionsPanel
                visible: Application.panels.actionsPanelVisible
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                currentMode: Application.activeCanvas && Application.activeCanvas.controller ? Application.activeCanvas.controller.mode : CanvasController.Select
                onModeChanged: (mode) => {
                    if (Application.activeCanvas && Application.activeCanvas.controller) {
                        Application.activeCanvas.controller.mode = mode
                    }
                }
                onCompileClicked: {
                    if (Application.activeCanvas && Application.activeCanvas.activeScripts) {
                        // Compile the scripts
                        var compiledJson = Application.activeCanvas.activeScripts.compile()
                        
                        if (compiledJson) {
                            // Mark as compiled
                            Application.activeCanvas.activeScripts.isCompiled = true
                            console.log("Scripts compiled successfully")
                        }
                    }
                }
                onCreateVariableClicked: {
                    if (Application.activeCanvas && Application.activeCanvas.controller) {
                        Application.activeCanvas.controller.createVariable()
                    }
                }
                
                Connections {
                    target: Application.activeCanvas ? Application.activeCanvas.controller : null
                    function onModeChanged() {
                        if (Application.activeCanvas && Application.activeCanvas.controller) {
                            console.log("ActionsPanel Connections: mode changed to", Application.activeCanvas.controller.mode)
                            actionsPanel.currentMode = Application.activeCanvas.controller.mode
                        }
                    }
                }
            }

            FPSMonitor {
                id: fpsMonitor
                visible: Application.panels.fpsMonitorVisible
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
            }
        }

        // Right Panel - Detail Panel
        DetailPanel {
            id: detailPanel
            visible: Application.panels.detailPanelVisible
            SplitView.preferredWidth: parent.width * 0.3
            SplitView.minimumWidth: 250
            elementModel: Application.activeCanvas ? Application.activeCanvas.elementModel : null
            selectionManager: Application.activeCanvas ? Application.activeCanvas.selectionManager : null
            
        }
    }
}