import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    
    // Properties
    property var options: []  // Array of {text: string, value: any} objects
    property int preferredWidth: 150
    property int itemHeight: 36
    property int padding: 8
    
    // Signals
    signal optionSelected(var value)
    signal dismissed()
    
    // Dimensions
    width: preferredWidth
    height: listView.contentHeight + (padding * 2)
    
    // Appearance
    color: Qt.rgba(0, 0, 0, 0.9)
    radius: 8
    
    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: root.padding
        
        model: root.options
        spacing: 2
        interactive: false  // Disable scrolling since we size to content
        
        delegate: Rectangle {
            width: listView.width
            height: root.itemHeight
            color: mouseArea.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
            radius: 4
            
            property bool hovered: false
            
            Text {
                anchors.centerIn: parent
                text: modelData.text || ""
                color: parent.hovered ? "#ffffff" : "#cccccc"
                font.pixelSize: 14
            }
            
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                property bool hovered: false
                
                onEntered: {
                    parent.hovered = true
                    hovered = true
                }
                
                onExited: {
                    parent.hovered = false
                    hovered = false
                }
                
                onClicked: {
                    root.optionSelected(modelData.value !== undefined ? modelData.value : modelData.text)
                    root.dismissed()
                }
            }
        }
    }
}