import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit

ColumnLayout {
    id: root
    spacing: 10
    
    property var selectedElement
    
    // Platforms section when no element selected
    GroupBox {
        Layout.fillWidth: true
        Layout.margins: 10
        title: "Platforms"
        visible: !selectedElement && Application.activeCanvas
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            RowLayout {
                Layout.fillWidth: true
                
                ComboBox {
                    id: platformCombo
                    Layout.fillWidth: true
                    model: {
                        if (!Application.activeCanvas) return []
                        
                        var allTargets = ["iOS", "Android", "web"]
                        var currentPlatforms = Application.activeCanvas.platforms
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
                        if (Application.activeCanvas && platformCombo.currentText) {
                            var platforms = Application.activeCanvas.platforms
                            platforms.push(platformCombo.currentText)
                            Application.activeCanvas.platforms = platforms
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
    }
    
    // Added platforms sections
    Repeater {
        model: Application.activeCanvas ? Application.activeCanvas.platforms : []
        
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: modelData
            visible: !selectedElement && Application.activeCanvas
            
            Button {
                text: "Remove"
                anchors.centerIn: parent
                onClicked: {
                    if (Application.activeCanvas) {
                        var platforms = Application.activeCanvas.platforms
                        var index = platforms.indexOf(modelData)
                        if (index > -1) {
                            platforms.splice(index, 1)
                            Application.activeCanvas.platforms = platforms
                        }
                    }
                }
            }
        }
    }
}