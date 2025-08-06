import QtQuick
import Cubit 1.0

// Visual bounding box that surrounds all selected elements
Rectangle {
    id: root
    
    Component.onCompleted: {
    }
    
    // Properties for canvas coordinate system
    property var selectionManager: null  // Selection manager passed from parent
    property var canvas: null  // Canvas/Project passed from parent to check editingElement
    property real canvasMinX: 0
    property real canvasMinY: 0
    property real zoomLevel: 1.0
    property var flickable: null
    
    // Bounding box in canvas coordinates
    property real boundingX: 0
    property real boundingY: 0
    property real boundingWidth: 0
    property real boundingHeight: 0
    
    // Check if all selected elements are component-related
    property bool allSelectedAreComponentRelated: {
        if (!selectionManager) {
            return false
        }
        var selectedElements = selectionManager.selectedElements
        if (selectedElements.length === 0) {
            return false
        }
        
        for (var i = 0; i < selectedElements.length; i++) {
            var element = selectedElements[i]
            if (!element) {
                continue
            }
            
            // Debug logging
            
            // Check if element has instanceOf property set (component instance)
            // instanceOf is a QString property on DesignElement
            var hasInstanceOf = element.instanceOf !== undefined && element.instanceOf !== null && element.instanceOf !== ""
            
            if (!hasInstanceOf) {
                return false
            }
        }
        return true
    }
    
    // Visual properties
    color: "transparent"
    border.color: {
        // Use purple color when editingElement is defined (editing a component/variant)
        var isEditingElement = canvas && canvas.editingElement !== null && canvas.editingElement !== undefined
        var color = (isEditingElement || allSelectedAreComponentRelated) ? ConfigObject.componentControlsBorderColorString : ConfigObject.controlsBorderColorString
        return color
    }
    border.width: 1
    
    // Convert canvas coordinates to viewport coordinates
    x: (boundingX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
    y: (boundingY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
    width: boundingWidth * zoomLevel
    height: boundingHeight * zoomLevel
}