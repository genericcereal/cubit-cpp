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
    property var canvas: null  // Will be set by parent
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    // Show when we have a selected element that can have actions, or when nothing is selected
    visible: {
        if (!selectedElement) return canvas !== null
        
        // Show for design elements (Frame, Text)
        if (selectedElement.isDesignElement) return true
        
        // Show for components (to show Variants button)
        if (selectedElement.elementType === "Component") return true
        
        // Show for component instances
        if (selectedElement.elementType === "FrameComponentInstance" || 
            selectedElement.elementType === "TextComponentInstance" ||
            selectedElement.elementType === "WebTextInputComponentInstance") return true
            
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
            visible: !selectedElement && canvas
            
            
            onClicked: {
                if (canvas) {
                    canvas.setEditingElement(null, "script")
                }
            }
        }
        
        // Create component button - only for design elements that are not component instances
        Button {
            Layout.fillWidth: true
            text: "Create component"
            font.pixelSize: 14
            visible: selectedDesignElement !== null && 
                     selectedElement.elementType !== "FrameComponentInstance" && 
                     selectedElement.elementType !== "TextComponentInstance" &&
                     selectedElement.elementType !== "WebTextInputComponentInstance"
            
            
            onClicked: {
                if (selectedDesignElement && canvas && canvas.controller) {
                    canvas.controller.createComponent(selectedDesignElement)
                }
            }
        }
        
        // Scripts button - shows for elements that can have scripts (but not variants)
        Button {
            Layout.fillWidth: true
            text: selectedElement && selectedElement.name ? selectedElement.name + " Scripts" : "Scripts"
            font.pixelSize: 14
            visible: selectedElement && 
                     selectedElement.elementType !== "FrameComponentVariant" &&
                     selectedElement.elementType !== "TextComponentVariant" &&
                     selectedElement.elementType !== "WebTextInputComponentVariant" &&
                     (selectedElement.isDesignElement || 
                      selectedElement.elementType === "Component" ||
                      selectedElement.elementType === "FrameComponentInstance" ||
                      selectedElement.elementType === "TextComponentInstance" ||
                      selectedElement.elementType === "WebTextInputComponentInstance")
            
            
            onClicked: {
                if (canvas && selectedElement) {
                    // Components need special handling
                    if (selectedElement.elementType === "Component") {
                        canvas.setEditingComponent(selectedElement, "script")
                    } else {
                        canvas.setEditingElement(selectedElement, "script")
                    }
                }
            }
        }
        
        // Variants button - only visible when a component is selected
        Button {
            Layout.fillWidth: true
            text: "Variants"
            font.pixelSize: 14
            visible: selectedElement && selectedElement.elementType === "Component"
            
            
            onClicked: {
                if (canvas && selectedElement && selectedElement.elementType === "Component") {
                    canvas.setEditingComponent(selectedElement, "variant")
                }
            }
        }
    }
}