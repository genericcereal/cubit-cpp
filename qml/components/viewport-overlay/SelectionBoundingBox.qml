import QtQuick
import Cubit 1.0

// Visual bounding box that surrounds all selected elements
Rectangle {
    id: root
    
    // Properties for canvas coordinate system
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
        if (!Application || !Application.activeSelectionManager) return false
        var selectedElements = Application.activeSelectionManager.selectedElements
        if (selectedElements.length === 0) return false
        
        for (var i = 0; i < selectedElements.length; i++) {
            var element = selectedElements[i]
            if (!element) continue
            
            // Check if it's a ComponentInstance or ComponentVariant
            if (element.elementType !== "ComponentInstance" && element.elementType !== "ComponentVariant") {
                return false
            }
        }
        return true
    }
    
    // Visual properties
    color: "transparent"
    border.color: allSelectedAreComponentRelated ? ConfigObject.componentControlsBorderColorString : ConfigObject.controlsBorderColorString
    border.width: 1
    visible: boundingWidth > 0 && boundingHeight > 0
    
    // Convert canvas coordinates to viewport coordinates
    x: (boundingX - canvasMinX) * zoomLevel - (flickable?.contentX ?? 0)
    y: (boundingY - canvasMinY) * zoomLevel - (flickable?.contentY ?? 0)
    width: boundingWidth * zoomLevel
    height: boundingHeight * zoomLevel
}