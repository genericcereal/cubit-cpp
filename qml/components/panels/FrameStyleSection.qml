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
            type: "object_input",
            popoverType: "fill",
            placeholderText: "Add fill",
            valueGetter: () => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("fill"))) && selectedElement.fill !== undefined) {
                    // Check if color is transparent
                    if (selectedElement.fill.a === 0) {
                        return null; // Return null for transparent colors
                    }
                    return selectedElement.fill;
                }
                return null;
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
            name: "Radius",
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
            name: "Border",
            type: "object_input",
            popoverType: "border",
            placeholderText: "Add border...",
            valueGetter: () => {
                if (selectedElement && selectedElement.borderWidth !== undefined && selectedElement.borderWidth > 0) {
                    return {
                        width: selectedElement.borderWidth,
                        color: selectedElement.borderColor || Qt.rgba(0, 0, 0, 1),
                        style: selectedElement.borderStyle || "Solid"
                    }
                }
                return null
            },
            visible: () => PropertyHelpers.canShowBorder(selectedElement, editableProperties)
        },
        {
            name: "Box Shadow",
            type: "object_input",
            popoverType: "boxShadow",
            placeholderText: "Add shadow...",
            valueGetter: () => {
                if (selectedElement && selectedElement.boxShadow && selectedElement.boxShadow.enabled) {
                    return selectedElement.boxShadow
                }
                return null
            },
            visible: () => PropertyHelpers.canShowBoxShadow(selectedElement, editableProperties)
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
        VariableAwareSpinBox {
            Layout.fillWidth: true
            // propertyName will be set by the loader
        }
    }
    
    Component {
        id: propertyPopoverComp
        PropertyPopover {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: objectInputComp
        ObjectInput {
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
                                       : modelData.type === "object_input" ? objectInputComp
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
                                // Map display names to property names
                                var propName = modelData.name.replace(" ", "")  // Remove spaces
                                propName = propName.charAt(0).toLowerCase() + propName.slice(1)  // camelCase
                                item.propertyName = propName  // "borderRadius", "borderWidth"
                                item.elementId = root.selectedElement ? root.selectedElement.elementId : ""
                                
                                // Update elementId when selectedElement changes
                                var updateElementId = function() {
                                    if (item) {
                                        try {
                                            item.elementId = root.selectedElement ? root.selectedElement.elementId : ""
                                        } catch (e) {
                                            // Component may have been destroyed
                                        }
                                    }
                                }
                                root.selectedElementChanged.connect(updateElementId)
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
                            } else if (modelData.type === "object_input") {
                                if (modelData.valueGetter) {
                                    item.value = Qt.binding(modelData.valueGetter)
                                }
                                if (modelData.placeholderText) {
                                    item.placeholderText = modelData.placeholderText
                                }
                                item.displayText = Qt.binding(function() {
                                    var val = item.value
                                    if (modelData.popoverType === "boxShadow") {
                                        // Custom display for box shadow
                                        if (val && val.enabled) {
                                            return `${val.offsetX}px ${val.offsetY}px ${val.blurRadius}px`
                                        }
                                        return ""
                                    } else if (modelData.popoverType === "border") {
                                        // Custom display for border
                                        if (val && val.width > 0) {
                                            return `${val.width}px ${val.style}`
                                        }
                                        return ""
                                    } else if (val && val.a !== undefined && val.a > 0) {
                                        // Color display
                                        var format = selectedElement && selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                                        return item.formatColor(val, format)
                                    }
                                    return ""
                                })
                                item.previewType = (modelData.popoverType === "boxShadow" || modelData.popoverType === "border") ? ObjectInput.PreviewType.None : ObjectInput.PreviewType.Color
                                item.clicked.connect(function() {
                                    // If no value, set default before opening picker
                                    if (!item.value && selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                                        if (modelData.popoverType === "fill") {
                                            selectedElement.fill = Qt.rgba(173/255, 216/255, 230/255, 1) // Default light blue
                                        } else if (modelData.popoverType === "border") {
                                            selectedElement.borderWidth = 1
                                            selectedElement.borderColor = Qt.rgba(0, 0, 0, 1)
                                            selectedElement.borderStyle = "Solid"
                                        } else if (modelData.popoverType === "boxShadow") {
                                            selectedElement.boxShadow = {
                                                offsetX: 0,
                                                offsetY: 4,
                                                blurRadius: 8,
                                                spreadRadius: 0,
                                                color: Qt.rgba(0.267, 0.267, 0.267, 1),
                                                enabled: true
                                            }
                                        }
                                    }
                                    root.panelSelectorClicked(item, modelData.popoverType)
                                })
                                item.cleared.connect(function() {
                                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                                        if (modelData.popoverType === "fill") {
                                            selectedElement.fill = Qt.rgba(0, 0, 0, 0) // Fully transparent
                                        } else if (modelData.popoverType === "border") {
                                            selectedElement.borderWidth = 0
                                        } else if (modelData.popoverType === "boxShadow") {
                                            // Use controller to set box shadow to disabled
                                            if (Application.activeController) {
                                                var shadow = {
                                                    offsetX: 0,
                                                    offsetY: 0,
                                                    blurRadius: 0,
                                                    spreadRadius: 0,
                                                    color: Qt.rgba(0.267, 0.267, 0.267, 1),
                                                    enabled: false
                                                }
                                                Application.activeController.setElementProperty(selectedElement, "boxShadow", shadow)
                                            }
                                        }
                                    }
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}