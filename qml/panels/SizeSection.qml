import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"

ColumnLayout {
    id: root
    spacing: 0
    
    property var selectedElement
    property var editableProperties: []
    
    property var positionSizeProps: [
            {
                name: "X",
                type: "spinbox",
                from: -9999,
                to: 9999,
                getter: () => selectedElement && selectedElement.isVisual ? Math.round(selectedElement.x) : 0,
                setter: v => {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (v === undefined || isNaN(v)) return
                    if (v !== Math.round(selectedElement.x)) {
                        selectedElement.x = v
                    }
                }
            },
            {
                name: "Y",
                type: "spinbox",
                from: -9999,
                to: 9999,
                getter: () => selectedElement && selectedElement.isVisual ? Math.round(selectedElement.y) : 0,
                setter: v => {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (v === undefined || isNaN(v)) return
                    if (v !== Math.round(selectedElement.y)) {
                        selectedElement.y = v
                    }
                }
            },
            {
                name: "Width",
                type: "spinbox_combobox",
                from: 1,
                to: 9999,
                valueGetter: () => selectedElement && selectedElement.isVisual ? Math.round(selectedElement.width) : 0,
                valueSetter: v => {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (v === undefined || isNaN(v)) return
                    if (v !== Math.round(selectedElement.width)) {
                        selectedElement.width = v
                    }
                },
                comboGetter: () => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.widthType) {
                            case 0: return "fixed"
                            case 1: return "relative"
                            case 2: return "fill"
                            case 3: return "fit content"
                            case 4: return "viewport"
                            default: return "fixed"
                        }
                    }
                    return "fixed"
                },
                comboSetter: v => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        const index = ["fixed", "relative", "fill", "fit content", "viewport"].indexOf(v)
                        selectedElement.widthType = index >= 0 ? index : 0
                    }
                },
                comboModel: () => ["fixed", "relative", "fill", "fit content", "viewport"],
                comboVisible: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
            },
            {
                name: "Height",
                type: "spinbox_combobox",
                from: 1,
                to: 9999,
                valueGetter: () => selectedElement && selectedElement.isVisual ? Math.round(selectedElement.height) : 0,
                valueSetter: v => {
                    if (!selectedElement || !selectedElement.isVisual) return
                    if (v === undefined || isNaN(v)) return
                    if (v !== Math.round(selectedElement.height)) {
                        selectedElement.height = v
                    }
                },
                comboGetter: () => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.heightType) {
                            case 0: return "fixed"
                            case 1: return "relative"
                            case 2: return "fill"
                            case 3: return "fit content"
                            case 4: return "viewport"
                            default: return "fixed"
                        }
                    }
                    return "fixed"
                },
                comboSetter: v => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        const index = ["fixed", "relative", "fill", "fit content", "viewport"].indexOf(v)
                        selectedElement.heightType = index >= 0 ? index : 0
                    }
                },
                comboModel: () => ["fixed", "relative", "fill", "fit content", "viewport"],
                comboVisible: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
            }
        ]
    
    property var sizeProps: [
            {
                name: "Width",
                type: "spinbox_combobox",
                from: 1,
                to: 9999,
                valueGetter: () => selectedElement ? Math.round(selectedElement.width) : 0,
                valueSetter: v => {
                    if (selectedElement && v !== Math.round(selectedElement.width)) {
                        selectedElement.width = v
                    }
                },
                comboGetter: () => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.widthType) {
                            case 0: return "fixed"
                            case 1: return "relative"
                            case 2: return "fill"
                            case 3: return "fit content"
                            case 4: return "viewport"
                            default: return "fixed"
                        }
                    }
                    return "fixed"
                },
                comboSetter: v => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        const index = ["fixed", "relative", "fill", "fit content", "viewport"].indexOf(v)
                        selectedElement.widthType = index >= 0 ? index : 0
                    }
                },
                comboModel: () => ["fixed", "relative", "fill", "fit content", "viewport"],
                comboVisible: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
            },
            {
                name: "Height",
                type: "spinbox_combobox",
                from: 1,
                to: 9999,
                valueGetter: () => selectedElement ? Math.round(selectedElement.height) : 0,
                valueSetter: v => {
                    if (selectedElement && v !== Math.round(selectedElement.height)) {
                        selectedElement.height = v
                    }
                },
                comboGetter: () => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.heightType) {
                            case 0: return "fixed"
                            case 1: return "relative"
                            case 2: return "fill"
                            case 3: return "fit content"
                            case 4: return "viewport"
                            default: return "fixed"
                        }
                    }
                    return "fixed"
                },
                comboSetter: v => {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")) {
                        const index = ["fixed", "relative", "fill", "fit content", "viewport"].indexOf(v)
                        selectedElement.heightType = index >= 0 ? index : 0
                    }
                },
                comboModel: () => ["fixed", "relative", "fill", "fit content", "viewport"],
                comboVisible: () => selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentInstance")
            }
        ]
    
    Component {
        id: spinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: spinBoxComboComp
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            SpinBox {
                id: spinBox
                Layout.fillWidth: true
            }
            
            ComboBox {
                id: comboBox
                Layout.preferredWidth: 100
            }
        }
    }
    
    // Position & Size section - for elements without a parent
    PropertyGroup {
        title: "Position & Size"
        visible: selectedElement && selectedElement.isVisual && !(selectedElement.isDesignElement && selectedElement.parentId)
        
        content: [
            Repeater {
                model: root.positionSizeProps
                
                delegate: LabeledField {
                    label: modelData.name
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: modelData.type === "spinbox" ? spinBoxComp
                                           : modelData.type === "spinbox_combobox" ? spinBoxComboComp
                                           : null
                            
                            onLoaded: {
                                if (modelData.type === "spinbox") {
                                    item.from = modelData.from
                                    item.to = modelData.to
                                    item.value = Qt.binding(modelData.getter)
                                    item.valueModified.connect(function() {
                                        modelData.setter(item.value)
                                    })
                                } else if (modelData.type === "spinbox_combobox") {
                                    var spinBox = item.children[0]
                                    var comboBox = item.children[1]
                                    
                                    spinBox.from = modelData.from
                                    spinBox.to = modelData.to
                                    spinBox.value = Qt.binding(modelData.valueGetter)
                                    spinBox.valueModified.connect(function() {
                                        modelData.valueSetter(spinBox.value)
                                    })
                                    
                                    comboBox.model = Qt.binding(modelData.comboModel)
                                    comboBox.visible = Qt.binding(modelData.comboVisible)
                                    comboBox.currentIndex = Qt.binding(function() {
                                        var value = modelData.comboGetter()
                                        var index = comboBox.model.indexOf(value)
                                        return index >= 0 ? index : 0
                                    })
                                    comboBox.activated.connect(function(index) {
                                        var value = comboBox.model[index]
                                        modelData.comboSetter(value)
                                    })
                                }
                            }
                        }
                    ]
                }
            }
        ]
    }
    
    // Size section - for DesignElements with a parent
    PropertyGroup {
        title: "Size"
        visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
        
        content: [
            Repeater {
                model: root.sizeProps
                
                delegate: LabeledField {
                    label: modelData.name
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: spinBoxComboComp
                            
                            onLoaded: {
                                var spinBox = item.children[0]
                                var comboBox = item.children[1]
                                
                                spinBox.from = modelData.from
                                spinBox.to = modelData.to
                                spinBox.value = Qt.binding(modelData.valueGetter)
                                spinBox.valueModified.connect(function() {
                                    modelData.valueSetter(spinBox.value)
                                })
                                
                                comboBox.model = Qt.binding(modelData.comboModel)
                                comboBox.visible = Qt.binding(modelData.comboVisible)
                                comboBox.currentIndex = Qt.binding(function() {
                                    var value = modelData.comboGetter()
                                    var index = comboBox.model.indexOf(value)
                                    return index >= 0 ? index : 0
                                })
                                comboBox.activated.connect(function(index) {
                                    var value = comboBox.model[index]
                                    modelData.comboSetter(value)
                                })
                            }
                        }
                    ]
                }
            }
        ]
    }
}