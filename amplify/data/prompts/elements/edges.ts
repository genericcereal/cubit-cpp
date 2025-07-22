export const EDGES_PROMPTS = `
EDGE ELEMENT DOCUMENTATION:

Definition:
- Edge elements connect nodes in the script canvas
- Element type: edge
- Is container: false
- Requires source and target nodes

Properties:
- sourceNodeId: ID of the source node
- targetNodeId: ID of the target node
- Additional properties for styling and routing as needed

Creation:
- Command: createElement
- Required fields: elementType, sourceNodeId, targetNodeId
- Edges are created after nodes exist

Positioning Rules:
- Edges automatically route between connected nodes
- Position is determined by source and target node locations

Usage:
- Edges represent data flow or control flow between nodes
- Must connect existing nodes`;