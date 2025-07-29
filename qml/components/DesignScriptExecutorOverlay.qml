import QtQuick
import Cubit 1.0

Item {
    id: root

    required property var elementModel
    required property bool canvasVisible
    required property var canvas

    property bool scriptsExecutedForCurrentLoad: false

    // Function to execute onEditorLoad events
    function executeOnEditorLoad() {

        // Execute onEditorLoad for canvas scripts
        if (root.canvas) {
            root.canvas.executeScriptEvent("onEditorLoad");
        }

        // Also execute onEditorLoad for all design elements
        if (elementModel) {
            var elements = elementModel.getAllElements();
            for (var i = 0; i < elements.length; i++) {
                var element = elements[i];
                if (!element)
                    continue;
                var elementType = element.elementType || "unknown";

                // DesignScriptExecutorOverlay - Checking element

                // Only DesignElements (Frame, Text, WebTextInput, and their ComponentInstance variants) can execute scripts
                if (elementType === "Frame" || elementType === "Text" || elementType === "WebTextInput" ||
                    elementType === "FrameComponentInstance" || elementType === "TextComponentInstance") {
                    try {
                        element.executeScriptEvent("onEditorLoad");
                    } catch (e) {
                        console.error("  -> Error executing onEditorLoad for element:", element.elementId, "error:", e);
                    }
                }
            }
        }
    }

    // Watch for when elementModel is set
    onElementModelChanged: {
        if (canvasVisible && elementModel && elementModel.rowCount() > 0 && !scriptsExecutedForCurrentLoad) {
            scriptsExecutedForCurrentLoad = true;
            Qt.callLater(executeOnEditorLoad);
        }
    }

    // Execute when canvasVisible AND has elements
    onVisibleChanged: {
        if (canvasVisible) {
            if (!scriptsExecutedForCurrentLoad) {
                scriptsExecutedForCurrentLoad = true;
                if (elementModel && elementModel.rowCount() > 0) {
                    // Use Qt.callLater to ensure elements are fully loaded
                    Qt.callLater(executeOnEditorLoad);
                } else {
                    // If no elements yet, just execute canvas scripts
                    if (root.canvas) {
                        root.canvas.executeScriptEvent("onEditorLoad");
                    }
                }
            }
        } else {
            // Reset the flag when becoming incanvasVisible
            scriptsExecutedForCurrentLoad = false;
        }
    }

    // Also execute when first created if we already have elements
    Component.onCompleted: {
        if (canvasVisible && elementModel && elementModel.rowCount() > 0) {
            scriptsExecutedForCurrentLoad = true;
            executeOnEditorLoad();
        } else if (canvasVisible && root.canvas) {
            // If no elements yet, just execute canvas scripts
            // Don't set scriptsExecutedForCurrentLoad to true here, as we may get elements later
            root.canvas.executeScriptEvent("onEditorLoad");
        }
    }

    // Watch for when canvas becomes visible
    // The onEditorLoad scripts will be executed when canvasVisible becomes true
    onCanvasVisibleChanged: {
        if (canvasVisible && !scriptsExecutedForCurrentLoad) {
            scriptsExecutedForCurrentLoad = true;
            Qt.callLater(executeOnEditorLoad);
        }
    }
}
