import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit

Rectangle {
    id: root
    color: "#F5F5F5"

    property bool isLoading: false
    property string errorMessage: ""

    // Dynamic model for projects from API only
    ListModel {
        id: projectsModel
    }

    // Connect to Application signals
    Connections {
        target: Application
        
        function onProjectsFetchedFromAPI(projects) {
            console.log("ProjectList: Received", projects.length, "projects from API")
            updateProjectsModel(projects)
            isLoading = false
        }
        
        function onApiErrorOccurred(error) {
            console.error("ProjectList: API error occurred:", error)
            errorMessage = "API Error: " + error
            isLoading = false
        }
    }

    // JavaScript functions
    function fetchProjects() {
        isLoading = true
        errorMessage = ""
        Application.fetchProjectsFromAPI()
    }

    function updateProjectsModel(apiProjects) {
        // Clear existing model
        projectsModel.clear()
        
        // Add projects from API only
        for (let i = 0; i < apiProjects.length; i++) {
            let project = apiProjects[i]
            let lastModified = formatLastModified(project.updatedAt || project.createdAt)
            
            projectsModel.append({
                "id": project.id,
                "name": project.name,
                "lastModified": lastModified,
                "createdAt": project.createdAt,
                "updatedAt": project.updatedAt,
                "canvasData": project.canvasData,
                "teamId": project.teamId
            })
        }
    }

    function formatLastModified(dateString) {
        if (!dateString) return "Unknown"
        
        let date = new Date(dateString)
        let now = new Date()
        let diffMs = now - date
        let diffMinutes = Math.floor(diffMs / (1000 * 60))
        let diffHours = Math.floor(diffMinutes / 60)
        let diffDays = Math.floor(diffHours / 24)
        
        if (diffMinutes < 1) return "Just now"
        if (diffMinutes < 60) return diffMinutes + " minutes ago"
        if (diffHours < 24) return diffHours + " hours ago"
        if (diffDays === 1) return "Yesterday"
        if (diffDays < 7) return diffDays + " days ago"
        
        return date.toLocaleDateString()
    }

    function openProject(projectData) {
        console.log("ProjectList: Opening API project:", projectData.name)
        
        if (!projectData.id || !projectData.name) {
            console.error("ProjectList: Invalid project data - missing id or name")
            errorMessage = "Invalid project data"
            return
        }
        
        if (!projectData.canvasData) {
            console.error("ProjectList: No canvas data available for project:", projectData.name)
            errorMessage = "Project has no canvas data"
            return
        }
        
        try {
            Application.openAPIProject(projectData.id, projectData.name, projectData.canvasData)
            console.log("ProjectList: Successfully opened API project:", projectData.name)
        } catch (error) {
            console.error("ProjectList: Error opening API project:", error)
            errorMessage = "Failed to open project: " + error.message
        }
    }

    // Component initialization
    Component.onCompleted: {
        console.log("ProjectList: Component loaded, fetching projects...")
        fetchProjects()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

        // Header with refresh button
        RowLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Projects"
                font.pixelSize: 32
                font.weight: Font.Bold
                color: "#1A1A1A"
                Layout.fillWidth: true
            }
            
            // Refresh button
            Rectangle {
                width: 100
                height: 35
                color: refreshMouseArea.pressed ? "#0066FF" : (refreshMouseArea.containsMouse ? "#3385FF" : "#1976FF")
                radius: 6
                visible: !isLoading
                
                Text {
                    anchors.centerIn: parent
                    text: "Refresh"
                    font.pixelSize: 14
                    color: "white"
                }
                
                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    
                    onClicked: fetchProjects()
                }
            }
            
            // Loading indicator
            Rectangle {
                width: 100
                height: 35
                color: "#CCCCCC"
                radius: 6
                visible: isLoading
                
                Text {
                    anchors.centerIn: parent
                    text: "Loading..."
                    font.pixelSize: 14
                    color: "#666666"
                }
            }
        }

        // Error message
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#FFEBEE"
            border.color: "#F44336"
            border.width: 1
            radius: 8
            visible: errorMessage !== ""
            
            Text {
                anchors.centerIn: parent
                text: errorMessage
                color: "#D32F2F"
                font.pixelSize: 14
            }
        }

        // Projects list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridView {
                id: projectGrid
                anchors.fill: parent
                cellWidth: 300
                cellHeight: 200
                delegate: projectDelegate
                model: projectsModel
                
                // Empty state
                Rectangle {
                    anchors.centerIn: parent
                    width: 300
                    height: 200
                    color: "transparent"
                    visible: projectsModel.count === 0 && !isLoading
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 20
                        
                        Text {
                            text: "☁️"
                            font.pixelSize: 64
                            anchors.horizontalCenter: parent.horizontalCenter
                            opacity: 0.3
                        }
                        
                        Text {
                            text: "No projects found"
                            font.pixelSize: 18
                            color: "#666666"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        
                        Text {
                            text: "Create your first project to get started"
                            font.pixelSize: 14
                            color: "#999999"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }

        // Create project button
        Rectangle {
            id: createProjectButton
            Layout.alignment: Qt.AlignHCenter
            width: 200
            height: 50
            color: mouseArea.pressed ? "#0066FF" : (mouseArea.containsMouse ? "#3385FF" : "#1976FF")
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Create New Project"
                font.pixelSize: 16
                color: "white"
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                
                onClicked: {
                    Application.createNewProject()
                }
            }
        }
    }

    Component {
        id: projectDelegate

        Rectangle {
            width: 280
            height: 180
            color: "white"
            radius: 12
            border.color: "#E0E0E0"
            border.width: 1

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onEntered: parent.border.color = "#1976FF"
                onExited: parent.border.color = "#E0E0E0"
                onClicked: {
                    openProject(model)
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                // Project thumbnail placeholder
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#F0F0F0"
                    radius: 8

                    Text {
                        anchors.centerIn: parent
                        text: "☁️"
                        font.pixelSize: 48
                        opacity: 0.5
                    }
                }

                // Project name
                Text {
                    text: model.name
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#1A1A1A"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Last modified
                Text {
                    text: "Modified " + model.lastModified
                    font.pixelSize: 12
                    color: "#666666"
                }
            }
        }
    }
}