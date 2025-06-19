import QtQuick
import Cubit 1.0

Item {
    id: root
    
    required property var elementModel
    required property bool canvasVisible
    
    property bool scriptsExecutedForCurrentLoad: false
    
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
    
    // Watch for when elementModel is set
    onElementModelChanged: {
        console.log("DesignCanvas: elementModel changed -", elementModel ? "set" : "null", "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (canvasVisible && elementModel && elementModel.rowCount() > 0 && !scriptsExecutedForCurrentLoad) {
            console.log("DesignCanvas: ElementModel set with elements, executing scripts")
            scriptsExecutedForCurrentLoad = true
            Qt.callLater(executeOnEditorLoad)
        }
    }
    
    // Execute when canvasVisible AND has elements
    onVisibleChanged: {
        console.log("DesignCanvas: onVisibleChanged - canvasVisible:", canvasVisible, "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (canvasVisible) {
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
            // Reset the flag when becoming incanvasVisible
            scriptsExecutedForCurrentLoad = false
        }
    }
    
    // Also execute when first created if we already have elements
    Component.onCompleted: {
        console.log("DesignCanvas: Component.onCompleted - canvasVisible:", canvasVisible, "rowCount:", elementModel ? elementModel.rowCount() : 0)
        if (canvasVisible && elementModel && elementModel.rowCount() > 0) {
            scriptsExecutedForCurrentLoad = true
            executeOnEditorLoad()
        } else if (canvasVisible && Application.activeCanvas) {
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
            if (canvasVisible && !scriptsExecutedForCurrentLoad) {
                scriptsExecutedForCurrentLoad = true
                Qt.callLater(executeOnEditorLoad)
            }
        }
    }
}