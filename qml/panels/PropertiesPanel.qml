import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cubit
import "../components"
import "../PropertyHelpers.js" as PropertyHelpers

ScrollView {
    id: root
    
    property var selectionManager
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
        }
        
        PlatformsSection {
            selectedElement: root.selectedElement
        }
        
        GeneralProperties {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
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
            onPanelSelectorClicked: root.panelSelectorClicked(selector, type)
        }
        
        FlexLayoutSection {
            selectedElement: root.selectedElement
            editableProperties: root.editableProperties
        }
        
        TextSection {
            selectedElement: root.selectedElement
        }
        
        VariableSection {
            selectedElement: root.selectedElement
        }
        
        Item {
            Layout.fillHeight: true
        }
    }
}