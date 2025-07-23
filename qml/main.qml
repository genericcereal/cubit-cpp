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

    // Main content area - ProjectList only
    ProjectList {
        anchors.fill: parent
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
    
    // Note: DetailPanel position tracking removed since DetailPanel is now in CanvasScreen
    
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
    
    // Note: PropertiesPanel signal connections removed since PropertiesPanel is now in CanvasScreen
    
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