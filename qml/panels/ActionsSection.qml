import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

GroupBox {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    title: "Actions"
    
    property var selectedElement
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    visible: selectedElement && selectedElement.isDesignElement
    
    GridLayout {
        anchors.fill: parent
        columns: 1
        columnSpacing: 10
        rowSpacing: 5
        
        Button {
            Layout.fillWidth: true
            text: "Create component"
            font.pixelSize: 14
            
            background: Rectangle {
                color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                border.color: "#d0d0d0"
                border.width: 1
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (selectedDesignElement) {
                    var component = selectedDesignElement.createComponent()
                    if (component) {
                        ConsoleMessageRepository.addOutput("Component created with ID: " + component.elementId)
                    }
                }
            }
        }
    }
}