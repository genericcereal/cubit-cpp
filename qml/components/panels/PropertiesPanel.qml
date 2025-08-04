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
    
    // Signal emitted when PropertyPopover is clicked
    signal panelSelectorClicked(var selector, string type)
    
    
    ColumnLayout {
        width: root.width
        spacing: 10
        
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
                root.panelSelectorClicked(selector, type)
            }
        }
        
        FlexLayoutSection {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
        }
        
        TextSection {
            selectedElement: root.selectedElement
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type)
            }
        }
        
        ShapeSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
            onPanelSelectorClicked: function(selector, type) {
                root.panelSelectorClicked(selector, type)
            }
        }
        
        VariableSection {
            selectedElement: root.selectedElement
            canvas: root.canvas
        }
        
        Item {
            Layout.fillHeight: true
        }
    }
}