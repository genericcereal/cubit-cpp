import QtQuick
import QtQuick.Controls
import Cubit 1.0

QtObject {
    id: root

    property var controller: null

    function createDefaultNodes() {
        console.log("DefaultNodesCreator.createDefaultNodes called");
        if (!controller) {
            console.log("  Missing controller");
            return;
        }

        // Example 1: Create a single node from JSON
        var singleNodeJson = JSON.stringify({
            id: "node_1",
            name: "Data Processor",
            x: -200,
            y: -100,
            color: "#FFE6E6"  // Light red
            ,
            targets: [
                {
                    id: "flow_in",
                    type: "Flow",
                    label: "Start"
                },
                {
                    id: "data_in",
                    type: "Variable",
                    label: "Input Data"
                }
            ],
            sources: [
                {
                    id: "flow_out",
                    type: "Flow",
                    label: "Next"
                },
                {
                    id: "result",
                    type: "Variable",
                    label: "Result"
                },
                {
                    id: "error",
                    type: "Variable",
                    label: "Error"
                }
            ]
        });

        console.log("  Creating single node from JSON");
        controller.createNodeFromJson(singleNodeJson);

        // Example 2: Create multiple nodes from JSON array
        var multipleNodesJson = JSON.stringify([
            {
                id: "validator",
                name: "Validator",
                x: 50,
                y: -150,
                color: "#E6F3FF"  // Light blue
                ,
                targets: [
                    {
                        id: "flow_in",
                        type: "Flow",
                        label: "Execute"
                    },
                    {
                        id: "value",
                        type: "Variable",
                        label: "Value"
                    }
                ],
                sources: [
                    {
                        id: "flow_valid",
                        type: "Flow",
                        label: "Valid"
                    },
                    {
                        id: "flow_invalid",
                        type: "Flow",
                        label: "Invalid"
                    }
                ]
            },
            {
                id: "logger",
                name: "Logger",
                x: 50,
                y: 50,
                color: "#FFFACD"  // Light yellow
                ,
                targets: [
                    {
                        id: "flow_in",
                        type: "Flow",
                        label: "Log"
                    },
                    {
                        id: "message",
                        type: "Variable",
                        label: "Message"
                    },
                    {
                        id: "level",
                        type: "Variable",
                        label: "Level"
                    }
                ]
            }
        ]);

        console.log("  Creating multiple nodes from JSON array");
        controller.createNodesFromJson(multipleNodesJson);

        // Example 3: Create a complete graph with nodes and edges
        var graphJson = JSON.stringify({
            nodes: [
                {
                    id: "input",
                    name: "User Input",
                    x: -400,
                    y: 100,
                    sources: [
                        {
                            id: "flow",
                            type: "Flow",
                            label: "Start"
                        },
                        {
                            id: "value",
                            type: "Variable",
                            label: "Input"
                        }
                    ]
                },
                {
                    id: "transform",
                    name: "Transform",
                    x: -200,
                    y: 100,
                    targets: [
                        {
                            id: "flow_in",
                            type: "Flow",
                            label: "Execute"
                        },
                        {
                            id: "input",
                            type: "Variable",
                            label: "Data"
                        }
                    ],
                    sources: [
                        {
                            id: "flow_out",
                            type: "Flow",
                            label: "Complete"
                        },
                        {
                            id: "output",
                            type: "Variable",
                            label: "Transformed"
                        }
                    ]
                }
            ],
            edges: [
                {
                    sourceNodeId: "input",
                    targetNodeId: "transform",
                    sourcePortId: "flow",
                    targetPortId: "flow_in"
                },
                {
                    sourceNodeId: "input",
                    targetNodeId: "transform",
                    sourcePortId: "value",
                    targetPortId: "input"
                }
            ]
        });

        console.log("  Creating complete graph from JSON");
        controller.createGraphFromJson(graphJson);

        console.log("  All example nodes created");
    }
}
