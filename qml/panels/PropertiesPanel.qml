import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"

ScrollView {
    id: root
    
    property var selectionManager
    property var selectedElement: selectionManager && selectionManager.selectionCount === 1 
                                 ? selectionManager.selectedElements[0] : null
    
    // Helper property to access DesignElement-specific properties
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    // Signal emitted when PropertyPopover is clicked
    signal panelSelectorClicked(var selector, string type)
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
        
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
        
        // Actions section for design elements
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Actions"
            visible: selectedElement && selectedElement.isDesignElement
            
            GridLayout {
                anchors.fill: parent
                columns: 1
                columnSpacing: 10
                rowSpacing: 5
                
                Button {
                    Layout.fillWidth: true
                    text: "Create component"
                    font.pixelSize: 14
                    
                    background: Rectangle {
                        color: parent.pressed ? "#e0e0e0" : (parent.hovered ? "#f0f0f0" : "#ffffff")
                        border.color: "#d0d0d0"
                        border.width: 1
                        radius: 4
                        antialiasing: true
                    }
                    
                    onClicked: {
                        if (selectedDesignElement) {
                            // Call createComponent on the selected design element
                            var component = selectedDesignElement.createComponent()
                            if (component) {
                                ConsoleMessageRepository.addOutput("Component created with ID: " + component.elementId)
                            }
                        }
                    }
                }
            }
        }
        
        // Properties when element is selected
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "General"
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
                    visible: selectedElement && selectedElement.elementType === "Frame" && Application.activeCanvas && Application.activeCanvas.platforms.length > 0
                }
                ComboBox {
                    Layout.fillWidth: true
                    visible: selectedElement && selectedElement.elementType === "Frame" && Application.activeCanvas && Application.activeCanvas.platforms.length > 0
                    model: {
                        if (!Application.activeCanvas || !selectedElement || selectedElement.elementType !== "Frame") {
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
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            var platform = selectedElement.platform || "undefined"
                            var index = model.indexOf(platform)
                            return index >= 0 ? index : 0
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            var platform = model[index]
                            selectedElement.platform = platform === "undefined" ? "" : platform
                        }
                    }
                }
                
                Label { 
                    text: "Role:" 
                    visible: selectedElement && selectedElement.elementType === "Frame" && selectedElement.platform && selectedElement.platform !== ""
                }
                ComboBox {
                    Layout.fillWidth: true
                    visible: selectedElement && selectedElement.elementType === "Frame" && selectedElement.platform && selectedElement.platform !== ""
                    model: ["container"]
                    currentIndex: 0  // Always show container since it's the only option
                    onActivated: function(index) {
                        if (selectedElement && selectedElement.elementType === "Frame") {
                            // Since we only have "container" option, always set role to container (value 1)
                            selectedElement.role = 1  // container
                        }
                    }
                }
            }
        }
        
        // Position section - only for DesignElements with a parent
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Position"
            visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                // Position type for Frame and Text elements
                Label { 
                    text: "Position:" 
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")
                }
                ComboBox {
                    Layout.fillWidth: true
                    visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")
                    model: ["Relative", "Absolute", "Fixed"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")) {
                            // Map PositionType enum values to combo box indices
                            switch (selectedElement.position) {
                                case 0: return 0  // Relative
                                case 1: return 1  // Absolute
                                case 2: return 2  // Fixed
                                default: return 0  // Default to Relative
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "Text")) {
                            // Map combo box index to PositionType enum value
                            selectedElement.position = index
                        }
                    }
                }
                
                Label { text: "Left:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: leftSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedDesignElement && selectedDesignElement.left !== undefined ? Math.round(selectedDesignElement.left) : 0
                        onValueModified: function(value) {
                            if (selectedDesignElement && selectedDesignElement.left !== undefined && value !== Math.round(selectedDesignElement.left)) {
                                selectedDesignElement.left = value
                            }
                        }
                    }
                    
                    // Anchor button for left
                    Button {
                        id: leftAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedDesignElement && selectedDesignElement.leftAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: leftAnchorButton.checked ? "#4080ff" : (leftAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: leftAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedDesignElement) {
                                if (checked) {
                                    // Set the anchor value to the spin box value before enabling
                                    selectedDesignElement.left = leftSpinBox.value
                                }
                                selectedDesignElement.leftAnchored = checked
                            }
                        }
                    }
                }
                
                Label { text: "Top:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: topSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedDesignElement && selectedDesignElement.top !== undefined ? Math.round(selectedDesignElement.top) : 0
                        onValueModified: function(value) {
                            if (selectedDesignElement && selectedDesignElement.top !== undefined && value !== Math.round(selectedDesignElement.top)) {
                                selectedDesignElement.top = value
                            }
                        }
                    }
                    
                    // Anchor button for top
                    Button {
                        id: topAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedDesignElement && selectedDesignElement.topAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: topAnchorButton.checked ? "#4080ff" : (topAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: topAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedDesignElement) {
                                if (checked) {
                                    // Set the anchor value to the spin box value before enabling
                                    selectedDesignElement.top = topSpinBox.value
                                }
                                selectedDesignElement.topAnchored = checked
                            }
                        }
                    }
                }
                
                Label { text: "Right:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: rightSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedDesignElement && selectedDesignElement.right !== undefined ? Math.round(selectedDesignElement.right) : 0
                        onValueModified: function(value) {
                            if (selectedDesignElement && selectedDesignElement.right !== undefined && value !== Math.round(selectedDesignElement.right)) {
                                selectedDesignElement.right = value
                            }
                        }
                    }
                    
                    // Anchor button for right
                    Button {
                        id: rightAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedDesignElement && selectedDesignElement.rightAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: rightAnchorButton.checked ? "#4080ff" : (rightAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: rightAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedDesignElement) {
                                if (checked) {
                                    // Set the anchor value to the spin box value before enabling
                                    selectedDesignElement.right = rightSpinBox.value
                                }
                                selectedDesignElement.rightAnchored = checked
                            }
                        }
                    }
                }
                
                Label { text: "Bottom:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: bottomSpinBox
                        Layout.fillWidth: true
                        from: -9999
                        to: 9999
                        value: selectedDesignElement && selectedDesignElement.bottom !== undefined ? Math.round(selectedDesignElement.bottom) : 0
                        onValueModified: function(value) {
                            if (selectedDesignElement && selectedDesignElement.bottom !== undefined && value !== Math.round(selectedDesignElement.bottom)) {
                                selectedDesignElement.bottom = value
                            }
                        }
                    }
                    
                    // Anchor button for bottom
                    Button {
                        id: bottomAnchorButton
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        checkable: true
                        checked: selectedDesignElement && selectedDesignElement.bottomAnchored
                        text: "⚓"
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: bottomAnchorButton.checked ? "#4080ff" : (bottomAnchorButton.hovered ? "#f0f0f0" : "#ffffff")
                            border.color: bottomAnchorButton.checked ? "#2060ff" : "#d0d0d0"
                            border.width: 1
                            radius: 4
                        }
                        
                        onToggled: {
                            if (selectedDesignElement) {
                                if (checked) {
                                    // Set the anchor value to the spin box value before enabling
                                    selectedDesignElement.bottom = bottomSpinBox.value
                                }
                                selectedDesignElement.bottomAnchored = checked
                            }
                        }
                    }
                }
            }
        }
        
        // Position & Size section - for elements without a parent
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Position & Size"
            visible: selectedElement && selectedElement.isVisual && !(selectedElement.isDesignElement && selectedElement.parentId)
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "X:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.x) : 0
                    onValueModified: {
                        if (!selectedElement || !selectedElement.isVisual) return
                        if (value === undefined || isNaN(value)) return
                        if (value !== Math.round(selectedElement.x)) {
                            selectedElement.x = value
                        }
                    }
                }
                
                Label { text: "Y:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: -9999
                    to: 9999
                    value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.y) : 0
                    onValueModified: {
                        if (!selectedElement || !selectedElement.isVisual) return
                        if (value === undefined || isNaN(value)) return
                        if (value !== Math.round(selectedElement.y)) {
                            selectedElement.y = value
                        }
                    }
                }
                
                Label { text: "Width:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: widthSpinBox
                        Layout.fillWidth: true
                        from: 1
                        to: 9999
                        value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.width) : 0
                        onValueModified: {
                            if (!selectedElement || !selectedElement.isVisual) return
                            if (value === undefined || isNaN(value)) return
                            if (value !== Math.round(selectedElement.width)) {
                                selectedElement.width = value
                            }
                        }
                    }
                    
                    ComboBox {
                        Layout.preferredWidth: 100
                        model: ["fixed", "relative", "fill", "fit content", "viewport"]
                        visible: selectedElement && selectedElement.elementType === "Frame"
                        currentIndex: {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                switch (selectedElement.widthType) {
                                    case 0: return 0  // SizeFixed
                                    case 1: return 1  // SizeRelative
                                    case 2: return 2  // SizeFill
                                    case 3: return 3  // SizeFitContent
                                    case 4: return 4  // SizeViewport
                                    default: return 0
                                }
                            }
                            return 0
                        }
                        onActivated: function(index) {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                selectedElement.widthType = index
                            }
                        }
                    }
                }
                
                Label { text: "Height:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: heightSpinBox
                        Layout.fillWidth: true
                        from: 1
                        to: 9999
                        value: selectedElement && selectedElement.isVisual ? Math.round(selectedElement.height) : 0
                        onValueModified: {
                            if (!selectedElement || !selectedElement.isVisual) return
                            if (value === undefined || isNaN(value)) return
                            if (value !== Math.round(selectedElement.height)) {
                                selectedElement.height = value
                            }
                        }
                    }
                    
                    ComboBox {
                        Layout.preferredWidth: 100
                        model: ["fixed", "relative", "fill", "fit content", "viewport"]
                        visible: selectedElement && selectedElement.elementType === "Frame"
                        currentIndex: {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                switch (selectedElement.heightType) {
                                    case 0: return 0  // SizeFixed
                                    case 1: return 1  // SizeRelative
                                    case 2: return 2  // SizeFill
                                    case 3: return 3  // SizeFitContent
                                    case 4: return 4  // SizeViewport
                                    default: return 0
                                }
                            }
                            return 0
                        }
                        onActivated: function(index) {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                selectedElement.heightType = index
                            }
                        }
                    }
                }
            }
        }
        
        // Size section - for DesignElements with a parent
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Size"
            visible: selectedElement && selectedElement.isDesignElement && selectedElement.parentId
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Width:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: widthSpinBoxSeparate
                        Layout.fillWidth: true
                        from: 1
                        to: 9999
                        value: selectedElement ? Math.round(selectedElement.width) : 0
                        onValueModified: function(value) {
                            if (selectedElement && value !== Math.round(selectedElement.width)) {
                                selectedElement.width = value
                            }
                        }
                    }
                    
                    ComboBox {
                        Layout.preferredWidth: 100
                        model: ["fixed", "relative", "fill", "fit content", "viewport"]
                        visible: selectedElement && selectedElement.elementType === "Frame"
                        currentIndex: {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                switch (selectedElement.widthType) {
                                    case 0: return 0  // SizeFixed
                                    case 1: return 1  // SizeRelative
                                    case 2: return 2  // SizeFill
                                    case 3: return 3  // SizeFitContent
                                    case 4: return 4  // SizeViewport
                                    default: return 0
                                }
                            }
                            return 0
                        }
                        onActivated: function(index) {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                selectedElement.widthType = index
                            }
                        }
                    }
                }
                
                Label { text: "Height:" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    
                    SpinBox {
                        id: heightSpinBoxSeparate
                        Layout.fillWidth: true
                        from: 1
                        to: 9999
                        value: selectedElement ? Math.round(selectedElement.height) : 0
                        onValueModified: function(value) {
                            if (selectedElement && value !== Math.round(selectedElement.height)) {
                                selectedElement.height = value
                            }
                        }
                    }
                    
                    ComboBox {
                        Layout.preferredWidth: 100
                        model: ["fixed", "relative", "fill", "fit content", "viewport"]
                        visible: selectedElement && selectedElement.elementType === "Frame"
                        currentIndex: {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                switch (selectedElement.heightType) {
                                    case 0: return 0  // SizeFixed
                                    case 1: return 1  // SizeRelative
                                    case 2: return 2  // SizeFill
                                    case 3: return 3  // SizeFitContent
                                    case 4: return 4  // SizeViewport
                                    default: return 0
                                }
                            }
                            return 0
                        }
                        onActivated: function(index) {
                            if (selectedElement && selectedElement.elementType === "Frame") {
                                selectedElement.heightType = index
                            }
                        }
                    }
                }
            }
        }
        
        // Frame-specific properties
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: selectedElement && selectedElement.elementType === "ComponentVariant" ? "Variant Style" : "Frame"
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Fill:" }
                PropertyPopover {
                    id: fillSelector
                    Layout.fillWidth: true
                    
                    elementFillColor: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant") ? selectedElement.fill : "#add8e6"
                    text: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            var color = selectedElement.fill
                            var format = selectedElement.colorFormat !== undefined ? selectedElement.colorFormat : 1 // Default to HEX
                            
                            switch (format) {
                                case 0: // RGB (Frame.RGB)
                                    var r = Math.round(color.r * 255)
                                    var g = Math.round(color.g * 255)
                                    var b = Math.round(color.b * 255)
                                    return "rgb(" + r + ", " + g + ", " + b + ")"
                                    
                                case 1: // HEX (Frame.HEX)
                                    var rh = Math.round(color.r * 255).toString(16).padStart(2, '0')
                                    var gh = Math.round(color.g * 255).toString(16).padStart(2, '0')
                                    var bh = Math.round(color.b * 255).toString(16).padStart(2, '0')
                                    return "#" + rh + gh + bh
                                    
                                case 2: // HSL (Frame.HSL)
                                    var h = Math.round(color.hslHue * 360)
                                    var s = Math.round(color.hslSaturation * 100)
                                    var l = Math.round(color.hslLightness * 100)
                                    return "hsl(" + h + ", " + s + "%, " + l + "%)"
                                    
                                default:
                                    return elementFillColor.toString()
                            }
                        }
                        return elementFillColor.toString()
                    }
                    
                    onPanelRequested: {
                        // Pass both the selector and its type so parent knows what content to show
                        root.panelSelectorClicked(fillSelector, "fill")
                    }
                }
                
                Label { text: "Overflow:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Hidden", "Scroll", "Visible"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map Frame.OverflowMode enum values to combo box indices
                            switch (selectedElement.overflow) {
                                case 0: return 0  // Hidden
                                case 1: return 1  // Scroll
                                case 2: return 2  // Visible
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map combo box index to Frame.OverflowMode enum value
                            selectedElement.overflow = index
                        }
                    }
                }
                
                Label { text: "Border Radius:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: selectedElement && selectedElement.borderRadius !== undefined ? selectedElement.borderRadius : 0
                    onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) selectedElement.borderRadius = value
                }
                
                Label { text: "Border Width:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 20
                    value: selectedElement && selectedElement.borderWidth !== undefined ? selectedElement.borderWidth : 0
                    onValueChanged: if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) selectedElement.borderWidth = value
                }
                
                Label { text: "Style:" }
                PropertyPopover {
                    id: styleSelector
                    Layout.fillWidth: true
                    placeholderText: "Select style..."
                    
                    onPanelRequested: {
                        // Panel will be shown/hidden by the parent component
                        console.log("PropertyPopover clicked!")
                        // Pass the selector item itself so parent can calculate position
                        root.panelSelectorClicked(styleSelector, "style")
                    }
                }
            }
        }
        
        // Flex Layout checkbox for Frame
        CheckBox {
            id: flexCheckBox
            Layout.fillWidth: true
            Layout.margins: 10
            Layout.leftMargin: 10
            Layout.bottomMargin: 0
            text: "Flex Layout"
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")
            checked: selectedElement && selectedElement.flex !== undefined ? selectedElement.flex : false
            onToggled: {
                if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                    selectedElement.flex = checked
                }
            }
        }
        
        // Flex Layout properties GroupBox
        GroupBox {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.topMargin: 5
            Layout.bottomMargin: 10
            title: ""
            visible: selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant") && flexCheckBox.checked
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Orientation:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Row", "Column"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map Frame.LayoutOrientation enum values to combo box indices
                            switch (selectedElement.orientation) {
                                case 0: return 0  // Row
                                case 1: return 1  // Column
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map combo box index to Frame.LayoutOrientation enum value
                            selectedElement.orientation = index
                        }
                    }
                }
                
                Label { text: "Gap:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: selectedElement && selectedElement.gap !== undefined ? Math.round(selectedElement.gap) : 0
                    onValueChanged: {
                        if (value !== undefined && selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            selectedElement.gap = value
                        }
                    }
                }
                
                Label { text: "Justify:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Start", "End", "Center", "Space Between", "Space Around", "Space Evenly"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map Frame.JustifyContent enum values to combo box indices
                            switch (selectedElement.justify) {
                                case 0: return 0  // JustifyStart
                                case 1: return 1  // JustifyEnd
                                case 2: return 2  // JustifyCenter
                                case 3: return 3  // JustifySpaceBetween
                                case 4: return 4  // JustifySpaceAround
                                case 5: return 5  // JustifySpaceEvenly
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map combo box index to Frame.JustifyContent enum value
                            selectedElement.justify = index
                        }
                    }
                }
                
                Label { text: "Align:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Start", "End", "Center", "Baseline", "Stretch"]
                    currentIndex: {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map Frame.AlignItems enum values to combo box indices
                            switch (selectedElement.align) {
                                case 0: return 0  // AlignStart
                                case 1: return 1  // AlignEnd
                                case 2: return 2  // AlignCenter
                                case 3: return 3  // AlignBaseline
                                case 4: return 4  // AlignStretch
                                default: return 0
                            }
                        }
                        return 0
                    }
                    onActivated: function(index) {
                        if (selectedElement && (selectedElement.elementType === "Frame" || selectedElement.elementType === "ComponentVariant")) {
                            // Map combo box index to Frame.AlignItems enum value
                            selectedElement.align = index
                        }
                    }
                }
                
                Label { text: "Wrap:" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Yes", "No"]
                    currentIndex: 1  // Default to "No"
                }
                
                Label { text: "Padding:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: 0
                }
            }
        }
        
        // Text-specific properties
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Text"
            visible: selectedElement && selectedElement.elementType === "Text"
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Content:" }
                TextField {
                    Layout.fillWidth: true
                    text: selectedElement && selectedElement.content !== undefined ? selectedElement.content : ""
                    onTextChanged: if (selectedElement && selectedElement.elementType === "Text") selectedElement.content = text
                }
                
                Label { text: "Size:" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 8
                    to: 144
                    value: selectedElement && selectedElement.font ? selectedElement.font.pixelSize : 14
                    onValueChanged: {
                        if (selectedElement && selectedElement.elementType === "Text") {
                            var newFont = selectedElement.font
                            newFont.pixelSize = value
                            selectedElement.font = newFont
                        }
                    }
                }
            }
        }
        
        // Component-specific properties (for frames inside component variants)
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Component"
            visible: {
                if (!selectedElement || selectedElement.elementType !== "Frame" || !selectedElement.parentId) {
                    return false
                }
                // Check if parent element is a ComponentVariant
                if (!Application.activeCanvas) return false
                var model = Application.activeCanvas.elementModel
                if (!model) return false
                
                var parentElement = model.getElementById(selectedElement.parentId)
                return parentElement && parentElement.elementType === "ComponentVariant"
            }
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Accepts children:" }
                CheckBox {
                    Layout.fillWidth: true
                    checked: false
                    onToggled: {
                        // For now, this doesn't do anything as requested
                    }
                }
            }
        }
        
        // Component Variant properties
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Component Variant"
            visible: selectedElement && selectedElement.elementType === "ComponentVariant"
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 5
                
                Label { text: "Accepts children:" }
                CheckBox {
                    Layout.fillWidth: true
                    checked: selectedElement && selectedElement.elementType === "ComponentVariant" ? selectedElement.instancesAcceptChildren : true
                    onToggled: {
                        if (selectedElement && selectedElement.elementType === "ComponentVariant") {
                            selectedElement.instancesAcceptChildren = checked
                        }
                    }
                }
            }
        }
        
        // Variable-specific properties
        GroupBox {
            Layout.fillWidth: true
            Layout.margins: 10
            title: "Variable"
            visible: selectedElement && selectedElement.elementType === "Variable"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                // Array toggle
                RowLayout {
                    Layout.fillWidth: true
                    
                    Label { text: "Is Array:" }
                    CheckBox {
                        id: isArrayCheckBox
                        checked: selectedElement && selectedElement.isArray ? selectedElement.isArray : false
                        onToggled: if (selectedElement && selectedElement.elementType === "Variable") selectedElement.isArray = checked
                    }
                }
                
                // Single value field (shown when not array)
                RowLayout {
                    Layout.fillWidth: true
                    visible: !isArrayCheckBox.checked
                    
                    Label { 
                        text: "Value:" 
                        Layout.preferredWidth: 50
                    }
                    TextField {
                        Layout.fillWidth: true
                        text: selectedElement && selectedElement.value !== undefined ? selectedElement.value.toString() : ""
                        onTextChanged: if (selectedElement && selectedElement.elementType === "Variable") selectedElement.value = text
                    }
                }
                
                // Array values (shown when is array)
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: isArrayCheckBox.checked
                    spacing: 5
                    
                    Label {
                        text: "Array Values:"
                        font.weight: Font.Medium
                    }
                    
                    // Array items
                    Repeater {
                        model: selectedElement && selectedElement.isArray && selectedElement.arrayValues ? selectedElement.arrayValues : []
                        
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            
                            Label {
                                text: "[" + index + "]"
                                Layout.preferredWidth: 40
                            }
                            
                            TextField {
                                Layout.fillWidth: true
                                text: modelData ? modelData.toString() : ""
                                onTextChanged: {
                                    if (selectedElement && selectedElement.elementType === "Variable" && selectedElement.isArray) {
                                        selectedElement.setArrayValue(index, text)
                                    }
                                }
                            }
                            
                            Button {
                                text: "−"
                                Layout.preferredWidth: 30
                                Layout.preferredHeight: 30
                                enabled: selectedElement && selectedElement.arrayLength > 1
                                onClicked: {
                                    if (selectedElement && selectedElement.elementType === "Variable") {
                                        selectedElement.removeArrayValue(index)
                                    }
                                }
                            }
                        }
                    }
                    
                    // Add new value button
                    Button {
                        text: "+ Add Value"
                        Layout.fillWidth: true
                        onClicked: {
                            if (selectedElement && selectedElement.elementType === "Variable" && selectedElement.isArray) {
                                selectedElement.addArrayValue("")
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
}