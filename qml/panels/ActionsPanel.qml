import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 300
    height: 50
    radius: 8
    color: Qt.rgba(0, 0, 0, 0.8)
    
    signal modeChanged(string mode)
    
    property string currentMode: "select"
    
    onCurrentModeChanged: {
        console.log("ActionsPanel currentMode changed to:", currentMode)
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        
        ToolButton {
            id: selectButton
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === "select"
            
            contentItem: Text {
                text: "S"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
            }
            
            onClicked: {
                currentMode = "select"
                root.modeChanged("select")
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Select Mode"
        }
        
        ToolButton {
            id: frameButton
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === "frame"
            
            contentItem: Text {
                text: "F"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
            }
            
            onClicked: {
                currentMode = "frame"
                root.modeChanged("frame")
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Frame Mode"
        }
        
        ToolButton {
            id: textButton
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === "text"
            
            contentItem: Text {
                text: "T"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
            }
            
            onClicked: {
                currentMode = "text"
                root.modeChanged("text")
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Text Mode"
        }
        
        ToolButton {
            id: htmlButton
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === "html"
            
            contentItem: Text {
                text: "H"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
            }
            
            onClicked: {
                currentMode = "html"
                root.modeChanged("html")
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "HTML Mode"
        }
        
        ToolButton {
            id: variableButton
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === "variable"
            
            contentItem: Text {
                text: "V"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
            }
            
            onClicked: {
                currentMode = "variable"
                root.modeChanged("variable")
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Variable Mode"
        }
        
        Item {
            Layout.fillWidth: true
        }
    }
}