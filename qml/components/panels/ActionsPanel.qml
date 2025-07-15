import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "."

Rectangle {
    id: root
    width: 300
    height: 50
    radius: 8
    color: Qt.rgba(0, 0, 0, 0.8)
    antialiasing: true
    
    signal modeChanged(int mode)
    signal compileClicked()
    signal createVariableClicked()
    signal webElementsClicked()
    
    // Expose the web elements button for positioning
    property alias webElementsButton: webElementsButton
    
    property int currentMode: CanvasController.Select
    property bool isScriptMode: Application.activeCanvas && Application.activeCanvas.viewMode === "script"
    property bool isVariantMode: Application.activeCanvas && Application.activeCanvas.viewMode === "variant"
    property bool needsCompilation: {
        if (!Application.activeCanvas) return false
        if (!Application.activeCanvas.activeScripts) return false
        return !Application.activeCanvas.activeScripts.isCompiled
    }
    property bool hasWebPlatform: {
        if (!Application.activeCanvas) return false
        if (!Application.activeCanvas.platforms) return false
        return Application.activeCanvas.platforms.indexOf("web") !== -1
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
        
        // Back to Design Canvas button
        ToolButton {
            id: backButton
            visible: isScriptMode || isVariantMode
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
                if (Application.activeCanvas) {
                    Application.activeCanvas.setEditingElement(null, "design")
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
                ConsoleMessageRepository.addInfo("WebElements option selected: " + value)
                webElementsPopoverVisible = false
                
                // Switch canvas mode based on selection
                if (value === "text" && Application.activeCanvas && Application.activeCanvas.controller) {
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
}