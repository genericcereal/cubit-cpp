import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit
import "components"
import "components/panels"
import "components/viewport-overlay"

SplitView {
    id: root
    orientation: Qt.Horizontal
    
    // The canvas to display - can be passed in or use the active canvas
    property var canvas: Application.activeCanvas

    // Left Panel - Canvas Area
    Item {
        id: canvasContainer
        SplitView.preferredWidth: parent.width * 0.7
        SplitView.minimumWidth: 400

        Loader {
            id: canvasLoader
            anchors.fill: parent
            sourceComponent: {
                if (!root.canvas) return null
                
                switch (root.canvas.viewMode) {
                    case "design":
                    case "variant":
                        return designCanvasComponent
                    case "script":
                        return scriptCanvasComponent
                    default:
                        return designCanvasComponent
                }
            }
            
            property alias canvasView: canvasLoader.item
            
            onLoaded: {
                if (item) {
                    if (root.canvas) {
                        item.controller = root.canvas.controller
                        item.selectionManager = root.canvas.selectionManager
                        item.elementModel = root.canvas.elementModel
                    }
                    // Set viewportControls reference for DesignCanvas
                    if (item.hasOwnProperty("viewportControls")) {
                        // Use Qt.binding to ensure the connection stays alive
                        item.viewportControls = Qt.binding(function() { return viewportOverlay.selectionControls })
                    }
                    
                    // Restore viewport state when switching back to design/variant canvas
                    if ((root.canvas.viewMode === "design" || root.canvas.viewMode === "variant") && 
                        root.canvas.controller) {
                        var savedState = {
                            contentX: root.canvas.controller.savedContentX,
                            contentY: root.canvas.controller.savedContentY,
                            zoom: root.canvas.controller.savedZoom
                        }
                        // Only restore if we have valid saved state
                        if (savedState.zoom > 0) {
                            item.setViewportState(savedState)
                        }
                    }
                }
            }
        }
        
        // Connections to handle viewport state saving
        Connections {
            target: root.canvas || null
            enabled: root.canvas !== null
            
            function onViewportStateShouldBeSaved() {
                // Save current viewport state from design/variant canvas
                if (canvasLoader.item && canvasLoader.item.getViewportState && 
                    root.canvas.controller) {
                    var state = canvasLoader.item.getViewportState()
                    root.canvas.controller.savedContentX = state.contentX
                    root.canvas.controller.savedContentY = state.contentY
                    root.canvas.controller.savedZoom = state.zoom
                }
            }
            
            function onViewModeChanged() {
                // Center the viewport when entering variant mode
                if (!root.canvas) return
                if (root.canvas.viewMode === "variant" && canvasLoader.item && canvasLoader.item.moveToPoint) {
                    // Calculate center point based on selected component or canvas bounds
                    var centerPoint = Qt.point(0, 0)
                    
                    if (root.canvas.selectionManager) {
                        var selectedElements = root.canvas.selectionManager.selectedElements
                        if (selectedElements.length > 0) {
                            // Calculate the center of all selected elements
                            var minX = Number.MAX_VALUE
                            var minY = Number.MAX_VALUE
                            var maxX = Number.MIN_VALUE
                            var maxY = Number.MIN_VALUE
                            
                            for (var i = 0; i < selectedElements.length; i++) {
                                var element = selectedElements[i]
                                if (element && element.isVisual) {
                                    minX = Math.min(minX, element.x)
                                    minY = Math.min(minY, element.y)
                                    maxX = Math.max(maxX, element.x + element.width)
                                    maxY = Math.max(maxY, element.y + element.height)
                                }
                            }
                            
                            if (minX !== Number.MAX_VALUE) {
                                centerPoint = Qt.point(
                                    (minX + maxX) / 2,
                                    (minY + maxY) / 2
                                )
                            }
                        }
                    }
                    
                    // Move to the calculated center point without animation
                    canvasLoader.item.moveToPoint(centerPoint, false)
                }
            }
            
            function onViewportStateShouldBeRestored() {
                // The restoration happens in the Loader.onLoaded handler above
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
            currentMode: root.canvas && root.canvas.controller ? root.canvas.controller.mode : CanvasController.Select
            onModeChanged: (mode) => {
                if (root.canvas && root.canvas.controller) {
                    root.canvas.controller.mode = mode
                }
            }
            onCompileClicked: {
                if (root.canvas && root.canvas.activeScripts) {
                    // Compile the scripts
                    var compiledJson = root.canvas.activeScripts.compile()
                    
                    if (compiledJson) {
                        // Mark as compiled
                        root.canvas.activeScripts.isCompiled = true
                        // Scripts compiled successfully
                    }
                }
            }
            onCreateVariableClicked: {
                if (root.canvas && root.canvas.controller) {
                    root.canvas.controller.createVariable()
                }
            }
            onWebElementsClicked: {
                // Handled internally by ActionsPanel
            }
            
            Connections {
                target: root.canvas ? root.canvas.controller : null
                function onModeChanged() {
                    if (root.canvas && root.canvas.controller) {
                        actionsPanel.currentMode = root.canvas.controller.mode
                    }
                }
            }
        }
        
        // Prototype Panel - positioned relative to canvas container
        PrototypePanel {
            id: prototypePanel
            visible: root.canvas && 
                     root.canvas.prototypeController && 
                     root.canvas.prototypeController.isPrototyping
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 10
            anchors.topMargin: 20
            canvasView: canvasLoader.item
        }
    }

    // Right Panel - Detail Panel
    DetailPanel {
        id: detailPanel
        visible: Application.panels.detailPanelVisible
        SplitView.preferredWidth: parent.width * 0.3
        SplitView.minimumWidth: 250
        elementModel: root.canvas ? root.canvas.elementModel : null
        selectionManager: root.canvas ? root.canvas.selectionManager : null
    }
}