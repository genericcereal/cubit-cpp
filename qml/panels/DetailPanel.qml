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
    property string currentCanvasType: Application.activeCanvas ? Application.activeCanvas.viewMode : "design"
    property var editingElement: Application.activeCanvas ? Application.activeCanvas.editingElement : null
    
    // Helper to get the currently selected element for script editing
    function getSelectedElementForScripts() {
        if (!Application.activeCanvas || !Application.activeCanvas.selectionManager) return null
        var selectedElements = Application.activeCanvas.selectionManager.selectedElements
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
                                text: {
                                    if (root.currentCanvasType === "design") {
                                        return "Design Canvas"
                                    } else if (root.currentCanvasType === "variant") {
                                        return "Variant Canvas"
                                    } else {
                                        // Check if we have an editing element (DesignElement, ComponentInstance, or Component)
                                        if (root.editingElement && root.editingElement.name) {
                                            return root.editingElement.name + " Scripts"
                                        }
                                        // Check if we have a selected element
                                        var selectedElement = root.getSelectedElementForScripts()
                                        if (selectedElement && selectedElement.name) {
                                            return selectedElement.name + " Scripts"
                                        }
                                        return "Script Canvas"
                                    }
                                }
                                font.pixelSize: 14
                                
                                background: Rectangle {
                                    color: canvasTypeButton.pressed ? "#e0e0e0" : (canvasTypeButton.hovered ? "#f0f0f0" : "#ffffff")
                                    border.color: "#d0d0d0"
                                    border.width: 1
                                    radius: 4
                                    antialiasing: true
                                }
                                
                                onClicked: {
                                    if (Application.activeCanvas) {
                                        var newMode
                                        if (root.currentCanvasType === "design") {
                                            newMode = "script"
                                        } else if (root.currentCanvasType === "script") {
                                            newMode = "design"
                                        } else {
                                            // From variant, go back to design
                                            newMode = "design"
                                        }
                                        
                                        console.log("Canvas toggle clicked, switching to:", newMode)
                                        
                                        if (newMode === "script") {
                                            // When switching to script mode, check if we have a selected design element
                                            var selectionManager = Application.activeCanvas.selectionManager
                                            console.log("Selection manager:", selectionManager)
                                            console.log("Has visual selection:", selectionManager.hasVisualSelection)
                                            console.log("Selection count:", selectionManager.selectionCount)
                                            
                                            // For now, just switch to script mode
                                            // The C++ code will handle setting editing element based on selection
                                            Application.activeCanvas.setEditingElement(null, "script")
                                        } else {
                                            // Switching back to design mode
                                            Application.activeCanvas.setEditingElement(null, newMode)
                                        }
                                        
                                        root.currentCanvasType = newMode
                                    }
                                }
                            }
                            
                            // Variant Canvas button - only visible when a component is selected
                            Button {
                                id: variantButton
                                visible: {
                                    // Show when a component is selected
                                    var selectedElements = root.selectionManager ? root.selectionManager.selectedElements : []
                                    if (selectedElements && selectedElements.length === 1) {
                                        var element = selectedElements[0]
                                        // Check if the selected element is a Component
                                        return element && element.elementType === "Component"
                                    }
                                    return false
                                }
                                Layout.preferredWidth: 100
                                text: "Variants"
                                font.pixelSize: 14
                                
                                background: Rectangle {
                                    color: variantButton.pressed ? "#e0e0e0" : (variantButton.hovered ? "#f0f0f0" : "#ffffff")
                                    border.color: "#d0d0d0"
                                    border.width: 1
                                    radius: 4
                                    antialiasing: true
                                }
                                
                                onClicked: {
                                    if (Application.activeCanvas) {
                                        // Get the selected component
                                        var selectedElements = root.selectionManager ? root.selectionManager.selectedElements : []
                                        if (selectedElements && selectedElements.length === 1) {
                                            var component = selectedElements[0]
                                            if (component && component.elementType === "Component") {
                                                // Set the component as the editing element
                                                Application.activeCanvas.setEditingComponent(component, "variant")
                                            }
                                        }
                                    }
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