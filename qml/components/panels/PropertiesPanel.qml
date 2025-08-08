import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import ".."
import "../../PropertyHelpers.js" as PropertyHelpers

ScrollView {
    id: root
    
    property var selectionManager
    property var canvas: null  // Will be set by parent
    property var selectedElement: selectionManager && selectionManager.selectionCount === 1 
                                 ? selectionManager.selectedElements[0] : null
    
    // Find the outermost instance for the selected element (or the element itself if it's outermost)
    property var outermostInstance: {
        if (!selectedElement || !canvas || !canvas.elementModel) {
            return null
        }
        try {
            var outermost = canvas.elementModel.findOutermostInstanceForElement(selectedElement)
            return outermost || null
        } catch (e) {
            // Handle case where objects are being destroyed
            return null
        }
    }
    
    // Use the outermost instance for display if available, otherwise use selected element
    property var displayElement: outermostInstance || selectedElement
    
    // Helper property to access DesignElement-specific properties
    property var selectedDesignElement: displayElement && displayElement.isDesignElement ? displayElement : null
    
    // Helper property to get editable properties for ComponentInstance
    property var editableProperties: {
        if (displayElement && displayElement.elementType === "FrameComponentInstance") {
            return displayElement.getEditableProperties()
        }
        return []
    }
    
    // Get all descendants when displaying an outermost instance configuration
    property var descendants: {
        if (!selectedDesignElement || !canvas || !canvas.elementModel) {
            return []
        }
        
        try {
            if (selectedDesignElement.isInstance && 
                selectedDesignElement.isInstance() && 
                (!selectedDesignElement.ancestorInstance || selectedDesignElement.ancestorInstance === "")) {
                // This is an outermost instance (has instanceOf but no ancestorInstance)
                if (selectedDesignElement.instanceOf) {
                    var descendantsList = canvas.elementModel.getDescendantsOfInstance(selectedDesignElement.instanceOf)
                    // Convert to a JavaScript array to ensure proper access in QML
                    var result = []
                    for (var i = 0; i < descendantsList.length; i++) {
                        result.push(descendantsList[i])
                    }
                    return result
                }
            }
        } catch (e) {
            // Handle case where objects are being destroyed
            return []
        }
        return []
    }
    
    // Signal emitted when PropertyPopover is clicked
    signal panelSelectorClicked(var selector, string type, var targetElement)
    
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
        // Sections for the main display element (outermost instance or selected element)
        
        // Non-collapsible sections that should always be visible
        ActionsSection {
            selectedElement: root.displayElement
            canvas: root.canvas
        }
        
        PlatformsSection {
            selectedElement: root.displayElement
            canvas: root.canvas
        }
        
        GeneralProperties {
            selectedElement: root.displayElement
            editableProperties: root.editableProperties
            canvas: root.canvas
        }
        
        // Show frame properties normally when not showing an outermost instance
        PositionSection {
            visible: root.outermostInstance === null
            selectedElement: root.displayElement
            editableProperties: root.editableProperties
        }
        
        SizeSection {
            visible: root.outermostInstance === null
            selectedElement: root.displayElement
            editableProperties: root.editableProperties
        }
        
        FrameStyleSection {
            visible: root.outermostInstance === null
            selectedElement: root.displayElement
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.displayElement)
            }
        }
        
        FlexLayoutSection {
            visible: root.outermostInstance === null
            selectedElement: root.displayElement
            editableProperties: root.editableProperties
        }
        
        // Collapsible section for Frame-specific properties when showing outermost instance
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            visible: root.outermostInstance !== null  // Only show when we have an outermost instance
            
            property bool isCollapsed: true
            
            // Separator
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#e0e0e0"
            }
            
            // Collapsible header
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                color: frameMouseArea.containsMouse ? "#f5f5f5" : "transparent"
                
                MouseArea {
                    id: frameMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: parent.parent.isCollapsed = !parent.parent.isCollapsed
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 5
                    
                    // Chevron icon
                    Text {
                        text: parent.parent.parent.isCollapsed ? "▶" : "▼"
                        font.pixelSize: 12
                        color: "#666"
                    }
                    
                    Label {
                        text: "Frame Properties"
                        font.weight: Font.Medium
                        font.pixelSize: 14
                        color: "#333"
                        Layout.fillWidth: true
                    }
                }
            }
            
            // Collapsible content
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                visible: !parent.isCollapsed
                
                PositionSection {
                    selectedElement: root.displayElement
                    editableProperties: root.editableProperties
                }
                
                SizeSection {
                    selectedElement: root.displayElement
                    editableProperties: root.editableProperties
                }
                
                FrameStyleSection {
                    selectedElement: root.displayElement
                    onPanelSelectorClicked: function(selector, type) {
                        root.panelSelectorClicked(selector, type, root.displayElement)
                    }
                }
                
                FlexLayoutSection {
                    selectedElement: root.displayElement
                    editableProperties: root.editableProperties
                }
            }
        }
        
        // Non-frame sections that should always be visible
        
        TextSection {
            selectedElement: root.displayElement
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.displayElement)
            }
        }
        
        ShapeSection {
            selectedElement: root.displayElement
            canvas: root.canvas
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.displayElement)
            }
        }
        
        VariableSection {
            selectedElement: root.displayElement
            canvas: root.canvas
        }
        
        // Render all sections for each descendant
        Repeater {
            model: root.descendants
            
            delegate: ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                
                property var descendantElement: root.descendants[index]
                property bool isCollapsed: true
                
                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#e0e0e0"
                }
                
                // Collapsible header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    color: mouseArea.containsMouse ? "#f5f5f5" : "transparent"
                    
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: isCollapsed = !isCollapsed
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 5
                        
                        // Chevron icon
                        Text {
                            text: isCollapsed ? "▶" : "▼"
                            font.pixelSize: 12
                            color: "#666"
                        }
                        
                        Label {
                            text: "Instance Child: " + (descendantElement ? (descendantElement.name || descendantElement.elementId) : "Unknown")
                            font.weight: Font.Medium
                            font.pixelSize: 14
                            color: "#333"
                            Layout.fillWidth: true
                        }
                    }
                }
                
                // Collapsible content
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    visible: !isCollapsed
                    
                    GeneralProperties {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        editableProperties: descendantElement && descendantElement.elementType === "FrameComponentInstance" ? descendantElement.getEditableProperties() : []
                        canvas: root.canvas
                    }
                    
                    PositionSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        editableProperties: descendantElement && descendantElement.elementType === "FrameComponentInstance" ? descendantElement.getEditableProperties() : []
                    }
                    
                    SizeSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        editableProperties: descendantElement && descendantElement.elementType === "FrameComponentInstance" ? descendantElement.getEditableProperties() : []
                    }
                    
                    FrameStyleSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        editableProperties: descendantElement && descendantElement.elementType === "FrameComponentInstance" ? descendantElement.getEditableProperties() : []
                        onPanelSelectorClicked: function(selector, type) {
                            root.panelSelectorClicked(selector, type, descendantElement)
                        }
                    }
                    
                    FlexLayoutSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        editableProperties: descendantElement && descendantElement.elementType === "FrameComponentInstance" ? descendantElement.getEditableProperties() : []
                    }
                    
                    TextSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        onPanelSelectorClicked: function(selector, type) {
                            root.panelSelectorClicked(selector, type, descendantElement)
                        }
                    }
                    
                    ShapeSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        canvas: root.canvas
                        onPanelSelectorClicked: function(selector, type) {
                            root.panelSelectorClicked(selector, type, descendantElement)
                        }
                    }
                    
                    VariableSection {
                        Layout.fillWidth: true
                        selectedElement: descendantElement
                        canvas: root.canvas
                    }
                }
            }
        }
        
        Item {
            Layout.fillHeight: true
        }
    }
}