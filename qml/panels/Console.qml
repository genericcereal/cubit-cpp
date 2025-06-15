import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

Rectangle {
    id: root
    color: "#ffffff"
    
    // Signal emitted when user submits a command
    signal commandSubmitted(string command)
    
    property var repository: ConsoleMessageRepository
    
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
                model: root.repository.messages
                clip: true
                
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
                            font.family: "Consolas, Monaco, monospace"
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
        
        // Input area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#f8f8f8"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Label {
                    text: ">"
                    font.pixelSize: 14
                    font.family: "Consolas, Monaco, monospace"
                    color: "#666666"
                }
                
                TextField {
                    id: inputField
                    Layout.fillWidth: true
                    font.pixelSize: 13
                    font.family: "Consolas, Monaco, monospace"
                    placeholderText: "Enter command..."
                    selectByMouse: true
                    
                    background: Rectangle {
                        color: "#ffffff"
                        border.color: inputField.activeFocus ? "#0066cc" : "#d0d0d0"
                        border.width: 1
                        radius: 2
                    }
                    
                    Keys.onReturnPressed: {
                        if (text.trim() !== "") {
                            // Add input to repository
                            root.repository.addInput(text)
                            // Emit signal
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
                    
                    background: Rectangle {
                        color: parent.enabled ? (parent.pressed ? "#0055aa" : (parent.hovered ? "#0077cc" : "#0066cc")) : "#cccccc"
                        radius: 4
                    }
                    
                    contentItem: Label {
                        text: parent.text
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 13
                    }
                    
                    onClicked: {
                        if (inputField.text.trim() !== "") {
                            root.repository.addInput(inputField.text)
                            root.commandSubmitted(inputField.text)
                            inputField.text = ""
                        }
                    }
                }
            }
        }
    }
}