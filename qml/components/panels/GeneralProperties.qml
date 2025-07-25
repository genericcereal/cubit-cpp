import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

PropertyGroup {
    id: root
    title: "General"
    
    property var selectedElement
    property var editableProperties: []
    property var canvas: null  // Will be set by parent
    
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
            name: "Variant",
            type: "combobox",
            getter: () => {
                if (selectedElement && selectedElement.sourceVariant) {
                    return selectedElement.sourceVariant.variantName || "Default"
                }
                return "Default"
            },
            setter: v => {
                if (selectedElement && selectedElement.sourceComponent) {
                    var variants = selectedElement.sourceComponent().variants
                    for (var i = 0; i < variants.length; i++) {
                        if (variants[i].variantName === v) {
                            selectedElement.sourceVariant = variants[i]
                            break
                        }
                    }
                }
            },
            model: () => {
                if (!selectedElement || !selectedElement.sourceComponent) {
                    return ["Default"]
                }
                var variantNames = []
                var component = selectedElement.sourceComponent()
                if (component && component.variants) {
                    for (var i = 0; i < component.variants.length; i++) {
                        var variant = component.variants[i]
                        variantNames.push(variant.variantName || "Variant" + (i + 1))
                    }
                }
                return variantNames.length > 0 ? variantNames : ["Default"]
            },
            visible: () => selectedElement && selectedElement.isComponentInstance && selectedElement.isComponentInstance()
        },
        {
            name: "Platform",
            type: "combobox",
            getter: () => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance" || selectedElement.elementType === "Variable")) {
                    return selectedElement.platform || "undefined"
                }
                return "undefined"
            },
            setter: v => {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "FrameComponentVariant" || selectedElement.elementType === "FrameComponentInstance" || selectedElement.elementType === "Variable")) {
                    selectedElement.platform = v === "undefined" ? "" : v
                }
            },
            model: () => {
                if (!canvas || !selectedElement || (selectedElement.elementType !== "Frame" && selectedElement.elementType !== "FrameComponentVariant" && selectedElement.elementType !== "FrameComponentInstance" && selectedElement.elementType !== "Variable")) {
                    return ["undefined"]
                }
                
                var platforms = ["undefined"]
                var canvasPlatforms = canvas.platforms
                for (var i = 0; i < canvasPlatforms.length; i++) {
                    platforms.push(canvasPlatforms[i].name)
                }
                return platforms
            },
            visible: () => PropertyHelpers.canShowPlatform(selectedElement, editableProperties, canvas)
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
                                // Set initial value
                                item.text = modelData.getter()
                                
                                // Create a connection to update the text when the source changes
                                var updateText = function() {
                                    if (modelData && item) {
                                        item.text = modelData.getter()
                                    }
                                }
                                
                                // Connect to property changes if the element has a signal
                                if (root.selectedElement && modelData.property) {
                                    try {
                                        var signalName = modelData.property + "Changed"
                                        var signal = root.selectedElement[signalName]
                                        if (signal && typeof signal.connect === "function") {
                                            signal.connect(updateText)
                                        }
                                    } catch (e) {
                                        // Signal doesn't exist, ignore
                                    }
                                }
                                
                                item.textChanged.connect(function() {
                                    if (modelData && item) {
                                        modelData.setter(item.text)
                                    }
                                })
                            } else if (modelData.type === "label") {
                                // Set initial value
                                item.text = modelData.getter()
                                
                                // Create a connection to update the text when the source changes
                                var updateLabel = function() {
                                    if (modelData && item) {
                                        item.text = modelData.getter()
                                    }
                                }
                                
                                // Connect to property changes if the element has a signal
                                if (root.selectedElement && modelData.property) {
                                    try {
                                        var signalName = modelData.property + "Changed"
                                        var signal = root.selectedElement[signalName]
                                        if (signal && typeof signal.connect === "function") {
                                            signal.connect(updateLabel)
                                        }
                                    } catch (e) {
                                        // Signal doesn't exist, ignore
                                    }
                                }
                            } else if (modelData.type === "combobox") {
                                // Set initial values
                                item.model = modelData.model()
                                var value = modelData.getter()
                                var index = item.model.indexOf(value)
                                item.currentIndex = index >= 0 ? index : 0
                                
                                // Create a connection to update when the source changes
                                var updateComboBox = function() {
                                    if (modelData && item) {
                                        item.model = modelData.model()
                                        var value = modelData.getter()
                                        var index = item.model.indexOf(value)
                                        item.currentIndex = index >= 0 ? index : 0
                                    }
                                }
                                
                                // Connect to property changes if the element has a signal
                                if (root.selectedElement && modelData.property) {
                                    try {
                                        var signalName = modelData.property + "Changed"
                                        var signal = root.selectedElement[signalName]
                                        if (signal && typeof signal.connect === "function") {
                                            signal.connect(updateComboBox)
                                        }
                                    } catch (e) {
                                        // Signal doesn't exist, ignore
                                    }
                                }
                                
                                item.activated.connect(function(index) {
                                    if (modelData && item && item.model) {
                                        var value = item.model[index]
                                        modelData.setter(value)
                                    }
                                })
                            }
                        }
                    }
                ]
            }
        }
    ]
}