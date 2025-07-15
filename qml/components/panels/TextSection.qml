import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

PropertyGroup {
    id: root
    title: "Text"
    
    property var selectedElement
    
    signal panelSelectorClicked(var selector, string type)
    
    visible: selectedElement && (selectedElement.elementType === "Text" || selectedElement.elementType === "TextComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("content")))
    
    property var textProps: [
        {
            name: "Content",
            type: "text",
            getter: () => selectedElement && selectedElement.content !== undefined ? selectedElement.content : "",
            setter: v => {
                if (selectedElement && selectedElement.content !== undefined) {
                    selectedElement.content = v
                }
            }
        },
        {
            name: "Font",
            type: "property_popover",
            popoverType: "font",
            textGetter: () => selectedElement && selectedElement.font ? selectedElement.font.family : "System Default",
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.family = v
                    selectedElement.font = newFont
                }
            }
        },
        {
            name: "Size",
            type: "font_spinbox",
            from: 8,
            to: 144,
            getter: () => selectedElement && selectedElement.font ? selectedElement.font.pixelSize : 14,
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.pixelSize = v
                    selectedElement.font = newFont
                }
            }
        },
        {
            name: "Weight",
            type: "combobox",
            getter: () => {
                if (selectedElement && selectedElement.font) {
                    switch (selectedElement.font.weight) {
                        case Font.Thin: return "Thin"
                        case Font.ExtraLight: return "Extra Light"
                        case Font.Light: return "Light"
                        case Font.Normal: return "Normal"
                        case Font.Medium: return "Medium"
                        case Font.DemiBold: return "Demi Bold"
                        case Font.Bold: return "Bold"
                        case Font.ExtraBold: return "Extra Bold"
                        case Font.Black: return "Black"
                        default: return "Normal"
                    }
                }
                return "Normal"
            },
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    switch (v) {
                        case "Thin": newFont.weight = Font.Thin; break
                        case "Extra Light": newFont.weight = Font.ExtraLight; break
                        case "Light": newFont.weight = Font.Light; break
                        case "Normal": newFont.weight = Font.Normal; break
                        case "Medium": newFont.weight = Font.Medium; break
                        case "Demi Bold": newFont.weight = Font.DemiBold; break
                        case "Bold": newFont.weight = Font.Bold; break
                        case "Extra Bold": newFont.weight = Font.ExtraBold; break
                        case "Black": newFont.weight = Font.Black; break
                        default: newFont.weight = Font.Normal
                    }
                    selectedElement.font = newFont
                }
            },
            model: () => ["Thin", "Extra Light", "Light", "Normal", "Medium", "Demi Bold", "Bold", "Extra Bold", "Black"]
        }
    ]
    
    Component {
        id: textFieldComp
        TextField {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: fontSpinBoxComp
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
    
    Component {
        id: comboBoxComp
        ComboBox {
            Layout.fillWidth: true
        }
    }
    
    content: [
        Repeater {
            model: textProps
            
            delegate: LabeledField {
                label: modelData.name
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "text" ? textFieldComp
                                       : modelData.type === "font_spinbox" ? fontSpinBoxComp
                                       : modelData.type === "property_popover" ? propertyPopoverComp
                                       : modelData.type === "combobox" ? comboBoxComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "text") {
                                item.text = Qt.binding(modelData.getter)
                                item.textChanged.connect(function() {
                                    modelData.setter(item.text)
                                })
                            } else if (modelData.type === "font_spinbox") {
                                item.from = modelData.from
                                item.to = modelData.to
                                item.value = Qt.binding(modelData.getter)
                                item.valueModified.connect(function() {
                                    modelData.setter(item.value)
                                })
                            } else if (modelData.type === "property_popover") {
                                if (modelData.textGetter) {
                                    item.text = Qt.binding(modelData.textGetter)
                                }
                                if (modelData.placeholderText) {
                                    item.placeholderText = modelData.placeholderText
                                }
                                item.panelRequested.connect(function() {
                                    root.panelSelectorClicked(item, modelData.popoverType)
                                })
                            } else if (modelData.type === "combobox") {
                                item.model = modelData.model()
                                item.currentIndex = Qt.binding(function() {
                                    var value = modelData.getter()
                                    var idx = modelData.model().indexOf(value)
                                    return idx >= 0 ? idx : 0
                                })
                                item.onCurrentTextChanged.connect(function() {
                                    modelData.setter(item.currentText)
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}