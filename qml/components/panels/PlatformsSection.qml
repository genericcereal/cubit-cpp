import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

ColumnLayout {
    id: root
    spacing: 10
    visible: !selectedElement && canvas
    
    property var selectedElement
    property var canvas: null  // Will be set by parent
    
    // Main platforms management
    PropertyGroup {
        title: "Platforms"
        
        content: [
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    ComboBox {
                        id: platformCombo
                        Layout.fillWidth: true
                        model: {
                            if (!canvas) return []
                            
                            var allTargets = ["iOS", "Android", "web"]
                            var availablePlatforms = []
                            
                            // Check which platforms are already added
                            for (var i = 0; i < allTargets.length; i++) {
                                var platformExists = false
                                for (var j = 0; j < canvas.platforms.length; j++) {
                                    if (canvas.platforms[j].name === allTargets[i]) {
                                        platformExists = true
                                        break
                                    }
                                }
                                if (!platformExists) {
                                    availablePlatforms.push(allTargets[i])
                                }
                            }
                            
                            return availablePlatforms
                        }
                        enabled: count > 0
                    }
                    
                    Button {
                        text: "Add"
                        Layout.preferredWidth: 80
                        enabled: platformCombo.count > 0
                        onClicked: {
                            if (canvas && platformCombo.currentText) {
                                canvas.addPlatform(platformCombo.currentText)
                            }
                        }
                    }
                }
                
                Label {
                    visible: platformCombo.count === 0
                    text: "All platforms added"
                    color: "#666666"
                    font.italic: true
                }
            }
        ]
    }
    
    // Added platforms sections
    Repeater {
        model: canvas ? canvas.platforms : []
        
        PropertyGroup {
            title: modelData ? modelData.displayName : ""
            visible: !selectedElement && canvas
            
            required property var modelData
            
            content: [
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Label {
                        text: {
                            if (!modelData || !modelData.scripts) {
                                return "Scripts: 0 nodes, 0 edges"
                            }
                            return "Scripts: " + modelData.scripts.nodeCount + " nodes, " + 
                                  modelData.scripts.edgeCount + " edges"
                        }
                        color: "#666666"
                        font.italic: true
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Button {
                            text: "Edit Scripts"
                            Layout.fillWidth: true
                            enabled: modelData && modelData.scripts
                            onClicked: {
                                if (canvas && modelData) {
                                    // Switch to script canvas view mode
                                    canvas.setEditingPlatform(modelData, "script")
                                }
                            }
                        }
                        
                        Button {
                            text: "Remove"
                            Layout.preferredWidth: 80
                            onClicked: {
                                if (canvas && modelData) {
                                    canvas.removePlatform(modelData.name)
                                }
                            }
                        }
                    }
                }
            ]
        }
    }
}