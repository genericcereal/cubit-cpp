import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit
import "components"
import "components/panels"
import "components/viewport-overlay"
import "components/color-picker"

Item {
    id: root
    
    // The canvas to display - must be passed in by parent
    property var canvas: null
    // The panels object to use - must be passed in by parent
    property var panels: null
    // The design controls controller to use - must be passed in by parent
    property var designControlsController: null
    
    onCanvasChanged: {
        // Canvas changed
    }
    
    onDesignControlsControllerChanged: {
        // Design controls controller changed
    }
    
    Component.onCompleted: {
        // CanvasScreen initialized
    }

    // Main split view for canvas and detail panel
    SplitView {
        id: splitView
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
                        // Set canvas on loaded item
                        item.canvas = root.canvas
                    } else {
                        // No canvas available
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
            designControlsController: root.designControlsController
        }

        // Overlay panels
        ActionsPanel {
            id: actionsPanel
            visible: root.panels ? root.panels.actionsPanelVisible : false
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 20
            canvas: root.canvas
            currentMode: root.canvas && root.canvas.controller ? root.canvas.controller.mode : CanvasController.Select
            onModeChanged: (mode) => {
                // Actions panel mode changed
                if (root.canvas && root.canvas.controller) {
                    root.canvas.controller.mode = mode
                } else {
                    // Warning: Cannot set mode
                }
            }
            onCompileClicked: {
                if (root.canvas && root.canvas.controller) {
                    // Execute compile command through command history for undo/redo support
                    root.canvas.controller.compileScripts()
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
            canvas: root.canvas
        }
    }

        // Right Panel - Detail Panel
        DetailPanel {
            id: detailPanel
            visible: root.panels ? root.panels.detailPanelVisible : false
            SplitView.preferredWidth: parent.width * 0.3
            SplitView.minimumWidth: 250
            canvas: root.canvas
            elementModel: root.canvas ? root.canvas.elementModel : null
            selectionManager: root.canvas ? root.canvas.selectionManager : null
        }
    } // End of SplitView
    
    // Create an overlay item to ensure the panel appears on top
    Item {
        id: popoverOverlay
        anchors.fill: parent
        z: 1000 // Above everything
        
        // Property selector panel that appears on top of content
        PropertyPopoverPanel {
            id: selectorPanel
            visible: false
            
            property var anchorSelector: null
            property real offsetY: 0
            property string currentType: ""
            property bool manuallyPositioned: false
            
            // Position will be set dynamically based on where it was triggered
            x: 0
            y: 0
            
            onCloseRequested: {
                selectorPanel.visible = false
                selectorPanel.anchorSelector = null
                selectorPanel.currentType = ""
                selectorPanel.manuallyPositioned = false
            }
            
            // Dynamic content based on type
            contentComponent: {
                switch (selectorPanel.currentType) {
                    case "fill":
                    case "edgeColor":
                    case "fillColor":
                        return fillComponent
                    case "style":
                        return styleComponent
                    case "font":
                        return fontComponent
                    default:
                        return null
                }
            }
        }
    }
    
    // Component for Fill selector
    Component {
        id: fillComponent
        ColorPicker {
            property var selectedElement: root.canvas && root.canvas.selectionManager 
                ? root.canvas.selectionManager.selectedElements[0] : null
            
            // Get the current color based on the popover type
            property var currentColor: {
                if (!selectedElement) return Qt.rgba(0.8, 0.8, 0.8, 1)
                
                switch (selectorPanel.currentType) {
                    case "fill":
                        if (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance") {
                            return selectedElement.fill || Qt.rgba(0.8, 0.8, 0.8, 1)
                        }
                        break
                    case "edgeColor":
                        if (selectedElement.elementType === "Shape") {
                            return selectedElement.edgeColor || Qt.rgba(0, 0, 0, 1)
                        }
                        break
                    case "fillColor":
                        if (selectedElement.elementType === "Shape") {
                            return selectedElement.fillColor || Qt.rgba(1, 1, 1, 1)
                        }
                        break
                }
                return Qt.rgba(0.8, 0.8, 0.8, 1)
            }
            
            Component.onCompleted: {
                // Set initial values to avoid binding loops
                if (currentColor) {
                    hue = currentColor.hslHue * 360  // Qt returns hue as 0-1, convert to 0-360
                    saturation = currentColor.hslSaturation
                    lightness = currentColor.hslLightness
                    alphaValue = currentColor.a  // Alpha channel
                }
            }
            
            onCurrentColorChanged: {
                // Update values when color changes to avoid binding loops
                if (currentColor) {
                    hue = currentColor.hslHue * 360
                    saturation = currentColor.hslSaturation
                    lightness = currentColor.hslLightness
                    alphaValue = currentColor.a
                }
            }
            
            colorFormat: {
                if (selectedElement && selectedElement.colorFormat !== undefined) {
                    return selectedElement.colorFormat
                }
                return 1  // Default to HEX
            }
            
            fillType: 0  // Default to Solid for now
            
            onColorChanged: function(newColor) {
                if (!selectedElement) return
                
                switch (selectorPanel.currentType) {
                    case "fill":
                        if (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance") {
                            selectedElement.fill = newColor
                        }
                        break
                    case "edgeColor":
                        if (selectedElement.elementType === "Shape") {
                            selectedElement.edgeColor = newColor
                        }
                        break
                    case "fillColor":
                        if (selectedElement.elementType === "Shape") {
                            selectedElement.fillColor = newColor
                        }
                        break
                }
            }
            
            onColorFormatChanged: function(newFormat) {
                if (selectedElement && selectedElement.colorFormat !== undefined) {
                    selectedElement.colorFormat = newFormat
                }
            }
            
            onFillTypeChanged: function(newType) {
                // No functionality for now
            }
            
            onEyeButtonClicked: {
                // No functionality for now
            }
        }
    }
    
    // Component for Style selector
    Component {
        id: styleComponent
        Text {
            text: "Style options coming soon"
            anchors.centerIn: parent
            font.pixelSize: 14
            color: "#666666"
        }
    }
    
    // Component for Font selector
    Component {
        id: fontComponent
        FontPicker {
            id: fontPickerInstance
            property var selectedElement: root.canvas && root.canvas.selectionManager 
                ? root.canvas.selectionManager.selectedElements[0] : null
            
            currentFontFamily: selectedElement && selectedElement.font ? selectedElement.font.family : ""
            
            onFontSelected: function(fontFamily) {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.family = fontFamily
                    selectedElement.font = newFont
                }
                // Close the popover panel
                selectorPanel.visible = false
            }
        }
    }
    
    // Connect properties panel to selector panel
    Connections {
        target: detailPanel.propertiesPanel
        function onPanelSelectorClicked(selector, type) {
            // Panel selector clicked
            
            // Position the panel to the left of the detail panel
            var detailPanelPos = detailPanel.mapToItem(popoverOverlay, 0, 0)
            // Position selector panel
            
            // Map selector position to get vertical alignment
            var selectorPos = selector.mapToItem(popoverOverlay, 0, 0)
            
            // Position to the left of the detail panel with some margin
            selectorPanel.x = detailPanelPos.x - selectorPanel.width - 10
            
            // Set type and make visible first so height is calculated
            selectorPanel.currentType = type
            selectorPanel.anchorSelector = selector
            selectorPanel.manuallyPositioned = false
            selectorPanel.visible = true
            
            // Use Qt.callLater to ensure the panel's height is calculated after becoming visible
            Qt.callLater(function() {
                
                // Align the middle of the popover with the middle of the selector
                var selectorMiddleY = selectorPos.y + (selector.height / 2)
                selectorPanel.y = selectorMiddleY - (selectorPanel.height / 2)
                
                
                // Ensure the panel stays within bounds
                if (selectorPanel.x < 0) {
                    // If not enough space on the left, position inside the detail panel
                    selectorPanel.x = detailPanelPos.x + 10
                }
                
                var maxY = popoverOverlay.height - selectorPanel.height
                if (selectorPanel.y > maxY) {
                    selectorPanel.y = maxY
                }
                if (selectorPanel.y < 0) {
                    selectorPanel.y = 0
                }
                
            })
        }
    }
    
    // Close panel when selection changes
    Connections {
        target: root.canvas ? root.canvas.selectionManager : null
        function onSelectionChanged() {
            if (selectorPanel.visible) {
                selectorPanel.visible = false
            }
        }
    }
    
    // Update panel position when detail panel moves or resizes (unless manually positioned)
    Connections {
        target: detailPanel
        function onWidthChanged() {
            updatePopoverPosition()
        }
        function onXChanged() {
            updatePopoverPosition()
        }
    }
    
    function updatePopoverPosition() {
        if (selectorPanel.visible && !selectorPanel.manuallyPositioned && selectorPanel.anchorSelector) {
            // Recalculate position
            var detailPanelPos = detailPanel.mapToItem(popoverOverlay, 0, 0)
            var selectorPos = selectorPanel.anchorSelector.mapToItem(popoverOverlay, 0, 0)
            
            // Position to the left of detail panel
            selectorPanel.x = detailPanelPos.x - selectorPanel.width - 10
            
            // Ensure it stays within bounds
            if (selectorPanel.x < 0) {
                selectorPanel.x = detailPanelPos.x + 10
            }
            
            // Update vertical position to stay aligned with selector
            var selectorMiddleY = selectorPos.y + (selectorPanel.anchorSelector.height / 2)
            selectorPanel.y = selectorMiddleY - (selectorPanel.height / 2)
            
            // Ensure vertical bounds
            var maxY = popoverOverlay.height - selectorPanel.height
            if (selectorPanel.y > maxY) {
                selectorPanel.y = maxY
            }
            if (selectorPanel.y < 0) {
                selectorPanel.y = 0
            }
        }
    }
}