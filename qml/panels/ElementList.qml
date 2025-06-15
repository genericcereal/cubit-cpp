import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

ScrollView {
    id: root
    
    property var elementModel
    property var selectionManager
    
    ListView {
        id: listView
        model: elementModel
        spacing: 1
        
        delegate: Rectangle {
            width: listView.width
            height: visible ? 40 : 0
            visible: model.elementType !== "Node" && model.elementType !== "Edge"
            color: model.selected ? "#e3f2fd" : (mouseArea.containsMouse ? "#f5f5f5" : "#ffffff")
            antialiasing: true
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Rectangle {
                    Layout.preferredWidth: 4
                    Layout.fillHeight: true
                    color: {
                        switch(model.elementType) {
                            case "Frame": return "#2196F3"
                            case "Text": return "#4CAF50"
                            case "Html": return "#FF9800"
                            case "Variable": return "#9C27B0"
                            default: return "#757575"
                        }
                    }
                    antialiasing: true
                }
                
                Label {
                    Layout.fillWidth: true
                    text: model.name
                    elide: Text.ElideRight
                }
                
                Label {
                    text: model.elementType
                    color: "#666666"
                    font.pixelSize: 11
                }
            }
            
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (selectionManager) {
                        selectionManager.selectOnly(model.element)
                    }
                }
            }
        }
    }
}