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
            }
        }
        
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Position & Size"
            visible: selectedElement
            
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
        
        Item {
            Layout.fillHeight: true
        }
    }
}