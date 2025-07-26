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
    // Note: PropertyPopoverPanel and its components have been moved to CanvasScreen
    
    // Authentication/Loading overlay - covers entire window when loading or not authenticated
    Rectangle {
        id: authOverlay
        anchors.fill: parent
        color: "white"
        opacity: 1.0
        visible: !authManager.isAuthenticated || authManager.isLoading
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
                text: authManager.isLoading && authManager.isAuthenticated ? "Loading..." : "Authentication Required"
                font.pixelSize: 24
                font.weight: Font.Medium
                color: "#333333"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: authManager.isLoading ? "Checking authentication..." : "Please log in to continue"
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
            
            Button {
                text: "Open Login URL in Browser"
                visible: !authManager.isLoading
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    var url = authManager.getLoginUrl()
                    Qt.openUrlExternally(url)
                    // Start checking for callback
                    authManager.login()
                }
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