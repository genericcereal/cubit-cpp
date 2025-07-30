import QtQuick
import QtQml
import Cubit 1.0

Item {
    id: root
    
    property var elementModel
    property ViewportCache viewportCache
    property real zoomLevel: 1.0
    
    // Component definitions
    property Component frameComponent
    property Component textComponent
    
    // Map to track created items by element ID
    property var elementItems: ({})
    
    Instantiator {
        id: instantiator
        model: elementModel
        active: true
        
        delegate: Item {
            id: elementContainer
            
            required property var element
            required property string elementType
            required property int index
            
            parent: root
            visible: element && viewportCache ? viewportCache.isElementVisible(element.elementId, element.x, element.y, element.width, element.height) : false
            
            // Position elements relative to canvas origin using cached conversions
            x: element && viewportCache ? viewportCache.canvasXToRelative(element.x) : 0
            y: element && viewportCache ? viewportCache.canvasYToRelative(element.y) : 0
            width: element ? element.width : 0
            height: element ? element.height : 0
            
            // Create the actual element component
            Component.onCompleted: {
                var component = null
                switch(elementType) {
                    case "Frame": 
                        component = frameComponent
                        break
                    case "Text": 
                        component = textComponent
                        break
                    default:
                        console.warn("Unknown element type:", elementType)
                        return
                }
                
                if (component) {
                    var item = component.createObject(elementContainer, {
                        element: element,
                        elementModel: root.elementModel
                    })
                    
                    if (item) {
                        // Fill the container
                        item.anchors.fill = elementContainer
                        
                        // Store reference for cleanup
                        elementContainer.contentItem = item
                        
                        // Add to tracking map
                        if (element && element.elementId) {
                            root.elementItems[element.elementId] = elementContainer
                        }
                    }
                }
            }
            
            Component.onDestruction: {
                // Remove from tracking map
                if (element && element.elementId && root.elementItems[element.elementId] === elementContainer) {
                    delete root.elementItems[element.elementId]
                }
                
                // Clean up content item
                if (elementContainer.contentItem) {
                    elementContainer.contentItem.destroy()
                }
            }
            
            // Store reference to the created element
            property var contentItem: null
            
            // Update visibility when viewport changes
            Connections {
                target: viewportCache
                function onVisibilityChanged() {
                    if (element && viewportCache) {
                        elementContainer.visible = viewportCache.isElementVisible(
                            element.elementId, 
                            element.x, 
                            element.y, 
                            element.width, 
                            element.height
                        )
                    }
                }
            }
            
            // Update position and size when element changes
            Connections {
                target: element
                function onXChanged() {
                    if (viewportCache) {
                        elementContainer.x = viewportCache.canvasXToRelative(element.x)
                    }
                }
                function onYChanged() {
                    if (viewportCache) {
                        elementContainer.y = viewportCache.canvasYToRelative(element.y)
                    }
                }
                function onWidthChanged() {
                    elementContainer.width = element.width
                }
                function onHeightChanged() {
                    elementContainer.height = element.height
                }
            }
        }
        
        // Handle items being added
        onObjectAdded: (index, object) => {
            // Element added
        }
        
        // Handle items being removed
        onObjectRemoved: (index, object) => {
            // Element removed
        }
    }
    
    // Function to get an element item by ID (useful for external access)
    function getElementItem(elementId) {
        return elementItems[elementId] || null
    }
}