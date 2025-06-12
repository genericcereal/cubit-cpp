import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#ffffff"
    antialiasing: true
    
    property var elementModel
    property var selectionManager
    property string currentCanvasType: "design"
    
    signal canvasTypeChanged(string canvasType)
    
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
        
        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#e0e0e0"
            antialiasing: true
        }
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#f0f0f0"
            antialiasing: true
            
            Label {
                anchors.centerIn: parent
                text: "Elements"
                font.pixelSize: 16
                font.weight: Font.Medium
            }
        }
        
        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#e0e0e0"
            antialiasing: true
        }
        
        // Content area with tabs
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            
            TabButton {
                text: "Elements"
            }
            
            TabButton {
                text: "Properties"
            }
        }
        
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            
            // Elements tab
            ElementList {
                elementModel: root.elementModel
                selectionManager: root.selectionManager
            }
            
            // Properties tab
            PropertiesPanel {
                selectionManager: root.selectionManager
            }
        }
    }
}