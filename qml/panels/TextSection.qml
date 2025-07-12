import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

GroupBox {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    title: "Text"
    
    property var selectedElement
    
    visible: selectedElement && (selectedElement.elementType === "Text" || selectedElement.elementType === "TextVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("content")))
    
    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: 10
        rowSpacing: 5
        
        Label { text: "Content:" }
        TextField {
            Layout.fillWidth: true
            text: selectedElement && selectedElement.content !== undefined ? selectedElement.content : ""
            onTextChanged: if (selectedElement && selectedElement.content !== undefined) selectedElement.content = text
        }
        
        Label { text: "Size:" }
        SpinBox {
            id: fontSizeSpinBox
            Layout.fillWidth: true
            from: 8
            to: 144
            
            Binding {
                target: fontSizeSpinBox
                property: "value"
                value: selectedElement && selectedElement.font ? selectedElement.font.pixelSize : 14
                when: !fontSizeSpinBox.activeFocus
            }
            
            onValueModified: {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.pixelSize = value
                    selectedElement.font = newFont
                }
            }
        }
    }
}