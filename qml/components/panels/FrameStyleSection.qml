import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

PropertyGroup {
    id: root
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
    
    property var frameStyleProps: [
        {
            name: "Fill",
            type: "property_popover",
            popoverType: "fill",
            elementFillColor: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("fill"))) && selectedElement.fill !== undefined ? selectedElement.fill : "#add8e6",
            textGetter: () => {
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
                            return root.frameStyleProps[0].elementFillColor().toString()
                    }
                }
                return root.frameStyleProps[0].elementFillColor().toString()
            },
            visible: () => PropertyHelpers.canShowFill(selectedElement, editableProperties)
        },
        {
            name: "Overflow",
            type: "combobox",
            getter: () => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    switch (selectedElement.overflow) {
                        case 0: return "Hidden"
                        case 1: return "Scroll"
                        case 2: return "Visible"
                        default: return "Hidden"
                    }
                }
                return "Hidden"
            },
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    const index = ["Hidden", "Scroll", "Visible"].indexOf(v)
                    selectedElement.overflow = index >= 0 ? index : 0
                }
            },
            model: () => ["Hidden", "Scroll", "Visible"],
            visible: () => PropertyHelpers.canShowOverflow(selectedElement, editableProperties)
        },
        {
            name: "Border Radius",
            type: "spinbox",
            from: 0,
            to: 100,
            getter: () => selectedElement && selectedElement.borderRadius !== undefined ? selectedElement.borderRadius : 0,
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.borderRadius = v
                }
            },
            visible: () => PropertyHelpers.canShowBorderRadius(selectedElement, editableProperties)
        },
        {
            name: "Border Width",
            type: "spinbox",
            from: 0,
            to: 20,
            getter: () => selectedElement && selectedElement.borderWidth !== undefined ? selectedElement.borderWidth : 0,
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.borderWidth = v
                }
            },
            visible: () => PropertyHelpers.canShowBorderWidth(selectedElement, editableProperties)
        },
        {
            name: "Style",
            type: "property_popover",
            popoverType: "style",
            placeholderText: "Select style...",
            visible: () => PropertyHelpers.canShowBorderColor(selectedElement, editableProperties)
        }
    ]
    
    Component {
        id: comboBoxComp
        ComboBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: spinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: propertyPopoverComp
        PropertyPopover {
            Layout.fillWidth: true
        }
    }
    
    content: [
        Repeater {
            model: frameStyleProps.filter(prop => !prop.visible || prop.visible())
            
            delegate: LabeledField {
                label: modelData.name
                visible: !modelData.visible || modelData.visible()
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "combobox" ? comboBoxComp
                                       : modelData.type === "spinbox" ? spinBoxComp
                                       : modelData.type === "property_popover" ? propertyPopoverComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "combobox") {
                                item.model = Qt.binding(modelData.model)
                                item.currentIndex = Qt.binding(function() {
                                    var value = modelData.getter()
                                    var index = item.model.indexOf(value)
                                    return index >= 0 ? index : 0
                                })
                                item.activated.connect(function(index) {
                                    var value = item.model[index]
                                    modelData.setter(value)
                                })
                            } else if (modelData.type === "spinbox") {
                                item.from = modelData.from
                                item.to = modelData.to
                                item.value = Qt.binding(modelData.getter)
                                item.valueChanged.connect(function() {
                                    modelData.setter(item.value)
                                })
                            } else if (modelData.type === "property_popover") {
                                if (modelData.elementFillColor) {
                                    item.elementFillColor = Qt.binding(modelData.elementFillColor)
                                }
                                if (modelData.textGetter) {
                                    item.text = Qt.binding(modelData.textGetter)
                                }
                                if (modelData.placeholderText) {
                                    item.placeholderText = modelData.placeholderText
                                }
                                item.panelRequested.connect(function() {
                                    root.panelSelectorClicked(item, modelData.popoverType)
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}