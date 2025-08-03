import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

PropertyGroup {
    id: root
    title: "Text"
    
    property var selectedElement
    property string currentFontFamily: selectedElement && selectedElement.font ? selectedElement.font.family : ""
    property int fontLoadTrigger: 0  // Used to trigger updates without binding loops
    
    signal panelSelectorClicked(var selector, string type)
    
    visible: selectedElement && (selectedElement.elementType === "Text" || selectedElement.elementType === "TextComponentVariant" || (selectedElement.elementType === "FrameComponentInstance" && selectedElement.hasOwnProperty("content")))
    
    // Update weight list when fonts are loaded
    Connections {
        target: GoogleFonts
        function onFontLoaded(fontFamily) {
            if (fontFamily === root.currentFontFamily) {
                // Trigger update without causing binding loop
                root.fontLoadTrigger++
            }
        }
    }
    
    property var textProps: [
        {
            name: "Content",
            type: "text",
            getter: () => selectedElement && selectedElement.content !== undefined ? selectedElement.content : "",
            setter: v => {
                if (selectedElement && selectedElement.content !== undefined) {
                    selectedElement.content = v
                }
            }
        },
        {
            name: "Font",
            type: "property_popover",
            popoverType: "font",
            textGetter: () => selectedElement && selectedElement.font ? selectedElement.font.family : "System Default",
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.family = v
                    selectedElement.font = newFont
                }
            }
        },
        {
            name: "Size",
            type: "font_spinbox",
            from: 8,
            to: 144,
            getter: () => selectedElement && selectedElement.font ? selectedElement.font.pixelSize : 14,
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    newFont.pixelSize = v
                    selectedElement.font = newFont
                }
            }
        },
        {
            name: "Weight",
            type: "combobox",
            getter: () => {
                if (selectedElement && selectedElement.font) {
                    switch (selectedElement.font.weight) {
                        case Font.Thin: return "Thin"
                        case Font.ExtraLight: return "Extra Light"
                        case Font.Light: return "Light"
                        case Font.Normal: return "Normal"
                        case Font.Medium: return "Medium"
                        case Font.DemiBold: return "Demi Bold"
                        case Font.Bold: return "Bold"
                        case Font.ExtraBold: return "Extra Bold"
                        case Font.Black: return "Black"
                        default: return "Normal"
                    }
                }
                return "Normal"
            },
            setter: v => {
                if (selectedElement && selectedElement.font !== undefined) {
                    var newFont = selectedElement.font
                    var weightValue = Font.Normal
                    var googleWeight = "regular"
                    
                    switch (v) {
                        case "Thin": 
                            weightValue = Font.Thin
                            googleWeight = "100"
                            break
                        case "Extra Light": 
                            weightValue = Font.ExtraLight
                            googleWeight = "200"
                            break
                        case "Light": 
                            weightValue = Font.Light
                            googleWeight = "300"
                            break
                        case "Normal": 
                            weightValue = Font.Normal
                            googleWeight = "regular"
                            break
                        case "Medium": 
                            weightValue = Font.Medium
                            googleWeight = "500"
                            break
                        case "Demi Bold": 
                            weightValue = Font.DemiBold
                            googleWeight = "600"
                            break
                        case "Bold": 
                            weightValue = Font.Bold
                            googleWeight = "700"
                            break
                        case "Extra Bold": 
                            weightValue = Font.ExtraBold
                            googleWeight = "800"
                            break
                        case "Black": 
                            weightValue = Font.Black
                            googleWeight = "900"
                            break
                        default: 
                            weightValue = Font.Normal
                            googleWeight = "regular"
                    }
                    
                    // Load the specific weight if not already loaded
                    if (!GoogleFonts.isFontLoaded(newFont.family, googleWeight)) {
                        GoogleFonts.loadFont(newFont.family, googleWeight)
                    }
                    
                    newFont.weight = weightValue
                    selectedElement.font = newFont
                }
            },
            model: () => {
                if (!selectedElement || !selectedElement.font) {
                    return ["Normal"]
                }
                
                var fontFamily = selectedElement.font.family
                var availableWeights = GoogleFonts.getAvailableWeights(fontFamily)
                
                if (availableWeights.length === 0) {
                    // Font not loaded from Google Fonts, show all options
                    return ["Thin", "Extra Light", "Light", "Normal", "Medium", "Demi Bold", "Bold", "Extra Bold", "Black"]
                }
                
                // Map Google font weights to display names
                var displayWeights = []
                for (var i = 0; i < availableWeights.length; i++) {
                    var weight = availableWeights[i]
                    switch (weight) {
                        case "100": displayWeights.push("Thin"); break
                        case "200": displayWeights.push("Extra Light"); break
                        case "300": displayWeights.push("Light"); break
                        case "regular":
                        case "400": displayWeights.push("Normal"); break
                        case "500": displayWeights.push("Medium"); break
                        case "600": displayWeights.push("Demi Bold"); break
                        case "700":
                        case "bold": displayWeights.push("Bold"); break
                        case "800": displayWeights.push("Extra Bold"); break
                        case "900": displayWeights.push("Black"); break
                        default: 
                            // Handle italic variants
                            if (weight.endsWith("italic")) {
                                // Skip italic variants for now
                                continue
                            }
                            displayWeights.push(weight)
                    }
                }
                
                // Remove duplicates and sort by weight
                var uniqueWeights = []
                var weightOrder = ["Thin", "Extra Light", "Light", "Normal", "Medium", "Demi Bold", "Bold", "Extra Bold", "Black"]
                for (var j = 0; j < weightOrder.length; j++) {
                    if (displayWeights.indexOf(weightOrder[j]) !== -1) {
                        uniqueWeights.push(weightOrder[j])
                    }
                }
                
                return uniqueWeights.length > 0 ? uniqueWeights : ["Normal"]
            }
        },
        {
            name: "Box Shadow",
            type: "property_popover",
            popoverType: "boxShadow",
            placeholderText: "Add shadow...",
            textGetter: () => {
                if (selectedElement && selectedElement.boxShadow && selectedElement.boxShadow.enabled) {
                    const shadow = selectedElement.boxShadow
                    return `${shadow.offsetX}px ${shadow.offsetY}px ${shadow.blurRadius}px`
                }
                return ""
            }
        }
    ]
    
    Component {
        id: textFieldComp
        TextField {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: fontSpinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: propertyPopoverComp
        PropertyPopover {
            Layout.fillWidth: true
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
            model: textProps
            
            delegate: LabeledField {
                label: modelData.name
                
                delegate: [
                    Loader {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        sourceComponent: modelData.type === "text" ? textFieldComp
                                       : modelData.type === "font_spinbox" ? fontSpinBoxComp
                                       : modelData.type === "property_popover" ? propertyPopoverComp
                                       : modelData.type === "combobox" ? comboBoxComp
                                       : null
                        
                        onLoaded: {
                            if (modelData.type === "text") {
                                item.text = Qt.binding(modelData.getter)
                                item.textChanged.connect(function() {
                                    modelData.setter(item.text)
                                })
                            } else if (modelData.type === "font_spinbox") {
                                item.from = modelData.from
                                item.to = modelData.to
                                item.value = Qt.binding(modelData.getter)
                                item.valueModified.connect(function() {
                                    modelData.setter(item.value)
                                })
                            } else if (modelData.type === "property_popover") {
                                if (modelData.textGetter) {
                                    item.text = Qt.binding(modelData.textGetter)
                                }
                                if (modelData.placeholderText) {
                                    item.placeholderText = modelData.placeholderText
                                }
                                item.panelRequested.connect(function() {
                                    root.panelSelectorClicked(item, modelData.popoverType)
                                })
                            } else if (modelData.type === "combobox") {
                                // For weight combobox, update model when font changes
                                if (modelData.name === "Weight") {
                                    function updateWeightModel() {
                                        var currentValue = modelData.getter()
                                        item.model = modelData.model()
                                        
                                        // Try to find exact match
                                        var idx = item.model.indexOf(currentValue)
                                        
                                        if (idx < 0 && item.model.length > 0) {
                                            // If exact match not found, find closest weight
                                            var weightMap = {
                                                "Thin": 100,
                                                "Extra Light": 200,
                                                "Light": 300,
                                                "Normal": 400,
                                                "Medium": 500,
                                                "Demi Bold": 600,
                                                "Bold": 700,
                                                "Extra Bold": 800,
                                                "Black": 900
                                            }
                                            
                                            var currentWeight = weightMap[currentValue] || 400
                                            var closestIdx = 0
                                            var closestDiff = 1000
                                            
                                            for (var i = 0; i < item.model.length; i++) {
                                                var modelWeight = weightMap[item.model[i]] || 400
                                                var diff = Math.abs(modelWeight - currentWeight)
                                                if (diff < closestDiff) {
                                                    closestDiff = diff
                                                    closestIdx = i
                                                }
                                            }
                                            
                                            idx = closestIdx
                                        }
                                        
                                        item.currentIndex = idx >= 0 ? idx : 0
                                    }
                                    
                                    // Initial setup
                                    updateWeightModel()
                                    
                                    // Watch for font family changes on the selected element
                                    if (root.selectedElement) {
                                        root.selectedElement.fontChanged.connect(updateWeightModel)
                                    }
                                    
                                    // Update when fonts are loaded
                                    root.fontLoadTriggerChanged.connect(updateWeightModel)
                                } else {
                                    item.model = modelData.model()
                                    item.currentIndex = Qt.binding(function() {
                                        var value = modelData.getter()
                                        var idx = modelData.model().indexOf(value)
                                        return idx >= 0 ? idx : 0
                                    })
                                }
                                
                                item.onCurrentTextChanged.connect(function() {
                                    // Only set if the value actually changed
                                    if (item.currentText && item.currentText !== modelData.getter()) {
                                        modelData.setter(item.currentText)
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