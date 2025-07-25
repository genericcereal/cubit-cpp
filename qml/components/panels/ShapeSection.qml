import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

ColumnLayout {
    id: root
    spacing: 0
    
    property var selectedElement
    
    signal panelSelectorClicked(var selector, string type)
    
    visible: selectedElement && selectedElement.elementType === "Shape"
    
    property var shapeProps: [
        {
            name: "Shape Type",
            type: "label",
            getter: () => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    switch (selectedElement.shapeType) {
                        case 0: return "Square"
                        case 1: return "Triangle"
                        case 2: return "Line"
                        default: return "Unknown"
                    }
                }
                return ""
            }
        },
        {
            name: "Edge Width",
            type: "spinbox",
            from: 0,
            to: 20,
            getter: () => selectedElement && selectedElement.edgeWidth !== undefined ? selectedElement.edgeWidth : 2,
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    selectedElement.edgeWidth = v
                }
            }
        },
        {
            name: "Edge Color",
            type: "object_input",
            popoverType: "edgeColor",
            placeholderText: "Add edge color",
            valueGetter: () => {
                if (selectedElement && selectedElement.elementType === "Shape" && selectedElement.edgeColor !== undefined) {
                    // Check if color is transparent
                    if (selectedElement.edgeColor.a === 0) {
                        return null; // Return null for transparent colors
                    }
                    return selectedElement.edgeColor;
                }
                return null;
            }
        },
        {
            name: "Fill",
            type: "object_input",
            popoverType: "fillColor",
            placeholderText: "Add fill",
            valueGetter: () => {
                if (selectedElement && selectedElement.elementType === "Shape" && selectedElement.fillColor !== undefined) {
                    // Check if color is transparent
                    if (selectedElement.fillColor.a === 0) {
                        return null; // Return null for transparent colors
                    }
                    return selectedElement.fillColor;
                }
                return null;
            }
        }
    ]
    
    Component {
        id: labelComp
        Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
    
    Component {
        id: spinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: checkBoxComp
        CheckBox {
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
        id: objectInputComp
        ObjectInput {
            Layout.fillWidth: true
        }
    }
    
    PropertyGroup {
        title: "Shape"
        Layout.fillWidth: true
        
        content: [
            Repeater {
                model: shapeProps.filter(prop => !prop.visible || prop.visible())
                
                delegate: LabeledField {
                    label: modelData.name
                    visible: !modelData.visible || modelData.visible()
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: modelData.type === "label" ? labelComp
                                           : modelData.type === "spinbox" ? spinBoxComp
                                           : modelData.type === "checkbox" ? checkBoxComp
                                           : modelData.type === "property_popover" ? propertyPopoverComp
                                           : modelData.type === "object_input" ? objectInputComp
                                           : null
                            
                            onLoaded: {
                                if (modelData.type === "label") {
                                    item.text = Qt.binding(modelData.getter)
                                } else if (modelData.type === "spinbox") {
                                    item.from = modelData.from
                                    item.to = modelData.to
                                    item.value = Qt.binding(modelData.getter)
                                    item.valueChanged.connect(function() {
                                        modelData.setter(item.value)
                                    })
                                } else if (modelData.type === "checkbox") {
                                    item.checked = Qt.binding(modelData.getter)
                                    item.toggled.connect(function() {
                                        modelData.setter(item.checked)
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
                                        if (val && val.a !== undefined && val.a > 0) {
                                            var format = selectedElement && selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                                            return item.formatColor(val, format)
                                        }
                                        return ""
                                    })
                                    item.previewType = ObjectInput.PreviewType.Color
                                    item.clicked.connect(function() {
                                        // If no value, set default color before opening picker
                                        if (!item.value && selectedElement && selectedElement.elementType === "Shape") {
                                            if (modelData.popoverType === "edgeColor") {
                                                selectedElement.edgeColor = Qt.rgba(0, 0, 0, 1) // Default black for edges
                                            } else if (modelData.popoverType === "fillColor") {
                                                if (selectedElement.shapeType === 2) {
                                                    // For Lines, default to transparent
                                                    selectedElement.fillColor = Qt.rgba(0, 0, 0, 0) // Default transparent for line fill
                                                } else {
                                                    // For other shapes, default to white
                                                    selectedElement.fillColor = Qt.rgba(1, 1, 1, 1) // Default white for fill
                                                }
                                            }
                                        }
                                        root.panelSelectorClicked(item, modelData.popoverType)
                                    })
                                    item.cleared.connect(function() {
                                        if (selectedElement && selectedElement.elementType === "Shape") {
                                            if (modelData.popoverType === "edgeColor") {
                                                selectedElement.edgeColor = Qt.rgba(0, 0, 0, 0) // Fully transparent
                                            } else if (modelData.popoverType === "fillColor") {
                                                selectedElement.fillColor = Qt.rgba(0, 0, 0, 0) // Fully transparent
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
    
    PropertyGroup {
        title: "Joints"
        Layout.fillWidth: true
        visible: selectedElement && selectedElement.elementType === "Shape" && selectedElement.joints && selectedElement.joints.length > 0
        
        content: [
            Repeater {
                model: selectedElement && selectedElement.elementType === "Shape" ? selectedElement.joints : []
                
                delegate: CollapsibleGroup {
                    groupTitle: "Joint " + (index + 1)
                    
                    content: [
                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            
                            Label {
                                text: "X:"
                                font.pixelSize: 12
                            }
                            
                            TextField {
                                Layout.preferredWidth: 80
                                Layout.fillWidth: true
                                text: modelData.x !== undefined ? modelData.x.toFixed(2) : "0.00"
                                selectByMouse: true
                                
                                onEditingFinished: {
                                    var newX = parseFloat(text)
                                    if (!isNaN(newX) && selectedElement && selectedElement.elementType === "Shape") {
                                        var joints = selectedElement.joints.slice() // Create a copy of the array
                                        if (joints && index < joints.length) {
                                            joints[index] = Qt.point(newX, joints[index].y)
                                            selectedElement.setJoints(joints)
                                        }
                                    }
                                }
                            }
                            
                            Label {
                                text: "Y:"
                                font.pixelSize: 12
                            }
                            
                            TextField {
                                Layout.preferredWidth: 80
                                Layout.fillWidth: true
                                text: modelData.y !== undefined ? modelData.y.toFixed(2) : "0.00"
                                selectByMouse: true
                                
                                onEditingFinished: {
                                    var newY = parseFloat(text)
                                    if (!isNaN(newY) && selectedElement && selectedElement.elementType === "Shape") {
                                        var joints = selectedElement.joints.slice() // Create a copy of the array
                                        if (joints && index < joints.length) {
                                            joints[index] = Qt.point(joints[index].x, newY)
                                            selectedElement.setJoints(joints)
                                        }
                                    }
                                }
                            }
                        }
                    ]
                }
            }
        ]
    }
}