import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"
import "../PropertyHelpers.js" as PropertyHelpers

GroupBox {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    title: selectedElement && selectedElement.elementType === "FrameComponentVariant" ? "Variant Style" : "Frame"
    
    property var selectedElement
    property var editableProperties: {
        if (selectedElement && selectedElement.elementType === "FrameComponentInstance") {
            return selectedElement.getEditableProperties()
        }
        return []
    }
    
    signal panelSelectorClicked(var selector, string type)
    
    visible: PropertyHelpers.showFrameStyle(selectedElement)
    
    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: 10
        rowSpacing: 5
        
        Label { 
            text: "Fill:" 
            visible: PropertyHelpers.canShowFill(selectedElement, editableProperties)
        }
        PropertyPopover {
            id: fillSelector
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowFill(selectedElement, editableProperties)
            
            elementFillColor: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("fill"))) && selectedElement.fill !== undefined ? selectedElement.fill : "#add8e6"
            text: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance") && selectedElement.fill !== undefined) {
                    var color = selectedElement.fill
                    var format = selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                    
                    switch (format) {
                        case 0:
                            var r = Math.round(color.r * 255)
                            var g = Math.round(color.g * 255)
                            var b = Math.round(color.b * 255)
                            return "rgb(" + r + ", " + g + ", " + b + ")"
                            
                        case 1:
                            var rh = Math.round(color.r * 255).toString(16).padStart(2, '0')
                            var gh = Math.round(color.g * 255).toString(16).padStart(2, '0')
                            var bh = Math.round(color.b * 255).toString(16).padStart(2, '0')
                            return "#" + rh + gh + bh
                            
                        case 2:
                            var h = Math.round(color.hslHue * 360)
                            var s = Math.round(color.hslSaturation * 100)
                            var l = Math.round(color.hslLightness * 100)
                            return "hsl(" + h + ", " + s + "%, " + l + "%)"
                            
                        default:
                            return elementFillColor.toString()
                    }
                }
                return elementFillColor.toString()
            }
            
            onPanelRequested: {
                root.panelSelectorClicked(fillSelector, "fill")
            }
        }
        
        Label { 
            text: "Overflow:" 
            visible: PropertyHelpers.canShowOverflow(selectedElement, editableProperties)
        }
        ComboBox {
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowOverflow(selectedElement, editableProperties)
            model: ["Hidden", "Scroll", "Visible"]
            currentIndex: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    switch (selectedElement.overflow) {
                        case 0: return 0
                        case 1: return 1
                        case 2: return 2
                        default: return 0
                    }
                }
                return 0
            }
            onActivated: function(index) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.overflow = index
                }
            }
        }
        
        Label { 
            text: "Border Radius:" 
            visible: PropertyHelpers.canShowBorderRadius(selectedElement, editableProperties)
        }
        SpinBox {
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowBorderRadius(selectedElement, editableProperties)
            from: 0
            to: 100
            value: selectedElement && selectedElement.borderRadius !== undefined ? selectedElement.borderRadius : 0
            onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) selectedElement.borderRadius = value
        }
        
        Label { 
            text: "Border Width:" 
            visible: PropertyHelpers.canShowBorderWidth(selectedElement, editableProperties)
        }
        SpinBox {
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowBorderWidth(selectedElement, editableProperties)
            from: 0
            to: 20
            value: selectedElement && selectedElement.borderWidth !== undefined ? selectedElement.borderWidth : 0
            onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) selectedElement.borderWidth = value
        }
        
        Label { 
            text: "Style:" 
            visible: PropertyHelpers.canShowBorderColor(selectedElement, editableProperties)
        }
        PropertyPopover {
            id: styleSelector
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowBorderColor(selectedElement, editableProperties)
            placeholderText: "Select style..."
            
            onPanelRequested: {
                console.log("PropertyPopover clicked!")
                root.panelSelectorClicked(styleSelector, "style")
            }
        }
    }
}