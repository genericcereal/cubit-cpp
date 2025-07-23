import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

FocusScope {
    id: root
    
    // Signal emitted when user submits a command
    signal commandSubmitted(string command)
    
    property var canvas: null  // Will be set by parent (actually a Project instance)
    property var repository: canvas && canvas.console ? canvas.console : null
    
    // Handle arrow key navigation
    Keys.onUpPressed: {
        if (repository && repository.isUsingAI && repository.showAIPrompt && repository.selectedOption > 0) {
            repository.selectedOption = 0
        }
    }
    
    Keys.onDownPressed: {
        if (repository && repository.isUsingAI && repository.showAIPrompt && repository.selectedOption < 1) {
            repository.selectedOption = 1
        }
    }
    
    // Handle Enter key at the root level to work even when input is disabled
    Keys.onReturnPressed: {
        if (repository && repository.isUsingAI && repository.showAIPrompt && repository.selectedOption === 0) {
            // User selected "Accept" and pressed Enter
            root.commandSubmitted("")
        }
    }
    
    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
        
        // Global mouse area to remove focus when clicking anywhere
        MouseArea {
            anchors.fill: parent
            onClicked: {
                // Remove focus from input field
                root.focus = true
            }
        }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Messages list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            ListView {
                id: listView
                model: root.repository ? root.repository.messages : []
                clip: true
                
                // Remove focus when clicking on the list
                TapHandler {
                    onTapped: {
                        if (inputField.activeFocus) {
                            inputField.focus = false
                        }
                    }
                }
                
                // Auto-scroll to bottom when new messages are added
                Connections {
                    target: root.repository
                    enabled: root.repository !== null
                    function onMessagesChanged() {
                        // Use a timer to ensure the ListView has updated before scrolling
                        scrollTimer.restart()
                    }
                }
                
                Timer {
                    id: scrollTimer
                    interval: 10
                    onTriggered: {
                        listView.positionViewAtEnd()
                    }
                }
                
                delegate: Rectangle {
                    width: listView.width
                    height: messageText.implicitHeight + 16
                    color: modelData.type === "error" ? "#fff0f0" : 
                           modelData.type === "input" ? "#f0f8ff" : 
                           "#ffffff"
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8
                        
                        Label {
                            text: modelData.timestamp
                            font.pixelSize: 11
                            color: "#888888"
                            Layout.preferredWidth: 60
                        }
                        
                        Label {
                            id: messageText
                            text: modelData.text
                            font.pixelSize: 13
                            font.family: Qt.platform.os === "osx" ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono")
                            color: modelData.type === "error" ? "#cc0000" : 
                                   modelData.type === "input" ? "#0066cc" : 
                                   "#333333"
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }
                    }
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: "#e0e0e0"
                    }
                }
            }
        }
        
        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#e0e0e0"
        }
        
        // Options bar (only visible when both isUsingAI and showAIPrompt are true)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: (repository && repository.isUsingAI && repository.showAIPrompt) ? 80 : 0
            visible: repository && repository.isUsingAI && repository.showAIPrompt
            color: "#f0f0f0"
            
            // Ensure root has focus when options are shown
            onVisibleChanged: {
                if (visible) {
                    root.forceActiveFocus()
                }
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                
                // Option 1: Accept
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    radius: 4
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { if (repository) repository.selectedOption = 0 }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8
                        
                        Label {
                            text: (repository && repository.selectedOption === 0) ? ">" : " "
                            color: "#0066cc"
                            font.pixelSize: 13
                            font.family: Qt.platform.os === "osx" ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono")
                            Layout.preferredWidth: 15
                        }
                        
                        Label {
                            text: "Yes, accept"
                            color: (repository && repository.selectedOption === 0) ? "#0066cc" : "#333333"
                            font.pixelSize: 13
                            Layout.fillWidth: true
                        }
                    }
                }
                
                // Option 2: Feedback
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "transparent"
                    radius: 4
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { if (repository) repository.selectedOption = 1 }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8
                        
                        Label {
                            text: (repository && repository.selectedOption === 1) ? ">" : " "
                            color: "#0066cc"
                            font.pixelSize: 13
                            font.family: Qt.platform.os === "osx" ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono")
                            Layout.preferredWidth: 15
                        }
                        
                        Label {
                            text: "No, tell AI what to do differently"
                            color: (repository && repository.selectedOption === 1) ? "#0066cc" : "#333333"
                            font.pixelSize: 13
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
        
        // Input area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: (repository && repository.isUsingAI) ? "#e6f2ff" : "#f8f8f8"
            
            // Stop propagation of mouse events in input area
            MouseArea {
                anchors.fill: parent
                // Accept events but don't do anything - prevents global MouseArea from getting them
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Label {
                    text: (repository && repository.isUsingAI) ? "AI>" : ">"
                    font.pixelSize: 14
                    font.family: Qt.platform.os === "osx" ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono")
                    color: (repository && repository.isUsingAI) ? "#0066cc" : "#666666"
                }
                
                TextField {
                    id: inputField
                    Layout.fillWidth: true
                    font.pixelSize: 13
                    font.family: Qt.platform.os === "osx" ? "Menlo" : (Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono")
                    placeholderText: (repository && repository.isUsingAI) ? 
                        (repository.showAIPrompt && repository.selectedOption === 0 ? "Accept selected - press Enter to confirm" : "Ask AI a question...") : 
                        "Enter command..."
                    selectByMouse: true
                    enabled: !(repository && repository.isUsingAI && repository.showAIPrompt && repository.selectedOption === 0)
                    
                    
                    Keys.onReturnPressed: {
                        // If AI prompt is showing and "yes" is selected, treat Enter as confirmation
                        if (repository && repository.isUsingAI && repository.showAIPrompt && repository.selectedOption === 0) {
                            // Emit empty command to trigger the "yes" confirmation
                            root.commandSubmitted("")
                        } else if (text.trim() !== "") {
                            // Normal command submission
                            root.commandSubmitted(text)
                            // Clear input
                            text = ""
                        }
                    }
                }
                
                Button {
                    text: "Send"
                    Layout.preferredWidth: 60
                    enabled: inputField.text.trim() !== ""
                    
                    
                    onClicked: {
                        if (inputField.text.trim() !== "") {
                            // Emit signal (Application will handle logging)
                            root.commandSubmitted(inputField.text)
                            inputField.text = ""
                        }
                    }
                }
            }
        }
    }
    }
}