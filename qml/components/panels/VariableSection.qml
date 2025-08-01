import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

ColumnLayout {
    id: root
    spacing: 10
    
    property var selectedElement
    property var canvas: null  // Will be set by parent
    
    property var variableProps: [
        {
            name: "Type",
            type: "combobox",
            getter: () => selectedElement && selectedElement.variableType ? selectedElement.variableType : "string",
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Variable") {
                    selectedElement.variableType = v
                }
            },
            options: PropertyTypeMapper.getAvailableVariableTypes()
        },
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
    
    Component {
        id: comboboxComp
        ComboBox {
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
                                               : modelData.type === "combobox" ? comboboxComp
                                               : null
                                
                                onLoaded: {
                                    if (modelData.type === "checkbox") {
                                        // Set initial value
                                        item.checked = modelData.getter()
                                        
                                        // Create update function
                                        var updateCheckbox = function() {
                                            if (modelData && item) {
                                                item.checked = modelData.getter()
                                            }
                                        }
                                        
                                        // Connect to property changes if the element has a signal
                                        if (root.selectedElement && modelData.property) {
                                            // Try to connect to the property changed signal
                                            try {
                                                root.selectedElement[modelData.property + "Changed"].connect(updateCheckbox)
                                            } catch (e) {
                                                // Signal may not exist, ignore
                                            }
                                        }
                                        item.toggled.connect(function() {
                                            if (modelData && item) {
                                                modelData.setter(item.checked)
                                            }
                                        })
                                    } else if (modelData.type === "text") {
                                        // Set initial value
                                        item.text = modelData.getter()
                                        
                                        // Create update function
                                        var updateText = function() {
                                            if (modelData && item) {
                                                item.text = modelData.getter()
                                            }
                                        }
                                        
                                        // Connect to property changes if the element has a signal
                                        if (root.selectedElement && modelData.property) {
                                            try {
                                                root.selectedElement[modelData.property + "Changed"].connect(updateText)
                                            } catch (e) {
                                                // Signal may not exist, ignore
                                            }
                                        }
                                        item.textChanged.connect(function() {
                                            if (modelData && item) {
                                                modelData.setter(item.text)
                                            }
                                        })
                                    } else if (modelData.type === "combobox") {
                                        // Set initial values
                                        var model = modelData.model ? modelData.model() : modelData.options
                                        item.model = model || []
                                        var value = modelData.getter()
                                        var index = model ? model.indexOf(value) : 0
                                        item.currentIndex = index >= 0 ? index : 0
                                        
                                        // Create update function
                                        var updateCombobox = function() {
                                            if (modelData && item) {
                                                var model = modelData.model ? modelData.model() : modelData.options
                                                item.model = model || []
                                                var value = modelData.getter()
                                                var index = model ? model.indexOf(value) : 0
                                                item.currentIndex = index >= 0 ? index : 0
                                            }
                                        }
                                        
                                        // Connect to property changes if the element has a signal
                                        if (root.selectedElement && modelData.property) {
                                            try {
                                                root.selectedElement[modelData.property + "Changed"].connect(updateCombobox)
                                            } catch (e) {
                                                // Signal may not exist, ignore
                                            }
                                        }
                                        item.activated.connect(function(index) {
                                            if (modelData && item) {
                                                var model = modelData.model ? modelData.model() : modelData.options
                                                if (model) {
                                                    var value = model[index]
                                                    modelData.setter(value)
                                                }
                                            }
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
            if (!canvas) return false
            var model = canvas.elementModel
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
                                // Set initial value
                                item.checked = modelData.getter()
                                
                                // Create update function
                                var updateCheckbox3 = function() {
                                    if (modelData && item) {
                                        item.checked = modelData.getter()
                                    }
                                }
                                
                                // Connect to property changes if the element has a signal
                                if (root.selectedElement && modelData.property) {
                                    try {
                                        root.selectedElement[modelData.property + "Changed"].connect(updateCheckbox3)
                                    } catch (e) {
                                        // Signal may not exist, ignore
                                    }
                                }
                                
                                item.toggled.connect(function() {
                                    if (modelData && item) {
                                        modelData.setter(item.checked)
                                    }
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
                                // Set initial value
                                item.checked = modelData.getter()
                                
                                // Create update function
                                var updateCheckbox3 = function() {
                                    if (modelData && item) {
                                        item.checked = modelData.getter()
                                    }
                                }
                                
                                // Connect to property changes if the element has a signal
                                if (root.selectedElement && modelData.property) {
                                    try {
                                        root.selectedElement[modelData.property + "Changed"].connect(updateCheckbox3)
                                    } catch (e) {
                                        // Signal may not exist, ignore
                                    }
                                }
                                
                                item.toggled.connect(function() {
                                    if (modelData && item) {
                                        modelData.setter(item.checked)
                                    }
                                })
                            }
                        }
                    ]
                }
            }
        ]
    }
}