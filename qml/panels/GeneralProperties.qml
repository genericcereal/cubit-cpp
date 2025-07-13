import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"
import "../PropertyHelpers.js" as PropertyHelpers

PropertyGroup {
    id: root
    title: "General"
    
    property var selectedElement
    property var editableProperties: []
    
    visible: selectedElement
    
    property var generalProps: [
        { 
            name: "Name", 
            type: "text", 
            getter: () => selectedElement ? selectedElement.name : "",
            setter: v => { if (selectedElement) selectedElement.name = v }
        },
        { 
            name: "Type", 
            type: "label", 
            getter: () => selectedElement ? selectedElement.elementType : ""
        },
        { 
            name: "ID", 
            type: "label", 
            getter: () => selectedElement ? selectedElement.elementId : ""
        },
        { 
            name: "Parent ID", 
            type: "label", 
            getter: () => selectedElement && selectedElement.parentId ? selectedElement.parentId : "None",
            visible: () => selectedElement && selectedElement.isDesignElement
        },
        {
            name: "Platform",
            type: "combobox",
            getter: () => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    return selectedElement.platform || "undefined"
                }
                return "undefined"
            },
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.platform = v === "undefined" ? "" : v
                }
            },
            model: () => {
                if (!Application.activeCanvas || !selectedElement || (selectedElement.elementType !== "Frame" && selectedElement.elementType !== "FrameComponentVariant" && selectedElement.elementType !== "FrameComponentInstance")) {
                    return ["undefined"]
                }
                
                var platforms = ["undefined"]
                var canvasPlatforms = Application.activeCanvas.platforms
                for (var i = 0; i < canvasPlatforms.length; i++) {
                    platforms.push(canvasPlatforms[i])
                }
                return platforms
            },
            visible: () => PropertyHelpers.canShowPlatform(selectedElement, editableProperties, Application)
        },
        {
            name: "Role",
            type: "combobox",
            getter: () => "container",
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance")) {
                    selectedElement.role = 1
                }
            },
            model: () => ["container"],
            visible: () => PropertyHelpers.canShowRole(selectedElement, editableProperties)
        }
    ]
    
    Component {
        id: textFieldComp
        TextField {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: labelComp
        Label {
            color: "#666666"
        }
    }
    
    Component {
        id: comboBoxComp
        ComboBox {
            Layout.fillWidth: true
        }
    }
    
    content: [
        Repeater {
            model: generalProps.filter(prop => !prop.visible || prop.visible())
            
            delegate: LabeledField {
                label: modelData.name
                visible: !modelData.visible || modelData.visible()
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "text" ? textFieldComp
                                       : modelData.type === "label" ? labelComp
                                       : modelData.type === "combobox" ? comboBoxComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "text") {
                                item.text = Qt.binding(modelData.getter)
                                item.textChanged.connect(function() {
                                    modelData.setter(item.text)
                                })
                            } else if (modelData.type === "label") {
                                item.text = Qt.binding(modelData.getter)
                            } else if (modelData.type === "combobox") {
                                item.model = Qt.binding(modelData.model)
                                item.currentIndex = Qt.binding(function() {
                                    var value = modelData.getter()
                                    var index = item.model.indexOf(value)
                                    return index >= 0 ? index : 0
                                })
                                item.activated.connect(function(index) {
                                    var value = item.model[index]
                                    modelData.setter(value)
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}