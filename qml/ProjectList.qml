import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit

Rectangle {
    id: root
    color: "#F5F5F5"

    property bool isLoading: false
    property string errorMessage: ""
    property var newProjectWindow: null

    // Dynamic model for projects from API only
    ListModel {
        id: projectsModel
    }

    // Connect to Application signals
    Connections {
        target: Application
        
        function onProjectsListedFromAPI(projects, nextToken) {
            // ProjectList: Received projects from API
            updateProjectsModel(projects)
            isLoading = false
            // TODO: Handle nextToken for pagination if needed
        }
        
        function onProjectFetchedFromAPI(projectId, project) {
            // Extract canvasData from the project object
            let canvasData = project.canvasData
            if (canvasData && canvasData.elements) {
            } else {
            }
            openProjectWithCanvasData(project, canvasData)
        }
        
        function onProjectCreatedInAPI(projectId, projectName) {
            // ProjectList: Project created successfully
            // Refresh the project list to show the new project
            fetchProjects()
        }
        
        function onProjectDeletedFromAPI(projectId) {
            // ProjectList: Project deleted successfully
            // Refresh the project list
            fetchProjects()
        }
        
        function onApiErrorOccurred(error) {
            console.error("ProjectList: API error occurred:", error)
            errorMessage = "API Error: " + error
            isLoading = false
        }
    }
    
    // Connect to AuthenticationManager to retry after token refresh
    Connections {
        target: authManager
        
        function onTokensRefreshed() {
            // ProjectList: Tokens refreshed, retrying fetch...
            fetchProjects()
        }
    }

    // JavaScript functions
    function fetchProjects() {
        isLoading = true
        errorMessage = ""
        // List projects without filter, with default limit
        Application.listProjects()
    }

    function updateProjectsModel(apiProjects) {
        // Clear existing model
        projectsModel.clear()
        
        // Add projects from API
        for (let i = 0; i < apiProjects.length; i++) {
            let project = apiProjects[i]
            let lastModified = formatLastModified(project.updatedAt || project.createdAt)
            
            projectsModel.append({
                "id": project.id,
                "name": project.name,
                "lastModified": lastModified,
                "createdAt": project.createdAt,
                "updatedAt": project.updatedAt,
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
        // ProjectList: Fetching canvas data for project
        
        if (!projectData.id || !projectData.name) {
            console.error("ProjectList: Invalid project data - missing id or name")
            errorMessage = "Invalid project data"
            return
        }
        
        // Show loading state
        isLoading = true
        errorMessage = ""
        
        // Always fetch the latest project data to ensure we have the most up-to-date version
        Application.fetchProjectFromAPI(projectData.id)
    }
    
    function openProjectWithCanvasData(projectData, canvasData) {
        // ProjectList: Opening API project with canvas data
        
        try {
            Application.openAPIProject(projectData.id, projectData.name, canvasData)
            // ProjectList: Successfully opened API project
            isLoading = false
        } catch (error) {
            console.error("ProjectList: Error opening API project:", error)
            errorMessage = "Failed to open project: " + error.message
            isLoading = false
        }
    }

    // Component initialization
    Component.onCompleted: {
        // ProjectList: Component loaded, fetching projects...
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
                    if (root.newProjectWindow && root.newProjectWindow.visible) {
                        // Window already exists and is visible, just bring it to focus
                        root.newProjectWindow.raise()
                        root.newProjectWindow.requestActivate()
                    } else {
                        // Create new window or show existing hidden window
                        if (!root.newProjectWindow) {
                            var component = Qt.createComponent("NewProjectWindow.qml")
                            if (component.status === Component.Ready) {
                                root.newProjectWindow = component.createObject()
                            } else {
                                console.error("Error creating new project window:", component.errorString())
                                return
                            }
                        }
                        root.newProjectWindow.show()
                        root.newProjectWindow.raise()
                        root.newProjectWindow.requestActivate()
                    }
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
            border.color: isHovered ? "#1976FF" : "#E0E0E0"
            border.width: 1
            
            property bool isHovered: projectHoverArea.containsMouse || deleteMouseArea.containsMouse

            MouseArea {
                id: projectHoverArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    openProject(model)
                }
            }

            // Delete button - appears on hover in top right corner
            Rectangle {
                id: deleteButton
                width: 30
                height: 30
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
                color: deleteMouseArea.pressed ? "#CC0000" : (deleteMouseArea.containsMouse ? "#FF3333" : "#FF5555")
                radius: 6
                visible: parent.isHovered
                opacity: visible ? 1.0 : 0.0
                z: 10  // Ensure button is above all other content
                
                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    font.pixelSize: 16
                    color: "white"
                    font.weight: Font.Bold
                }

                MouseArea {
                    id: deleteMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    propagateComposedEvents: true
                    
                    onClicked: function(mouse) {
                        // Delete project clicked
                        mouse.accepted = true
                        // TODO: Add confirmation dialog
                        Application.deleteProject(model.id)
                        // Don't refresh immediately - wait for API confirmation
                    }
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