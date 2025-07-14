import QtQuick

QtObject {
    id: root

    // Node type definitions
    readonly property var catalog: {
        "onEditorLoad": {
            "id": "onEditorLoad",
            "name": "On Editor Load",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        },
        "consoleLog": {
            "id": "consoleLog",
            "name": "Console Log",
            "type": "Operation",
            "script": "(params) => { console.log(params.message || ''); }",
            "targets": [
                {
                    "id": "exec",
                    "type": "Flow",
                    "label": "Exec"
                },
                {
                    "id": "message",
                    "type": "String",
                    "label": "Message"
                }
            ],
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        },
        "componentOnEditorLoadEvents": {
            "id": "componentOnEditorLoadEvents",
            "name": "Component On Editor Load Events",
            "type": "Operation",
            "targets": [
                {
                    "id": "exec",
                    "type": "Flow",
                    "label": "Exec"
                }
            ],
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        }
    }

    // Helper function to get a node definition
    function getNodeType(typeName) {
        return catalog[typeName] || null;
    }

    // Get all available node types
    function getAllNodeTypes() {
        return Object.keys(catalog);
    }

    // Create node data with position
    function createNodeData(typeName, x, y) {
        var nodeType = getNodeType(typeName);
        if (!nodeType) {
            console.warn("NodeCatalog: Unknown node type:", typeName);
            return null;
        }

        // Create a copy with position
        var nodeData = {
            "name": nodeType.name,
            "type": nodeType.type || "Operation",
            "x": x,
            "y": y,
            "targets": nodeType.targets || [],
            "sources": nodeType.sources || []
        };

        // Include script if present
        if (nodeType.script) {
            nodeData.script = nodeType.script;
        }

        return nodeData;
    }
}
