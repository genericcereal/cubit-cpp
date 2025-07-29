import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "../.."

Item {
    id: hoverBadgeRoot
    
    property alias text: hoverBadgeLabel.text
    property point mousePosition: Qt.point(0, 0)
    property bool allSelectedAreComponentRelated: false
    
    width: hoverBadgeBackground.width
    height: hoverBadgeBackground.height
    
    // Position the badge 5px right and 10px down from cursor
    x: Math.round(Math.min(mousePosition.x + 5, parent.width - width - 5))
    y: Math.round(Math.min(mousePosition.y + 10, parent.height - height - 5))
    
    // Use visible property directly from Item - no animation for instant appearance
    opacity: visible ? 1.0 : 0.0
    
    Rectangle {
        id: hoverBadgeBackground
        width: hoverBadgeLabel.width + 16
        height: hoverBadgeLabel.height + 8
        radius: 4
        color: hoverBadgeRoot.allSelectedAreComponentRelated ? ConfigObject.componentHoverBadgeBackgroundColor : ConfigObject.hoverBadgeBackgroundColor
        border.color: hoverBadgeRoot.allSelectedAreComponentRelated ? ConfigObject.componentHoverBadgeBorderColor : ConfigObject.hoverBadgeBorderColor
        border.width: 1
        antialiasing: true
        
        Text {
            id: hoverBadgeLabel
            anchors.centerIn: parent
            font.pixelSize: 12
            font.family: "Arial"
            color: ConfigObject.hoverBadgeTextColor
            text: "0 x 0"
        }
    }
}