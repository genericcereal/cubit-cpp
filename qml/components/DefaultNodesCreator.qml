import QtQuick
import QtQuick.Controls
import Cubit 1.0
import "."

QtObject {
    id: root

    property var controller: null
    
    // Node catalog instance
    property var nodeCatalog: NodeCatalog {}

    function createDefaultNodes() {
        console.log("DefaultNodesCreator.createDefaultNodes called");
        if (!controller) {
            console.log("  Missing controller");
            return;
        }

        // Example 1: Create a user input node using the catalog
        var userInputData = nodeCatalog.createNodeData("userInput", -400, -100);
        if (userInputData) {
            console.log("  Creating user input node from catalog");
            controller.createNodeFromJson(JSON.stringify(userInputData));
        }

        // Example 2: Create multiple nodes using the catalog
        var mathOpData = nodeCatalog.createNodeData("mathOperation", -200, -100);
        if (mathOpData) {
            console.log("  Creating math operation node from catalog");
            controller.createNodeFromJson(JSON.stringify(mathOpData));
        }
        
        var displayData = nodeCatalog.createNodeData("displayOutput", 0, -100);
        if (displayData) {
            console.log("  Creating display output node from catalog");
            controller.createNodeFromJson(JSON.stringify(displayData));
        }
        
        var conditionData = nodeCatalog.createNodeData("condition", -200, 50);
        if (conditionData) {
            console.log("  Creating condition node from catalog");
            controller.createNodeFromJson(JSON.stringify(conditionData));
        }

        // Example 3: Create a complete graph with catalog nodes
        var inputNode = nodeCatalog.createNodeData("userInput", -400, 200);
        var loopNode = nodeCatalog.createNodeData("loop", -200, 200);
        var functionNode = nodeCatalog.createNodeData("function", 0, 200);
        var httpNode = nodeCatalog.createNodeData("httpRequest", 200, 200);
        
        // Assign IDs for edge connections
        inputNode.id = "input1";
        loopNode.id = "loop1";
        functionNode.id = "func1";
        httpNode.id = "http1";
        
        var graphJson = JSON.stringify({
            nodes: [inputNode, loopNode, functionNode, httpNode],
            edges: [
                {
                    sourceNodeId: "input1",
                    targetNodeId: "loop1",
                    sourcePortId: "flow",
                    targetPortId: "flow"
                },
                {
                    sourceNodeId: "loop1",
                    targetNodeId: "func1",
                    sourcePortId: "loop",
                    targetPortId: "flow"
                },
                {
                    sourceNodeId: "func1",
                    targetNodeId: "http1",
                    sourcePortId: "flow",
                    targetPortId: "flow"
                }
            ]
        });

        console.log("  Creating complete graph from catalog");
        controller.createGraphFromJson(graphJson);

        console.log("  All example nodes created");
    }
}
