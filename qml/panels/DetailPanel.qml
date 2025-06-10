import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#ffffff"
    antialiasing: true
    
    property var elementModel
    property var selectionManager
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
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