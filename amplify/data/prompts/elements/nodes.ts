export const NODES_PROMPTS = `
NODE ELEMENT DOCUMENTATION:

Definition:
- Node elements are used in the script canvas for visual programming
- Element type: node
- Is container: false
- Top-level element (no parent required)

Properties:
- To be documented based on node implementation

Creation:
- Command: createElement
- Required fields: elementType, x, y, width, height
- Additional node-specific properties as needed

Positioning Rules:
- Nodes are top-level elements
- Should not overlap with other nodes
- Maintain adequate spacing for edge connections

Usage:
- Nodes are connected by edges to create visual programs
- Different node types serve different purposes in the script canvas`;