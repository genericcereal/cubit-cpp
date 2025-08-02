import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import Cubit 1.0
import "."

Rectangle {
    id: root
    width: 300
    height: 50
    radius: 8
    color: ConfigObject.actionsPanelBackground
    antialiasing: true
    
    signal modeChanged(int mode)
    signal compileClicked()
    signal createVariableClicked()
    signal webElementsClicked()
    
    // Expose the web elements button for positioning
    property alias webElementsButton: webElementsButton
    property alias shapeButton: shapeButton
    
    property int currentMode: CanvasController.Select
    property var canvas: null  // Will be set by parent
    property bool isScriptMode: canvas && canvas.viewMode === "script"
    property bool isVariantMode: canvas && canvas.viewMode === "variant"
    property bool isGlobalElementsMode: canvas && canvas.viewMode === "globalElements"
    property bool needsCompilation: {
        if (!canvas) return false
        if (!canvas.activeScripts) return false
        return !canvas.activeScripts.isCompiled
    }
    property bool hasWebPlatform: {
        if (!canvas) return false
        if (!canvas.project) return false
        if (!canvas.project.platforms) return false
        // Check if any platform has name "web"
        for (var i = 0; i < canvas.project.platforms.length; i++) {
            if (canvas.project.platforms[i].name === "web") {
                return true
            }
        }
        return false
    }
    
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        
        // Script mode: Show action buttons (no active state)
        ToolButton {
            id: compileButton
            visible: isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            
            contentItem: Text {
                text: "C"
                color: {
                    if (needsCompilation) {
                        return parent.hovered ? "#ffffff" : "#00ff00"  // Green when needs compilation
                    } else {
                        return parent.hovered ? "#ffffff" : "#aaaaaa"  // Gray when compiled
                    }
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: {
                    if (parent.hovered) {
                        return needsCompilation ? Qt.rgba(0.0, 0.5, 0.0, 0.3) : Qt.rgba(0.4, 0.4, 0.4, 0.3)
                    } else {
                        return "transparent"
                    }
                }
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                root.compileClicked()
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Compile"
        }
        
        // Variable button for script mode
        ToolButton {
            id: scriptVariableButton
            visible: isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            
            contentItem: Text {
                text: "V"
                color: parent.hovered ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(0.4, 0.4, 0.4, 0.3) : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                root.createVariableClicked()
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Create Variable"
        }
        
        // Back to Design Canvas button
        ToolButton {
            id: backButton
            visible: isScriptMode || isVariantMode || isGlobalElementsMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            
            contentItem: Text {
                text: "B"
                color: parent.hovered ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(0.4, 0.4, 0.4, 0.3) : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (canvas) {
                    canvas.setEditingElement(null, "design")
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Back to Design Canvas"
        }
        
        // Design mode buttons
        ToolButton {
            id: selectButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === CanvasController.Select
            
            contentItem: Text {
                text: "S"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    currentMode = CanvasController.Select
                    root.modeChanged(CanvasController.Select)
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Select Mode"
        }
        
        ToolButton {
            id: frameButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === CanvasController.Frame
            
            contentItem: Text {
                text: "F"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    currentMode = CanvasController.Frame
                    root.modeChanged(CanvasController.Frame)
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Frame Mode"
        }
        
        ToolButton {
            id: textButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === CanvasController.Text
            
            contentItem: Text {
                text: "T"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    currentMode = CanvasController.Text
                    root.modeChanged(CanvasController.Text)
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Text Mode"
        }
        
        ToolButton {
            id: shapeButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: false  // Not a toggle button anymore
            
            contentItem: Text {
                text: "Sh"
                color: parent.hovered ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(0.4, 0.4, 0.4, 0.3) : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    shapePopoverVisible = !shapePopoverVisible
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Shapes"
        }
        
        ToolButton {
            id: penButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: true
            checked: currentMode === CanvasController.ShapePen
            
            contentItem: Text {
                text: "P"
                color: parent.checked ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.checked ? "#0066cc" : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    currentMode = CanvasController.ShapePen
                    root.modeChanged(CanvasController.ShapePen)
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Pen Mode"
        }
        
        
        ToolButton {
            id: variableButton
            visible: !isScriptMode
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: false  // Not a toggle button anymore
            
            contentItem: Text {
                text: "V"
                color: parent.hovered ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(0.4, 0.4, 0.4, 0.3) : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    root.createVariableClicked()
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Create Variable"
        }
        
        ToolButton {
            id: webElementsButton
            visible: !isScriptMode && hasWebPlatform
            Layout.fillHeight: true
            Layout.preferredWidth: height
            checkable: false
            
            contentItem: Text {
                text: "W"
                color: parent.hovered ? "#ffffff" : "#aaaaaa"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
            }
            
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(0.4, 0.4, 0.4, 0.3) : "transparent"
                radius: 4
                antialiasing: true
            }
            
            onClicked: {
                if (!isScriptMode) {
                    webElementsPopoverVisible = !webElementsPopoverVisible
                }
            }
            
            ToolTip.visible: hovered
            ToolTip.text: "Web Elements"
        }
        
        Item {
            Layout.fillWidth: true
        }
    }
    
    // Property to track popover visibility
    property bool webElementsPopoverVisible: false
    property bool shapePopoverVisible: false
    
    // WebElements popover
    Item {
        id: popoverContainer
        visible: webElementsPopoverVisible
        
        // Fill the entire window to catch clicks
        anchors.fill: parent
        parent: root.Window.window ? root.Window.window.contentItem : root
        z: 1000
        
        // Transparent background to catch clicks
        MouseArea {
            anchors.fill: parent
            onClicked: {
                webElementsPopoverVisible = false
            }
        }
        
        // The actual popover
        ActionsSelectPopover {
            id: webElementsPopover
            
            // Position above the W button using manual positioning
            Component.onCompleted: updatePosition()
            
            function updatePosition() {
                if (webElementsButton) {
                    var buttonPos = webElementsButton.mapToItem(parent, 0, 0)
                    x = buttonPos.x + (webElementsButton.width / 2) - (width / 2)
                    y = buttonPos.y - height - 10
                }
            }
            
            options: [
                { text: "Select", value: "select" },
                { text: "Text", value: "text" },
                { text: "Radio", value: "radio" }
            ]
            
            onOptionSelected: function(value) {
                if (canvas && canvas.console) {
                    canvas.console.addInfo("WebElements option selected: " + value)
                }
                webElementsPopoverVisible = false
                
                // Switch canvas mode based on selection
                if (value === "text" && canvas && canvas.controller) {
                    currentMode = CanvasController.WebTextInput
                    root.modeChanged(CanvasController.WebTextInput)
                }
            }
            
            onDismissed: {
                webElementsPopoverVisible = false
            }
        }
    }
    
    // Update popover position when visibility changes
    onWebElementsPopoverVisibleChanged: {
        if (webElementsPopoverVisible && popoverContainer.children[1]) {
            popoverContainer.children[1].updatePosition()
        }
    }
    
    // Shape popover
    Item {
        id: shapePopoverContainer
        visible: shapePopoverVisible
        
        // Fill the entire window to catch clicks
        anchors.fill: parent
        parent: root.Window.window ? root.Window.window.contentItem : root
        z: 1000
        
        // Transparent background to catch clicks
        MouseArea {
            anchors.fill: parent
            onClicked: {
                shapePopoverVisible = false
            }
        }
        
        // The actual popover
        ActionsSelectPopover {
            id: shapePopover
            
            // Position above the Sh button using manual positioning
            Component.onCompleted: updatePosition()
            
            function updatePosition() {
                if (shapeButton) {
                    var buttonPos = shapeButton.mapToItem(parent, 0, 0)
                    x = buttonPos.x + (shapeButton.width / 2) - (width / 2)
                    y = buttonPos.y - height - 10
                }
            }
            
            options: [
                { text: "Square", value: "square" },
                { text: "Triangle", value: "triangle" }
            ]
            
            onOptionSelected: function(value) {
                if (canvas && canvas.console) {
                    canvas.console.addInfo("Shape option selected: " + value)
                }
                shapePopoverVisible = false
                
                // Switch canvas mode based on shape selection
                if (canvas && canvas.controller) {
                    if (value === "square") {
                        currentMode = CanvasController.ShapeSquare
                        root.modeChanged(CanvasController.ShapeSquare)
                    } else if (value === "triangle") {
                        currentMode = CanvasController.ShapeTriangle
                        root.modeChanged(CanvasController.ShapeTriangle)
                    }
                }
            }
            
            onDismissed: {
                shapePopoverVisible = false
            }
        }
    }
    
    // Update shape popover position when visibility changes
    onShapePopoverVisibleChanged: {
        if (shapePopoverVisible && shapePopoverContainer.children[1]) {
            shapePopoverContainer.children[1].updatePosition()
        }
    }
}