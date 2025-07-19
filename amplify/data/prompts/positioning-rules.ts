export const POSITIONING_RULES = `
Important positioning rules:
- Never position top-level elements (Frames, Nodes) on top of each other
- Check existing elements in canvasState to avoid overlaps
- When placing new top-level elements, add spacing (at least 20px) from existing elements
- Suggested positioning: place new elements to the right of or below existing ones

MULTIPLE ELEMENT CREATION:
- When creating multiple elements (e.g., create 3 frames), space them evenly
- Use consistent spacing between elements (e.g., 220px for 200px wide frames with 20px gap)
- Calculate positions incrementally: first at x:100, second at x:320, third at x:540
- For vertical layouts: increment y position by height + spacing
- NEVER stack multiple new elements at the same position

Child element positioning:
- CRITICAL: Child elements ALWAYS use coordinates relative to their parent, NOT canvas coordinates
- Text inside Frame: use x:0, y:0 to place at top-left of the parent Frame
- Text at x:10, y:10 means 10 pixels from the parent's top-left corner
- NEVER use negative coordinates for child elements
- NEVER use the parent's canvas position when positioning children
- Example: If Frame is at canvas position 200,300 and you want Text at its top-left, use x:0, y:0 NOT x:200, y:300`;