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
    property var canvas: null  // Will be set by parent
    property string currentCanvasType: canvas ? canvas.viewMode : "design"
    property var editingElement: canvas ? canvas.editingElement : null
    
    // Expose the properties panel for external connections
    property alias propertiesPanel: propertiesPanel
    
    // Helper to get the currently selected element for script editing
    function getSelectedElementForScripts() {
        if (!canvas || !canvas.selectionManager) return null
        var selectedElements = canvas.selectionManager.selectedElements
        if (selectedElements && selectedElements.length === 1) {
            return selectedElements[0]
        }
        return null
    }
    
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
                                    canvas: root.canvas
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
                                    id: propertiesPanel
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    canvas: root.canvas
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
                canvas: root.canvas
                
                Component.onCompleted: {
                    // Initial welcome message removed
                }
                
                onCommandSubmitted: function(command) {
                    // Process the command through the project's console with canvas context
                    if (root.canvas && root.canvas.console) {
                        root.canvas.console.processConsoleCommand(command, root.canvas)
                    }
                }
            }
        }
    }
}