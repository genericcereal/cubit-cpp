import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../PropertyHelpers.js" as PropertyHelpers

GroupBox {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    title: "General"
    
    property var selectedElement
    property var editableProperties: []
    
    visible: selectedElement
    
    
    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: 10
        rowSpacing: 5
        
        Label { text: "Name:" }
        TextField {
            Layout.fillWidth: true
            text: selectedElement ? selectedElement.name : ""
            onTextChanged: if (selectedElement) selectedElement.name = text
        }
        
        Label { text: "Type:" }
        Label {
            text: selectedElement ? selectedElement.elementType : ""
            color: "#666666"
        }
        
        Label { text: "ID:" }
        Label {
            text: selectedElement ? selectedElement.elementId : ""
            color: "#666666"
        }
        
        Label { 
            text: "Parent ID:" 
            visible: selectedElement && selectedElement.isDesignElement
        }
        Label {
            text: selectedElement && selectedElement.parentId ? selectedElement.parentId : "None"
            color: "#666666"
            visible: selectedElement && selectedElement.isDesignElement
        }
        
        Label { 
            text: "Platform:" 
            visible: PropertyHelpers.canShowPlatform(selectedElement, editableProperties, Application)
        }
        ComboBox {
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowPlatform(selectedElement, editableProperties, Application)
            model: {
                if (!Application.activeCanvas || !selectedElement || (selectedElement.elementType !== "Frame" && selectedElement.elementType !== "ComponentVariant" && selectedElement.elementType !== "ComponentInstance")) {
                    return ["undefined"]
                }
                
                var platforms = ["undefined"]
                var canvasPlatforms = Application.activeCanvas.platforms
                for (var i = 0; i < canvasPlatforms.length; i++) {
                    platforms.push(canvasPlatforms[i])
                }
                return platforms
            }
            currentIndex: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var platform = selectedElement.platform || "undefined"
                    var index = model.indexOf(platform)
                    return index >= 0 ? index : 0
                }
                return 0
            }
            onActivated: function(index) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    var platform = model[index]
                    selectedElement.platform = platform === "undefined" ? "" : platform
                }
            }
        }
        
        Label { 
            text: "Role:" 
            visible: PropertyHelpers.canShowRole(selectedElement, editableProperties)
        }
        ComboBox {
            Layout.fillWidth: true
            visible: PropertyHelpers.canShowRole(selectedElement, editableProperties)
            model: ["container"]
            currentIndex: 0
            onActivated: function(index) {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.role = 1
                }
            }
        }
    }
}