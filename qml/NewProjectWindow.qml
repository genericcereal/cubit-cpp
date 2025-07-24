import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Cubit

ApplicationWindow {
    id: newProjectWindow
    title: "New Project"
    width: 400
    height: 200
    minimumWidth: 400
    minimumHeight: 200
    maximumWidth: 400
    maximumHeight: 200
    
    flags: Qt.Dialog
    
    property alias projectName: projectNameField.text
    
    // Center the window on screen
    Component.onCompleted: {
        x = (Screen.width - width) / 2
        y = (Screen.height - height) / 2
        projectNameField.forceActiveFocus()
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20
        
        Label {
            text: "Enter a name for your new project:"
            font.pixelSize: 14
            Layout.fillWidth: true
        }
        
        TextField {
            id: projectNameField
            placeholderText: "Project Name"
            Layout.fillWidth: true
            font.pixelSize: 14
            selectByMouse: true
            
            // Submit on Enter key
            Keys.onReturnPressed: {
                if (text.trim() !== "") {
                    acceptProject()
                }
            }
            
            Keys.onEnterPressed: {
                if (text.trim() !== "") {
                    acceptProject()
                }
            }
        }
        
        Item {
            Layout.fillHeight: true
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                text: "Cancel"
                Layout.preferredWidth: 80
                onClicked: {
                    newProjectWindow.close()
                }
            }
            
            Button {
                text: "OK"
                Layout.preferredWidth: 80
                enabled: projectNameField.text.trim() !== ""
                highlighted: true
                
                onClicked: {
                    acceptProject()
                }
            }
        }
    }
    
    function acceptProject() {
        if (projectName.trim() !== "") {
            Application.createNewProject(projectName.trim())
            newProjectWindow.close()
        }
    }
}