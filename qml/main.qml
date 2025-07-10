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
                        if (Application.activeCanvas) {
                            item.controller = Application.activeCanvas.controller
                            item.selectionManager = Application.activeCanvas.selectionManager
                            item.elementModel = Application.activeCanvas.elementModel
                        }
                        // Set viewportControls reference for DesignCanvas
                        if (item.hasOwnProperty("viewportControls")) {
                            // Use Qt.binding to ensure the connection stays alive
                            item.viewportControls = Qt.binding(function() { return viewportOverlay.selectionControls })
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
                            actionsPanel.currentMode = Application.activeCanvas.controller.mode
                        }
                    }
                }
            }
            
            // Prototype Panel - positioned relative to canvas container
            PrototypePanel {
                id: prototypePanel
                visible: Application.activeCanvas && 
                         Application.activeCanvas.prototypeController && 
                         Application.activeCanvas.prototypeController.isPrototyping
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
            elementModel: Application.activeCanvas ? Application.activeCanvas.elementModel : null
            selectionManager: Application.activeCanvas ? Application.activeCanvas.selectionManager : null
            
        }
    }
    
    // Global selector panel that appears outside of PropertiesPanel
    PropertyPopoverPanel {
        id: globalSelectorPanel
        visible: false
        
        property bool manuallyPositioned: false
        property var anchorSelector: null
        property real offsetY: 0
        property string currentType: ""
        
        // Position will be set dynamically based on where it was triggered
        x: 0
        y: 0
        z: 1000 // Ensure it appears above other content
        
        onCloseRequested: {
            globalSelectorPanel.visible = false
            globalSelectorPanel.manuallyPositioned = false
            globalSelectorPanel.anchorSelector = null
            globalSelectorPanel.currentType = ""
        }
        
        // Dynamic content based on type
        contentComponent: {
            switch (globalSelectorPanel.currentType) {
                case "fill":
                    return fillComponent
                case "style":
                    return styleComponent
                default:
                    return null
            }
        }
    }
    
    // Component for Fill selector
    Component {
        id: fillComponent
        Item {
            // Set implicit height based on content
            implicitHeight: colorTypeRow.y + colorTypeRow.height + 10
            
            property var selectedElement: Application.activeCanvas && Application.activeCanvas.selectionManager 
                ? Application.activeCanvas.selectionManager.selectedElements[0] : null
            
            property real currentHue: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    var color = selectedElement.fill
                    return color.hslHue * 360  // Qt returns hue as 0-1, convert to 0-360
                }
                return 200  // Default to light blue hue
            }
            
            property real currentSaturation: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    var color = selectedElement.fill
                    return color.hslSaturation
                }
                return 1.0
            }
            
            property real currentLightness: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    var color = selectedElement.fill
                    return color.hslLightness
                }
                return 0.5
            }
            
            property real currentOpacity: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    var color = selectedElement.fill
                    return color.a  // Alpha channel
                }
                return 1.0
            }
            
            // Fill type selector
            ComboBox {
                id: fillTypeCombo
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 10
                width: parent.width - 20
                model: ["Solid", "Linear", "Radial", "Conic", "Image"]
                currentIndex: 0  // Default to Solid
                
                onCurrentIndexChanged: {
                    // No functionality for now
                }
            }
            
            ShadeSelector {
                id: shadeSelector
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fillTypeCombo.bottom
                anchors.topMargin: 10
                width: parent.width - 20
                height: 100
                
                hue: currentHue
                saturation: currentSaturation
                lightness: currentLightness
                
                onShadeUpdated: function(newSaturation, newLightness) {
                    updateColor()
                }
            }
            
            HueSlider {
                id: hueSlider
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: shadeSelector.bottom
                anchors.topMargin: 10
                width: parent.width - 20
                
                hue: currentHue
                // Always show full saturation and medium lightness for the hue gradient
                saturation: 1.0
                lightness: 0.5
                
                onHueUpdated: function(newHue) {
                    shadeSelector.hue = newHue
                    updateColor()
                }
            }
            
            OpacitySlider {
                id: opacitySlider
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: hueSlider.bottom
                anchors.topMargin: 10
                width: parent.width - 20
                
                color: Qt.hsla(shadeSelector.hue / 360, shadeSelector.saturation, shadeSelector.lightness, 1.0)
                alpha: currentOpacity
                
                onAlphaUpdated: function(newAlpha) {
                    updateColor()
                }
            }
            
            // Row with color value and opacity percentage inputs
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: opacitySlider.bottom
                anchors.topMargin: 10
                width: parent.width - 20
                spacing: 10
                
                // Color value input
                TextField {
                    id: colorInput
                    width: (parent.width - parent.spacing) * 0.65  // Take up more space for color
                    text: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            var color = selectedElement.fill
                            
                            switch (colorTypeCombo.currentIndex) {
                                case 0: // RGB
                                    var r = Math.round(color.r * 255)
                                    var g = Math.round(color.g * 255)
                                    var b = Math.round(color.b * 255)
                                    return "rgb(" + r + ", " + g + ", " + b + ")"
                                    
                                case 1: // HEX
                                    var rh = Math.round(color.r * 255).toString(16).padStart(2, '0')
                                    var gh = Math.round(color.g * 255).toString(16).padStart(2, '0')
                                    var bh = Math.round(color.b * 255).toString(16).padStart(2, '0')
                                    return "#" + rh + gh + bh
                                    
                                case 2: // HSL
                                    var h = Math.round(color.hslHue * 360)
                                    var s = Math.round(color.hslSaturation * 100)
                                    var l = Math.round(color.hslLightness * 100)
                                    return "hsl(" + h + ", " + s + "%, " + l + "%)"
                                    
                                default:
                                    return "#000000"
                            }
                        }
                        return colorTypeCombo.currentIndex === 1 ? "#000000" : (colorTypeCombo.currentIndex === 0 ? "rgb(0, 0, 0)" : "hsl(0, 0%, 0%)")
                    }
                    
                    onAccepted: {
                        var text = this.text.trim()
                        
                        if (colorTypeCombo.currentIndex === 0) { // RGB
                            // Parse rgb(r, g, b) format
                            var rgbMatch = text.match(/rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/)
                            if (rgbMatch) {
                                var r = Math.max(0, Math.min(255, parseInt(rgbMatch[1]))) / 255
                                var g = Math.max(0, Math.min(255, parseInt(rgbMatch[2]))) / 255
                                var b = Math.max(0, Math.min(255, parseInt(rgbMatch[3]))) / 255
                                
                                var color = Qt.rgba(r, g, b, 1)
                                shadeSelector.hue = color.hslHue * 360
                                shadeSelector.saturation = color.hslSaturation
                                shadeSelector.lightness = color.hslLightness
                                updateColor()
                            }
                        } else if (colorTypeCombo.currentIndex === 1) { // HEX
                            // Parse hex color input
                            var hex = text
                            if (hex.startsWith("#")) {
                                hex = hex.substring(1)
                            }
                            if (hex.length === 6) {
                                var r = parseInt(hex.substring(0, 2), 16) / 255
                                var g = parseInt(hex.substring(2, 4), 16) / 255
                                var b = parseInt(hex.substring(4, 6), 16) / 255
                                
                                var color = Qt.rgba(r, g, b, 1)
                                shadeSelector.hue = color.hslHue * 360
                                shadeSelector.saturation = color.hslSaturation
                                shadeSelector.lightness = color.hslLightness
                                updateColor()
                            }
                        } else if (colorTypeCombo.currentIndex === 2) { // HSL
                            // Parse hsl(h, s%, l%) format
                            var hslMatch = text.match(/hsl\s*\(\s*(\d+)\s*,\s*(\d+)%?\s*,\s*(\d+)%?\s*\)/)
                            if (hslMatch) {
                                var h = Math.max(0, Math.min(360, parseInt(hslMatch[1])))
                                var s = Math.max(0, Math.min(100, parseInt(hslMatch[2]))) / 100
                                var l = Math.max(0, Math.min(100, parseInt(hslMatch[3]))) / 100
                                
                                shadeSelector.hue = h
                                shadeSelector.saturation = s
                                shadeSelector.lightness = l
                                updateColor()
                            }
                        }
                    }
                }
                
                // Opacity percentage input
                TextField {
                    id: opacityInput
                    width: (parent.width - parent.spacing) * 0.35
                    text: Math.round(opacitySlider.alpha * 100) + "%"
                    
                    onAccepted: {
                        // Parse percentage input
                        var value = text.trim()
                        if (value.endsWith("%")) {
                            value = value.substring(0, value.length - 1)
                        }
                        var percentage = parseFloat(value)
                        if (!isNaN(percentage)) {
                            opacitySlider.alpha = Math.max(0, Math.min(100, percentage)) / 100
                            updateColor()
                        }
                    }
                }
            }
            
            // Row with color type selector and eye button
            Row {
                id: colorTypeRow
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.children[parent.children.length - 2].bottom  // Reference the previous row
                anchors.topMargin: 10
                width: parent.width - 20
                spacing: 10
                
                // Color type selector
                ComboBox {
                    id: colorTypeCombo
                    width: (parent.width - parent.spacing) * 0.65
                    model: ["RGB", "HEX", "HSL"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            return selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                        }
                        return 1  // Default to HEX
                    }
                    
                    onCurrentIndexChanged: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            selectedElement.colorFormat = currentIndex
                        }
                    }
                }
                
                // Eye button
                Button {
                    width: (parent.width - parent.spacing) * 0.35
                    text: "Eye"
                    
                    onClicked: {
                        // No functionality for now
                    }
                }
            }
            
            function updateColor() {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    // Create a new color with the selected hue, saturation, lightness and opacity
                    var newColor = Qt.hsla(shadeSelector.hue / 360, shadeSelector.saturation, shadeSelector.lightness, opacitySlider.alpha)
                    selectedElement.fill = newColor
                }
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
    
    // Update position when PropertiesPanel moves (unless manually positioned)
    Connections {
        target: detailPanel
        enabled: globalSelectorPanel.visible && !globalSelectorPanel.manuallyPositioned
        
        function onXChanged() {
            if (globalSelectorPanel.anchorSelector) {
                var propertiesPanelPos = detailPanel.propertiesPanel.mapToItem(mainWindow.contentItem, 0, 0)
                globalSelectorPanel.x = propertiesPanelPos.x - globalSelectorPanel.width - 10
            }
        }
        
        function onWidthChanged() {
            if (globalSelectorPanel.anchorSelector) {
                var propertiesPanelPos = detailPanel.propertiesPanel.mapToItem(mainWindow.contentItem, 0, 0)
                globalSelectorPanel.x = propertiesPanelPos.x - globalSelectorPanel.width - 10
            }
        }
        
        function onYChanged() {
            if (globalSelectorPanel.anchorSelector) {
                var propertiesPanelPos = detailPanel.propertiesPanel.mapToItem(mainWindow.contentItem, 0, 0)
                globalSelectorPanel.y = propertiesPanelPos.y + globalSelectorPanel.offsetY
            }
        }
    }
    
    // Close panel when selection changes
    Connections {
        target: Application.activeCanvas ? Application.activeCanvas.selectionManager : null
        function onSelectionChanged() {
            if (globalSelectorPanel.visible) {
                globalSelectorPanel.visible = false
            }
        }
    }
    
    // Connect to PropertiesPanel signals
    Connections {
        target: detailPanel.propertiesPanel
        function onPanelSelectorClicked(selector, type) {
            console.log("Signal received in main.qml, selector:", selector, "type:", type)
            
            // Store the anchor selector and reset manual positioning
            globalSelectorPanel.anchorSelector = selector
            globalSelectorPanel.manuallyPositioned = false
            globalSelectorPanel.currentType = type
            
            // Get the PropertiesPanel's position in the window
            var propertiesPanelPos = detailPanel.propertiesPanel.mapToItem(mainWindow.contentItem, 0, 0)
            console.log("PropertiesPanel position:", propertiesPanelPos)
            
            // Get the selector's position relative to the window for Y coordinate
            var selectorPos = selector.mapToItem(mainWindow.contentItem, 0, 0)
            
            // Store the Y offset from the selector
            globalSelectorPanel.offsetY = selectorPos.y - propertiesPanelPos.y
            
            // Position the panel so its right edge is 10px from the PropertiesPanel's left edge
            globalSelectorPanel.x = propertiesPanelPos.x - globalSelectorPanel.width - 10
            globalSelectorPanel.y = selectorPos.y
            
            // Ensure the panel stays within window bounds
            if (globalSelectorPanel.x < 10) {
                globalSelectorPanel.x = 10
            }
            if (globalSelectorPanel.y + globalSelectorPanel.height > mainWindow.height) {
                globalSelectorPanel.y = mainWindow.height - globalSelectorPanel.height - 10
            }
            
            // Only show the panel, don't toggle
            globalSelectorPanel.visible = true
            console.log("Panel visible:", globalSelectorPanel.visible)
            console.log("Panel position:", globalSelectorPanel.x, globalSelectorPanel.y)
        }
    }
}