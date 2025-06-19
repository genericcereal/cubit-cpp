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
                    id: xSpinBox
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.x) : 0
                    onValueModified: {
                        if (selectedElement && value !== Math.round(selectedElement.x)) {
                            selectedElement.x = value
                        }
                    }
                }
                
                Label { text: "Y:" }
                SpinBox {
                    id: ySpinBox
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.y) : 0
                    onValueModified: {
                        if (selectedElement && value !== Math.round(selectedElement.y)) {
                            selectedElement.y = value
                        }
                    }
                }
                
                Label { text: "Width:" }
                SpinBox {
                    id: widthSpinBox
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.width) : 0
                    onValueModified: {
                        if (selectedElement && value !== Math.round(selectedElement.width)) {
                            selectedElement.width = value
                        }
                    }
                }
                
                Label { text: "Height:" }
                SpinBox {
                    id: heightSpinBox
                    Layout.fillWidth: true
                    from: 1
                    to: 9999
                    value: selectedElement ? Math.round(selectedElement.height) : 0
                    onValueModified: {
                        if (selectedElement && value !== Math.round(selectedElement.height)) {
                            selectedElement.height = value
                        }
                    }
                }
            }
        }
        
        // Frame-specific properties
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Frame"
            visible: selectedElement && selectedElement.elementType === "Frame"
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Fill:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Light Blue", "Dark Blue", "Green", "Red"]
                    currentIndex: {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            // Map Frame.FillColor enum values to combo box indices
                            switch (selectedElement.fillColor) {
                                case 0: return 0  // LightBlue
                                case 1: return 1  // DarkBlue
                                case 2: return 2  // Green
                                case 3: return 3  // Red
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            // Map combo box index to Frame.FillColor enum value
                            selectedElement.fillColor = index
                        }
                    }
                }
                
                Label { text: "Overflow:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Hidden", "Scroll", "Visible"]
                    currentIndex: {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            // Map Frame.OverflowMode enum values to combo box indices
                            switch (selectedElement.overflow) {
                                case 0: return 0  // Hidden
                                case 1: return 1  // Scroll
                                case 2: return 2  // Visible
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            // Map combo box index to Frame.OverflowMode enum value
                            selectedElement.overflow = index
                        }
                    }
                }
                
                Label { text: "Border Radius:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: selectedElement && selectedElement.borderRadius !== undefined ? selectedElement.borderRadius : 0
                    onValueChanged: if (selectedElement && selectedElement.elementType === "Frame") selectedElement.borderRadius = value
                }
                
                Label { text: "Border Width:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 20
                    value: selectedElement && selectedElement.borderWidth !== undefined ? selectedElement.borderWidth : 0
                    onValueChanged: if (selectedElement && selectedElement.elementType === "Frame") selectedElement.borderWidth = value
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