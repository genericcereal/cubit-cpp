import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

PropertyGroup {
    id: root
    title: "Position"
    
    property var selectedElement
    property var editableProperties: []
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    visible: selectedElement && selectedElement.isDesignElement === true && selectedElement.parentId !== undefined && selectedElement.parentId !== ""
    
    property var positionProps: [
        {
            name: "Position",
            type: "combobox",
            getter: () => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text" || selectedElement.elementType === "WebTextInput")) {
                    switch (selectedElement.position) {
                        case 0: return "Relative"
                        case 1: return "Absolute"
                        case 2: return "Fixed"
                        default: return "Relative"
                    }
                }
                return "Relative"
            },
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text" || selectedElement.elementType === "WebTextInput")) {
                    const index = ["Relative", "Absolute", "Fixed"].indexOf(v)
                    selectedElement.position = index >= 0 ? index : 0
                }
            },
            model: () => ["Relative", "Absolute", "Fixed"],
            visible: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text" || selectedElement.elementType === "WebTextInput")
        },
        {
            name: "Left",
            type: "spinbox_anchor",
            valueGetter: () => selectedDesignElement && selectedDesignElement.left !== undefined ? Math.round(selectedDesignElement.left) : 0,
            valueSetter: v => {
                if (selectedDesignElement && selectedDesignElement.left !== undefined) {
                    selectedDesignElement.left = v
                }
            },
            anchorGetter: () => selectedDesignElement && selectedDesignElement.leftAnchored,
            anchorSetter: (checked, value) => {
                if (selectedDesignElement) {
                    if (checked) {
                        selectedDesignElement.left = value
                    }
                    selectedDesignElement.leftAnchored = checked
                }
            }
        },
        {
            name: "Top",
            type: "spinbox_anchor",
            valueGetter: () => selectedDesignElement && selectedDesignElement.top !== undefined ? Math.round(selectedDesignElement.top) : 0,
            valueSetter: v => {
                if (selectedDesignElement && selectedDesignElement.top !== undefined) {
                    selectedDesignElement.top = v
                }
            },
            anchorGetter: () => selectedDesignElement && selectedDesignElement.topAnchored,
            anchorSetter: (checked, value) => {
                if (selectedDesignElement) {
                    if (checked) {
                        selectedDesignElement.top = value
                    }
                    selectedDesignElement.topAnchored = checked
                }
            }
        },
        {
            name: "Right",
            type: "spinbox_anchor",
            valueGetter: () => selectedDesignElement && selectedDesignElement.right !== undefined ? Math.round(selectedDesignElement.right) : 0,
            valueSetter: v => {
                if (selectedDesignElement && selectedDesignElement.right !== undefined) {
                    selectedDesignElement.right = v
                }
            },
            anchorGetter: () => selectedDesignElement && selectedDesignElement.rightAnchored,
            anchorSetter: (checked, value) => {
                if (selectedDesignElement) {
                    if (checked) {
                        selectedDesignElement.right = value
                    }
                    selectedDesignElement.rightAnchored = checked
                }
            }
        },
        {
            name: "Bottom",
            type: "spinbox_anchor",
            valueGetter: () => selectedDesignElement && selectedDesignElement.bottom !== undefined ? Math.round(selectedDesignElement.bottom) : 0,
            valueSetter: v => {
                if (selectedDesignElement && selectedDesignElement.bottom !== undefined) {
                    selectedDesignElement.bottom = v
                }
            },
            anchorGetter: () => selectedDesignElement && selectedDesignElement.bottomAnchored,
            anchorSetter: (checked, value) => {
                if (selectedDesignElement) {
                    if (checked) {
                        selectedDesignElement.bottom = value
                    }
                    selectedDesignElement.bottomAnchored = checked
                }
            }
        }
    ]
    
    Component {
        id: comboBoxComp
        ComboBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: spinBoxAnchorComp
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: spinBox
                Layout.fillWidth: true
                from: -9999
                to: 9999
            }
            
            Button {
                id: anchorButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                checkable: true
                text: "⚓"
                font.pixelSize: 16
                // Use native button styling - checked state will be handled by the native style
            }
        }
    }
    
    content: [
        Repeater {
            model: positionProps.filter(prop => !prop.visible || prop.visible())
            
            delegate: LabeledField {
                label: modelData.name
                visible: !modelData.visible || modelData.visible()
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "combobox" ? comboBoxComp
                                       : modelData.type === "spinbox_anchor" ? spinBoxAnchorComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "combobox") {
                                item.model = Qt.binding(function() { 
                                    if (!modelData || !item) return []
                                    return modelData.model() 
                                })
                                item.currentIndex = Qt.binding(function() {
                                    if (!modelData || !item || !item.model) return 0
                                    var value = modelData.getter()
                                    var index = item.model.indexOf(value)
                                    return index >= 0 ? index : 0
                                })
                                item.activated.connect(function(index) {
                                    if (modelData && item && item.model) {
                                        var value = item.model[index]
                                        modelData.setter(value)
                                    }
                                })
                            } else if (modelData.type === "spinbox_anchor") {
                                var spinBox = item.children[0]
                                var anchorButton = item.children[1]
                                
                                spinBox.value = Qt.binding(function() { 
                                    if (!modelData || !spinBox) return 0
                                    return modelData.valueGetter() 
                                })
                                spinBox.valueModified.connect(function() {
                                    if (modelData && spinBox && spinBox.value !== modelData.valueGetter()) {
                                        modelData.valueSetter(spinBox.value)
                                    }
                                })
                                
                                anchorButton.checked = Qt.binding(function() { 
                                    if (!modelData || !anchorButton) return false
                                    return modelData.anchorGetter() 
                                })
                                anchorButton.toggled.connect(function() {
                                    if (modelData && anchorButton && spinBox) {
                                        modelData.anchorSetter(anchorButton.checked, spinBox.value)
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