import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

ColumnLayout {
    id: root
    spacing: 5
    
    property var selectedElement
    property var editableProperties: []
    
    
    // Flex Layout checkbox for Frame
    CheckBox {
        id: flexCheckBox
        Layout.fillWidth: true
        Layout.margins: 10
        Layout.leftMargin: 10
        Layout.bottomMargin: 0
        text: "Flex Layout"
        visible: PropertyHelpers.canShowFlex(selectedElement, editableProperties)
        checked: selectedElement && selectedElement.flex !== undefined ? selectedElement.flex : false
        onToggled: {
            if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("flex")))) {
                selectedElement.flex = checked
            }
        }
    }
    
    // Flex Layout properties GroupBox
    GroupBox {
        Layout.fillWidth: true
        Layout.leftMargin: 10
        Layout.rightMargin: 10
        Layout.topMargin: 5
        Layout.bottomMargin: 10
        title: ""
        visible: PropertyHelpers.canShowFlex(selectedElement, editableProperties) && flexCheckBox.checked
        
        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 10
            rowSpacing: 5
            
            Label { 
                text: "Orientation:" 
                visible: PropertyHelpers.canShowOrientation(selectedElement, editableProperties)
            }
            ComboBox {
                Layout.fillWidth: true
                visible: PropertyHelpers.canShowOrientation(selectedElement, editableProperties)
                model: ["Row", "Column"]
                currentIndex: {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.orientation) {
                            case 0: return 0
                            case 1: return 1
                            default: return 0
                        }
                    }
                    return 0
                }
                onActivated: function(index) {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        selectedElement.orientation = index
                    }
                }
            }
            
            Label { 
                text: "Gap:" 
                visible: PropertyHelpers.canShowGap(selectedElement, editableProperties)
            }
            VariableAwareSpinBox {
                Layout.fillWidth: true
                visible: PropertyHelpers.canShowGap(selectedElement, editableProperties)
                from: 0
                to: 100
                value: selectedElement && selectedElement.gap !== undefined ? Math.round(selectedElement.gap) : 0
                propertyName: "gap"
                elementId: selectedElement ? selectedElement.elementId : ""
                onValueChanged: {
                    if (value !== undefined && selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        selectedElement.gap = value
                    }
                }
            }
            
            Label { 
                text: "Justify:" 
                visible: PropertyHelpers.canShowJustify(selectedElement, editableProperties)
            }
            ComboBox {
                Layout.fillWidth: true
                visible: PropertyHelpers.canShowJustify(selectedElement, editableProperties)
                model: ["Start", "End", "Center", "Space Between", "Space Around", "Space Evenly"]
                currentIndex: {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.justify) {
                            case 0: return 0
                            case 1: return 1
                            case 2: return 2
                            case 3: return 3
                            case 4: return 4
                            case 5: return 5
                            default: return 0
                        }
                    }
                    return 0
                }
                onActivated: function(index) {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        selectedElement.justify = index
                    }
                }
            }
            
            Label { 
                text: "Align:" 
                visible: PropertyHelpers.canShowAlign(selectedElement, editableProperties)
            }
            ComboBox {
                Layout.fillWidth: true
                visible: PropertyHelpers.canShowAlign(selectedElement, editableProperties)
                model: ["Start", "End", "Center", "Baseline", "Stretch"]
                currentIndex: {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        switch (selectedElement.align) {
                            case 0: return 0
                            case 1: return 1
                            case 2: return 2
                            case 3: return 3
                            case 4: return 4
                            default: return 0
                        }
                    }
                    return 0
                }
                onActivated: function(index) {
                    if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                        selectedElement.align = index
                    }
                }
            }
            
            Label { text: "Wrap:" }
            ComboBox {
                Layout.fillWidth: true
                model: ["Yes", "No"]
                currentIndex: 1
            }
            
            Label { text: "Padding:" }
            SpinBox {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: 0
            }
        }
    }
}