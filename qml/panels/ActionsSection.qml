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
    
    // Show when we have a selected element that can have actions, or when nothing is selected
    visible: {
        if (!selectedElement) return Application.activeCanvas !== null
        
        // Show for design elements (Frame, Text)
        if (selectedElement.isDesignElement) return true
        
        // Show for components (to show Variants button)
        if (selectedElement.elementType === "Component") return true
        
        // Show for component instances
        if (selectedElement.elementType === "FrameComponentInstance" || 
            selectedElement.elementType === "TextComponentInstance") return true
            
        return false
    }
    
    GridLayout {
        anchors.fill: parent
        columns: 1
        columnSpacing: 10
        rowSpacing: 5
        
        // Canvas Scripts button - shows when nothing is selected
        Button {
            Layout.fillWidth: true
            text: "Canvas Scripts"
            font.pixelSize: 14
            visible: !selectedElement && Application.activeCanvas
            
            background: Rectangle {
                color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                border.color: "#d0d0d0"
                border.width: 1
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (Application.activeCanvas) {
                    Application.activeCanvas.setEditingElement(null, "script")
                }
            }
        }
        
        // Create component button - only for design elements
        Button {
            Layout.fillWidth: true
            text: "Create component"
            font.pixelSize: 14
            visible: selectedDesignElement !== null
            
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
        
        // Scripts button - shows for elements that can have scripts
        Button {
            Layout.fillWidth: true
            text: selectedElement && selectedElement.name ? selectedElement.name + " Scripts" : "Scripts"
            font.pixelSize: 14
            visible: selectedElement && (selectedElement.isDesignElement || 
                                       selectedElement.elementType === "Component" ||
                                       selectedElement.elementType === "FrameComponentInstance" ||
                                       selectedElement.elementType === "TextComponentInstance")
            
            background: Rectangle {
                color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                border.color: "#d0d0d0"
                border.width: 1
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (Application.activeCanvas && selectedElement) {
                    Application.activeCanvas.setEditingElement(selectedElement, "script")
                }
            }
        }
        
        // Variants button - only visible when a component is selected
        Button {
            Layout.fillWidth: true
            text: "Variants"
            font.pixelSize: 14
            visible: selectedElement && selectedElement.elementType === "Component"
            
            background: Rectangle {
                color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                border.color: "#d0d0d0"
                border.width: 1
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (Application.activeCanvas && selectedElement && selectedElement.elementType === "Component") {
                    Application.activeCanvas.setEditingComponent(selectedElement, "variant")
                }
            }
        }
    }
}