import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

ColumnLayout {
    id: root
    spacing: 0
    
    property var selectedElement
    property var canvas: null
    
    signal panelSelectorClicked(var selector, string type)
    
    // Track expanded state of joint groups
    property var expandedJoints: ({})
    
    visible: selectedElement && selectedElement.elementType === "Shape"
    
    // Monitor shape editing state and collapse all joints when exiting edit mode
    Connections {
        target: Window.window ? Window.window.shapeControlsController : null
        enabled: target !== null
        function onIsEditingShapeChanged() {
            if (target && !target.isEditingShape) {
                // Collapse all joint groups when exiting shape edit mode
                root.expandedJoints = {}
            }
        }
    }
    
    property var shapeProps: [
        {
            name: "Shape Type",
            type: "label",
            getter: () => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    switch (selectedElement.shapeType) {
                        case 0: return "Square"
                        case 1: return "Triangle"
                        case 2: return "Pen"
                        default: return "Unknown"
                    }
                }
                return ""
            }
        },
        {
            name: "Edge Width",
            type: "spinbox",
            from: 0,
            to: 20,
            getter: () => selectedElement && selectedElement.edgeWidth !== undefined ? selectedElement.edgeWidth : 2,
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    selectedElement.edgeWidth = v
                }
            }
        },
        {
            name: "Edge Color",
            type: "object_input",
            popoverType: "edgeColor",
            placeholderText: "Add edge color",
            valueGetter: () => {
                if (selectedElement && selectedElement.elementType === "Shape" && selectedElement.edgeColor !== undefined) {
                    // Check if color is transparent
                    if (selectedElement.edgeColor.a === 0) {
                        return null; // Return null for transparent colors
                    }
                    return selectedElement.edgeColor;
                }
                return null;
            }
        },
        {
            name: "Fill",
            type: "object_input",
            popoverType: "fillColor",
            placeholderText: "Add fill",
            valueGetter: () => {
                if (selectedElement && selectedElement.elementType === "Shape" && selectedElement.fillColor !== undefined) {
                    // Check if color is transparent
                    if (selectedElement.fillColor.a === 0) {
                        return null; // Return null for transparent colors
                    }
                    return selectedElement.fillColor;
                }
                return null;
            }
        },
        {
            name: "Line Join",
            type: "combobox",
            model: ["miter", "round", "bevel"],
            getter: () => selectedElement && selectedElement.lineJoin !== undefined ? selectedElement.lineJoin : "miter",
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    selectedElement.lineJoin = v
                }
            }
        },
        {
            name: "Line Cap",
            type: "combobox",
            model: ["butt", "round", "square"],
            getter: () => selectedElement && selectedElement.lineCap !== undefined ? selectedElement.lineCap : "round",
            setter: v => {
                if (selectedElement && selectedElement.elementType === "Shape") {
                    selectedElement.lineCap = v
                }
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
        id: labelComp
        Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
    
    Component {
        id: spinBoxComp
        SpinBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: checkBoxComp
        CheckBox {
            Layout.fillWidth: true
        }
    }
    
    Component {
        id: comboBoxComp
        ComboBox {
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
        id: objectInputComp
        ObjectInput {
            Layout.fillWidth: true
        }
    }
    
    PropertyGroup {
        title: "Shape"
        Layout.fillWidth: true
        
        content: [
            Repeater {
                model: shapeProps.filter(prop => !prop.visible || prop.visible())
                
                delegate: LabeledField {
                    label: modelData.name
                    visible: !modelData.visible || modelData.visible()
                    
                    delegate: [
                        Loader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            sourceComponent: modelData.type === "label" ? labelComp
                                           : modelData.type === "spinbox" ? spinBoxComp
                                           : modelData.type === "checkbox" ? checkBoxComp
                                           : modelData.type === "combobox" ? comboBoxComp
                                           : modelData.type === "property_popover" ? propertyPopoverComp
                                           : modelData.type === "object_input" ? objectInputComp
                                           : null
                            
                            onLoaded: {
                                if (modelData.type === "label") {
                                    item.text = Qt.binding(modelData.getter)
                                } else if (modelData.type === "spinbox") {
                                    item.from = modelData.from
                                    item.to = modelData.to
                                    item.value = Qt.binding(modelData.getter)
                                    item.valueChanged.connect(function() {
                                        modelData.setter(item.value)
                                    })
                                } else if (modelData.type === "checkbox") {
                                    item.checked = Qt.binding(modelData.getter)
                                    item.toggled.connect(function() {
                                        modelData.setter(item.checked)
                                    })
                                } else if (modelData.type === "combobox") {
                                    item.model = modelData.model
                                    item.currentIndex = Qt.binding(function() {
                                        var value = modelData.getter()
                                        var idx = modelData.model.indexOf(value)
                                        return idx !== -1 ? idx : 0
                                    })
                                    item.currentIndexChanged.connect(function() {
                                        if (item.currentIndex >= 0 && item.currentIndex < modelData.model.length) {
                                            modelData.setter(modelData.model[item.currentIndex])
                                        }
                                    })
                                } else if (modelData.type === "property_popover") {
                                    if (modelData.elementFillColor) {
                                        item.elementFillColor = Qt.binding(modelData.elementFillColor)
                                    }
                                    if (modelData.textGetter) {
                                        item.text = Qt.binding(modelData.textGetter)
                                    }
                                    if (modelData.placeholderText) {
                                        item.placeholderText = modelData.placeholderText
                                    }
                                    item.panelRequested.connect(function() {
                                        root.panelSelectorClicked(item, modelData.popoverType)
                                    })
                                } else if (modelData.type === "object_input") {
                                    if (modelData.valueGetter) {
                                        item.value = Qt.binding(modelData.valueGetter)
                                    }
                                    if (modelData.placeholderText) {
                                        item.placeholderText = modelData.placeholderText
                                    }
                                    item.displayText = Qt.binding(function() {
                                        var val = item.value
                                        if (val && val.a !== undefined && val.a > 0) {
                                            var format = selectedElement && selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1
                                            return item.formatColor(val, format)
                                        }
                                        return ""
                                    })
                                    item.previewType = ObjectInput.PreviewType.Color
                                    item.clicked.connect(function() {
                                        // If no value, set default color before opening picker
                                        if (!item.value && selectedElement && selectedElement.elementType === "Shape") {
                                            if (modelData.popoverType === "edgeColor") {
                                                selectedElement.edgeColor = Qt.rgba(0, 0, 0, 1) // Default black for edges
                                            } else if (modelData.popoverType === "fillColor") {
                                                if (selectedElement.shapeType === 2) {
                                                    // For Lines, default to transparent
                                                    selectedElement.fillColor = Qt.rgba(0, 0, 0, 0) // Default transparent for line fill
                                                } else {
                                                    // For other shapes, default to white
                                                    selectedElement.fillColor = Qt.rgba(1, 1, 1, 1) // Default white for fill
                                                }
                                            }
                                        }
                                        root.panelSelectorClicked(item, modelData.popoverType)
                                    })
                                    item.cleared.connect(function() {
                                        if (selectedElement && selectedElement.elementType === "Shape") {
                                            if (modelData.popoverType === "edgeColor") {
                                                selectedElement.edgeColor = Qt.rgba(0, 0, 0, 0) // Fully transparent
                                            } else if (modelData.popoverType === "fillColor") {
                                                selectedElement.fillColor = Qt.rgba(0, 0, 0, 0) // Fully transparent
                                            }
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
    
    PropertyGroup {
        title: "Joints"
        Layout.fillWidth: true
        visible: selectedElement && selectedElement.elementType === "Shape" && selectedElement.joints && selectedElement.joints.length > 0
        
        content: [
            Repeater {
                model: selectedElement && selectedElement.elementType === "Shape" ? selectedElement.joints : []
                
                delegate: CollapsibleGroup {
                    groupTitle: "Joint " + (index + 1)
                    
                    // Track the joint index for this delegate
                    property int jointIndex: index
                    
                    // Initialize expanded state from external tracking
                    Component.onCompleted: {
                        expanded = root.expandedJoints[jointIndex] === true
                    }
                    
                    // Watch for external changes to expandedJoints
                    Connections {
                        target: root
                        function onExpandedJointsChanged() {
                            // Update expanded state when expandedJoints changes externally
                            expanded = root.expandedJoints[jointIndex] === true
                        }
                    }
                    
                    onExpandedChanged: {
                        // Update the tracking object when user manually toggles
                        var newExpandedJoints = Object.assign({}, root.expandedJoints)
                        if (expanded) {
                            newExpandedJoints[jointIndex] = true
                        } else {
                            delete newExpandedJoints[jointIndex]
                        }
                        root.expandedJoints = newExpandedJoints
                    }
                    
                    // Check if this joint is currently selected
                    property bool isActiveJoint: {
                        // Access shapeControlsController from the window
                        var window = Window.window
                        if (window && window.shapeControlsController) {
                            return window.shapeControlsController.selectedJointIndex === index
                        }
                        return false
                    }
                    
                    // Override border color when active
                    border.color: isActiveJoint ? "#0066ff" : "#e0e0e0"
                    border.width: isActiveJoint ? 2 : 1
                    
                    content: [
                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            
                            Label {
                                text: "X:"
                                font.pixelSize: 12
                            }
                            
                            TextField {
                                Layout.preferredWidth: 80
                                Layout.fillWidth: true
                                text: modelData.x !== undefined ? modelData.x.toFixed(2) : "0.00"
                                selectByMouse: true
                                
                                onEditingFinished: {
                                    var newX = parseFloat(text)
                                    if (!isNaN(newX) && selectedElement && selectedElement.elementType === "Shape") {
                                        var joints = selectedElement.joints.slice() // Create a copy of the array
                                        if (joints && index < joints.length) {
                                            joints[index] = Qt.point(newX, joints[index].y)
                                            selectedElement.setJoints(joints)
                                        }
                                    }
                                }
                            }
                            
                            Label {
                                text: "Y:"
                                font.pixelSize: 12
                            }
                            
                            TextField {
                                Layout.preferredWidth: 80
                                Layout.fillWidth: true
                                text: modelData.y !== undefined ? modelData.y.toFixed(2) : "0.00"
                                selectByMouse: true
                                
                                onEditingFinished: {
                                    var newY = parseFloat(text)
                                    if (!isNaN(newY) && selectedElement && selectedElement.elementType === "Shape") {
                                        var joints = selectedElement.joints.slice() // Create a copy of the array
                                        if (joints && index < joints.length) {
                                            joints[index] = Qt.point(joints[index].x, newY)
                                            selectedElement.setJoints(joints)
                                        }
                                    }
                                }
                            }
                        },
                        
                        // Mirroring mode - only visible for Pen shapes
                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            visible: selectedElement && selectedElement.shapeType === 2 // Pen shape
                            
                            Label {
                                text: "Mirroring:"
                                font.pixelSize: 12
                                Layout.preferredWidth: 90
                            }
                            
                            ComboBox {
                                id: mirroringCombo
                                Layout.fillWidth: true
                                model: ["No Mirroring", "Mirror Angle", "Mirror Angle and Length"]
                                currentIndex: {
                                    if (modelData.mirroring !== undefined) {
                                        // 0 = NoMirroring, 1 = MirrorAngle, 2 = MirrorAngleAndLength
                                        return Math.max(0, Math.min(2, modelData.mirroring))
                                    }
                                    return 0
                                }
                                
                                onCurrentIndexChanged: {
                                    if (selectedElement && selectedElement.elementType === "Shape") {
                                        // Map combo index directly to mirroring type enum
                                        selectedElement.setJointMirroring(index, currentIndex)
                                    }
                                }
                                
                                Connections {
                                    target: selectedElement
                                    function onJointsChanged() {
                                        if (selectedElement && selectedElement.joints && index < selectedElement.joints.length) {
                                            var joint = selectedElement.joints[index]
                                            if (joint && joint.mirroring !== undefined) {
                                                mirroringCombo.currentIndex = Math.max(0, Math.min(2, joint.mirroring))
                                            }
                                        }
                                    }
                                }
                            }
                        },
                        
                        // Corner radius - only visible for Pen shapes
                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            visible: selectedElement && selectedElement.shapeType === 2 // Pen shape
                            
                            Label {
                                text: "Corner Radius:"
                                font.pixelSize: 12
                                Layout.preferredWidth: 90
                            }
                            
                            SpinBox {
                                id: cornerRadiusSpinbox
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: modelData.cornerRadius !== undefined ? modelData.cornerRadius : 0
                                editable: true
                                
                                onValueModified: {
                                    if (selectedElement && selectedElement.elementType === "Shape") {
                                        selectedElement.setJointCornerRadius(index, value)
                                    }
                                }
                                
                                Connections {
                                    target: selectedElement
                                    function onJointsChanged() {
                                        if (selectedElement && selectedElement.joints && index < selectedElement.joints.length) {
                                            cornerRadiusSpinbox.value = selectedElement.joints[index].cornerRadius || 0
                                        }
                                    }
                                }
                            }
                        }
                    ]
                }
            }
        ]
    }
}