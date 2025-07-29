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
        "onWebLoad": {
            "id": "onWebLoad",
            "name": "On Web Load",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        },
        "oniOSLoad": {
            "id": "oniOSLoad",
            "name": "On iOS Load",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                }
            ]
        },
        "onAndroidLoad": {
            "id": "onAndroidLoad",
            "name": "On Android Load",
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
            "isAsync": false,
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
        },
        "onChange": {
            "id": "onChange",
            "name": "On Change",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                },
                {
                    "id": "value",
                    "type": "String",
                    "label": "Value"
                }
            ]
        },
        "onBlur": {
            "id": "onBlur",
            "name": "On Blur",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                },
                {
                    "id": "value",
                    "type": "String",
                    "label": "Value"
                }
            ]
        },
        "onFocus": {
            "id": "onFocus",
            "name": "On Focus",
            "type": "Event",
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                },
                {
                    "id": "value",
                    "type": "String",
                    "label": "Value"
                }
            ]
        },
        "eventData": {
            "id": "eventData",
            "name": "Event Data",
            "type": "Param",
            "sources": [
                {
                    "id": "value",
                    "type": "String",
                    "label": "Value"
                }
            ]
        },
        "aiPrompt": {
            "id": "aiPrompt",
            "name": "AI Prompt",
            "type": "Operation",
            "isAsync": true,
            "script": `(params, context) => { 
                return new Promise((resolve, reject) => {
                    const prompt = params.prompt || '';
                    if (!prompt) {
                        resolve({ response: 'No prompt provided' });
                        return;
                    }
                    
                    // Get auth token from the global authManager
                    const token = typeof authManager !== 'undefined' && authManager.getAuthToken ? authManager.getAuthToken() : '';
                    if (!token) {
                        resolve({ response: 'Authentication required' });
                        return;
                    }
                    
                    // Make GraphQL request to CubitAi
                    const xhr = new XMLHttpRequest();
                    const query = \`
                        query CallCubitAI($prompt: String!) {
                            CubitAi(prompt: $prompt)
                        }
                    \`;
                    
                    const body = JSON.stringify({
                        query: query,
                        variables: { prompt: prompt }
                    });
                    
                    xhr.open('POST', 'https://enrxjqdgdvc7pkragdib6abhyy.appsync-api.us-west-2.amazonaws.com/graphql');
                    xhr.setRequestHeader('Content-Type', 'application/json');
                    xhr.setRequestHeader('Authorization', token);
                    
                    xhr.onreadystatechange = function() {
                        if (xhr.readyState === XMLHttpRequest.DONE) {
                            if (xhr.status === 200) {
                                try {
                                    const response = JSON.parse(xhr.responseText);
                                    if (response.data && response.data.CubitAi) {
                                        resolve({ response: response.data.CubitAi });
                                    } else if (response.errors) {
                                        resolve({ response: 'Error: ' + JSON.stringify(response.errors) });
                                    } else {
                                        resolve({ response: 'Unknown error' });
                                    }
                                } catch (e) {
                                    resolve({ response: 'Failed to parse response: ' + e.toString() });
                                }
                            } else {
                                resolve({ response: 'HTTP Error: ' + xhr.status });
                            }
                        }
                    };
                    
                    xhr.send(body);
                });
            }`,
            "targets": [
                {
                    "id": "exec",
                    "type": "Flow",
                    "label": "Exec"
                },
                {
                    "id": "prompt",
                    "type": "String",
                    "label": "Prompt"
                }
            ],
            "sources": [
                {
                    "id": "done",
                    "type": "Flow",
                    "label": "Done"
                },
                {
                    "id": "response",
                    "type": "String",
                    "label": "Response"
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
        
        // Include isAsync if present
        if (nodeType.isAsync !== undefined) {
            nodeData.isAsync = nodeType.isAsync;
        }

        return nodeData;
    }
}
