import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

ScrollView {
    id: root
    
    property var selectionManager
    property var selectedElement: selectionManager && selectionManager.selectionCount === 1 
                                 ? selectionManager.selectedElements[0] : null
    
    // Helper property to access DesignElement-specific properties
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
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
        
        // Actions section for design elements
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Actions"
            visible: selectedElement && selectedElement.isDesignElement
            
            GridLayout {
                anchors.fill: parent
                columns: 1
                columnSpacing: 10
                rowSpacing: 5
                
                Button {
                    Layout.fillWidth: true
                    text: "Create component"
                    font.pixelSize: 14
                    
                    background: Rectangle {
                        color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                        border.color: "#d0d0d0"
                        border.width: 1
                        radius: 4
                        antialiasing: true
                    }
                    
                    onClicked: {
                        if (selectedDesignElement) {
                            // Call createComponent on the selected design element
                            var component = selectedDesignElement.createComponent()
                            if (component) {
                                ConsoleMessageRepository.addOutput("Component created with ID: " + component.elementId)
                            }
                        }
                    }
                }
            }
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
                
                // Show x/y when no parent, or left/top when parent exists
                Label { 
                    text: selectedElement && selectedElement.isDesignElement && selectedElement.parentId ? "Left:" : "X:" 
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: xSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: {
                            if (!selectedElement) return 0
                            if (selectedElement.isDesignElement && selectedElement.parentId) {
                                return Math.round(selectedElement.left)
                            }
                            return Math.round(selectedElement.x)
                        }
                        onValueModified: {
                            if (!selectedElement) return
                            if (selectedElement.isDesignElement && selectedElement.parentId) {
                                if (value !== Math.round(selectedElement.left)) {
                                    selectedElement.left = value
                                }
                            } else {
                                if (value !== Math.round(selectedElement.x)) {
                                    selectedElement.x = value
                                }
                            }
                        }
                    }
                    
                    // Anchor button for left
                    Button {
                        id: leftAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                        checkable: true
                        checked: selectedElement && selectedElement.isDesignElement && selectedElement.leftAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: leftAnchorButton.checked ? "#4080ff" : (leftAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: leftAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedElement && selectedElement.isDesignElement) {
                                selectedElement.leftAnchored = checked
                            }
                        }
                    }
                }
                
                Label { 
                    text: selectedElement && selectedElement.isDesignElement && selectedElement.parentId ? "Top:" : "Y:" 
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: ySpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: {
                            if (!selectedElement) return 0
                            if (selectedElement.isDesignElement && selectedElement.parentId) {
                                return Math.round(selectedElement.top)
                            }
                            return Math.round(selectedElement.y)
                        }
                        onValueModified: {
                            if (!selectedElement) return
                            if (selectedElement.isDesignElement && selectedElement.parentId) {
                                if (value !== Math.round(selectedElement.top)) {
                                    selectedElement.top = value
                                }
                            } else {
                                if (value !== Math.round(selectedElement.y)) {
                                    selectedElement.y = value
                                }
                            }
                        }
                    }
                    
                    // Anchor button for top
                    Button {
                        id: topAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                        checkable: true
                        checked: selectedElement && selectedElement.isDesignElement && selectedElement.topAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: topAnchorButton.checked ? "#4080ff" : (topAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: topAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedElement && selectedElement.isDesignElement) {
                                selectedElement.topAnchored = checked
                            }
                        }
                    }
                }
                
                // Right anchor (only shown when element has parent)
                Label { 
                    text: "Right:" 
                    visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                    
                    SpinBox {
                        id: rightSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedElement ? Math.round(selectedElement.right) : 0
                        onValueModified: {
                            if (selectedElement && value !== Math.round(selectedElement.right)) {
                                selectedElement.right = value
                            }
                        }
                    }
                    
                    // Anchor button for right
                    Button {
                        id: rightAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedElement && selectedElement.isDesignElement && selectedElement.rightAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: rightAnchorButton.checked ? "#4080ff" : (rightAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: rightAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedElement && selectedElement.isDesignElement) {
                                selectedElement.rightAnchored = checked
                            }
                        }
                    }
                }
                
                // Bottom anchor (only shown when element has parent)
                Label { 
                    text: "Bottom:" 
                    visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
                    
                    SpinBox {
                        id: bottomSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedElement ? Math.round(selectedElement.bottom) : 0
                        onValueModified: {
                            if (selectedElement && value !== Math.round(selectedElement.bottom)) {
                                selectedElement.bottom = value
                            }
                        }
                    }
                    
                    // Anchor button for bottom
                    Button {
                        id: bottomAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedElement && selectedElement.isDesignElement && selectedElement.bottomAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: bottomAnchorButton.checked ? "#4080ff" : (bottomAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: bottomAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedElement && selectedElement.isDesignElement) {
                                selectedElement.bottomAnchored = checked
                            }
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
            title: selectedElement && selectedElement.elementType === "ComponentVariant" ? "Variant Style" : "Frame"
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")
            
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
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
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
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
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
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
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
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
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
                    onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) selectedElement.borderRadius = value
                }
                
                Label { text: "Border Width:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 20
                    value: selectedElement && selectedElement.borderWidth !== undefined ? selectedElement.borderWidth : 0
                    onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) selectedElement.borderWidth = value
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