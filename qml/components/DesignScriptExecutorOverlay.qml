import QtQuick
import Cubit 1.0

Item {
    id: root

    required property var elementModel
    required property bool canvasVisible

    property bool scriptsExecutedForCurrentLoad: false

    // Function to execute onEditorLoad events
    function executeOnEditorLoad() {

        // Execute onEditorLoad for canvas scripts
        if (Application.activeCanvas) {
            Application.activeCanvas.executeScriptEvent("onEditorLoad");
        }

        // Also execute onEditorLoad for all design elements
        if (elementModel) {
            var elements = elementModel.getAllElements();
            for (var i = 0; i < elements.length; i++) {
                var element = elements[i];
                if (!element)
                    continue;
                var elementType = element.elementType || "unknown";


                // Only Frame, Text, and Html elements (DesignElements) can execute scripts
                if (elementType === "Frame" || elementType === "Text" || elementType === "Html") {
                    try {
                        element.executeScriptEvent("onEditorLoad");
                    } catch (e) {
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
                    if (Application.activeCanvas) {
                        Application.activeCanvas.executeScriptEvent("onEditorLoad");
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
        } else if (canvasVisible && Application.activeCanvas) {
            // If no elements yet, just execute canvas scripts
            // Don't set scriptsExecutedForCurrentLoad to true here, as we may get elements later
            Application.activeCanvas.executeScriptEvent("onEditorLoad");
        }
    }

    // Watch for when elements are loaded into the model
    Connections {
        target: elementModel
        function onRowsInserted(parent, first, last) {
            if (canvasVisible && !scriptsExecutedForCurrentLoad) {
                scriptsExecutedForCurrentLoad = true;
                Qt.callLater(executeOnEditorLoad);
            }
        }
    }
}
