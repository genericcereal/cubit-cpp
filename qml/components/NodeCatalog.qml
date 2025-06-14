import QtQuick

QtObject {
    id: root
    
    // Node type definitions
    readonly property var catalog: {
        "userInput": {
            "id": "userInput",
            "name": "User Input",
            "color": "#4CAF50",
            "targets": [
                {"id": "trigger", "type": "Flow", "label": "Trigger"}
            ],
            "sources": [
                {"id": "flow", "type": "Flow", "label": "Start"},
                {"id": "value", "type": "Variable", "label": "Input"}
            ]
        },
        
        "displayOutput": {
            "id": "displayOutput",
            "name": "Display Output",
            "color": "#2196F3",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Execute"},
                {"id": "value", "type": "Variable", "label": "Value"}
            ],
            "sources": [
                {"id": "done", "type": "Flow", "label": "Done"}
            ]
        },
        
        "mathOperation": {
            "id": "mathOperation",
            "name": "Math Operation",
            "color": "#FF9800",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Execute"},
                {"id": "a", "type": "Variable", "label": "A"},
                {"id": "b", "type": "Variable", "label": "B"}
            ],
            "sources": [
                {"id": "flow", "type": "Flow", "label": "Next"},
                {"id": "result", "type": "Variable", "label": "Result"}
            ]
        },
        
        "condition": {
            "id": "condition",
            "name": "Condition",
            "color": "#9C27B0",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Execute"},
                {"id": "value", "type": "Variable", "label": "Test"}
            ],
            "sources": [
                {"id": "true", "type": "Flow", "label": "True"},
                {"id": "false", "type": "Flow", "label": "False"}
            ]
        },
        
        "variable": {
            "id": "variable",
            "name": "Variable",
            "color": "#607D8B",
            "targets": [
                {"id": "set", "type": "Variable", "label": "Set"}
            ],
            "sources": [
                {"id": "get", "type": "Variable", "label": "Get"}
            ]
        },
        
        "loop": {
            "id": "loop",
            "name": "For Loop",
            "color": "#E91E63",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Execute"},
                {"id": "start", "type": "Variable", "label": "Start"},
                {"id": "end", "type": "Variable", "label": "End"}
            ],
            "sources": [
                {"id": "loop", "type": "Flow", "label": "Loop Body"},
                {"id": "done", "type": "Flow", "label": "Done"},
                {"id": "index", "type": "Variable", "label": "Index"}
            ]
        },
        
        "function": {
            "id": "function",
            "name": "Function",
            "color": "#00BCD4",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Call"},
                {"id": "param1", "type": "Variable", "label": "Param 1"},
                {"id": "param2", "type": "Variable", "label": "Param 2"}
            ],
            "sources": [
                {"id": "flow", "type": "Flow", "label": "Return"},
                {"id": "result", "type": "Variable", "label": "Result"}
            ]
        },
        
        "httpRequest": {
            "id": "httpRequest",
            "name": "HTTP Request",
            "color": "#FFC107",
            "targets": [
                {"id": "flow", "type": "Flow", "label": "Execute"},
                {"id": "url", "type": "Variable", "label": "URL"},
                {"id": "method", "type": "Variable", "label": "Method"},
                {"id": "body", "type": "Variable", "label": "Body"}
            ],
            "sources": [
                {"id": "success", "type": "Flow", "label": "Success"},
                {"id": "error", "type": "Flow", "label": "Error"},
                {"id": "response", "type": "Variable", "label": "Response"},
                {"id": "status", "type": "Variable", "label": "Status"}
            ]
        }
    }
    
    // Helper function to get a node definition
    function getNodeType(typeName) {
        return catalog[typeName] || null
    }
    
    // Get all available node types
    function getAllNodeTypes() {
        return Object.keys(catalog)
    }
    
    // Create node data with position
    function createNodeData(typeName, x, y) {
        var nodeType = getNodeType(typeName)
        if (!nodeType) {
            console.warn("NodeCatalog: Unknown node type:", typeName)
            return null
        }
        
        // Create a copy with position
        return {
            "name": nodeType.name,
            "x": x,
            "y": y,
            "color": nodeType.color || "",
            "targets": nodeType.targets || [],
            "sources": nodeType.sources || []
        }
    }
}