import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    property alias content: contentLayout.children
    property bool expanded: false
    property string groupTitle: ""
    
    Layout.fillWidth: true
    implicitHeight: headerArea.height + (expanded ? contentLayout.height + 8 : 0)
    color: "transparent"
    border.color: "#e0e0e0"
    border.width: 1
    radius: 4
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Header area
        Item {
            id: headerArea
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 4
                
                Text {
                    text: root.expanded ? "▼" : "▶"
                    font.pixelSize: 10
                    color: "#666666"
                }
                
                Label {
                    text: root.groupTitle
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.expanded = !root.expanded
            }
        }
        
        // Content area
        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            Layout.margins: 8
            Layout.topMargin: 0
            visible: root.expanded
            spacing: 4
        }
    }
}