import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Cubit

ApplicationWindow {
    id: newProjectWindow
    title: "New Project"
    width: 400
    height: 350
    minimumWidth: 400
    minimumHeight: 350
    maximumWidth: 400
    maximumHeight: 350
    
    flags: Qt.Dialog
    
    property alias projectName: projectNameField.text
    property string selectedTemplate: "blank"
    
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
        
        // Template selection grid
        GridLayout {
            columns: 3
            columnSpacing: 20
            rowSpacing: 10
            Layout.fillWidth: true
            
            // Blank template
            ColumnLayout {
                spacing: 5
                
                Rectangle {
                    width: 80
                    height: 80
                    color: selectedTemplate === "blank" ? "#007AFF" : "#f0f0f0"
                    border.color: selectedTemplate === "blank" ? "#0051D5" : "#d0d0d0"
                    border.width: selectedTemplate === "blank" ? 2 : 1
                    radius: 8
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectedTemplate = "blank"
                    }
                    
                    Text {
                        anchors.centerIn: parent
                        text: "📄"
                        font.pixelSize: 32
                    }
                }
                
                Label {
                    text: "Blank"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    font.pixelSize: 12
                }
            }
            
            // iOS template
            ColumnLayout {
                spacing: 5
                
                Rectangle {
                    width: 80
                    height: 80
                    color: selectedTemplate === "ios" ? "#007AFF" : "#f0f0f0"
                    border.color: selectedTemplate === "ios" ? "#0051D5" : "#d0d0d0"
                    border.width: selectedTemplate === "ios" ? 2 : 1
                    radius: 8
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectedTemplate = "ios"
                    }
                    
                    Text {
                        anchors.centerIn: parent
                        text: "📱"
                        font.pixelSize: 32
                    }
                }
                
                Label {
                    text: "iOS"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    font.pixelSize: 12
                }
            }
            
            // Spacer
            Item {
                Layout.fillWidth: true
            }
        }
        
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
            if (selectedTemplate === "ios") {
                Application.createNewProjectWithTemplate(projectName.trim(), "qml/components/project-templates/ios-project-template.qml")
            } else {
                Application.createNewProject(projectName.trim())
            }
            newProjectWindow.close()
        }
    }
}