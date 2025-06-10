import QtQuick
import Cubit.UI 1.0

Item {
    id: root
    
    property alias text: label.text
    property point mousePosition: Qt.point(0, 0)
    
    width: background.width
    height: background.height
    
    // Position the badge 5px right and 10px down from cursor
    x: Math.min(mousePosition.x + 5, parent.width - width - 5)
    y: Math.min(mousePosition.y + 10, parent.height - height - 5)
    
    // Use visible property directly from Item - no animation for instant appearance
    opacity: visible ? 1.0 : 0.0
    
    Rectangle {
        id: background
        width: label.width + 16
        height: label.height + 8
        radius: 4
        color: Config.hoverBadgeBackgroundColor
        border.color: Config.hoverBadgeBorderColor
        border.width: 1
        antialiasing: true
        
        Text {
            id: label
            anchors.centerIn: parent
            font.pixelSize: 12
            font.family: "Arial"
            color: Config.hoverBadgeTextColor
            text: "0 x 0"
        }
    }
}