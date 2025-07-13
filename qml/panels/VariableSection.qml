import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"

ColumnLayout {
    id: root
    spacing: 10
    
    property var selectedElement
    
    property var variableProps: [
        {
            name: "Is Array",
            type: "checkbox",
            getter: () => selectedElement && selectedElement.isArray ? selectedElement.isArray : false,
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Variable") {
                    selectedElement.isArray = v
                }
            }
        },
        {
            name: "Value",
            type: "text",
            getter: () => selectedElement && selectedElement.value !== undefined ? selectedElement.value.toString() : "",
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Variable") {
                    selectedElement.value = v
                }
            },
            visible: () => !(selectedElement && selectedElement.isArray)
        }
    ]
    
    property var componentProps: [
        {
            name: "Accepts children",
            type: "checkbox",
            getter: () => false,
            setter: v => {
                // For now, this doesn't do anything as requested
            }
        }
    ]
    
    property var componentVariantProps: [
        {
            name: "Accepts children",
            type: "checkbox",
            getter: () => selectedElement && selectedElement.elementType === "FrameComponentVariant" ? selectedElement.instancesAcceptChildren : true,
            setter: v => {
                if (selectedElement && selectedElement.elementType === "FrameComponentVariant") {
                    selectedElement.instancesAcceptChildren = v
                }
            }
        }
    ]
    
    Component {
        id: checkboxComp
        CheckBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: textFieldComp
        TextField {
            Layout.fillWidth: true
        }
    }
    
    // Variable-specific properties
    PropertyGroup {
        title: "Variable"
        visible: selectedElement && selectedElement.elementType === "Variable"
        
        content: [
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                
                // Basic properties
                Repeater {
                    model: root.variableProps.filter(prop => !prop.visible || prop.visible())
                    
                    delegate: LabeledField {
                        label: modelData.name
                        
                        delegate: [
                            Loader {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                sourceComponent: modelData.type === "checkbox" ? checkboxComp
                                               : modelData.type === "text" ? textFieldComp
                                               : null
                                
                                onLoaded: {
                                    if (modelData.type === "checkbox") {
                                        item.checked = Qt.binding(modelData.getter)
                                        item.toggled.connect(function() {
                                            modelData.setter(item.checked)
                                        })
                                    } else if (modelData.type === "text") {
                                        item.text = Qt.binding(modelData.getter)
                                        item.textChanged.connect(function() {
                                            modelData.setter(item.text)
                                        })
                                    }
                                }
                            }
                        ]
                    }
                }
                
                // Array values (shown when is array)
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: selectedElement && selectedElement.isArray !== undefined && selectedElement.isArray
                    spacing: 5
                    
                    Label {
                        text: "Array Values:"
                        font.weight: Font.Medium
                    }
                    
                    // Array items
                    Repeater {
                        model: selectedElement && selectedElement.isArray !== undefined && selectedElement.isArray && selectedElement.arrayValues ? selectedElement.arrayValues : []
                        
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            
                            Label {
                                text: "[" + index + "]"
                                Layout.preferredWidth: 40
                            }
                            
                            TextField {
                                Layout.fillWidth: true
                                text: modelData ? modelData.toString() : ""
                                onTextChanged: {
                                    if (selectedElement && selectedElement.elementType === "Variable" && selectedElement.isArray) {
                                        selectedElement.setArrayValue(index, text)
                                    }
                                }
                            }
                            
                            Button {
                                text: "−"
                                Layout.preferredWidth: 30
                                Layout.preferredHeight: 30
                                enabled: selectedElement && selectedElement.arrayLength > 1
                                onClicked: {
                                    if (selectedElement && selectedElement.elementType === "Variable") {
                                        selectedElement.removeArrayValue(index)
                                    }
                                }
                            }
                        }
                    }
                    
                    // Add new value button
                    Button {
                        text: "+ Add Value"
                        Layout.fillWidth: true
                        onClicked: {
                            if (selectedElement && selectedElement.elementType === "Variable" && selectedElement.isArray) {
                                selectedElement.addArrayValue("")
                            }
                        }
                    }
                }
            }
        ]
    }
    
    // Component-specific properties (for frames inside component variants)
    PropertyGroup {
        title: "Component"
        visible: {
            if (!selectedElement || selectedElement.elementType !== "Frame" || !selectedElement.parentId) {
                return false
            }
            if (!Application.activeCanvas) return false
            var model = Application.activeCanvas.elementModel
            if (!model) return false
            
            var parentElement = model.getElementById(selectedElement.parentId)
            return parentElement && parentElement.elementType === "ComponentVariant"
        }
        
        content: [
            Repeater {
                model: root.componentProps
                
                delegate: LabeledField {
                    label: modelData.name
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: checkboxComp
                            
                            onLoaded: {
                                item.checked = Qt.binding(modelData.getter)
                                item.toggled.connect(function() {
                                    modelData.setter(item.checked)
                                })
                            }
                        }
                    ]
                }
            }
        ]
    }
    
    // Component Variant properties
    PropertyGroup {
        title: "Component Variant"
        visible: selectedElement && selectedElement.elementType === "FrameComponentVariant"
        
        content: [
            Repeater {
                model: root.componentVariantProps
                
                delegate: LabeledField {
                    label: modelData.name
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: checkboxComp
                            
                            onLoaded: {
                                item.checked = Qt.binding(modelData.getter)
                                item.toggled.connect(function() {
                                    modelData.setter(item.checked)
                                })
                            }
                        }
                    ]
                }
            }
        ]
    }
}