import QtQuick

QtObject {
    id: root

    // Node type definitions
    readonly property var catalog: {
        "userInput": {
            "id": "userInput",
            "name": "User Input",
            "type": "Event",
            "targets": [
                {
                    "id": "trigger",
                    "type": "Flow",
                    "label": "Trigger"
                }
            ],
            "sources": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Start"
                },
                {
                    "id": "value",
                    "type": "String",
                    "label": "Input"
                }
            ]
        },
        "displayOutput": {
            "id": "displayOutput",
            "name": "Display Output",
            "type": "Operation",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Execute"
                },
                {
                    "id": "value",
                    "type": "String",
                    "label": "Value"
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
        "mathOperation": {
            "id": "mathOperation",
            "name": "Math Operation",
            "type": "Operation",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Execute"
                },
                {
                    "id": "a",
                    "type": "Number",
                    "label": "A"
                },
                {
                    "id": "b",
                    "type": "Number",
                    "label": "B"
                }
            ],
            "sources": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Next"
                },
                {
                    "id": "result",
                    "type": "Number",
                    "label": "Result"
                }
            ]
        },
        "condition": {
            "id": "condition",
            "name": "Condition",
            "type": "Param",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Execute"
                },
                {
                    "id": "value",
                    "type": "Boolean",
                    "label": "Test"
                }
            ],
            "sources": [
                {
                    "id": "true",
                    "type": "Flow",
                    "label": "True"
                },
                {
                    "id": "false",
                    "type": "Flow",
                    "label": "False"
                }
            ]
        },
        "variable": {
            "id": "variable",
            "name": "Variable",
            "type": "Param",
            "targets": [
                {
                    "id": "set",
                    "type": "String",
                    "label": "Set"
                }
            ],
            "sources": [
                {
                    "id": "get",
                    "type": "String",
                    "label": "Get"
                }
            ]
        },
        "loop": {
            "id": "loop",
            "name": "For Loop",
            "type": "Param",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Execute"
                },
                {
                    "id": "start",
                    "type": "Number",
                    "label": "Start"
                },
                {
                    "id": "end",
                    "type": "Number",
                    "label": "End"
                }
            ],
            "sources": [
                {
                    "id": "loop",
                    "type": "Flow",
                    "label": "Loop Body"
                },
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                },
                {
                    "id": "index",
                    "type": "Number",
                    "label": "Index"
                }
            ]
        },
        "function": {
            "id": "function",
            "name": "Function",
            "type": "Operation",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Call"
                },
                {
                    "id": "param1",
                    "type": "String",
                    "label": "Param 1"
                },
                {
                    "id": "param2",
                    "type": "String",
                    "label": "Param 2"
                }
            ],
            "sources": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Return"
                },
                {
                    "id": "result",
                    "type": "String",
                    "label": "Result"
                }
            ]
        },
        "onLoad": {
            "id": "onLoad",
            "name": "On Load",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        },
        "httpRequest": {
            "id": "httpRequest",
            "name": "HTTP Request",
            "type": "Operation",
            "targets": [
                {
                    "id": "flow",
                    "type": "Flow",
                    "label": "Execute"
                },
                {
                    "id": "url",
                    "type": "String",
                    "label": "URL"
                },
                {
                    "id": "method",
                    "type": "String",
                    "label": "Method"
                },
                {
                    "id": "body",
                    "type": "String",
                    "label": "Body"
                }
            ],
            "sources": [
                {
                    "id": "success",
                    "type": "Flow",
                    "label": "Success"
                },
                {
                    "id": "error",
                    "type": "Flow",
                    "label": "Error"
                },
                {
                    "id": "response",
                    "type": "String",
                    "label": "Response"
                },
                {
                    "id": "status",
                    "type": "Number",
                    "label": "Status"
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
        return {
            "name": nodeType.name,
            "type": nodeType.type || "Operation",
            "x": x,
            "y": y,
            "targets": nodeType.targets || [],
            "sources": nodeType.sources || []
        };
    }
}
