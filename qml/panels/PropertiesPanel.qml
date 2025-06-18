import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    
    property var selectionManager
    property var selectedElement: selectionManager && selectionManager.selectionCount === 1 
                                 ? selectionManager.selectedElements[0] : null
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
        // No selection message
        Label {
            Layout.fillWidth: true
            Layout.margins: 20
            text: "No element selected"
            visible: !selectedElement
            horizontalAlignment: Text.AlignHCenter
            color: "#666666"
        }
        
        // Properties when element is selected
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "General"
            visible: selectedElement
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Name:" }
                TextField {
                    Layout.fillWidth: true
                    text: selectedElement ? selectedElement.name : ""
                    onTextChanged: if (selectedElement) selectedElement.name = text
                }
                
                Label { text: "Type:" }
                Label {
                    text: selectedElement ? selectedElement.elementType : ""
                    color: "#666666"
                }
                
                Label { text: "ID:" }
                Label {
                    text: selectedElement ? selectedElement.elementId : ""
                    color: "#666666"
                }
                
                Label { 
                    text: "Parent ID:" 
                    visible: selectedElement && selectedElement.isDesignElement
                }
                Label {
                    text: selectedElement && selectedElement.parentId ? selectedElement.parentId : "None"
                    color: "#666666"
                    visible: selectedElement && selectedElement.isDesignElement
                }
            }
        }
        
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Position & Size"
            visible: selectedElement && selectedElement.isVisual
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "X:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement ? selectedElement.x : 0
                    onValueChanged: if (selectedElement) selectedElement.x = value
                }
                
                Label { text: "Y:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement ? selectedElement.y : 0
                    onValueChanged: if (selectedElement) selectedElement.y = value
                }
                
                Label { text: "Width:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? selectedElement.width : 0
                    onValueChanged: if (selectedElement) selectedElement.width = value
                }
                
                Label { text: "Height:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? selectedElement.height : 0
                    onValueChanged: if (selectedElement) selectedElement.height = value
                }
            }
        }
        
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
        
        Item {
            Layout.fillHeight: true
        }
    }
}