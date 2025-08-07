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
    
    // Helper property to access DesignElement-specific properties
    property var selectedDesignElement: selectedElement && selectedElement.isDesignElement ? selectedElement : null
    
    // Helper property to get editable properties for ComponentInstance
    property var editableProperties: {
        if (selectedElement && selectedElement.elementType === "FrameComponentInstance") {
            return selectedElement.getEditableProperties()
        }
        return []
    }
    
    // Get all descendants when an outermost instance is selected
    property var descendants: {
        if (selectedDesignElement && selectedDesignElement.isInstance && 
            selectedDesignElement.isInstance() && 
            (!selectedDesignElement.ancestorInstance || selectedDesignElement.ancestorInstance === "")) {
            // This is an outermost instance (has instanceOf but no ancestorInstance)
            var elementModel = canvas ? canvas.elementModel : null
            if (elementModel && selectedDesignElement.instanceOf) {
                var descendantsList = elementModel.getDescendantsOfInstance(selectedDesignElement.instanceOf)
                // Convert to a JavaScript array to ensure proper access in QML
                var result = []
                for (var i = 0; i < descendantsList.length; i++) {
                    result.push(descendantsList[i])
                }
                return result
            }
        }
        return []
    }
    
    // Signal emitted when PropertyPopover is clicked
    signal panelSelectorClicked(var selector, string type, var targetElement)
    
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
        // Sections for the main selected element
        ActionsSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
        }
        
        PlatformsSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
        }
        
        GeneralProperties {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
            canvas: root.canvas
        }
        
        PositionSection {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
        }
        
        SizeSection {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
        }
        
        FrameStyleSection {
            selectedElement: root.selectedElement
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.selectedElement)
            }
        }
        
        FlexLayoutSection {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
        }
        
        TextSection {
            selectedElement: root.selectedElement
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.selectedElement)
            }
        }
        
        ShapeSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type, root.selectedElement)
            }
        }
        
        VariableSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
        }
        
        // Render all sections for each descendant
        Repeater {
            model: root.descendants
            
            delegate: ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                
                property var descendantElement: root.descendants[index]
                
                // Separator and header for this descendant
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#e0e0e0"
                }
                
                Label {
                    text: "Instance Child: " + (descendantElement ? (descendantElement.name || descendantElement.elementId) : "Unknown")
                    font.weight: Font.Medium
                    font.pixelSize: 14
                    color: "#333"
                    padding: 10
                    Layout.fillWidth: true
                }
                
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
        
        Item {
            Layout.fillHeight: true
        }
    }
}