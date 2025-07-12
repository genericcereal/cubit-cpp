import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

ColumnLayout {
    id: root
    spacing: 10
    
    property var selectedElement
    
    // Variable-specific properties
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
        title: "Variable"
        visible: selectedElement && selectedElement.elementType === "Variable"
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            // Array toggle
            RowLayout {
                Layout.fillWidth: true
                
                Label { text: "Is Array:" }
                CheckBox {
                    id: isArrayCheckBox
                    checked: selectedElement && selectedElement.isArray ? selectedElement.isArray : false
                    onToggled: if (selectedElement && selectedElement.elementType === "Variable") selectedElement.isArray = checked
                }
            }
            
            // Single value field (shown when not array)
            RowLayout {
                Layout.fillWidth: true
                visible: !isArrayCheckBox.checked
                
                Label { 
                    text: "Value:" 
                    Layout.preferredWidth: 50
                }
                TextField {
                    Layout.fillWidth: true
                    text: selectedElement && selectedElement.value !== undefined ? selectedElement.value.toString() : ""
                    onTextChanged: if (selectedElement && selectedElement.elementType === "Variable") selectedElement.value = text
                }
            }
            
            // Array values (shown when is array)
            ColumnLayout {
                Layout.fillWidth: true
                visible: isArrayCheckBox.checked
                spacing: 5
                
                Label {
                    text: "Array Values:"
                    font.weight: Font.Medium
                }
                
                // Array items
                Repeater {
                    model: selectedElement && selectedElement.isArray && selectedElement.arrayValues ? selectedElement.arrayValues : []
                    
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
    }
    
    // Component-specific properties (for frames inside component variants)
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
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
        
        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            
            Label { text: "Accepts children:" }
            CheckBox {
                Layout.fillWidth: true
                checked: false
                onToggled: {
                    // For now, this doesn't do anything as requested
                }
            }
        }
    }
    
    // Component Variant properties
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
        title: "Component Variant"
        visible: selectedElement && selectedElement.elementType === "FrameComponentVariant"
        
        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            
            Label { text: "Accepts children:" }
            CheckBox {
                Layout.fillWidth: true
                checked: selectedElement && selectedElement.elementType === "FrameComponentVariant" ? selectedElement.instancesAcceptChildren : true
                onToggled: {
                    if (selectedElement && selectedElement.elementType === "FrameComponentVariant") {
                        selectedElement.instancesAcceptChildren = checked
                    }
                }
            }
        }
    }
}