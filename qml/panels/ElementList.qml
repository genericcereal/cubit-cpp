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
                drag.target: dragItem
                
                onClicked: {
                    if (selectionManager) {
                        selectionManager.selectOnly(model.element)
                    }
                }
                
                onPressed: (mouse) => {
                    dragItem.elementType = model.elementType
                    dragItem.elementName = model.name
                    dragItem.elementId = model.element.elementId
                    // Ensure drag item starts at the correct position
                    var globalPos = mapToItem(dragItem.parent, mouse.x, mouse.y)
                    dragItem.x = globalPos.x - dragItem.width / 2
                    dragItem.y = globalPos.y - dragItem.height / 2
                }
                
                onReleased: {
                    if (drag.active) {
                        dragItem.Drag.drop()
                    }
                    // Reset position after drag
                    dragItem.x = 0
                    dragItem.y = 0
                }
            }
            
            // Drag preview that looks like a node
            Rectangle {
                id: dragItem
                width: 200
                height: 100
                visible: mouseArea.drag.active
                opacity: 0.8
                parent: root.parent // Parent to the root's parent to allow movement outside list bounds
                
                property string elementType: ""
                property string elementName: ""
                property string elementId: ""
                
                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2
                
                // Node-like appearance
                color: "#f5f5f5"
                border.color: "#ddd"
                border.width: 1
                radius: 4
                
                // Header
                Rectangle {
                    id: header
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 30
                    color: "#9C27B0"  // Purple for Variable nodes (all elements create Variable nodes)
                    radius: 4
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 4
                        color: parent.color
                    }
                    
                    Label {
                        anchors.centerIn: parent
                        text: dragItem.elementName  // All elements create Variable nodes
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}