import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

Rectangle {
    id: root
    color: "#ffffff"
    antialiasing: true
    
    property var elementModel
    property var selectionManager
    property string currentCanvasType: Application.activeCanvasViewMode
    
    signal canvasTypeChanged(string canvasType)
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Tab bar at the top
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            
            TabButton {
                text: "Details"
                font.pixelSize: 14
            }
            
            TabButton {
                text: "Console"
                font.pixelSize: 14
            }
        }
        
        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            
            // Details tab content
            Rectangle {
                color: "#ffffff"
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0
                    
                    // Canvas Type Switcher
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        color: "#f8f8f8"
                        antialiasing: true
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10
                            
                            Label {
                                text: "Canvas:"
                                font.pixelSize: 14
                            }
                            
                            Button {
                                id: canvasTypeButton
                                Layout.fillWidth: true
                                text: root.currentCanvasType === "design" ? "Design Canvas" : "Script Canvas"
                                font.pixelSize: 14
                                
                                background: Rectangle {
                                    color: canvasTypeButton.pressed ? "#e0e0e0" : (canvasTypeButton.hovered ? "#f0f0f0" : "#ffffff")
                                    border.color: "#d0d0d0"
                                    border.width: 1
                                    radius: 4
                                    antialiasing: true
                                }
                                
                                onClicked: {
                                    root.currentCanvasType = root.currentCanvasType === "design" ? "script" : "design"
                                    root.canvasTypeChanged(root.currentCanvasType)
                                }
                            }
                        }
                    }
                    
                    // Content area with splitter
                    SplitView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        orientation: Qt.Vertical
                        
                        // Elements section
                        Rectangle {
                            SplitView.fillWidth: true
                            SplitView.preferredHeight: parent.height * 0.5
                            SplitView.minimumHeight: 100
                            color: "#ffffff"
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 0
                                
                                // Elements header
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "#f8f8f8"
                                    
                                    Label {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "Elements"
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                    }
                                }
                                
                                // Separator
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: "#e0e0e0"
                                }
                                
                                // Elements list
                                ElementList {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    elementModel: root.elementModel
                                    selectionManager: root.selectionManager
                                }
                            }
                        }
                        
                        // Properties section
                        Rectangle {
                            SplitView.fillWidth: true
                            SplitView.fillHeight: true
                            SplitView.minimumHeight: 100
                            color: "#ffffff"
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 0
                                
                                // Properties header
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "#f8f8f8"
                                    
                                    Label {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "Properties"
                                        font.pixelSize: 14
                                        font.weight: Font.Medium
                                    }
                                }
                                
                                // Separator
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: "#e0e0e0"
                                }
                                
                                // Properties panel
                                PropertiesPanel {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    selectionManager: root.selectionManager
                                }
                            }
                        }
                    }
                }
            }
            
            // Console tab content
            Console {
                id: consolePanel
                
                Component.onCompleted: {
                    // Add initial welcome message via repository
                    ConsoleMessageRepository.addOutput("Welcome to Cubit Console")
                }
                
                onCommandSubmitted: function(command) {
                    // Handle command - for now just echo it back
                    ConsoleMessageRepository.addOutput("Command received: " + command)
                }
            }
        }
    }
}