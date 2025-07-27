import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."

ScrollView {
    id: root
    
    property var selectionManager
    property var canvas: null
    property var selectedElement: selectionManager && selectionManager.selectionCount === 1 
                                 ? selectionManager.selectedElements[0] : null
    
    // Signal emitted when PropertyPopover is clicked
    signal panelSelectorClicked(var selector, string type)
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
        // Dynamic property groups based on PropertyRegistry
        Repeater {
            model: propertyGroups
            
            PropertyGroup {
                title: modelData.title
                visible: modelData.properties.length > 0
                
                Repeater {
                    model: modelData.properties
                    
                    PropertyField {
                        id: field
                        name: modelData.name
                        visible: modelData.visible
                        
                        Component {
                            id: textFieldComponent
                            TextField {
                                text: modelData.value
                                onTextChanged: {
                                    if (root.selectedElement && text !== modelData.value) {
                                        root.selectedElement.setProperty(modelData.name, text)
                                    }
                                }
                            }
                        }
                        
                        Component {
                            id: colorFieldComponent
                            ColorField {
                                color: modelData.value
                                onColorChanged: {
                                    if (root.selectedElement && color !== modelData.value) {
                                        root.selectedElement.setProperty(modelData.name, color)
                                    }
                                }
                                onPopoverClicked: function(rect) {
                                    root.panelSelectorClicked(rect, "color")
                                }
                            }
                        }
                        
                        Component {
                            id: numberFieldComponent
                            SpinBox {
                                value: modelData.value
                                from: modelData.min || 0
                                to: modelData.max || 9999
                                onValueChanged: {
                                    if (root.selectedElement && value !== modelData.value) {
                                        root.selectedElement.setProperty(modelData.name, value)
                                    }
                                }
                            }
                        }
                        
                        Component {
                            id: boolFieldComponent
                            CheckBox {
                                checked: modelData.value
                                onCheckedChanged: {
                                    if (root.selectedElement && checked !== modelData.value) {
                                        root.selectedElement.setProperty(modelData.name, checked)
                                    }
                                }
                            }
                        }
                        
                        Component {
                            id: labelComponent
                            Label {
                                text: modelData.value
                                color: "#666666"
                            }
                        }
                        
                        Component {
                            id: enumFieldComponent
                            ComboBox {
                                model: modelData.enumValues || []
                                currentIndex: model.indexOf(modelData.value)
                                onCurrentTextChanged: {
                                    if (root.selectedElement && currentText !== modelData.value) {
                                        root.selectedElement.setProperty(modelData.name, currentText)
                                    }
                                }
                            }
                        }
                        
                        content: {
                            switch(modelData.type) {
                                case "text": return textFieldComponent
                                case "color": return colorFieldComponent
                                case "number": return numberFieldComponent
                                case "bool": return boolFieldComponent
                                case "label": return labelComponent
                                case "enum": return enumFieldComponent
                                default: return labelComponent
                            }
                        }
                    }
                }
            }
        }
        
        Item {
            Layout.fillHeight: true
        }
    }
    
    // Property groups dynamically generated from PropertyRegistry
    property var propertyGroups: []
    
    // Update property groups when selection changes
    onSelectedElementChanged: updatePropertyGroups()
    
    // Connect to property changes
    Connections {
        target: selectedElement
        
        function onPropertyChanged(name, value) {
            updatePropertyGroups()
        }
    }
    
    function updatePropertyGroups() {
        if (!selectedElement) {
            propertyGroups = []
            return
        }
        
        var groups = []
        var properties = selectedElement.propertyNames()
        
        // General properties
        var generalProps = []
        if (properties.includes("name")) {
            generalProps.push({
                name: "Name",
                type: "text",
                value: selectedElement.getProperty("name"),
                visible: true
            })
        }
        
        generalProps.push({
            name: "Type",
            type: "label",
            value: selectedElement.elementType,
            visible: true
        })
        
        generalProps.push({
            name: "ID",
            type: "label",
            value: selectedElement.elementId,
            visible: true
        })
        
        if (generalProps.length > 0) {
            groups.push({
                title: "General",
                properties: generalProps
            })
        }
        
        // Position properties
        var positionProps = []
        if (properties.includes("x")) {
            positionProps.push({
                name: "X",
                type: "number",
                value: Math.round(selectedElement.getProperty("x")),
                visible: true
            })
        }
        if (properties.includes("y")) {
            positionProps.push({
                name: "Y",
                type: "number",
                value: Math.round(selectedElement.getProperty("y")),
                visible: true
            })
        }
        
        if (positionProps.length > 0) {
            groups.push({
                title: "Position",
                properties: positionProps
            })
        }
        
        // Size properties
        var sizeProps = []
        if (properties.includes("width")) {
            sizeProps.push({
                name: "Width",
                type: "number",
                value: Math.round(selectedElement.getProperty("width")),
                visible: true
            })
        }
        if (properties.includes("height")) {
            sizeProps.push({
                name: "Height",
                type: "number",
                value: Math.round(selectedElement.getProperty("height")),
                visible: true
            })
        }
        
        if (sizeProps.length > 0) {
            groups.push({
                title: "Size",
                properties: sizeProps
            })
        }
        
        // Style properties (for Frame)
        var styleProps = []
        if (properties.includes("fill")) {
            styleProps.push({
                name: "Fill",
                type: "color",
                value: selectedElement.getProperty("fill"),
                visible: true
            })
        }
        if (properties.includes("borderColor")) {
            styleProps.push({
                name: "Border Color",
                type: "color",
                value: selectedElement.getProperty("borderColor"),
                visible: true
            })
        }
        if (properties.includes("borderWidth")) {
            styleProps.push({
                name: "Border Width",
                type: "number",
                value: selectedElement.getProperty("borderWidth"),
                min: 0,
                max: 20,
                visible: true
            })
        }
        if (properties.includes("borderRadius")) {
            styleProps.push({
                name: "Border Radius",
                type: "number",
                value: selectedElement.getProperty("borderRadius"),
                min: 0,
                max: 50,
                visible: true
            })
        }
        
        if (styleProps.length > 0) {
            groups.push({
                title: "Style",
                properties: styleProps
            })
        }
        
        // Text properties
        var textProps = []
        if (properties.includes("content")) {
            textProps.push({
                name: "Content",
                type: "text",
                value: selectedElement.getProperty("content"),
                visible: true
            })
        }
        if (properties.includes("color")) {
            textProps.push({
                name: "Color",
                type: "color",
                value: selectedElement.getProperty("color"),
                visible: true
            })
        }
        
        if (textProps.length > 0) {
            groups.push({
                title: "Text",
                properties: textProps
            })
        }
        
        // Shape properties
        var shapeProps = []
        if (properties.includes("shapeType")) {
            shapeProps.push({
                name: "Shape Type",
                type: "enum",
                value: ["Square", "Triangle", "Line"][selectedElement.getProperty("shapeType")],
                enumValues: ["Square", "Triangle", "Line"],
                visible: true
            })
        }
        if (properties.includes("edgeColor")) {
            shapeProps.push({
                name: "Edge Color",
                type: "color",
                value: selectedElement.getProperty("edgeColor"),
                visible: true
            })
        }
        if (properties.includes("fillColor")) {
            shapeProps.push({
                name: "Fill Color",
                type: "color",
                value: selectedElement.getProperty("fillColor"),
                visible: true
            })
        }
        if (properties.includes("edgeWidth")) {
            shapeProps.push({
                name: "Edge Width",
                type: "number",
                value: selectedElement.getProperty("edgeWidth"),
                min: 0,
                max: 10,
                visible: true
            })
        }
        
        if (shapeProps.length > 0) {
            groups.push({
                title: "Shape",
                properties: shapeProps
            })
        }
        
        // Layout properties
        var layoutProps = []
        if (properties.includes("flex")) {
            layoutProps.push({
                name: "Flex Enabled",
                type: "bool",
                value: selectedElement.getProperty("flex"),
                visible: true
            })
        }
        
        if (layoutProps.length > 0) {
            groups.push({
                title: "Layout",
                properties: layoutProps
            })
        }
        
        propertyGroups = groups
    }
}