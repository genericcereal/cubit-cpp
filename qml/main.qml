import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Cubit 1.0
import "components/panels"
import "components"
import "components/viewport-overlay"
import "components/color-picker"

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1280
    height: 720
    title: qsTr("Cubit")
    
    property alias dragOverlay: dragOverlay
    
    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Open...")
                onTriggered: {
                    Application.openFile()
                }
            }
            MenuSeparator { }
            MenuItem {
                text: qsTr("Save As...")
                onTriggered: {
                    Application.saveAs()
                }
            }
        }
    }
    
    Component.onCompleted: {
        // Request focus for keyboard shortcuts
        mainWindow.requestActivate()
        
        // Start authentication on launch
        authManager.checkAutoLogin()
    }

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
                        
                        // Restore viewport state when switching back to design/variant canvas
                        if ((Application.activeCanvas.viewMode === "design" || Application.activeCanvas.viewMode === "variant") && 
                            Application.activeCanvas.controller) {
                            var savedState = {
                                contentX: Application.activeCanvas.controller.savedContentX,
                                contentY: Application.activeCanvas.controller.savedContentY,
                                zoom: Application.activeCanvas.controller.savedZoom
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
                target: Application.activeCanvas
                enabled: Application.activeCanvas !== null
                
                function onViewportStateShouldBeSaved() {
                    // Save current viewport state from design/variant canvas
                    if (canvasLoader.item && canvasLoader.item.getViewportState && 
                        Application.activeCanvas.controller) {
                        var state = canvasLoader.item.getViewportState()
                        Application.activeCanvas.controller.savedContentX = state.contentX
                        Application.activeCanvas.controller.savedContentY = state.contentY
                        Application.activeCanvas.controller.savedZoom = state.zoom
                    }
                }
                
                function onViewModeChanged() {
                    // Center the viewport when entering variant mode
                    if (Application.activeCanvas.viewMode === "variant" && canvasLoader.item && canvasLoader.item.moveToPoint) {
                        // Calculate center point based on selected component or canvas bounds
                        var centerPoint = Qt.point(0, 0)
                        
                        if (Application.activeCanvas.selectionManager) {
                            var selectedElements = Application.activeCanvas.selectionManager.selectedElements
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
                            // Scripts compiled successfully
                        }
                    }
                }
                onCreateVariableClicked: {
                    if (Application.activeCanvas && Application.activeCanvas.controller) {
                        Application.activeCanvas.controller.createVariable()
                    }
                }
                onWebElementsClicked: {
                    // Handled internally by ActionsPanel
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
                case "font":
                    return fontComponent
                default:
                    return null
            }
        }
    }
    
    // Component for Fill selector
    Component {
        id: fillComponent
        ColorPicker {
            property var selectedElement: Application.activeCanvas && Application.activeCanvas.selectionManager 
                ? Application.activeCanvas.selectionManager.selectedElements[0] : null
            
            hue: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var color = selectedElement.fill
                    return color.hslHue * 360  // Qt returns hue as 0-1, convert to 0-360
                }
                return 200  // Default to light blue hue
            }
            
            saturation: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var color = selectedElement.fill
                    return color.hslSaturation
                }
                return 1.0
            }
            
            lightness: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var color = selectedElement.fill
                    return color.hslLightness
                }
                return 0.5
            }
            
            alphaValue: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var color = selectedElement.fill
                    return color.a  // Alpha channel
                }
                return 1.0
            }
            
            colorFormat: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant")) {
                    return selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                }
                return 1  // Default to HEX
            }
            
            fillType: 0  // Default to Solid for now
            
            onColorChanged: function(newColor) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.fill = newColor
                }
            }
            
            onColorFormatChanged: function(newFormat) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
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
            property var selectedElement: Application.activeCanvas && Application.activeCanvas.selectionManager 
                ? Application.activeCanvas.selectionManager.selectedElements[0] : null
            
            currentFontFamily: selectedElement && selectedElement.font ? selectedElement.font.family : ""
            
            onFontSelected: function(fontFamily) {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.family = fontFamily
                    selectedElement.font = newFont
                }
                // Close the popover panel
                globalSelectorPanel.visible = false
            }
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
    
    // Authentication overlay - covers entire window when not authenticated
    Rectangle {
        id: authOverlay
        anchors.fill: parent
        color: "white"
        opacity: 0.95
        visible: !authManager.isAuthenticated
        z: 9999 // Above everything else
        
        // Prevent interaction with underlying content
        MouseArea {
            anchors.fill: parent
            onClicked: {}
            onDoubleClicked: {}
            onPressed: {}
            onReleased: {}
            onWheel: {}
        }
        
        // Authentication UI
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "Authentication Required"
                font.pixelSize: 24
                font.weight: Font.Medium
                color: "#333333"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: authManager.isLoading ? "Authenticating..." : "Please log in to continue"
                font.pixelSize: 14
                color: "#666666"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Button {
                text: "Login with AWS Cognito"
                visible: !authManager.isLoading
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: authManager.login()
                
            }
            
            BusyIndicator {
                running: authManager.isLoading
                visible: authManager.isLoading
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
        
        // Error message
        Text {
            text: ""
            id: authErrorText
            color: "#f44336"
            font.pixelSize: 12
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            visible: text !== ""
        }
        
        Connections {
            target: authManager
            function onAuthenticationError(error) {
                authErrorText.text = "Error: " + error
                errorTimer.restart()
            }
        }
        
        Timer {
            id: errorTimer
            interval: 5000
            onTriggered: authErrorText.text = ""
        }
    }
    
    // Drag overlay for ElementList
    Item {
        id: dragOverlay
        anchors.fill: parent
        visible: false
        z: 10000
        
        property string draggedElementName: ""
        property string draggedElementType: ""
        property var draggedElement: null
        property point ghostPosition: Qt.point(0, 0)
        
        Rectangle {
            id: dragGhost
            x: dragOverlay.ghostPosition.x - width / 2
            y: dragOverlay.ghostPosition.y - height / 2
            width: 200
            height: 28
            color: "#f5f5f5"
            border.color: "#2196F3"
            border.width: 2
            radius: 4
            opacity: 0.8
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 6
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                spacing: 4
                
                Label {
                    Layout.fillWidth: true
                    text: dragOverlay.draggedElementName
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
                
                Label {
                    Layout.alignment: Qt.AlignRight
                    text: dragOverlay.draggedElementType
                    color: "#666666"
                    font.pixelSize: 10
                }
            }
        }
        
        // Remove the MouseArea - we'll track position through ghostPosition property instead
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
    
    // File dialogs
    FileDialog {
        id: saveFileDialog
        title: "Save Project As"
        nameFilters: ["Cubit Files (*.cbt)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "cbt"
        
        onAccepted: {
            // Convert file URL to local path
            var filePath = selectedFile.toString()
            // Remove "file://" prefix
            if (Qt.platform.os === "windows") {
                filePath = filePath.replace(/^(file:\/{3})/, "")
            } else {
                filePath = filePath.replace(/^(file:\/{2})/, "")
            }
            
            Application.saveToFile(filePath)
        }
    }
    
    FileDialog {
        id: openFileDialog
        title: "Open Project"
        nameFilters: ["Cubit Files (*.cbt)"]
        fileMode: FileDialog.OpenFile
        
        onAccepted: {
            // Convert file URL to local path
            var filePath = selectedFile.toString()
            // Remove "file://" prefix
            if (Qt.platform.os === "windows") {
                filePath = filePath.replace(/^(file:\/{3})/, "")
            } else {
                filePath = filePath.replace(/^(file:\/{2})/, "")
            }
            
            Application.loadFromFile(filePath)
        }
    }
    
    // Connect to Application signals
    Connections {
        target: Application
        
        function onSaveFileRequested() {
            saveFileDialog.open()
        }
        
        function onOpenFileRequested() {
            openFileDialog.open()
        }
    }
}