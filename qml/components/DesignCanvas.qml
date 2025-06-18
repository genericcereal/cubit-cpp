import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

BaseCanvas {
    id: root
    
    canvasType: "design"
    
    // Update parentId of selected elements when hovering
    onHoveredElementChanged: {
        if (!selectionManager || selectionManager.selectionCount === 0) {
            return
        }
        
        // Get all selected elements
        var selectedElements = selectionManager.selectedElements
        
        // Update parentId for each selected element
        for (var i = 0; i < selectedElements.length; i++) {
            var element = selectedElements[i]
            if (element) {
                if (hoveredElement) {
                    // Don't set an element as its own parent
                    if (hoveredElement.elementId !== element.elementId) {
                        // Set parentId to the hovered element's ID
                        console.log("Setting parentId of", element.elementId, "to", hoveredElement.elementId)
                        element.parentId = hoveredElement.elementId
                    }
                } else {
                    // Clear parentId when no element is hovered
                    console.log("Clearing parentId of", element.elementId)
                    element.parentId = ""
                }
            }
        }
    }
    
    // Function to execute onEditorLoad events
    function executeOnEditorLoad() {
        console.log("DesignCanvas: executeOnEditorLoad called")
        
        // Execute onEditorLoad for canvas scripts
        if (Application.activeCanvas) {
            console.log("DesignCanvas: Executing canvas scripts")
            Application.activeCanvas.executeScriptEvent("onEditorLoad")
        }
        
        // Also execute onEditorLoad for all design elements
        if (elementModel) {
            var elements = elementModel.getAllElements()
            console.log("DesignCanvas: Found", elements.length, "elements")
            for (var i = 0; i < elements.length; i++) {
                var element = elements[i]
                if (!element) continue
                
                var elementType = element.elementType || "unknown"
                
                console.log("DesignCanvas: Element", i, "id:", element.elementId, 
                            "type:", elementType)
                
                // Only Frame, Text, and Html elements (DesignElements) can execute scripts
                if (elementType === "Frame" || elementType === "Text" || elementType === "Html") {
                    console.log("DesignCanvas: Found design element", element.elementId, "- executing script")
                    try {
                        element.executeScriptEvent("onEditorLoad")
                    } catch (e) {
                        console.log("DesignCanvas: Error executing script for", element.elementId, ":", e)
                    }
                }
            }
        }
    }
    
    property bool scriptsExecutedForCurrentLoad: false
    
    // Watch for when elementModel is set
    onElementModelChanged: {
        console.log("DesignCanvas: elementModel changed -", elementModel ? "set" : "null", "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (visible && elementModel && elementModel.rowCount() > 0 && !scriptsExecutedForCurrentLoad) {
            console.log("DesignCanvas: ElementModel set with elements, executing scripts")
            scriptsExecutedForCurrentLoad = true
            Qt.callLater(executeOnEditorLoad)
        }
    }
    
    // Execute when visible AND has elements
    onVisibleChanged: {
        console.log("DesignCanvas: onVisibleChanged - visible:", visible, "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (visible) {
            if (!scriptsExecutedForCurrentLoad) {
                scriptsExecutedForCurrentLoad = true
                if (elementModel && elementModel.rowCount() > 0) {
                    console.log("DesignCanvas: Visible with elements, executing scripts")
                    // Use Qt.callLater to ensure elements are fully loaded
                    Qt.callLater(executeOnEditorLoad)
                } else {
                    // If no elements yet, just execute canvas scripts
                    if (Application.activeCanvas) {
                        console.log("DesignCanvas: No elements, executing canvas scripts only")
                        Application.activeCanvas.executeScriptEvent("onEditorLoad")
                    }
                }
            }
        } else {
            // Reset the flag when becoming invisible
            scriptsExecutedForCurrentLoad = false
        }
    }
    
    // Also execute when first created if we already have elements
    Component.onCompleted: {
        console.log("DesignCanvas: Component.onCompleted - visible:", visible, "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (visible && elementModel && elementModel.rowCount() > 0) {
            scriptsExecutedForCurrentLoad = true
            executeOnEditorLoad()
        } else if (visible && Application.activeCanvas) {
            // If no elements yet, just execute canvas scripts
            // Don't set scriptsExecutedForCurrentLoad to true here, as we may get elements later
            Application.activeCanvas.executeScriptEvent("onEditorLoad")
        }
    }
    
    // Watch for when elements are loaded into the model
    Connections {
        target: elementModel
        function onRowsInserted(parent, first, last) {
            console.log("DesignCanvas: Rows inserted -", last - first + 1, "new elements")
            if (visible && !scriptsExecutedForCurrentLoad) {
                scriptsExecutedForCurrentLoad = true
                Qt.callLater(executeOnEditorLoad)
            }
        }
    }
    
    // Additional properties for design canvas
    property var hoveredElement: null
    property alias creationDragHandler: creationDragHandler
    
    // Clear hover when selection changes
    Connections {
        target: selectionManager
        function onSelectionChanged() {
            // If the hovered element is now selected, clear the hover
            if (hoveredElement && selectionManager.isSelected(hoveredElement)) {
                hoveredElement = null
            }
        }
    }
    
    // Clear hover when mode changes
    Connections {
        target: controller
        function onModeChanged() {
            hoveredElement = null
        }
    }
    
    // Add elements into the default contentData
    contentData: [
        Repeater {
            id: elementRepeater
            model: root.elementModel
            
            delegate: Loader {
                property var element: model.element
                property string elementType: model.elementType
                
                // Position elements relative to canvas origin
                x: element ? element.x - root.canvasMinX : 0
                y: element ? element.y - root.canvasMinY : 0
                
                sourceComponent: {
                    if (!element || !elementType) return null
                    switch(elementType) {
                        case "Frame": return frameComponent
                        case "Text": return textComponent
                        default: return null
                    }
                }
                
                onLoaded: {
                    if (item && element) {
                        item.element = element
                        item.elementModel = root.elementModel
                    }
                }
            }
        }
    ]
    
    // Creation drag handler
    QtObject {
        id: creationDragHandler
        property bool active: false
        property point startPoint: Qt.point(0, 0)
        property point currentPoint: Qt.point(0, 0)
        
        function start(point) {
            active = true
            startPoint = point
            currentPoint = point
        }
        
        function update(point) {
            currentPoint = point
        }
        
        function end() {
            // The C++ controller already handles element creation
            // We just need to reset our state
            active = false
        }
    }
    
    // Implement behavior by overriding handler functions
    function handleDragStart(pt) {
        if (controller.mode === "select") {
            controller.handleMousePress(pt.x, pt.y)
        } else {
            creationDragHandler.start(pt)
            controller.handleMousePress(pt.x, pt.y)
        }
    }
    
    function handleDragMove(pt) {
        if (creationDragHandler.active) {
            creationDragHandler.update(pt)
        }
        controller.handleMouseMove(pt.x, pt.y)
    }
    
    function handleDragEnd(pt) {
        if (creationDragHandler.active) {
            creationDragHandler.end()
        }
        controller.handleMouseRelease(pt.x, pt.y)
    }
    
    function handleClick(pt) {
        // Click is already handled in handleDragStart/End
        // This is called only for quick clicks without drag
        if (controller.mode === "select") {
            controller.handleMousePress(pt.x, pt.y)
            controller.handleMouseRelease(pt.x, pt.y)
        }
    }
    
    function handleHover(pt) {
        // Track hover for design elements (use hitTestForHover to exclude selected elements)
        if (controller.mode === "select") {
            hoveredElement = controller.hitTestForHover(pt.x, pt.y)
        } else {
            hoveredElement = null
        }
    }
    
    function handleExit() {
        hoveredElement = null
    }
    
    function handleSelectionRect(rect) {
        // Select design elements in rect
        controller.selectElementsInRect(rect)
    }
    
    // Component definitions
    Component {
        id: frameComponent
        FrameElement {}
    }
    
    Component {
        id: textComponent
        TextElement {}
    }
}