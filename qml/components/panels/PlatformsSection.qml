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
                            var currentPlatforms = canvas.platforms
                            var availablePlatforms = []
                            
                            for (var i = 0; i < allTargets.length; i++) {
                                if (currentPlatforms.indexOf(allTargets[i]) === -1) {
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
                                var platforms = canvas.platforms
                                platforms.push(platformCombo.currentText)
                                canvas.platforms = platforms
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
            title: modelData
            visible: !selectedElement && canvas
            
            content: [
                Button {
                    text: "Remove"
                    Layout.alignment: Qt.AlignCenter
                    onClicked: {
                        if (canvas) {
                            var platforms = canvas.platforms
                            var index = platforms.indexOf(modelData)
                            if (index > -1) {
                                platforms.splice(index, 1)
                                canvas.platforms = platforms
                            }
                        }
                    }
                }
            ]
        }
    }
}