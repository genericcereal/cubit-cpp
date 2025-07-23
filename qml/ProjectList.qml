import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Cubit

Rectangle {
    id: root
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40
        spacing: 30

        // Header
        Text {
            text: "Projects"
            font.pixelSize: 32
            font.weight: Font.Bold
            color: "#1A1A1A"
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
                model: ListModel {
                    id: projectsModel
                    ListElement {
                        name: "My First Project"
                        lastModified: "2 hours ago"
                    }
                    ListElement {
                        name: "Mobile App Design"
                        lastModified: "Yesterday"
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
                    // Functionality to be implemented later
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
                        text: "📋"
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