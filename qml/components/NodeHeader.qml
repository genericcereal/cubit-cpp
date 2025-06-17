import QtQuick
import QtQuick.Controls
import Cubit 1.0
import ".."

Rectangle {
    id: root
    
    property string nodeName: ""
    property string nodeType: "Operation" // Event, Operation, Param, or Variable
    
    height: Config.nodeHeaderHeight
    
    Component.onCompleted: {
        console.log("NodeHeader created - name:", nodeName, "type:", nodeType, "color:", color)
    }
    
    // Background color based on node type
    color: {
        switch(nodeType) {
            case "Event":
                return Config.nodeHeaderEventColor
            case "Operation":
                return Config.nodeHeaderOperationColor
            case "Param":
                return Config.nodeHeaderParamColor
            case "Variable":
                return Config.nodeHeaderVariableColor
            default:
                return Config.nodeHeaderOperationColor
        }
    }
    
    Text {
        id: headerText
        text: nodeName
        color: Config.nodeHeaderTextColor
        font.pixelSize: Config.nodeHeaderTextSize
        anchors {
            left: parent.left
            leftMargin: Config.nodeHeaderPadding
            verticalCenter: parent.verticalCenter
        }
        elide: Text.ElideRight
        width: parent.width - (2 * Config.nodeHeaderPadding)
    }
}